/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _OSL_FILE_H_
#define _OSL_FILE_H_

#include "sal/config.h"

#include "osl/time.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @file

Main goals and usage hints

The main intentention of this interface is to provide an universal portable and
high performance access to file system issues on any operating system.<p>

There are a few main goals:<p>

1.The path specifications always has to be absolut. Any usage of relative path
specifications is forbidden. Exceptions are <code>osl_getSystemPathFromFileURL</code>,
<code>osl_getFileURLFromSystemPath</code> and <code>osl_getAbsoluteFileURL</code>. Most operating systems
provide a "Current Directory" per process. This is the reason why relative path
specifications can cause problems in multithreading environments.<p>

2.Proprietary notations of file paths are not supported. Every path notation
must the file URL specification. File URLs must be encoded in UTF8 and
after that escaped. Although the URL parameter is a unicode string, the must
contain only ASCII characters<p>

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
    osl_File_E_NOTREADY,
    osl_File_E_invalidError,        /* unmapped error: always last entry in enum! */
    osl_File_E_TIMEDOUT,
    osl_File_E_NETWORK,
    osl_File_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslFileError;

typedef void *oslDirectory;
typedef void *oslDirectoryItem;


/** Open a directory for enumerating its contents.

    @param  pustrDirectoryURL [in]
    The full qualified URL of the directory.

    @param  pDirectory [out]
    On success it receives a handle used for subsequent calls by osl_getNextDirectoryItem().
    The handle has to be released by a call to osl_closeDirectory().

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOENT the specified path doesn't exist<br>
    osl_File_E_NOTDIR the specified path is not an directory <br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_MFILE too many open files used by the process<br>
    osl_File_E_NFILE too many open files in the system<br>
    osl_File_E_NAMETOOLONG File name too long<br>
    osl_File_E_LOOP Too many symbolic links encountered<p>

    @see osl_getNextDirectoryItem()
    @see osl_closeDirectory()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_openDirectory(
        rtl_uString *pustrDirectoryURL, oslDirectory *pDirectory);


/** Retrieve the next item of a previously opened directory.

    Retrieves the next item of a previously opened directory.
    All handles have an initial refcount of 1.

    @param  Directory [in]
    A directory handle received from a previous call to osl_openDirectory().

    @param  pItem [out]
    On success it receives a handle that can be used for subsequent calls to osl_getFileStatus().
    The handle has to be released by a call to osl_releaseDirectoryItem().

    @param  uHint [in]
    With this parameter the caller can tell the implementation that (s)he
    is going to call this function uHint times afterwards. This enables the implementation to
    get the information for more than one file and cache it until the next calls.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_NOENT no more entries in this directory<br>
    osl_File_E_BADF invalid oslDirectory parameter<br>
    osl_File_E_OVERFLOW the value too large for defined data type

    @see osl_releaseDirectoryItem()
    @see osl_acquireDirectoryItem()
    @see osl_getDirectoryItem()
    @see osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getNextDirectoryItem(
    oslDirectory Directory,
    oslDirectoryItem *pItem,
    sal_uInt32   uHint
    );


/** Release a directory handle.

    @param Directory [in]
    A handle received by a call to osl_openDirectory().

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures<br>
    osl_File_E_BADF invalid oslDirectory parameter<br>
    osl_File_E_INTR the function call was interrupted<p>

    @see osl_openDirectory()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_closeDirectory(
        oslDirectory Directory);


/** Retrieve a single directory item.

    Retrieves a single directory item. The returned handle has an initial refcount of 1.
    Due to performance issues it is not recommended to use this function while
    enumerating the contents of a directory. In this case use osl_getNextDirectoryItem() instead.

    @param pustrFileURL [in]
    An absolute file URL.

    @param pItem [out]
    On success it receives a handle which can be used for subsequent calls to osl_getFileStatus().
    The handle has to be released by a call to osl_releaseDirectoryItem().

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_MFILE too many open files used by the process<br>
    osl_File_E_NFILE too many open files in the system<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_NAMETOOLONG the file name is too long<br>
    osl_File_E_NOTDIR a component of the path prefix of path is not a directory<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_INTR the function call was interrupted<p>

    @see osl_releaseDirectoryItem()
    @see osl_acquireDirectoryItem()
    @see osl_getFileStatus()
    @see osl_getNextDirectoryItem()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getDirectoryItem(
    rtl_uString *pustrFileURL,
    oslDirectoryItem *pItem
    );


/** Increase the refcount of a directory item handle.

    The caller responsible for releasing the directory item handle using osl_releaseDirectoryItem().

    @param  Item [in]
    A handle received by a call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>

    @see osl_getDirectoryItem()
    @see osl_getNextDirectoryItem()
    @see osl_releaseDirectoryItem()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_acquireDirectoryItem(
        oslDirectoryItem Item );


/** Decrease the refcount of a directory item handle.

    Decreases the refcount of a directory item handle.
    If the refcount reaches 0 the data associated with
    this directory item handle will be released.

    @param  Item [in]
    A handle received by a call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>

    @see osl_getDirectoryItem()
    @see osl_getNextDirectoryItem()
    @see osl_acquireDirectoryItem()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_releaseDirectoryItem(
        oslDirectoryItem Item );

/** Determine if two directory items point the same underlying file

    The comparison is done first by URL, and then by resolving links to
    find the target, and finally by comparing inodes on unix.

    @param  pItemA [in]
    A directory handle to compare with another handle

    @param  pItemB [in]
    A directory handle to compare with pItemA

    @return
    sal_True: if the items point to an identical resource<br>
    sal_False: if the items point to a different resource, or a fatal error occured<br>

    @see osl_getDirectoryItem()

    @since LibreOffice 3.6
*/

