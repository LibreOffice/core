/*************************************************************************
 *
 *  $RCSfile: file.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-27 15:25:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _OSL_FILE_H_
#define _OSL_FILE_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifndef _RTL_USTRING_H
#   include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @name Main goals and usage hints

The main intentention of this interface is to provide an universal portable and
high performance access to file system issues on any operating system.<p>

There are a few main goals:<p>

1.The path specifications always have to be absolut. Any usage of relative path
specifications is forbidden. Most operating systems provide a
"Current Directory" per process. This is the reason why relative path
specifications can cause problems in multithreading environments.<p>

2.Proprietary notations of file paths are not supported. Every path notation
must follow the UNC notation. The directory divider is the slash character.<p>

3.The caller cannot get any information whether a file system is case sensitive,
case preserving or not. The operating system implementation itself should
determine if it can map case-insensitive paths. The case correct notation of
a filename or file path is part of the "File Info". This case correct name
can be used as a unique key if neccessary.<p>

4. Obtaining information about files or volumes is controlled by a
bitmask which specifies which fields are of interest. Due to performance
issues it is not recommended to obtain information which is not needed.
But if the operating system provides more information anyway the
implementation can set more fields on output as were requested. It is in the
responsibility of the caller to decide if he uses this additional information
or not. But he should do so to prevent further unnecessary calls if the information
is already there.<br>

The input bitmask supports a flag <code>osl_FileStatus_Mask_Validate</code> which
can be used to force retrieving uncached validated information. Setting this flag
when calling <code>osl_getFileStatus</code> in combination with no other flag is
a synonym for a "FileExists". This should only be done when processing a single file
(f.e. before opening) and NEVER during enumeration of directory contents on any step
of information processing. This would change the runtime behaviour from O(n) to
O(n*n/2) on nearly every file system.<br>
On Windows NT reading the contents of an directory with 7000 entries and
getting full information about every file only takes 0.6 seconds. Specifying the
flag <code>osl_FileStatus_Mask_Validate</code> for each entry will increase the
time to 180 seconds (!!!).

*/



/* Error codes according to errno */

