#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <mapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <psapi.h>

#include <shellapi.h>

#define _UNICODE
#include <tchar.h>

#define _RICHEDIT_VER   0x0200
#include <richedit.h>


#if _RICHEDIT_VER >= 0x0200
#define RICHEDIT    TEXT("riched20.dll")
#else
#define RICHEDIT    TEXT("riched32.dll")
#endif

#include <systools/win32/uwinapi.h>

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <winsock.h>
#include <malloc.h>
#include <process.h>

#if _MSC_VER < 1300
#define COMEX "9"
#else
#define COMEX "8"
#endif

#ifdef PRODUCT
#define _INPATH "wntmsci" COMEX ".pro"
#else
#define _INPATH "wntmsci" COMEX
#endif

#include "resource.h"
#include "base64.h"

#define FORMATBUFSIZE   (8*1024)
#define MAX_TEXT_BUFFER (32*1024-1)
#define MAX_HOSTNAME    (1024)

#include <dbghelp.h>

using namespace ::std;


string  g_strDefaultLanguage;
FILE    *g_fpStackFile = NULL;
DWORD   g_dwExceptionCode = 0;

CHAR    g_szReportServerA[MAX_HOSTNAME] = "";
USHORT  g_uReportPort = 80;

TCHAR   g_szBuildId[256] = TEXT("");

TCHAR   g_szDumpFileName[MAX_PATH] = TEXT("");

CHAR    g_szDumpFileNameA[MAX_PATH] = "";
CHAR    g_szCommentFileNameA[MAX_PATH] = "";
CHAR    g_szReportFileNameA[MAX_PATH] = "";

#define REPORT_SERVER   g_szReportServerA
#define REPORT_PORT     g_uReportPort


//***************************************************************************
// tmpfile from msvcrt creates the temporary file in the root of the current
// volume and can fail.

static FILE *_tmpfile(void)
{
    FILE *fp = NULL;

    TCHAR   szTempPath[MAX_PATH];

    if ( GetTempPath( elementsof(szTempPath), szTempPath ) )
    {
        TCHAR   szFileName[MAX_PATH];

        if ( GetTempFileName( szTempPath, TEXT("CRT"), 0, szFileName ) )
        {
            HANDLE  hFile =  CreateFile(
                szFileName,
                GENERIC_READ | GENERIC_WRITE,
                0, NULL,
                OPEN_EXISTING,
                FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_NORMAL,
                NULL );

            if ( IsValidHandle( hFile ) )
            {
                int fd = _open_osfhandle( (int)hFile, 0 );

                fp = _fdopen( fd, "w+b" );
            }
        }
    }

    return fp;
}

//***************************************************************************

struct CrashReportParams
{
    BOOL                fAllowContact;
    TCHAR               szEmail[1024];
    TCHAR               szTitle[1024];
    TCHAR               szComment[2048];
    ULONG               uInternetConnection;
    TCHAR               szProxyServer[1024];
    TCHAR               szProxyPort[50];

    CrashReportParams();
    void WriteToRegistry();
    void ReadFromRegistry();
};

bool SendCrashReport( HWND hwndParent, const CrashReportParams &rParams );
BOOL WriteCommentFile( LPCTSTR lpComment );

//***************************************************************************

LONG RegReadValue( HKEY hBaseKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPVOID lpData, DWORD cbData )
{
    HKEY    hKey = NULL;
    LONG    lResult;

    lResult = RegOpenKeyEx( hBaseKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegQueryValueEx( hKey, lpValueName, NULL, NULL, (LPBYTE)lpData, &cbData );
        RegCloseKey( hKey );
    }

    return lResult;
}

//***************************************************************************

LONG RegWriteValue( HKEY hBaseKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData )
{
    HKEY    hKey = NULL;
    LONG    lResult;

    lResult = RegCreateKeyEx( hBaseKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegSetValueEx( hKey, lpValueName, NULL, dwType, (CONST BYTE *)lpData, cbData );
        RegCloseKey( hKey );
    }

    return lResult;
}

//***************************************************************************

CrashReportParams::CrashReportParams()
{
    fAllowContact = FALSE;
    _tcscpy( szTitle, TEXT("") );
    _tcscpy( szComment, TEXT("") );
    _tcscpy( szEmail, TEXT("") );
    uInternetConnection = 0;
    _tcscpy( szProxyServer, TEXT("") );
    _tcscpy( szProxyPort, TEXT("") );
}

//***************************************************************************

void CrashReportParams::ReadFromRegistry()
{
    RegReadValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPProxyServer"),
        szProxyServer,
        sizeof(szProxyServer) );

    DWORD   dwProxyPort;

    if ( ERROR_SUCCESS == RegReadValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPProxyPort"),
        &dwProxyPort,
        sizeof(dwProxyPort) ) )
        _stprintf( szProxyPort, TEXT("%d"), dwProxyPort );

    RegReadValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("ReturnAddress"),
        szEmail,
        sizeof(szEmail) );

    RegReadValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("AllowContact"),
        &fAllowContact,
        sizeof(fAllowContact) );

    RegReadValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPConnection"),
        &uInternetConnection,
        sizeof(uInternetConnection) );

}

//***************************************************************************

void CrashReportParams::WriteToRegistry()
{
    RegWriteValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPProxyServer"), REG_SZ,
        szProxyServer,
        sizeof(TCHAR) * (_tcslen( szProxyServer ) + 1) );

    LPTSTR endptr = NULL;
    DWORD dwProxyPort = _tcstoul( szProxyPort, &endptr, 10 );

    RegWriteValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPProxyPort"), REG_DWORD,
        &dwProxyPort,
        sizeof(DWORD) );

    RegWriteValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("AllowContact"), REG_DWORD,
        &fAllowContact,
        sizeof(DWORD) );


    RegWriteValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("HTTPConnection"), REG_DWORD,
        &uInternetConnection,
        sizeof(DWORD) );

    RegWriteValue(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
        TEXT("ReturnAddress"), REG_SZ,
        szEmail,
        sizeof(TCHAR) * (_tcslen( szEmail ) + 1) );
}

//***************************************************************************

typedef BOOL (WINAPI *MiniDumpWriteDump_PROC)(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
    );

//***************************************************************************

static BOOL WINAPI InitRichEdit()
{
    return (NULL != LoadLibrary( RICHEDIT ));
}

//***************************************************************************

static BOOL WINAPI DeinitRichEdit()
{
    return FreeLibrary( GetModuleHandle( RICHEDIT ) );
}

//***************************************************************************

static string trim_string( const string& rString )
{
    string temp = rString;

    while ( temp.length() && temp[0] == ' ' || temp[0] == '\t' )
        temp.erase( 0, 1 );

    string::size_type   len = temp.length();

    while ( len && temp[len-1] == ' ' || temp[len-1] == '\t' )
    {
        temp.erase( len - 1, 1 );
        len = temp.length();
    }

    return temp;
}

//***************************************************************************

static int LoadAndFormatString( HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax )
{
    TCHAR   szBuffer[FORMATBUFSIZE];
    TCHAR   szBuffer2[FORMATBUFSIZE];

    LoadString( hInstance, uID, szBuffer, elementsof(szBuffer) );

    LPCTSTR src;
    LPTSTR  dest;
    for ( dest = szBuffer2, src = szBuffer; *src; src++, dest++ )
    {
        switch ( *src )
        {
        case '~':
            *dest = '&';
            break;
        case '\\':
            switch ( *(++src) )
            {
            case 'n':
                *dest = '\n';
                break;
            case 'r':
                *dest = '\r';
                break;
            default:
                *dest = *src;
                break;
            }
            break;
        default:
            *dest = *src;
            break;
        }
    }

    *dest = *src;

    return ExpandEnvironmentStrings( szBuffer2, lpBuffer, nBufferMax );
}


//***************************************************************************

