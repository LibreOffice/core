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

/*
 * This file is part of LibreOffice published API.
 */

#ifndef INCLUDED_OSL_FILE_HXX
#define INCLUDED_OSL_FILE_HXX

#include "sal/config.h"

#include <string.h>

#include <cstddef>

#include "sal/log.hxx"
#include "osl/time.h"
#include "rtl/ustring.hxx"

#include "osl/file.h"
#include "osl/diagnose.h"

namespace rtl { class ByteSequence; }

namespace osl
{


/** Base class for all File System specific objects.

    @see Directory
    @see DirectoryItem
    @see File
 */

class FileBase
{
public:

    enum RC {
        E_None         = osl_File_E_None,           ///< on success
        E_PERM         = osl_File_E_PERM,           ///< operation not permitted
        E_NOENT        = osl_File_E_NOENT,          ///< no such file or directory
        E_SRCH         = osl_File_E_SRCH,           ///< no process matches the PID
        E_INTR         = osl_File_E_INTR,           ///< function call was interrupted
        E_IO           = osl_File_E_IO,             ///< I/O error occurred
        E_NXIO         = osl_File_E_NXIO,           ///< no such device or address
        E_2BIG         = osl_File_E_2BIG,           ///< argument list too long
        E_NOEXEC       = osl_File_E_NOEXEC,         ///< invalid executable file format
        E_BADF         = osl_File_E_BADF,           ///< bad file descriptor
        E_CHILD        = osl_File_E_CHILD,          ///< there are no child processes
        E_AGAIN        = osl_File_E_AGAIN,          ///< resource temp unavailable, try again later
        E_NOMEM        = osl_File_E_NOMEM,          ///< no memory available
        E_ACCES        = osl_File_E_ACCES,          ///< file permissions do not allow operation
        E_FAULT        = osl_File_E_FAULT,          ///< bad address; an invalid pointer detected
        E_BUSY         = osl_File_E_BUSY,           ///< resource busy
        E_EXIST        = osl_File_E_EXIST,          ///< file exists where should only be created
        E_XDEV         = osl_File_E_XDEV,           ///< improper link across file systems detected
        E_NODEV        = osl_File_E_NODEV,          ///< wrong device type specified
        E_NOTDIR       = osl_File_E_NOTDIR,         ///< file isn't a directory where one is needed
        E_ISDIR        = osl_File_E_ISDIR,          ///< file is a directory, invalid operation
        E_INVAL        = osl_File_E_INVAL,          ///< invalid argument to library function
        E_NFILE        = osl_File_E_NFILE,          ///< too many distinct file openings
        E_MFILE        = osl_File_E_MFILE,          ///< process has too many distinct files open
        E_NOTTY        = osl_File_E_NOTTY,          ///< inappropriate I/O control operation
        E_FBIG         = osl_File_E_FBIG,           ///< file too large
        E_NOSPC        = osl_File_E_NOSPC,          ///< no space left on device, write failed
        E_SPIPE        = osl_File_E_SPIPE,          ///< invalid seek operation (such as on pipe)
        E_ROFS         = osl_File_E_ROFS,           ///< illegal modification to read-only filesystem
        E_MLINK        = osl_File_E_MLINK,          ///< too many links to file
        E_PIPE         = osl_File_E_PIPE,           ///< broken pipe; no process reading from other end of pipe
        E_DOM          = osl_File_E_DOM,            ///< domain error (mathematical error)
        E_RANGE        = osl_File_E_RANGE,          ///< range error (mathematical error)
        E_DEADLK       = osl_File_E_DEADLK,         ///< deadlock avoided
        E_NAMETOOLONG  = osl_File_E_NAMETOOLONG,    ///< filename too long
        E_NOLCK        = osl_File_E_NOLCK,          ///< no locks available
        E_NOSYS        = osl_File_E_NOSYS,          ///< function not implemented
        E_NOTEMPTY     = osl_File_E_NOTEMPTY,       ///< directory not empty
        E_LOOP         = osl_File_E_LOOP,           ///< too many levels of symbolic links found during name lookup
        E_ILSEQ        = osl_File_E_ILSEQ,          ///< invalid or incomplete byte sequence of multibyte char found
        E_NOLINK       = osl_File_E_NOLINK,         ///< link has been severed
        E_MULTIHOP     = osl_File_E_MULTIHOP,       ///< remote resource is not directly available
        E_USERS        = osl_File_E_USERS,          ///< file quote system is confused as there are too many users
        E_OVERFLOW     = osl_File_E_OVERFLOW,       ///< value too large for defined data type
        E_NOTREADY     = osl_File_E_NOTREADY,       ///< device not ready
        E_invalidError = osl_File_E_invalidError,   ///< unmapped error: always last entry in enum!
        E_TIMEDOUT     = osl_File_E_TIMEDOUT,       ///< socket operation timed out
        E_NETWORK      = osl_File_E_NETWORK
    };


public:

    /** Determine a valid unused canonical name for a requested name.

        Determines a valid unused canonical name for a requested name.
        Depending on the Operating System and the File System the illegal characters are replaced by valid ones.
        If a file or directory with the requested name already exists a new name is generated following
        the common rules on the actual Operating System and File System.

        @param[in] ustrRequestedURL
        Requested name of a file or directory.

        @param[out] ustrValidURL
        On success receives a name which is unused and valid on the actual Operating System and
        File System.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid

        @see DirectoryItem::getFileStatus()
    */

    static RC getCanonicalName( const ::rtl::OUString& ustrRequestedURL, ::rtl::OUString& ustrValidURL )
    {
        return static_cast< RC >( osl_getCanonicalName( ustrRequestedURL.pData, &ustrValidURL.pData ) );
    }

    /** Convert a path relative to a given directory into an full qualified file URL.

        Convert a path relative to a given directory into an full qualified file URL.
        The function resolves symbolic links if possible and path ellipses, so on success
        the resulting absolute path is fully resolved.

        @param[in] ustrBaseDirectoryURL
        Base directory URL to which the relative path is related to.

        @param[in] ustrRelativeFileURL
        A URL of a file or directory relative to the directory path specified by ustrBaseDirectoryURL
        or an absolute path.
        If ustrRelativeFileURL denotes an absolute path ustrBaseDirectoryURL will be ignored.

        @param[out] ustrAbsoluteFileURL
        On success it receives the full qualified absolute file URL.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_NOTDIR not a directory
        @retval E_ACCES permission denied
        @retval E_NOENT no such file or directory
        @retval E_NAMETOOLONG file name too long
        @retval E_OVERFLOW value too large for defined data type
        @retval E_FAULT bad address
        @retval E_INTR function call was interrupted
        @retval E_LOOP too many symbolic links encountered
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed

        @see DirectoryItem::getFileStatus()
    */

