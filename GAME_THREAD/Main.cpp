#include "icb_gui.h"
#include <cstdlib>
#include <ctime>
#include <windows.h>
//sebahattin gökcen Özden


HANDLE FireMutex;


int FRM1;
int keypressed; // Klavyeden basýlan tuþun ASCII deðeri
ICBYTES m;

void ICGUI_Create() {
    ICG_MWTitle("GAME");
    ICG_MWSize(450, 500);
}

// Thread fonksiyonu
VOID* SlidingBox(PVOID lpParam) {
    void** sParams = (void**)lpParam;
    int* pBoxX = (int*)sParams[0];
    int* pBoxY = (int*)sParams[1];
    bool* pThreadContinue = (bool*)sParams[2];

    while (*pThreadContinue) { // Düzeltme 1: pointer dereferencing
        FillRect(m, *pBoxX, *pBoxY, 20, 6, 0x000000); //20 geniþliðinde,6 boyunda bir kutu oluþturur.
        if (keypressed == 37) *pBoxX -= 2; // sol  
        if (keypressed == 39) *pBoxX += 2; // sað
        if (keypressed == 38); // yukarý
        if (keypressed == 40); // aþaðý

        if (*pBoxX < 0) *pBoxX = 0;   // Eðer sol sýnýrdaysa, sýfýrlanýr
        if (*pBoxX > 380) *pBoxX = 380; // Eðer sað sýnýrdaysa, 380'e ayarlanýr
        FillRect(m, *pBoxX, *pBoxY, 20, 6, 0xff0000);
        DisplayImage(FRM1, m);
        Sleep(30);
    }
    return NULL;
}
VOID* BulletMove(PVOID lpParam) {
    void** bParams = (void**)lpParam;
    int* BulletX = (int*)bParams[0];
    int* BulletY = (int*)bParams[1];
    bool* bThreadContinue = (bool*)bParams[2];
    int* fBoxX = (int*)bParams[3];
    int* fBoxY = (int*)bParams[4];
    int* sBoxX = (int*)bParams[5];
    int* sBoxY = (int*)bParams[6];
    //bool* bAteslendi_mi = (bool*)bParams[7];
    bool* collusion = (bool*)bParams[7];

    while (*bThreadContinue) {
        FillRect(m, *BulletX, *BulletY, 1, 1, 0x000000);

        // Eðer space tuþuna basýldýysa (ateþ edildi)
        if (keypressed == 32) {

            DWORD waitResult = WaitForSingleObject(FireMutex, 0);

            if (waitResult == WAIT_OBJECT_0) {
                *BulletX = *sBoxX + 10;
                *BulletY = *sBoxY - 2;

            }
        }

        // Ateþlendiðinde yukarý doðru hareket etsin
        if (*BulletY < 378) *BulletY -= 4;

        //çarpýþma olduysa ya da mermi sýnýra geldiyse gemide tekrar oluþ
        if (BulletY <= 0 || (
            *BulletX >= *fBoxX && *BulletX <= *fBoxX + 30 &&
            *BulletY >= *fBoxY && *BulletY <= *fBoxY + 30)
            ) {
            if (// çarpýþma olduysa collusion true dönder
                *BulletX >= *fBoxX && *BulletX <= *fBoxX + 30 &&
                *BulletY >= *fBoxY && *BulletY <= *fBoxY + 30)
            {
                    *collusion = true;
            }
            *BulletY = 378;
            *BulletX = *sBoxX + 10;
            ReleaseMutex(FireMutex);
            keypressed = 0;
        }
        
        if (*BulletY >= 378) {
            *BulletX = *sBoxX + 10;
        }


        FillRect(m, *BulletX, *BulletY, 1, 1, 0xFFFFFF);
        DisplayImage(FRM1, m);
        Sleep(30);
    }
    return NULL;
}