static string xml_encode( const string &rString )
{
    string temp = rString;
    string::size_type pos = 0;

    // First replace all occurences of '&' because it may occur in further
    // encoded chardters too

    for( pos = 0; (pos = temp.find( '&', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&amp;" );

    for( pos = 0; (pos = temp.find( '<', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&lt;" );

    for( pos = 0; (pos = temp.find( '>', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&gt;" );

    return temp;
}

//***************************************************************************

static size_t fcopy( FILE *fpin, FILE *fpout )
{
    char buffer[1024];
    size_t nBytes;
    size_t nBytesWritten = 0;

    if ( fpin && fpout )
    {
        while ( 0 != (nBytes = fread( buffer, 1, sizeof(buffer), fpin )) )
        {
            nBytesWritten += fwrite( buffer, 1, nBytes, fpout );
        }
    }

    return nBytesWritten;
}

//***************************************************************************

static string GetModuleDirectory( HMODULE hModule )
{
    TCHAR   szBuffer[256] = TEXT("");
    TCHAR   szModuleName[MAX_PATH] = TEXT("");
    TCHAR   szDrive[_MAX_DRIVE];
    TCHAR   szDir[_MAX_DIR];
    TCHAR   szFName[_MAX_FNAME];
    TCHAR   szExt[_MAX_EXT];

    if ( GetModuleFileName( hModule, szModuleName, MAX_PATH ) )
    {
        _tsplitpath( szModuleName, szDrive, szDir, szFName, szExt );
        _tmakepath( szModuleName, szDrive, szDir, _T(""), _T("") );
    }

    CHAR    szModuleNameUTF8[MAX_PATH] = "";

    WideCharToMultiByte( CP_UTF8, 0, szModuleName, -1, szModuleNameUTF8, elementsof(szModuleNameUTF8), NULL, NULL );
    return string( szModuleNameUTF8 );
}

//***************************************************************************

string GetFileDirectory( const string& rFilePath )
{
    string aDir = rFilePath;
    size_t pos = aDir.rfind( '\\' );

    if ( string::npos != pos )
        aDir.erase( pos + 1 );
    else
        aDir = "";

    return aDir;
}

//***************************************************************************

string GetFileName( const string& rFilePath )
{
    string aName = rFilePath;
    size_t pos = aName.rfind( '\\' );

    if ( string::npos != pos )
        return aName.substr( pos + 1 );
    else
        return aName;
}

//***************************************************************************

BOOL WriteReportFile( CrashReportParams *pParams )
{
    BOOL    fSuccess = FALSE;
    TCHAR   szTempPath[MAX_PATH];

    if ( GetTempPath( elementsof(szTempPath), szTempPath ) )
    {
        TCHAR   szFileName[MAX_PATH];

        if ( GetTempFileName( szTempPath, TEXT("RPM"), 0, szFileName ) )
        {
            HANDLE  hFile =  CreateFile( szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hFile )
            {
                int fd = _open_osfhandle( (LONG)hFile, _O_TEXT );
                FILE    *fp = _fdopen( fd, "w+t" );
                CHAR    szTitle[1024] = "";
                CHAR    szBuildId[1024] = "";
                CHAR    szEmail[1024] = "";
                CHAR    szUserName[256] = "";
                TCHAR   tszUserName[256] = TEXT("");
                DWORD   dwUserNameSize = elementsof(tszUserName);
                const char *pszUserType = getenv( "STAROFFICE_USERTYPE" );
                const char *pszProductName = getenv( "PRODUCTNAME" );

                GetUserName( tszUserName, &dwUserNameSize );

                WideCharToMultiByte( CP_UTF8, 0, pParams->szTitle, -1, szTitle, sizeof(szTitle), NULL, NULL );
                WideCharToMultiByte( CP_UTF8, 0, g_szBuildId, -1, szBuildId, sizeof(szBuildId), NULL, NULL );
                WideCharToMultiByte( CP_UTF8, 0, pParams->szEmail, -1, szEmail, sizeof(szEmail), NULL, NULL );
                WideCharToMultiByte( CP_UTF8, 0, tszUserName, -1, szUserName, sizeof(szUserName), NULL, NULL );

                fprintf( fp,
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<!DOCTYPE errormail:errormail PUBLIC \"-//OpenOffice.org//DTD ErrorMail 1.0//EN\" \"errormail.dtd\">\n"
                    "<errormail:errormail xmlns:errormail=\"http://openoffice.org/2002/errormail\" uid=\"%s\" usertype=\"%s\">\n"
                    "<reportmail:mail xmlns:reportmail=\"http://openoffice.org/2002/reportmail\" version=\"1.0\" feedback=\"%s\" email=\"%s\">\n",
                    szUserName,
                    pszUserType ? pszUserType : "",
                    pParams->fAllowContact ? "true" : "false",
                    pParams->fAllowContact ? xml_encode(szEmail).c_str() : ""
                    );

                fprintf( fp,
                    "<reportmail:title>%s</reportmail:title>\n",
                    xml_encode(szTitle).c_str() );

                fprintf( fp,
                    "<reportmail:attachment name=\"description.txt\" media-type=\"text/plain;charset=UTF-8\" class=\"UserComment\"/>\n"
                    "<reportmail:attachment name=\"user.dmp\" media-type=\"application/octet-stream\" class=\"UserDump\"/>\n"
                    "</reportmail:mail>\n"

                    "<officeinfo:officeinfo xmlns:officeinfo=\"http://openoffice.org/2002/officeinfo\" build=\"%s\" platform=\"%s\" language=\"%s\" procpath=\"%s\" exceptiontype=\"0x%08X\" product=\"%s\"/>\n",
                    szBuildId,
                    _INPATH,
                    g_strDefaultLanguage.c_str(),
                    GetModuleDirectory( NULL ).c_str(),
                    g_dwExceptionCode,
                    pszProductName ? pszProductName : ""
                    );

                OSVERSIONINFO   VersionInfo;

                ZeroMemory( &VersionInfo, sizeof(VersionInfo) );
                VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo );

                GetVersionEx( &VersionInfo );

                fprintf( fp,
                    "<systeminfo:systeminfo xmlns:systeminfo=\"http://openoffice.org/2002/systeminfo\">\n"
                    "<systeminfo:System name=\"%s\" version=\"%d.%d\" build=\"%d\" locale=\"0x%08x\"/>\n"
                    ,
                    VER_PLATFORM_WIN32_NT == VersionInfo.dwPlatformId ? "Windows NT" : "Windows",
                    VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion,
                    VersionInfo.dwBuildNumber,
                    GetUserDefaultLangID()

                    );
                fprintf( fp, "<systeminfo:CPU type=\"x86\"/>\n" );
                fprintf( fp, "</systeminfo:systeminfo>\n" );

                fseek( g_fpStackFile, 0, SEEK_SET );
                fcopy( g_fpStackFile, fp );

                fprintf( fp, "</errormail:errormail>\n" );

                fclose( fp );

                fSuccess = TRUE;

                WideCharToMultiByte( CP_ACP, 0, szFileName, -1, g_szReportFileNameA, MAX_PATH, NULL, NULL );
            }

            if ( !fSuccess )
                DeleteFile( szFileName );
        }
    }

    return fSuccess;
}

//***************************************************************************

static BOOL SaveDumpFile( HWND hwndOwner )
{
    OPENFILENAME    ofn;
    TCHAR   szFileName[MAX_PATH] = TEXT("");

    ZeroMemory( &ofn, sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);

    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFilter = TEXT("*.dmp\0*.dmp\0*.*\0*.*\0");
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_ENABLESIZING | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = TEXT("dmp");

    if ( GetSaveFileName( &ofn ) )
    {
        return CopyFile( g_szDumpFileName, szFileName, FALSE );
    }


    return FALSE;
}

//***************************************************************************

static BOOL ScreenToClientRect( HWND hwnd, LPRECT lprc )
{
    return ScreenToClient( hwnd, (LPPOINT)&lprc->left ) && ScreenToClient( hwnd, (LPPOINT)&lprc->right );
}

static BOOL SetWindowRect( HWND hwnd, const RECT *lprc, BOOL fRepaint )
{
    return MoveWindow( hwnd, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, fRepaint );
}

#define GM_LOX  0x01
#define GM_HIX  0x02
#define GM_LOY  0x04
#define GM_HIY  0x08

static BOOL SetGrowMode( HWND hwnd, DWORD dwGrowMode )
{
    return SetProp( hwnd, TEXT("GrowMode"), (HANDLE)dwGrowMode );
}

static DWORD GetGrowMode( HWND hwnd )
{
    return (DWORD)GetProp( hwnd, TEXT("GrowMode") );
}

static BOOL GrowWindow( HWND hwnd, LONG dxClient, LONG dyClient, BOOL fRepaint )
{
    DWORD   dwGrowMode = GetGrowMode( hwnd );
    RECT    rc;

    GetWindowRect( hwnd, &rc );

    if ( dwGrowMode & GM_LOX )
        rc.left += dxClient;
    if ( dwGrowMode & GM_HIX )
        rc.right += dxClient;
    if ( dwGrowMode & GM_LOY )
        rc.top += dyClient;
    if ( dwGrowMode & GM_HIY )
        rc.bottom += dyClient;

    ScreenToClientRect( GetParent( hwnd ), &rc );
    SetWindowRect( hwnd, &rc, fRepaint );

    return TRUE;
}

BOOL CALLBACK GrowChildWindows(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    LONG    cx = (SHORT)LOWORD( lParam );
    LONG    cy = (SHORT)HIWORD( lParam );

    GrowWindow( hwnd, cx, cy, TRUE );

    return TRUE;
}

/*
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
    HFONT aFont = *((HFONT*) lParam);
    HDC hDC = GetDC( hwndChild );
    SelectObject( hDC, aFont );
    ReleaseDC( hwndChild, hDC );
    return TRUE;
}

void ApplySystemFont( HWND hwndDlg )
{
    NONCLIENTMETRICSA aNonClientMetrics;
    aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
    if ( SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
    {
        HFONT aSysFont = CreateFontIndirectA( &aNonClientMetrics.lfMessageFont );
        EnumChildWindows(hwndDlg, EnumChildProc, (LPARAM) &aSysFont);
    }
}
*/

BOOL CALLBACK PreviewDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static RECT rcClient;

    switch ( uMsg )
    {
    case WM_SIZE:
        {
        LONG    cx = LOWORD( lParam );
        LONG    cy = HIWORD( lParam );
        LONG    dxClient, dyClient;

        dxClient = cx - rcClient.right;
        dyClient = cy - rcClient.bottom;

        EnumChildWindows( hwndDlg, GrowChildWindows, MAKELONG( (SHORT)dxClient, (SHORT)dyClient) );

        GetClientRect( hwndDlg, &rcClient );
        }
        break;
    case WM_INITDIALOG:
        {
            GetClientRect( hwndDlg, &rcClient );
            SetGrowMode( GetDlgItem(hwndDlg, IDC_EDIT_PREVIEW), GM_HIX | GM_HIY );
            SetGrowMode( GetDlgItem(hwndDlg, IDOK), GM_LOX | GM_HIX | GM_LOY | GM_HIY );

            CrashReportParams *pParams = (CrashReportParams *)lParam;

            TCHAR   szBuffer[256] = TEXT("");
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong( hwndDlg, GWL_HINSTANCE );
            HWND    hwndParent = (HWND)GetWindowLong( hwndDlg, GWL_HWNDPARENT );

            GetWindowText( hwndParent, szBuffer, elementsof(szBuffer) );
            SetWindowText( hwndDlg, szBuffer );

            LoadAndFormatString( hInstance, IDS_OK_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDOK), szBuffer );

            basic_string<TCHAR> aString;

            aString.append( pParams->szTitle );
            aString.append( _T("\r\n\r\n") );
            aString.append( pParams->szComment );
            aString.append( _T("\r\n---------- report ----------\r\n") );

            FILE    *fp = fopen( g_szReportFileNameA, "r" );

            if ( fp )
            {
                char    buf[1024];

                while ( fgets( buf, elementsof(buf), fp ) != NULL )
                {
                    WCHAR   bufW[1024];

                    MultiByteToWideChar( CP_UTF8, 0, buf, -1, bufW, elementsof(bufW) );

                    aString.append( bufW );
                }

                fclose( fp );
            }

            aString.append( _T("\r\n---------- stack ----------\r\n") );

            fp = fopen( g_szDumpFileNameA, "rb" );

            if ( fp )
            {
                unsigned char   buf[16];
                int     count;

                do
                {
                    int i;

                    count = fread( buf, sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]), fp );

                    for ( i = 0; i < count; i++ )
                    {
                        TCHAR   output[16];

                        _sntprintf( output, elementsof(output), _T("%02X\x20"), buf[i] );
                        aString.append( output );
                    }
                    for ( ; i < elementsof(buf); i++ )
                    {
                        aString.append( _T("\x20\x20\x20") );
                    }

                    for ( i = 0; i < count; i++ )
                    {
                        TCHAR   output[2];

                        if ( (int)buf[i] >= 0x20 && (int)buf[i] <= 0x7F )
                            output[0] = (int)buf[i];
                        else
                            output[0] = '.';
                        output[1] = 0;
                        aString.append( output );
                    }

                    aString.append( _T("\r\n") );

                } while ( count );

                fclose( fp );
            }

            Edit_SetText( GetDlgItem(hwndDlg, IDC_EDIT_PREVIEW), aString.c_str() );


            SetWindowFont( GetDlgItem(hwndDlg, IDC_EDIT_PREVIEW), GetStockObject( SYSTEM_FIXED_FONT ), TRUE );
        }
        return TRUE;
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDOK:
        case IDCANCEL:
            EndDialog( hwndDlg, wParam );
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}
//***************************************************************************

static void PreviewReport( HWND hwndParent, CrashReportParams *pParams )
{
    HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE );

    WriteReportFile( pParams );

    DialogBoxParam(
        hInstance,
        MAKEINTRESOURCE(IDD_PREVIEW_FRAME),
        hwndParent,
        PreviewDialogProc,
        (LPARAM)pParams
        );

    DeleteFileA( g_szReportFileNameA );
}
//***************************************************************************
void UpdateOptionsDialogControls( HWND hwndDlg )
{
    if ( Button_GetCheck( GetDlgItem(hwndDlg, IDC_RADIO_MANUAL) ) & BST_CHECKED )
    {
        EnableWindow( GetDlgItem(hwndDlg, IDC_EDIT_PROXYSERVER), TRUE );
        EnableWindow( GetDlgItem(hwndDlg, IDC_EDIT_PROXYPORT), TRUE );
    }
    else
    {
        EnableWindow( GetDlgItem(hwndDlg, IDC_EDIT_PROXYSERVER), FALSE );
        EnableWindow( GetDlgItem(hwndDlg, IDC_EDIT_PROXYPORT), FALSE );
    }
}

//***************************************************************************

BOOL CALLBACK OptionsDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static CrashReportParams *pParams;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            TCHAR   szBuffer[1024] = TEXT("");
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong( hwndDlg, GWL_HINSTANCE );
            HWND    hwndParent = (HWND)GetWindowLong( hwndDlg, GWL_HWNDPARENT );

            pParams = (CrashReportParams *)lParam;

            LoadAndFormatString( hInstance, IDS_OPTIONS_CAPTION, szBuffer, elementsof(szBuffer) );
            SetWindowText( hwndDlg, szBuffer );

            LoadAndFormatString( hInstance, IDS_PROXY_SETTINGS_HEADER, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_PROXY_SETTINGS), szBuffer );

            LoadAndFormatString( hInstance, IDS_PROXY_SYSTEM, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_RADIO_SYSTEM), szBuffer );

            LoadAndFormatString( hInstance, IDS_PROXY_DIRECT, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_RADIO_DIRECT), szBuffer );

            LoadAndFormatString( hInstance, IDS_PROXY_MANUAL, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_RADIO_MANUAL), szBuffer );

            LoadAndFormatString( hInstance, IDS_LABEL_PROXYSERVER, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_LABEL_PROXYSERVER), szBuffer );

            LoadAndFormatString( hInstance, IDS_LABEL_PROXYPORT, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_LABEL_PROXYPORT), szBuffer );

            LoadAndFormatString( hInstance, IDS_OK_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDOK), szBuffer );

            LoadAndFormatString( hInstance, IDS_CANCEL_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDCANCEL), szBuffer );

            Edit_SetText( GetDlgItem(hwndDlg, IDC_EDIT_PROXYSERVER), pParams->szProxyServer );
            Edit_SetText( GetDlgItem(hwndDlg, IDC_EDIT_PROXYPORT), pParams->szProxyPort );

            Button_SetCheck( GetDlgItem(hwndDlg, IDC_RADIO_SYSTEM + pParams->uInternetConnection), BST_CHECKED );

            SendMessage(
                GetDlgItem(hwndDlg, IDC_PROXY_DESCRIPTION),
                EM_SETBKGNDCOLOR,
                (WPARAM)FALSE,
                GetSysColor( COLOR_3DFACE ) );
            LoadAndFormatString( hInstance, IDS_PROXY_DESCRIPTION, szBuffer, elementsof(szBuffer) );
            Edit_SetText( GetDlgItem(hwndDlg, IDC_PROXY_DESCRIPTION), szBuffer );

            UpdateOptionsDialogControls( hwndDlg );
        }
        return TRUE;
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_RADIO_SYSTEM:
        case IDC_RADIO_DIRECT:
        case IDC_RADIO_MANUAL:
            if ( BN_CLICKED == HIWORD(wParam) )
                UpdateOptionsDialogControls( hwndDlg );
            break;
        case IDOK:
            {
            Edit_GetText( GetDlgItem(hwndDlg, IDC_EDIT_PROXYSERVER), pParams->szProxyServer, elementsof(pParams->szProxyServer) );
            Edit_GetText( GetDlgItem(hwndDlg, IDC_EDIT_PROXYPORT), pParams->szProxyPort, elementsof(pParams->szProxyPort) );
            if ( Button_GetCheck( GetDlgItem(hwndDlg, IDC_RADIO_DIRECT) ) & BST_CHECKED )
                pParams->uInternetConnection = 1;
            else if ( Button_GetCheck( GetDlgItem(hwndDlg, IDC_RADIO_MANUAL) ) & BST_CHECKED )
                pParams->uInternetConnection = 2;
            else
                pParams->uInternetConnection = 0;
            }
        case IDCANCEL:
            EndDialog( hwndDlg, wParam );
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}

