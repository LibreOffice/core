/*************************************************************************
 *
 *  $RCSfile: file.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:12 $
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


#ifndef _OSL_FILE_HXX_
#define _OSL_FILE_HXX_

#ifdef __cplusplus

#ifndef _RTL_MEMORY_H_
#   include <rtl/memory.h>
#endif

#ifndef _RTL_USTRING_
#   include <rtl/ustring>
#endif

#include <osl/file.h>
#include <osl/macros.hxx>


#ifdef _USE_NAMESPACE
namespace osl
{
#endif


/** The VolumeInfo class

    @see Directory::getVolumeInfo
*/

#define VolumeInfoMask_Attributes     osl_VolumeInfo_Mask_Attributes
#define VolumeInfoMask_TotalSpace     osl_VolumeInfo_Mask_TotalSpace
#define VolumeInfoMask_UsedSpace      osl_VolumeInfo_Mask_UsedSpace
#define VolumeInfoMask_FreeSpace      osl_VolumeInfo_Mask_FreeSpace
#define VolumeInfoMask_MaxNameLength  osl_VolumeInfo_Mask_MaxNameLength
#define VolumeInfoMask_MaxPathLength  osl_VolumeInfo_Mask_MaxPathLengt
#define VolumeInfoMask_FileSystemName osl_VolumeInfo_Mask_FileSystemName

class Directory;

class VolumeInfo
{
    oslVolumeInfo   _aInfo;
    sal_uInt32      _nMask;
    rtl_uString     *_strFileSystemName;

    /** define copy c'tor and assginment operator privat
     */

    VolumeInfo( VolumeInfo& );
    VolumeInfo& operator = ( VolumeInfo& );

public:

    /** C'tor

     @param nMask set of flaggs decribing the demanded information.
    */

    VolumeInfo( sal_uInt32 nMask ): _nMask( nMask )
    {
        _aInfo.uStructSize = sizeof( oslVolumeInfo );
        rtl_fillMemory( &_aInfo.uValidFields, sizeof( oslVolumeInfo ) - sizeof( sal_uInt32 ), 0 );
        _strFileSystemName=NULL;
        _aInfo.pstrFileSystemName = &_strFileSystemName;
    }

    /** D'tor
     */

    ~VolumeInfo()
    {
        if (_strFileSystemName!=NULL)
            rtl_uString_release(_strFileSystemName);
    }

    /** check if specified fields are valid

     @param set of flags for the fields to check
     @return sal_True if all fields are valid, sal_False otherwise.
    */

    inline sal_Bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aInfo.uValidFields ) == nMask;
    }

    /** @return sal_True if Attributes are valid and the volume is remote,
     sal_False otherwise.
    */

    inline sal_Bool getRemoteFlag() const
    {
        return (sal_Bool) (_aInfo.uAttributes & osl_Volume_Attribute_Remote);
    }

    /** @return sal_True if attributes are valid and the volume is removable,
     sal_False otherwise.
    */

    inline sal_Bool getRemoveableFlag() const
    {
        return (sal_Bool) (_aInfo.uAttributes & osl_Volume_Attribute_Removeable);
    }

    /** @return the total diskspace of this volume if this information is valid,
     0 otherwise.
      */

    inline sal_uInt64 getTotalSpace() const
    {
        return _aInfo.uTotalSpace;
    }

    /** @return the free diskspace of this volume if this information is valid,
     0 otherwise.
      */

    inline sal_uInt64 getFreeSpace() const
    {
        return _aInfo.uFreeSpace;
    }

    /** @return the used diskspace of this volume if this information is valid,
     0 otherwise.
      */

    inline sal_uInt64 getUsedSpace() const
    {
        return _aInfo.uUsedSpace;
    }

    /** @return the maximal length of a file name if this information is valid,
     0 otherwise.
      */

    inline sal_uInt32 getMaxNameLength() const
    {
        return _aInfo.uMaxNameLength;
    }

    /** @return the maximal length of a path if this information is valid,
     0 otherwise.
      */

    inline sal_uInt32 getMaxPathLength() const
    {
        return _aInfo.uMaxPathLength;
    }

    /** @return the name of the volume's fielsystem if this information is valid,
     otherwise an empty string.
      */

    inline ::rtl::OUString getFileSystemName() const
    {
        return _strFileSystemName;
    }

    friend class Directory;
};