typedef enum {
    osl_File_E_None,
    osl_File_E_PERM,
    osl_File_E_NOENT,
    osl_File_E_SRCH,
    osl_File_E_INTR,
    osl_File_E_IO,
    osl_File_E_NXIO,
    osl_File_E_2BIG,
    osl_File_E_NOEXEC,
    osl_File_E_BADF,
    osl_File_E_CHILD,
    osl_File_E_AGAIN,
    osl_File_E_NOMEM,
    osl_File_E_ACCES,
    osl_File_E_FAULT,
    osl_File_E_BUSY,
    osl_File_E_EXIST,
    osl_File_E_XDEV,
    osl_File_E_NODEV,
    osl_File_E_NOTDIR,
    osl_File_E_ISDIR,
    osl_File_E_INVAL,
    osl_File_E_NFILE,
    osl_File_E_MFILE,
    osl_File_E_NOTTY,
    osl_File_E_FBIG,
    osl_File_E_NOSPC,
    osl_File_E_SPIPE,
    osl_File_E_ROFS,
    osl_File_E_MLINK,
    osl_File_E_PIPE,
    osl_File_E_DOM,
    osl_File_E_RANGE,
    osl_File_E_DEADLK,
    osl_File_E_NAMETOOLONG,
    osl_File_E_NOLCK,
    osl_File_E_NOSYS,
    osl_File_E_NOTEMPTY,
    osl_File_E_LOOP,
    osl_File_E_ILSEQ,
    osl_File_E_NOLINK,
    osl_File_E_MULTIHOP,
    osl_File_E_USERS,
    osl_File_E_OVERFLOW,
    osl_File_E_invalidError,        /* unmapped error: always last entry in enum! */
    osl_File_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslFileError;

typedef void *oslDirectory;
typedef void *oslDirectoryItem;

/** Opens a directory for enumerating its contents.

    @param  strDirenctoryPath [in] Denotes the full qualified path of the directory follwing the UNC
    notation. The path devider is '/'. Relative path specifications are not allowed.
    @param  pDirectory [out] on success it receives a handle used for subsequent calls by <code>osl_getNextDirectoryItem</code>
    on error it receives NULL. The handle has to be released by a call to <code>osl_closeDirectory</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOENT        the specified path doesn't exist<br>
    osl_File_E_NOTDIR       the specified path is not an directory <br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_ACCES        permission denied<br>
    osl_File_E_MFILE        too many open files used by the process<br>
    osl_File_E_NFILE        too many open files in the system<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_LOOP         Too many symbolic links encountered<p>

    @see osl_getNextDirectoryItem
    @see osl_closeDirectory
*/

oslFileError SAL_CALL osl_openDirectory( rtl_uString *strDirectoryPath, oslDirectory *pDirectory);

/** Retrieves the next item(s) of a previously openend directory. All handles have an initial
    refcount of 1.

    @param  Directory [in] is the directory handle received from a previous call to <code>osl_openDirectory</code>.
    @param  pItem [out] On success it receives a handle that can be used for subsequent calls to <code>osl_getFileStatus</code>.
    The handle has to be released by a call to <code>osl_releaseDirectoryItem</code>.
    @param  uHint [in] With this parameter the caller can tell the implementation that (s)he
    is going to call this function uHint times afterwards. This enables the implementation to
    get the information for more than one file and cache it until the next calls.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_NOENT        No more entries in this directory<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_BADF         oslDirectory parameter is not valid<br>
    osl_File_E_OVERFLOW     Value too large for defined data type<p>

    @see osl_releaseDirectoryItem
    @see osl_acquireDirectoryItem
    @see osl_getDirectoryItem
    @see osl_getFileStatus
    @see osl_createDirectoryItemFromHandle
*/

oslFileError SAL_CALL osl_getNextDirectoryItem(
    oslDirectory Directory,
    oslDirectoryItem *pItem,
    sal_uInt32   uHint
    );


/** Releases a directory handle

    @param  Directory [in] a handle received by a call to <code>osl_openDirectory</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_BADF         Invalid oslDirectory parameter<br>
    osl_File_E_INTR         function call was interrupted<p>

    @see osl_openDirectory
*/

oslFileError SAL_CALL osl_closeDirectory(
    oslDirectory Directory
    );


/** Retrieves a single directory item handle with an initial refcount of 1.

    @param  strFilePath [in] absolute file path following the notation explained in the documentation for
    <code>osl_openDirectory</code>. Due to performance issues it is not recommended to use this function
    while enumerating the contents of a directory. In this case use <code>osl_getNextDirectoryItem</code> instead.
    @param  pItem [out] on success it receives a handle which can be used for subsequent calls to <code>osl_getFileStatus</code>.
    The handle has to be released by a call to <code>osl_releaseDirectoryItem</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_ACCES        permission denied<br>
    osl_File_E_MFILE        too many open files used by the process<br>
    osl_File_E_NFILE        too many open files in the system<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOTDIR       A component of the path prefix of path is not a directory<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_IO           I/O error<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_INTR         function call was interrupted<p>


    @see osl_releaseDirectoryItem
    @see osl_acquireDirectoryItem
    @see osl_getFileStatus
    @see osl_getNextDirectoryItem
    @see osl_createDirectoryItemFromHandle
*/

oslFileError SAL_CALL osl_getDirectoryItem(
    rtl_uString *strFilePath,
    oslDirectoryItem *pItem
    );


/** Increases the refcount of a directory item handle

    @param  Item [in] a handle received by a call to <code>osl_getDirectoryItem</code>, <code>osl_getNextDirectoryItem</code> or
    <code>osl_createDirectoryItemFromHandle</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<p>

    @see osl_getDirectoryItem
    @see osl_getNextDirectoryItem
    @see osl_createDirectoryItemFromHandle
    @see osl_releaseDirectoryItem
*/

oslFileError SAL_CALL osl_acquireDirectoryItem( oslDirectoryItem Item );

/** Decreases the refcount of a directory item handle and releases the data if the
    refcount reaches 0.

    @param  Item [in] a handle received by a call to <code>osl_getDirectoryItem</code>, <code>osl_getNextDirectoryItem</code> or
    <code>osl_createDirectoryItemFromHandle</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<p>

    @see osl_getDirectoryItem
    @see osl_getNextDirectoryItem
    @see osl_acquireDirectoryItemosl_createDirectoryItemFromHandle
    @see osl_acquireDirectoryItem
*/

oslFileError SAL_CALL osl_releaseDirectoryItem( oslDirectoryItem Item );

/* File types */

typedef enum {
    osl_File_Type_Directory,
    osl_File_Type_Volume,
    osl_File_Type_Regular,
    osl_File_Type_Fifo,
    osl_File_Type_Socket,
    osl_File_Type_Link,
    osl_File_Type_Special,
    osl_File_Type_Unknown
} oslFileType;

/* File attributes */
#define osl_File_Attribute_ReadOnly             0x00000001
#define osl_File_Attribute_Hidden               0x00000002
#define osl_File_Attribute_Executable           0x00000010
#define osl_File_Attribute_GrpWrite             0x00000020
#define osl_File_Attribute_GrpRead              0x00000040
#define osl_File_Attribute_GrpExe               0x00000080
#define osl_File_Attribute_OwnWrite             0x00000100
#define osl_File_Attribute_OwnRead              0x00000200
#define osl_File_Attribute_OwnExe               0x00000400
#define osl_File_Attribute_OthWrite             0x00000800
#define osl_File_Attribute_OthRead              0x00001000
#define osl_File_Attribute_OthExe               0x00002000

/* Flags specifying which fields to retreive by osl_getFileStatus */

#define osl_FileStatus_Mask_Type                0x00000001
#define osl_FileStatus_Mask_Attributes          0x00000002
#define osl_FileStatus_Mask_CreationTime        0x00000010
#define osl_FileStatus_Mask_AccessTime          0x00000020
#define osl_FileStatus_Mask_ModifyTime          0x00000040
#define osl_FileStatus_Mask_FileSize            0x00000080
#define osl_FileStatus_Mask_FileName            0x00000100
#define osl_FileStatus_Mask_FilePath            0x00000200
#define osl_FileStatus_Mask_NativePath          0x00000400
#define osl_FileStatus_Mask_All                 0x7FFFFFFF
#define osl_FileStatus_Mask_Validate            0x80000000


typedef

/** Structure containing information about files and directories

    @see    osl_getFileStatus
    @see    oslFileType
*/

struct _oslFileStatus {
/** Must be initialized with the size in bytes of the structure before passing it to any function */
    sal_uInt32      uStructSize;
/** Determines which members of the structure contain valid data */
    sal_uInt32      uValidFields;
/** The type of the file (file, directory, volume). */
    oslFileType eType;
/** File attributes */
    sal_uInt64  uAttributes;
/** First creation time in nanoseconds since 1/1/1970. Can be the last modify time depending on
    platform or file system. */
    TimeValue   aCreationTime;
/** Last access time in nanoseconds since 1/1/1970. Can be the last modify time depending on
    platform or file system. */
    TimeValue   aAccessTime;
/** Last modify time in nanoseconds since 1/1/1970. */
    TimeValue   aModifyTime;
/** Size in bytes of the file. Zero for directories and volumes. */
    sal_uInt64  uFileSize;
/** Case correct name of the file. Should be set to zero before calling <code>osl_getFileStatus</code>
    and released after usage. */
    rtl_uString *strFileName;
/** Full path of the file in UNC notation. Should be set to zero before calling <code>osl_getFileStatus</code>
    and released after usage. */
    rtl_uString **pstrFilePath;
/** Full path of the file in platform depending notation. This is for displaying the path in the UI.
    Should be set to zero before calling <code>osl_getFileStatus</code>
    and released after usage. */
    rtl_uString **pstrNativePath;
} oslFileStatus;

/** Retrieves information about a single file or directory

    @param  Item [in] a handle received by a previous call to <code>osl_getDirectoryItem</code>,
    <code>osl_getNextDirectoryItem</code> or <code>osl_createDirectoryItemFromHandle</code>.
    @param  pStatus [in/out] points to a structure which receives the information of the file or directory
    represented by the handle <code>Item</code>. The member <code>uStructSize</code> has to be initialized to
    <code>sizeof(oslFileStatus)</code> before calling this function.
    @param  uFieldMask [in] specifies which fields of the structure pointed to by <code>pStatus</code>
    are of interest to the caller.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_NAMETOOLONG  file name too long<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_BADF         Invalid oslDirectoryItem parameter<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_OVERFLOW     Value too large for defined data type<br>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_MULTIHOP     Components of path require hopping to multiple remote machines and the file system does not allow it<br>
    osl_File_E_MFILE        too many open files used by the process<br>
    osl_File_E_NFILE        too many open files in the system<br>
    osl_File_E_NOSPC        No space left on device<br>
    osl_File_E_NXIO         No such device or address<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_NOSYS        Function not implemented<p>

    @see osl_getDirectoryItem
    @see osl_getNextDirectoryItem
    @see osl_createDirectoryItemFromHandle
    @see oslFileStatus
*/

oslFileError SAL_CALL osl_getFileStatus( oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask );


typedef void *oslVolumeDeviceHandle;


oslFileError SAL_CALL osl_unmountVolumeDevice( oslVolumeDeviceHandle Handle );

oslFileError SAL_CALL osl_automountVolumeDevice( oslVolumeDeviceHandle Handle );

oslFileError SAL_CALL osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle );

oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle );

/** Gets the normalized absolute file system path where a device is mounted to.

    @param  Handle  [in]    Device handle of the volume retrieved with <code>osl_getVolumeInformation</code>.
    @param  pstrPath [out] Receives the path where the device is mounted to.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_ACCES        permission denied<br>
    osl_File_E_NXIO         No such device or address<br>
    osl_File_E_NODEV        No such device<br>
    osl_File_E_NOENT        No such file or directory<br>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_FAULT        Bad address<br>
    osl_FilE_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_EOVERFLOW    Value too large for defined data type<p>

    @see    osl_getVolumeInformation
    @see    osl_automountVolumeDevice
    @see    osl_unmountVolumeDevice
*/

oslFileError SAL_CALL osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath );

/* Volume attributes */

#define osl_Volume_Attribute_Removeable 0x00000001L
#define osl_Volume_Attribute_Remote     0x00000002L

/* Flags specifying which fields to retreive by osl_getVolumeInfo */

#define osl_VolumeInfo_Mask_Attributes      0x00000001L
#define osl_VolumeInfo_Mask_TotalSpace      0x00000002L
#define osl_VolumeInfo_Mask_UsedSpace       0x00000004L
#define osl_VolumeInfo_Mask_FreeSpace       0x00000008L
#define osl_VolumeInfo_Mask_MaxNameLength   0x00000010L
#define osl_VolumeInfo_Mask_MaxPathLength   0x00000020L
#define osl_VolumeInfo_Mask_FileSystemName  0x00000040L
#define osl_VolumeInfo_Mask_DeviceHandle    0x00000080L