//***************************************************************************

static void OptionsDialog( HWND hwndParent, CrashReportParams *pParams )
{
    HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE );

    if ( IDOK == DialogBoxParam(
        hInstance,
        MAKEINTRESOURCE(IDD_OPTIONS_FRAME),
        hwndParent,
        OptionsDialogProc,
        (LPARAM)pParams
        ) )
        pParams->WriteToRegistry();

}
//***************************************************************************

void UpdateReportDialogControls( HWND hwndDlg )
{
    EnableWindow(
        GetDlgItem(hwndDlg, IDC_EDIT_EMAIL),
        Button_GetCheck(GetDlgItem(hwndDlg, IDC_ALLOW_CONTACT)) & BST_CHECKED ? TRUE : FALSE );
    EnableWindow(
        GetDlgItem(hwndDlg, IDC_LABEL_EMAIL),
        Button_GetCheck(GetDlgItem(hwndDlg, IDC_ALLOW_CONTACT)) & BST_CHECKED ? TRUE : FALSE );
}

//***************************************************************************

BOOL CALLBACK ReportDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            CrashReportParams   *pParams = (CrashReportParams*)GetWindowLong( GetParent(hwndDlg), GWL_USERDATA );
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE );
            TCHAR       szBuffer[FORMATBUFSIZE];

            LoadAndFormatString( hInstance, IDS_REPORT_INTRO, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_REPORT_INTRO), szBuffer );

            Edit_SetText( GetDlgItem(hwndDlg, IDC_EDIT3), szBuffer );

            LoadAndFormatString( hInstance, IDS_ENTER_TITLE, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_ENTER_TITLE), szBuffer );

            LoadAndFormatString( hInstance, IDS_ENTER_DESCRIPTION, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_ENTER_DESCRIPTION), szBuffer );

            LoadAndFormatString( hInstance, IDS_SHOW_REPORT_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_SHOW_REPORT), szBuffer );

            LoadAndFormatString( hInstance, IDS_SAVE_REPORT_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_SAVE_REPORT), szBuffer );

            const char *pszUserType = getenv( "STAROFFICE_USERTYPE" );
            if ( pszUserType )
                ShowWindow( GetDlgItem(hwndDlg, IDC_SAVE_REPORT), SW_SHOW );
            else
                ShowWindow( GetDlgItem(hwndDlg, IDC_SAVE_REPORT), SW_HIDE );

            LoadAndFormatString( hInstance, IDS_OPTIONS_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_OPTIONS), szBuffer );

            LoadAndFormatString( hInstance, IDS_ALLOW_CONTACT, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_ALLOW_CONTACT), szBuffer );
            Button_SetCheck( GetDlgItem(hwndDlg, IDC_ALLOW_CONTACT), pParams->fAllowContact ? BST_CHECKED : BST_UNCHECKED );

            LoadAndFormatString( hInstance, IDS_LABEL_EMAIL, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDC_LABEL_EMAIL), szBuffer );

            Edit_SetText( GetDlgItem(hwndDlg, IDC_EDIT_EMAIL), pParams->szEmail );

            UpdateReportDialogControls( hwndDlg );
        }
        return TRUE;
    case WM_SHOWWINDOW:
        if ( (BOOL)wParam )
        {
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE );
            TCHAR       szBuffer[FORMATBUFSIZE];

            LoadAndFormatString( hInstance, IDS_REPORT_CAPTION, szBuffer, elementsof(szBuffer) );
            SetWindowText( GetParent(hwndDlg), szBuffer );

            LoadAndFormatString( hInstance, IDS_REPORT_HEADER, szBuffer, elementsof(szBuffer) );
            SetWindowText( GetDlgItem(GetParent(hwndDlg), IDC_HEADER), szBuffer );

            LoadAndFormatString( hInstance, IDS_DONOT_SEND_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(GetParent(hwndDlg), IDCANCEL), szBuffer );


            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDBACK), TRUE );
            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDFINISH), TRUE );
            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDNEXT), FALSE );

            /*
            SetWindowLong( GetDlgItem(GetParent(hwndDlg),IDFINISH), GWL_STYLE,
                GetWindowLong( GetDlgItem(GetParent(hwndDlg),IDFINISH), GWL_STYLE) | BS_DEFPUSHBUTTON );
            SetWindowLong( GetDlgItem(GetParent(hwndDlg),IDBACK), GWL_STYLE,
                GetWindowLong( GetDlgItem(GetParent(hwndDlg),IDBACK), GWL_STYLE) &~ BS_DEFPUSHBUTTON );
                */
            SetFocus( GetDlgItem(hwndDlg,IDC_EDIT_TITLE) );
        }
        break;
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_SHOW_REPORT:
            {
                CrashReportParams   *pParams = (CrashReportParams*)GetWindowLong( GetParent(hwndDlg), GWL_USERDATA );

                pParams->fAllowContact = Button_GetCheck( GetDlgItem(hwndDlg, IDC_ALLOW_CONTACT) ) ? TRUE : FALSE;
                Edit_GetText( GetDlgItem(hwndDlg, IDC_EDIT_TITLE), pParams->szTitle, elementsof(pParams->szTitle) );
                Edit_GetText( GetDlgItem(hwndDlg, IDC_EDIT_DESCRIPTION), pParams->szComment, elementsof(pParams->szComment) );
                Edit_GetText( GetDlgItem(hwndDlg, IDC_EDIT_EMAIL), pParams->szEmail, elementsof(pParams->szEmail) );
                PreviewReport( GetParent(hwndDlg), pParams );
            }
            return TRUE;
        case IDC_SAVE_REPORT:
            SaveDumpFile( GetParent(hwndDlg) );
            return TRUE;
        case IDC_OPTIONS:
            {
                CrashReportParams   *pParams = (CrashReportParams*)GetWindowLong( GetParent(hwndDlg), GWL_USERDATA );
                OptionsDialog( GetParent(hwndDlg), pParams );
            }
            return TRUE;
        case IDC_ALLOW_CONTACT:
            if ( BN_CLICKED == HIWORD(wParam) )
                UpdateReportDialogControls( hwndDlg );
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}
//***************************************************************************