// -----------------------------------------------------------------------------

/** The FileStatus class

    @see DirectoryItem::getFileStatus
*/

#define FileStatusMask_Type         osl_FileStatus_Mask_Type
#define FileStatusMask_Attributes   osl_FileStatus_Mask_Attributes
#define FileStatusMask_CreationTime osl_FileStatus_Mask_CreationTime
#define FileStatusMask_AccessTime   osl_FileStatus_Mask_AccessTime
#define FileStatusMask_ModifyTime   osl_FileStatus_Mask_ModifyTime
#define FileStatusMask_FileSize     osl_FileStatus_Mask_FileSize
#define FileStatusMask_FileName     osl_FileStatus_Mask_FileName
#define FileStatusMask_FilePath     osl_FileStatus_Mask_FilePath
#define FileStatusMask_NativePath   osl_FileStatus_Mask_NativePath
#define FileStatusMask_All          osl_FileStatus_Mask_All
#define FileStatusMask_Validate     osl_FileStatus_Mask_Validate

#define Attribute_ReadOnly     osl_File_Attribute_ReadOnly
#define Attribute_Hidden       osl_File_Attribute_Hidden
#define Attribute_Executable   osl_File_Attribute_Executable
#define Attribute_GrpWrite     osl_File_Attribute_GrpWrite
#define Attribute_GrpRead      osl_File_Attribute_GrpRead
#define Attribute_GrpExe       osl_File_Attribute_GrpExe
#define Attribute_OwnWrite     osl_File_Attribute_OwnWrite
#define Attribute_OwnRead      osl_File_Attribute_OwnRead
#define Attribute_OwnExe       osl_File_Attribute_OwnExe
#define Attribute_OthWrite     osl_File_Attribute_OthWrite
#define Attribute_OthRead      osl_File_Attribute_OthRead
#define Attribute_OthExe       osl_File_Attribute_OthExe

class DirectoryItem;

class FileStatus
{
    oslFileStatus   _aStatus;
    sal_uInt32      _nMask;
    rtl_uString     *_strNativePath;
    rtl_uString     *_strFilePath;

    /** define copy c'tor and assginment operator privat
     */

    FileStatus( FileStatus& );
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

    /** C'tor

     @param nMask set of flaggs decribing the demanded information.
    */

    FileStatus( sal_uInt32 nMask ): _nMask( nMask ), _strNativePath( NULL ), _strFilePath( NULL )
    {
        _aStatus.uStructSize = sizeof( oslFileStatus );
        rtl_fillMemory( &_aStatus.uValidFields, sizeof( oslFileStatus ) - sizeof( sal_uInt32 ), 0 );
        _aStatus.pstrNativePath = &_strNativePath;
        _aStatus.pstrFilePath = &_strFilePath;
    }

    /** D'tor
     */

    ~FileStatus()
    {
        if ( _strFilePath )
            rtl_uString_release( _strFilePath );
        if ( _strNativePath )
            rtl_uString_release( _strNativePath );
        if ( _aStatus.strFileName )
            rtl_uString_release( _aStatus.strFileName );
    }

    /** check if specified fields are valid

     @param set of flags for the fields to check
     @return sal_True if all fields are valid, sal_False otherwise.
    */
    inline sal_Bool isValid( sal_uInt32 nMask ) const
    {
        return ( nMask & _aStatus.uValidFields ) == nMask;
    }

    /** @return the file type if this information is valid,
     Unknown otherwise.
     */
    inline Type getFileType() const
    {
        return (_aStatus.uValidFields & FileStatusMask_Type) ?  (Type) _aStatus.eType : Unknown;
    }

    /** @return the set of attribute flags of this file
    */

    inline sal_uInt64 getAttributes() const
    {
        return _aStatus.uAttributes;
    }

    /** @return the creation time if this information is valid,
      an uninitialized TimeValue otherwise.
    */

    inline TimeValue getCreationTime() const
    {
        return _aStatus.aCreationTime;
    }

    /** @return the last access time if this information is valid,
      an uninitialized TimeValue otherwise.
    */

    inline TimeValue getAccessTime() const
    {
        return _aStatus.aAccessTime;
    }

    /** @return the last modified time if this information is valid,
      an uninitialized TimeValue otherwise.
    */

