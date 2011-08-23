// XMergeFilter.cpp: implementation of the CXMergeFilter class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "XMergeFilter.h"

#include <string>


#define ERR_NOJAVA       1
#define ERR_BADCLASSPATH 2
#define ERR_INITJAVA     3

                      
const LPTSTR CXMergeFilter::m_pszPSWExportCLSID		= _T("{BDD611C3-7BAB-460F-8711-5B9AC9EF6020}");
const LPTSTR CXMergeFilter::m_pszPSWExportExt		= _T("sxw");
const LPTSTR CXMergeFilter::m_pszPSWExportDesc		= _T("OpenOffice.org Writer XML Document");
const LPTSTR CXMergeFilter::m_pszPSWExportShortDesc	= _T("OpenOffice.org Writer");

const LPTSTR CXMergeFilter::m_pszPSWImportCLSID		= _T("{CB43F086-838D-4FA4-B5F6-3406B9A57439}");
const LPTSTR CXMergeFilter::m_pszPSWImportExt		= _T("psw");
const LPTSTR CXMergeFilter::m_pszPSWImportDesc		= _T("Pocket Word Document - Pocket PC");
const LPTSTR CXMergeFilter::m_pszPSWImportShortDesc	= _T("Pocket Word");

const LPTSTR CXMergeFilter::m_pszPXLExportCLSID		= _T("{C6AB3E74-9F4F-4370-8120-A8A6FABB7A7C}");
const LPTSTR CXMergeFilter::m_pszPXLExportExt		= _T("sxc");
const LPTSTR CXMergeFilter::m_pszPXLExportDesc		= _T("OpenOffice.org Calc XML Document");
const LPTSTR CXMergeFilter::m_pszPXLExportShortDesc	= _T("OpenOffice.org Calc");

const LPTSTR CXMergeFilter::m_pszPXLImportCLSID		= _T("{43887C67-4D5D-4127-BAAC-87A288494C7C}");
const LPTSTR CXMergeFilter::m_pszPXLImportExt		= _T("pxl");
const LPTSTR CXMergeFilter::m_pszPXLImportDesc		= _T("Pocket Excel Document - Pocket PC");
const LPTSTR CXMergeFilter::m_pszPXLImportShortDesc	= _T("Pocket Excel");


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMergeFilter::CXMergeFilter() : m_cRef(1)
{
    m_bHaveExcel = FALSE;
    m_bHaveWord  = FALSE;

    m_szClasspath   = NULL;
    m_szJavaBaseDir = NULL;
}

