/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OSL_FILE_HXX_
#define _OSL_FILE_HXX_

#include "sal/config.h"

#include <string.h>

#include <cassert>

#include <osl/time.h>
#include <rtl/memory.h>
#include <rtl/ustring.hxx>

#include <osl/file.h>
#include <rtl/byteseq.hxx>

#include <stdio.h>

namespace osl
{


// -----------------------------------------------------------------------------
/** Base class for all File System specific objects.

    @see Directory
    @see DirectoryItem
    @see File
 */

class FileBase
{
public:

    enum RC {
        E_None         = osl_File_E_None,
        E_PERM         = osl_File_E_PERM,
        E_NOENT        = osl_File_E_NOENT,
        E_SRCH         = osl_File_E_SRCH,
        E_INTR         = osl_File_E_INTR,
        E_IO           = osl_File_E_IO,
        E_NXIO         = osl_File_E_NXIO,
        E_2BIG         = osl_File_E_2BIG,
        E_NOEXEC       = osl_File_E_NOEXEC,
        E_BADF         = osl_File_E_BADF,
        E_CHILD        = osl_File_E_CHILD,
        E_AGAIN        = osl_File_E_AGAIN,
        E_NOMEM        = osl_File_E_NOMEM,
        E_ACCES        = osl_File_E_ACCES,
        E_FAULT        = osl_File_E_FAULT,
        E_BUSY         = osl_File_E_BUSY,
        E_EXIST        = osl_File_E_EXIST,
        E_XDEV         = osl_File_E_XDEV,
        E_NODEV        = osl_File_E_NODEV,
        E_NOTDIR       = osl_File_E_NOTDIR,
        E_ISDIR        = osl_File_E_ISDIR,
        E_INVAL        = osl_File_E_INVAL,
        E_NFILE        = osl_File_E_NFILE,
        E_MFILE        = osl_File_E_MFILE,
        E_NOTTY        = osl_File_E_NOTTY,
        E_FBIG         = osl_File_E_FBIG,
        E_NOSPC        = osl_File_E_NOSPC,
        E_SPIPE        = osl_File_E_SPIPE,
        E_ROFS         = osl_File_E_ROFS,
        E_MLINK        = osl_File_E_MLINK,
        E_PIPE         = osl_File_E_PIPE,
        E_DOM          = osl_File_E_DOM,
        E_RANGE        = osl_File_E_RANGE,
        E_DEADLK       = osl_File_E_DEADLK,
        E_NAMETOOLONG  = osl_File_E_NAMETOOLONG,
        E_NOLCK        = osl_File_E_NOLCK,
        E_NOSYS        = osl_File_E_NOSYS,
        E_NOTEMPTY     = osl_File_E_NOTEMPTY,
        E_LOOP         = osl_File_E_LOOP,
        E_ILSEQ        = osl_File_E_ILSEQ,
        E_NOLINK       = osl_File_E_NOLINK,
        E_MULTIHOP     = osl_File_E_MULTIHOP,
        E_USERS        = osl_File_E_USERS,
        E_OVERFLOW     = osl_File_E_OVERFLOW,
        E_NOTREADY     = osl_File_E_NOTREADY,
        E_invalidError = osl_File_E_invalidError,   /* unmapped error: always last entry in enum! */
        E_TIMEDOUT     = osl_File_E_TIMEDOUT,
        E_NETWORK      = osl_File_E_NETWORK
    };


public:

    /** Determine a valid unused canonical name for a requested name.

        Determines a valid unused canonical name for a requested name.
        Depending on the Operating System and the File System the illegal characters are replaced by valid ones.
        If a file or directory with the requested name already exists a new name is generated following
        the common rules on the actual Operating System and File System.

        @param ustrRequestedURL [in]
        Requested name of a file or directory.

        @param ustrValidURL [out]
        On success receives a name which is unused and valid on the actual Operating System and
        File System.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid

        @see DirectoryItem::getFileStatus()
    */

    static inline RC getCanonicalName( const ::rtl::OUString& ustrRequestedURL, ::rtl::OUString& ustrValidURL )
    {
        return (RC) osl_getCanonicalName( ustrRequestedURL.pData, &ustrValidURL.pData );
    }

    /** Convert a path relative to a given directory into an full qualified file URL.

        Convert a path relative to a given directory into an full qualified file URL.
        The function resolves symbolic links if possible and path ellipses, so on success
        the resulting absolute path is fully resolved.

        @param ustrBaseDirectoryURL [in]
        Base directory URL to which the relative path is related to.

        @param ustrRelativeFileURL [in]
        An URL of a file or directory relative to the directory path specified by ustrBaseDirectoryURL
        or an absolute path.
        If ustrRelativeFileURL denotes an absolute path ustrBaseDirectoryURL will be ignored.

        @param ustrAbsoluteFileURL [out]
        On success it receives the full qualified absoulte file URL.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_NOTDIR not a directory
        E_ACCES permission denied
        E_NOENT no such file or directory
        E_NAMETOOLONG file name too long
        E_OVERFLOW value too large for defined data type
        E_FAULT bad address
        E_INTR function call was interrupted
        E_LOOP too many symbolic links encountered
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed

        @see DirectoryItem::getFileStatus()
    */