typedef

/** Structure containing information about volumes

    @see    osl_getVolumeInformation
    @see    oslFileType
*/

struct _oslVolumeInfo {
/** Must be initialized with the size in bytes of the structure before passing it to any function */
    sal_uInt32      uStructSize;
/** Determines which members of the structure contain valid data */
    sal_uInt32      uValidFields;
/** Attributes of the volume (remote and/or removable) */
    sal_uInt32      uAttributes;
/** Total availiable space on the volume for the current process/user */
    sal_uInt64      uTotalSpace;
/** Used space on the volume for the current process/user */
    sal_uInt64      uUsedSpace;
/** Free space on the volume for the current process/user */
    sal_uInt64      uFreeSpace;
/** Maximum length of file name of a single item */
    sal_uInt32      uMaxNameLength;
/** Maximum length of a full quallified path in UNC notation */
    sal_uInt32      uMaxPathLength;
/** Name of the file system type. Should be set to zero before calling <code>osl_getVolumeInformation</code>
    and released after usage. */
    rtl_uString     **pstrFileSystemName;
/** Handle of underlying device */
    oslVolumeDeviceHandle   *pDeviceHandle;
} oslVolumeInfo;


/** Retrieves information about a volume. A volume can either be a mount point, a network
    resource or a drive depending on operating system and file system. Before calling this
    function <code>osl_getFileStatus</code> should be called to determine if the type is
    <code>osl_file_Type_Volume</code>.

    @param  strDirectory [in]   Full qualified UNC path to the volume
    @param  pInfo [out] On success it receives information about the volume.
    @param  uFieldMask [in] Specifies which members of the structure should be filled
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_ACCES        permission denied<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    ols_File_E_FAULT        Bad address<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_NOSYS        Function not implemented<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_INTR         function call was interrupted<p>

    @see    osl_getFileStatus
    @see    oslVolumeInfo
*/

oslFileError SAL_CALL osl_getVolumeInformation( rtl_uString *strDirectory, oslVolumeInfo *pInfo, sal_uInt32 uFieldMask );

typedef void *oslFileHandle;

/* Open flags */

#define osl_File_OpenFlag_Read      0x00000001L
#define osl_File_OpenFlag_Write     0x00000002L
#define osl_File_OpenFlag_Create    0x00000004L