BOOL CALLBACK WelcomeDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE );
            HWND    hwndRichEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT21);
            TCHAR   szBuffer[FORMATBUFSIZE];
            TCHAR   szBuffer2[FORMATBUFSIZE];
            TCHAR   szURL[256];
            TCHAR   szCaption[256];

            SendMessage(
                hwndRichEdit,
                EM_SETBKGNDCOLOR,
                (WPARAM)FALSE,
                GetSysColor( COLOR_3DFACE ) );

            SendMessage( hwndRichEdit, EM_SETEVENTMASK, 0, ENM_LINK );
            SendMessage( hwndRichEdit, EM_AUTOURLDETECT, TRUE, 0 );

            LoadAndFormatString( hInstance, IDS_WELCOME_BODY1, szBuffer, elementsof(szBuffer) );
            LoadAndFormatString( hInstance, IDS_WELCOME_BODY2, szBuffer2, elementsof(szBuffer2) );
            _tcsncat( szBuffer, szBuffer2, elementsof(szBuffer) );
            LoadAndFormatString( hInstance, IDS_WELCOME_BODY3, szBuffer2, elementsof(szBuffer2) );
            _tcsncat( szBuffer, szBuffer2, elementsof(szBuffer) );
            LoadString( hInstance, IDS_PRIVACY_URL, szURL, elementsof(szURL) );
            _tcsncat( szBuffer, szURL, elementsof(szBuffer) );
            SetWindowText( hwndRichEdit, szBuffer );

            LoadAndFormatString( hInstance, IDS_WELCOME_CAPTION, szCaption, elementsof(szCaption) );
            SetWindowText( GetParent(hwndDlg), szCaption );

        }
        return TRUE;
    case WM_SHOWWINDOW:
        if ( (BOOL)wParam )
        {
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE );
            TCHAR       szBuffer[FORMATBUFSIZE];

            LoadAndFormatString( hInstance, IDS_WELCOME_CAPTION, szBuffer, elementsof(szBuffer) );
            SetWindowText( GetParent(hwndDlg), szBuffer );

            LoadAndFormatString( hInstance, IDS_WELCOME_HEADER, szBuffer, elementsof(szBuffer) );
            SetWindowText( GetDlgItem(GetParent(hwndDlg), IDC_HEADER), szBuffer );

            LoadAndFormatString( hInstance, IDS_CANCEL_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(GetParent(hwndDlg), IDCANCEL), szBuffer );

            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDBACK), FALSE );
            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDFINISH), FALSE );
            ShowWindow( GetDlgItem(GetParent(hwndDlg),IDNEXT), TRUE );

            SetFocus( GetDlgItem(GetParent(hwndDlg),IDNEXT) );
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            if ( pnmh->idFrom == IDC_RICHEDIT21 && pnmh->code == EN_LINK )
            {
                ENLINK  *plink = (ENLINK*)lParam;

                if ( plink->msg == WM_LBUTTONUP )
                {
                    TCHAR   szBuffer[256];
                    TEXTRANGE   range;

                    range.chrg = plink->chrg;
                    range.lpstrText = szBuffer;

                    SendMessage( pnmh->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&range );

                    ShellExecute( hwndDlg, NULL, szBuffer, NULL, NULL, SW_SHOWDEFAULT );
                }

            }
        }
        break;
    default:
        break;
    }

    return FALSE;
}
//***************************************************************************

BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static  HWND    hwndPages[2] = { NULL };
    static  int     iActualPage = 0;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE );
            TCHAR       szBuffer[FORMATBUFSIZE];

            SetWindowLong( hwndDlg, GWL_USERDATA, (LONG)lParam );
            hwndPages[0] = CreateDialog(
                hInstance,
                MAKEINTRESOURCE(IDD_WELCOME_PAGE),
                hwndDlg,
                WelcomeDialogProc );

            hwndPages[1] = CreateDialog(
                hInstance,
                MAKEINTRESOURCE(IDD_REPORT_PAGE),
                hwndDlg,
                ReportDialogProc );

            CHARFORMAT  chfmt;

            chfmt.cbSize = sizeof(chfmt);
            chfmt.dwMask = CFM_BOLD;
            chfmt.dwEffects = CFE_BOLD;

            SendMessage(
                GetDlgItem(hwndDlg, IDC_HEADER),
                EM_SETCHARFORMAT,
                SCF_ALL,
                (LPARAM)&chfmt );

            LoadAndFormatString( hInstance, IDS_CANCEL_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDCANCEL), szBuffer );

            LoadAndFormatString( hInstance, IDS_NEXT_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDNEXT), szBuffer );

            LoadAndFormatString( hInstance, IDS_SEND_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDFINISH), szBuffer );

            LoadAndFormatString( hInstance, IDS_BACK_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDBACK), szBuffer );

            ShowWindow( hwndPages[1], SW_HIDE );
            ShowWindow( hwndPages[0], SW_SHOW );
        }
        return FALSE;
    case WM_CTLCOLORSTATIC:
        return (BOOL)CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDBACK:
            if ( iActualPage > 0 )
            {
                ShowWindow( hwndPages[iActualPage], SW_HIDE );
                ShowWindow( hwndPages[--iActualPage], SW_SHOW );
            }
            return TRUE;
        case IDNEXT:
            if ( iActualPage < elementsof(hwndPages) - 1 )
            {
                ShowWindow( hwndPages[iActualPage], SW_HIDE );
                ShowWindow( hwndPages[++iActualPage], SW_SHOW );
            }
            return TRUE;
        case IDFINISH:
            {
                CrashReportParams   *pParams = (CrashReportParams*)GetWindowLong( hwndDlg, GWL_USERDATA );

                pParams->fAllowContact = Button_GetCheck( GetDlgItem(hwndPages[1], IDC_ALLOW_CONTACT) ) ? TRUE : FALSE;
                Edit_GetText( GetDlgItem(hwndPages[1], IDC_EDIT_TITLE), pParams->szTitle, elementsof(pParams->szTitle) );
                Edit_GetText( GetDlgItem(hwndPages[1], IDC_EDIT_DESCRIPTION), pParams->szComment, elementsof(pParams->szComment) );
                Edit_GetText( GetDlgItem(hwndPages[1], IDC_EDIT_EMAIL), pParams->szEmail, elementsof(pParams->szEmail) );

                if ( pParams->fAllowContact && !pParams->szEmail[0] )
                {
                    TCHAR   szMessage[MAX_TEXT_BUFFER];

                    LoadAndFormatString( GetModuleHandle(NULL), IDS_ERROR_MSG_NOEMAILADDRESS, szMessage, elementsof(szMessage) );

                    MessageBox( hwndDlg, szMessage, NULL, MB_ICONERROR | MB_OK );
                    break;  // Don't end the dialog
                }
                else
                {
                    pParams->WriteToRegistry();

                    WriteCommentFile( pParams->szComment );
                    WriteReportFile( pParams );

                    if ( !SendCrashReport( hwndDlg, *pParams ) )
                        break; // Don't end the dialog
                }
            }
            // Fallthrough !!!
        case IDCANCEL:
            EndDialog( hwndDlg, wParam );
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}



//***************************************************************************

static bool WriteStackFile( FILE *fout, DWORD dwProcessId, PEXCEPTION_POINTERS pExceptionPointers )
{
    bool    fSuccess = false;

    if ( fout && dwProcessId && pExceptionPointers )
    {
        HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );

        if ( IsValidHandle(hProcess) )
        {
            EXCEPTION_POINTERS  aExceptionPointers;
            CONTEXT             aContextRecord;

            ReadProcessMemory(
                hProcess,
                pExceptionPointers,
                &aExceptionPointers,
                sizeof(aExceptionPointers),
                NULL );

            ReadProcessMemory(
                hProcess,
                aExceptionPointers.ContextRecord,
                &aContextRecord,
                sizeof(aContextRecord),
                NULL );

            STACKFRAME  frame;

            ZeroMemory( &frame, sizeof(frame) );
            frame.AddrPC.Offset = aContextRecord.Eip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = aContextRecord.Ebp;
            frame.AddrFrame.Mode = AddrModeFlat;

            BOOL bSuccess;
            int frameNum = 0;

            SymInitialize( hProcess, NULL, TRUE );

            fprintf( fout, "<errormail:Stack type=\"Win32\">\n" );

            do
            {
                fSuccess = true;

                bSuccess = StackWalk( IMAGE_FILE_MACHINE_I386,
                    hProcess,
                    NULL,
                    &frame,
                    &aContextRecord,
                    (PREAD_PROCESS_MEMORY_ROUTINE)ReadProcessMemory,
                    SymFunctionTableAccess,
                    SymGetModuleBase,
                    NULL );

                if ( bSuccess )
                {
                    // Note: ImageHelp ANSI functions do not have an A postfix while
                    //       Unicode versions have a W postfix. There's no macro
                    //       that depends on define UNICODE

                    IMAGEHLP_MODULE moduleInfo;

                    ZeroMemory( &moduleInfo, sizeof(moduleInfo) );
                    moduleInfo.SizeOfStruct = sizeof(moduleInfo);

                    if ( SymGetModuleInfo( hProcess, frame.AddrPC.Offset, &moduleInfo ) )
                    {
                        DWORD   dwRelOffset = 0;
                        BYTE    symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 256 ];
                        PIMAGEHLP_SYMBOL    pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;

                        ZeroMemory( symbolBuffer, sizeof(symbolBuffer) );
                        pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
                        pSymbol->MaxNameLength = 256;

                        if ( SymGetSymFromAddr( hProcess, frame.AddrPC.Offset, &dwRelOffset, pSymbol ) )
                            fprintf( fout, "<errormail:StackInfo " \
                                "pos=\"%d\" ip=\"0x%p\" rel=\"0x%p\" ordinal=\"%s+0x%p\" name=\"%s\" path=\"%s\"/>\n",
                                frameNum,
                                frame.AddrPC.Offset,
                                frame.AddrPC.Offset - moduleInfo.BaseOfImage,
                                pSymbol->Name,
                                frame.AddrPC.Offset - pSymbol->Address,
                                GetFileName( moduleInfo.LoadedImageName ).c_str(),
                                GetFileDirectory( moduleInfo.LoadedImageName ).c_str()
                                );
                        else
                            fprintf( fout, "<errormail:StackInfo " \
                                "pos=\"%d\" ip=\"0x%p\" rel=\"0x%p\" name=\"%s\" path=\"%s\"/>\n",
                                frameNum,
                                frame.AddrPC.Offset,
                                frame.AddrPC.Offset - moduleInfo.BaseOfImage,
                                GetFileName( moduleInfo.LoadedImageName ).c_str(),
                                GetFileDirectory( moduleInfo.LoadedImageName ).c_str()
                                );
                    }
                    else
                        fprintf( fout, "<errormail:StackInfo pos=\"%d\" ip=\"0x%p\"/>\n",
                            frameNum,
                            frame.AddrPC.Offset
                            );

                    frameNum++;
                }

            } while ( bSuccess );

            fprintf( fout, "</errormail:Stack>\n" );

            SymCleanup( hProcess );

            CloseHandle( hProcess );
        }

    }

    return fSuccess;
}

//***************************************************************************