    static inline RC getAbsoluteFileURL( const ::rtl::OUString& ustrBaseDirectoryURL, const ::rtl::OUString& ustrRelativeFileURL, ::rtl::OUString& ustrAbsoluteFileURL )
    {
        return (RC) osl_getAbsoluteFileURL( ustrBaseDirectoryURL.pData, ustrRelativeFileURL.pData, &ustrAbsoluteFileURL.pData );
    }

    /** Convert a file URL into a system dependend path.

        @param ustrFileURL [in]
        A File URL.

        @param ustrSystemPath [out]
        On success it receives the system path.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid

        @see getFileURLFromSystemPath()
    */

    static inline RC getSystemPathFromFileURL( const ::rtl::OUString& ustrFileURL, ::rtl::OUString& ustrSystemPath )
    {
        return (RC) osl_getSystemPathFromFileURL( ustrFileURL.pData, &ustrSystemPath.pData );
    }

    /** Convert a system dependend path into a file URL.

        @param ustrSystemPath [in]
        A System dependent path of a file or directory.

        @param ustrFileURL [out]
        On success it receives the file URL.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid

        @see getSystemPathFromFileURL()
    */

    static inline RC getFileURLFromSystemPath( const ::rtl::OUString& ustrSystemPath, ::rtl::OUString& ustrFileURL )
    {
        return (RC) osl_getFileURLFromSystemPath( ustrSystemPath.pData, &ustrFileURL.pData );
    }

    /** Searche a full qualified system path or a file URL.

        @param ustrFileName [in]
        A system dependent path, a file URL, a file or relative directory

        @param ustrSearchPath [in]
        A list of system paths, in which a given file has to be searched. The Notation of a path list is
        system dependend, e.g. on UNIX system "/usr/bin:/bin" and on Windows "C:\BIN;C:\BATCH".
        These paths are only for the search of a file or a relative path, otherwise it will be ignored.
        If ustrSearchPath is NULL or while using the search path the search failed, the function searches for
        a matching file in all system directories and in the directories listed in the PATH environment
        variable.
        The value of an environment variable should be used (e.g. LD_LIBRARY_PATH) if the caller is not
        aware of the Operating System and so doesn't know which path list delimiter to use.

        @param ustrFileURL [out]
        On success it receives the full qualified file URL.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOTDIR not a directory
        E_NOENT no such file or directory not found

        @see getFileURLFromSystemPath()
        @see getSystemPathFromFileURL()
    */

    static inline RC searchFileURL( const ::rtl::OUString& ustrFileName, const ::rtl::OUString& ustrSearchPath, ::rtl::OUString& ustrFileURL )
    {
        return (RC) osl_searchFileURL( ustrFileName.pData, ustrSearchPath.pData, &ustrFileURL.pData );
    }

    /** Retrieves the file URL of the system's temporary directory path.

        @param[out] ustrTempDirURL
        On success receives the URL of system's temporary directory path.

        @return
        E_None on success
        E_NOENT no such file or directory not found
    */

    static inline RC getTempDirURL( ::rtl::OUString& ustrTempDirURL )
    {
        return (RC) osl_getTempDirURL( &ustrTempDirURL.pData );
    }

    /** Creates a temporary file in the directory provided by the caller or the
        directory returned by getTempDirURL.
        Under UNIX Operating Systems the file will be created with read and write
        access for the user exclusively.
        If the caller requests only a handle to the open file but not the name of
        it, the file will be automatically removed on close else the caller is
        responsible for removing the file on success.<br><br>

        @param  pustrDirectoryURL [in]
        Specifies the full qualified URL where the temporary file should be created.
        If pustrDirectoryURL is 0 the path returned by osl_getTempDirURL will be used.

        @param  pHandle [out]
        On success receives a handle to the open file.
        If pHandle is 0 the file will be closed on return, in this case
        pustrTempFileURL must not be 0.

        @param  pustrTempFileURL [out]
        On success receives the full qualified URL of the temporary file.
        If pustrTempFileURL is 0 the file will be automatically removed
        on close, in this case pHandle must not be 0.
        If pustrTempFileURL is not 0 the caller receives the name of the
        created file and is responsible for removing the file.

        @descr
        Description of the different pHandle, ppustrTempFileURL parameter combinations.
        pHandle is 0 and pustrTempDirURL is 0 - this combination is invalid<br>
        pHandle is not 0 and pustrTempDirURL is 0 - a handle to the open file
        will be returned on success and the file will be automatically removed on close<br>
        pHandle is 0 and pustrTempDirURL is not 0 - the name of the file will be
        returned, the caller is responsible for opening, closing and removing the file.<br>
        pHandle is not 0 and pustrTempDirURL is not 0 - a handle to the open file as well as
        the file name will be returned, the caller is responsible for closing and removing
        the file.<br>

        @return
        E_None   on success
        E_INVAL  the format of the parameter is invalid
        E_NOMEM  not enough memory for allocating structures
        E_ACCES  Permission denied
        E_NOENT  No such file or directory
        E_NOTDIR Not a directory
        E_ROFS   Read-only file system
        E_NOSPC  No space left on device
        E_DQUOT  Quota exceeded

        @see getTempDirURL()
    */