SAL_DLLPUBLIC sal_Bool SAL_CALL osl_identicalDirectoryItem(
                                        oslDirectoryItem pItemA,
                                        oslDirectoryItem pItemB );

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

/* Flags specifying which fields to retrieve by osl_getFileStatus */

#define osl_FileStatus_Mask_Type                0x00000001
#define osl_FileStatus_Mask_Attributes          0x00000002
#define osl_FileStatus_Mask_CreationTime        0x00000010
#define osl_FileStatus_Mask_AccessTime          0x00000020
#define osl_FileStatus_Mask_ModifyTime          0x00000040
#define osl_FileStatus_Mask_FileSize            0x00000080
#define osl_FileStatus_Mask_FileName            0x00000100
#define osl_FileStatus_Mask_FileURL             0x00000200
#define osl_FileStatus_Mask_LinkTargetURL       0x00000400
#define osl_FileStatus_Mask_All                 0x7FFFFFFF
#define osl_FileStatus_Mask_Validate            0x80000000


typedef

/** Structure containing information about files and directories

    @see    osl_getFileStatus()
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
    rtl_uString *ustrFileName;
/** Full URL of the file. Should be set to zero before calling <code>osl_getFileStatus</code>
    and released after usage. */
    rtl_uString *ustrFileURL;
/** Full URL of the target file if the file itself is a link.
    Should be set to zero before calling <code>osl_getFileStatus</code>
    and released after usage. */
    rtl_uString *ustrLinkTargetURL;
} oslFileStatus;


/** Retrieve information about a single file or directory.

    @param  Item [in]
    A handle received by a previous call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @param  pStatus [in|out]
    Points to a structure which receives the information of the file or directory
    represented by the handle Item. The member uStructSize has to be initialized to
    sizeof(oslFileStatus) before calling this function.

    @param  uFieldMask [in]
    Specifies which fields of the structure pointed to by pStatus are of interest to the caller.

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_BADF invalid oslDirectoryItem parameter<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_OVERFLOW value too large for defined data type<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_MULTIHOP components of path require hopping to multiple remote machines and the file system does not allow it<br>
    osl_File_E_MFILE too many open files used by the process<br>
    osl_File_E_NFILE too many open files in the system<br>
    osl_File_E_NOSPC no space left on device<br>
    osl_File_E_NXIO no such device or address<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_NOSYS function not implemented<p>

    @see osl_getDirectoryItem()
    @see osl_getNextDirectoryItem()
    @see oslFileStatus
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileStatus(
        oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask );


typedef void *oslVolumeDeviceHandle;

/** Release a volume device handle.

    Releases the given oslVolumeDeviceHandle which was acquired by a call to
    osl_getVolumeInformation() or osl_acquireVolumeDeviceHandle().

    @param Handle [in]
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @return
    osl_File_E_None on success<br>

    @todo
    specify all error codes that may be returned

    @see osl_acquireVolumeDeviceHandle()
    @see osl_getVolumeInformation()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_releaseVolumeDeviceHandle(
        oslVolumeDeviceHandle Handle );

/** Acquire a volume device handle.

    Acquires the given oslVolumeDeviceHandle which was acquired by a call to
    osl_getVolumeInformation(). The caller is responsible for releasing the
    acquired handle by calling osl_releaseVolumeDeviceHandle().

    @param Handle [in]
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @return
    osl_File_E_None on success<br>

    @todo
    specify all error codes that may be returned

    @see osl_getVolumeInformation()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_acquireVolumeDeviceHandle(
        oslVolumeDeviceHandle Handle );


/** Get the full qualified URL where a device is mounted to.

    @param Handle [in]
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @param ppustrDirectoryURL [out]
    Receives the full qualified URL where the device is mounted to.

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_NXIO  no such device or address<br>
    osl_File_E_NODEV no such device<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_FAULT bad address<br>
    osl_FilE_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_EOVERFLOW value too large for defined data type<br>

    @see    osl_getVolumeInformation()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getVolumeDeviceMountPath(
        oslVolumeDeviceHandle Handle, rtl_uString **ppustrDirectoryURL);

/* Volume attributes */

