#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif
#ifndef _FILERROR_HXX_
#include "filerror.hxx"
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


using namespace ucbhelper;
using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;



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
#ifdef TF_FILEURL
        /* TODO: If file exist and is a link get link target URL */
        rtl_uString_assign( pustrResolvedURL, ustrPath );
#else
        rtl_uString_assign( pustrResolvedURL, ustrPath );
#endif

        return osl_File_E_None;
    }



//----------------------------------------------------------------------------
//  makeAbsolute Path
//----------------------------------------------------------------------------

#ifdef TF_FILEURL

    sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString&    aRelPath, rtl::OUString& aAbsPath )
    {
        sal_Int32   nIndex = 6;

        std::vector< rtl::OUString >    aTokenStack;

        // should no longer happen
        OSL_ASSERT( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "//./" ), 4 ) );

        if ( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "file://" ), 7 ) )
            return sal_False;

        aRelPath.getToken( 0, '/', nIndex );

        while ( nIndex >= 0 )
        {
            rtl::OUString   aToken = aRelPath.getToken( 0, '/', nIndex );

            if ( aToken.compareToAscii( ".." ) == 0 )
                aTokenStack.pop_back();
            else
                aTokenStack.push_back( aToken );
        }


        std::vector< rtl::OUString >::iterator it;
        aAbsPath = rtl::OUString::createFromAscii("file:/");

        for ( it = aTokenStack.begin(); it != aTokenStack.end(); it++ )
        {
            aAbsPath += rtl::OUString::createFromAscii( "/" );
            aAbsPath += *it;
        }

        return sal_True;
    }

#else


    sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString& aRelPath, rtl::OUString& aAbsPath )
    {
        sal_Int32   nIndex = 0;

        std::vector< rtl::OUString >    aTokenStack;

        if ( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "//./" ), 4 ) )
            return sal_False;

        aRelPath.getToken( 0, '/', nIndex );

        while ( nIndex >= 0 )
        {
            rtl::OUString   aToken = aRelPath.getToken( 0, '/', nIndex );

            if ( aToken.compareToAscii( ".." ) == 0 )
                aTokenStack.pop_back();
            else
                aTokenStack.push_back( aToken );
        }


        std::vector< rtl::OUString >::iterator it;
        aAbsPath = rtl::OUString::createFromAscii("file:");

        for ( it = aTokenStack.begin(); it != aTokenStack.end(); it++ )
        {
            aAbsPath += rtl::OUString::createFromAscii( "/" );
            aAbsPath += *it;
        }

        return sal_True;
    }