    static inline RC createTempFile(
        ::rtl::OUString* pustrDirectoryURL,
        oslFileHandle*   pHandle,
        ::rtl::OUString* pustrTempFileURL)
    {
        rtl_uString*  pustr_dir_url       = pustrDirectoryURL ? pustrDirectoryURL->pData : 0;
        rtl_uString** ppustr_tmp_file_url = pustrTempFileURL  ? &pustrTempFileURL->pData : 0;

        return (RC) osl_createTempFile(pustr_dir_url, pHandle, ppustr_tmp_file_url);
    }
};


// -----------------------------------------------------------------------------
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

    inline VolumeDevice & operator =( const VolumeDevice & rDevice )
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
    inline rtl::OUString getMountPath()
    {
        rtl::OUString   aPath;
        osl_getVolumeDeviceMountPath( _aHandle, &aPath.pData );
        return aPath;
    }

    friend class VolumeInfo;
};

// -----------------------------------------------------------------------------

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

    VolumeInfo( VolumeInfo& );

    /** Assginment operator.
    */

    VolumeInfo& operator = ( VolumeInfo& );

public:

    /** Constructor.

        @param nMask
        Set of flags decribing the demanded information.
    */

    VolumeInfo( sal_uInt32 nMask ): _nMask( nMask )
    {
        _aInfo.uStructSize = sizeof( oslVolumeInfo );
        memset( &_aInfo.uValidFields, 0, sizeof( oslVolumeInfo ) - sizeof( sal_uInt32 ) );
        _aInfo.pDeviceHandle = &_aDevice._aHandle;
    }

    /** Destructor.
    */

    ~VolumeInfo()
    {
        if( _aInfo.ustrFileSystemName )
            rtl_uString_release( _aInfo.ustrFileSystemName );
    }

    /** Check if specified fields are valid.

        @param nMask
        Set of flags for the fields to check.

        @return sal_True if all fields are valid else sal_False.
    */

    inline sal_Bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aInfo.uValidFields ) == nMask;
    }

    /** Check the remote flag.

        @return
        sal_True if Attributes are valid and the volume is remote else sal_False.
    */

    inline sal_Bool getRemoteFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_Remote);
    }

    /** Check the removeable flag.

        @return
        sal_True if attributes are valid and the volume is removable else sal_False.
    */

    inline sal_Bool getRemoveableFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_Removeable);
    }

    /** Check the compact disc flag.

        @return
        sal_True if attributes are valid and the volume is a CDROM else sal_False.
    */

    inline sal_Bool getCompactDiscFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_CompactDisc);
    }

    /** Check the floppy disc flag.

        @return
        sal_True if attributes are valid and the volume is a floppy disk else sal_False.
    */

    inline sal_Bool getFloppyDiskFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_FloppyDisk);
    }

    /** Check the fixed disk flag.

        @return
        sal_True if attributes are valid and the volume is a fixed disk else sal_False.
    */

    inline sal_Bool getFixedDiskFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_FixedDisk);
    }

    /** Check the RAM disk flag.

        @return
        sal_True if attributes are valid and the volume is a RAM disk else sal_False.
    */

    inline sal_Bool getRAMDiskFlag() const
    {
        return 0 != (_aInfo.uAttributes & osl_Volume_Attribute_RAMDisk);
    }

    /** Determine the total space of a volume device.

        @return
        The total diskspace of this volume if this information is valid,
        0 otherwise.
    */

    inline sal_uInt64 getTotalSpace() const
    {
        return _aInfo.uTotalSpace;
    }

    /** Determine the free space of a volume device.

        @return
        The free diskspace of this volume if this information is valid,
        0 otherwise.
    */

    inline sal_uInt64 getFreeSpace() const
    {
        return _aInfo.uFreeSpace;
    }

    /** Determine the used space of a volume device.

        @return
        The used diskspace of this volume if this information is valid,
        0 otherwise.
    */

    inline sal_uInt64 getUsedSpace() const
    {
        return _aInfo.uUsedSpace;
    }

    /** Determine the maximal length of a file name.

        @return
        The maximal length of a file name if this information is valid,
        0 otherwise.
    */

    inline sal_uInt32 getMaxNameLength() const
    {
        return _aInfo.uMaxNameLength;
    }

    /** Determine the maximal length of a path name.

        @return
        The maximal length of a path if this information is valid,
        0 otherwise.
    */

    inline sal_uInt32 getMaxPathLength() const
    {
        return _aInfo.uMaxPathLength;
    }

    /** Determine the name of the volume device's File System.

        @return
        The name of the volume's fielsystem if this information is valid,
        otherwise an empty string.
    */

    inline ::rtl::OUString getFileSystemName() const
    {
        return _aInfo.ustrFileSystemName ? ::rtl::OUString( _aInfo.ustrFileSystemName ) : ::rtl::OUString();
    }


    /** Get the volume device handle.

        @return
        The device handle of the volume if this information is valid,
        otherwise returns NULL;
    */

    inline VolumeDevice getDeviceHandle() const
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
        return (_aInfo.uAttributes & osl_Volume_Attribute_Case_Sensitive);
    }

    /** Return whether the file system preserves the case of
        file and directory names or not

        @return
        true if the file system preserves the case of file and
        directory names false otherwise
    */
    bool isCasePreservingFileSystem() const
    {
        return (_aInfo.uAttributes & osl_Volume_Attribute_Case_Is_Preserved);
    }

    friend class Directory;
};

