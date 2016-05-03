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

#include "filglob.hxx"
#include "filerror.hxx"
#include "shell.hxx"
#include "bc.hxx"
#include <osl/file.hxx>
#include <vector>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include "sal/types.h"

using namespace ucbhelper;
using namespace osl;
using namespace ::com::sun::star;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;

namespace {

    Sequence< Any > generateErrorArguments(
        OUString const & rPhysicalUrl)
    {
        OUString aResourceName;
        OUString aResourceType;
        bool bRemovable = false;
        bool bResourceName = false;
        bool bResourceType = false;
        bool bRemoveProperty = false;

        if (osl::FileBase::getSystemPathFromFileURL(
                rPhysicalUrl,
                aResourceName)
            == osl::FileBase::E_None)
            bResourceName = true;

        // The resource types "folder" (i.e., directory) and
        // "volume" seem to be
        // the most interesting when producing meaningful error messages:
        osl::DirectoryItem aItem;
        if (osl::DirectoryItem::get(rPhysicalUrl, aItem) ==
            osl::FileBase::E_None)
        {
            osl::FileStatus aStatus( osl_FileStatus_Mask_Type );
            if (aItem.getFileStatus(aStatus) == osl::FileBase::E_None)
                switch (aStatus.getFileType())
                {
                    case osl::FileStatus::Directory:
                        aResourceType = "folder";
                        bResourceType = true;
                        break;

                    case osl::FileStatus::Volume:
                    {
                        aResourceType = "volume";
                        bResourceType = true;
                        osl::VolumeInfo aVolumeInfo(
                            osl_VolumeInfo_Mask_Attributes );
                        if( osl::Directory::getVolumeInfo(
                            rPhysicalUrl,aVolumeInfo ) ==
                            osl::FileBase::E_None )
                        {
                            bRemovable = aVolumeInfo.getRemoveableFlag();
                            bRemoveProperty = true;
                        }
                    }
                    break;
                    case osl::FileStatus::Regular:
                    case osl::FileStatus::Fifo:
                    case osl::FileStatus::Socket:
                    case osl::FileStatus::Link:
                    case osl::FileStatus::Special:
                    case osl::FileStatus::Unknown:
                        // do nothing for now
                        break;
                }
        }

        Sequence< Any > aArguments( 1              +
                                    (bResourceName ? 1 : 0)     +
                                    (bResourceType ? 1 : 0)     +
                                    (bRemoveProperty ? 1 : 0) );
        sal_Int32 i = 0;
        aArguments[i++]
            <<= PropertyValue(OUString( "Uri"),
                              -1,
                              makeAny(rPhysicalUrl),
                              PropertyState_DIRECT_VALUE);
        if (bResourceName)
            aArguments[i++]
                <<= PropertyValue(OUString( "ResourceName"),
                                  -1,
                                  makeAny(aResourceName),
                                  PropertyState_DIRECT_VALUE);
        if (bResourceType)
            aArguments[i++]
                <<= PropertyValue(OUString( "ResourceType"),
                                  -1,
                                  makeAny(aResourceType),
                                  PropertyState_DIRECT_VALUE);
        if (bRemoveProperty)
            aArguments[i++]
                <<= PropertyValue(OUString( "Removable"),
                                  -1,
                                  makeAny(bRemovable),
                                  PropertyState_DIRECT_VALUE);

        return aArguments;
    }
}


namespace fileaccess {


    bool isChild( const OUString& srcUnqPath,
                      const OUString& dstUnqPath )
    {
        static sal_Unicode slash = '/';
        // Simple lexical comparison
        sal_Int32 srcL = srcUnqPath.getLength();
        sal_Int32 dstL = dstUnqPath.getLength();

        return (
            ( srcUnqPath == dstUnqPath )
            ||
            ( ( dstL > srcL )
              &&
              dstUnqPath.startsWith(srcUnqPath)
              &&
              ( dstUnqPath[ srcL ] == slash ) )
        );
    }


