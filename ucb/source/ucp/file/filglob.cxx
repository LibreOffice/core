 /*************************************************************************
 *
 *  $RCSfile: filglob.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:41 $
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

#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif
#ifndef _FILERROR_HXX_
#include "filerror.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _BC_HXX_
#include "bc.hxx"
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_UNSUPPORTEDCOMMANDEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_UNSUPPORTEDOPENMODEEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_MISSINGPROPERTIESEXCEPTION_HPP_
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_MISSINGINPUTSTREAMEXCEPTION_HPP_
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDNAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include "com/sun/star/breans/PropertyState.hpp"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include "com/sun/star/breans/PropertyValue.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include "com/sun/star/uno/Any.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include "rtl/ustrbuf.hxx"
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

using namespace ucbhelper;
using namespace osl;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;

namespace {

    Sequence< Any > generateErrorArguments(
        fileaccess::shell * pShell,
        rtl::OUString const & rPhysicalUrl)
    {
        OSL_ENSURE(pShell,"specification violation");

        rtl::OUString aUri;
        rtl::OUString aResourceName;
        rtl::OUString aResourceType;
        sal_Bool      bRemovable;
        bool bUri = false;
        bool bResourceName = false;
        bool bResourceType = false;
        bool bRemoveProperty = false;

        if (pShell->uncheckMountPoint(rPhysicalUrl, aUri))
        {
            bUri = true;
            // For security reasons, exhibit the system path only if it is not
            // subject to mount point mapping:
            if (rPhysicalUrl == aUri
                && osl::FileBase::getSystemPathFromFileURL(
                    rPhysicalUrl,
                    aResourceName)
                == osl::FileBase::E_None)
                bResourceName = true;
        }

        // The resource types "folder" (i.e., directory) and
        // "volume" seem to be
        // the most interesting when producing meaningful error messages:
        osl::DirectoryItem aItem;
        if (osl::DirectoryItem::get(rPhysicalUrl, aItem) ==
            osl::FileBase::E_None)
        {
            osl::FileStatus aStatus( FileStatusMask_Type );
            if (aItem.getFileStatus(aStatus) == osl::FileBase::E_None)
                switch (aStatus.getFileType())
                {
                    case osl::FileStatus::Directory:
                        aResourceType
                            = rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("folder"));
                        bResourceType = true;
                        break;

                    case osl::FileStatus::Volume:
                        aResourceType
                            = rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("volume"));
                        bResourceType = true;
                        osl::VolumeInfo aVolumeInfo(
                            VolumeInfoMask_Attributes );
                        if( osl::Directory::getVolumeInfo(
                            rPhysicalUrl,aVolumeInfo ) ==
                            osl::FileBase::E_None )
                        {
                            bRemovable = aVolumeInfo.getRemoveableFlag();
                            bRemoveProperty = true;
                        }
                        break;
                }
        }

        Sequence< Any > aArguments( (bUri ? 1 : 0)              +
                                    (bResourceName ? 1 : 0)     +
                                    (bResourceType ? 1 : 0)     +
                                    (bRemoveProperty ? 1 : 0) );
        sal_Int32 i = 0;
        if (bUri)
            aArguments[i++]
                <<= PropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Uri")),
                                  -1,
                                  makeAny(aUri),
                                  PropertyState_DIRECT_VALUE);
        if (bResourceName)
            aArguments[i++]
                <<= PropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "ResourceName")),
                                  -1,
                                  makeAny(aResourceName),
                                  PropertyState_DIRECT_VALUE);
        if (bResourceType)
            aArguments[i++]
                <<= PropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "ResourceType")),
                                  -1,
                                  makeAny(aResourceType),
                                  PropertyState_DIRECT_VALUE);
        if (bRemoveProperty)
            aArguments[i++]
                <<= PropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Removable")),
                                  -1,
                                  makeAny(bRemovable),
                                  PropertyState_DIRECT_VALUE);

        return aArguments;
    }
}



namespace fileaccess {


    sal_Bool isChild( const rtl::OUString& srcUnqPath,
                      const rtl::OUString& dstUnqPath )
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
              ( srcUnqPath.compareTo( dstUnqPath, srcL ) == 0 )
              &&
              ( dstUnqPath[ srcL ] == slash ) )
        );
    }


    rtl::OUString newName(
        const rtl::OUString& aNewPrefix,
        const rtl::OUString& aOldPrefix,
        const rtl::OUString& old_Name )
    {
        sal_Int32 srcL = aOldPrefix.getLength();

        rtl::OUString new_Name = old_Name.copy( srcL );
        new_Name = ( aNewPrefix + new_Name );
        return new_Name;
    }


    rtl::OUString getTitle( const rtl::OUString& aPath )
    {
        sal_Unicode slash = '/';
        sal_Int32 lastIndex = aPath.lastIndexOf( slash );
        return aPath.copy( lastIndex + 1 );
    }


    rtl::OUString getParentName( const rtl::OUString& aFileName )
    {
        sal_Int32 lastIndex = aFileName.lastIndexOf( sal_Unicode('/') );
        rtl::OUString aParent = aFileName.copy( 0,lastIndex );

        if( aParent[ aParent.getLength()-1] == sal_Unicode(':') && aParent.getLength() == 6 )
            aParent += rtl::OUString::createFromAscii( "/" );

        if( 0 == aParent.compareToAscii( "file://" ) )
            aParent = rtl::OUString::createFromAscii( "file:///" );

        return aParent;
    }


    osl::FileBase::RC osl_File_copy( const rtl::OUString& strPath,
                                     const rtl::OUString& strDestPath,
                                     sal_Bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::copy( strPath,strDestPath );
    }


    osl::FileBase::RC osl_File_move( const rtl::OUString& strPath,
                                     const rtl::OUString& strDestPath,
                                     sal_Bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::move( strPath,strDestPath );
    }


    oslFileError getResolvedURL(rtl_uString* ustrPath, rtl_uString** pustrResolvedURL)
    {
        /* TODO: If file exist and is a link get link target URL */
        rtl_uString_assign( pustrResolvedURL, ustrPath );

        return osl_File_E_None;
    }



