/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file_url.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 09:51:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

 /***************************************************
  * Internal header file, declares all functions
  * that are not part of the offical API and are
  * not defined in the osl header files
  **************************************************/

 #ifndef _FILE_URL_H_
 #define _FILE_URL_H_

 #ifndef _FILE_H_
 #include <osl/file.h>
 #endif


/***************************************************
 * constants
 **************************************************/

#define _tcslen(a)      wcslen((const wchar_t *) a)
#define _tcsspn(a,b)    wcsspn((const wchar_t *) a, (const wchar_t *) b)
#define _istalpha(a)    iswalpha((wint_t) a)

const sal_Unicode UNICHAR_SLASH = ((sal_Unicode)'/');
const sal_Unicode UNICHAR_COLON = ((sal_Unicode)':');
const sal_Unicode UNICHAR_DOT   = ((sal_Unicode)'.');

#define ELEMENTS_OF_ARRAY(arr) (sizeof(arr)/(sizeof((arr)[0])))

#if OSL_DEBUG_LEVEL > 0
#define OSL_ENSURE_FILE( cond, msg, file ) ( (cond) ?  (void)0 : _osl_warnFile( msg, file ) )
#else
#define OSL_ENSURE_FILE( cond, msg, file ) ((void)0)
#endif

typedef sal_Unicode                         TCHAR;
typedef sal_Unicode                         *LPTSTR;
typedef const sal_Unicode                   *LPCTSTR;
typedef wchar_t                         *LPWSTR;
typedef const wchar_t                   *LPCWSTR;
typedef sal_Unicode                         DWORD;
#define WINAPI

#define CHARSET_SEPARATOR                   L"\\/"
#define WSTR_SYSTEM_ROOT_PATH               L"\\\\.\\"


/******************************************************************************
 *
 *                  Data Type Definition
 *
 ******************************************************************************/

#define PATHTYPE_ERROR                      0
#define PATHTYPE_RELATIVE                   1
#define PATHTYPE_ABSOLUTE_UNC               2
#define PATHTYPE_ABSOLUTE_LOCAL             3
#define PATHTYPE_MASK_TYPE                  0xFF
#define PATHTYPE_IS_VOLUME                  0x0100
#define PATHTYPE_IS_SERVER                  0x0200

#define VALIDATEPATH_NORMAL                 0x0000
#define VALIDATEPATH_ALLOW_WILDCARDS        0x0001
#define VALIDATEPATH_ALLOW_ELLIPSE          0x0002
#define VALIDATEPATH_ALLOW_RELATIVE         0x0004
#define VALIDATEPATH_ALLOW_UNC              0x0008

typedef struct {
    UINT            uType;
    rtl_uString*    ustrDrive;
    rtl_uString*    ustrFilePath;   /* holds native directory path */
    int             d_attr;         /* OS/2 file attributes */
    int             nRefCount;
}DirectoryItem_Impl;

#define DIRECTORYTYPE_LOCALROOT     0
#define DIRECTORYTYPE_NETROOT       1
#define DIRECTORYTYPE_NETRESORCE    2
#define DIRECTORYTYPE_FILESYSTEM    3

#define DIRECTORYITEM_DRIVE     0
#define DIRECTORYITEM_FILE      1
#define DIRECTORYITEM_SERVER    2

typedef struct
{
    UINT         uType;
    rtl_uString* ustrPath;           /* holds native directory path */
    DIR*         pDirStruct;
    ULONG        ulDriveMap;
    ULONG        ulNextDrive;
    ULONG        ulNextDriveMask;
} DirectoryImpl;

/* Different types of paths */
typedef enum _PATHTYPE
{
    PATHTYPE_SYNTAXERROR = 0,
    PATHTYPE_NETROOT,
    PATHTYPE_NETSERVER,
    PATHTYPE_VOLUME,
    PATHTYPE_FILE
} PATHTYPE;

DWORD WINAPI IsValidFilePath(LPCTSTR, LPCTSTR*, DWORD);

typedef struct
{
    rtl_uString* ustrFilePath;      /* holds native file path */
    int fd;
} oslFileHandleImpl;


typedef struct _oslVolumeDeviceHandleImpl
{
    sal_Char pszMountPoint[PATH_MAX];
    sal_Char pszFilePath[PATH_MAX];
    sal_Char pszDevice[PATH_MAX];
    sal_Char ident[4];
    sal_uInt32   RefCount;
} oslVolumeDeviceHandleImpl;

/* OS error to errno values mapping table */
struct errentry {
    unsigned long oscode;   /* OS return value */
    int errnocode;          /* System V error code */
};

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************
* _osl_getSystemPathFromFileURL
*************************************************/

#define FURL_ALLOW_RELATIVE sal_True
#define FURL_DENY_RELATIVE  sal_False

oslFileError osl_getSystemPathFromFileURL_Ex(rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath, sal_Bool bAllowRelative);

/**************************************************
* FileURLToPath
*************************************************/

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL);


#ifdef __cplusplus
}
#endif


#endif /* #define _FILE_URL_H_ */