VOID* FallingBox(PVOID lpParam) {
    
    void** fParams = (void**)lpParam;
    int* fBoxX = (int*)fParams[0];
    int* fBoxY = (int*)fParams[1];
    bool* fThreadContinue = (bool*)fParams[2];
    int* BulletX = (int*)fParams[3];
    int* BulletY = (int*)fParams[4];
    bool* collusion = (bool*)fParams[5];
    bool* flying = (bool*)fParams[6];

    

    while (*fThreadContinue) {
        FillRect(m, *fBoxX, *fBoxY, 30, 30, 0x000000);

        if (*flying) {
            *fBoxY -= 2;  // "Uçma" modunda yukarý hareket
            if (*BulletX < *fBoxX + 9) *fBoxX += 2; // Sað üst köþeye gider
            else if (*BulletX > *fBoxX + 21) *fBoxX -= 2; // Sol üst köþeye gider

            if (*fBoxY <= 0 || *fBoxX < -10 || *fBoxX>390) {  // Kutunun üst sýnýrý geçtiðinde yeniden baþla
                *fBoxX = rand() % 360 + 10;
                *fBoxY = 0;
                *flying = false;
            }
        }
        else {
            (*fBoxY)++;  // Normal düþme hareketi
            if (*fBoxY >= 380) { // Alt sýnýrý geçtiðinde rastgele baþla
                *fBoxX = rand() % 360 + 10;
                *fBoxY = 0;
            }
        }

        // Çarpýþma kontrolü
        if (*collusion && !*flying) {
            if (*BulletX >= *fBoxX && *BulletX < *fBoxX + 9) { // Sol bölgeye çarptý
                *flying = true;
            }
            else if (*BulletX > *fBoxX + 21 && *BulletX <= *fBoxX + 30) { // Sað bölgeye çarptý
                *flying = true;
            }
            else { // Orta bölgeye çarptýysa yeniden baþlat
                *fBoxX = rand() % 360 + 10;
                *fBoxY = 0;
            }
            *collusion = false; // Çarpýþma durumunu sýfýrla
        }

        // Kutunun bölgeye göre renk çizimi
        FillRect(m, *fBoxX, *fBoxY, 9, 30, 0xB8B8B8);      // Sol bölge (gri)
        FillRect(m, *fBoxX + 9, *fBoxY, 12, 30, 0x55FF55); // Orta bölge (yeþil)
        FillRect(m, *fBoxX + 21, *fBoxY, 9, 30, 0xB8B8B8);  // Sað bölge (gri)

        DisplayImage(FRM1, m);
        Sleep(30);
    }

    return NULL;
}



void butonfonk() {
    //threadler için durum
    static bool thread_continue = false;
    
    //slidingbox bilgileri
    static void* slidingParams[3];
    static int slidingBoxX = 200;          
    static int slidingBoxY = 380;
    
    //fallingbox parametreleri
    static void* fallingBoxParams[7];
    static int fallingBoxX = rand() % 360 + 10;
    static int fallingBoxY = 0;
    static bool flying = false; // kutu vurulduktan sonra pause olduðunda bilgileri tutmalýyýs
    
    //mermi parametreleri
    static void* bulletParams[8];
    static int bulletX = slidingBoxX + 10;// mermi konumu slidingbox'ýn ortasýnda konumlanmalý
    static int bulletY = slidingBoxY-2; // üstünde konumlan
    //static bool ateslendi_mi = false; // artýk mutex için devre dýþý býrakýyorum bunu
    static bool collusion = false;
    

    if (!thread_continue) {
        if (FireMutex == NULL) {
            FireMutex = CreateMutex(
                NULL,
                TRUE,
                NULL
            );
        }

        thread_continue = true;

        slidingParams[0] = &slidingBoxX;         
        slidingParams[1] = &slidingBoxY;
        slidingParams[2] = &thread_continue;

        DWORD dw1;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SlidingBox, slidingParams, 0, &dw1);
        
        fallingBoxParams[0] = &fallingBoxX;
        fallingBoxParams[1] = &fallingBoxY;
        fallingBoxParams[2] = &thread_continue;
        fallingBoxParams[3] = &bulletX;
        fallingBoxParams[4] = &bulletY;
        fallingBoxParams[5] = &collusion;
        fallingBoxParams[6] = &flying;
        
        DWORD dw2;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FallingBox, fallingBoxParams, 0, &dw2);
        
        bulletParams[0] = &bulletX;
        bulletParams[1] = &bulletY;
        bulletParams[2] = &thread_continue;
        bulletParams[3] = &fallingBoxX;
        bulletParams[4] = &fallingBoxY;
        bulletParams[5] = &slidingBoxX;
        bulletParams[6] = &slidingBoxY;
        //bulletParams[7] = &ateslendi_mi;
        bulletParams[7] = &collusion;

        DWORD dw3;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletMove, bulletParams, 0, &dw3);
        
        CreateImage(m, 400, 400, ICB_UINT);
        SetFocus(ICG_GetMainWindow());
    }
    else {
        thread_continue = false;

        if (FireMutex != NULL) {
            CloseHandle(FireMutex);
            FireMutex = NULL;
        }
    }
}

void WhenKeyPressed(int k) {
    keypressed = k;
}

void ICGUI_main() {
    ICG_Button(5, 5, 120, 25, "START / STOP", butonfonk);
    FRM1 = ICG_FrameMedium(5, 40, 400, 400);
    ICG_SetOnKeyPressed(WhenKeyPressed);
    CreateImage(m, 400, 400, ICB_UINT);
}