#define osl_Volume_Attribute_Removeable            0x00000001L
#define osl_Volume_Attribute_Remote                0x00000002L
#define osl_Volume_Attribute_CompactDisc           0x00000004L
#define osl_Volume_Attribute_FixedDisk             0x00000008L
#define osl_Volume_Attribute_RAMDisk               0x00000010L
#define osl_Volume_Attribute_FloppyDisk            0x00000020L

#define osl_Volume_Attribute_Case_Is_Preserved     0x00000040L
#define osl_Volume_Attribute_Case_Sensitive        0x00000080L

/* Flags specifying which fields to retrieve by osl_getVolumeInfo */

#define osl_VolumeInfo_Mask_Attributes             0x00000001L
#define osl_VolumeInfo_Mask_TotalSpace             0x00000002L
#define osl_VolumeInfo_Mask_UsedSpace              0x00000004L
#define osl_VolumeInfo_Mask_FreeSpace              0x00000008L
#define osl_VolumeInfo_Mask_MaxNameLength          0x00000010L
#define osl_VolumeInfo_Mask_MaxPathLength          0x00000020L
#define osl_VolumeInfo_Mask_FileSystemName         0x00000040L
#define osl_VolumeInfo_Mask_DeviceHandle           0x00000080L
#define osl_VolumeInfo_Mask_FileSystemCaseHandling 0x00000100L

typedef

/** Structure containing information about volumes

    @see    osl_getVolumeInformation()
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
/** Maximum length of a full quallified path in system notation */
    sal_uInt32      uMaxPathLength;
/** Points to a string that receives the name of the file system type. String should be set to zero before calling <code>osl_getVolumeInformation</code>
    and released after usage. */
    rtl_uString     *ustrFileSystemName;
/** Pointer to handle the receives underlying device. Handle should be set to zero before calling <code>osl_getVolumeInformation</code>*/
    oslVolumeDeviceHandle   *pDeviceHandle;
} oslVolumeInfo;


/** Retrieve information about a volume.

    Retrieves information about a volume. A volume can either be a mount point, a network
    resource or a drive depending on Operating System and File System. Before calling this
    function osl_getFileStatus() should be called to determine if the type is
    osl_file_Type_Volume.

    @param pustrDirectoryURL [in]
    Full qualified URL of the volume

    @param pInfo [out]
    On success it receives information about the volume.

    @param uFieldMask [in]
    Specifies which members of the structure should be filled

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    ols_File_E_FAULT Bad address<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_NOSYS function not implemented<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_INTR function call was interrupted<br>

    @see    osl_getFileStatus()
    @see    oslVolumeInfo
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getVolumeInformation(
    rtl_uString *pustrDirectoryURL,
    oslVolumeInfo *pInfo,
    sal_uInt32 uFieldMask );

typedef void *oslFileHandle;

/* Open flags */

#define osl_File_OpenFlag_Read      0x00000001L
#define osl_File_OpenFlag_Write     0x00000002L
#define osl_File_OpenFlag_Create    0x00000004L
#define osl_File_OpenFlag_NoLock    0x00000008L
/* larger bit-fields reserved for internal use cf. detail/file.h */