BOOL WriteDumpFile( DWORD dwProcessId, PEXCEPTION_POINTERS pExceptionPointers, DWORD dwThreadId )
{
    BOOL    fSuccess = FALSE;
    PMINIDUMP_EXCEPTION_INFORMATION lpExceptionParam = NULL;
    MINIDUMP_EXCEPTION_INFORMATION  ExceptionParam;

    HMODULE hDbgHelp = LoadLibrary( _T("DBGHELP.DLL" ) );
    MiniDumpWriteDump_PROC  pMiniDumpWriteDump = NULL;

    if ( hDbgHelp )
    {
        pMiniDumpWriteDump = (MiniDumpWriteDump_PROC)GetProcAddress( hDbgHelp, "MiniDumpWriteDump" );

        if ( !pMiniDumpWriteDump )
        {
            FreeLibrary( hDbgHelp );
            return false;
        }
    }

    if ( !pMiniDumpWriteDump )
        return false;

    HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );

    if ( IsValidHandle(hProcess) )
    {
        TCHAR   szTempPath[MAX_PATH];

        if ( GetTempPath( elementsof(szTempPath), szTempPath ) )
        {
            TCHAR   szFileName[MAX_PATH];

            if ( GetTempFileName( szTempPath, TEXT("DMP"), 0, szFileName ) )
            {
                HANDLE  hFile =  CreateFile( szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

                if ( hFile )
                {
                    if ( pExceptionPointers && dwThreadId )
                    {
                        ExceptionParam.ThreadId = dwThreadId;
                        ExceptionParam.ExceptionPointers = pExceptionPointers;
                        ExceptionParam.ClientPointers = TRUE;

                        EXCEPTION_POINTERS  aExceptionPointers;
                        EXCEPTION_RECORD    aExceptionRecord;

                        ReadProcessMemory(
                            hProcess,
                            pExceptionPointers,
                            &aExceptionPointers,
                            sizeof(aExceptionPointers),
                            NULL );


                        ReadProcessMemory(
                            hProcess,
                            aExceptionPointers.ExceptionRecord,
                            &aExceptionRecord,
                            sizeof(aExceptionRecord),
                            NULL );

                        g_dwExceptionCode = aExceptionRecord.ExceptionCode;

                        lpExceptionParam = &ExceptionParam;
                    }

                    fSuccess = pMiniDumpWriteDump( hProcess, dwProcessId, hFile, MiniDumpNormal, lpExceptionParam, NULL, NULL );

                    CloseHandle( hFile );

                    WideCharToMultiByte( CP_ACP, 0, szFileName, -1, g_szDumpFileNameA, MAX_PATH, NULL, NULL );
                    _tcscpy( g_szDumpFileName, szFileName );
                }

                if ( !fSuccess )
                    DeleteFile( szFileName );
            }
        }

        CloseHandle( hProcess );
    }

    FreeLibrary( hDbgHelp );

    return fSuccess;
}

//***************************************************************************

static DWORD FindProcessForImage( LPCTSTR lpImagePath )
{
    DWORD   dwProcessId = 0;
    DWORD   aProcesses[1024];
    DWORD   dwSize = 0;
    TCHAR   szShortImagePath[MAX_PATH];

    if ( GetShortPathName( lpImagePath, szShortImagePath, elementsof(szShortImagePath) ) &&
        EnumProcesses( aProcesses, sizeof(aProcesses), &dwSize ) )
    {
        unsigned nProcesses = dwSize / sizeof(aProcesses[0]);

        for ( unsigned i = 0; !dwProcessId && i < nProcesses; i++ )
        {
            HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i] );

            if ( IsValidHandle(hProcess) )
            {
                TCHAR   szModulePath[MAX_PATH+1];

                if ( GetModuleFileNameEx( hProcess, NULL, szModulePath, MAX_PATH ) )
                {
                    TCHAR   szShortModulePath[MAX_PATH];

                    if ( GetShortPathName( szModulePath, szShortModulePath, elementsof(szShortModulePath) ) )
                    {
                        if ( 0 == _tcsicmp( szShortModulePath, szShortImagePath ) )
                            dwProcessId = aProcesses[i];
                    }
                }

                CloseHandle( hProcess );
            }
        }
    }

    return dwProcessId;
}
//***************************************************************************

static bool ParseCommandArgs( LPDWORD pdwProcessId, PEXCEPTION_POINTERS* ppException, LPDWORD pdwThreadId )
{
    int     argc = __argc;
    TCHAR   **argv = __targv;
    bool    bSuccess = true;

    for ( int argn = 1; bSuccess && argn < argc; argn++ )
    {
        if ( 0 == _tcsicmp( argv[argn], _T("-h") ) ||
             0 == _tcsicmp( argv[argn], _T("/h") ) ||
             0 == _tcsicmp( argv[argn], _T("-?") ) ||
             0 == _tcsicmp( argv[argn], _T("/?") ) ||
             0 == _tcsicmp( argv[argn], _T("/help") ) ||
             0 == _tcsicmp( argv[argn], _T("-help") ) ||
             0 == _tcsicmp( argv[argn], _T("--help") )
             )
        {
            HINSTANCE   hInstance = GetModuleHandle(NULL);
            TCHAR   szUsage[FORMATBUFSIZE];
            TCHAR   szProcess[FORMATBUFSIZE];
            TCHAR   szProcessDescription[FORMATBUFSIZE];
            TCHAR   szHelpDescription[FORMATBUFSIZE];

            LoadAndFormatString( hInstance, IDS_MSG_CMDLINE_USAGE, szUsage, elementsof(szUsage) );
            LoadAndFormatString( hInstance, IDS_MSG_PARAM_PROCESSID, szProcess, elementsof(szProcess) );
            LoadAndFormatString( hInstance, IDS_MSG_PARAM_PROCESSID_DESCRIPTION, szProcessDescription, elementsof(szProcessDescription) );
            LoadAndFormatString( hInstance, IDS_MSG_PARAM_HELP_DESCRIPTION, szHelpDescription, elementsof(szHelpDescription) );

            _tprintf(
                TEXT("\n%s: crashrep %s\n\n")
                TEXT("/?, -h[elp]          %s\n\n")
                TEXT("%-20s %s\n\n"),
                szUsage, szProcess, szHelpDescription, szProcess, szProcessDescription
                );

            return true;
        }
        else if ( 0 == _tcsicmp( argv[argn], _T("-p") ) ||
             0 == _tcsicmp( argv[argn], _T("/p") ) )
        {
            if ( ++argn < argc )
                *pdwProcessId = _tcstoul( argv[argn], NULL, 0 );
            else
                bSuccess = false;
        }
        else if ( 0 == _tcsicmp( argv[argn], _T("-excp") ) ||
                  0 == _tcsicmp( argv[argn], _T("/excp") ) )
        {
            if ( ++argn < argc )
                *ppException = (PEXCEPTION_POINTERS)_tcstoul( argv[argn], NULL, 0 );
            else
                bSuccess = false;
        }
        else if ( 0 == _tcsicmp( argv[argn], _T("-t") ) ||
                  0 == _tcsicmp( argv[argn], _T("/t") ) )
        {
            if ( ++argn < argc )
                *pdwThreadId = _tcstoul( argv[argn], NULL, 0 );
            else
                bSuccess = false;
        }
        else // treat parameter as image path
        {
            TCHAR   szImagePath[MAX_PATH];
            LPTSTR  lpImageName;

            if ( GetFullPathName( argv[argn], MAX_PATH, szImagePath, &lpImageName ) )
            {
                DWORD   dwProcessId = FindProcessForImage( szImagePath );

                if ( dwProcessId )
                    *pdwProcessId = dwProcessId;
                else
                    bSuccess = false;
            }
        }
    }

    if ( !*pdwProcessId )
    {
        TCHAR   szImagePath[MAX_PATH];
        LPTSTR  lpImageName;

        if ( GetFullPathName( TEXT("soffice.exe"), MAX_PATH, szImagePath, &lpImageName ) )
        {
            DWORD   dwProcessId = FindProcessForImage( szImagePath );

            if ( dwProcessId )
                *pdwProcessId = dwProcessId;
            else
                bSuccess = false;
        }
    }

    return bSuccess;
}

//***************************************************************************

BOOL WriteCommentFile( LPCTSTR lpComment )
{
    BOOL    fSuccess = FALSE;
    TCHAR   szTempPath[MAX_PATH];

    if ( GetTempPath( elementsof(szTempPath), szTempPath ) )
    {
        TCHAR   szFileName[MAX_PATH];

        if ( GetTempFileName( szTempPath, TEXT("CMT"), 0, szFileName ) )
        {
            HANDLE  hFile =  CreateFile( szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hFile )
            {
                DWORD   dwBytesWritten;

                int needed = WideCharToMultiByte( CP_UTF8, 0, lpComment, -1, NULL, 0, NULL, NULL );
                if ( needed )
                {
                    char *lpCommentUTF8 = (char *)alloca( needed );
                    WideCharToMultiByte( CP_UTF8, 0, lpComment, -1, lpCommentUTF8, needed, NULL, NULL );
                    fSuccess = WriteFile( hFile, lpCommentUTF8, strlen(lpCommentUTF8), &dwBytesWritten, NULL );
                }
                else
                    fSuccess = TRUE;


                CloseHandle( hFile );

                WideCharToMultiByte( CP_ACP, 0, szFileName, -1, g_szCommentFileNameA, MAX_PATH, NULL, NULL );
            }

            if ( !fSuccess )
                DeleteFile( szFileName );
        }
    }

    return fSuccess;
}