/** Opens a file.

    @param  strPath [in] Full qualified path to the file to open. Only regular files
    can be openend.
    @param  pHandle [out] On success it receives a handle to the open file.
    @param  uFlags [in] Specifies the open mode.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NAMETOOLONG  pathname was too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_ACCES        permission denied<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_NXIO         No such device or address<br>
    osl_File_E_NODEV        No such device<br>
    osl_File_E_ROFS         Read-only file system<br>
    osl_File_E_TXTBSY       Text file busy<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_NOSPC        No space left on device<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_MFILE        too many open files used by the process<br>
    osl_File_E_NFILE        too many open files in the system<br>
    osl_File_E_DQUOT        Quota exceeded<br>
    osl_File_E_EXIST        File exists<br>
    osl_FilE_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_EOVERFLOW    Value too large for defined data type<p>

    @see    osl_closeFile
    @see    osl_setFilePos
    @see    osl_getFilePos
    @see    osl_readFile
    @see    osl_writeFile
    @see    osl_setFileSize
    @see    osl_createDirectoryItemFromHandle
*/

oslFileError SAL_CALL osl_openFile( rtl_uString *strPath, oslFileHandle *pHandle, sal_uInt32 uFlags );

#define osl_Pos_Absolut 1
#define osl_Pos_Current 2
#define osl_Pos_End     3

/** Sets the internal position pointer of an open file.
    @param  Handle [in] Handle of an open file received by a previous call to <code>osl_openFile</code>.
    @param  uHow [in] Distance to move the internal position pointer (from uPos).
    @param  uPos [in] Absolute position from the beginning of the file.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW     The resulting file offset would be a value which cannot
    be represented correctly for regular files<p>

    @see    osl_openFile
    @see    osl_getFilePos
*/

oslFileError SAL_CALL osl_setFilePos( oslFileHandle Handle, sal_uInt32 uHow, sal_uInt64 uPos );


/** Retrieves the current position of the internal pointer of an open file.
    @param  Handle [in] Handle to an open file.
    @param  pPos [out] On Success it receives the current position of the file pointer.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW     The resulting file offset would be a value which cannot
    be represented correctly for regular files<p>

    @see    osl_openFile
    @see    osl_setFilePos
    @see    osl_readFile
    @see    osl_writeFile
*/

oslFileError SAL_CALL osl_getFilePos( oslFileHandle Handle, sal_uInt64 *pPos );


/** Sets the file size of an open file. The file can be truncated or enlarged by the function.
    The position of the file pointer is not affeced by this function.
    @param  Handle [in] Handle to an open file.
    @param  uSize [int] New size in bytes.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW     The resulting file offset would be a value which cannot
    be represented correctly for regular files<p>

    @see    osl_openFile
    @see    osl_setFilePos
    @see    osl_getFileStatus
*/

oslFileError SAL_CALL osl_setFileSize( oslFileHandle Handle, sal_uInt64 uSize );


/** Reads a number of bytes from a file. The internal file pointer is increased by the number of bytes
    read.
    @param  Handle [in] Handle to an open file.
    @param  pBuffer [out] Points to a buffer which receives data. The buffer must be large enough
    to hold <code>uBytesRequested</code> bytes.
    @param  uBytesRequested [in] Number of bytes which should be retrieved.
    @param  pBytesRead [out] On success the number of bytes which have actually been retrieved.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_BADF         Bad file<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_AGAIN        Operation would block<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see    osl_openFile
    @see    osl_writeFile
    @see    osl_readLine
    @see    osl_setFilePos
*/

oslFileError SAL_CALL osl_readFile( oslFileHandle Handle, void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64 *pBytesRead );


/** Writes a number of bytes to a file. The internal file pointer is increased by the number of bytes
    read.
    @param  Handle [in] Handle to an open file.
    @param  pBuffer [in] Points to a buffer which contains the data.
    @param  uBytesToWrite [in] Number of bytes which should be written.
    @param  pBytesWritten [out] On success the number of bytes which have actually been written.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_FBIG         File too large<br>
    osl_File_E_DQUOT        Quota exceeded<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_AGAIN        Operation would block<br>
    osl_File_E_BADF         Bad file<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_NOLCK        No record locks available<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_NOSPC        No space left on device<br>
    osl_File_E_NXIO         No such device or address<p>

    @see    osl_openFile
    @see    osl_readFile
    @see    osl_setFilePos
*/

