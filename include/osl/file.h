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

#ifndef INCLUDED_OSL_FILE_H
#define INCLUDED_OSL_FILE_H

#include "sal/config.h"

#include "osl/time.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @file

Main goals and usage hints

The main intention of this interface is to provide an universal portable and
high performance access to file system functionality on any operating
system.

There are a few main goals:

1. The path specifications always has to be absolute. Any usage of relative
path specifications is forbidden. Exceptions are osl_getSystemPathFromFileURL,
osl_getFileURLFromSystemPath and osl_getAbsoluteFileURL. Most operating
systems provide a "Current Directory" per process. This is the reason why
relative path specifications can cause problems in multithreading
environments.

2. Proprietary notations of file paths are not supported. Every path notation
must the file URL specification. File URLs must be encoded in UTF8 and after
that escaped. Although the URL parameter is a unicode string, the must
contain only ASCII characters.

3. The caller cannot get any information whether a file system is case
sensitive, case preserving or not. The operating system implementation
itself should determine if it can map case-insensitive paths. The case
correct notation of a filename or file path is part of the "File Info". This
case correct name can be used as a unique key if necessary.

4. Obtaining information about files or volumes is controlled by a bitmask
which specifies which fields are of interest. Due to performance reasons it
is not recommended to obtain information which is not needed.  But if the
operating system provides more information anyway the implementation can set
more fields on output as were requested. It is in the responsibility of the
caller to decide if they use this additional information or not. But they
should do so to prevent further unnecessary calls if the information is
already there.

The input bitmask supports a flag osl_FileStatus_Mask_Validate which can be
used to force retrieving uncached validated information. Setting this flag
when calling osl_getFileStatus in combination with no other flag is a synonym
for a "FileExists". This should only be done when processing a single file
(i.e. before opening) and NEVER during enumeration of directory contents on
any step of information processing. This would change the runtime behaviour
from O(n) to O(n*n/2) on nearly every file system.  On Windows NT reading the
contents of an directory with 7000 entries and getting full information about
every file only takes 0.6 seconds. Specifying the flag
osl_FileStatus_Mask_Validate for each entry will increase the time to 180
seconds (!!!).

*/