/** Open a regular file.

    Open a file. Only regular files can be openend.

    @param pustrFileURL [in]
    The full qualified URL of the file to open.

    @param pHandle [out]
    On success it receives a handle to the open file.

    @param uFlags [in]
    Specifies the open mode.

    On Android, if the file path is below the /assets folder, the file
    exists only as a hopefully uncompressed element inside the app
    package (.apk), which has been mapped into memory as a whole by
    the LibreOffice Android bootstrapping code. So files "opened" from
    there aren't actually files in the OS sense.

    @return
    osl_File_E_None on success<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NAMETOOLONG pathname was too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_AGAIN a write lock could not be established<br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_NXIO no such device or address<br>
    osl_File_E_NODEV no such device<br>
    osl_File_E_ROFS read-only file system<br>
    osl_File_E_TXTBSY text file busy<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_NOSPC no space left on device<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_MFILE too many open files used by the process<br>
    osl_File_E_NFILE too many open files in the system<br>
    osl_File_E_DQUOT quota exceeded<br>
    osl_File_E_EXIST file exists<br>
    osl_FilE_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_EOVERFLOW value too large for defined data type<br>

    @see osl_closeFile()
    @see osl_setFilePos()
    @see osl_getFilePos()
    @see osl_readFile()
    @see osl_writeFile()
    @see osl_setFileSize()
    @see osl_getFileSize()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_openFile(
        rtl_uString *pustrFileURL, oslFileHandle *pHandle, sal_uInt32 uFlags );

#define osl_Pos_Absolut 1
#define osl_Pos_Current 2
#define osl_Pos_End     3

/** Set the internal position pointer of an open file.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param uHow [in]
    Distance to move the internal position pointer (from uPos).

    @param uPos [in]
    Absolute position from the beginning of the file.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files<br>

    @see    osl_openFile()
    @see    osl_getFilePos()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFilePos(
        oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uPos ) SAL_WARN_UNUSED_RESULT;


/** Retrieve the current position of the internal pointer of an open file.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param pPos [out]
    On success receives the current position of the file pointer.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files<br>

    @see osl_openFile()
    @see osl_setFilePos()
    @see osl_readFile()
    @see osl_writeFile()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFilePos(
        oslFileHandle Handle, sal_uInt64 *pPos );


/** Set the file size of an open file.

    Sets the file size of an open file. The file can be truncated or enlarged by the function.
    The position of the file pointer is not affeced by this function.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param uSize [in]
    New size in bytes.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files<br>

    @see osl_openFile()
    @see osl_setFilePos()
    @see osl_getFileStatus()
    @see osl_getFileSize()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFileSize(
        oslFileHandle Handle, sal_uInt64 uSize );


/** Get the file size of an open file.

    Gets the file size of an open file.
    The position of the file pointer is not affeced by this function.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param pSize [out]
    Current size in bytes.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files<br>

    @see osl_openFile()
    @see osl_setFilePos()
    @see osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileSize(
        oslFileHandle Handle, sal_uInt64 *pSize );


/** Map flags.

    @since UDK 3.2.10
 */
#define osl_File_MapFlag_RandomAccess ((sal_uInt32)(0x1))

/** Map flag denoting that the mapped address space will be accessed by the
    process soon (and it is advantageous for the operating system to already
    start paging in the data).

    @since UDK 3.2.12
 */
#define osl_File_MapFlag_WillNeed ((sal_uInt32)(0x2))

/** Map a shared file into memory.

    Don't know what the "shared" is supposed to mean there? Also,
    obviously this API can be used to map *part* of a file into
    memory, and different parts can be mapped separately even.

    On Android, if the Handle refers to a file that is actually inside
    the app package (.apk zip archive), no new mapping is created,
    just a pointer to the file inside the already mapped .apk is
    returned.

    @since UDK 3.2.10
 */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_mapFile (
  oslFileHandle Handle,
  void**        ppAddr,
  sal_uInt64    uLength,
  sal_uInt64    uOffset,
  sal_uInt32    uFlags
);


#ifndef ANDROID

/** Unmap a shared file from memory.

    Ditto here, why do we need to mention "shared"?

    This function just won't work on Android in general where for
    (uncompressed) files inside the .apk, per SDK conventions in the
    /assets folder, osl_mapFile() returns a pointer to the file inside
    the already by LibreOffice Android-specific bootstrapping code
    mmapped .apk archive. We can't go and randomly munmap part of the
    .apk archive. So this function is not present on Android.

    @since UDK 3.2.10
 */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_unmapFile (
  void*      pAddr,
  sal_uInt64 uLength
);

#endif

/** Unmap a file segment from memory.

    Like osl_unmapFile(), but takes also the oslFileHandle argument
    passed to osl_mapFile() when creating this mapping.

    On Android, for files below /assets, i.e. located inside the app
    archive (.apk), this won't actually unmap anything; all the .apk
    stays mapped.

    @since UDK 3.6
 */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_unmapMappedFile (
  oslFileHandle Handle,
  void*      pAddr,
  sal_uInt64 uLength
);