    static RC getAbsoluteFileURL( const ::rtl::OUString& ustrBaseDirectoryURL, const ::rtl::OUString& ustrRelativeFileURL, ::rtl::OUString& ustrAbsoluteFileURL )
    {
        return static_cast< RC >( osl_getAbsoluteFileURL( ustrBaseDirectoryURL.pData, ustrRelativeFileURL.pData, &ustrAbsoluteFileURL.pData ) );
    }

    /** Convert a file URL into a system dependent path.

        @param[in] ustrFileURL
        A File URL.

        @param[out] ustrSystemPath
        On success it receives the system path.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid

        @see getFileURLFromSystemPath()
    */

    static RC getSystemPathFromFileURL( const ::rtl::OUString& ustrFileURL, ::rtl::OUString& ustrSystemPath )
    {
        return static_cast< RC >( osl_getSystemPathFromFileURL( ustrFileURL.pData, &ustrSystemPath.pData ) );
    }

    /** Convert a system dependent path into a file URL.

        @param[in] ustrSystemPath
        A System dependent path of a file or directory.

        @param[out] ustrFileURL
        On success it receives the file URL.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid

        @see getSystemPathFromFileURL()
    */

    static RC getFileURLFromSystemPath( const ::rtl::OUString& ustrSystemPath, ::rtl::OUString& ustrFileURL )
    {
        return static_cast< RC >( osl_getFileURLFromSystemPath( ustrSystemPath.pData, &ustrFileURL.pData ) );
    }

    /** Search a full qualified system path or a file URL.

        @param[in] ustrFileName
        A system dependent path, a file URL, a file or relative directory

        @param[in] ustrSearchPath
        A list of system paths, in which a given file has to be searched. The Notation of a path list is
        system dependent, e.g. on UNIX system "/usr/bin:/bin" and on Windows "C:\BIN;C:\BATCH".
        These paths are only for the search of a file or a relative path, otherwise it will be ignored.
        If ustrSearchPath is NULL or while using the search path the search failed, the function searches for
        a matching file in all system directories and in the directories listed in the PATH environment
        variable.
        The value of an environment variable should be used (e.g. LD_LIBRARY_PATH) if the caller is not
        aware of the Operating System and so doesn't know which path list delimiter to use.

        @param[out] ustrFileURL
        On success it receives the full qualified file URL.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOTDIR not a directory
        @retval E_NOENT no such file or directory not found

        @see getFileURLFromSystemPath()
        @see getSystemPathFromFileURL()
    */

    static RC searchFileURL( const ::rtl::OUString& ustrFileName, const ::rtl::OUString& ustrSearchPath, ::rtl::OUString& ustrFileURL )
    {
        return static_cast< RC >( osl_searchFileURL( ustrFileName.pData, ustrSearchPath.pData, &ustrFileURL.pData ) );
    }

    /** Retrieves the file URL of the system's temporary directory path.

        @param[out] ustrTempDirURL
        On success receives the URL of system's temporary directory path.

        @retval E_None on success
        @retval E_NOENT no such file or directory not found
    */

    static RC getTempDirURL( ::rtl::OUString& ustrTempDirURL )
    {
        return static_cast< RC >( osl_getTempDirURL( &ustrTempDirURL.pData ) );
    }

    /** Creates a temporary file in the directory provided by the caller or the
        directory returned by getTempDirURL.
        Under UNIX Operating Systems the file will be created with read and write
        access for the user exclusively.
        If the caller requests only a handle to the open file but not the name of
        it, the file will be automatically removed on close else the caller is
        responsible for removing the file on success.<br><br>

        @param[in]  pustrDirectoryURL
        Specifies the full qualified URL where the temporary file should be created.
        If pustrDirectoryURL is 0 the path returned by osl_getTempDirURL will be used.

        @param[out] pHandle
        On success receives a handle to the open file.
        If pHandle is 0 the file will be closed on return, in this case
        pustrTempFileURL must not be 0.

        @param[out] pustrTempFileURL
        On success receives the full qualified URL of the temporary file.
        If pustrTempFileURL is 0 the file will be automatically removed
        on close, in this case pHandle must not be 0.
        If pustrTempFileURL is not 0 the caller receives the name of the
        created file and is responsible for removing the file.

        Description of the different pHandle, ppustrTempFileURL parameter combinations.
        pHandle is 0 and pustrTempDirURL is 0 - this combination is invalid<br>
        pHandle is not 0 and pustrTempDirURL is 0 - a handle to the open file
        will be returned on success and the file will be automatically removed on close<br>
        pHandle is 0 and pustrTempDirURL is not 0 - the name of the file will be
        returned, the caller is responsible for opening, closing and removing the file.<br>
        pHandle is not 0 and pustrTempDirURL is not 0 - a handle to the open file as well as
        the file name will be returned, the caller is responsible for closing and removing
        the file.<br>

        @retval E_None   on success
        @retval E_INVAL  the format of the parameter is invalid
        @retval E_NOMEM  not enough memory for allocating structures
        @retval E_ACCES  Permission denied
        @retval E_NOENT  No such file or directory
        @retval E_NOTDIR Not a directory
        @retval E_ROFS   Read-only file system
        @retval E_NOSPC  No space left on device
        @retval E_DQUOT  Quota exceeded

        @see getTempDirURL()
    */

    static RC createTempFile(
        ::rtl::OUString* pustrDirectoryURL,
        oslFileHandle*   pHandle,
        ::rtl::OUString* pustrTempFileURL)
    {
        rtl_uString*  pustr_dir_url       = pustrDirectoryURL ? pustrDirectoryURL->pData : NULL;
        rtl_uString** ppustr_tmp_file_url = pustrTempFileURL  ? &pustrTempFileURL->pData : NULL;

        return static_cast< RC >( osl_createTempFile(pustr_dir_url, pHandle, ppustr_tmp_file_url) );
    }
};


/** The VolumeDevice class.

    @see VolumeInfo
*/

class VolumeDevice : public FileBase
{
    oslVolumeDeviceHandle   _aHandle;

public:

    /** Constructor.
    */

    VolumeDevice() : _aHandle( NULL )
    {
    }

    /** Copy constructor.

        @param rDevice
        The other volume device.
    */

    VolumeDevice( const VolumeDevice & rDevice )
    {
        _aHandle = rDevice._aHandle;
        if ( _aHandle )
            osl_acquireVolumeDeviceHandle( _aHandle );
    }

    /** Destructor.
    */

    ~VolumeDevice()
    {
        if ( _aHandle )
            osl_releaseVolumeDeviceHandle( _aHandle );
    }

    /** Assignment operator.

        @param rDevice
        The other volume device.
    */

    VolumeDevice & operator =( const VolumeDevice & rDevice )
    {
        oslVolumeDeviceHandle   newHandle = rDevice._aHandle;

        if ( newHandle )
            osl_acquireVolumeDeviceHandle( newHandle );

        if ( _aHandle )
            osl_releaseVolumeDeviceHandle( _aHandle );

        _aHandle = newHandle;

        return *this;
    }

    /** Get the full qualified URL where a device is mounted to.

        @return
        The full qualified URL where the device is mounted to.
    */
    rtl::OUString getMountPath()
    {
        rtl::OUString   aPath;
        osl_getVolumeDeviceMountPath( _aHandle, &aPath.pData );
        return aPath;
    }

    friend class VolumeInfo;
};


class Directory;

/** The VolumeInfo class.

    Neither copy nor assignment is allowed for this class.

    @see Directory::getVolumeInfo
*/


class VolumeInfo
{
    oslVolumeInfo   _aInfo;
    sal_uInt32      _nMask;
    VolumeDevice    _aDevice;

    /** Copy constructor.
    */

    VolumeInfo( VolumeInfo& ) SAL_DELETED_FUNCTION;

    /** Assignment operator.
    */

    VolumeInfo& operator = ( VolumeInfo& ) SAL_DELETED_FUNCTION;

public:

    /** Constructor.

        @param nMask
        Set of flags describing the demanded information.
    */
    VolumeInfo( sal_uInt32 nMask )
        : _nMask( nMask )
    {
        memset( &_aInfo, 0, sizeof( oslVolumeInfo ));
        _aInfo.uStructSize = sizeof( oslVolumeInfo );
        _aInfo.pDeviceHandle = &_aDevice._aHandle;
    }

    ~VolumeInfo()
    {
        if( _aInfo.ustrFileSystemName )
            rtl_uString_release( _aInfo.ustrFileSystemName );
    }

    /** Check if specified fields are valid.

        @param nMask
        Set of flags for the fields to check.

        @return true if all fields are valid else false.
    */
    bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aInfo.uValidFields ) == nMask;
    }

    /** Check the remote flag.

        @return
        true if Attributes are valid and the volume is remote else false.
    */
    bool getRemoteFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_Remote) != 0;
    }

    /** Check the removable flag.

        @return
        true if attributes are valid and the volume is removable else false.
    */
    bool getRemoveableFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_Removeable) != 0;
    }

    /** Check the compact disc flag.

        @return
        true if attributes are valid and the volume is a CDROM else false.
    */

    bool getCompactDiscFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_CompactDisc) != 0;
    }

    /** Check the floppy disc flag.

        @return
        true if attributes are valid and the volume is a floppy disk else false.
    */

    bool getFloppyDiskFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_FloppyDisk) != 0;
    }

    /** Check the fixed disk flag.

        @return
        true if attributes are valid and the volume is a fixed disk else false.
    */

    bool getFixedDiskFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_FixedDisk) != 0;
    }

    /** Check the RAM disk flag.

        @return
        true if attributes are valid and the volume is a RAM disk else false.
    */

    bool getRAMDiskFlag() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_RAMDisk) != 0;
    }

    /** Determine the total space of a volume device.

        @return
        The total diskspace of this volume if this information is valid,
        0 otherwise.
    */

    sal_uInt64 getTotalSpace() const
    {
        return _aInfo.uTotalSpace;
    }

    /** Determine the free space of a volume device.

        @return
        The free diskspace of this volume if this information is valid,
        0 otherwise.
    */

    sal_uInt64 getFreeSpace() const
    {
        return _aInfo.uFreeSpace;
    }

    /** Determine the used space of a volume device.

        @return
        The used diskspace of this volume if this information is valid,
        0 otherwise.
    */

    sal_uInt64 getUsedSpace() const
    {
        return _aInfo.uUsedSpace;
    }

    /** Determine the maximal length of a file name.

        @return
        The maximal length of a file name if this information is valid,
        0 otherwise.
    */

    sal_uInt32 getMaxNameLength() const
    {
        return _aInfo.uMaxNameLength;
    }

    /** Determine the maximal length of a path name.

        @return
        The maximal length of a path if this information is valid,
        0 otherwise.
    */

    sal_uInt32 getMaxPathLength() const
    {
        return _aInfo.uMaxPathLength;
    }

    /** Determine the name of the volume device's File System.

        @return
        The name of the volume's filesystem if this information is valid,
        otherwise an empty string.
    */

    ::rtl::OUString getFileSystemName() const
    {
        return _aInfo.ustrFileSystemName ? ::rtl::OUString( _aInfo.ustrFileSystemName ) : ::rtl::OUString();
    }


    /** Get the volume device handle.

        @return
        The device handle of the volume if this information is valid,
        otherwise returns NULL;
    */

    VolumeDevice getDeviceHandle() const
    {
        return _aDevice;
    }

    /** Return whether the file system is case sensitive or
        case insensitive

        @return
        true if the file system is case sensitive false otherwise
    */
    bool isCaseSensitiveFileSystem() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_Case_Sensitive) != 0;
    }

    /** Return whether the file system preserves the case of
        file and directory names or not

        @return
        true if the file system preserves the case of file and
        directory names false otherwise
    */
    bool isCasePreservingFileSystem() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_Case_Is_Preserved) != 0;
    }

    friend class Directory;
};


class DirectoryItem;

/** The FileStatus class.

    @see DirectoryItem::getFileStatus
*/