oslFileError SAL_CALL osl_writeFile( oslFileHandle Handle, const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64 *pBytesWritten );

/** Reads a line from given file. The new line delemeter(s) are NOT returned!

    @param  Handle [in] Handle to an open file.
    @param  ppSequence [in/out] a pointer to a valid sequence. Will hold the line read on return.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_BADF         Bad file<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_AGAIN        Operation would block<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see    osl_openFile
    @see    osl_readFile
    @see    osl_writeFile
    @see    osl_setFilePos
*/

oslFileError SAL_CALL osl_readLine( oslFileHandle Handle, sal_Sequence** ppSequence );


/** Closes an open file.
    @param  Handle [in] Handle to a file previously open by a call to <code>osl_openFile</code>.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_BADF         Bad file<br>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_NOSPC        No space left on device<br>
    osl_File_E_IO           I/O error<p>
*/

oslFileError SAL_CALL osl_closeFile( oslFileHandle Handle );


/** Get a directory item handle from an open file with an initial refcount of 1.
    @param  Handle [in] A Hdnale to an open file.
    @param  pItem [out] On success it receives a handle to a directory item which can be used
    in subsequent calls to <code>osl_getFileStatus</code>.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<p>

    @see    osl_openFile
    @see    osl_getFileStatus
    @see    osl_acquireDirectoryItem
    @see    osl_releaseDirectoryItem
*/

oslFileError SAL_CALL osl_createDirectoryItemFromHandle( oslFileHandle Handle, oslDirectoryItem *pItem );


/** Creates a directory.
    @param  strPatg [in] Full qualified UNC path of the directory to create.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_EXIST        File exists<br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_ROFS         Read-only file system<br>
    osl_File_E_NOSPC        No space left on device<br>
    osl_File_E_DQUOT        Quota exceeded<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_FAULT        Bad address<br>
    osl_FileE_IO            I/O error<br>
    osl_File_E_MLINK        Too many links<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see    osl_removeDirectory
*/

oslFileError SAL_CALL osl_createDirectory( rtl_uString* strPath );

/** Removes an empty directory.

    @param  strPath [in] Full qualified UNC path of the directory.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_PERM         Operation not permitted<br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_NOTEMPTY     Directory not empty<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_BUSY         Device or resource busy<br>
    osl_File_E_ROFS         Read-only file system<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_BUSY         Device or resource busy<br>
    osl_File_E_EXIST        File exists<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see osl_createDirectory
*/

oslFileError SAL_CALL osl_removeDirectory( rtl_uString* strPath );


/** Removes (erases) a regular file.

    @param  strPath [in] Full qualified UNC path of the directory.
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_PERM         Operation not permitted<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_ROFS         Read-only file system<p>

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_BUSY         Device or resource busy<br>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<br>
    osl_File_E_TXTBSY       Text file busy<p>
*/

oslFileError SAL_CALL osl_removeFile( rtl_uString* strPath );


/** Copies a file to a new destination. Copies only files not directories. No assumptions should
    be made about preserving attributes or file time.
    @param  strPath [in] Full qualified UNC path of the source file.
    @param  strDestPath [in] Full qualified UNC path of the destination file. A directory is
    NOT a valid destination file !
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_PERM         Operation not permitted<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_ROFS         Read-only file system<p>

    @see    osl_moveFile
    @see    osl_removeFile
    @see    osl_createDirectory
*/

oslFileError SAL_CALL osl_copyFile( rtl_uString* strPath, rtl_uString *strDestPath );


/** Moves a file or directory to a new destination or renames it. File time and attributes
    are preserved.
    @param  strPath [in] Full qualified UNC path of the source file.
    @param  strDestPath [in] Full qualified UNC path of the destination file. An existing directory
    is NOT a valid destination !
    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_PERM         Operation not permitted<br>
    osl_File_E_NAMETOOLONG  File name too long<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_ROFS         Read-only file system<p>

    @see    osl_copyFile
*/

oslFileError SAL_CALL osl_moveFile( rtl_uString* strPath, rtl_uString *strDestPath );


/** Determines a valid unused canonical name for a requested name. Depending on file system and operation system
    the illegal characters are replaced by valid ones. If a file or directory with the requested name
    already exists a new name is generated following the common rules on the actual file system and
    operating system.

    @param  strRequested [in] Requested name of a file or directory.
    @param  strValid [out] On success receives a name which is unused and valid on the actual operating system and
    file system.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    @see    osl_getFileStatus
*/