/** Read a number of bytes from a file.

    Reads a number of bytes from a file. The internal file pointer is
    increased by the number of bytes read.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param pBuffer [out]
    Points to a buffer which receives data. The buffer must be large enough
    to hold uBytesRequested bytes.

    @param uBytesRequested [in]
    Number of bytes which should be retrieved.

    @param pBytesRead [out]
    On success the number of bytes which have actually been retrieved.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_BADF bad file<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_AGAIN operation would block<br>
    osl_File_E_NOLINK link has been severed<br>

    @see osl_openFile()
    @see osl_writeFile()
    @see osl_readLine()
    @see osl_setFilePos()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_readFile(
        oslFileHandle Handle, void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64 *pBytesRead );


/** Test if the end of a file is reached.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param pIsEOF [out]
    Points to a variable that receives the end-of-file status.

    @return
    osl_File_E_None on success <br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_BADF bad file<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_AGAIN operation would block<br>
    osl_File_E_NOLINK link has been severed<p>

    @see osl_openFile()
    @see osl_readFile()
    @see osl_readLine()
    @see osl_setFilePos()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_isEndOfFile(
        oslFileHandle Handle, sal_Bool *pIsEOF );


/** Write a number of bytes to a file.

    Writes a number of bytes to a file.
    The internal file pointer is increased by the number of bytes read.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param pBuffer [in]
    Points to a buffer which contains the data.

    @param uBytesToWrite [in]
    Number of bytes which should be written.

    @param pBytesWritten [out]
    On success the number of bytes which have actually been written.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_FBIG file too large<br>
    osl_File_E_DQUOT quota exceeded<p>
    osl_File_E_AGAIN operation would block<br>
    osl_File_E_BADF bad file<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errosr<br>
    osl_File_E_NOLCK no record locks available<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_NOSPC no space left on device<br>
    osl_File_E_NXIO no such device or address<br>

    @see osl_openFile()
    @see osl_readFile()
    @see osl_setFilePos()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_writeFile(
        oslFileHandle Handle, const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64 *pBytesWritten );

/** Read a number of bytes from a specified offset in a file.

    The current position of the internal file pointer may or may not be changed.

    @since UDK 3.2.10
 */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_readFileAt(
  oslFileHandle Handle,
  sal_uInt64    uOffset,
  void*         pBuffer,
  sal_uInt64    uBytesRequested,
  sal_uInt64*   pBytesRead
);


/** Write a number of bytes to a specified offset in a file.

    The current position of the internal file pointer may or may not be changed.

    @since UDK 3.2.10
 */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_writeFileAt(
  oslFileHandle Handle,
  sal_uInt64    uOffset,
  const void*   pBuffer,
  sal_uInt64    uBytesToWrite,
  sal_uInt64*   pBytesWritten
);


/** Read a line from a file.

    Reads a line from a file. The new line delimiter is NOT returned!

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @param  ppSequence [in/out]
    A pointer pointer to a sal_Sequence that will hold the line read on success.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_BADF bad file<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_AGAIN operation would block<br>
    osl_File_E_NOLINK link has been severed<p>

    @see osl_openFile()
    @see osl_readFile()
    @see osl_writeFile()
    @see osl_setFilePos()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_readLine(
        oslFileHandle Handle, sal_Sequence** ppSequence );

/** Synchronize the memory representation of a file with that on the physical medium.

     The function ensures that all modified data and attributes of the file associated with
     the given file handle have been written to the physical medium.
     In case the hard disk has a write cache enabled, the data may not really be on
     permanent storage when osl_syncFile returns.

      @param Handle
      [in] Handle to a file received by a previous call to osl_openFile().

      @return
      <dl>
      <dt>osl_File_E_None</dt>
      <dd>On success</dd>
      <dt>osl_File_E_INVAL</dt>
      <dd>The value of the input parameter is invalid</dd>
      </dl>
      <br><p><strong>In addition to these error codes others may occur as well, for instance:</strong></p><br>
      <dl>
      <dt>osl_File_E_BADF</dt>
      <dd>The file associated with the given file handle is not open for writing</dd>
      <dt>osl_File_E_IO</dt>
      <dd>An I/O error occurred</dd>
      <dt>osl_File_E_NOSPC</dt>
      <dd>There is no enough space on the target device</dd>
      <dt>osl_File_E_ROFS</dt>
      <dd>The file associated with the given file handle is located on a read only file system</dd>
      <dt>osl_File_E_TIMEDOUT</dt>
      <dd>A remote connection timed out. This may happen when a file is on a remote location</dd>
      </dl>

      @see osl_openFile()
      @see osl_writeFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_syncFile( oslFileHandle Handle );

/** Close an open file.

    @param Handle [in]
    Handle to a file received by a previous call to osl_openFile().

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_BADF Bad file<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_NOSPC no space left on device<br>
    osl_File_E_IO on I/O errors<br>

    @see osl_openFile()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_closeFile( oslFileHandle Handle );


/** Create a directory.

    @param pustrDirectoryURL [in]
    Full qualified URL of the directory to create.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_EXIST file exists<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_ROFS read-only file system<br>
    osl_File_E_NOSPC no space left on device<br>
    osl_File_E_DQUOT quota exceeded<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_FAULT bad address<br>
    osl_FileE_IO on I/O errors<br>
    osl_File_E_MLINK too many links<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>

    @see osl_removeDirectory()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_createDirectory( rtl_uString* pustrDirectoryURL );


/** Remove an empty directory.

    @param pustrDirectoryURL [in]
    Full qualified URL of the directory.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_PERM operation not permitted<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_NOTEMPTY directory not empty<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_BUSY device or resource busy<br>
    osl_File_E_ROFS read-only file system<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_BUSY device or resource busy<br>
    osl_File_E_EXIST file exists<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>

    @see osl_createDirectory()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_removeDirectory( rtl_uString* pustrDirectoryURL );

/** Function pointer representing a function that will be called by osl_createDirectoryPath
    if a directory has been created.

    To avoid unpredictable results the callee must not access the directory whose
    creation is just notified.

    @param pData
    [in] User specified data given in osl_createDirectoryPath.

    @param aDirectoryUrl
    [in] The absolute file URL of the directory that was just created by
    osl_createDirectoryPath.

    @see osl_createDirectoryPath
*/
typedef void (SAL_CALL *oslDirectoryCreationCallbackFunc)(void* pData, rtl_uString* aDirectoryUrl);