class FileStatus
{
    oslFileStatus   _aStatus;
    sal_uInt32      _nMask;

    /** Copy constructor.
    */

    FileStatus( FileStatus& ) SAL_DELETED_FUNCTION;

    /** Assignment operator.
    */

    FileStatus& operator = ( FileStatus& ) SAL_DELETED_FUNCTION;

public:

    enum Type {
        Directory = osl_File_Type_Directory,
        Volume    = osl_File_Type_Volume,
        Regular   = osl_File_Type_Regular,
        Fifo      = osl_File_Type_Fifo,
        Socket    = osl_File_Type_Socket,
        Link      = osl_File_Type_Link,
        Special   = osl_File_Type_Special,
        Unknown   = osl_File_Type_Unknown
    };

    /** Constructor.

        @param nMask
        Set of flags describing the demanded information.
    */
    FileStatus(sal_uInt32 nMask)
        : _nMask(nMask)
    {
        memset(&_aStatus, 0, sizeof(_aStatus));
        _aStatus.uStructSize = sizeof(_aStatus);
    }

    /** Destructor.
    */
    ~FileStatus()
    {
        if ( _aStatus.ustrFileURL )
            rtl_uString_release( _aStatus.ustrFileURL );
        if ( _aStatus.ustrLinkTargetURL )
            rtl_uString_release( _aStatus.ustrLinkTargetURL );
        if ( _aStatus.ustrFileName )
            rtl_uString_release( _aStatus.ustrFileName );
    }

    /** Check if specified fields are valid.

        @param nMask
        Set of flags for the fields to check.

        @return
        true if all fields are valid else false.
    */

    bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aStatus.uValidFields ) == nMask;
    }

    /** Get the file type.

        @return
        The file type.
    */
    Type getFileType() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_Type), "sal.osl",
            "no FileStatus Type determined");
        return isValid(osl_FileStatus_Mask_Type)
            ? static_cast< Type >(_aStatus.eType) : Unknown;
    }

    /** Is it a directory?
        This method returns True for both directories, and volumes.

        @return
        True if it's a directory, False otherwise.

        @see getFileType
        @since LibreOffice 3.6
    */
    bool isDirectory() const
    {
        return ( getFileType() == Directory || getFileType() == Volume );
    }

    /** Is it a regular file?

        @return
        True if it's a regular file, False otherwise.

        @see getFileType
        @see isFile
        @see isLink
        @since LibreOffice 3.6
    */
    bool isRegular() const
    {
        return ( getFileType() == Regular );
    }

    /** Is it a link?

        @return
        True if it's a link, False otherwise.

        @see getFileType
        @since LibreOffice 3.6
    */
    bool isLink() const
    {
        return ( getFileType() == Link );
    }

    /** Get the file attributes.

        @return
        The set of attribute flags of this file.
    */

    sal_uInt64 getAttributes() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_Attributes), "sal.osl",
            "no FileStatus Attributes determined");
        return _aStatus.uAttributes;
    }

    /** Get the creation time of this file.

        @return
        The creation time if this information is valid, an uninitialized
        TimeValue otherwise.
    */

    TimeValue getCreationTime() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_CreationTime), "sal.osl",
            "no FileStatus CreationTime determined");
        return _aStatus.aCreationTime;
    }

    /** Get the file access time.

        @return
        The last access time if this information is valid, an uninitialized
        TimeValue otherwise.
    */

    TimeValue getAccessTime() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_AccessTime), "sal.osl",
            "no FileStatus AccessTime determined");
        return _aStatus.aAccessTime;
    }

    /** Get the file modification time.

        @return
        The last modified time if this information is valid, an uninitialized
        TimeValue otherwise.
    */

    TimeValue getModifyTime() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_ModifyTime), "sal.osl",
            "no FileStatus ModifyTime determined");
        return _aStatus.aModifyTime;
    }

    /** Get the size of the file.

        @return
        The actual file size if this information is valid, 0 otherwise.
    */

    sal_uInt64 getFileSize() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_FileSize), "sal.osl",
            "no FileStatus FileSize determined");
        return _aStatus.uFileSize;
    }

    /** Get the file name.

        @return
        The file name if this information is valid, an empty string otherwise.
    */

    ::rtl::OUString getFileName() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_FileName), "sal.osl",
            "no FileStatus FileName determined");
        return isValid(osl_FileStatus_Mask_FileName)
            ? rtl::OUString(_aStatus.ustrFileName) : rtl::OUString();
    }


    /** Get the URL of the file.

        @return
        The full qualified URL of the file if this information is valid, an
        empty string otherwise.
    */

    ::rtl::OUString getFileURL() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_FileURL), "sal.osl",
            "no FileStatus FileURL determined");
        return isValid(osl_FileStatus_Mask_FileURL)
            ? rtl::OUString(_aStatus.ustrFileURL) : rtl::OUString();
    }

    /** Get the link target URL.

        @return
        The link target URL if this information is valid, an empty string
        otherwise.
    */

    ::rtl::OUString getLinkTargetURL() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_LinkTargetURL), "sal.osl",
            "no FileStatus LinkTargetURL determined");
        return isValid(osl_FileStatus_Mask_LinkTargetURL)
            ? rtl::OUString(_aStatus.ustrLinkTargetURL) : rtl::OUString();
    }

    friend class DirectoryItem;
};


/** The file class object provides access to file contents and attributes.

    @see Directory
    @see DirectoryItem
 */

class File: public FileBase
{
    oslFileHandle   _pData;
    ::rtl::OUString _aPath;

    /** Copy constructor.
    */

    File( File& ) SAL_DELETED_FUNCTION;

    /** Assignment operator.
    */

    File& operator = ( File& ) SAL_DELETED_FUNCTION;

public:

    /** Constructor.

        @param[in]  ustrFileURL
        The full qualified URL of the file. Relative paths are not allowed.
    */

    File( const ::rtl::OUString& ustrFileURL ): _pData( NULL ), _aPath( ustrFileURL ) {}

    /** Destructor
    */

    ~File()
    {
        close();
    }

    /** Obtain the URL.

        @return
        the URL with which this File instance was created.

        @since LibreOffice 4.1
    */
    rtl::OUString getURL() const { return _aPath; }