// -----------------------------------------------------------------------------
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

    FileStatus( FileStatus& );

    /** Assignment operator.
    */

    FileStatus& operator = ( FileStatus& );

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
        Set of flags decribing the demanded information.
    */

    FileStatus( sal_uInt32 nMask ): _nMask( nMask )
    {
        _aStatus.uStructSize = sizeof( oslFileStatus );
        memset( &_aStatus.uValidFields, 0, sizeof( oslFileStatus ) - sizeof( sal_uInt32 ) );
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
        sal_True if all fields are valid else sal_False.
    */

    inline sal_Bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aStatus.uValidFields ) == nMask;
    }

    /** Get the file type.

        @return
        The file type.
    */
    inline Type getFileType() const
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
    inline sal_Bool isDirectory() const
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
    inline sal_Bool isRegular() const
    {
        return ( getFileType() == Regular );
    }

    /** Is it a link?

        @return
        True if it's a link, False otherwise.

        @see getFileType
        @since LibreOffice 3.6
    */
    inline sal_Bool isLink() const
    {
        return ( getFileType() == Link );
    }

    /** Get the file attributes.

        @return
        The set of attribute flags of this file.
    */

    inline sal_uInt64 getAttributes() const
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

    inline TimeValue getCreationTime() const
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

    inline TimeValue getAccessTime() const
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

    inline TimeValue getModifyTime() const
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

    inline sal_uInt64 getFileSize() const
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

    inline ::rtl::OUString getFileName() const
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

    inline ::rtl::OUString getFileURL() const
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

    inline ::rtl::OUString getLinkTargetURL() const
    {
        SAL_INFO_IF(
            !isValid(osl_FileStatus_Mask_LinkTargetURL), "sal.osl",
            "no FileStatus LinkTargetURL determined");
        return isValid(osl_FileStatus_Mask_LinkTargetURL)
            ? rtl::OUString(_aStatus.ustrLinkTargetURL) : rtl::OUString();
    }

    friend class DirectoryItem;
};


// -----------------------------------------------------------------------------
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

    File( File& );

    /** Assginment operator.
    */

    File& operator = ( File& );