    inline TimeValue getModifyTime() const
    {
        return _aStatus.aModifyTime;
    }

    /** @return the actual file size if this information is valid,
      0 otherwise.
    */

    inline sal_uInt64 getFileSize() const
    {
        return _aStatus.uFileSize;
    }

    /** @return the file name if this information is valid, an empty
     string otherwise.
    */

    inline ::rtl::OUString getFileName() const
    {
        return _aStatus.strFileName ? ::rtl::OUString(_aStatus.strFileName) : ::rtl::OUString();
    }

    /** @return the file path in UNC notation if this information is valid,
     an empty string otherwise.
    */

    inline ::rtl::OUString getFilePath() const
    {
        return _strFilePath ? ::rtl::OUString(_strFilePath) : ::rtl::OUString();
    }

    /** @return the file path in host notation if this information is valid,
     an empty string otherwise.
    */

    inline ::rtl::OUString getNativePath() const
    {
        return _strNativePath ? ::rtl::OUString(_strNativePath) : ::rtl::OUString();
    }

    friend class DirectoryItem;
};

// -----------------------------------------------------------------------------

/** Base class for all filesystem specific objects

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
        E_invalidError = osl_File_E_invalidError    /* unmapped error: always last entry in enum! */
    };


public:

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

    static inline RC getCanonicalName( const ::rtl::OUString& strRequested, ::rtl::OUString& strValid )
    {
        return (RC) osl_getCanonicalName( strRequested.pData, &strValid.pData );
    }

    /** Converts a path relative to a given directory into an full qualified UNC path.

     @param strDirBase [in] Base directory to which the relative path is related to.
     @param strRelative[in] Path of a file or directory relative to the directory path
     specified by <code>strDirBase</code>.
     @param strAbsolute [out] On success it receives the full qualified UNC path of the
     requested relative path.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_NOTDIR       Not a directory<br>
     E_ACCES        Permission denied<br>
     E_NOENT        No such file or directory<br>
     E_NAMETOOLONG  File name too long<p>


     These errorcodes can (eventually) be returned:<p>
     E_OVERFLOW     Value too large for defined data type<p>
     E_FAULT        Bad address<br>
     E_INTR         function call was interrupted<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<p>

     @see   getStatus
    */

    static inline RC getAbsolutePath( const ::rtl::OUString& strDirBase, const ::rtl::OUString& strRelative, ::rtl::OUString& strAbsolute )
    {
        return (RC) osl_getAbsolutePath( strDirBase.pData, strRelative.pData, &strAbsolute.pData );
    }

    /** Converts a system dependent path into a full qualified UNC path

     @param strSysPath[in] System dependent path of a file or a directory
     @param strPath[out] On success it receives the full qualified UNC path

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>

     @see   getCanonicalName
    */

    static inline RC normalizePath( const ::rtl::OUString& strSysPath, ::rtl::OUString& strPath )
    {
        return (RC) osl_normalizePath( strSysPath.pData, &strPath.pData );
    }

    /** Converts a File URL into a full qualified UNC path

     @param urlPath[in] System dependent path of a file or a directory
     @param strPath[out] On success it receives the full qualified UNC path

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>

     @see   normalizePath
    */

    static inline RC getNormalizedPathFromFileURL( const ::rtl::OUString& strPath, ::rtl::OUString& normPath )
    {
        return (RC) osl_getNormalizedPathFromFileURL( strPath.pData, &normPath.pData );
    }

    /** Converts a full qualified UNC path into a FileURL

     @param dir[in] System dependent path of a file or a directory
     @param strPath[out] On success it receives the full qualified UNC path

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>

     @see   normalizePath
     @see   getNormalizedPathFromFileURL
    */

    static inline RC getFileURLFromNormalizedPath( const ::rtl::OUString& normPath, ::rtl::OUString& strPath )
    {
        return (RC) osl_getFileURLFromNormalizedPath( normPath.pData, &strPath.pData );
    }

    /** Converts a full qualified UNC path into a system depended path

     @param dir[in] Full qualified UNC path
     @param strPath[out] On success it receives the System dependent path of a file or a directory

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>

     @see   normalizePath
     @see   getNormalizedPathFromFileURL
    */

    static inline RC getSystemPathFromNormalizedPath( const ::rtl::OUString& normPath, ::rtl::OUString& strPath )
    {
        return (RC)osl_getSystemPathFromNormalizedPath( normPath.pData, &strPath.pData );
    }

    /** Searches a full qualified UNC-Path/File

     @param filePath[in] System dependent path / Normalized Path / File-URL or file or relative directory
     @param searchPath[in] Paths, in which a given file has to be searched. These paths are only for the search of a
     file or a relative path, otherwise it will be ignored. If it is set to NULL or while using the
     search path the search failed the function searches for a matching file in all system directories and in the directories
     listed in the PATH environment variable
     @param strPath[out] On success it receives the full qualified UNC path

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOTDIR       Not a directory<br>
     E_NOENT        No such file or directory not found<br>
     @see   normalizePath
    */

    static inline RC searchNormalizedPath( const ::rtl::OUString& filePath, const ::rtl::OUString& searchPath, ::rtl::OUString& strPath )
    {
        return (RC) osl_searchNormalizedPath( filePath.pData, searchPath.pData, &strPath.pData );
    }
};