    /** Open a regular file.

        Open a file. Only regular files can be opened.

        @param[in] uFlags
        Specifies the open mode.

        @retval E_None on success
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NAMETOOLONG pathname was too long
        @retval E_NOENT no such file or directory
        @retval E_ACCES permission denied
        @retval E_AGAIN a write lock could not be established
        @retval E_NOTDIR not a directory
        @retval E_NXIO no such device or address
        @retval E_NODEV no such device
        @retval E_ROFS read-only file system
        @retval E_TXTBSY text file busy
        @retval E_FAULT bad address
        @retval E_LOOP too many symbolic links encountered
        @retval E_NOSPC no space left on device
        @retval E_ISDIR is a directory
        @retval E_MFILE too many open files used by the process
        @retval E_NFILE too many open files in the system
        @retval E_DQUOT quota exceeded
        @retval E_EXIST file exists
        @retval E_INTR function call was interrupted
        @retval E_IO on I/O errors
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed
        @retval E_EOVERFLOW value too large for defined data type

        @see close()
        @see setPos()
        @see getPos()
        @see read()
        @see write()
        @see getSize()
        @see setSize()
    */

    RC open( sal_uInt32 uFlags )
    {
        return static_cast< RC >( osl_openFile( _aPath.pData, &_pData, uFlags ) );
    }

    /** Close an open file.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_BADF Bad file
        @retval E_INTR function call was interrupted
        @retval E_NOLINK link has been severed
        @retval E_NOSPC no space left on device
        @retval E_IO on I/O errors

        @see open()
    */

    RC close()
    {
        oslFileError Error = osl_File_E_BADF;

        if( _pData )
        {
            Error=osl_closeFile( _pData );
            _pData = NULL;
        }

        return static_cast< RC >( Error );
    }

    /** Set the internal position pointer of an open file.

        @param[in] uHow
        Distance to move the internal position pointer (from uPos).

        @param[in] uPos
        Absolute position from the beginning of the file.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files

        @see open()
        @see getPos()
    */

    SAL_WARN_UNUSED_RESULT RC setPos( sal_uInt32 uHow, sal_Int64 uPos )
    {
        return static_cast< RC >( osl_setFilePos( _pData, uHow, uPos ) );
    }

    /** Retrieve the current position of the internal pointer of an open file.

        @param[out] uPos
        On success receives the current position of the file pointer.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files

        @see open()
        @see setPos()
        @see read()
        @see write()
    */

    RC getPos( sal_uInt64& uPos )
    {
        return static_cast< RC >( osl_getFilePos( _pData, &uPos ) );
    }

    /** Test if the end of a file is reached.

        @param[out] pIsEOF
        Points to a variable that receives the end-of-file status.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_INTR function call was interrupted
        @retval E_IO on I/O errors
        @retval E_ISDIR is a directory
        @retval E_BADF bad file
        @retval E_FAULT bad address
        @retval E_AGAIN operation would block
        @retval E_NOLINK link has been severed

        @see open()
        @see read()
        @see readLine()
        @see setPos()
    */

    RC isEndOfFile( sal_Bool *pIsEOF )
    {
        return static_cast< RC >( osl_isEndOfFile( _pData, pIsEOF ) );
    }

    /** Set the file size of an open file.

        Sets the file size of an open file. The file can be truncated or enlarged by the function.
        The position of the file pointer is not affeced by this function.

        @param[in] uSize
        New size in bytes.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_OVERFLOW the resulting file offset would be a value which cannot  be represented correctly for regular files

        @see open()
        @see setPos()
        @see getStatus()
    */

    RC setSize( sal_uInt64 uSize )
    {
        return static_cast< RC >( osl_setFileSize( _pData, uSize ) );
    }

    /** Get the file size of an open file.

        Gets the file size of an open file.
        The position of the file pointer is not affeced by this function.

        @param[out] rSize
        Current size in bytes.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_OVERFLOW the resulting file offset would be a value which cannot  be represented correctly for regular files

        @see open()
        @see setPos()
        @see getSize()
        @see setSize()
        @see getStatus()
    */

    RC getSize( sal_uInt64 &rSize )
    {
        return static_cast< RC >( osl_getFileSize( _pData, &rSize ) );
    }

    /** Read a number of bytes from a file.

        Reads a number of bytes from a file. The internal file pointer is
        increased by the number of bytes read.

        @param[out] pBuffer
        Points to a buffer which receives data. The buffer must be large enough
        to hold uBytesRequested bytes.

        @param[in] uBytesRequested
        Number of bytes which should be retrieved.

        @param[out] rBytesRead
        On success the number of bytes which have actually been retrieved.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_INTR function call was interrupted
        @retval E_IO on I/O errors
        @retval E_ISDIR is a directory
        @retval E_BADF bad file
        @retval E_FAULT bad address
        @retval E_AGAIN operation would block
        @retval E_NOLINK link has been severed

        @see open()
        @see write()
        @see readLine()
        @see setPos()
    */

    RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
    {
        return static_cast< RC >( osl_readFile( _pData, pBuffer, uBytesRequested, &rBytesRead ) );
    }

    /** Write a number of bytes to a file.

        Writes a number of bytes to a file.
        The internal file pointer is increased by the number of bytes read.

        @param[in] pBuffer
        Points to a buffer which contains the data.

        @param[in] uBytesToWrite
        Number of bytes which should be written.

        @param[out] rBytesWritten
        On success the number of bytes which have actually been written.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_FBIG file too large
        @retval E_DQUOT quota exceeded
        @retval E_AGAIN operation would block
        @retval E_BADF bad file
        @retval E_FAULT bad address
        @retval E_INTR function call was interrupted
        @retval E_IO on I/O errors
        @retval E_NOLCK no record locks available
        @retval E_NOLINK link has been severed
        @retval E_NOSPC no space left on device
        @retval E_NXIO no such device or address

        @see open()
        @see read()
        @see setPos()
    */

    RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
    {
        return static_cast< RC >( osl_writeFile( _pData, pBuffer, uBytesToWrite, &rBytesWritten ) );
    }


    /** Read a line from a file.

        Reads a line from a file. The new line delimiter is NOT returned!

        @param[in,out] aSeq
        A reference to a ::rtl::ByteSequence that will hold the line read on success.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_INTR function call was interrupted
        @retval E_IO on I/O errors
        @retval E_ISDIR is a directory
        @retval E_BADF bad file
        @retval E_FAULT bad address
        @retval E_AGAIN operation would block
        @retval E_NOLINK link has been severed

        @see open()
        @see read()
        @see write()
        @see setPos()
    */