oslFileError SAL_CALL osl_getCanonicalName( rtl_uString *strRequested, rtl_uString **strValid);


/** Converts a path relative to a given directory into an full qualified UNC path.

    @param  strDirBase [in] Base directory to which the relative path is related to.
    @param  strRelative[in] Path of a file or directory relative to the directory path
    specified by <code>strDirBase</code>.
    @param  strAbsolute [out] On success it receives the full qualified UNC path of the
    requested relative path.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        not enough memory for allocating structures <br>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_ACCES        Permission denied<br>
    osl_File_E_NOENT        No such file or directory<br>
    osl_File_E_NAMETOOLONG  File name too long<p>


    These errorcodes can (eventually) be returned:<p>
    osl_File_E_OVERFLOW     Value too large for defined data type<p>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_LOOP         Too many symbolic links encountered<br>
    osl_File_E_MULTIHOP     Multihop attempted<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see    osl_getFileStatus
*/


oslFileError SAL_CALL osl_getAbsolutePath( rtl_uString* strDirBase, rtl_uString *strRelative, rtl_uString **strAbsolute );


/** Converts a system dependent path into a full qualified UNC path

    @param strSysPath[in] System dependent path of a file or a directory
    @param strPath[out] On success it receives the full qualified UNC path

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    @see    osl_getCanonicalName

*/

oslFileError SAL_CALL osl_normalizePath( rtl_uString *strSysPath, rtl_uString **strPath );


/** Converts a File URL into a full qualified UNC path

    @param urlPath[in] System dependent path of a file or a directory
    @param strPath[out] On success it receives the full qualified UNC path

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    @see    osl_normalizePath

*/

oslFileError SAL_CALL osl_getNormalizedPathFromFileURL( rtl_uString *urlPath, rtl_uString **strPath );


/** Converts a full qualified UNC path into a FileURL

    @param dir[in] System dependent path of a file or a directory
    @param strPath[out] On success it receives the full qualified UNC path

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    @see    osl_normalizePath
    @see    osl_getNormalizedPathFromFileURL

*/

oslFileError SAL_CALL osl_getFileURLFromNormalizedPath( rtl_uString *normPath, rtl_uString **strPath);


/** Searches a full qualified UNC-Path/File

    @param filePath[in] System dependent path / Normalized Path / File-URL or file or relative directory
    @param searchPath[in] Paths, in which a given file has to be searched. These paths are only for the search of a
    file or a relative path, otherwise it will be ignored. If it is set to NULL or while using the
    search path the search failed the function searches for a matching file in all system directories and in the directories
    listed in the PATH environment variable
    @param strPath[out] On success it receives the full qualified UNC path

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOTDIR       Not a directory<br>
    osl_File_E_NOENT        No such file or directory not found<br>
    @see    osl_normalizePath

*/

oslFileError SAL_CALL osl_searchNormalizedPath( rtl_uString *filePath, rtl_uString *searchPath, rtl_uString **strPath );


/** Converts a full qualified UNC path into a system dependend path

    @param dir[in] Full qualified UNC path
    @param strPath[out] On success it receives the system dependent path of a file or a directory

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>

    @see    osl_normalizePath
    @see    osl_getNormalizedPathFromFileURL

*/

oslFileError SAL_CALL osl_getSystemPathFromNormalizedPath( rtl_uString *normPath, rtl_uString **strPath);


/** Sets file-attributes

    @param filePath[in] Path of the file
    @param uAttributes[in] Attributes of the file to be set

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    @see    osl_getFileStatus

*/

oslFileError SAL_CALL osl_setFileAttributes( rtl_uString *filePath, sal_uInt64 uAttributes );


/** Sets file-Time

    @param filePath[in] Path of the file
    @param aCreationTime[in] creation time of the given file
    @param aLastAccessTime[in]  time of the last access of the given file
    @param aLastWriteTime[in]  time of the last modifying of the given file


    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>
    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOENT        No such file or directory not found<br>
    @see    osl_getFileStatus

*/

oslFileError SAL_CALL osl_setFileTime( rtl_uString *filePath,   TimeValue *aCreationTime,
                                                                TimeValue *aLastAccessTime,
                                                                TimeValue *aLastWriteTime);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_FILE_H_ */