#endif




    void throw_handler( sal_Int32 errorCode,
                        sal_Int32 minorCode,
                        const Reference< XCommandEnvironment >& xEnv,
                        const rtl::OUString& aUncPath )
    {
        Any aAny;
        IOErrorCode ioErrorCode;
        aAny <<= CommandAbortedException();

        switch( errorCode )
        {
            case TASKHANDLER_UNSUPPORTED_COMMAND:
                aAny <<= UnsupportedCommandException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_WRONG_SETPROPERTYVALUES_ARGUMENT:
            case TASKHANDLING_WRONG_GETPROPERTYVALUES_ARGUMENT:
            case TASKHANDLING_WRONG_OPEN_ARGUMENT:
            case TASKHANDLING_WRONG_DELETE_ARGUMENT:
            case TASKHANDLING_WRONG_TRANSFER_ARGUMENT:
            case TASKHANDLING_WRONG_INSERT_ARGUMENT:
                aAny <<= IllegalArgumentException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_UNSUPPORTED_OPEN_MODE:
                aAny <<= UnsupportedOpenModeException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_DELETED_STATE_IN_OPEN_COMMAND:
            case TASKHANDLING_INSERTED_STATE_IN_OPEN_COMMAND:
            case TASKHANDLING_NOFRESHINSERT_IN_INSERT_COMMAND:
                // what to do here ?
                break;

            case TASKHANDLING_NO_OPEN_FILE_FOR_OVERWRITE:  // error in opening file
            case TASKHANDLING_NO_OPEN_FILE_FOR_WRITE:      // error in opening file
            case TASKHANDLING_OPEN_FOR_STREAM:             // error in opening file
            case TASKHANDLING_OPEN_FOR_INPUTSTREAM:        // error in opening file
            case TASKHANDLING_OPEN_FILE_FOR_PAGING:        // error in opening file
            {
                switch( minorCode )
                {
                    case FileBase::E_NAMETOOLONG:       // pathname was too long<br>
                        ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                        break;
                    case FileBase::E_NXIO:          // No such device or address<br>
                    case FileBase::E_NODEV:         // No such device<br>
                        ioErrorCode = IOErrorCode_INVALID_DEVICE;
                        break;
                    case FileBase::E_NOENT:         // No such file or directory<br>
                        ioErrorCode = IOErrorCode_NOT_EXISTING;
                        break;
                    case FileBase::E_ACCES:         // permission denied<P>
                        ioErrorCode = IOErrorCode_ACCESS_DENIED;
                        break;
                    case FileBase::E_ISDIR:         // Is a directory<p>
                        ioErrorCode = IOErrorCode_NO_FILE;
                        break;
                    case FileBase::E_MFILE:         // too many open files used by the process<br>
                    case FileBase::E_NFILE:         // too many open files in the system<br>
                        ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                        break;
                    case FileBase::E_INVAL:         // the format of the parameters was not valid<br>
                        ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                        break;
                    case FileBase::E_NOMEM:         // not enough memory for allocating structures <br>
                        ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                        break;
                    case FileBase::E_BUSY:          // Text file busy<br>
                        ioErrorCode = IOErrorCode_LOCKING_VIOLATION;
                        break;
                    case FileBase::E_FAULT:         // Bad address<br>
                    case FileBase::E_LOOP:          // Too many symbolic links encountered<br>
                    case FileBase::E_NOSPC:         // No space left on device<br>
                    case FileBase::E_INTR:          // function call was interrupted<br>
                    case FileBase::E_IO:            // I/O error<br>
                    case FileBase::E_MULTIHOP:      // Multihop attempted<br>
                    case FileBase::E_NOLINK:        // Link has been severed<br>
                    default:
                        ioErrorCode = IOErrorCode_GENERAL;
                        break;
                }

                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }

            case TASKHANDLING_OPEN_FOR_DIRECTORYLISTING:
            case TASKHANDLING_OPENDIRECTORY_FOR_REMOVE:
            {
                switch( minorCode )
                {
                    case FileBase::E_INVAL:          // the format of the parameters was not valid<br>
                        ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                        break;
                    case FileBase::E_NOENT:          // the specified path doesn't exist<br>
                        ioErrorCode = IOErrorCode_NOT_EXISTING;
                        break;
                    case FileBase::E_NOTDIR:             // the specified path is not an directory <br>
                        ioErrorCode = IOErrorCode_NO_DIRECTORY;
                        break;
                    case FileBase::E_NOMEM:          // not enough memory for allocating structures <br>
                        ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                        break;
                    case FileBase::E_ACCES:          // permission denied<br>
                        ioErrorCode = IOErrorCode_ACCESS_DENIED;
                        break;
                    case FileBase::E_MFILE:          // too many open files used by the process<br>
                    case FileBase::E_NFILE:          // too many open files in the system<br>
                        ioErrorCode = IOErrorCode_OUT_OF_FILE_HANDLES;
                        break;
                    case FileBase::E_NAMETOOLONG:        // File name too long<br>
                        ioErrorCode = IOErrorCode_NAME_TOO_LONG;
                        break;
                    case FileBase::E_LOOP:           // Too many symbolic links encountered<p>
                    default:
                        ioErrorCode = IOErrorCode_GENERAL;
                        break;
                }

                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }

            case TASKHANDLING_NOTCONNECTED_FOR_WRITE:
            case TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_WRITE:
            case TASKHANDLING_IOEXCEPTION_FOR_WRITE:
            case TASKHANDLING_NOTCONNECTED_FOR_PAGING:
            case TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_PAGING:
            case TASKHANDLING_IOEXCEPTION_FOR_PAGING:
                ioErrorCode = IOErrorCode_UNKNOWN;
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;

            case TASKHANDLING_FILEIOERROR_FOR_WRITE:
            case TASKHANDLING_READING_FILE_FOR_PAGING:
            {
                switch( minorCode )
                {
                    case FileBase::E_INVAL:             // the format of the parameters was not valid
                        ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                        break;
                    case FileBase::E_FBIG:              // File too large<br>
                        ioErrorCode = IOErrorCode_CANT_WRITE;
                        break;
                    case FileBase::E_NOSPC:             // No space left on device<br>
                        ioErrorCode = IOErrorCode_OUT_OF_DISK_SPACE;
                        break;
                    case FileBase::E_NXIO:              // No such device or address<p>
                        ioErrorCode = IOErrorCode_INVALID_DEVICE;
                        break;
                    case FileBase::E_NOLINK:            // Link has been severed<p>
                    case FileBase::E_ISDIR:             // Is a directory<br>
                        ioErrorCode = IOErrorCode_NO_FILE;
                        break;
                    case FileBase::E_NOLCK:             // No record locks available<br>
                    case FileBase::E_IO:                // I/O error<br>
                    case FileBase::E_BADF:              // Bad file<br>
                    case FileBase::E_FAULT:             // Bad address<br>
                    case FileBase::E_AGAIN:             // Operation would block<br>
                    case FileBase::E_INTR:              // function call was interrupted<br>
                    default:
                        ioErrorCode = IOErrorCode_GENERAL;
                        break;
                }
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }

            case TASKHANDLING_NONAMESET_INSERT_COMMAND:
            case TASKHANDLING_NOCONTENTTYPE_INSERT_COMMAND:
            {
                Sequence< ::rtl::OUString > aSeq( 1 );
                aSeq[0] =
                    ( errorCode == TASKHANDLING_NONAMESET_INSERT_COMMAND )  ?
                    rtl::OUString::createFromAscii( "Title" )               :
                    rtl::OUString::createFromAscii( "ContentType" );

                aAny <<= MissingPropertiesException( rtl::OUString(),
                                                     0,
                                                     aSeq );
                cancelCommandExecution( aAny,xEnv );
                break;
            }

            case TASKHANDLING_TRANSFER_INVALIDSCHEME:
                aAny <<= InteractiveBadTransferURLException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_TRANSFER_BY_MOVE_SOURCE:
                break;
            case TASKHANDLING_TRANSFER_BY_MOVE_SOURCESTAT:
                break;
            case TASKHANDLING_TRANSFER_BY_MOVE_FILETYPE:
                break;

            case TASKHANDLING_FILESIZE_FOR_WRITE:
            {
                switch( minorCode )
                {
                    case FileBase::E_INVAL:     // the format of the parameters was not valid<br>
                    case FileBase::E_OVERFLOW:  // The resulting file offset would be a value which cannot
                        //                      //  be represented correctly for regular files
                        ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                        break;
                    default:
                        ioErrorCode = IOErrorCode_GENERAL;
                        break;
                }
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }
            case TASKHANDLING_INPUTSTREAM_FOR_WRITE:
                aAny <<= MissingInputStreamException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_NOREPLACE_FOR_WRITE:    // Overwrite = false and file exists
                aAny <<= NameClashException();
                cancelCommandExecution( aAny,xEnv );
                break;

            case TASKHANDLING_ENSUREDIR_FOR_WRITE:
            case TASKHANDLING_CREATEDIRECTORY_MKDIR:
                ioErrorCode = IOErrorCode_NOT_EXISTING_PATH;
                cancelCommandExecution( ioErrorCode,getParentName( aUncPath ),xEnv );
                break;

            case TASKHANDLING_VALIDFILESTATUSWHILE_FOR_REMOVE:
            case TASKHANDLING_VALIDFILESTATUS_FOR_REMOVE:
            case TASKHANDLING_NOSUCHFILEORDIR_FOR_REMOVE:
            {
                switch( minorCode )
                {
                    case FileBase::E_INVAL:         // the format of the parameters was not valid<br>
                        ioErrorCode = IOErrorCode_INVALID_PARAMETER;
                        break;
                    case FileBase::E_NOMEM:         // not enough memory for allocating structures <br>
                        ioErrorCode = IOErrorCode_OUT_OF_MEMORY;
                        break;
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
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }

            case TASKHANDLING_DELETEFILE_FOR_REMOVE:
            case TASKHANDLING_DELETEDIRECTORY_FOR_REMOVE:
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
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;
            }

            case TASKHANDLING_FILETYPE_FOR_REMOVE:
            case TASKHANDLING_DIRECTORYEXHAUSTED_FOR_REMOVE:
                ioErrorCode = IOErrorCode_GENERAL;
                cancelCommandExecution( ioErrorCode,aUncPath,xEnv );
                break;

            case TASKHANDLER_NO_ERROR:
            default:
                return;
        }
    }


}   // end namespace fileaccess