    OUString newName(
        const OUString& aNewPrefix,
        const OUString& aOldPrefix,
        const OUString& old_Name )
    {
        sal_Int32 srcL = aOldPrefix.getLength();

        OUString new_Name = old_Name.copy( srcL );
        new_Name = ( aNewPrefix + new_Name );
        return new_Name;
    }


    OUString getTitle( const OUString& aPath )
    {
        sal_Unicode slash = '/';
        sal_Int32 lastIndex = aPath.lastIndexOf( slash );
        return aPath.copy( lastIndex + 1 );
    }


    OUString getParentName( const OUString& aFileName )
    {
        sal_Int32 lastIndex = aFileName.lastIndexOf( '/' );
        OUString aParent = aFileName.copy( 0,lastIndex );

        if( aParent.endsWith(":") && aParent.getLength() == 6 )
            aParent += "/";

        if ( aParent == "file://" )
            aParent = "file:///";

        return aParent;
    }


    osl::FileBase::RC osl_File_copy( const OUString& strPath,
                                     const OUString& strDestPath,
                                     bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::copy( strPath,strDestPath );
    }


    osl::FileBase::RC osl_File_move( const OUString& strPath,
                                     const OUString& strDestPath,
                                     bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::move( strPath,strDestPath );
    }

    void throw_handler(
        sal_Int32 errorCode,
        sal_Int32 minorCode,
        const Reference< XCommandEnvironment >& xEnv,
        const OUString& aUncPath,
        BaseContent* pContent,
        bool isHandled )
    {
        Reference<XCommandProcessor> xComProc(pContent);
        Any aAny;
        IOErrorCode ioErrorCode;

        if( errorCode ==  TASKHANDLER_UNSUPPORTED_COMMAND )
        {
            aAny <<= UnsupportedCommandException( OSL_LOG_PREFIX );
            cancelCommandExecution( aAny,xEnv );
        }
        else if( errorCode == TASKHANDLING_WRONG_SETPROPERTYVALUES_ARGUMENT ||
                 errorCode == TASKHANDLING_WRONG_GETPROPERTYVALUES_ARGUMENT ||
                 errorCode == TASKHANDLING_WRONG_OPEN_ARGUMENT              ||
                 errorCode == TASKHANDLING_WRONG_DELETE_ARGUMENT            ||
                 errorCode == TASKHANDLING_WRONG_TRANSFER_ARGUMENT          ||
                 errorCode == TASKHANDLING_WRONG_INSERT_ARGUMENT            ||
                 errorCode == TASKHANDLING_WRONG_CREATENEWCONTENT_ARGUMENT )
        {
            IllegalArgumentException excep;
            excep.ArgumentPosition = 0;
            cancelCommandExecution(Any(excep), xEnv);
        }
        else if( errorCode == TASKHANDLING_UNSUPPORTED_OPEN_MODE )
        {
            UnsupportedOpenModeException excep;
            excep.Mode = sal::static_int_cast< sal_Int16 >(minorCode);
            cancelCommandExecution( Any(excep),xEnv );
        }
        else if(errorCode == TASKHANDLING_DELETED_STATE_IN_OPEN_COMMAND  ||
                errorCode == TASKHANDLING_INSERTED_STATE_IN_OPEN_COMMAND ||
                errorCode == TASKHANDLING_NOFRESHINSERT_IN_INSERT_COMMAND )
        {
            // What to do here?
        }
        else if(
            // error in opening file
            errorCode == TASKHANDLING_NO_OPEN_FILE_FOR_OVERWRITE ||
            // error in opening file
            errorCode == TASKHANDLING_NO_OPEN_FILE_FOR_WRITE     ||
            // error in opening file
            errorCode == TASKHANDLING_OPEN_FOR_STREAM            ||
            // error in opening file
            errorCode == TASKHANDLING_OPEN_FOR_INPUTSTREAM       ||
            // error in opening file
            errorCode == TASKHANDLING_OPEN_FILE_FOR_PAGING )
        {
            switch( minorCode )
            {
                case FileBase::E_NAMETOOLONG:
                    // pathname was too long
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NXIO:
                    // No such device or address
                case FileBase::E_NODEV:
                    // No such device
                    ioErrorCode = IOErrorCode_INVALID_DEVICE;
                    break;
                case FileBase::E_NOTDIR:
                    ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
                    break;
                case FileBase::E_NOENT:
                    // No such file or directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_ROFS:
                    // #i4735# handle ROFS transparently as ACCESS_DENIED
                case FileBase::E_ACCES:
                    // permission denied<P>
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_ISDIR:
                    // Is a directory<p>
                    ioErrorCode = IOErrorCode_NO_FILE;
                    break;
                case FileBase::E_NOTREADY:
                    ioErrorCode = IOErrorCode_DEVICE_NOT_READY;
                    break;
                case FileBase::E_MFILE:
                    // too many open files used by the process
                case FileBase::E_NFILE:
                    // too many open files in the system
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:
                    // not enough memory for allocating structures
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_BUSY:
                    // Text file busy
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_AGAIN:
                    // Operation would block
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_NOLCK:  // No record locks available
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;

                case FileBase::E_FAULT: // Bad address
                case FileBase::E_LOOP:  // Too many symbolic links encountered
                case FileBase::E_NOSPC: // No space left on device
                case FileBase::E_INTR:  // function call was interrupted
                case FileBase::E_IO:    // I/O error
                case FileBase::E_MULTIHOP:      // Multihop attempted
                case FileBase::E_NOLINK:        // Link has been severed
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }

            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "an error occurred during file opening",
                xComProc);
        }
        else if( errorCode == TASKHANDLING_OPEN_FOR_DIRECTORYLISTING  ||
                 errorCode == TASKHANDLING_OPENDIRECTORY_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOENT:
                    // the specified path doesn't exist
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_NOTDIR:
                    // the specified path is not an directory
                    ioErrorCode = IOErrorCode_NO_DIRECTORY;
                    break;
                case FileBase::E_NOMEM:
                    // not enough memory for allocating structures
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ROFS:
                    // #i4735# handle ROFS transparently as ACCESS_DENIED
                case FileBase::E_ACCES:          // permission denied
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_NOTREADY:
                    ioErrorCode = IOErrorCode_DEVICE_NOT_READY;
                    break;
                case FileBase::E_MFILE:
                    // too many open files used by the process
                case FileBase::E_NFILE:
                    // too many open files in the system
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_NAMETOOLONG:
                    // File name too long
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_LOOP:
                    // Too many symbolic links encountered<p>
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }

            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "an error occurred during opening a directory",
                xComProc);
        }
        else if( errorCode == TASKHANDLING_NOTCONNECTED_FOR_WRITE          ||
                 errorCode == TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_WRITE    ||
                 errorCode == TASKHANDLING_IOEXCEPTION_FOR_WRITE           ||
                 errorCode == TASKHANDLING_NOTCONNECTED_FOR_PAGING         ||
                 errorCode == TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_PAGING   ||
                 errorCode == TASKHANDLING_IOEXCEPTION_FOR_PAGING         )
        {
            ioErrorCode = IOErrorCode_UNKNOWN;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "an error occurred writing or reading from a file",
                xComProc );
        }
        else if( errorCode == TASKHANDLING_FILEIOERROR_FOR_NO_SPACE )
        {
            ioErrorCode = IOErrorCode_OUT_OF_DISK_SPACE;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "device full",
                xComProc);
        }
        else if( errorCode == TASKHANDLING_FILEIOERROR_FOR_WRITE ||
                 errorCode == TASKHANDLING_READING_FILE_FOR_PAGING )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_FBIG:
                    // File too large
                    ioErrorCode = IOErrorCode_CANT_WRITE;
                    break;
                case FileBase::E_NOSPC:
                    // No space left on device
                    ioErrorCode = IOErrorCode_OUT_OF_DISK_SPACE;
                    break;
                case FileBase::E_NXIO:
                    // No such device or address
                    ioErrorCode = IOErrorCode_INVALID_DEVICE;
                    break;
                case FileBase::E_NOLINK:
                    // Link has been severed
                case FileBase::E_ISDIR:
                    // Is a directory
                    ioErrorCode = IOErrorCode_NO_FILE;
                    break;
                case FileBase::E_AGAIN:
                    // Operation would block
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_TIMEDOUT:
                    ioErrorCode = IOErrorCode_DEVICE_NOT_READY;
                    break;
                case FileBase::E_NOLCK:  // No record locks available
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_IO:     // I/O error
                case FileBase::E_BADF:   // Bad file
                case FileBase::E_FAULT:  // Bad address
                case FileBase::E_INTR:   // function call was interrupted
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "an error occurred during opening a file",
                xComProc);
        }
        else if( errorCode == TASKHANDLING_NONAMESET_INSERT_COMMAND ||
                 errorCode == TASKHANDLING_NOCONTENTTYPE_INSERT_COMMAND )
        {
            Sequence< OUString > aSeq( 1 );
            aSeq[0] =
                ( errorCode == TASKHANDLING_NONAMESET_INSERT_COMMAND )  ?
                OUString("Title")               :
                OUString("ContentType");

            aAny <<= MissingPropertiesException(
                OUString( "a property is missing necessary"
                               "to create a content"),
                xComProc,
                aSeq);
            cancelCommandExecution(aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_FILESIZE_FOR_WRITE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid
                case FileBase::E_OVERFLOW:
                    // The resulting file offset would be a value which cannot
                    // be represented correctly for regular files
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "there were problems with the filesize",
                xComProc);
        }
        else if(errorCode == TASKHANDLING_INPUTSTREAM_FOR_WRITE)
        {
            aAny <<=
                MissingInputStreamException(
                    OUString( "the inputstream is missing necessary"
                                   "to create a content"),
                    xComProc);
            cancelCommandExecution(aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_NOREPLACE_FOR_WRITE )
            // Overwrite = false and file exists
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            excep.Context = Reference<XInterface>( xComProc, UNO_QUERY );
            excep.Message = "file exists and overwrite forbidden";
            cancelCommandExecution( Any(excep), xEnv );
        }
        else if( errorCode == TASKHANDLING_INVALID_NAME_MKDIR )
        {
            InteractiveAugmentedIOException excep;
            excep.Code = IOErrorCode_INVALID_CHARACTER;
            PropertyValue prop;
            prop.Name = "ResourceName";
            prop.Handle = -1;
            OUString m_aClashingName(
                rtl::Uri::decode(
                    getTitle(aUncPath),
                    rtl_UriDecodeWithCharset,
                    RTL_TEXTENCODING_UTF8));
            prop.Value <<= m_aClashingName;
            Sequence<Any> seq(1);
            seq[0] <<= prop;
            excep.Arguments = seq;
            excep.Classification = InteractionClassification_ERROR;
            excep.Context = Reference<XInterface>( xComProc, UNO_QUERY );
            excep.Message = "the name contained invalid characters";
            if(isHandled)
                throw excep;
            else {
                cancelCommandExecution( Any(excep), xEnv );
            }
//              ioErrorCode = IOErrorCode_INVALID_CHARACTER;
//              cancelCommandExecution(
//                  ioErrorCode,
//                  generateErrorArguments(aUncPath),
//                  xEnv,
//                  OUString( "the name contained invalid characters"),
//                  xComProc );
        }
        else if( errorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR )
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            excep.Context = xComProc;
            excep.Message = "folder exists and overwrite forbidden";
            if(isHandled)
                throw excep;
            else {
                cancelCommandExecution( Any(excep), xEnv );
            }