// -----------------------------------------------------------------------------

/** The VolumeDevice class

    @see VolumeInfo
*/

class VolumeDevice : public FileBase
{
    oslVolumeDeviceHandle   _aHandle;

public:
    VolumeDevice() : _aHandle( NULL )
    {
    }

    VolumeDevice( const VolumeDevice & rDevice )
    {
        _aHandle = rDevice._aHandle;
        if ( _aHandle )
            osl_acquireVolumeDeviceHandle( _aHandle );
    }

    ~VolumeDevice()
    {
        if ( _aHandle )
            osl_releaseVolumeDeviceHandle( _aHandle );
    }


    inline VolumeDevice & operator =( const VolumeDevice & rDevice )
    {
        oslVolumeDeviceHandle   newHandle = rDevice._aHandle;

        if ( newHandle )
            osl_acquireVolumeDeviceHandle( _aHandle );

        if ( _aHandle )
            osl_releaseVolumeDeviceHandle( _aHandle );

        _aHandle = newHandle;

        return *this;
    }

    inline RC automount()
    {
        return (RC)osl_automountVolumeDevice( _aHandle );
    }

    inline RC unmount()
    {
        return (RC)osl_unmountVolumeDevice( _aHandle );
    }

    inline rtl::OUString getMountPath()
    {
        rtl::OUString   aPath;
        osl_getVolumeDeviceMountPath( _aHandle, &aPath.pData );
        return aPath;
    }

    friend class VolumeInfo;
};

// -----------------------------------------------------------------------------

/** The file class object provides access to file contents and attributes

    @see Directory
    @see DirectoryItem
 */

class File: public FileBase
{
    oslFileHandle   _pData;
    ::rtl::OUString _aPath;

    /** define copy c'tor and assginment operator privat
     */

    File( File& );
    File& operator = ( File& );

public:

    /** C'tor

      @param  strPath [in] The full qualified path of the file in UNC notation.
      The path delimiter is '/'. Relative paths are not allowed.
     */

    File( const ::rtl::OUString& strPath ): _pData( 0 ), _aPath( strPath ) {};

    /** D'tor
     */

    inline ~File()
    {
        close();
    }

    /** Opens a file.
     @param uFlags [in] Specifies the open mode.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_NOMEM        not enough memory for allocating structures <br>
     E_INVAL        the format of the parameters was not valid<br>
     E_NAMETOOLONG  pathname was too long<br>
     E_NOENT        No such file or directory<br>
     E_ACCES        permission denied<P>
     E_ISDIR        Is a directory<p>

     These errorcodes can (eventually) be returned:<p>
     E_NOTDIR       Not a directory<br>
     E_NXIO         No such device or address<br>
     E_NODEV        No such device<br>
     E_ROFS         Read-only file system<br>
     E_TXTBSY       Text file busy<br>
     E_ISDIR        Is a directory<br>
     E_FAULT        Bad address<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_NOSPC        No space left on device<br>
     E_MFILE        too many open files used by the process<br>
     E_NFILE        too many open files in the system<br>
     E_DQUOT        Quota exceeded<br>
     E_EXIST        File exists<br>
     E_INTR         function call was interrupted<br>
     E_IO           I/O error<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<br>
     E_EOVERFLOW    Value too large for defined data type<p>

     @see   close
     @see   setPos
     @see   getPos
     @see   read
     @see   write
     @see   setSize
    */