public:

    /** Constructor.

        @param  ustrFileURL [in]
        The full qualified URL of the file. Relative paths are not allowed.
    */

    File( const ::rtl::OUString& ustrFileURL ): _pData( 0 ), _aPath( ustrFileURL ) {}

    /** Destructor
    */

    inline ~File()
    {
        close();
    }

    /** Open a regular file.

        Open a file. Only regular files can be openend.

        @param uFlags [in]
        Specifies the open mode.

        @return
        E_None on success
        E_NOMEM not enough memory for allocating structures
        E_INVAL the format of the parameters was not valid
        E_NAMETOOLONG pathname was too long
        E_NOENT no such file or directory
        E_ACCES permission denied
        E_AGAIN a write lock could not be established
        E_NOTDIR not a directory
        E_NXIO no such device or address
        E_NODEV no such device
        E_ROFS read-only file system
        E_TXTBSY text file busy
        E_FAULT bad address
        E_LOOP too many symbolic links encountered
        E_NOSPC no space left on device
        E_ISDIR is a directory
        E_MFILE too many open files used by the process
        E_NFILE too many open files in the system
        E_DQUOT quota exceeded
        E_EXIST file exists
        E_INTR function call was interrupted
        E_IO on I/O errors
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed
        E_EOVERFLOW value too large for defined data type

        @see close()
        @see setPos()
        @see getPos()
        @see read()
        @see write()
        @see getSize()
        @see setSize()
    */

    inline RC open( sal_uInt32 uFlags )
    {
        return (RC) osl_openFile( _aPath.pData, &_pData, uFlags );
    }

    /** Close an open file.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_BADF Bad file
        E_INTR function call was interrupted
        E_NOLINK link has been severed
        E_NOSPC no space left on device
        E_IO on I/O errors

        @see open()
    */

    inline RC close()
    {
        oslFileError Error = osl_File_E_BADF;

        if( _pData )
        {
            Error=osl_closeFile( _pData );
            _pData = NULL;
        }

        return (RC) Error;
    }

    /** Set the internal position pointer of an open file.

        @param uHow [in]
        Distance to move the internal position pointer (from uPos).

        @param uPos [in]
        Absolute position from the beginning of the file.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files

        @see open()
        @see getPos()
    */

    inline RC setPos( sal_uInt32 uHow, sal_Int64 uPos ) SAL_WARN_UNUSED_RESULT
    {
        return (RC) osl_setFilePos( _pData, uHow, uPos );
    }

    /** Retrieve the current position of the internal pointer of an open file.

        @param uPos [out]
        On success receives the current position of the file pointer.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_OVERFLOW the resulting file offset would be a value which cannot be represented correctly for regular files

        @see open()
        @see setPos()
        @see read()
        @see write()
    */

    inline RC getPos( sal_uInt64& uPos )
    {
        return (RC) osl_getFilePos( _pData, &uPos );
    }

    /** Test if the end of a file is reached.

        @param pIsEOF [out]
        Points to a variable that receives the end-of-file status.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_INTR function call was interrupted
        E_IO on I/O errors
        E_ISDIR is a directory
        E_BADF bad file
        E_FAULT bad address
        E_AGAIN operation would block
        E_NOLINK link has been severed

        @see open()
        @see read()
        @see readLine()
        @see setPos()
    */

    inline RC isEndOfFile( sal_Bool *pIsEOF )
    {
        return (RC) osl_isEndOfFile( _pData, pIsEOF );
    }

    /** Set the file size of an open file.

        Sets the file size of an open file. The file can be truncated or enlarged by the function.
        The position of the file pointer is not affeced by this function.

        @param uSize [in]
        New size in bytes.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_OVERFLOW the resulting file offset would be a value which cannot  be represented correctly for regular files

        @see open()
        @see setPos()
        @see getStatus()
    */

    inline RC setSize( sal_uInt64 uSize )
    {
        return (RC) osl_setFileSize( _pData, uSize );
    }

    /** Get the file size of an open file.

        Gets the file size of an open file.
        The position of the file pointer is not affeced by this function.

        @param rSize [out]
        Current size in bytes.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_OVERFLOW the resulting file offset would be a value which cannot  be represented correctly for regular files

        @see open()
        @see setPos()
        @see getSize()
        @see setSize()
        @see getStatus()
    */

    inline RC getSize( sal_uInt64 &rSize )
    {
        return (RC) osl_getFileSize( _pData, &rSize );
    }

    /** Read a number of bytes from a file.

        Reads a number of bytes from a file. The internal file pointer is
        increased by the number of bytes read.

        @param pBuffer [out]
        Points to a buffer which receives data. The buffer must be large enough
        to hold uBytesRequested bytes.

        @param uBytesRequested [in]
        Number of bytes which should be retrieved.

        @param rBytesRead [out]
        On success the number of bytes which have actually been retrieved.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_INTR function call was interrupted
        E_IO on I/O errors
        E_ISDIR is a directory
        E_BADF bad file
        E_FAULT bad address
        E_AGAIN operation would block
        E_NOLINK link has been severed

        @see open()
        @see write()
        @see readLine()
        @see setPos()
    */

    inline RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
    {
        return (RC) osl_readFile( _pData, pBuffer, uBytesRequested, &rBytesRead );
    }

    /** Write a number of bytes to a file.

        Writes a number of bytes to a file.
        The internal file pointer is increased by the number of bytes read.

        @param pBuffer [in]
        Points to a buffer which contains the data.

        @param uBytesToWrite [in]
        Number of bytes which should be written.

        @param rBytesWritten [out]
        On success the number of bytes which have actually been written.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_FBIG file too large
        E_DQUOT quota exceeded
        E_AGAIN operation would block
        E_BADF bad file
        E_FAULT bad address
        E_INTR function call was interrupted
        E_IO on I/O errosr
        E_NOLCK no record locks available
        E_NOLINK link has been severed
        E_NOSPC no space left on device
        E_NXIO no such device or address

        @see open()
        @see read()
        @see setPos()
    */

    inline RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
    {
        return (RC) osl_writeFile( _pData, pBuffer, uBytesToWrite, &rBytesWritten );
    }


    /** Read a line from a file.

        Reads a line from a file. The new line delimiter is NOT returned!

        @param  aSeq [in/out]
        A reference to a ::rtl::ByteSequence that will hold the line read on success.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_INTR function call was interrupted
        E_IO on I/O errors
        E_ISDIR is a directory
        E_BADF bad file
        E_FAULT bad address
        E_AGAIN operation would block
        E_NOLINK link has been severed

        @see open()
        @see read()
        @see write()
        @see setPos()
    */

    inline RC readLine( ::rtl::ByteSequence& aSeq )
    {
        return (RC) osl_readLine( _pData, reinterpret_cast<sal_Sequence**>(&aSeq) );
    }

    /** Synchronize the memory representation of a file with that on the physical medium.

    The function ensures that all modified data and attributes of the file associated with
    the given file handle have been written to the physical medium.
    In case the hard disk has a write cache enabled, the data may not really be on
    permanent storage when osl_syncFile returns.

    @return
    <dl>
    <dt>E_None</dt>
    <dd>On success</dd>
    <dt>E_INVAL</dt>
    <dd>The value of the input parameter is invalid</dd>
    </dl>
    <br><p><strong>In addition to these error codes others may occur as well, for instance:</strong></p><br>
    <dt>E_BADF</dt>
    <dd>The file is not open for writing</dd>
    <dt>E_IO</dt>
    <dd>An I/O error occurred</dd>
    <dt>E_NOSPC</dt>
    <dd>There is no enough space on the target device</dd>
    <dt>E_ROFS</dt>
    <dd>The file is located on a read only file system</dd>
    <dt>E_TIMEDOUT</dt>
    <dd>A remote connection timed out. This may happen when a file is on a remote location</dd>
    </dl>

    @see osl_syncFile()
    @see open()
    @see write()
    */
    inline RC sync() const
    {
        OSL_PRECOND(_pData, "File::sync(): File not open");
        return (RC)osl_syncFile(_pData);
    }

    /** Copy a file to a new destination.

        Copies a file to a new destination. Copies only files not directories.
        No assumptions should be made about preserving attributes or file time.

        @param ustrSourceFileURL [in]
        Full qualified URL of the source file.

        @param ustrDestFileURL [in]
        Full qualified URL of the destination file. A directory is NOT a valid destination file!

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_PERM operation not permitted
        E_NAMETOOLONG file name too long
        E_NOENT no such file or directory
        E_ISDIR is a directory
        E_ROFS read-only file system

        @see move()
        @see remove()
    */

    inline static RC copy( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    {
        return (RC) osl_copyFile( ustrSourceFileURL.pData, ustrDestFileURL.pData );
    }

    /** Move a file or directory to a new destination or renames it.

        Moves a file or directory to a new destination or renames it.
        File time and attributes are preserved.

        @param ustrSourceFileURL [in]
        Full qualified URL of the source file.

        @param ustrDestFileURL [in]
        Full qualified URL of the destination file. An existing directory is NOT a valid destination !

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_PERM operation not permitted
        E_NAMETOOLONG file name too long
        E_NOENT no such file or directory
        E_ROFS read-only file system

        @see copy()
    */

    inline static RC move( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    {
        return (RC) osl_moveFile( ustrSourceFileURL.pData, ustrDestFileURL.pData );
    }

    /** Remove a regular file.

        @param ustrFileURL [in]
        Full qualified URL of the file to remove.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_PERM operation not permitted
        E_NAMETOOLONG file name too long
        E_NOENT no such file or directory
        E_ISDIR is a directory
        E_ROFS read-only file system
        E_FAULT bad address
        E_LOOP too many symbolic links encountered
        E_IO on I/O errors
        E_BUSY device or resource busy
        E_INTR function call was interrupted
        E_LOOP too many symbolic links encountered
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed
        E_TXTBSY text file busy

        @see open()
    */

    inline static RC remove( const ::rtl::OUString& ustrFileURL )
    {
        return (RC) osl_removeFile( ustrFileURL.pData );
    }

    /** Set file attributes.

        @param ustrFileURL [in]
        The full qualified file URL.

        @param uAttributes [in]
        Attributes of the file to be set.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid

        @see FileStatus
    */

    inline static RC setAttributes( const ::rtl::OUString& ustrFileURL, sal_uInt64 uAttributes )
    {
        return (RC) osl_setFileAttributes( ustrFileURL.pData, uAttributes );
    }

    /** Set the file time.

        @param ustrFileURL [in]
        The full qualified URL of the file.

        @param rCreationTime [in]
        Creation time of the given file.

        @param rLastAccessTime [in]
        Time of the last access of the given file.

        @param rLastWriteTime [in]
        Time of the last modifying of the given file.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOENT no such file or directory not found

        @see FileStatus
    */

    inline static RC setTime(
        const ::rtl::OUString& ustrFileURL,
        const TimeValue& rCreationTime,
        const TimeValue& rLastAccessTime,
        const TimeValue& rLastWriteTime )
    {
        return (RC)  osl_setFileTime(
            ustrFileURL.pData,
            &rCreationTime,
            &rLastAccessTime,
            &rLastWriteTime );
    }

    friend class DirectoryItem;
};

// -----------------------------------------------------------------------------
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
        sal_True if object is valid directory item else sal_False.
     */

    inline sal_Bool is()
    {
        return _pData != NULL;
    }

    /** Retrieve a single directory item.

        Retrieves a single directory item. The returned handle has an initial refcount of 1.
        Due to performance issues it is not recommended to use this function while
        enumerating the contents of a directory. In this case use osl_getNextDirectoryItem() instead.

        @param ustrFileURL [in]
        An absolute file URL.

        @param rItem [out]
        On success it receives a handle which can be used for subsequent calls to osl_getFileStatus().
        The handle has to be released by a call to osl_releaseDirectoryItem().

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_MFILE too many open files used by the process
        E_NFILE too many open files in the system
        E_NOENT no such file or directory
        E_LOOP  too many symbolic links encountered
        E_NAMETOOLONG the file name is too long
        E_NOTDIR a component of the path prefix of path is not a directory
        E_IO on I/O errors
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed
        E_FAULT bad address
        E_INTR the function call was interrupted

        @see FileStatus
        @see Directory::getNextItem()
    */

    static inline RC get( const ::rtl::OUString& ustrFileURL, DirectoryItem& rItem )
    {
        if( rItem._pData)
        {
            osl_releaseDirectoryItem( rItem._pData );
            rItem._pData = NULL;
        }

        return (RC) osl_getDirectoryItem( ustrFileURL.pData, &rItem._pData );
    }

    /** Retrieve information about a single file or directory.

        @param  rStatus [in|out]
        Reference to a class which receives the information of the file or directory
        represented by this directory item.

        @return
        E_None on success
        E_NOMEM not enough memory for allocating structures
        E_INVAL the format of the parameters was not valid
        E_LOOP too many symbolic links encountered
        E_ACCES permission denied
        E_NOENT no such file or directory
        E_NAMETOOLONG file name too long
        E_BADF invalid oslDirectoryItem parameter
        E_FAULT bad address
        E_OVERFLOW value too large for defined data type
        E_INTR function call was interrupted
        E_NOLINK link has been severed
        E_MULTIHOP components of path require hopping to multiple remote machines and the file system does not allow it
        E_MFILE too many open files used by the process
        E_NFILE too many open files in the system
        E_NOSPC no space left on device
        E_NXIO no such device or address
        E_IO on I/O errors
        E_NOSYS function not implemented

        @see get()
        @see Directory::getNextItem()
        @see FileStatus
    */

    inline RC getFileStatus( FileStatus& rStatus )
    {
        return (RC) osl_getFileStatus( _pData, &rStatus._aStatus, rStatus._nMask );
    }

/** Determine if a directory item point the the same underlying file

    The comparison is done first by URL, and then by resolving links to
    find the target, and finally by comparing inodes on unix.

    @param[in]  pOther
    A directory handle to compare with the underlying object's item

    @return
    sal_True: if the items point to an identical resource<br>
    sal_False: if the items point to a different resource, or a fatal error occured<br>

    @see osl_getDirectoryItem()

    @since LibreOffice 3.6
*/
    inline sal_Bool isIdenticalTo( const DirectoryItem &pOther )
    {
        return osl_identicalDirectoryItem( _pData, pOther._pData );
    }

    friend class Directory;
};