//***************************************************************************

static int _tsetenv( const _TCHAR *lpVar, const _TCHAR *lpValue )
{
    if ( !lpValue )
        lpValue = _T("");

    _TCHAR  *envstr = (TCHAR *)alloca( (_tcslen( lpVar ) + _tcslen( lpValue ) + 2) * sizeof(_TCHAR) );

    _tcscpy( envstr, lpVar );
    _tcscat( envstr, _T("=") );
    _tcscat( envstr, lpValue );

    return _tputenv( envstr );
}

static bool read_line( FILE *fp, string& rLine )
{
    char szBuffer[1024];
    bool bSuccess = false;
    bool bEOL = false;
    string  line;


    while ( !bEOL && fgets( szBuffer, sizeof(szBuffer), fp ) )
    {
        int len = strlen(szBuffer);

        bSuccess = true;

        while ( len && szBuffer[len - 1] == '\n' )
        {
            szBuffer[--len] = 0;
            bEOL = true;
        }

        line.append( szBuffer );
    }

    rLine = line;
    return bSuccess;
}

static string get_script_string( const char *pFileName, const char *pKeyName )
{
    FILE    *fp = fopen( pFileName, "rt" );
    string  retValue;

    if ( fp )
    {
        string line;
        string section;

        while ( read_line( fp, line ) )
        {
            line = trim_string( line );


            string::size_type iEqualSign = line.find( '=', 0 );

            if ( iEqualSign != string::npos )
            {
                string  keyname = line.substr( 0, iEqualSign );
                keyname = trim_string( keyname );

                string  value = line.substr( iEqualSign + 1, -1 );
                value = trim_string( value );

                if ( value.length() && '\"' == value[0] )
                {
                    value.erase( 0, 1 );

                    string::size_type iQuotes = value.find( '"', 0 );

                    if ( iQuotes != string::npos )
                        value.erase( iQuotes );
                }

                if ( 0 == stricmp( keyname.c_str(), pKeyName ) )
                {
                    retValue = value;
                    break;
                }
            }
        }

        fclose( fp );
    }

    return retValue;
}

static bool ReadBootstrapParams()
{
    TCHAR   szBuffer[256] = TEXT("");
    TCHAR   szModuleName[MAX_PATH];
    TCHAR   szDrive[_MAX_DRIVE];
    TCHAR   szDir[_MAX_DIR];
    TCHAR   szFName[_MAX_FNAME];
    TCHAR   szExt[_MAX_EXT];
    TCHAR   szReportServer[MAX_HOSTNAME];
    TCHAR   szReportPort[256];
    bool    bSuccess = false;

    GetModuleFileName( NULL, szModuleName, MAX_PATH );
    _tsplitpath( szModuleName, szDrive, szDir, szFName, szExt );
    _tmakepath( szModuleName, szDrive, szDir, _T("bootstrap"), _T(".ini") );

    if (
        GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ProductKey"),
        TEXT("OpenOffice.org"),
        szBuffer,
        elementsof(szBuffer),
        szModuleName )
        )
    {
        TCHAR   *pVersion = _tcschr( szBuffer, ' ' );

        if ( pVersion )
        {
            *pVersion = 0;
            pVersion++;
        }
        else
            pVersion = TEXT("");

        if ( !_tgetenv( _T("PRODUCTNAME") ) )
        {
            _tsetenv( TEXT("PRODUCTNAME"), szBuffer );
        }
        if ( !_tgetenv( _T("PRODUCTVERSION") ) )
            _tsetenv( TEXT("PRODUCTVERSION"), pVersion );
    }

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("buildid"),
        TEXT("unknown"),
        g_szBuildId, elementsof(g_szBuildId),
        szModuleName );

    g_strDefaultLanguage = get_script_string( "instdb.inf", "DefaultLanguage" );

    if ( GetPrivateProfileString(
        TEXT("ErrorReport"),
        TEXT("ErrorReportPort"),
        TEXT("80"),
        szReportPort, elementsof(szReportPort),
        szModuleName
        ) )
    {
        TCHAR *endptr = NULL;

        unsigned short uReportPort = (unsigned short)_tcstoul( szReportPort, &endptr, 10 );
        if ( uReportPort )
            g_uReportPort = uReportPort;
    }

    if ( GetPrivateProfileString(
        TEXT("ErrorReport"),
        TEXT("ErrorReportServer"),
        TEXT(""),
        szReportServer, elementsof(szReportServer),
        szModuleName
        ) )
    {
        bSuccess = 0 != WideCharToMultiByte( CP_ACP, 0, szReportServer, -1, g_szReportServerA, elementsof(g_szReportServerA), NULL, NULL );
    }

    return bSuccess;
}

//***************************************************************************

bool SendHTTPRequest(
                FILE *fp,
                const char *pszServer,
                unsigned short uPort = 80,
                const char *pszProxyServer = NULL,
                unsigned short uProxyPort = 8080 )
{
    bool success = false;

    struct hostent *hp;

    if ( pszProxyServer )
        hp = gethostbyname( pszProxyServer );
    else
        hp = gethostbyname( pszServer );

    if ( hp )
    {
        SOCKET  s = socket( AF_INET, SOCK_STREAM, 0 );

        if ( s )
        {
            struct sockaddr_in address;

            memcpy(&(address.sin_addr.s_addr), *(hp->h_addr_list),sizeof(struct in_addr));
            address.sin_family = AF_INET;

            if ( pszProxyServer )
                address.sin_port = ntohs( uProxyPort );
            else
                address.sin_port = ntohs( uPort );

            if ( 0 == connect( s, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) )
            {
                fseek( fp, 0, SEEK_END );
                size_t length = ftell( fp );
                fseek( fp, 0, SEEK_SET );

                char buffer[2048];

                if ( pszProxyServer )
                    sprintf( buffer,
                    "POST http://%s:%d/soap/servlet/rpcrouter HTTP/1.0\r\n"
                        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                        "Content-Length: %d\r\n"
                        "SOAPAction: \"\"\r\n\r\n",
                        pszServer,
                        uPort,
                        length
                        );
                else
                    sprintf( buffer,
                        "POST /soap/servlet/rpcrouter HTTP/1.0\r\n"
                        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                        "Content-Length: %d\r\n"
                        "SOAPAction: \"\"\r\n\r\n",
                        length
                        );

                if ( SOCKET_ERROR != send( s, buffer, strlen(buffer), 0 ) )
                {
                    size_t nBytes;

                    do
                    {
                        nBytes = fread( buffer, 1, sizeof(buffer), fp );

                        if ( nBytes )
                            success = SOCKET_ERROR != send( s, buffer, nBytes, 0 );
                    } while( nBytes && success );

                    if ( success )
                    {
                        memset( buffer, 0, sizeof(buffer) );
                        success = SOCKET_ERROR != recv( s, buffer, sizeof(buffer), 0 );
                        if ( success )
                        {
                            char szHTTPSignature[sizeof(buffer)] = "";
                            unsigned uHTTPReturnCode = 0;

                            sscanf( buffer, "%s %d ", szHTTPSignature, &uHTTPReturnCode );
                            success = uHTTPReturnCode == 200;
                        }
                    }
                }

            }

            closesocket( s );
        }
    }

    return success;
}

//***************************************************************************

static void WriteSOAPRequest( FILE *fp )
{
    fprintf( fp,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"\n"
        "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\"\n"
        "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\"\n"
        "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"\n"
        "xmlns:rds=\"urn:ReportDataService\"\n"
        "xmlns:apache=\"http://xml.apache.org/xml-soap\"\n"
        "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n"
        "<SOAP-ENV:Body>\n"
        );

    fprintf( fp, "<rds:submitReport>\n"  );
    fprintf( fp, "<body xsi:type=\"xsd:string\">This is an autogenerated crash report mail.</body>\n" );
    fprintf( fp, "<hash xsi:type=\"apache:Map\">\n" );

    FILE    *fpin = fopen( g_szReportFileNameA, "r" );
    if ( fpin )
    {
        fprintf( fp,
            "<item>\n"
            "<key xsi:type=\"xsd:string\">reportmail.xml</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[" );
        fcopy( fpin, fp );
        fprintf( fp, "]]></value></item>\n" );
        fclose( fpin );
    }

    fpin = fopen( g_szCommentFileNameA, "r" );
    if ( fpin )
    {
        fprintf( fp,
            "<item>\n"
            "<key xsi:type=\"xsd:string\">description.txt</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[" );
        fcopy( fpin, fp );
        fprintf( fp, "]]></value></item>\n" );
        fclose( fpin );
    };


    fpin = fopen( g_szDumpFileNameA, "rb" );
    if ( fpin )
    {
        FILE *fptemp = _tmpfile();

        if ( fptemp )
        {
            if ( base64_encode( fpin, fptemp ) )
            {
                fseek( fptemp, 0, SEEK_SET );
                fprintf( fp,
                    "<item>\n"
                    "<key xsi:type=\"xsd:string\">user.dmp</key>\n"
                    "<value xsi:type=\"xsd:string\">" );
                fcopy( fptemp, fp );
                fprintf( fp, "</value></item>\n" );
            }
            fclose( fptemp );
        }
        fclose( fpin );
    }

    fprintf( fp,
        "</hash>\n"
        "</rds:submitReport>\n"
        "</SOAP-ENV:Body>\n"
        "</SOAP-ENV:Envelope>\n"
        );
}

