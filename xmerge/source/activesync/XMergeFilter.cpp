// XMergeFilter.cpp: implementation of the CXMergeFilter class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "XMergeFilter.h"

#include <string>


#define ERR_NOJAVA       1
#define ERR_BADCLASSPATH 2
#define ERR_INITJAVA     3


const LPTSTR CXMergeFilter::m_pszPSWExportCLSID     = _T("{BDD611C3-7BAB-460F-8711-5B9AC9EF6020}");
const LPTSTR CXMergeFilter::m_pszPSWExportExt       = _T("sxw");
const LPTSTR CXMergeFilter::m_pszPSWExportDesc      = _T("OpenOffice.org Writer XML Document");
const LPTSTR CXMergeFilter::m_pszPSWExportShortDesc = _T("OpenOffice.org Writer");

const LPTSTR CXMergeFilter::m_pszPSWImportCLSID     = _T("{CB43F086-838D-4FA4-B5F6-3406B9A57439}");
const LPTSTR CXMergeFilter::m_pszPSWImportExt       = _T("psw");
const LPTSTR CXMergeFilter::m_pszPSWImportDesc      = _T("Pocket Word Document - Pocket PC");
const LPTSTR CXMergeFilter::m_pszPSWImportShortDesc = _T("Pocket Word");

const LPTSTR CXMergeFilter::m_pszPXLExportCLSID     = _T("{C6AB3E74-9F4F-4370-8120-A8A6FABB7A7C}");
const LPTSTR CXMergeFilter::m_pszPXLExportExt       = _T("sxc");
const LPTSTR CXMergeFilter::m_pszPXLExportDesc      = _T("OpenOffice.org Calc XML Document");
const LPTSTR CXMergeFilter::m_pszPXLExportShortDesc = _T("OpenOffice.org Calc");

const LPTSTR CXMergeFilter::m_pszPXLImportCLSID     = _T("{43887C67-4D5D-4127-BAAC-87A288494C7C}");
const LPTSTR CXMergeFilter::m_pszPXLImportExt       = _T("pxl");
const LPTSTR CXMergeFilter::m_pszPXLImportDesc      = _T("Pocket Excel Document - Pocket PC");
const LPTSTR CXMergeFilter::m_pszPXLImportShortDesc = _T("Pocket Excel");


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMergeFilter::CXMergeFilter() : m_cRef(1)
{
}

CXMergeFilter::~CXMergeFilter()
{

}


//////////////////////////////////////////////////////////////////////
// IUnknown Methods
//////////////////////////////////////////////////////////////////////