//###########################################

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

//###########################################
// This just an internal helper function for
// private use.
extern "C" inline void SAL_CALL onDirectoryCreated(void* pData, rtl_uString* aDirectoryUrl)
{
    (static_cast<DirectoryCreationObserver*>(pData))->DirectoryCreated(aDirectoryUrl);
}

/** The directory class object provides a enumeration of DirectoryItems.

    @see DirectoryItem
    @see File
 */

class Directory: public FileBase
{
    oslDirectory    _pData;
    ::rtl::OUString _aPath;

    /** Copy constructor.
    */

    Directory( Directory& );

    /**  Assignment operator.
    */

    Directory& operator = ( Directory& );

public:

    /** Constructor.

        @param strPath [in]
        The full qualified URL of the directory.
        Relative URLs are not allowed.
     */

    Directory( const ::rtl::OUString& strPath ): _pData( 0 ), _aPath( strPath )
    {
    }

    /** Destructor.
    */

    ~Directory()
    {
        close();
    }

    /** Open a directory for enumerating its contents.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOENT the specified path doesn't exist
        E_NOTDIR the specified path is not an directory
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_MFILE too many open files used by the process
        E_NFILE too many open files in the system
        E_NAMETOOLONG File name too long
        E_LOOP Too many symbolic links encountered

        @see getNextItem()
        @see close()
    */