CXMergeFilter::~CXMergeFilter()
{
    if (m_szClasspath != NULL)
    {
        delete m_szClasspath;
    }

    if (m_szJavaBaseDir != NULL)
    {
        delete m_szJavaBaseDir;
    }

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
    TCHAR errMsg[1024];
    
    HKEY  hKey   = NULL;
    DWORD dwSize = 1024;


    long lRet = 0;

    // Attempt to find the messages in the registry
    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Sun Microsystems\\StarOffice\\XMergeSync\\Messages\\Error"),
                            0, KEY_READ, &hKey);
    if (lRet != ERROR_SUCCESS) 
    {
        // Try the user's portion of the registry
        lRet = ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Sun Microsystems\\StarOffice\\XMergeSync\\Messages\\Error"),
                            0, KEY_READ, &hKey);
        if (lRet != ERROR_SUCCESS) 
        {
            hKey = NULL;
        }
    }


    switch(dwMessageId)
    {
    case ERR_NOJAVA:
        lRet = ::RegQueryValueEx(hKey, _T("Java"), 0, NULL, (LPBYTE)errMsg, &dwSize);
        if (lRet != ERROR_SUCCESS)
        {
            lstrcpy(errMsg, "Unable to locate Java 1.4/1.5 installation.");
        }
        break;

    case ERR_BADCLASSPATH:
        lRet = ::RegQueryValueEx(hKey, _T("Classpath"), 0, NULL, (LPBYTE)errMsg, &dwSize);
        if (lRet != ERROR_SUCCESS)
        {
            lstrcpy(errMsg, "Unable to locate XMerge Jar files.");
        }
        break;

    case ERR_INITJAVA:
        lRet = ::RegQueryValueEx(hKey, _T("JavaInit"), 0, NULL, (LPBYTE)errMsg, &dwSize);
        if (lRet != ERROR_SUCCESS)
        {
            lstrcpy(errMsg, "Error initialising the Java Runtime Environment.");
        }
        break;
    }

    char* buf = (char*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (lstrlen(errMsg) + 1) * sizeof(TCHAR));
    lstrcpyn(buf, errMsg, lstrlen(errMsg));

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


    /*
     * First step: Locate Java and establish the classpath.  If these can't 
     *             be done succesfully, then avoid all further processing.
     */

    // Locate Java Home if it hasn't already been done.
    if (m_szJavaBaseDir == NULL)
    {
        m_szJavaBaseDir = GetJavaBaseDir();

        if (m_szJavaBaseDir == NULL)
        {
            *perr = ERR_NOJAVA;		
            return HRESULT_FROM_WIN32(E_FAIL);
        }
    }

    // Get the StarOffice/OpenOffice class directory
    if (m_szClasspath == NULL)
    {
        m_szClasspath = GetXMergeClassPath();
        
        if (m_szClasspath == NULL)
        {
            *perr = ERR_BADCLASSPATH;
            return HRESULT_FROM_WIN32(E_FAIL);
        }
    }


    /*
     * Second step:  Check the files we're going to process.  If we don't have 
     *				 an XMerge plugin for the file then we can't convert.
     */
    if ((!lstrcmp(psf->szExtension, "sxw")  || !lstrcmp(psf->szExtension, "psw"))
            && !m_bHaveWord)
    {
        *perr = ERR_BADCLASSPATH;
        return HRESULT_FROM_WIN32(E_FAIL);
    }
    else if ((!lstrcmp(psf->szExtension, "sxc")  || !lstrcmp(psf->szExtension, "pxl"))
                 && !m_bHaveExcel)
    {
        *perr = ERR_BADCLASSPATH;
        return HRESULT_FROM_WIN32(E_FAIL);
    }
    

    /*
     * Third step:  Locate the Java executable and build and execute the command
     *				line to carry out the conversion.
     */

    // Find the Java executable and make sure it exists
    appName += m_szJavaBaseDir;
    appName += "\\bin\\javaw.exe";

    if (GetFileAttributes(appName.c_str()) == INVALID_FILE_SIZE)
    {
        *perr = ERR_NOJAVA;
        return HRESULT_FROM_WIN32(E_FAIL);
    }

    // Wrap the executable path in quotes in case of spaces
    appName.insert(0, "\"");
    appName.append("\"");


    
    // Need to build the entire command line for calling out to Java	
    appArgs =  appName + " -Djava.class.path=";
    appArgs += m_szClasspath;
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
                  NULL,				// No Process Attributes
                  NULL,				// No Thread Attributes
                  FALSE,			// Don't want this process getting handles
                  CREATE_NO_WINDOW,	// No console
                  NULL,				// No special environment
                  NULL,				// Current Working Directory is okay
                  &si,			
                  &pi))
    {
        *perr = ERR_INITJAVA;
        return HRESULT_FROM_WIN32(E_FAIL);
    }

    // Wait for the new process to work
    WaitForSingleObject(pi.hProcess, INFINITE);

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
    TCHAR szCurrentJava[_MAX_PATH] = "\0";
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

    // Locations shouldn't be greater than _MAX_PATH
    TCHAR*  szJavaHome = new TCHAR[_MAX_PATH + 1];
    DWORD dwSize = _MAX_PATH + 1;

    /* use current version */
    lRet = ::RegQueryValueEx(hKey, _T("CurrentVersion"), 0, NULL, (LPBYTE)szCurrentJava, &dwSize);
    
    /*
    for (DWORD i = 0; lRet != ERROR_NO_MORE_ITEMS; i++)
    {
        lRet = ::RegEnumKeyEx(hKey, i, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL);
        if(!strncmp(szKeyName, "1.4", 3))
            break;
        dwKeyName = _MAX_PATH;
    }
    // Found a Java 1.4 installation.  Can now read its home directory.
    */
    
    
    lRet = ::RegOpenKeyEx(hKey, _T(szCurrentJava), 0, KEY_READ, &hDataKey);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        delete [] szJavaHome;
        return NULL;
    }


    // Now read the JavaHome value
    dwSize = _MAX_PATH + 1;
    lRet = ::RegQueryValueEx(hDataKey, _T("JavaHome"), 0, NULL, (LPBYTE)szJavaHome, &dwSize);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hDataKey);
        RegCloseKey(hKey);
        delete [] szJavaHome;
        return NULL;
    }

    RegCloseKey(hDataKey);
    RegCloseKey(hKey);

    
    // Check that the directory exists before returning it
    DWORD dwAttrs = GetFileAttributes(szJavaHome);

    if (((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) || dwAttrs == INVALID_FILE_SIZE)
    {
        delete [] szJavaHome;
        return NULL;
    }

    return szJavaHome;
}