    #define OpenFlag_Read   osl_File_OpenFlag_Read
    #define OpenFlag_Write  osl_File_OpenFlag_Write
    #define OpenFlag_Create osl_File_OpenFlag_Create

    inline RC open( sal_uInt32 uFlags )
    {
        return (RC) osl_openFile( _aPath.pData, &_pData, uFlags );
    }

    /** Closes an open file.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<p>

     These errorcodes can (eventually) be returned:<p>
     E_BADF         Bad file<br>
     E_INTR         function call was interrupted<br>
     E_NOLINK       Link has been severed<br>
     E_NOSPC        No space left on device<br>
     E_IO           I/O error<p>
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
    };


    /** Sets the internal position pointer of an open file.
     @param uHow [in] Distance to move the internal position pointer (from uPos).
     @param uPos [in] Absolute position from the beginning of the file.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_OVERFLOW     The resulting file offset would be a value which cannot
     be represented correctly for regular files<p>

     @see    open
     @see    getPos
    */

    #define Pos_Absolut osl_Pos_Absolut
    #define Pos_Current osl_Pos_Current
    #define Pos_End     osl_Pos_End

    inline RC setPos( sal_uInt32 uHow, sal_uInt64 uPos )
    {
        return (RC) osl_setFilePos( _pData, uHow, uPos );
    }

    /** Retrieves the current position of the internal pointer of an open file.
     @param pPos [out] On Success it receives the current position of the file pointer.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_OVERFLOW     The resulting file offset would be a value which cannot
     be represented correctly for regular files<p>

     @see   open
     @see   setPos
     @see   read
     @see   write
    */

    inline RC getPos( sal_uInt64& uPos )
    {
        return (RC) osl_getFilePos( _pData, &uPos );
    }


    /** Sets the file size of an open file. The file can be truncated or enlarged by the function.
     The position of the file pointer is not affeced by this function.
     @param uSize [int] New size in bytes.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_OVERFLOW     The resulting file offset would be a value which cannot
     be represented correctly for regular files<p>

     @see   open
     @see   setPos
     @see   getStatus
    */

    inline RC setSize( sal_uInt64 uSize )
    {
        return (RC) osl_setFileSize( _pData, uSize );
    }

    /** Reads a number of bytes from a file. The internal file pointer is increased by the number of bytes
     read.
     @param pBuffer [out] Points to a buffer which receives data. The buffer must be large enough
     to hold <code>uBytesRequested</code> bytes.
     @param uBytesRequested [in] Number of bytes which should be retrieved.
     @param rBytesRead [out] On success the number of bytes which have actually been retrieved.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>

     These errorcodes can (eventually) be returned:<p>
     E_INTR         function call was interrupted<br>
     E_IO           I/O error<br>
     E_ISDIR        Is a directory<br>
     E_BADF         Bad file<br>
     E_FAULT        Bad address<br>
     E_AGAIN        Operation would block<br>
     E_NOLINK       Link has been severed<p>

     @see   open
     @see   write
     @see   setPos
    */

    inline RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
    {
        return (RC) osl_readFile( _pData, pBuffer, uBytesRequested, &rBytesRead );
    }

    /** Writes a number of bytes to a file. The internal file pointer is increased by the number of bytes
     read.
     @param pBuffer [in] Points to a buffer which contains the data.
     @param uBytesToWrite [in] Number of bytes which should be written.
     @param rBytesWritten [out] On success the number of bytes which have actually been written.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_FBIG         File too large<br>
     E_DQUOT        Quota exceeded<p>

     These errorcodes can (eventually) be returned:<p>
     E_AGAIN        Operation would block<br>
     E_BADF         Bad file<br>
     E_FAULT        Bad address<br>
     E_INTR         function call was interrupted<br>
     E_IO           I/O error<br>
     E_NOLCK        No record locks available<br>
     E_NOLINK       Link has been severed<br>
     E_NOSPC        No space left on device<br>
     E_NXIO         No such device or address<p>

     @see   openFile
     @see   readFile
     @see   setFilePos
    */

    inline RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
    {
        return (RC) osl_writeFile( _pData, pBuffer, uBytesToWrite, &rBytesWritten );
    }