    RC readLine( ::rtl::ByteSequence& aSeq )
    {
        return static_cast< RC >( osl_readLine( _pData, reinterpret_cast<sal_Sequence**>(&aSeq) ) );
    }

    /** Synchronize the memory representation of a file with that on the physical medium.

        The function ensures that all modified data and attributes of the file associated with
        the given file handle have been written to the physical medium.
        In case the hard disk has a write cache enabled, the data may not really be on
        permanent storage when osl_syncFile returns.

        @retval E_None On success
        @retval E_INVAL The value of the input parameter is invalid
        @retval E_BADF The file is not open for writing
        @retval E_IO An I/O error occurred
        @retval E_NOSPC There is no enough space on the target device
        @retval E_ROFS The file is located on a read only file system
        @retval E_TIMEDOUT A remote connection timed out. This may happen when a file is on a remote location

        @see osl_syncFile()
        @see open()
        @see write()
    */
    RC sync() const
    {
        OSL_PRECOND(_pData, "File::sync(): File not open");
        return static_cast< RC >(osl_syncFile(_pData));
    }

    /** Copy a file to a new destination.

        Copies a file to a new destination. Copies only files not directories.
        No assumptions should be made about preserving attributes or file time.

        @param[in] ustrSourceFileURL
        Full qualified URL of the source file.

        @param[in] ustrDestFileURL
        Full qualified URL of the destination file. A directory is NOT a valid destination file!

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_PERM operation not permitted
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file or directory
        @retval E_ISDIR is a directory
        @retval E_ROFS read-only file system

        @see move()
        @see remove()
    */