TCHAR* CXMergeFilter::GetXMergeClassPath()
{
    /*
     * The DLL will be installed by setup in the program directory of
     * the installation.  The XMerge Jar files, if present, will be 
     * located in the classes directory below program.
     */

    TCHAR szJarPath[MAX_PATH];
    TCHAR szTmpPath[MAX_PATH];

    ZeroMemory(szJarPath, MAX_PATH);
    ZeroMemory(szTmpPath, MAX_PATH);

    WIN32_FILE_ATTRIBUTE_DATA fInfo;

    std::string clsPath;


    // Get the location of the module.
    GetModuleFileName(_Module.m_hInst, szTmpPath, MAX_PATH);

    // Strip off the xmergesync.dll component
    _strlwr(szTmpPath);
    char* modName = strstr(szTmpPath, "xmergesync.dll");
    strncpy(szJarPath, szTmpPath, modName - szTmpPath);

    // Append the classes directory
    strncat(szJarPath, "classes\\", 8);


    // The core xmerge.jar must be present
    ZeroMemory(szTmpPath, MAX_PATH);
    _snprintf(szTmpPath, MAX_PATH, "%s%s\0", szJarPath, "xmerge.jar");

    if (!GetFileAttributesEx(szTmpPath, GetFileExInfoStandard, &fInfo))
    {
        return NULL;
    }
    else 
    {
        clsPath += szTmpPath;
        clsPath += ";";
    }


    // Now check for Pocket Word
    ZeroMemory(szTmpPath, MAX_PATH);
    _snprintf(szTmpPath, MAX_PATH, "%s%s\0", szJarPath, "pocketword.jar");

    if (!GetFileAttributesEx(szTmpPath, GetFileExInfoStandard, &fInfo))
    {
        m_bHaveWord = FALSE;
    }
    else
    {
        m_bHaveWord = TRUE;
        clsPath += szTmpPath;
        clsPath += ";";
    }

    // Now check for Pocket Excel
    ZeroMemory(szTmpPath, MAX_PATH);
    _snprintf(szTmpPath, MAX_PATH, "%s%s\0", szJarPath, "pexcel.jar");

    if (!GetFileAttributesEx(szTmpPath, GetFileExInfoStandard, &fInfo))
    {
        m_bHaveExcel = FALSE;
    }
    else
    {
        m_bHaveExcel = TRUE;
        clsPath += szTmpPath;
        clsPath += ";";
    }

    // Quotes may be need around the ClassPath
    clsPath.insert(0, "\"");
    clsPath += "\"";


    // Return the data
    return _strdup(clsPath.c_str());
}