//----------------------------------------------------------------------------
//  makeAbsolute Path
//----------------------------------------------------------------------------


    sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString& aRelPath,
                                        rtl::OUString&       aAbsPath )
    {
        // should no longer happen
        OSL_ASSERT( 0 != aRelPath.compareToAscii( "//./" , 4 ) );

        if ( 0 != aRelPath.compareToAscii( "file://" , 7 ) )
            return sal_False;

        // The 'upward' ('/../') pattern.
        static const sal_Unicode pattern[5] =
        {
            '/', '.', '.', '/', 0
        };

        // Ensure 'relative path' contains 'pattern'.
        if (rtl_ustr_indexOfStr_WithLength (
            aRelPath.getStr(), aRelPath.getLength(), pattern, 4) < 0)
        {
            // Path already absolute.
            aAbsPath = aRelPath;
            return sal_True;
        }

        // Tokenize 'relative path'.
        std::vector< rtl::OUString > aTokenStack;
        sal_Int32                    nIndex = 6;

        aRelPath.getToken( 0, '/', nIndex );
        while ( nIndex >= 0 )
        {
            rtl::OUString aToken (aRelPath.getToken( 0, '/', nIndex ));

            if ( aToken.compareToAscii( ".." ) == 0 )
                aTokenStack.pop_back();
            else
                aTokenStack.push_back( aToken );
        }

        // Reassemble as 'absolute path'.
        rtl::OUStringBuffer aBuffer (aRelPath.getLength());
        aBuffer.appendAscii ("file:/", 6);

        std::vector< rtl::OUString >::const_iterator it;
        for (it = aTokenStack.begin(); it != aTokenStack.end(); ++it)
        {
            aBuffer.append (sal_Unicode('/'));
            aBuffer.append (*it);
        }

        aAbsPath = aBuffer.makeStringAndClear();

        return sal_True;
    }




    void throw_handler(
        shell * pShell,
        sal_Int32 errorCode,
        sal_Int32 minorCode,
        const Reference< XCommandEnvironment >& xEnv,
        const rtl::OUString& aUncPath,
        BaseContent* pContent,
        bool isHandled )
    {
        Reference<XCommandProcessor> xComProc(pContent);
        Any aAny;
        IOErrorCode ioErrorCode;

        if( errorCode ==  TASKHANDLER_UNSUPPORTED_COMMAND )
        {
            aAny <<= UnsupportedCommandException();
            cancelCommandExecution( aAny,xEnv );
        }
        else if( errorCode == TASKHANDLING_WRONG_SETPROPERTYVALUES_ARGUMENT ||
                 errorCode == TASKHANDLING_WRONG_GETPROPERTYVALUES_ARGUMENT ||
                 errorCode == TASKHANDLING_WRONG_OPEN_ARGUMENT              ||
                 errorCode == TASKHANDLING_WRONG_DELETE_ARGUMENT            ||
                 errorCode == TASKHANDLING_WRONG_TRANSFER_ARGUMENT          ||
                 errorCode == TASKHANDLING_WRONG_INSERT_ARGUMENT )
        {
            IllegalArgumentException excep;
            excep.ArgumentPosition = 0;
            aAny <<= excep;
            cancelCommandExecution(
                aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_UNSUPPORTED_OPEN_MODE )
        {
            UnsupportedOpenModeException excep;
            excep.Mode = minorCode;
            aAny <<= excep;
                cancelCommandExecution( aAny,xEnv );
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
                    // pathname was too long<br>
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NXIO:
                    // No such device or address<br>
                case FileBase::E_NODEV:
                    // No such device<br>
                    ioErrorCode = IOErrorCode_INVALID_DEVICE;
                    break;
                case FileBase::E_NOENT:
                    // No such file or directory<br>
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
                    // too many open files used by the process<br>
                case FileBase::E_NFILE:
                    // too many open files in the system<br>
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid<br>
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:
                    // not enough memory for allocating structures <br>
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_BUSY:
                    // Text file busy<br>
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_AGAIN:
                    // Operation would block<br>
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_FAULT: // Bad address<br>
                case FileBase::E_LOOP:  // Too many symbolic links encountered<br>
                case FileBase::E_NOSPC: // No space left on device<br>
                case FileBase::E_INTR:  // function call was interrupted<br>
                case FileBase::E_IO:    // I/O error<br>
                case FileBase::E_MULTIHOP:      // Multihop attempted<br>
                case FileBase::E_NOLINK:        // Link has been severed<br>
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }

            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell,aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "an error occured during file opening")),
                xComProc);
        }
        else if( errorCode == TASKHANDLING_OPEN_FOR_DIRECTORYLISTING  ||
                 errorCode == TASKHANDLING_OPENDIRECTORY_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid<br>
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOENT:
                    // the specified path doesn't exist<br>
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_NOTDIR:
                    // the specified path is not an directory <br>
                    ioErrorCode = IOErrorCode_NO_DIRECTORY;
                    break;
                case FileBase::E_NOMEM:
                    // not enough memory for allocating structures <br>
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ROFS:
                    // #i4735# handle ROFS transparently as ACCESS_DENIED
                case FileBase::E_ACCES:          // permission denied<br>
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_NOTREADY:
                    ioErrorCode = IOErrorCode_DEVICE_NOT_READY;
                    break;
                case FileBase::E_MFILE:
                    // too many open files used by the process<br>
                case FileBase::E_NFILE:
                    // too many open files in the system<br>
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_NAMETOOLONG:
                    // File name too long<br>
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
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "an error occured during opening a directory")),
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
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "an error occured writing or reading from a file")),
                xComProc );
        }
        else if( errorCode == TASKHANDLING_FILEIOERROR_FOR_NO_SPACE )
        {
            ioErrorCode = IOErrorCode_OUT_OF_DISK_SPACE;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell,aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "device full")),
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
                    // File too large<br>
                    ioErrorCode = IOErrorCode_CANT_WRITE;
                    break;
                case FileBase::E_NOSPC:
                    // No space left on device<br>
                    ioErrorCode = IOErrorCode_OUT_OF_DISK_SPACE;
                    break;
                case FileBase::E_NXIO:
                    // No such device or address<p>
                    ioErrorCode = IOErrorCode_INVALID_DEVICE;
                    break;
                case FileBase::E_NOLINK:
                    // Link has been severed<p>
                case FileBase::E_ISDIR:
                    // Is a directory<br>
                    ioErrorCode = IOErrorCode_NO_FILE;
                    break;
                case FileBase::E_AGAIN:
                    // Operation would block<br>
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_NOLCK:  // No record locks available<br>
                case FileBase::E_IO:     // I/O error<br>
                case FileBase::E_BADF:   // Bad file<br>
                case FileBase::E_FAULT:  // Bad address<br>
                case FileBase::E_INTR:   // function call was interrupted<br>
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "an error occured during opening a file")),
                xComProc);
        }
        else if( errorCode == TASKHANDLING_NONAMESET_INSERT_COMMAND ||
                 errorCode == TASKHANDLING_NOCONTENTTYPE_INSERT_COMMAND )
        {
            Sequence< ::rtl::OUString > aSeq( 1 );
            aSeq[0] =
                ( errorCode == TASKHANDLING_NONAMESET_INSERT_COMMAND )  ?
                rtl::OUString::createFromAscii( "Title" )               :
                rtl::OUString::createFromAscii( "ContentType" );

            aAny <<= MissingPropertiesException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "a property is missing necessary"
                        "to create a content")),
                xComProc,
                aSeq);
            cancelCommandExecution(aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_FILESIZE_FOR_WRITE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:
                    // the format of the parameters was not valid<br>
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
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "there were problems with the filesize")),
                xComProc);
        }
        else if(errorCode == TASKHANDLING_INPUTSTREAM_FOR_WRITE)
        {
            Reference<XInterface> xContext(xComProc,UNO_QUERY);
            aAny <<=
                MissingInputStreamException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "the inputstream is missing necessary"
                            "to create a content")),
                    xContext);
            cancelCommandExecution(aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_NOREPLACE_FOR_WRITE )
            // Overwrite = false and file exists
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            Reference<XInterface> xContext(xComProc,UNO_QUERY);
            excep.Context = xContext;
            excep.Message = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "file exists and overwritte forbidden"));
            aAny <<= excep;
            cancelCommandExecution( aAny,xEnv );
        }
        else if( errorCode == TASKHANDLING_INVALID_NAME_MKDIR )
        {
            InteractiveAugmentedIOException excep;
            excep.Code = IOErrorCode_INVALID_CHARACTER;
            PropertyValue prop;
            prop.Name = rtl::OUString::createFromAscii("ResourceName");
            prop.Handle = -1;
            rtl::OUString m_aClashingName(
                rtl::Uri::decode(
                    getTitle(aUncPath),
                    rtl_UriDecodeWithCharset,
                    RTL_TEXTENCODING_UTF8));
            prop.Value <<= m_aClashingName;
            Sequence<Any> seq(1);
            seq[0] <<= prop;
            excep.Arguments = seq;
            excep.Classification = InteractionClassification_ERROR;
            Reference<XInterface> xContext(xComProc,UNO_QUERY);
            excep.Context = xContext;
            excep.Message = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "the name contained invalid characters"));
            if(isHandled)
                throw excep;
            else {
                aAny <<= excep;
                cancelCommandExecution( aAny,xEnv );
            }