    static RC copy( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    {
        return static_cast< RC >( osl_copyFile( ustrSourceFileURL.pData, ustrDestFileURL.pData ) );
    }

    /** Move a file or directory to a new destination or renames it.

        Moves a file or directory to a new destination or renames it.
        File time and attributes are preserved.

        @param[in] ustrSourceFileURL
        Full qualified URL of the source file.

        @param[in] ustrDestFileURL
        Full qualified URL of the destination file. An existing directory is NOT a valid destination !

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_PERM operation not permitted
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file or directory
        @retval E_ROFS read-only file system

        @see copy()
    */

    static RC move( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    {
        return static_cast< RC >( osl_moveFile( ustrSourceFileURL.pData, ustrDestFileURL.pData ) );
    }

    /** Move a file to a new destination or rename it, taking old file's identity (if exists).

        Moves or renames a file, replacing an existing file if exist. If the old file existed,
        moved file's metadata, e.g. creation time (on FSes which keep files' creation time) or
        ACLs, are set to old one's (to keep the old file's identity) - currently this is only
        implemented fully on Windows; on other platforms, this is mostly equivalent to move().

        @param[in] ustrSourceFileURL
        Full qualified URL of the source file.

        @param[in] ustrDestFileURL
        Full qualified URL of the destination file.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_PERM operation not permitted
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file
        @retval E_ROFS read-only file system
        @retval E_BUSY device or resource busy

        @see move()

        @since LibreOffice 6.2
    */
    static RC replace(const ::rtl::OUString& ustrSourceFileURL,
                      const ::rtl::OUString& ustrDestFileURL)
    {
        return static_cast<RC>(osl_replaceFile(ustrSourceFileURL.pData, ustrDestFileURL.pData));
    }

    /** Remove a regular file.

        @param[in] ustrFileURL
        Full qualified URL of the file to remove.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_PERM operation not permitted
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file or directory
        @retval E_ISDIR is a directory
        @retval E_ROFS read-only file system
        @retval E_FAULT bad address
        @retval E_LOOP too many symbolic links encountered
        @retval E_IO on I/O errors
        @retval E_BUSY device or resource busy
        @retval E_INTR function call was interrupted
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed
        @retval E_TXTBSY text file busy

        @see open()
    */

    static RC remove( const ::rtl::OUString& ustrFileURL )
    {
        return static_cast< RC >( osl_removeFile( ustrFileURL.pData ) );
    }

    /** Set file attributes.

        @param[in] ustrFileURL
        The full qualified file URL.

        @param[in] uAttributes
        Attributes of the file to be set.

        @return
        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid

        @see FileStatus
    */

    static RC setAttributes( const ::rtl::OUString& ustrFileURL, sal_uInt64 uAttributes )
    {
        return static_cast< RC >( osl_setFileAttributes( ustrFileURL.pData, uAttributes ) );
    }

    /** Set the file time.

        @param[in] ustrFileURL
        The full qualified URL of the file.

        @param[in] rCreationTime
        Creation time of the given file.

        @param[in] rLastAccessTime
        Time of the last access of the given file.

        @param[in] rLastWriteTime
        Time of the last modifying of the given file.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOENT no such file or directory not found

        @see FileStatus
    */

    static RC setTime(
        const ::rtl::OUString& ustrFileURL,
        const TimeValue& rCreationTime,
        const TimeValue& rLastAccessTime,
        const TimeValue& rLastWriteTime )
    {
        return static_cast< RC >( osl_setFileTime(
            ustrFileURL.pData,
            &rCreationTime,
            &rLastAccessTime,
            &rLastWriteTime ) );
    }

    friend class DirectoryItem;
};


/** The directory item class object provides access to file status information.

    @see FileStatus
 */

class DirectoryItem: public FileBase
{
    oslDirectoryItem _pData;

public:

    /** Constructor.
    */

    DirectoryItem(): _pData( NULL )
    {
    }

    /** Copy constructor.
    */

    DirectoryItem( const DirectoryItem& rItem ): _pData( rItem._pData)
    {
        if( _pData )
            osl_acquireDirectoryItem( _pData );
    }

    /** Destructor.
    */

    ~DirectoryItem()
    {
        if( _pData )
            osl_releaseDirectoryItem( _pData );
    }

    /** Assignment operator.
    */

    DirectoryItem& operator=(const DirectoryItem& rItem )
    {
        if (&rItem != this)
        {
            if( _pData )
                osl_releaseDirectoryItem( _pData );

            _pData = rItem._pData;

            if( _pData )
                osl_acquireDirectoryItem( _pData );
        }
        return *this;
    }

    /** Check for validity of this instance.

        @return
        true if object is valid directory item else false.
     */

    bool is()
    {
        return _pData != NULL;
    }

    /** Retrieve a single directory item.

        Retrieves a single directory item. The returned handle has an initial refcount of 1.
        Due to performance issues it is not recommended to use this function while
        enumerating the contents of a directory. In this case use osl_getNextDirectoryItem() instead.

        @param[in] ustrFileURL
        An absolute file URL.

        @param[out] rItem
        On success it receives a handle which can be used for subsequent calls to osl_getFileStatus().
        The handle has to be released by a call to osl_releaseDirectoryItem().

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_MFILE too many open files used by the process
        @retval E_NFILE too many open files in the system
        @retval E_NOENT no such file or directory
        @retval E_LOOP  too many symbolic links encountered
        @retval E_NAMETOOLONG the file name is too long
        @retval E_NOTDIR a component of the path prefix of path is not a directory
        @retval E_IO on I/O errors
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed
        @retval E_FAULT bad address
        @retval E_INTR the function call was interrupted

        @see FileStatus
        @see Directory::getNextItem()
    */

    static RC get( const ::rtl::OUString& ustrFileURL, DirectoryItem& rItem )
    {
        if( rItem._pData)
        {
            osl_releaseDirectoryItem( rItem._pData );
            rItem._pData = NULL;
        }

        return static_cast< RC >( osl_getDirectoryItem( ustrFileURL.pData, &rItem._pData ) );
    }

    /** Retrieve information about a single file or directory.

        @param[in,out] rStatus
        Reference to a class which receives the information of the file or directory
        represented by this directory item.

        @retval E_None on success
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_INVAL the format of the parameters was not valid
        @retval E_LOOP too many symbolic links encountered
        @retval E_ACCES permission denied
        @retval E_NOENT no such file or directory
        @retval E_NAMETOOLONG file name too long
        @retval E_BADF invalid oslDirectoryItem parameter
        @retval E_FAULT bad address
        @retval E_OVERFLOW value too large for defined data type
        @retval E_INTR function call was interrupted
        @retval E_NOLINK link has been severed
        @retval E_MULTIHOP components of path require hopping to multiple remote machines and the file system does not allow it
        @retval E_MFILE too many open files used by the process
        @retval E_NFILE too many open files in the system
        @retval E_NOSPC no space left on device
        @retval E_NXIO no such device or address
        @retval E_IO on I/O errors
        @retval E_NOSYS function not implemented

        @see get()
        @see Directory::getNextItem()
        @see FileStatus
    */

    RC getFileStatus( FileStatus& rStatus )
    {
        return static_cast< RC >( osl_getFileStatus( _pData, &rStatus._aStatus, rStatus._nMask ) );
    }

/** Determine if a directory item point the same underlying file

    The comparison is done first by URL, and then by resolving links to
    find the target, and finally by comparing inodes on unix.

    @param[in]  pOther
    A directory handle to compare with the underlying object's item

    @retval true if the items point to an identical resource<br>
    @retval false if the items point to a different resource, or a fatal error occurred<br>

    @see osl_getDirectoryItem()

    @since LibreOffice 3.6
*/
    bool isIdenticalTo( const DirectoryItem &pOther )
    {
        return osl_identicalDirectoryItem( _pData, pOther._pData );
    }

    friend class Directory;
};


/** Base class for observers of directory creation notifications.

    Clients which uses the method createDirectoryPath of the class
    Directory may want to be informed about the directories that
    have been created. This may be accomplished by deriving from
    this base class and overwriting the virtual function
    DirectoryCreated.

    @see Directory::createPath
*/
class DirectoryCreationObserver
{
public:
    virtual ~DirectoryCreationObserver() {}

    /** This method will be called when a new directory has been
        created and needs to be overwritten by derived classes.
        You must not delete the directory that was just created
        otherwise you will run into an endless loop.

        @param aDirectoryUrl
        [in]The absolute file URL of the directory that was just created by
        ::osl::Directory::createPath.
    */
    virtual void DirectoryCreated(const rtl::OUString& aDirectoryUrl) = 0;
};


// This just an internal helper function for
// private use.
extern "C" inline void SAL_CALL onDirectoryCreated(void* pData, rtl_uString* aDirectoryUrl)
{
    static_cast<DirectoryCreationObserver*>(pData)->DirectoryCreated(aDirectoryUrl);
}

/** The directory class object provides an enumeration of DirectoryItems.

    @see DirectoryItem
    @see File
 */

class Directory: public FileBase
{
    oslDirectory    _pData;
    ::rtl::OUString _aPath;

    /** Copy constructor.
    */

    Directory( Directory& ) SAL_DELETED_FUNCTION;

    /**  Assignment operator.
    */

    Directory& operator = ( Directory& ) SAL_DELETED_FUNCTION;

public:

    /** Constructor.

        @param[in] strPath
        The full qualified URL of the directory.
        Relative URLs are not allowed.
     */

    Directory( const ::rtl::OUString& strPath ): _pData( NULL ), _aPath( strPath )
    {
    }

    /** Destructor.
    */

    ~Directory()
    {
        close();
    }

    /** Obtain the URL.

        @return
        the URL with which this Directory instance was created.

        @since LibreOffice 4.1
    */
    rtl::OUString getURL() const { return _aPath; }

    /** Open a directory for enumerating its contents.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOENT the specified path doesn't exist
        @retval E_NOTDIR the specified path is not a directory
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_MFILE too many open files used by the process
        @retval E_NFILE too many open files in the system
        @retval E_NAMETOOLONG File name too long
        @retval E_LOOP Too many symbolic links encountered

        @see getNextItem()
        @see close()
    */

    RC open()
    {
        return static_cast< RC >( osl_openDirectory( _aPath.pData, &_pData ) );
    }

    /** Query if directory is open.

        Query if directory is open and so item enumeration is valid.

        @retval true if the directory is open else false.

        @see open()
        @see close()
    */

    bool isOpen() { return _pData != NULL; }

    /** Close a directory.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_BADF invalid oslDirectory parameter
        @retval E_INTR the function call was interrupted

        @see open()
    */

    RC close()
    {
        oslFileError Error = osl_File_E_BADF;

        if( _pData )
        {
            Error=osl_closeDirectory( _pData );
            _pData = NULL;
        }

        return static_cast< RC >( Error );
    }


    /** Resets the directory item enumeration to the beginning.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOENT the specified path doesn't exist
        @retval E_NOTDIR the specified path is not a directory
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_ACCES permission denied
        @retval E_MFILE too many open files used by the process
        @retval E_NFILE too many open files in the system
        @retval E_NAMETOOLONG File name too long
        @retval E_LOOP Too many symbolic links encountered

        @see open()
    */

    RC reset()
    {
        close();
        return open();
    }

    /** Retrieve the next item of a previously opened directory.

        Retrieves the next item of a previously opened directory.

        @param[out] rItem
        On success a valid DirectoryItem.

        @param[in]  nHint
        With this parameter the caller can tell the implementation that (s)he
        is going to call this function uHint times afterwards. This enables the implementation to
        get the information for more than one file and cache it until the next calls.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_NOENT no more entries in this directory
        @retval E_BADF invalid oslDirectory parameter
        @retval E_OVERFLOW the value too large for defined data type

        @see DirectoryItem
    */

    RC getNextItem( DirectoryItem& rItem, sal_uInt32 nHint = 0 )
    {
        if( rItem._pData )
        {
            osl_releaseDirectoryItem( rItem._pData );
            rItem._pData = NULL;
        }
        return static_cast<RC>(osl_getNextDirectoryItem( _pData, &rItem._pData, nHint ));
    }


    /** Retrieve information about a volume.

        Retrieves information about a volume. A volume can either be a mount point, a network
        resource or a drive depending on Operating System and File System.

        @param[in] ustrDirectoryURL
        Full qualified URL of the volume

        @param[out] rInfo
        On success it receives information about the volume.

        @retval E_None on success
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOTDIR not a directory
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file or directory
        @retval E_ACCES permission denied
        @retval E_LOOP too many symbolic links encountered
        @retval E_FAULT Bad address
        @retval E_IO on I/O errors
        @retval E_NOSYS function not implemented
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed
        @retval E_INTR function call was interrupted

        @see FileStatus
        @see VolumeInfo
    */

    static RC getVolumeInfo( const ::rtl::OUString& ustrDirectoryURL, VolumeInfo& rInfo )
    {
        return static_cast< RC >( osl_getVolumeInformation( ustrDirectoryURL.pData, &rInfo._aInfo, rInfo._nMask ) );
    }

    /** Create a directory.

        @param[in] ustrDirectoryURL
        Full qualified URL of the directory to create.

        @param[in] flags
        Optional flags, see osl_createDirectoryWithFlags for details.  This
        defaulted parameter is new since LibreOffice 4.3.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_EXIST file exists
        @retval E_ACCES permission denied
        @retval E_NAMETOOLONG file name too long
        @retval E_NOENT no such file or directory
        @retval E_NOTDIR not a directory
        @retval E_ROFS read-only file system
        @retval E_NOSPC no space left on device
        @retval E_DQUOT quota exceeded
        @retval E_LOOP too many symbolic links encountered
        @retval E_FAULT bad address
        @retval E_IO on I/O errors
        @retval E_MLINK too many links
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed

        @see remove()
    */

    static RC create(
        const ::rtl::OUString& ustrDirectoryURL,
        sal_uInt32 flags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write )
    {
        return static_cast< RC >(
            osl_createDirectoryWithFlags( ustrDirectoryURL.pData, flags ) );
    }

    /** Remove an empty directory.

        @param[in] ustrDirectoryURL
        Full qualified URL of the directory.

        @retval E_None on success
        @retval E_INVAL the format of the parameters was not valid
        @retval E_NOMEM not enough memory for allocating structures
        @retval E_PERM operation not permitted
        @retval E_ACCES permission denied
        @retval E_NOENT no such file or directory
        @retval E_NOTDIR not a directory
        @retval E_NOTEMPTY directory not empty
        @retval E_FAULT bad address
        @retval E_NAMETOOLONG file name too long
        @retval E_BUSY device or resource busy
        @retval E_ROFS read-only file system
        @retval E_LOOP too many symbolic links encountered
        @retval E_EXIST file exists
        @retval E_IO on I/O errors
        @retval E_MULTIHOP multihop attempted
        @retval E_NOLINK link has been severed

        @see create()
    */

    static RC remove( const ::rtl::OUString& ustrDirectoryURL )
    {
        return static_cast< RC >( osl_removeDirectory( ustrDirectoryURL.pData ) );
    }

    /** Create a directory path.

        The osl_createDirectoryPath function creates a specified directory path.
        All nonexisting sub directories will be created.

        @attention You cannot rely on getting the error code E_EXIST for existing
        directories. Programming against this error code is in general a strong
        indication of a wrong usage of osl_createDirectoryPath.

        @param aDirectoryUrl
        [in] The absolute file URL of the directory path to create.
        A relative file URL will not be accepted.

        @param aDirectoryCreationObserver
        [in] Pointer to an instance of type DirectoryCreationObserver that will
        be informed about the creation of a directory. The value of this
        parameter may be NULL, in this case notifications will not be sent.

        @retval E_None On success
        @retval E_INVAL The format of the parameters was not valid
        @retval E_ACCES Permission denied
        @retval E_EXIST The final node of the specified directory path already exist
        @retval E_NAMETOOLONG The name of the specified directory path exceeds the maximum allowed length
        @retval E_NOTDIR A component of the specified directory path already exist as file in any part of the directory path
        @retval E_ROFS Read-only file system
        @retval E_NOSPC No space left on device
        @retval E_DQUOT Quota exceeded
        @retval E_FAULT Bad address
        @retval E_IO I/O error
        @retval E_LOOP Too many symbolic links encountered
        @retval E_NOLINK Link has been severed
        @retval E_invalidError An unknown error occurred

        @see DirectoryCreationObserver
        @see create
    */
    static RC createPath(
        const ::rtl::OUString& aDirectoryUrl,
        DirectoryCreationObserver* aDirectoryCreationObserver = NULL)
    {
        return static_cast< RC >(osl_createDirectoryPath(
            aDirectoryUrl.pData,
            aDirectoryCreationObserver ? onDirectoryCreated : NULL,
            aDirectoryCreationObserver));
    }
};

} /* namespace osl */

#endif // INCLUDED_OSL_FILE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