    inline RC open()
    {
        return (RC) osl_openDirectory( _aPath.pData, &_pData );
    }

    /** Query if directory is open.

        Query if directory is open and so item enumeration is valid.

        @return
        sal_True if the directory is open else sal_False.

        @see open()
        @see close()
    */

    inline sal_Bool isOpen() { return _pData != NULL; }

    /** Close a directory.

         @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_BADF invalid oslDirectory parameter
        E_INTR the function call was interrupted

        @see open()
    */

    inline RC close()
    {
        oslFileError Error = osl_File_E_BADF;

        if( _pData )
        {
            Error=osl_closeDirectory( _pData );
            _pData = NULL;
        }

        return (RC) Error;
    }


    /** Resets the directory item enumeration to the beginning.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOENT the specified path doesn't exist
        E_NOTDIR the specified path is not an directory
        E_NOMEM not enough memory for allocating structures
        E_ACCES permission denied
        E_MFILE too many open files used by the process
        E_NFILE too many open files in the system
        E_NAMETOOLONG File name too long
        E_LOOP Too many symbolic links encountered

        @see open()
    */

    inline RC reset()
    {
        close();
        return open();
    }

    /** Retrieve the next item of a previously opened directory.

        Retrieves the next item of a previously opened directory.

        @param  rItem [out]
        On success a valid DirectoryItem.

        @param  nHint [in]
        With this parameter the caller can tell the implementation that (s)he
        is going to call this function uHint times afterwards. This enables the implementation to
        get the information for more than one file and cache it until the next calls.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_NOENT no more entries in this directory
        E_BADF invalid oslDirectory parameter
        E_OVERFLOW the value too large for defined data type

        @see DirectoryItem
    */

    inline RC getNextItem( DirectoryItem& rItem, sal_uInt32 nHint = 0 )
    {
        if( rItem._pData )
        {
            osl_releaseDirectoryItem( rItem._pData );
            rItem._pData = 0;
        }
        return ( RC) osl_getNextDirectoryItem( _pData, &rItem._pData, nHint );
    }