/** Create a directory path.

    The osl_createDirectoryPath function creates a specified directory path.
    All nonexisting sub directories will be created.
    <p><strong>PLEASE NOTE:</strong> You cannot rely on getting the error code
    osl_File_E_EXIST for existing directories. Programming against this error
    code is in general a strong indication of a wrong usage of osl_createDirectoryPath.</p>

    @param aDirectoryUrl
    [in] The absolute file URL of the directory path to create.
    A relative file URL will not be accepted.

    @param aDirectoryCreationCallbackFunc
    [in] Pointer to a function that will be called synchronously
    for each sub directory that was created. The value of this
    parameter may be NULL, in this case notifications will not be
    sent.

    @param pData
    [in] User specified data to be passed to the directory creation
    callback function. The value of this parameter may be arbitrary
    and will not be interpreted by osl_createDirectoryPath.

    @return
    <dl>
    <dt>osl_File_E_None</dt>
    <dd>On success</dd>
    <dt>osl_File_E_INVAL</dt>
    <dd>The format of the parameters was not valid</dd>
    <dt>osl_File_E_ACCES</dt>
    <dd>Permission denied</dd>
    <dt>osl_File_E_EXIST</dt>
    <dd>The final node of the specified directory path already exist</dd>
    <dt>osl_File_E_NAMETOOLONG</dt>
    <dd>The name of the specified directory path exceeds the maximum allowed length</dd>
    <dt>osl_File_E_NOTDIR</dt>
    <dd>A component of the specified directory path already exist as file in any part of the directory path</dd>
    <dt>osl_File_E_ROFS</dt>
    <dd>Read-only file system</dd>
    <dt>osl_File_E_NOSPC</dt>
    <dd>No space left on device</dd>
    <dt>osl_File_E_DQUOT</dt>
    <dd>Quota exceeded</dd>
    <dt>osl_File_E_FAULT</dt>
    <dd>Bad address</dd>
    <dt>osl_File_E_IO</dt>
    <dd>I/O error</dd>
    <dt>osl_File_E_LOOP</dt>
    <dd>Too many symbolic links encountered</dd>
    <dt>osl_File_E_NOLINK</dt>
    <dd>Link has been severed</dd>
    <dt>osl_File_E_invalidError</dt>
    <dd>An unknown error occurred</dd>
    </dl>

    @see oslDirectoryCreationFunc
    @see oslFileError
    @see osl_createDirectory
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData);

/** Remove a regular file.

    @param pustrFileURL [in]
    Full qualified URL of the file to remove.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_PERM operation not permitted<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_ROFS read-only file system<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_IO on I/O errors<br>
    osl_File_E_BUSY device or resource busy<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>
    osl_File_E_TXTBSY text file busy<br>

    @see osl_openFile()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_removeFile(
        rtl_uString* pustrFileURL );


/** Copy a file to a new destination.

    Copies a file to a new destination. Copies only files not directories.
    No assumptions should be made about preserving attributes or file time.

    @param pustrSourceFileURL [in]
    Full qualified URL of the source file.

    @param pustrDestFileURL [in]
    Full qualified URL of the destination file. A directory is NOT a valid destination file!

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_PERM operation not permitted<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_ISDIR is a directory<br>
    osl_File_E_ROFS read-only file system<p>

    @see    osl_moveFile()
    @see    osl_removeFile()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_copyFile(
        rtl_uString* pustrSourceFileURL, rtl_uString *pustrDestFileURL );


/** Move a file or directory to a new destination or renames it.

    Moves a file or directory to a new destination or renames it.
    File time and attributes are preserved.

    @param pustrSourceFileURL [in]
    Full qualified URL of the source file.

    @param pustrDestFileURL [in]
    Full qualified URL of the destination file. An existing directory is NOT a valid destination !

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_PERM operation not permitted<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_ROFS read-only file system<br>

    @see osl_copyFile()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_moveFile(
        rtl_uString* pustrSourceFileURL, rtl_uString *pustrDestFileURL );


/** Determine a valid unused canonical name for a requested name.

    Determines a valid unused canonical name for a requested name.
    Depending on the Operating System and the File System the illegal characters are replaced by valid ones.
    If a file or directory with the requested name already exists a new name is generated following
    the common rules on the actual Operating System and File System.

    @param pustrRequestedURL [in]
    Requested name of a file or directory.

    @param ppustrValidURL [out]
    On success receives a name which is unused and valid on the actual Operating System and
    File System.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>

    @see osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getCanonicalName(
        rtl_uString *pustrRequestedURL, rtl_uString **ppustrValidURL);


/** Convert a path relative to a given directory into an full qualified file URL.

    Convert a path relative to a given directory into an full qualified file URL.
    The function resolves symbolic links if possible and path ellipses, so on success
    the resulting absolute path is fully resolved.

    @param pustrBaseDirectoryURL [in]
    Base directory URL to which the relative path is related to.

    @param pustrRelativeFileURL [in]
    An URL of a file or directory relative to the directory path specified by pustrBaseDirectoryURL
    or an absolute path.
    If pustrRelativeFileURL denotes an absolute path pustrBaseDirectoryURL will be ignored.

    @param ppustrAbsoluteFileURL [out]
    On success it receives the full qualified absolute file URL.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOMEM not enough memory for allocating structures <br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_ACCES permission denied<br>
    osl_File_E_NOENT no such file or directory<br>
    osl_File_E_NAMETOOLONG file name too long<br>
    osl_File_E_OVERFLOW value too large for defined data type<br>
    osl_File_E_FAULT bad address<br>
    osl_File_E_INTR function call was interrupted<br>
    osl_File_E_LOOP too many symbolic links encountered<br>
    osl_File_E_MULTIHOP multihop attempted<br>
    osl_File_E_NOLINK link has been severed<br>

    @see    osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getAbsoluteFileURL(
    rtl_uString* pustrBaseDirectoryURL,
    rtl_uString *pustrRelativeFileURL,
    rtl_uString **ppustrAbsoluteFileURL );


/** Convert a system dependend path into a file URL.

    @param pustrSystemPath [in]
    A System dependent path of a file or directory.

    @param ppustrFileURL [out]
    On success it receives the file URL.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>

    @see osl_getSystemPathFromFileURL()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileURLFromSystemPath(
        rtl_uString *pustrSystemPath, rtl_uString **ppustrFileURL);


/** Searche a full qualified system path or a file URL.

    @param pustrFileName [in]
    A system dependent path, a file URL, a file or relative directory.

    @param pustrSearchPath [in]
    A list of system paths, in which a given file has to be searched. The Notation of a path list is
    system dependend, e.g. on UNIX system "/usr/bin:/bin" and on Windows "C:\BIN;C:\BATCH".
    These paths are only for the search of a file or a relative path, otherwise it will be ignored.
    If pustrSearchPath is NULL or while using the search path the search failed, the function searches for
    a matching file in all system directories and in the directories listed in the PATH environment
    variable.
    The value of an environment variable should be used (e.g. LD_LIBRARY_PATH) if the caller is not
    aware of the Operating System and so doesn't know which path list delimiter to use.

    @param ppustrFileURL [out]
    On success it receives the full qualified file URL.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOTDIR not a directory<br>
    osl_File_E_NOENT no such file or directory not found<br>

    @see osl_getFileURLFromSystemPath()
    @see osl_getSystemPathFromFileURL()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_searchFileURL(
        rtl_uString *pustrFileName, rtl_uString *pustrSearchPath, rtl_uString **ppustrFileURL );


/** Convert a file URL into a system dependend path.

    @param pustrFileURL [in]
    A File URL.

    @param ppustrSystemPath [out]
    On success it receives the system path.

    @return
    osl_File_E_None on success
    osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getFileURLFromSystemPath()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getSystemPathFromFileURL(
        rtl_uString *pustrFileURL, rtl_uString **ppustrSystemPath);


/** Function pointer representing the function called back from osl_abbreviateSystemPath

    @param ustrText [in]
    Text to calculate the width for

    @return
    The width of the text specified by ustrText, e.g. it can return the width in pixel
    or the width in character count.

    @see osl_abbreviateSystemPath()
*/

