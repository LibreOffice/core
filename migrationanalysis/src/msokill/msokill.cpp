/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/*
 * Description: Put MSO in a state where it can be closed using
 *              automation or kill it completely
 */

#include "stdafx.h"
#include <stdio.h>


void KillOffice();
BOOL KillAppFromWindow(HWND hWnd, char *appName);
BOOL CloseActiveDialogs();
void printUsage();

//Callbacks used in closing
BOOL CALLBACK CloseOfficeDlgProc(HWND hwndChild, LPARAM lParam);
BOOL CALLBACK CountOfficeDlgProc(HWND hwndChild, LPARAM lParam);

//Global counters for number of windows found
int gWDDlgCount = 0;
int gXLDlgCount = 0;
int gPPDlgCount = 0;

//Dialog window class names for excel, powerpoint and word
//These are "Best guess" dialog names
const char *pWordDlg2k  = "bosa_sdm_Microsoft Word 9.0";
const char *pWordDlg2k3 = "bosa_sdm_Microsoft Office Word";
const char *pXLDlg2k    = "bosa_sdm_XL9";
const char *pPPDlg2k    = "#32770";
const char *pXLDlg2k3   = "bosa_sdm_XL9";
const char *pPPDlg2k3   = "#32770";
const char *pGenMSODlg  = "bosa_sdm_Mso96";
//consider adding - bosa_sdm_Mso96

//Command Line Argument constants
const char *ARG_HELP  = "--help";
const char *ARG_KILL  = "--kill";
const char *ARG_CLOSE = "--close";

//Window class names for MSO apps - if we need to look at other office instances
//then this list would need to be expanded
#define NUM_WINDOWCLASSNAMES 4
char *wndClassName[NUM_WINDOWCLASSNAMES] = {"OpusApp", "XLMAIN", "PP9FrameClass", "PP10FrameClass"};

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage();
        return 0;
    }

    if (strcmpi(argv[1], ARG_HELP) == 0) {
        printUsage();
        return 0;
    }

    if (strcmpi(argv[1], ARG_KILL) == 0) {
        KillOffice();
        return 0;
    }

    if (strcmpi(argv[1], ARG_CLOSE) == 0) {
        CloseActiveDialogs();
        return 0;
    }

    return 0;
}

/*--------------------------------------------------------------
  Find the MSO window if it is available and explicitly kill it
  MSO apps in this case are Excel, Word and PP
  Use FindWindow Win32 API to detect if they are available

  -------------------------------------------------------------*/
void KillOffice() {
    HWND hWnd;

    for (int i=0;i<NUM_WINDOWCLASSNAMES;i++) {
        int j = 0;
        while (((hWnd = FindWindow(wndClassName[i], NULL )) != NULL) && (j < 10)) {
            KillAppFromWindow(hWnd, wndClassName[i]);
            j++;
        }
    }
}

/*--------------------------------------------------------------
  Using window handle, get process handle and try to kill the
  app. This may not be successful if you do not have enough
  privileges to kill the app.

  --------------------------------------------------------------*/
BOOL KillAppFromWindow(
    HWND hWnd,
    char *
#ifdef _DEBUG
    appName
#endif
)
{
    BOOL bRet = TRUE;

    if(hWnd == NULL) {
        //The app doesn't appear to be running
#ifdef _DEBUG
        printf("App %s: window not found.\n,", appName);
#endif
        bRet = FALSE;
    } else {
        DWORD pid;  // Variable to hold the process ID.
        DWORD dThread;  // Variable to hold (unused) thread ID.
        dThread = GetWindowThreadProcessId(hWnd, &pid);
        HANDLE hProcess; // Handle to existing process

        hProcess = OpenProcess(SYNCHRONIZE | PROCESS_ALL_ACCESS, TRUE, pid);
        if (hProcess == NULL) {
#ifdef _DEBUG
            printf("App %s : Failed to get process handle",appName);
#endif
            bRet = FALSE;
        } else {
            if (!TerminateProcess(hProcess, 0)) {
                LPTSTR lpMsgBuf;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf, 0, NULL );
                printf("%s\n", lpMsgBuf);
                LocalFree( lpMsgBuf );
                bRet = FALSE;
            }
#ifdef _DEBUG
            else {
                printf("Kill %s appears to be successful.\n", appName);
            }
#endif
        }
    }
    return bRet;
}

/*--------------------------------------------------------------
  Close the dialogs if possible based on their window class
  Use the EnumChildWindows win32 api for this
  --------------------------------------------------------------*/
BOOL CloseActiveDialogs() {
    char buff[1024];

    gWDDlgCount = 0;
    gXLDlgCount = 0;
    gPPDlgCount = 0;

    EnumChildWindows(GetDesktopWindow(), CloseOfficeDlgProc, (LPARAM) 0);
    sprintf(buff, "Word: %d\tExcel: %d\tPP: %d", gWDDlgCount, gXLDlgCount, gPPDlgCount);
    return TRUE;
}

/*--------------------------------------------------------------
  Callback for EnumChildWindows that sends close message to
  any dialogs that match window class of MSO dialogs

  --------------------------------------------------------------*/
BOOL CALLBACK CloseOfficeDlgProc(HWND hwndChild, LPARAM)
{
    //bosa_sdm_Microsoft Word 9.0
    //bosa_sdm_XL9
    //#32770 (Dialog)

     char szBuff[4096];
    if (GetClassName(hwndChild, szBuff, 4096) == 0) {

    } else {
        if ((strcmpi(szBuff, pWordDlg2k) == 0) || (strcmpi(szBuff, pWordDlg2k3) == 0)) {
            gWDDlgCount++;
            SendMessage(hwndChild, WM_CLOSE, 0, 0);
        }
        if (strcmpi(szBuff, pXLDlg2k) == 0) {
            gXLDlgCount++;
            SendMessage(hwndChild, WM_CLOSE, 0, 0);
        }
        if (strcmpi(szBuff, pPPDlg2k) == 0) {
            gPPDlgCount++;
            SendMessage(hwndChild, WM_CLOSE, 0, 0);
        }
        if (strcmpi(szBuff, pGenMSODlg) == 0) {
            SendMessage(hwndChild, WM_CLOSE, 0, 0);
        }
    }

    return TRUE;
}


/*--------------------------------------------------------------
  Callback for EnumChildWindows that counts numnnber of
  dialogs that match window class of MSO dialogs

  --------------------------------------------------------------*/
BOOL CALLBACK CountOfficeDlgProc(HWND hwndChild, LPARAM)
{
     char szBuff[4096];
    if (GetClassName(hwndChild, szBuff, 4096) == 0) {

    } else {
        if ((strcmpi(szBuff, pWordDlg2k) == 0) || (strcmpi(szBuff, pWordDlg2k3) == 0)) {
            gWDDlgCount++;
        }
        if (strcmpi(szBuff, pXLDlg2k) == 0) {
            gXLDlgCount++;
        }
        if (strcmpi(szBuff, pPPDlg2k) == 0) {
            gPPDlgCount++;
        }
    }

    return TRUE;
}

/*--------------------------------------------------------------
  Simple usage message...

  -------------------------------------------------------------*/
void printUsage() {
    printf("Recovery Assistant Utility - try and put MSO apps in a recoverable state\n");
    printf("Copyright Sun Microsystems 2008\n");
    printf("Options:\n");
    printf("   --help : This message\n");
    printf("   --close: Attempt to close any open dialogs owned by \n");
    printf("            MSO apps so Application.Quit() can succeed\n");
    printf("   --kill : Kill any open MSO apps. Use with caution and only as a last resort\n\n");
}