STDMETHODIMP CXMergeFilter::QueryInterface(REFIID riid, void **ppvObject)
{
    if(ppvObject == NULL)
        return E_INVALIDARG;

    if (::IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
    }
    else if (::IsEqualIID(riid, IID_ICeFileFilter))
    {
        *ppvObject = static_cast<ICeFileFilter *>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CXMergeFilter::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CXMergeFilter::Release()
{
    if(::InterlockedDecrement(&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}


//////////////////////////////////////////////////////////////////////
// ICeFileFilter
//////////////////////////////////////////////////////////////////////

STDMETHODIMP CXMergeFilter::FilterOptions(HWND hwndParent)
{
    // We don't currently allow any options
    return HRESULT_FROM_WIN32(NOERROR);
}

STDMETHODIMP CXMergeFilter::FormatMessage(DWORD dwFlags, DWORD dwMessageId,
                        DWORD dwLanguageId, LPTSTR lpBuffer, DWORD nSize,
                        va_list *Arguments, DWORD *pcb)
{
    char errMsg[50];

    switch(dwMessageId)
    {
    case ERR_NOJAVA:
        lstrcpy(errMsg, "Unable to locate Java 1.4 installation.");
        break;

    case ERR_BADCLASSPATH:
        lstrcpy(errMsg, "Unable to locate XMerge Jar files.");
        break;

    case ERR_INITJAVA:
        lstrcpy(errMsg, "Error initialising Java Runtime Environment.");
        break;
    }

    char* buf = (char*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, strlen(errMsg));
    lstrcpy(buf, errMsg);

    *(char**)lpBuffer = buf;
    *pcb = strlen(errMsg);


    return HRESULT_FROM_WIN32(NOERROR);
}


STDMETHODIMP CXMergeFilter::NextConvertFile(int nConversion, CFF_CONVERTINFO *pci,
                             CFF_SOURCEFILE *psf, CFF_DESTINATIONFILE *pdf,
                             volatile BOOL *pbCancel, CF_ERROR *perr)
{
    std::string appArgs;
    std::string appName;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    ZeroMemory( &pi, sizeof(pi) );

    si.cb = sizeof(si);


    // Locate Java Home
    TCHAR* szJavaDir = GetJavaBaseDir();
    if (szJavaDir == NULL)
    {
        *perr = ERR_NOJAVA;
        return HRESULT_FROM_WIN32(E_FAIL);
    }


    // Make sure that there is a Java executable
    appName += szJavaDir;
    appName += "\\bin\\javaw.exe";

    // Make sure that Java actually exists
    if (GetFileAttributes(appName.c_str()) == INVALID_FILE_SIZE)
    {
        *perr = ERR_NOJAVA;
        return HRESULT_FROM_WIN32(E_FAIL);
    }

    // Wrap the executable path in quotes in case of spaces
    appName.insert(0, "\"");
    appName.append("\"");


    // Get the StarOffice/OpenOffice class directory
    TCHAR* szClassPath = GetXMergeClassPath();
    if (szClassPath == NULL)
    {
        *perr = ERR_BADCLASSPATH;
        delete szJavaDir;

        return HRESULT_FROM_WIN32(E_FAIL);
    }

    // Need to build the entire command line for calling out to Java
    appArgs =  appName + " -Djava.class.path=";
    appArgs += szClassPath;
    appArgs += " org.openoffice.xmerge.util.ActiveSyncDriver ";

    if (!lstrcmp(psf->szExtension, "sxw"))
    {
        appArgs += "staroffice/sxw ";
        appArgs += "application/x-pocket-word ";
    }
    else if(!lstrcmp(psf->szExtension, "psw"))
    {
        appArgs += "application/x-pocket-word ";
        appArgs += "staroffice/sxw ";
    }
    else if(!lstrcmp(psf->szExtension, "sxc"))
    {
        appArgs += "staroffice/sxc ";
        appArgs += "application/x-pocket-excel ";
    }
    else if(!lstrcmp(psf->szExtension, "pxl"))
    {
        appArgs += "application/x-pocket-excel ";
        appArgs += "staroffice/sxc ";
    }


    // ActiveSync sometimes gives out long file names, especially when automatically syncing
    appArgs += "\"";
    appArgs += psf->szFullpath;
    appArgs += "\" \"";
    appArgs += pdf->szFullpath;
    appArgs += "\"";


    if(!CreateProcess(NULL,
                  (char*)appArgs.c_str(),
                  NULL,             // No Process Attributes
                  NULL,             // No Thread Attributes
                  FALSE,            // Don't want this process getting handles
                  CREATE_NO_WINDOW, // No console
                  NULL,             // No special environment
                  NULL,             // Current Working Directory is okay
                  &si,
                  &pi))
    {
        delete szClassPath;
        delete szJavaDir;

        *perr = ERR_INITJAVA;
        return HRESULT_FROM_WIN32(E_FAIL);
    }

    // Wait for the new process to work
    WaitForSingleObject(pi.hProcess, INFINITE);

    delete szClassPath;
    delete szJavaDir;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}


typedef HRESULT (WINAPI *SHGETFOLDERPATH)( HWND, int, HANDLE, DWORD, LPTSTR );


TCHAR* CXMergeFilter::GetJavaBaseDir()
{
    HRESULT lRet;

    HKEY hKey = NULL;
    HKEY hDataKey = NULL;

    TCHAR szClassName[_MAX_PATH] = "\0";
    TCHAR szKeyName[_MAX_PATH]   = "\0";
    DWORD dwClassName            = _MAX_PATH;
    DWORD dwKeyName              = _MAX_PATH;

    /*
     * Java leaves registry keys at HKLM\SOFTWARE\JavaSoft.
     *
     * Check for a JRE installation first
     */
    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\JavaSoft\\Java Runtime Environment"), 0, KEY_READ, &hKey);
    if (lRet != ERROR_SUCCESS)
        return NULL;

    /*
     * Need to enumerate the subkeys.  The current version may not be 1.4 if
     * multiple versions are installed on the system.
     */
    for (DWORD i = 0; lRet != ERROR_NO_MORE_ITEMS; i++)
    {
        lRet = ::RegEnumKeyEx(hKey, i, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL);
        if(!strncmp(szKeyName, "1.4", 3))
            break;
    }

    // Found a Java 1.4 installation.  Can now read its home directory.
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_READ, &hDataKey);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return NULL;
    }

    // Location shouldn't be greater than _MAX_PATH
    TCHAR*  szJavaHome = new TCHAR[_MAX_PATH + 1];
    DWORD dwSize = _MAX_PATH + 1;

    // Now read the JavaHome value
    lRet = ::RegQueryValueEx(hDataKey, _T("JavaHome"), 0, NULL, (LPBYTE)szJavaHome, &dwSize);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hDataKey);
        RegCloseKey(hKey);
        delete szJavaHome;
        return NULL;
    }

    RegCloseKey(hDataKey);
    RegCloseKey(hKey);


    // Check that the directory exists before returning it
    DWORD dwAttrs = GetFileAttributes(szJavaHome);

    if (((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) || dwAttrs == INVALID_FILE_SIZE)
    {
        delete szJavaHome;
        return NULL;
    }

    return szJavaHome;
}



TCHAR* CXMergeFilter::GetXMergeClassPath()
{

    /*
     * The Office base directory can be found in the sversion.ini file in the
     * user's profile directory.
     *
     * Supposed to be platform agnostic, but not quite sure yet!
     */

    TCHAR szIniPath[MAX_PATH];


    /*
     * On Windows ME and Windows 2000, the SHGetFolderPath function is incorporated
     * into SHELL32.dll.  Unfortunately, this is not the case in Windows 95/98 so
     * the SHFOLDER library needs to be dynamically loaded to get an address for the
     * procedure.
     */
    SHGETFOLDERPATH SHGetFolderPath;
    HMODULE hModSHFolder = LoadLibrary("shfolder.dll");
    if ( hModSHFolder != NULL )
    {
        SHGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(hModSHFolder, "SHGetFolderPathA");
    }
    else
    {
        FreeLibrary(hModSHFolder);
        SHGetFolderPath = NULL;
    }


    if (SHGetFolderPath != NULL )
    {
        if(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, szIniPath) != S_OK)
        {
            FreeLibrary(hModSHFolder);
            return NULL;
        }
    }
    else
    {
        FreeLibrary(hModSHFolder);
        return NULL;
    }

    FreeLibrary(hModSHFolder);


    lstrcat(szIniPath, "\\sversion.ini");


    // Should check the existence of the file here
    TCHAR* szSectVal;
    WIN32_FILE_ATTRIBUTE_DATA fInfo;

    if (!GetFileAttributesEx(szIniPath, GetFileExInfoStandard, &fInfo))
        return NULL;


    szSectVal = new char[fInfo.nFileSizeLow];

    if (!GetPrivateProfileSection("Versions", szSectVal, 3000, szIniPath))
    {
        delete szSectVal;
        return NULL;
    }

    char* token = szSectVal;
    std::string clsDir;

    while (*token != '\0')
    {
        // Clear the clsDir each time we go around
        clsDir = "";

        // Check for a compatible StarOffice/OpenOffice version
        if (!strncmp(token, "StarOffice 6", 12) || !strncmp(token, "OpenOffice.org 1", 12))
        {
            char* uri = token;

            // Jump past the equals sign
            uri = strchr(uri, '=') + 1;

            // Lose the file:///
            uri += 8;

            for (int j = 0; j < strlen(uri); j++)
            {
                switch (uri[j])
                {
                case '/':
                    clsDir += '\\';
                    break;

                case '%':
                    // Read in the two following characters
                    char* stop;
                    char buf[3];
                    buf[0] = uri[++j];  buf[1] = uri[++j];  buf[2] = '\0';
                    clsDir += (char)strtol(buf, &stop, 16);
                    break;

                default:
                    clsDir += uri[j];
                }
            }
        }
        else
        {
            token += strlen(token) + 1;
            continue;
        }


        // Append the JAR file subdirectory
        clsDir += "\\program\\classes\\";

        // Check for the existence of the necessary JAR files
        std::string jars[3] = { clsDir + "xmerge.jar", clsDir + "pocketword.jar", clsDir + "pexcel.jar" };
        BOOL found = TRUE;

        for (int k = 0; k < 3; k++)
        {
            if (!GetFileAttributesEx(jars[k].c_str(), GetFileExInfoStandard, &fInfo))
            {
                found = FALSE;
                break;
            }
        }

        if (!found)
        {
            token += strlen(token) + 1;
            continue;
        }

        // All files are present so return the classpath.  Add in quotes in case of spaces
        std::string clsPath = "\"" + jars[0] + ";" + jars[1] + ";" + jars[2] + ";" + "\"";

        char* cPath = new char[clsPath.length() + 1];
        ZeroMemory(cPath, clsPath.length() + 1);
        strcpy(cPath, clsPath.c_str());

        delete szSectVal;

        return cPath;
    }

    delete szSectVal;

    return NULL;
}