//              ioErrorCode = IOErrorCode_INVALID_CHARACTER;
//              cancelCommandExecution(
//                  ioErrorCode,
//                  generateErrorArguments(pShell,aUncPath),
//                  xEnv,
//                  rtl::OUString(
//                      RTL_CONSTASCII_USTRINGPARAM(
//                          "the name contained invalid characters")),
//                  xComProc );
        }
        else if( errorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR )
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            Reference<XInterface> xContext(xComProc,UNO_QUERY);
            excep.Context = xContext;
            excep.Message = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "folder exists and overwritte forbidden"));
            if(isHandled)
                throw excep;
            else {
                aAny <<= excep;
                cancelCommandExecution( aAny,xEnv );
            }
//          ioErrorCode = IOErrorCode_ALREADY_EXISTING;
//          cancelCommandExecution(
//              ioErrorCode,
//              generateErrorArguments(pShell,aUncPath),
//              xEnv,
//              rtl::OUString(
//                  RTL_CONSTASCII_USTRINGPARAM(
//                      "the folder exists")),
//              xComProc );
        }
        else if( errorCode == TASKHANDLING_ENSUREDIR_FOR_WRITE  ||
                 errorCode == TASKHANDLING_CREATEDIRECTORY_MKDIR )
        {
            ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell,getParentName(aUncPath)),
                //TODO! ok to supply physical URL to getParentName()?
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "a folder could not be created")),
                xComProc  );
        }
        else if( errorCode == TASKHANDLING_VALIDFILESTATUSWHILE_FOR_REMOVE  ||
                 errorCode == TASKHANDLING_VALIDFILESTATUS_FOR_REMOVE       ||
                 errorCode == TASKHANDLING_NOSUCHFILEORDIR_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:         // the format of the parameters was not valid<br>
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures <br>
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ROFS: // #i4735# handle ROFS transparently as ACCESS_DENIED
                case FileBase::E_ACCES:         // permission denied<br>
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_MFILE:         // too many open files used by the process<br>
                case FileBase::E_NFILE:         // too many open files in the system<br>
                    ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                    break;
                case FileBase::E_NOLINK:        // Link has been severed<br>
                case FileBase::E_NOENT:         // No such file or directory<br>
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long<br>
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOTDIR:     // A component of the path prefix of path is not a directory
                    ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
                    break;
                case FileBase::E_LOOP:          // Too many symbolic links encountered<br>
                case FileBase::E_IO:            // I/O error<br>
                case FileBase::E_MULTIHOP:      // Multihop attempted<br>
                case FileBase::E_FAULT:         // Bad address<br>
                case FileBase::E_INTR:          // function call was interrupted<p>
                case FileBase::E_NOSYS:         // Function not implemented<p>
                case FileBase::E_NOSPC:         // No space left on device<br>
                case FileBase::E_NXIO:          // No such device or address<br>
                case FileBase::E_OVERFLOW:      // Value too large for defined data type<br>
                case FileBase::E_BADF:          // Invalid oslDirectoryItem parameter<br>
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "a file status object could not be filled")),
                xComProc  );
        }
        else if( errorCode == TASKHANDLING_DELETEFILE_FOR_REMOVE  ||
                 errorCode == TASKHANDLING_DELETEDIRECTORY_FOR_REMOVE )
        {
            switch( minorCode )
            {
                case FileBase::E_INVAL:         // the format of the parameters was not valid<br>
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures <br>
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ACCES:         // Permission denied<br>
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_PERM:          // Operation not permitted<br>
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long<br>
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOLINK:        // Link has been severed<br>
                case FileBase::E_NOENT:         // No such file or directory<br>
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
                    break;
                case FileBase::E_ISDIR:         // Is a directory<br>
                case FileBase::E_ROFS:          // Read-only file system<p>
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_BUSY:          // Device or resource busy<br>
                    ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                    break;
                case FileBase::E_FAULT:         // Bad address<br>
                case FileBase::E_LOOP:          // Too many symbolic links encountered<br>
                case FileBase::E_IO:            // I/O error<br>
                case FileBase::E_INTR:          // function call was interrupted<br>
                case FileBase::E_MULTIHOP:      // Multihop attempted<br>
                default:
                    ioErrorCode = IOErrorCode_GENERAL;
                    break;
            }
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "a file or directory could not be deleted")),
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_MOUNTPOINTS )
        {
            ioErrorCode = IOErrorCode_NOT_EXISTING;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "")),
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
            ioErrorCode = IOErrorCode_GENERAL;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "a general error during transfer command")),
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_ACCESSINGROOT )
        {
            ioErrorCode = IOErrorCode_WRITE_PROTECTED;
            cancelCommandExecution(
                ioErrorCode,
                generateErrorArguments(pShell,aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "accessing the root during transfer")),
                xComProc );
        }
        else if( errorCode == TASKHANDLING_TRANSFER_INVALIDSCHEME )
        {
            Reference<XInterface> xContext(xComProc,UNO_QUERY);

            aAny <<=
                InteractiveBadTransferURLException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad tranfer url")),
                    xContext);
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
            switch( minorCode )
            {
                case FileBase::E_EXIST:
                    ioErrorCode = IOErrorCode_ALREADY_EXISTING;
                    break;
                case FileBase::E_INVAL:         // the format of the parameters was not valid<br>
                    ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                    break;
                case FileBase::E_NOMEM:         // not enough memory for allocating structures <br>
                    ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                    break;
                case FileBase::E_ACCES:         // Permission denied<br>
                    ioErrorCode = IOErrorCode_ACCESS_DENIED;
                    break;
                case FileBase::E_PERM:          // Operation not permitted<br>
                    ioErrorCode = IOErrorCode_NOT_SUPPORTED;
                    break;
                case FileBase::E_NAMETOOLONG:   // File name too long<br>
                    ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                    break;
                case FileBase::E_NOENT:         // No such file or directory<br>
                    ioErrorCode = IOErrorCode_NOT_EXISTING;
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
                generateErrorArguments(pShell, aUncPath),
                xEnv,
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "general error during transfer")),
                xComProc );
        }
        else if( errorCode == TASKHANDLING_NAMECLASH_FOR_COPY   ||
                 errorCode == TASKHANDLING_NAMECLASH_FOR_MOVE )
        {
            NameClashException excep;
            excep.Name = getTitle(aUncPath);
            excep.Classification = InteractionClassification_ERROR;
            Reference<XInterface> xContext(xComProc,UNO_QUERY);
            excep.Context = xContext;
            excep.Message = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "name clash during copy or move"));
            aAny <<= excep;

            cancelCommandExecution(aAny,xEnv);
        }
        else if( errorCode == TASKHANDLING_NAMECLASHSUPPORT_FOR_MOVE   ||
                 errorCode == TASKHANDLING_NAMECLASHSUPPORT_FOR_COPY )
        {
            Reference<XInterface> xContext(
                xComProc,UNO_QUERY);
            UnsupportedNameClashException excep;
            excep.NameClash = minorCode;
            excep.Context = xContext;
            excep.Message = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "name clash value not supported during copy or move"));

            aAny <<= excep;
            cancelCommandExecution(aAny,xEnv);
        }
        else
        {
            // case TASKHANDLER_NO_ERROR:
            return;
        }
    }


}   // end namespace fileaccess
