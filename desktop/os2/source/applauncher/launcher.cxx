#include "launcher.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <process.h>

int main( int argc, char* argv[])
{
    PPIB    pib;
    APIRET   rc;
    RESULTCODES result = {0};
    char     szFail[ _MAX_PATH];

    HAB hab = WinInitialize( 0);
    HMQ hmq = WinCreateMsgQueue( hab, 0);
    ERRORID    erridErrorCode = 0;
    erridErrorCode = WinGetLastError(hab);

    // Calculate application name
    CHAR    szApplicationName[_MAX_PATH];
    CHAR    szDrive[_MAX_PATH];
    CHAR    szDir[_MAX_PATH];
    CHAR    szFileName[_MAX_PATH];
    CHAR    szExt[_MAX_PATH];

    // get executable fullpath
    DosGetInfoBlocks(NULL, &pib);
    DosQueryModuleName(pib->pib_hmte, sizeof(szApplicationName), szApplicationName);
    _splitpath( szApplicationName, szDrive, szDir, szFileName, szExt );

    // adjust libpath
    _makepath( szApplicationName, szDrive, szDir, NULL, NULL);
    strcat( szApplicationName, ";%BeginLIBPATH%");
    DosSetExtLIBPATH( (PCSZ)szApplicationName, BEGIN_LIBPATH);

    // adjust exe name
    _makepath( szApplicationName, szDrive, szDir, OFFICE_IMAGE_NAME, (".bin") );

    // copy command line parameters
    int i, len;
    len = strlen(szApplicationName)+1;
    for( i=1; i<argc; i++)
        len += strlen( argv[i]) + 1;

    char* pszCommandLine, *pszArgs;
    pszCommandLine = (char*) malloc( len);
    strcpy( pszCommandLine, szApplicationName);
    pszArgs = pszCommandLine + strlen(szApplicationName) + 1;
    strcat( pszArgs, APPLICATION_SWITCH);
    strcat( pszArgs, " ");
    for( i=1; i<argc; i++) {
        // add quotes if argument has spaces!
        if (strchr( argv[i], ' '))
            strcat( pszArgs, "\"");
        strcat( pszArgs, argv[i]);
        if (strchr( argv[i], ' '))
            strcat( pszArgs, "\"");
        strcat( pszArgs, " ");
    }
    pszArgs[ strlen( pszArgs) + 0] = 0;

    // execute
    rc = DosExecPgm(szFail, sizeof(szFail),
                   EXEC_SYNC, (PCSZ)pszCommandLine, (PCSZ)NULL, &result,
                   (PCSZ)szApplicationName);
    if (rc) {
        char     szMessage[ _MAX_PATH*2];
        sprintf( szMessage, "Execution failed! Contact technical support.\n\nReturn code: %d\nFailing module:%s\n", rc, szFail);
        rc = WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                            (PSZ)szMessage,
                            (PSZ)"Unable to start OpenOffice.org!",
                            0, MB_ERROR | MB_OK);
        WinDestroyMsgQueue( hmq);
        WinTerminate( hab);
        exit(1);
    }

    WinDestroyMsgQueue( hmq);
    WinTerminate( hab);

    exit( result.codeResult);
}