    /** Copies a file to a new destination. Copies only files not directories. No assumptions should
     be made about preserving attributes or file time.
     @param strPath [in] Full qualified UNC path of the source file.
     @param strDestPath [in] Full qualified UNC path of the destination file. A directory is
     NOT a valid destination file !
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_ACCES        Permission denied<br>
     E_PERM         Operation not permitted<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOENT        No such file or directory<br>
     E_ISDIR        Is a directory<br>
     E_ROFS         Read-only file system<p>

     @see   move
     @see   remove
    */
    inline static RC copy( const ::rtl::OUString& strPath, const ::rtl::OUString& strDestPath )
    {
        return (RC) osl_copyFile( strPath.pData, strDestPath.pData );
    }

    /** Moves a file or directory to a new destination or renames it. File time and attributes
     are preserved.
     @param strPath [in] Full qualified UNC path of the source file.
     @param strDestPath [in] Full qualified UNC path of the destination file. An existing directory
     is NOT a valid destination !
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_ACCES        Permission denied<br>
     E_PERM         Operation not permitted<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOENT        No such file or directory<br>
     E_ROFS         Read-only file system<p>

     @see   copy
    */

    inline static RC move( const ::rtl::OUString& strPath, const ::rtl::OUString& strDestPath )
    {
        return (RC) osl_moveFile( strPath.pData, strDestPath.pData );
    }

    /** Removes (erases) a regular file.

     @param strPath [in] Full qualified UNC path of the directory.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_ACCES        Permission denied<br>
     E_PERM         Operation not permitted<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOENT        No such file or directory<br>
     E_ISDIR        Is a directory<br>
     E_ROFS         Read-only file system<p>

     These errorcodes can (eventually) be returned:<p>
     E_FAULT        Bad address<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_IO           I/O error<br>
     E_BUSY         Device or resource busy<br>
     E_INTR         function call was interrupted<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<br>
     E_TXTBSY       Text file busy<p>
    */

    inline static RC remove( const ::rtl::OUString& strPath )
    {
        return (RC) osl_removeFile( strPath.pData );
    }

    /** Sets file-attributes

     @param filePath[in] Path of the file
     @param uAttributes[in] Attributes of the file to be set

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     @see   getStatus
     */

    inline static RC setAttributes( const ::rtl::OUString& strPath, sal_uInt64 uAttributes )
    {
        return (RC) osl_setFileAttributes( strPath.pData, uAttributes );
    }

    /** Sets file-Time

     @param filePath[in] Path of the file
     @param aCreationTime[in] creation time of the given file
     @param aLastAccessTime[in]  time of the last access of the given file
     @param aLastWriteTime[in]  time of the last modifying of the given file


     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOENT        No such file or directory not found<br>
     @see   getStatus
    */