/* Error codes according to errno */
typedef enum {
    osl_File_E_None,            /*!< on success                                                  */
    osl_File_E_PERM,            /*!< operation not permitted                                     */
    osl_File_E_NOENT,           /*!< no such file or directory                                   */
    osl_File_E_SRCH,            /*!< no process matches the PID                                  */
    osl_File_E_INTR,            /*!< function call was interrupted                               */
    osl_File_E_IO,              /*!< I/O error occurred                                          */
    osl_File_E_NXIO,            /*!< no such device or address                                   */
    osl_File_E_2BIG,            /*!< argument list too long                                      */
    osl_File_E_NOEXEC,          /*!< invalid executable file format                              */
    osl_File_E_BADF,            /*!< bad file descriptor                                         */
    osl_File_E_CHILD,           /*!< there are no child processes                                */
    osl_File_E_AGAIN,           /*!< resource temp unavailable, try again later                  */
    osl_File_E_NOMEM,           /*!< no memory available                                         */
    osl_File_E_ACCES,           /*!< file permissions do not allow operation                     */
    osl_File_E_FAULT,           /*!< bad address; an invalid pointer detected                    */
    osl_File_E_BUSY,            /*!< resource busy                                               */
    osl_File_E_EXIST,           /*!< file exists where should only be created                    */
    osl_File_E_XDEV,            /*!< improper link across file systems detected                  */
    osl_File_E_NODEV,           /*!< wrong device type specified                                 */
    osl_File_E_NOTDIR,          /*!< file isn't a directory where one is needed                  */
    osl_File_E_ISDIR,           /*!< file is a directory, invalid operation                      */
    osl_File_E_INVAL,           /*!< invalid argument to library function                        */
    osl_File_E_NFILE,           /*!< too many distinct file openings                             */
    osl_File_E_MFILE,           /*!< process has too many distinct files open                    */
    osl_File_E_NOTTY,           /*!< inappropriate I/O control operation                         */
    osl_File_E_FBIG,            /*!< file too large                                              */
    osl_File_E_NOSPC,           /*!< no space left on device, write failed                       */
    osl_File_E_SPIPE,           /*!< invalid seek operation (such as on pipe)                    */
    osl_File_E_ROFS,            /*!< illegal modification to read-only filesystem                */
    osl_File_E_MLINK,           /*!< too many links to file                                      */
    osl_File_E_PIPE,            /*!< broken pipe; no process reading from other end of pipe      */
    osl_File_E_DOM,             /*!< domain error (mathematical error)                           */
    osl_File_E_RANGE,           /*!< range error (mathematical error)                            */
    osl_File_E_DEADLK,          /*!< deadlock avoided                                            */
    osl_File_E_NAMETOOLONG,     /*!< filename too long                                           */
    osl_File_E_NOLCK,           /*!< no locks available                                          */
    osl_File_E_NOSYS,           /*!< function not implemented                                    */
    osl_File_E_NOTEMPTY,        /*!< directory not empty                                         */
    osl_File_E_LOOP,            /*!< too many levels of symbolic links found during name lookup  */
    osl_File_E_ILSEQ,           /*!< invalid or incomplete byte sequence of multibyte char found */
    osl_File_E_NOLINK,          /*!< link has been severed                                       */
    osl_File_E_MULTIHOP,        /*!< remote resource is not directly available                   */
    osl_File_E_USERS,           /*!< file quote system is confused as there are too many users   */
    osl_File_E_OVERFLOW,        /*!< value too large for defined data type                       */
    osl_File_E_NOTREADY,        /*!< device not ready                                            */
    osl_File_E_invalidError,    /*!< unmapped error: always last entry in enum!                  */
    osl_File_E_TIMEDOUT,        /*!< socket operation timed out                                  */
    osl_File_E_NETWORK,         /*!< unexpected network error occurred (Windows) - could be a
                                     user session was deleted, or an unexpected network error
                                     occurred                                                    */
    osl_File_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslFileError;

typedef void *oslDirectory;
typedef void *oslDirectoryItem;

/** Open a directory for enumerating its contents.

    @param[in]  pustrDirectoryURL
    The full qualified URL of the directory.

    @param[out]  pDirectory
    On success it receives a handle used for subsequent calls by osl_getNextDirectoryItem().
    The handle has to be released by a call to osl_closeDirectory().

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOENT the specified path doesn't exist
    @retval osl_File_E_NOTDIR the specified path is not an directory
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_MFILE too many open files used by the process
    @retval osl_File_E_NFILE too many open files in the system
    @retval osl_File_E_NAMETOOLONG File name too long
    @retval osl_File_E_LOOP Too many symbolic links encountered

    @see osl_getNextDirectoryItem()
    @see osl_closeDirectory()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_openDirectory(
        rtl_uString *pustrDirectoryURL, oslDirectory *pDirectory);

/** Retrieve the next item of a previously opened directory.

    Retrieves the next item of a previously opened directory.
    All handles have an initial refcount of 1.

    @param[in]  Directory
    A directory handle received from a previous call to osl_openDirectory().

    @param[out]  pItem
    On success it receives a handle that can be used for subsequent calls to osl_getFileStatus().
    The handle has to be released by a call to osl_releaseDirectoryItem().

    @param[in]  uHint
    With this parameter the caller can tell the implementation that (s)he
    is going to call this function uHint times afterwards. This enables the implementation to
    get the information for more than one file and cache it until the next calls.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_NOENT no more entries in this directory
    @retval osl_File_E_BADF invalid oslDirectory parameter
    @retval osl_File_E_OVERFLOW the value too large for defined data type

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

    @param[in] Directory
    A handle received by a call to osl_openDirectory().

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_BADF invalid oslDirectory parameter
    @retval osl_File_E_INTR the function call was interrupted

    @see osl_openDirectory()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_closeDirectory(
        oslDirectory Directory);

/** Retrieve a single directory item.

    Retrieves a single directory item. The returned handle has an initial refcount of 1.
    Due to performance issues it is not recommended to use this function while
    enumerating the contents of a directory. In this case use osl_getNextDirectoryItem() instead.

    @param[in] pustrFileURL
    An absolute file URL.

    @param[out] pItem
    On success it receives a handle which can be used for subsequent calls to osl_getFileStatus().
    The handle has to be released by a call to osl_releaseDirectoryItem().

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_MFILE too many open files used by the process
    @retval osl_File_E_NFILE too many open files in the system
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_NAMETOOLONG the file name is too long
    @retval osl_File_E_NOTDIR a component of the path prefix of path is not a directory
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_INTR the function call was interrupted

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

    @param[in]  Item
    A handle received by a call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid

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

    @param[in]  Item
    A handle received by a call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getDirectoryItem()
    @see osl_getNextDirectoryItem()
    @see osl_acquireDirectoryItem()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_releaseDirectoryItem(
        oslDirectoryItem Item );

/** Determine if two directory items point the same underlying file

    The comparison is done first by URL, and then by resolving links to
    find the target, and finally by comparing inodes on unix.

    @param[in]  pItemA
    A directory handle to compare with another handle

    @param[in]  pItemB
    A directory handle to compare with pItemA

    @retval sal_True if the items point to an identical resource
    @retval sal_False if the items point to a different resource, or a fatal error occurred

    @see osl_getDirectoryItem()

    @since LibreOffice 3.6
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_identicalDirectoryItem(
                                        oslDirectoryItem pItemA,
                                        oslDirectoryItem pItemB );

/**
   @defgroup filetype File types

   @{
 */
typedef enum {
    osl_File_Type_Directory,        /*< directory               */
    osl_File_Type_Volume,           /*< volume (e.g. C:, A:)    */
    osl_File_Type_Regular,          /*< regular file            */
    osl_File_Type_Fifo,             /*< named pipe              */
    osl_File_Type_Socket,           /*< socket                  */
    osl_File_Type_Link,             /*< file link               */
    osl_File_Type_Special,          /*< special device file     */
    osl_File_Type_Unknown           /*< unknown file type       */
} oslFileType;
/** @} */

/**
   @defgroup fileattrs File attributes

   @{
 */
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
/** @} */

/**
   @defgroup filestatus Flags specifying which fields to retrieve by osl_getFileStatus

   @{
 */
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
/** @} */

/** Structure containing information about files and directories

    @see    osl_getFileStatus()
    @see    oslFileType
*/
typedef struct _oslFileStatus {
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
/** Case correct name of the file. Should be set to zero before calling osl_getFileStatus
    and released after usage. */
    rtl_uString *ustrFileName;
/** Full URL of the file. Should be set to zero before calling osl_getFileStatus
    and released after usage. */
    rtl_uString *ustrFileURL;
/** Full URL of the target file if the file itself is a link.
    Should be set to zero before calling osl_getFileStatus
    and released after usage. */
    rtl_uString *ustrLinkTargetURL;
} oslFileStatus;

/** Retrieve information about a single file or directory.

    @param[in]  Item
    A handle received by a previous call to osl_getDirectoryItem() or osl_getNextDirectoryItem().

    @param[in,out] pStatus
    Points to a structure which receives the information of the file or directory
    represented by the handle Item. The member uStructSize has to be initialized to
    sizeof(oslFileStatus) before calling this function.

    @param[in]  uFieldMask
    Specifies which fields of the structure pointed to by pStatus are of interest to the caller.

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_BADF invalid oslDirectoryItem parameter
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_OVERFLOW value too large for defined data type
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_MULTIHOP components of path require hopping to multiple
        remote machines and the file system does not allow it
    @retval osl_File_E_MFILE too many open files used by the process
    @retval osl_File_E_NFILE too many open files in the system
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_NXIO no such device or address
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_NOSYS function not implemented

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

    @param[in] Handle
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @retval
    osl_File_E_None on success

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

    @param[in] Handle
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @retval
    osl_File_E_None on success

    @todo
    specify all error codes that may be returned

    @see osl_getVolumeInformation()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_acquireVolumeDeviceHandle(
        oslVolumeDeviceHandle Handle );

/** Get the full qualified URL where a device is mounted to.

    @param[in] Handle
    An oslVolumeDeviceHandle received by a call to osl_getVolumeInformation().

    @param[out] ppustrDirectoryURL
    Receives the full qualified URL where the device is mounted to.

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_NXIO  no such device or address
    @retval osl_File_E_NODEV no such device
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_FAULT bad address
    @retval osl_FilE_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_EOVERFLOW value too large for defined data type

    @see    osl_getVolumeInformation()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getVolumeDeviceMountPath(
        oslVolumeDeviceHandle Handle, rtl_uString **ppustrDirectoryURL);

/**
   @defgroup volattrs Volume attributes

   @{
 */
#define osl_Volume_Attribute_Removeable            0x00000001L
#define osl_Volume_Attribute_Remote                0x00000002L
#define osl_Volume_Attribute_CompactDisc           0x00000004L
#define osl_Volume_Attribute_FixedDisk             0x00000008L
#define osl_Volume_Attribute_RAMDisk               0x00000010L
#define osl_Volume_Attribute_FloppyDisk            0x00000020L

#define osl_Volume_Attribute_Case_Is_Preserved     0x00000040L
#define osl_Volume_Attribute_Case_Sensitive        0x00000080L

/** @} */

/**
    @defgroup volinfoflags Flags specifying which fields to retrieve by osl_getVolumeInfo

    @{
 */

#define osl_VolumeInfo_Mask_Attributes             0x00000001L
#define osl_VolumeInfo_Mask_TotalSpace             0x00000002L
#define osl_VolumeInfo_Mask_UsedSpace              0x00000004L
#define osl_VolumeInfo_Mask_FreeSpace              0x00000008L
#define osl_VolumeInfo_Mask_MaxNameLength          0x00000010L
#define osl_VolumeInfo_Mask_MaxPathLength          0x00000020L
#define osl_VolumeInfo_Mask_FileSystemName         0x00000040L
#define osl_VolumeInfo_Mask_DeviceHandle           0x00000080L
#define osl_VolumeInfo_Mask_FileSystemCaseHandling 0x00000100L

/** @} */

/** Structure containing information about volumes

    @see    osl_getVolumeInformation()
    @see    oslFileType
*/

typedef struct _oslVolumeInfo {
/** Must be initialized with the size in bytes of the structure before
    passing it to any function */
    sal_uInt32      uStructSize;
/** Determines which members of the structure contain valid data */
    sal_uInt32      uValidFields;
/** Attributes of the volume (remote and/or removable) */
    sal_uInt32      uAttributes;
/** Total available space on the volume for the current process/user */
    sal_uInt64      uTotalSpace;
/** Used space on the volume for the current process/user */
    sal_uInt64      uUsedSpace;
/** Free space on the volume for the current process/user */
    sal_uInt64      uFreeSpace;
/** Maximum length of file name of a single item */
    sal_uInt32      uMaxNameLength;
/** Maximum length of a full qualified path in system notation */
    sal_uInt32      uMaxPathLength;
/** Points to a string that receives the name of the file system type. String
    should be set to zero before calling osl_getVolumeInformation and released
    after usage. */
    rtl_uString     *ustrFileSystemName;
/** Pointer to handle the receives underlying device. Handle should be set to
    zero before calling osl_getVolumeInformation */
    oslVolumeDeviceHandle   *pDeviceHandle;
} oslVolumeInfo;

/** Retrieve information about a volume.

    Retrieves information about a volume. A volume can either be a mount point, a network
    resource or a drive depending on Operating System and File System. Before calling this
    function osl_getFileStatus() should be called to determine if the type is
    osl_file_Type_Volume.

    @param[in] pustrDirectoryURL
    Full qualified URL of the volume

    @param[out] pInfo
    On success it receives information about the volume.

    @param[in] uFieldMask
    Specifies which members of the structure should be filled

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval ols_File_E_FAULT Bad address
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_NOSYS function not implemented
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_INTR function call was interrupted

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

    Open a file. Only regular files can be opened.

    @param[in] pustrFileURL
    The full qualified URL of the file to open.

    @param[out] pHandle
    On success it receives a handle to the open file.

    @param[in] uFlags
    Specifies the open mode.

    On Android, if the file path is below the /assets folder, the file
    exists only as a hopefully uncompressed element inside the app
    package (.apk), which has been mapped into memory as a whole by
    the LibreOffice Android bootstrapping code. So files "opened" from
    there aren't actually files in the OS sense.

    @retval osl_File_E_None on success
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NAMETOOLONG pathname was too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_AGAIN a write lock could not be established
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_NXIO no such device or address
    @retval osl_File_E_NODEV no such device
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_TXTBSY text file busy
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_MFILE too many open files used by the process
    @retval osl_File_E_NFILE too many open files in the system
    @retval osl_File_E_DQUOT quota exceeded
    @retval osl_File_E_EXIST file exists
    @retval osl_FilE_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_EOVERFLOW value too large for defined data type

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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in] uHow
    How to calculate the offset - osl_Pos_Absolut means start at the
    beginning of the file, osl_Pos_Current means offset from the current
    seek position and osl_Pos_End means the offset will be negative and
    the position will be calculated backwards from the end of the file by
    the offset provided.

    @param[in] uPos
    Seek offset, depending on uHow. If uHow is osl_Pos_End then the value must be negative.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
        (e.g. if uHow is osl_Pos_End then must be negative)
    @retval osl_File_E_OVERFLOW the resulting file offset would be a
        value which cannot be represented correctly for regular files

    @see    osl_openFile()
    @see    osl_getFilePos()
*/
SAL_WARN_UNUSED_RESULT SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFilePos(
        oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uPos );

/** Retrieve the current position of the internal pointer of an open file.

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[out] pPos
    On success receives the current position of the file pointer.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_OVERFLOW the resulting file offset would be a value
        which cannot be represented correctly for regular files

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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in] uSize
    New size in bytes.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_OVERFLOW the resulting file offset would be a value
        which cannot be represented correctly for regular files

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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[out] pSize
    Current size in bytes.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_OVERFLOW the resulting file offset would be a value
        which cannot be represented correctly for regular files

    @see osl_openFile()
    @see osl_setFilePos()
    @see osl_getFileStatus()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileSize(
        oslFileHandle Handle, sal_uInt64 *pSize );

/** Indicate that the file can be accessed randomly (i.e. there is no sequential
    reading). Basically it means that the first byte of every page in the
    file-mapping will be read.

    @since UDK 3.2.10
 */
#define osl_File_MapFlag_RandomAccess ((sal_uInt32)(0x1))

/** Map flag denoting that the mapped address space will be accessed by the
    process soon (and it is advantageous for the operating system to already
    start paging in the data).

    @attention As this assumes that madvise() with the WILLREAD flag is
    asynchronous (which is I'm afraid an incorrect assumption), Linux systems
    will ignore this flag.

    @since UDK 3.2.12
*/
#define osl_File_MapFlag_WillNeed ((sal_uInt32)(0x2))

/** Map a shared file into memory.

    Files can be mapped into memory to allow multiple processes to use
    this memory-mapped file to share data.

    On Android, if the Handle refers to a file that is actually inside
    the app package (.apk zip archive), no new mapping is created,
    just a pointer to the file inside the already mapped .apk is
    returned.

    @param[in] Handle       Handle of the file to be mapped.
    @param[in,out] ppAddr   Memory address of the mapped file
    @param[in] uLength      Amount to map of the file from the offset
    @param[in] uOffset      Offset into the file to map
    @param[in] uFlags       osl_File_MapFlag_RandomAccess or
                            osl_File_MapFlag_WillNeed

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL invalid file handle, on Unix systems also
                can mean that the address, length of the file or the
                file offset are too large or not aligned on a page
                boundary; on Linux can also mean after Linux 2.6.12
                that the length was set to 0 (illogical).
    @retval osl_File_E_OVERFLOW requested mapping size too large,
                or the file offset was too large
    @retval osl_File_E_ACCES file descriptor to non-regular file, or
                file descriptor not open for reading, or the file
                descriptor is not open in read/write mode
    @retval osl_File_E_AGAIN file has been locked, or too much memory
                has been locked
    @retval osl_File_E_NODEV underlying filesystem of specified file
                does not support memory mapping
    @retval osl_File_E_TXTBSY on Linux means that writing to the mapped
                file is denied, but the file descriptor points to a file
                open for writing
    @retval osl_File_E_NOMEM process's maximum number of mappings have
                been exceeded

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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[out] pBuffer
    Points to a buffer which receives data. The buffer must be large enough
    to hold uBytesRequested bytes.

    @param[in] uBytesRequested
    Number of bytes which should be retrieved.

    @param[out] pBytesRead
    On success the number of bytes which have actually been retrieved.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_NOLINK link has been severed

    @see osl_openFile()
    @see osl_writeFile()
    @see osl_readLine()
    @see osl_setFilePos()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_readFile(
        oslFileHandle Handle, void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64 *pBytesRead );

/** Test if the end of a file is reached.

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[out] pIsEOF
    Points to a variable that receives the end-of-file status.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_NOLINK link has been severed

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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in] pBuffer
    Points to a buffer which contains the data.

    @param[in] uBytesToWrite
    Number of bytes which should be written.

    @param[out] pBytesWritten
    On success the number of bytes which have actually been written.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_FBIG file too large
    @retval osl_File_E_DQUOT quota exceeded
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errosr
    @retval osl_File_E_NOLCK no record locks available
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_NXIO no such device or address

    @see osl_openFile()
    @see osl_readFile()
    @see osl_setFilePos()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_writeFile(
        oslFileHandle Handle, const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64 *pBytesWritten );

/** Read a number of bytes from a specified offset in a file.

    The current position of the internal file pointer may or may not be changed.

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in] uOffset
    Offset position from start of file where read starts

    @param[out] pBuffer
    Points to a buffer which receives data. The buffer must be large enough
    to hold uBytesRequested bytes.

    @param[in] uBytesRequested
    Number of bytes which should be retrieved.

    @param[out] pBytesRead
    On success the number of bytes which have actually been retrieved.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_NOLINK link has been severed
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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in] uOffset
    Position of file to write into.

    @param[in] pBuffer
    Points to a buffer which contains the data.

    @param[in] uBytesToWrite
    Number of bytes which should be written.

    @param[out] pBytesWritten
    On success the number of bytes which have actually been written.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_FBIG file too large
    @retval osl_File_E_DQUOT quota exceeded
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errosr
    @retval osl_File_E_NOLCK no record locks available
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_NXIO no such device or address
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

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @param[in,out] ppSequence
    A pointer pointer to a sal_Sequence that will hold the line read on success.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_BADF bad file
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_AGAIN operation would block
    @retval osl_File_E_NOLINK link has been severed

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

      @retval osl_File_E_None On success
      @retval osl_File_E_INVAL The value of the input parameter is invalid
      @retval osl_File_E_BADF The file associated with the given file handle is not open for writing
      @retval osl_File_E_IO An I/O error occurred
      @retval osl_File_E_NOSPC There is no enough space on the target device
      @retval osl_File_E_ROFS The file associated with the given file handle is located on a read only file system
      @retval osl_File_E_TIMEDOUT A remote connection timed out. This may happen when a file is on a remote location

      @see osl_openFile()
      @see osl_writeFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_syncFile( oslFileHandle Handle );

/** Close an open file.

    @param[in] Handle
    Handle to a file received by a previous call to osl_openFile().

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_BADF Bad file
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_IO on I/O errors

    @see osl_openFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_closeFile( oslFileHandle Handle );

/** Create a directory.

    @param[in] pustrDirectoryURL
    Full qualified URL of the directory to create.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_EXIST file exists
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_NOSPC no space left on device
    @retval osl_File_E_DQUOT quota exceeded
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_FAULT bad address
    @retval osl_FileE_IO on I/O errors
    @retval osl_File_E_MLINK too many links
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed

    @see osl_removeDirectory()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_createDirectory( rtl_uString* pustrDirectoryURL );

/** Create a directory, passing flags.

    @param url
    File URL of the directory to create.

    @param flags
    A combination of the same osl_File_OpenFlag_*s used by osl_openFile,
    except that osl_File_OpenFlag_Create is implied and ignored.  Support for
    the various flags can differ across operating systems.

    @see osl_createDirectory()

    @since LibreOffice 4.3
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_createDirectoryWithFlags(
    rtl_uString * url, sal_uInt32 flags);

/** Remove an empty directory.

    @param[in] pustrDirectoryURL
    Full qualified URL of the directory.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_PERM operation not permitted
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_NOTEMPTY directory not empty
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_BUSY device or resource busy
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_EXIST file exists
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed

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

    @attention PLEASE NOTE You cannot rely on getting the error code
    osl_File_E_EXIST for existing directories. Programming against this error
    code is in general a strong indication of a wrong usage of osl_createDirectoryPath.

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

    @retval osl_File_E_None On success
    @retval osl_File_E_INVAL The format of the parameters was not valid
    @retval osl_File_E_ACCES Permission denied
    @retval osl_File_E_EXIST The final node of the specified directory path already exist
    @retval osl_File_E_NAMETOOLONG The name of the specified directory path exceeds the maximum allowed length
    @retval osl_File_E_NOTDIR A component of the specified directory path already exist as file in any part of the directory path
    @retval osl_File_E_ROFS Read-only file system
    @retval osl_File_E_NOSPC No space left on device
    @retval osl_File_E_DQUOT Quota exceeded
    @retval osl_File_E_FAULT Bad address
    @retval osl_File_E_IO I/O error
    @retval osl_File_E_LOOP Too many symbolic links encountered
    @retval osl_File_E_NOLINK Link has been severed
    @retval osl_File_E_invalidError An unknown error occurred

    @see oslDirectoryCreationFunc
    @see oslFileError
    @see osl_createDirectory
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData);

/** Remove a regular file.

    @param[in] pustrFileURL
    Full qualified URL of the file to remove.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_PERM operation not permitted
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_IO on I/O errors
    @retval osl_File_E_BUSY device or resource busy
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed
    @retval osl_File_E_TXTBSY text file busy

    @see osl_openFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_removeFile(
        rtl_uString* pustrFileURL );

/** Copy a file to a new destination.

    Copies a file to a new destination. Copies only files not directories.
    No assumptions should be made about preserving attributes or file time.

    @param[in] pustrSourceFileURL
    Full qualified URL of the source file.

    @param[in] pustrDestFileURL
    Full qualified URL of the destination file. A directory is NOT a valid destination file!

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_PERM operation not permitted
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_ISDIR is a directory
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_BUSY if the implementation internally requires resources that are
        (temporarily) unavailable (added with LibreOffice 4.4)

    @see    osl_moveFile()
    @see    osl_removeFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_copyFile(
        rtl_uString* pustrSourceFileURL, rtl_uString *pustrDestFileURL );

/** Move a file or directory to a new destination or renames it.

    Moves a file or directory to a new destination or renames it.
    File time and attributes are preserved.

    @param[in] pustrSourceFileURL
    Full qualified URL of the source file.

    @param[in] pustrDestFileURL
    Full qualified URL of the destination file. An existing directory is NOT a valid destination !

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_PERM operation not permitted
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_BUSY if the implementation internally requires resources that are
        (temporarily) unavailable (added with LibreOffice 4.4)

    @see osl_copyFile()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_moveFile(
        rtl_uString* pustrSourceFileURL, rtl_uString *pustrDestFileURL );

/** Determine a valid unused canonical name for a requested name.

    Determines a valid unused canonical name for a requested name.
    Depending on the Operating System and the File System the illegal characters are replaced by valid ones.
    If a file or directory with the requested name already exists a new name is generated following
    the common rules on the actual Operating System and File System.

    @param[in] pustrRequestedURL
    Requested name of a file or directory.

    @param[out] ppustrValidURL
    On success receives a name which is unused and valid on the actual Operating System and
    File System.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getFileStatus()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getCanonicalName(
        rtl_uString *pustrRequestedURL, rtl_uString **ppustrValidURL);

/** Convert a path relative to a given directory into an full qualified file URL.

    Convert a path relative to a given directory into an full qualified file URL.
    The function resolves symbolic links if possible and path ellipses, so on success
    the resulting absolute path is fully resolved.

    @param[in] pustrBaseDirectoryURL
    Base directory URL to which the relative path is related to.

    @param[in] pustrRelativeFileURL
    A URL of a file or directory relative to the directory path specified by pustrBaseDirectoryURL
    or an absolute path.
    If pustrRelativeFileURL denotes an absolute path pustrBaseDirectoryURL will be ignored.

    @param[out] ppustrAbsoluteFileURL
    On success it receives the full qualified absolute file URL.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_NOENT no such file or directory
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_OVERFLOW value too large for defined data type
    @retval osl_File_E_FAULT bad address
    @retval osl_File_E_INTR function call was interrupted
    @retval osl_File_E_LOOP too many symbolic links encountered
    @retval osl_File_E_MULTIHOP multihop attempted
    @retval osl_File_E_NOLINK link has been severed

    @see    osl_getFileStatus()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getAbsoluteFileURL(
    rtl_uString* pustrBaseDirectoryURL,
    rtl_uString *pustrRelativeFileURL,
    rtl_uString **ppustrAbsoluteFileURL );

/** Convert a system dependent path into a file URL.

    @param[in] pustrSystemPath
    A System dependent path of a file or directory.

    @param[out] ppustrFileURL
    On success it receives the file URL.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getSystemPathFromFileURL()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileURLFromSystemPath(
        rtl_uString *pustrSystemPath, rtl_uString **ppustrFileURL);

/** Search a full qualified system path or a file URL.

    @param[in] pustrFileName
    A system dependent path, a file URL, a file or relative directory.

    @param[in] pustrSearchPath
    @parblock
        A list of system paths, in which a given file has to be searched. The Notation of a path
        list is system dependent, e.g. on UNIX system "/usr/bin:/bin" and on Windows "C:\BIN;C:\BATCH".
        These paths are only for the search of a file or a relative path, otherwise it will be ignored.
        If pustrSearchPath is NULL or while using the search path the search failed, the function
        searches for a matching file in all system directories and in the directories listed in the PATH
        environment variable.

        The value of an environment variable should be used (e.g.
        LD_LIBRARY_PATH) if the caller is not aware of the Operating System and so doesn't know which
        path list delimiter to use.
    @endparblock

    @param[out] ppustrFileURL
    On success it receives the full qualified file URL.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOTDIR not a directory
    @retval osl_File_E_NOENT no such file or directory not found

    @see osl_getFileURLFromSystemPath()
    @see osl_getSystemPathFromFileURL()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_searchFileURL(
        rtl_uString *pustrFileName, rtl_uString *pustrSearchPath, rtl_uString **ppustrFileURL );

/** Convert a file URL into a system dependent path.

    @param[in] pustrFileURL
    A File URL.

    @param[out] ppustrSystemPath
    On success it receives the system path.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getFileURLFromSystemPath()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getSystemPathFromFileURL(
        rtl_uString *pustrFileURL, rtl_uString **ppustrSystemPath);

/** Function pointer representing the function called back from osl_abbreviateSystemPath

    @param[in] ustrText
    Text to calculate the width for

    @return
    The width of the text specified by ustrText, e.g. it can return the width in pixel
    or the width in character count.

    @see osl_abbreviateSystemPath()
*/
typedef sal_uInt32 (SAL_CALL *oslCalcTextWidthFunc)( rtl_uString *ustrText );

/** Abbreviate a system notation path.

    @param[in] ustrSystemPath
    The full system path to abbreviate

    @param[out] pustrCompacted
    Receives the compacted system path on output

    @param[in] pCalcWidth
    Function ptr that calculates the width of a string. Can be zero.

    @param[in] uMaxWidth
    Maximum width allowed that is returned from pCalcWidth.
    If pCalcWidth is zero the character count is assumed as width.

    @retval osl_File_E_None on success

    @see    oslCalcTextWidthFunc
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_abbreviateSystemPath(
    rtl_uString *ustrSystemPath,
    rtl_uString **pustrCompacted,
    sal_uInt32 uMaxWidth,
    oslCalcTextWidthFunc pCalcWidth );

/** Set file attributes.

    @param[in] pustrFileURL
    The full qualified file URL.

    @param[in] uAttributes
    Attributes of the file to be set.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid

    @see osl_getFileStatus()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_setFileAttributes(
        rtl_uString *pustrFileURL, sal_uInt64 uAttributes );

/** Set the file time.

    @param[in] pustrFileURL
    The full qualified URL of the file.

    @param[in] aCreationTime
    Creation time of the given file.

    @param[in] aLastAccessTime
    Time of the last access of the given file.

    @param[in] aLastWriteTime
    Time of the last modifying of the given file.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOENT no such file or directory not found

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

    @retval osl_File_E_None on success
    @retval osl_File_E_NOENT no such file or directory not found
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

    @param[in]  pustrDirectoryURL
    Specifies the full qualified URL where the temporary file should be created.
    If pustrDirectoryURL is 0 the path returned by osl_getTempDirURL will be used.

    @param[out]  pHandle
    On success receives a handle to the open file. If pHandle is 0 the file will
    be closed on return, in this case ppustrTempFileURL must not be 0.

    @param[out]  ppustrTempFileURL
    On success receives the full qualified URL of the temporary file.
    If ppustrTempFileURL is 0 the file will be automatically removed on close,
    in this case pHandle must not be 0.
    If ppustrTempFileURL is not 0 the caller receives the name of the created
    file and is responsible for removing the file, in this case
    *ppustrTempFileURL must be 0 or must point to a valid rtl_uString.

    @retval osl_File_E_None   on success
    @retval osl_File_E_INVAL  the format of the parameter is invalid
    @retval osl_File_E_NOMEM  not enough memory for allocating structures
    @retval osl_File_E_ACCES  Permission denied
    @retval osl_File_E_NOENT  No such file or directory
    @retval osl_File_E_NOTDIR Not a directory
    @retval osl_File_E_ROFS   Read-only file system
    @retval osl_File_E_NOSPC  No space left on device
    @retval osl_File_E_DQUOT  Quota exceeded

    @see    osl_getTempDirURL()
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL);

/** Move a file to a new destination or rename it, taking old file's identity (if exists).

    Moves or renames a file, replacing an existing file if exist. If the old file existed,
    moved file's metadata, e.g. creation time (on FSes which keep files' creation time) or
    ACLs, are set to old one's (to keep the old file's identity) - currently this is only
    implemented on Windows; on other platforms, this is equivalent to osl_moveFile.

    @param[in] pustrSourceFileURL
    Full qualified URL of the source file.

    @param[in] pustrDestFileURL
    Full qualified URL of the destination file.

    @retval osl_File_E_None on success
    @retval osl_File_E_INVAL the format of the parameters was not valid
    @retval osl_File_E_NOMEM not enough memory for allocating structures
    @retval osl_File_E_ACCES permission denied
    @retval osl_File_E_PERM operation not permitted
    @retval osl_File_E_NAMETOOLONG file name too long
    @retval osl_File_E_NOENT no such file
    @retval osl_File_E_ROFS read-only file system
    @retval osl_File_E_BUSY if the implementation internally requires resources that are
        (temporarily) unavailable

    @see osl_moveFile()

    @since LibreOffice 6.2
*/
SAL_DLLPUBLIC oslFileError SAL_CALL osl_replaceFile(rtl_uString* pustrSourceFileURL,
                                                    rtl_uString* pustrDestFileURL);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_FILE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