    /** Retrieve information about a volume.

        Retrieves information about a volume. A volume can either be a mount point, a network
        resource or a drive depending on Operating System and File System.

        @param ustrDirectoryURL [in]
        Full qualified URL of the volume

        @param rInfo [out]
        On success it receives information about the volume.

        @return
        E_None on success
        E_NOMEM not enough memory for allocating structures
        E_INVAL the format of the parameters was not valid
        E_NOTDIR not a directory
        E_NAMETOOLONG file name too long
        E_NOENT no such file or directory
        E_ACCES permission denied
        E_LOOP too many symbolic links encountered
        E_FAULT Bad address
        E_IO on I/O errors
        E_NOSYS function not implemented
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed
        E_INTR function call was interrupted

        @see FileStatus
        @see VolumeInfo
    */

    inline static RC getVolumeInfo( const ::rtl::OUString& ustrDirectoryURL, VolumeInfo& rInfo )
    {
        return (RC) osl_getVolumeInformation( ustrDirectoryURL.pData, &rInfo._aInfo, rInfo._nMask );
    }

    /** Create a directory.

        @param ustrDirectoryURL [in]
        Full qualified URL of the directory to create.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_EXIST file exists
        E_ACCES permission denied
        E_NAMETOOLONG file name too long
        E_NOENT no such file or directory
        E_NOTDIR not a directory
        E_ROFS read-only file system
        E_NOSPC no space left on device
        E_DQUOT quota exceeded
        E_LOOP too many symbolic links encountered
        E_FAULT bad address
        E_IO on I/O errors
        E_MLINK too many links
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed

        @see remove()
    */

    inline static RC create( const ::rtl::OUString& ustrDirectoryURL )
    {
        return (RC) osl_createDirectory( ustrDirectoryURL.pData );
    }

    /** Remove an empty directory.

        @param ustrDirectoryURL [in]
        Full qualified URL of the directory.

        @return
        E_None on success
        E_INVAL the format of the parameters was not valid
        E_NOMEM not enough memory for allocating structures
        E_PERM operation not permitted
        E_ACCES permission denied
        E_NOENT no such file or directory
        E_NOTDIR not a directory
        E_NOTEMPTY directory not empty
        E_FAULT bad address
        E_NAMETOOLONG file name too long
        E_BUSY device or resource busy
        E_ROFS read-only file system
        E_LOOP too many symbolic links encountered
        E_BUSY device or resource busy
        E_EXIST file exists
        E_IO on I/O errors
        E_MULTIHOP multihop attempted
        E_NOLINK link has been severed

        @see create()
    */

    inline static RC remove( const ::rtl::OUString& ustrDirectoryURL )
    {
        return (RC) osl_removeDirectory( ustrDirectoryURL.pData );
    }

    /** Create a directory path.

        The osl_createDirectoryPath function creates a specified directory path.
        All nonexisting sub directories will be created.
        <p><strong>PLEASE NOTE:</strong> You cannot rely on getting the error code
        E_EXIST for existing directories. Programming against this error code is
        in general a strong indication of a wrong usage of osl_createDirectoryPath.</p>

        @param aDirectoryUrl
        [in] The absolute file URL of the directory path to create.
        A relative file URL will not be accepted.

        @param aDirectoryCreationObserver
        [in] Pointer to an instance of type DirectoryCreationObserver that will
        be informed about the creation of a directory. The value of this
        parameter may be NULL, in this case notifications will not be sent.

        @return
        <dl>
        <dt>E_None</dt>
        <dd>On success</dd>
        <dt>E_INVAL</dt>
        <dd>The format of the parameters was not valid</dd>
        <dt>E_ACCES</dt>
        <dd>Permission denied</dd>
        <dt>E_EXIST</dt>
        <dd>The final node of the specified directory path already exist</dd>
        <dt>E_NAMETOOLONG</dt>
        <dd>The name of the specified directory path exceeds the maximum allowed length</dd>
        <dt>E_NOTDIR</dt>
        <dd>A component of the specified directory path already exist as file in any part of the directory path</dd>
        <dt>E_ROFS</dt>
        <dd>Read-only file system</dd>
        <dt>E_NOSPC</dt>
        <dd>No space left on device</dd>
        <dt>E_DQUOT</dt>
        <dd>Quota exceeded</dd>
        <dt>E_FAULT</dt>
        <dd>Bad address</dd>
        <dt>E_IO</dt>
        <dd>I/O error</dd>
        <dt>E_LOOP</dt>
        <dd>Too many symbolic links encountered</dd>
        <dt>E_NOLINK</dt>
        <dd>Link has been severed</dd>
        <dt>E_invalidError</dt>
        <dd>An unknown error occurred</dd>
        </dl>

        @see DirectoryCreationObserver
        @see create
    */
    static RC createPath(
        const ::rtl::OUString& aDirectoryUrl,
        DirectoryCreationObserver* aDirectoryCreationObserver = NULL)
    {
        return (RC)osl_createDirectoryPath(
            aDirectoryUrl.pData,
            (aDirectoryCreationObserver) ? onDirectoryCreated : NULL,
            aDirectoryCreationObserver);
    }
};

} /* namespace osl */

#endif  /* _OSL_FILE_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