    inline static RC setTime( const ::rtl::OUString& strPath, TimeValue& rCreationTime,
                                                              TimeValue& rLastAccessTime,
                                                              TimeValue& rLastWriteTime )
    {
        return (RC)  osl_setFileTime( strPath.pData, &rCreationTime,
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

    /** default c'tor
    */

    DirectoryItem(): _pData( NULL )
    {
    }

    /** Copy C'tor
     */

    DirectoryItem( DirectoryItem& rItem ): _pData( rItem._pData)
    {
        osl_acquireDirectoryItem( _pData );
    }


    /** D'tor
     */

    ~DirectoryItem()
    {
        if( _pData )
            osl_releaseDirectoryItem( _pData );
    }

    /** assignment operator
     */

    DirectoryItem& operator = ( DirectoryItem& rItem )
    {
        if( _pData )
            osl_releaseDirectoryItem( _pData );

        _pData = rItem._pData;
        osl_acquireDirectoryItem( _pData );
        return *this;
    };

    /** @return sal_True if object is valid directory item,
     sal_False otherwise.
     */

    inline sal_Bool is()
    {
        return _pData != NULL;
    }

    /** fills a single directory item object

     @param strFilePath [in] absolute file path following the notation explained in the documentation for
     <code>osl_openDirectory</code>. Due to performance issues it is not recommended to use this function
     while enumerating the contents of a directory. In this case use <code>osl_getNextDirectoryItem</code> instead.
     @param rItem [out] on success it receives a valid directory item object.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_ACCES        permission denied<br>
     E_MFILE        too many open files used by the process<br>
     E_NFILE        too many open files in the system<br>
     E_NOENT        No such file or directory<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOTDIR       A component of the path prefix of path is not a directory<p>

     These errorcodes can (eventually) be returned:<p>
     E_IO           I/O error<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<br>
     E_FAULT        Bad address<br>
     E_INTR         function call was interrupted<p>


     @see getFileStatus
     @see Directory::getNextItem
    */

    static inline RC get( const ::rtl::OUString& strPath, DirectoryItem& rItem )
    {
        if( rItem._pData)
            osl_releaseDirectoryItem( rItem._pData );

        return (RC) osl_getDirectoryItem( strPath.pData, &rItem._pData );
    }

    /** Get a directory item handle from an open file.
     @param rItem [out] On success it receives a handle to a directory item which can be used
     in subsequent calls to <code>getStatus</code>.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<p>

     @see   File
     @see   getStatus
    */

    static inline RC get( const File& rFile, DirectoryItem& rItem )
    {
        if( rItem._pData)
            osl_releaseDirectoryItem( rItem._pData );

        return (RC) osl_createDirectoryItemFromHandle( rFile._pData, &rItem._pData );
    }


    /** Retrieves information about a single file or directory

     @param rStatus [in/out] reference of FileStatus class object that should be filled.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_NOMEM        not enough memory for allocating structures <br>
     E_INVAL        the format of the parameters was not valid<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_ACCES        Permission denied<br>
     E_NOENT        No such file or directory<br>
     E_NAMETOOLONG  file name too long<p>

     These errorcodes can (eventually) be returned:<p>
     E_BADF         Invalid oslDirectoryItem parameter<br>
     E_FAULT        Bad address<br>
     E_OVERFLOW     Value too large for defined data type<br>
     E_INTR         function call was interrupted<br>
     E_NOLINK       Link has been severed<br>
     E_MULTIHOP     Components of path require hopping to multiple remote machines and the file system does not allow it<br>
     E_MFILE        too many open files used by the process<br>
     E_NFILE        too many open files in the system<br>
     E_NOSPC        No space left on device<br>
     E_NXIO         No such device or address<br>
     E_IO           I/O error<br>
     E_NOSYS        Function not implemented<p>

     @see DirectoryItem::get
     @see Directory::getNextItem
    */

    inline RC getFileStatus( FileStatus& rStatus )
    {
        return (RC) osl_getFileStatus( _pData, &rStatus._aStatus, rStatus._nMask );
    }

    friend class Directory;
};

// -----------------------------------------------------------------------------

/** The directory class object provides a enumeration of DirectoryItems.

    @see DirectoryItem
    @see File
 */

class Directory: public FileBase
{
    oslDirectory    _pData;
    ::rtl::OUString _aPath;

    /** define copy c'tor and assginment operator privat
     */
    Directory( Directory& );
    Directory& operator = ( Directory& );


public:

    /** C'tor

      @param  strPath [in] The full qualified path of the directory in UNC notation.
      The path delimiter is '/'. Relative paths are not allowed.
     */

    Directory( const ::rtl::OUString& strPath ): _pData( 0 ), _aPath( strPath )
    {
    };

    /** D'tor
     */

    ~Directory()
    {
        close();
    }

    /** Opens a directory for enumerating its contents.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOENT        the specified path doesn't exist<br>
     E_NOTDIR       the specified path is not an directory <br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_ACCES        permission denied<br>
     E_MFILE        too many open files used by the process<br>
     E_NFILE        too many open files in the system<br>
     E_NAMETOOLONG  File name too long<br>
     E_LOOP         Too many symbolic links encountered<p>

     @see getNextItem
     @see close
    */

    inline RC open()
    {
        return (RC) osl_openDirectory( _aPath.pData, &_pData );
    }

    /** query if directory item enumeration is valid.

     @return sal_True if enumeration is valid (opened), sal_False otherwise

     @see open
     @see close
    */

    inline sal_Bool isOpen() { return _pData != NULL; };

    /** closes a directory item enumeration

      @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <p>

     These errorcodes can (eventually) be returned:<p>
     E_BADF         Invalid oslDirectory parameter<br>
     E_INTR         function call was interrupted<p>

     @see open
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


    /** resets the directory item enumeration to the beginning.

     @return the same as open
     @see open
    */

    inline RC reset()
    {
        close();
        return open();
    }

    /** Retrieves the next item of a previously openend directory
     @param rItem [out] The class that receives the next item's data.
     @param nHint [in]  With this parameter the caller can tell the implementation that one
     is going to call this function uHint times afterwards. This enables the implementation to
     get the information for more than one file and cache it until the next calls.

     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_NOENT        No more entries in this directory<p>

     These errorcodes can (eventually) be returned:<p>
     E_BADF         oslDirectory parameter is not valid<br>
     E_OVERFLOW     Value too large for defined data type<p>

     @see DirectoryItem::get
    */

    inline RC getNextItem( DirectoryItem& rItem, sal_uInt32 nHint = 0 )
    {
        return ( RC) osl_getNextDirectoryItem( _pData, &rItem._pData, nHint );
    }

    /** Retrieves information about a volume. A volume can either be a mount point, a network
     resource or a drive depending on operating system and file system. Before calling this
     function <code>File::getStatus</code> should be called to determine if the type is
     <code>Type_Volume</code>.

     @param strDirectory [in]   Full qualified UNC path to the volume
     @param rInfo [out] On success it receives information about the volume.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_NOMEM        not enough memory for allocating structures <br>
     E_INVAL        the format of the parameters was not valid<p>

     These errorcodes can (eventually) be returned:<p>
     E_NOTDIR       Not a directory<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOENT        No such file or directory<br>
     E_ACCES        permission denied<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_FAULT        Bad address<br>
     E_IO           I/O error<br>
     E_NOSYS        Function not implemented<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<br>
     E_INTR         function call was interrupted<p>

     @see   getFileStatus
     @see   VolumeInfo
    */

    inline static RC getVolumeInfo( const ::rtl::OUString& strPath, VolumeInfo& rInfo )
    {
        return (RC) osl_getVolumeInformation( strPath.pData, &rInfo._aInfo, rInfo._nMask );
    }

    /** Creates a directory.

     @param   strPath [in]  Full qualified UNC path of the directory to create.
     @return E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_EXIST        File exists<br>
     E_ACCES        Permission denied<br>
     E_NAMETOOLONG  File name too long<br>
     E_NOENT        No such file or directory<br>
     E_NOTDIR       Not a directory<br>
     E_ROFS         Read-only file system<br>
     E_NOSPC        No space left on device<br>
     E_DQUOT        Quota exceeded<p>

     These errorcodes can (eventually) be returned:<p>
     E_LOOP         Too many symbolic links encountered<br>
     E_FAULT        Bad address<br>
     E_IO           I/O error<br>
     E_MLINK        Too many links<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<p>

     @see   remove
    */

    inline static RC create( const ::rtl::OUString& strPath )
    {
        return (RC) osl_createDirectory( strPath.pData );
    }

    /** Removes an empty directory.

     @param   strPath [in]   Full qualified UNC path of the empty directory to remove.
     @return  E_None on success otherwise one of the following errorcodes:<p>
     E_INVAL        the format of the parameters was not valid<br>
     E_NOMEM        not enough memory for allocating structures <br>
     E_PERM         Operation not permitted<br>
     E_ACCES        Permission denied<br>
     E_NOENT        No such file or directory<br>
     E_NOTDIR       Not a directory<br>
     E_NOTEMPTY     Directory not empty<p>

     These errorcodes can (eventually) be returned:<p>
     E_FAULT        Bad address<br>
     E_NAMETOOLONG  File name too long<br>
     E_BUSY         Device or resource busy<br>
     E_ROFS         Read-only file system<br>
     E_LOOP         Too many symbolic links encountered<br>
     E_BUSY         Device or resource busy<br>
     E_EXIST        File exists<br>
     E_IO           I/O error<br>
     E_MULTIHOP     Multihop attempted<br>
     E_NOLINK       Link has been severed<p>

     @see create
    */

    inline static RC remove( const ::rtl::OUString& strPath )
    {
        return (RC) osl_removeDirectory( strPath.pData );
    }
};

#ifdef _USE_NAMESPACE
}
#endif

#endif  /* __cplusplus */
#endif  /* _OSL_FILE_HXX_ */