typedef sal_uInt32 (SAL_CALL *oslCalcTextWidthFunc)( rtl_uString *ustrText );


/** Abbreviate a system notation path.

    @param ustrSystemPath [in]
    The full system path to abbreviate

    @param pustrCompacted [out]
    Receives the compacted system path on output

    @param pCalcWidth [in]
    Function ptr that calculates the width of a string. Can be zero.

    @param uMaxWidth [in]
    Maximum width allowed that is retunrned from pCalcWidth.
    If pCalcWidth is zero the character count is assumed as width.

    @return
    osl_File_E_None on success<br>

    @see    oslCalcTextWidthFunc
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_abbreviateSystemPath(
    rtl_uString *ustrSystemPath,
    rtl_uString **pustrCompacted,
    sal_uInt32 uMaxWidth,
    oslCalcTextWidthFunc pCalcWidth );


/** Set file attributes.

    @param pustrFileURL [in]
    The full qualified file URL.

    @param uAttributes [in]
    Attributes of the file to be set.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>

    @see osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFileAttributes(
        rtl_uString *pustrFileURL, sal_uInt64 uAttributes );


/** Set the file time.

    @param pustrFileURL [in]
    The full qualified URL of the file.

    @param aCreationTime [in]
    Creation time of the given file.

    @param aLastAccessTime [in]
    Time of the last access of the given file.

    @param aLastWriteTime [in]
    Time of the last modifying of the given file.

    @return
    osl_File_E_None on success<br>
    osl_File_E_INVAL the format of the parameters was not valid<br>
    osl_File_E_NOENT no such file or directory not found<br>

    @see osl_getFileStatus()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFileTime(
    rtl_uString *pustrFileURL,
    const TimeValue *aCreationTime,
    const TimeValue *aLastAccessTime,
    const TimeValue *aLastWriteTime);


/** Retrieves the file URL of the system's temporary directory path

    @param[out] pustrTempDirURL
    On success receives the URL of system's temporary directory path.

    @return
    osl_File_E_None on success
    osl_File_E_NOENT no such file or directory not found
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_getTempDirURL(
        rtl_uString **pustrTempDirURL );


/** Creates a temporary file in the directory provided by the caller or the
    directory returned by osl_getTempDirURL.

    Creates a temporary file in the directory provided by the caller or the
    directory returned by osl_getTempDirURL.
    Under UNIX Operating Systems the file will be created with read and write
    access for the user exclusively.
    If the caller requests only a handle to the open file but not the name of
    it, the file will be automatically removed on close else the caller is
    responsible for removing the file on success.

    Description of the different pHandle, ppustrTempFileURL parameter combinations.
    pHandle is 0 and ppustrTempDirURL is 0 - this combination is invalid
    pHandle is not 0 and ppustrTempDirURL is 0 - a handle to the open file
    will be returned on success and the file will be automatically removed on close.
    pHandle is 0 and ppustrTempDirURL is not 0 - the name of the file will be returned,
    the caller is responsible for opening, closing and removing the file.
    pHandle is not 0 and ppustrTempDirURL is not 0 - a handle to the open file as well as
    the file name will be returned, the caller is responsible for closing and removing
    the file.

    @param  pustrDirectoryURL [in]
    Specifies the full qualified URL where the temporary file should be created.
    If pustrDirectoryURL is 0 the path returned by osl_getTempDirURL will be used.

    @param  pHandle [out]
    On success receives a handle to the open file. If pHandle is 0 the file will
    be closed on return, in this case ppustrTempFileURL must not be 0.

    @param  ppustrTempFileURL [out]
    On success receives the full qualified URL of the temporary file.
    If ppustrTempFileURL is 0 the file will be automatically removed on close,
    in this case pHandle must not be 0.
    If ppustrTempFileURL is not 0 the caller receives the name of the created
    file and is responsible for removing the file, in this case
    *ppustrTempFileURL must be 0 or must point to a valid rtl_uString.

    @return
    osl_File_E_None   on success
    osl_File_E_INVAL  the format of the parameter is invalid
    osl_File_E_NOMEM  not enough memory for allocating structures
    osl_File_E_ACCES  Permission denied
    osl_File_E_NOENT  No such file or directory
    osl_File_E_NOTDIR Not a directory
    osl_File_E_ROFS   Read-only file system
    osl_File_E_NOSPC  No space left on device
    osl_File_E_DQUOT  Quota exceeded

    @see    osl_getTempDirURL()
*/

SAL_DLLPUBLIC oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_FILE_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