//              ioErrorCode = IOErrorCode_ALREADY_EXISTING;
//              cancelCommandExecution(
//                  ioErrorCode,
//                  generateErrorArguments(aUncPath),
//                  xEnv,
//                  OUString( "the folder exists"),
//                  xComProc );
        }
        else if( errorCode == TASKHANDLING_ENSUREDIR_FOR_WRITE  ||
                 errorCode == TASKHANDLING_CREATEDIRECTORY_MKDIR )
        {
            switch( minorCode )
            {
            case FileBase::E_ACCES:
                ioErrorCode = IOErrorCode_ACCESS_DENIED;
                break;
            case FileBase::E_ROFS:
                ioErrorCode = IOErrorCode_WRITE_PROTECTED;
                break;
            case FileBase::E_NAMETOOLONG:
                ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                break;
            default:
                ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
                break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(getParentName(aUncPath)),
                //TODO! ok to supply physical URL to getParentName()?
                xEnv,
                "a folder could not be created",
                xComProc  );
        }
        else if( errorCode == TASKHANDLING_VALIDFILESTATUSWHILE_FOR_REMOVE  ||
                 errorCode == TASKHANDLING_VALIDFILESTATUS_FOR_REMOVE       ||
                 errorCode == TASKHANDLING_NOSUCHFILEORDIR_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:         // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ROFS: // #i4735# handle ROFS transparently as ACCESS_DENIED
                case FileBase::E_ACCES:         // permission denied
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_MFILE:         // too many open files used by the process
                case FileBase::E_NFILE:         // too many open files in the system
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_NOLINK:        // Link has been severed
                case FileBase::E_NOENT:         // No such file or directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOTDIR:     // A component of the path prefix of path is not a directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
                    break;
                case FileBase::E_LOOP:          // Too many symbolic links encountered
                case FileBase::E_IO:            // I/O error
                case FileBase::E_MULTIHOP:      // Multihop attempted
                case FileBase::E_FAULT:         // Bad address
                case FileBase::E_INTR:          // function call was interrupted
                case FileBase::E_NOSYS:         // Function not implemented
                case FileBase::E_NOSPC:         // No space left on device
                case FileBase::E_NXIO:          // No such device or address
                case FileBase::E_OVERFLOW:      // Value too large for defined data type
                case FileBase::E_BADF:          // Invalid oslDirectoryItem parameter
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "a file status object could not be filled",
                xComProc  );
        }
        else if( errorCode == TASKHANDLING_DELETEFILE_FOR_REMOVE  ||
                 errorCode == TASKHANDLING_DELETEDIRECTORY_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:         // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ACCES:         // Permission denied
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_PERM:          // Operation not permitted
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOLINK:        // Link has been severed
                case FileBase::E_NOENT:         // No such file or directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_ISDIR:         // Is a directory
                case FileBase::E_ROFS:          // Read-only file system
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_BUSY:          // Device or resource busy
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_FAULT:         // Bad address
                case FileBase::E_LOOP:          // Too many symbolic links encountered
                case FileBase::E_IO:            // I/O error
                case FileBase::E_INTR:          // function call was interrupted
                case FileBase::E_MULTIHOP:      // Multihop attempted
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "a file or directory could not be deleted",
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_BY_COPY_SOURCE         ||
                 errorCode == TASKHANDLING_TRANSFER_BY_COPY_SOURCESTAT     ||
                 errorCode == TASKHANDLING_TRANSFER_BY_MOVE_SOURCE         ||
                 errorCode == TASKHANDLING_TRANSFER_BY_MOVE_SOURCESTAT     ||
                 errorCode == TASKHANDLING_TRANSFER_DESTFILETYPE           ||
                 errorCode == TASKHANDLING_FILETYPE_FOR_REMOVE             ||
                 errorCode == TASKHANDLING_DIRECTORYEXHAUSTED_FOR_REMOVE   ||
                 errorCode == TASKHANDLING_TRANSFER_INVALIDURL )
        {
            OUString aMsg;
            switch( minorCode )
            {
                case FileBase::E_NOENT:         // No such file or directory
                    if ( errorCode == TASKHANDLING_TRANSFER_BY_COPY_SOURCE         ||
                         errorCode == TASKHANDLING_TRANSFER_BY_COPY_SOURCESTAT     ||
                         errorCode == TASKHANDLING_TRANSFER_BY_MOVE_SOURCE         ||
                         errorCode == TASKHANDLING_TRANSFER_BY_MOVE_SOURCESTAT )
                    {
                        ioErrorCode = IOErrorCode_NOT_EXISTING;
                        aMsg = "source file/folder does not exist";
                        break;
                    }
                    else
                    {
                        ioErrorCode = IOErrorCode_GENERAL;
                        aMsg = "a general error during transfer command";
                    break;
                    }
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    aMsg = "a general error during transfer command";
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                aMsg,
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_ACCESSINGROOT )
        {
            ioErrorCode = IOErrorCode_WRITE_PROTECTED;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                "accessing the root during transfer",
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_INVALIDSCHEME )
        {
            aAny <<=
                InteractiveBadTransferURLException(
                    OUString( "bad transfer url"),
                    xComProc);
            cancelCommandExecution( aAny,xEnv );
        }
        else if( errorCode == TASKHANDLING_OVERWRITE_FOR_MOVE      ||
                 errorCode == TASKHANDLING_OVERWRITE_FOR_COPY      ||
                 errorCode == TASKHANDLING_NAMECLASHMOVE_FOR_MOVE  ||
                 errorCode == TASKHANDLING_NAMECLASHMOVE_FOR_COPY  ||
                 errorCode == TASKHANDLING_KEEPERROR_FOR_MOVE      ||
                 errorCode == TASKHANDLING_KEEPERROR_FOR_COPY      ||
                 errorCode == TASKHANDLING_RENAME_FOR_MOVE         ||
                 errorCode == TASKHANDLING_RENAME_FOR_COPY         ||
                 errorCode == TASKHANDLING_RENAMEMOVE_FOR_MOVE     ||
                 errorCode == TASKHANDLING_RENAMEMOVE_FOR_COPY    )
        {
            OUString aMsg(
                        "general error during transfer");

            switch( minorCode )
            {
                case FileBase::E_EXIST:
                    ioErrorCode = IOErrorCode_ALREADY_EXISTING;
                    break;
                case FileBase::E_INVAL:         // the format of the parameters was not valid
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ACCES:         // Permission denied
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_PERM:          // Operation not permitted
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOENT:         // No such file or directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    aMsg = "file/folder does not exist";
                    break;
                case FileBase::E_ROFS:          // Read-only file system<p>
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(aUncPath),
                xEnv,
                aMsg,
                xComProc );
        }
        else if( errorCode == TASKHANDLING_NAMECLASH_FOR_COPY   ||
                 errorCode == TASKHANDLING_NAMECLASH_FOR_MOVE )
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            excep.Context = Reference<XInterface>( xComProc, UNO_QUERY );
            excep.Message = "name clash during copy or move";

            cancelCommandExecution(Any(excep), xEnv);
        }
        else if( errorCode == TASKHANDLING_NAMECLASHSUPPORT_FOR_MOVE   ||
                 errorCode == TASKHANDLING_NAMECLASHSUPPORT_FOR_COPY )
        {
            UnsupportedNameClashException excep;
            excep.NameClash = minorCode;
            excep.Context = Reference<XInterface>( xComProc, UNO_QUERY );
            excep.Message = "name clash value not supported during copy or move";

            cancelCommandExecution(Any(excep), xEnv);
        }
        else
        {
            // case TASKHANDLER_NO_ERROR:
            return;
        }
    }


}   // end namespace fileaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