//***************************************************************************

struct RequestParams
{
    bool    success;
    FILE    *fpin;
    const char *lpServer;
    unsigned short uPort;
    const char *lpProxyServer;
    unsigned short uProxyPort;
    HWND    hwndStatus;
};

void _cdecl SendingThread( void *lpArgs )
{
    RequestParams *pParams = (RequestParams *)lpArgs;

    pParams->success = SendHTTPRequest( pParams->fpin, pParams->lpServer, pParams->uPort, pParams->lpProxyServer, pParams->uProxyPort );

    PostMessage( pParams->hwndStatus, WM_COMMAND, IDOK, 0 );
}

//***************************************************************************

BOOL CALLBACK SendingStatusDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static RequestParams *pRequest = NULL;
    static HANDLE hSendingThread = NULL;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            TCHAR   szBuffer[1024] = TEXT("");
            HINSTANCE   hInstance = (HINSTANCE)GetWindowLong( hwndDlg, GWL_HINSTANCE );
            HWND    hwndParent = (HWND)GetWindowLong( hwndDlg, GWL_HWNDPARENT );

            pRequest = (RequestParams *)lParam;

            LoadAndFormatString( hInstance, IDS_SENDING_REPORT_HEADER, szBuffer, elementsof(szBuffer) );
            SetWindowText( hwndDlg, szBuffer );

            LoadAndFormatString( hInstance, IDS_SENDING_REPORT_STATUS, szBuffer, elementsof(szBuffer) );
            Static_SetText( GetDlgItem(hwndDlg, IDC_SENDING_REPORT_STATUS), szBuffer );

            LoadAndFormatString( hInstance, IDS_CANCEL_BUTTON, szBuffer, elementsof(szBuffer) );
            Button_SetText( GetDlgItem(hwndDlg, IDCANCEL), szBuffer );

            pRequest->hwndStatus = hwndDlg;

            hSendingThread = (HANDLE)_beginthread( SendingThread, 0, pRequest );
        }
        return TRUE;
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDCANCEL:
            TerminateThread( hSendingThread, 0 );
        case IDOK:
            WaitForSingleObject( hSendingThread, INFINITE );
            CloseHandle( hSendingThread );
            EndDialog( hwndDlg, wParam );
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}

//***************************************************************************

bool SendCrashReport( HWND hwndParent, const CrashReportParams &rParams )
{
    bool success = false;
    char szProxyServer[1024] = "";
    unsigned short uProxyPort = 8080;
    TCHAR *endptr = NULL;

    switch ( rParams.uInternetConnection )
    {
    case 2:
        {
            WideCharToMultiByte(
                CP_ACP, 0, rParams.szProxyServer, -1,
                szProxyServer, sizeof(szProxyServer), NULL, NULL );
            uProxyPort = (unsigned short)_tcstoul( rParams.szProxyPort, &endptr, 10 );
        }
        break;
    case 0:
        {
            DWORD   dwProxyEnable = 0;

            RegReadValue( HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
                TEXT("ProxyEnable"),
                &dwProxyEnable,
                sizeof(dwProxyEnable) );

            if ( dwProxyEnable )
            {
                TCHAR   tszProxyServers[1024] = TEXT("");

                if ( ERROR_SUCCESS == RegReadValue( HKEY_CURRENT_USER,
                    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),                TEXT("ProxyServer"),
                    tszProxyServers,
                    sizeof(tszProxyServers) ) )
                {
                    TCHAR *lpHttpStart = _tcsstr( tszProxyServers, TEXT("http=") );

                    if ( lpHttpStart )
                        lpHttpStart += 5;
                    else
                        lpHttpStart = tszProxyServers;

                    TCHAR *lpHttpEnd = _tcschr( lpHttpStart, ';' );

                    if ( lpHttpEnd )
                        *lpHttpEnd = 0;

                    char    szHTTPProxyServer[1024] = "";
                    WideCharToMultiByte( CP_ACP, 0, lpHttpStart, -1, szHTTPProxyServer, sizeof(szHTTPProxyServer), NULL, NULL );

                    char *lpColon = strchr( szHTTPProxyServer, ':' );

                    if ( lpColon )
                    {
                        char *endptr = NULL;

                        *lpColon = 0;
                        uProxyPort = (unsigned short)strtoul( lpColon + 1, &endptr, 10 );
                    }
                    else
                        uProxyPort = 8080;

                    strcpy( szProxyServer, szHTTPProxyServer );

                }
            }
        }
        break;
    default:
    case 1:
        break;
    }

    FILE    *fptemp = _tmpfile();
    if ( fptemp )
    {
        WriteSOAPRequest( fptemp );
        fseek( fptemp, 0, SEEK_SET );

        RequestParams   request;

        request.success = false;
        request.fpin = fptemp;
        request.lpServer = REPORT_SERVER;
        request.uPort = REPORT_PORT;
        request.lpProxyServer = szProxyServer[0] ? szProxyServer : NULL;
        request.uProxyPort = uProxyPort;
        request.hwndStatus = NULL;

        int retid = DialogBoxParam(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_SENDING_STATUS),
            hwndParent,
            SendingStatusDialogProc,
            (LPARAM)&request
            );

        success = request.success;

        if ( IDOK == retid )
        {
            if ( !success )
            {
                TCHAR   szMessage[1024];

                LoadAndFormatString( GetModuleHandle(NULL), IDS_ERROR_MSG_PROXY, szMessage, elementsof(szMessage) );

                MessageBox( hwndParent, szMessage, NULL, MB_ICONERROR | MB_OK );
            }
            else
            {
                TCHAR   szMessage[1024];
                TCHAR   szTitle[1024];

                LoadAndFormatString( GetModuleHandle(NULL), IDS_SENDING_REPORT_STATUS_FINISHED, szMessage, elementsof(szMessage) );
                LoadAndFormatString( GetModuleHandle(NULL), IDS_SENDING_REPORT_HEADER, szTitle, elementsof(szTitle) );

                MessageBox( hwndParent, szMessage, szTitle, MB_ICONINFORMATION | MB_OK );
            }
        }

        fclose( fptemp );
    }
    else
    {
        TCHAR   szMessage[1024];

        LoadAndFormatString( GetModuleHandle(NULL), IDS_ERROR_MSG_DISK_FULL, szMessage, elementsof(szMessage) );

        MessageBox( hwndParent, szMessage, NULL, MB_ICONERROR | MB_OK );
    }

    return success;
}

//***************************************************************************

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int )
{
    int exitcode = -1;


    PEXCEPTION_POINTERS pExceptionPointers = NULL;
    DWORD               dwProcessId = 0;
    DWORD               dwThreadId = 0;

    WSADATA wsaData;
    WORD    wVersionRequested;

    wVersionRequested = MAKEWORD(1, 1);
    WSAStartup(wVersionRequested, &wsaData);

    if ( ReadBootstrapParams() && ParseCommandArgs( &dwProcessId, &pExceptionPointers, &dwThreadId ) && dwProcessId )
    {
        if ( WriteDumpFile( dwProcessId, pExceptionPointers, dwThreadId ) )
        {
            g_fpStackFile = _tmpfile();

            WriteStackFile( g_fpStackFile, dwProcessId, pExceptionPointers );

            InitCommonControls();

            if ( InitRichEdit() )
            {
                CrashReportParams   Params;

                Params.ReadFromRegistry();

                INT_PTR result = DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_DIALOG_FRAME), NULL, DialogProc, (LPARAM)&Params );

                if ( result > 0 )
                {
                    exitcode = 0;
                }
                DeinitRichEdit();
            }

            if ( g_szReportFileNameA[0] )
                DeleteFileA( g_szReportFileNameA );

            if ( g_szCommentFileNameA[0] )
                DeleteFileA( g_szCommentFileNameA );

            DeleteFileA( g_szDumpFileNameA );

            if ( g_fpStackFile )
                fclose( g_fpStackFile );
        }
    }


    return exitcode;
}

