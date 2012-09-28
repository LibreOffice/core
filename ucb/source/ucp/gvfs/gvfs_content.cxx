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

#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sal/macros.h>
#include <osl/time.h>
#include <osl/diagnose.h>

#include "osl/doublecheckedlocking.h"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkGeneralException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/simpleauthenticationrequest.hxx>

const int TRANSFER_BUFFER_SIZE = 65536;

/*
 * NB. Name escaping is done only for URIs
 * the 'Title' property is unescaped on set/get
 */
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-standard-callbacks.h>
extern "C" { // missing in the header: doh.
#  include <libgnomevfs/gnome-vfs-module-callback.h>
}

#include "gvfs_content.hxx"
#include "gvfs_provider.hxx"
#include "gvfs_directory.hxx"
#include "gvfs_stream.hxx"

using namespace gvfs;
using namespace com::sun::star;

#define CLEAR_INFO(info) memset((info), 0, sizeof ((info)[0]))


static char *
OUStringToGnome( const rtl::OUString &str )
{
    rtl::OString aTempStr = rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 );
    return g_strdup( aTempStr.getStr() );
}

static rtl::OUString
GnomeToOUString( const char *utf8_str)
{
    if (!utf8_str)
        return rtl::OUString();
    else
        return rtl::OUString( utf8_str, strlen( utf8_str ), RTL_TEXTENCODING_UTF8 );
}


Content::Content(
          const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
          ContentProvider* pProvider,
          const uno::Reference< ucb::XContentIdentifier >& Identifier)
  throw ( ucb::ContentCreationException )
    : ContentImplHelper( rxSMgr, pProvider, Identifier ),
      m_pProvider( pProvider ),
      m_bTransient( sal_False )
{
    CLEAR_INFO (&m_info);
#if OSL_DEBUG_LEVEL > 1
    g_warning ("New Content ('%s')", getURI());
#endif
}

Content::Content(
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
    ContentProvider                                   * pProvider,
    const uno::Reference< ucb::XContentIdentifier >&    Identifier,
    sal_Bool                                            IsFolder)
        throw ( ucb::ContentCreationException )
    : ContentImplHelper( rxSMgr, pProvider, Identifier ),
      m_pProvider( pProvider ),
      m_bTransient( sal_True )
{
    CLEAR_INFO (&m_info);

#if OSL_DEBUG_LEVEL > 1
    g_warning ("New Transient content ('%s') (%d)", getURI(), IsFolder);
#endif
//  m_info.name = FIXME: set name ?
    m_info.valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE;
    m_info.type = IsFolder ? GNOME_VFS_FILE_TYPE_DIRECTORY :
                         GNOME_VFS_FILE_TYPE_REGULAR;
}

// virtual
Content::~Content()
{
    gnome_vfs_file_info_clear( &m_info );
}

//
// XInterface methods.
//

void SAL_CALL Content::acquire()
    throw( )
{
    ContentImplHelper::acquire();
}
void SAL_CALL Content::release()
    throw( )
{
    ContentImplHelper::release();
}
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    // Note: isFolder may require network activities! So call it only
    //       if it is really necessary!!!
    uno::Any aRet = cppu::queryInterface( rType,
                        static_cast< ucb::XContentCreator * >( this ) );
    if ( aRet.hasValue() )
            return isFolder( uno::Reference< ucb::XCommandEnvironment >() )
            ? aRet : uno::Any();
    else
        return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//
// XTypeProvider methods.
//

XTYPEPROVIDER_COMMON_IMPL( Content );

uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection *pFolderCollection = NULL;
    static cppu::OTypeCollection *pFileCollection = NULL;

    if (!pFolderCollection) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

        if (!pFolderCollection) {
            static cppu::OTypeCollection aFolderCollection
                (CPPU_TYPE_REF( lang::XTypeProvider ),
                 CPPU_TYPE_REF( lang::XServiceInfo ),
                 CPPU_TYPE_REF( lang::XComponent ),
                 CPPU_TYPE_REF( ucb::XContent ),
                 CPPU_TYPE_REF( ucb::XCommandProcessor ),
                 CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                 CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                 CPPU_TYPE_REF( beans::XPropertyContainer ),
                 CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                 CPPU_TYPE_REF( container::XChild ),
                 CPPU_TYPE_REF( ucb::XContentCreator ) ); // !!
            static cppu::OTypeCollection aFileCollection
                (CPPU_TYPE_REF( lang::XTypeProvider ),
                 CPPU_TYPE_REF( lang::XServiceInfo ),
                 CPPU_TYPE_REF( lang::XComponent ),
                 CPPU_TYPE_REF( ucb::XContent ),
                 CPPU_TYPE_REF( ucb::XCommandProcessor ),
                 CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                 CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                 CPPU_TYPE_REF( beans::XPropertyContainer ),
                 CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                 CPPU_TYPE_REF( container::XChild ) );

            pFolderCollection = &aFolderCollection;
            pFileCollection = &aFileCollection;
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }
    else {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
        return pFolderCollection->getTypes();
    else
        return pFileCollection->getTypes();
}

//
// XServiceInfo methods.
//

rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    return rtl::OUString("com.sun.star.comp.GnomeVFSContent");
}

uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = rtl::OUString( "com.sun.star.ucb.GnomeVFSContent" );
    return aSNS;
}

//
// XContent methods.
//

rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
    if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
        return rtl::OUString( GVFS_FOLDER_TYPE );
    else
        return rtl::OUString( GVFS_FILE_TYPE );
}

//
// XCommandProcessor methods.
//

uno::Any Content::getBadArgExcept()
{
    return uno::makeAny( lang::IllegalArgumentException
                 ( rtl::OUString("Wrong argument type!"),
                   static_cast< cppu::OWeakObject * >( this ),
                   -1 ) );
}

#include <stdio.h>

uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

#if OSL_DEBUG_LEVEL > 1
    {
        uno::Reference< task::XInteractionHandler > xIH;

        if ( xEnv.is() )
            xIH = xEnv->getInteractionHandler();
        g_warning( "Execute command: '%s' with %s interaction env",
               OUStringToGnome( aCommand.Name ),
               xIH.is() ? "" : "NO" );
    }
#endif

#define COMMAND_IS(cmd,name) ( (cmd).Name == name )

    if ( COMMAND_IS( aCommand, "getPropertyValues" ) ) {
        uno::Sequence< beans::Property > Properties;

        if ( !( aCommand.Argument >>= Properties ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

        aRet <<= getPropertyValues( Properties, xEnv );

    } else if ( COMMAND_IS( aCommand, "setPropertyValues" ) ) {
        uno::Sequence< beans::PropertyValue > aProperties;

        if ( !( aCommand.Argument >>= aProperties ) ||
             !aProperties.getLength() )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

        aRet <<= setPropertyValues( aProperties, xEnv );

    } else if ( COMMAND_IS( aCommand, "getPropertySetInfo" ) ) {
        aRet <<= getPropertySetInfo( xEnv, sal_False );

    } else if ( COMMAND_IS( aCommand, "getCommandInfo" ) ) {
        aRet <<= getCommandInfo( xEnv, sal_False );

    } else if ( COMMAND_IS( aCommand, "open" ) ) {
        rtl::OUString str = m_xIdentifier->getContentIdentifier();
        rtl::OString stra(
            str.getStr(),
            str.getLength(),
            RTL_TEXTENCODING_UTF8);

        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == ucb::OpenMode::ALL ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENTS ) );

        if ( bOpenFolder && isFolder( xEnv ) ) {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(m_xSMgr, this, aOpenCommand, xEnv );
            aRet <<= xSet;

        } else if ( aOpenCommand.Sink.is() ) {

            if ( ( aOpenCommand.Mode
                   == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                   == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) ) {
                ucbhelper::cancelCommandExecution
                    ( uno::makeAny ( ucb::UnsupportedOpenModeException
                             ( rtl::OUString(),
                               static_cast< cppu::OWeakObject * >( this ),
                               sal_Int16( aOpenCommand.Mode ) ) ),
                      xEnv );
            }
            if ( !feedSink( aOpenCommand.Sink, xEnv ) ) {
                // Note: aOpenCommand.Sink may contain an XStream
                //       implementation. Support for this type of
                //       sink is optional...
#ifdef DEBUG
                g_warning ("Failed to load data from '%s'", getURI());
#endif
                ucbhelper::cancelCommandExecution
                    ( uno::makeAny (ucb::UnsupportedDataSinkException
                            ( rtl::OUString(),
                              static_cast< cppu::OWeakObject * >( this ),
                              aOpenCommand.Sink ) ),
                      xEnv );
            }
        }
#ifdef DEBUG
        else
            g_warning ("Open falling through ...");
#endif

    } else if ( COMMAND_IS( aCommand, "createNewContent" ) && isFolder( xEnv ) ) {
        ucb::ContentInfo arg;
        if ( !( aCommand.Argument >>= arg ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

        aRet <<= createNewContent( arg );

    } else if ( COMMAND_IS( aCommand, "insert" ) ) {
        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

        insert( arg.Data, arg.ReplaceExisting, xEnv );

    } else if ( COMMAND_IS( aCommand, "delete" ) ) {

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;

        ::rtl::OString aURI = getOURI();
        GnomeVFSResult result = gnome_vfs_unlink (aURI.getStr());

        if (result != GNOME_VFS_OK)
            cancelCommandExecution( result, xEnv, sal_True );

        destroy( bDeletePhysical );

    } else if ( COMMAND_IS( aCommand, "transfer" ) && isFolder( xEnv ) ) {
        ucb::TransferInfo transferArgs;

        if ( !( aCommand.Argument >>= transferArgs ) )
            ucbhelper::cancelCommandExecution( getBadArgExcept(), xEnv );

        transfer( transferArgs, xEnv );

    } else { // Unsuported
#ifdef DEBUG
        g_warning( "Unsupported command: '%s'",
               OUStringToGnome( aCommand.Name ) );
#endif
        ucbhelper::cancelCommandExecution
            ( uno::makeAny( ucb::UnsupportedCommandException
                    ( rtl::OUString(),
                      static_cast< cppu::OWeakObject * >( this ) ) ),
              xEnv );
    }
#undef COMMAND_IS

    return aRet;
}

void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
    // FIXME: we should use the GnomeVFSCancellation APIs here ...
}

//
// XContentCreator methods.
//

uno::Sequence< ucb::ContentInfo > Content::queryCreatableContentsInfo(
    const uno::Reference< ucb::XCommandEnvironment >& xEnv)
            throw( uno::RuntimeException )
{
    if ( isFolder( xEnv ) )
    {
        uno::Sequence< ucb::ContentInfo > seq(2);

        // Minimum set of props we really need
        uno::Sequence< beans::Property > props( 1 );
        props[0] = beans::Property(
            rtl::OUString("Title"),
            -1,
            getCppuType( static_cast< rtl::OUString* >( 0 ) ),
            beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::BOUND );

        // file
        seq[0].Type       = rtl::OUString( GVFS_FILE_TYPE );
        seq[0].Attributes = ( ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                              ucb::ContentInfoAttribute::KIND_DOCUMENT );
        seq[0].Properties = props;

        // folder
        seq[1].Type       = rtl::OUString( GVFS_FOLDER_TYPE );
        seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
        seq[1].Properties = props;

        return seq;
    }
    else
    {
        return uno::Sequence< ucb::ContentInfo >();
    }
}

uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
            throw( uno::RuntimeException )
{
    return queryCreatableContentsInfo( uno::Reference< ucb::XCommandEnvironment >() );
}

uno::Reference< ucb::XContent > SAL_CALL
Content::createNewContent( const ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    bool create_document;
    const char *name;

        if ( Info.Type == GVFS_FILE_TYPE )
        create_document = true;
    else if ( Info.Type == GVFS_FOLDER_TYPE )
        create_document = false;
    else {
#ifdef DEBUG
        g_warning( "Failed to create new content '%s'",
               OUStringToGnome( Info.Type ) );
#endif
        return uno::Reference< ucb::XContent >();
    }

#if OSL_DEBUG_LEVEL > 1
    g_warning( "createNewContent (%d)", (int) create_document );
#endif

        rtl::OUString aURL = getOUURI();

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
        aURL += rtl::OUString("/");

    name = create_document ? "[New_Content]" : "[New_Collection]";
    // This looks problematic to me cf. webdav
    aURL += rtl::OUString::createFromAscii( name );

        uno::Reference< ucb::XContentIdentifier > xId
        ( new ::ucbhelper::ContentIdentifier( m_xSMgr, aURL ) );

        try {
        return new ::gvfs::Content( m_xSMgr, m_pProvider, xId, !create_document );
    } catch ( ucb::ContentCreationException & ) {
        return uno::Reference< ucb::XContent >();
        }
}

rtl::OUString Content::getParentURL()
{
    rtl::OUString aParentURL;
    // <scheme>://              -> ""
    // <scheme>://foo           -> ""
    // <scheme>://foo/          -> ""
    // <scheme>://foo/bar       -> <scheme>://foo/
    // <scheme>://foo/bar/      -> <scheme>://foo/
    // <scheme>://foo/bar/abc   -> <scheme>://foo/bar/

    rtl::OUString aURL = getOUURI();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos == ( aURL.getLength() - 1 ) ) {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    sal_Int32 nPos1 = aURL.lastIndexOf( '/', nPos );
    if ( nPos1 != -1 )
        nPos1 = aURL.lastIndexOf( '/', nPos1 );

    if ( nPos1 != -1 )
        aParentURL = rtl::OUString( aURL.copy( 0, nPos + 1 ) );

#if OSL_DEBUG_LEVEL > 1
    g_warning ("getParentURL '%s' -> '%s'",
           getURI(), rtl::OUStringToOString
               ( aParentURL, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    return aParentURL;
}

static util::DateTime
getDateFromUnix (time_t t)
{
    TimeValue tv;
    tv.Nanosec = 0;
    tv.Seconds = t;
    oslDateTime dt;

    if ( osl_getDateTimeFromTimeValue( &tv, &dt ) )
        return util::DateTime( 0, dt.Seconds, dt.Minutes, dt.Hours,
                               dt.Day, dt.Month, dt.Year);
    else
        return util::DateTime();
}

uno::Reference< sdbc::XRow > Content::getPropertyValues(
                const uno::Sequence< beans::Property >& rProperties,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    int nProps;
    GnomeVFSResult result;
    uno::Sequence< beans::Property > allProperties;

    if( ( result = getInfo( xEnv ) ) != GNOME_VFS_OK )
        cancelCommandExecution( result, xEnv, sal_False );

    const beans::Property* pProps;

    if( rProperties.getLength() ) {
        nProps = rProperties.getLength();
        pProps = rProperties.getConstArray();
    } else {
        allProperties = getPropertySetInfo( xEnv )->getProperties();
        nProps = allProperties.getLength();
        pProps = allProperties.getConstArray();
    }

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( m_xSMgr );

        osl::Guard< osl::Mutex > aGuard( m_aMutex );
    for( sal_Int32 n = 0; n < nProps; ++n ) {
        const beans::Property& rProp = pProps[ n ];

        if ( rProp.Name == "Title" ) {
            if (m_info.name && m_info.name[0] == '/')
                g_warning ("Odd NFS title on item '%s' == '%s'",
                       getURI(), m_info.name);
            xRow->appendString( rProp, GnomeToOUString( m_info.name ) );
        }

        else if ( rProp.Name == "ContentType" )
            xRow->appendString( rProp, getContentType () );

        else if ( rProp.Name == "IsDocument" ) {
            if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
                xRow->appendBoolean( rProp, ( m_info.type == GNOME_VFS_FILE_TYPE_REGULAR ||
                                  m_info.type == GNOME_VFS_FILE_TYPE_UNKNOWN ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsFolder" ) {
            if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
                xRow->appendBoolean( rProp, ( m_info.type == GNOME_VFS_FILE_TYPE_DIRECTORY ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsReadOnly" ) {

            GnomeVFSFileInfo* fileInfo = gnome_vfs_file_info_new ();

            ::rtl::OString aURI = getOURI();
            gnome_vfs_get_file_info
                ( aURI.getStr(), fileInfo,
                        GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS );

            if (fileInfo->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_ACCESS) {
                bool read_only = true;

                if (fileInfo->permissions & GNOME_VFS_PERM_ACCESS_WRITABLE)
                                        read_only = false;

                xRow->appendBoolean( rProp, read_only );
            } else
                xRow->appendVoid( rProp );
            gnome_vfs_file_info_unref (fileInfo);
        }
        else if ( rProp.Name == "Size" ) {
            if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
                xRow->appendLong( rProp, m_info.size );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsHidden" )
            xRow->appendBoolean( rProp, ( m_info.name && m_info.name[0] == '.' ) );

        else if (rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsVolume" ) ) ||
             rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsCompactDisk" ) ) )
            xRow->appendBoolean( rProp, sal_False );

        else if ( rProp.Name == "DateCreated" ) {
            if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_CTIME)
                xRow->appendTimestamp( rProp, getDateFromUnix( m_info.ctime ) );
            else
                xRow->appendVoid( rProp );
        }

        else if ( rProp.Name == "DateModified" ) {
            if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
                xRow->appendTimestamp( rProp, getDateFromUnix( m_info.mtime ) );
            else
                xRow->appendVoid( rProp );
        }

        else if ( rProp.Name == "MediaType" ) {
            // We do this by sniffing in gnome-vfs; rather expensively.
#ifdef DEBUG
            g_warning ("FIXME: Requested mime-type - an expensive op. indeed!");
#endif
            xRow->appendVoid( rProp );
        } else if ( rProp.Name == "CreatableContentsInfo" )
            xRow->appendObject( rProp, uno::makeAny( queryCreatableContentsInfo( xEnv ) ) );

        else {
            xRow->appendVoid( rProp );
        }
    }
#if OSL_DEBUG_LEVEL > 1
    g_warning ("getPropertyValues on '%s' %d properties returned (of %d)",
           getURI(), 0, (int)nProps);
#endif

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

static lang::IllegalAccessException
getReadOnlyException( Content *ctnt )
{
    return lang::IllegalAccessException
        ( rtl::OUString("Property is read-only!"),
          static_cast< cppu::OWeakObject * >( ctnt ) );
}

rtl::OUString
Content::makeNewURL( const char */*newName*/ )
{
    rtl::OUString aNewURL = getParentURL();
    if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
        aNewURL += rtl::OUString("/");

    char *name = gnome_vfs_escape_string( m_info.name );
    aNewURL += GnomeToOUString( name );
    g_free( name );

    return aNewURL;
}

// This is slightly complicated by needing to support either 'move' or 'setname'
GnomeVFSResult
Content::doSetFileInfo( const GnomeVFSFileInfo *newInfo,
            GnomeVFSSetFileInfoMask setMask,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    GnomeVFSResult result = GNOME_VFS_OK;

    g_assert (!m_bTransient);

    ::rtl::OString aURI = getOURI();

        osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // The simple approach:
    if( setMask != GNOME_VFS_SET_FILE_INFO_NONE )
        result = gnome_vfs_set_file_info // missed a const in the API there
            ( aURI.getStr(), (GnomeVFSFileInfo *)newInfo, setMask );

    if ( result == GNOME_VFS_ERROR_NOT_SUPPORTED &&
         ( setMask & GNOME_VFS_SET_FILE_INFO_NAME ) ) {
        // Try a move instead
#ifdef DEBUG
        g_warning( "SetFileInfo not supported on '%s'", getURI() );
#endif

        char *newURI = OUStringToGnome( makeNewURL( newInfo->name ) );

        result = gnome_vfs_move (aURI.getStr(), newURI, FALSE);

        g_free (newURI);
    }

    return result;
}


uno::Sequence< uno::Any > Content::setPropertyValues(
                const uno::Sequence< beans::PropertyValue >& rValues,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    rtl::OUString    aNewTitle;
    GnomeVFSFileInfo newInfo;
    int              setMask = GNOME_VFS_SET_FILE_INFO_NONE;

    getInfo( xEnv );

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    gnome_vfs_file_info_copy( &newInfo, &m_info );

    Authentication aAuth( xEnv );

    int nChanged = 0, nTitlePos = 0;
    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
    aEvent.PropertyHandle = -1;
    // aEvent.PropertyName = fill in later ...
    // aEvent.OldValue     =
    // aEvent.NewValue     =

    int nCount = rValues.getLength();
    const beans::PropertyValue* pValues = rValues.getConstArray();

    for ( sal_Int32 n = 0; n < nCount; ++n ) {
        const beans::PropertyValue& rValue = pValues[ n ];

#if OSL_DEBUG_LEVEL > 1
        g_warning( "Set prop '%s'", OUStringToGnome( rValue.Name ) );
#endif
        if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) ||
             rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) ||
             rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) ||
             rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) ||
             rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Size" ) ) ||
             rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CreatableContentsInfo" ) ) )
            aRet[ n ] <<= getReadOnlyException( this );

        else if ( rValue.Name == "Title" ) {
            if ( rValue.Value >>= aNewTitle ) {
                if ( aNewTitle.isEmpty() )
                    aRet[ n ] <<= lang::IllegalArgumentException
                        ( rtl::OUString("Empty title not allowed!"),
                          static_cast< cppu::OWeakObject * >( this ), -1 );
                else {
                    char *newName = OUStringToGnome( aNewTitle );

                    if( !newName || !m_info.name || strcmp( newName, m_info.name ) ) {
#if OSL_DEBUG_LEVEL > 1
                        g_warning ("Set new name to '%s'", newName);
#endif

                        aEvent.PropertyName = rtl::OUString("Title");
                        aEvent.OldValue     = uno::makeAny( GnomeToOUString( newInfo.name ) );
                        aEvent.NewValue     = uno::makeAny( aNewTitle );
                        aChanges.getArray()[ nChanged ] = aEvent;
                        nTitlePos = nChanged++;

                        newInfo.name = newName;
                        setMask |= GNOME_VFS_SET_FILE_INFO_NAME;
                    } else // same name
                        g_free (newName);
                }
            } else
                aRet[ n ] <<= beans::IllegalTypeException
                    ( rtl::OUString("Property value has wrong type!"),
                      static_cast< cppu::OWeakObject * >( this ) );

        } else if ( rValue.Name == "DateCreated" || rValue.Name == "DateModified" ) {
            // FIXME: should be able to set the timestamps
            aRet[ n ] <<= getReadOnlyException( this );
        } else {
#ifdef DEBUG
            g_warning( "Unhandled property '%s'", OUStringToGnome( rValue.Name ) );
#endif
            aRet[ n ] <<= getReadOnlyException( this );
        }
    }

    GnomeVFSResult result = GNOME_VFS_OK;

    if ( !m_bTransient &&
         ( result = doSetFileInfo( &newInfo,
                       (GnomeVFSSetFileInfoMask) setMask,
                       xEnv ) ) != GNOME_VFS_OK ) {
        for (int i = 0; i < nChanged; i++)
            aRet[ i ] <<= mapVFSException( result, sal_True );

    }

    if ( result == GNOME_VFS_OK) {
        gnome_vfs_file_info_copy( &m_info, &newInfo );

        if ( setMask & GNOME_VFS_SET_FILE_INFO_NAME ) {
            uno::Reference< ucb::XContentIdentifier > xNewId
                = new ::ucbhelper::ContentIdentifier(
                    m_xSMgr, makeNewURL( newInfo.name ) );

            aGuard.clear();
            if (!exchangeIdentity( xNewId ) )
                aRet[ nTitlePos ] <<= uno::Exception
                    ( rtl::OUString("Exchange failed!"),
                      static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    gnome_vfs_file_info_clear( &newInfo );

    if ( nChanged > 0 ) {
            aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    rtl::OUString aURL = getOUURI();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
        aURL += rtl::OUString("/");

    sal_Int32 nLen = aURL.getLength();

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end ) {
        ::ucbhelper::ContentImplHelperRef xChild = (*it);
        rtl::OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) ) {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) ) {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back( ::gvfs::Content::ContentRef
                             (static_cast< ::gvfs::Content * >(xChild.get() ) ) );
            }
        }
        ++it;
    }
}

void Content::insert(
        const uno::Reference< io::XInputStream >               &xInputStream,
        sal_Bool                                                bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment > &xEnv )
        throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 1
    g_warning( "Insert '%s' (%d) (0x%x:%d)", getURI(), bReplaceExisting,
           m_info.valid_fields, m_info.type );
#endif

    GnomeVFSResult result = getInfo( xEnv );
    // a racy design indeed.
    if( !bReplaceExisting && !m_bTransient &&
        result != GNOME_VFS_ERROR_NOT_FOUND) {
#ifdef DEBUG
        g_warning ("Nasty error inserting to '%s' ('%s')",
               getURI(), gnome_vfs_result_to_string( result ));
#endif
        cancelCommandExecution( GNOME_VFS_ERROR_FILE_EXISTS, xEnv, sal_True );
    }

    if ( m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE &&
         m_info.type == GNOME_VFS_FILE_TYPE_DIRECTORY ) {
        ::rtl::OString aURI = getOURI();
        int perm;

        perm = ( GNOME_VFS_PERM_USER_ALL |
             GNOME_VFS_PERM_GROUP_READ |
             GNOME_VFS_PERM_OTHER_READ );

#if OSL_DEBUG_LEVEL > 1
        g_warning ("Make directory");
#endif
        result = gnome_vfs_make_directory( aURI.getStr(), perm );

        if( result != GNOME_VFS_OK )
            cancelCommandExecution( result, xEnv, sal_True );

        return;
    }

    if ( !xInputStream.is() ) {
        // FIXME: slightly unclear whether to accept this and create an empty file
        ucbhelper::cancelCommandExecution
            ( uno::makeAny
              ( ucb::MissingInputStreamException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ) ) ),
              xEnv );
    }

    GnomeVFSHandle *handle = NULL;
    ::rtl::OString aURI = getOURI();

    result = GNOME_VFS_OK;
    if ( bReplaceExisting ) {
        Authentication aAuth( xEnv );
        result = gnome_vfs_open( &handle, aURI.getStr(), GNOME_VFS_OPEN_WRITE );
    }

    if ( result != GNOME_VFS_OK ) {
        int perm;
        Authentication aAuth( xEnv );

        perm = ( ( GNOME_VFS_PERM_USER_WRITE | GNOME_VFS_PERM_USER_READ ) |
             ( GNOME_VFS_PERM_GROUP_WRITE | GNOME_VFS_PERM_GROUP_READ ) );

        result = gnome_vfs_create
            ( &handle, aURI.getStr(), GNOME_VFS_OPEN_WRITE, TRUE, perm );
    }

    if( result != GNOME_VFS_OK )
        cancelCommandExecution( result, xEnv, sal_True );

    if ( !xInputStream.is() ) {
        result = gnome_vfs_close( handle );
        if (result != GNOME_VFS_OK)
            cancelCommandExecution( result, xEnv, sal_True );

    } else { // copy it over
        uno::Reference < io::XOutputStream > xOutput =
            new gvfs::Stream( handle, &m_info );

        copyData( xInputStream, xOutput );
    }

    if (m_bTransient) {
        m_bTransient = sal_False;
        aGuard.clear();
        inserted();
    }
}

void Content::transfer(const ucb::TransferInfo & /*rArgs*/,
               const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    // FIXME: see gnome-vfs-xfer.h - but we need to be able to easily
    // detect which are gnome-vfs owned URI types ...
    ucbhelper::cancelCommandExecution
        ( uno::makeAny
            ( ucb::InteractiveBadTransferURLException
                ( rtl::OUString("Unsupported URL scheme!"),
                  static_cast< cppu::OWeakObject * >( this ) ) ),
          xEnv );
}

void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support
    rtl::OUString aURL = getOUURI();

    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instanciated children...
    ::gvfs::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end ) {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

// Used by the 'setPropertyValues' method for
// propagating the renaming of a Content.
sal_Bool Content::exchangeIdentity(
    const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    uno::Reference< ucb::XContent > xThis = this;

#if OSL_DEBUG_LEVEL > 1
    g_warning( "exchangeIdentity from '%s' to '%s'",
           getURI(), OUStringToGnome( xNewId->getContentIdentifier() ) );
#endif

    if ( m_bTransient ) {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        /* FIXME: can we not screw up an identically named
         * Content pointing to ourself here ? */
        m_xIdentifier = xNewId;
        return sal_False;
    }

    rtl::OUString aOldURL = getOUURI();

    // Exchange own identitity.
    if ( exchange( xNewId ) ) {

        // Process instanciated children...
        ContentRefList aChildren;
        queryChildren( aChildren );

        ContentRefList::const_iterator it  = aChildren.begin();
        ContentRefList::const_iterator end = aChildren.end();

        while ( it != end ) {
            ContentRef xChild = (*it);

            // Create new content identifier for the child...
            uno::Reference< ucb::XContentIdentifier >
                xOldChildId = xChild->getIdentifier();
            rtl::OUString aOldChildURL
                = xOldChildId->getContentIdentifier();
            rtl::OUString aNewChildURL
                = aOldChildURL.replaceAt(
                             0,
                             aOldURL.getLength(),
                             xNewId->getContentIdentifier() );
            uno::Reference< ucb::XContentIdentifier >
                xNewChildId
                = new ::ucbhelper::ContentIdentifier( m_xSMgr, aNewChildURL );

            if ( !xChild->exchangeIdentity( xNewChildId ) )
                return sal_False;

            ++it;
        }
        return sal_True;
    }

    return sal_False;
}

GnomeVFSResult
Content::getInfo( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    GnomeVFSResult result;
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (m_bTransient)
        result = GNOME_VFS_OK;

    else if ( !m_info.valid_fields ) {
        ::rtl::OString aURI = getOURI();
        Authentication aAuth( xEnv );
        result = gnome_vfs_get_file_info
            ( aURI.getStr(), &m_info, GNOME_VFS_FILE_INFO_DEFAULT );
        if (result != GNOME_VFS_OK)
            gnome_vfs_file_info_clear( &m_info );
    } else
        result = GNOME_VFS_OK;
#if OSL_DEBUG_LEVEL > 1
    g_warning( "getInfo on '%s' returns '%s' (%d) (0x%x)",
           getURI(), gnome_vfs_result_to_string( result ),
           result, m_info.valid_fields );
#endif
    return result;
}

sal_Bool
Content::isFolder(const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
    getInfo( xEnv );
    return (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE &&
        m_info.type == GNOME_VFS_FILE_TYPE_DIRECTORY);
}

uno::Any Content::mapVFSException( const GnomeVFSResult result, sal_Bool bWrite )
{
    uno::Any aException;
    const char *gvfs_message;
    rtl::OUString message;
    uno::Sequence< uno::Any > aArgs( 1 );

#if OSL_DEBUG_LEVEL > 1
    g_warning ("Map VFS exception '%s' (%d)",
           gnome_vfs_result_to_string( result ), result );
#endif

    if ((gvfs_message = gnome_vfs_result_to_string (result)))
        message = GnomeToOUString( gvfs_message );

    switch (result) {
    case GNOME_VFS_OK:
        g_warning("VFS_OK mapped to exception.");
        break;
    case GNOME_VFS_ERROR_EOF:
        g_warning ("VFS_EOF not handled somewhere.");
        break;
    case GNOME_VFS_ERROR_NOT_FOUND:
        aArgs[ 0 ] <<= m_xIdentifier->getContentIdentifier();
        aException <<=
            ucb::InteractiveAugmentedIOException
            ( rtl::OUString("Not found!"),
              static_cast< cppu::OWeakObject * >( this ),
              task::InteractionClassification_ERROR,
              ucb::IOErrorCode_NOT_EXISTING,
              aArgs );
        break;
    case GNOME_VFS_ERROR_BAD_PARAMETERS:
        aException <<=
            lang::IllegalArgumentException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ),
                  -1 );
        break;
    case GNOME_VFS_ERROR_GENERIC:
    case GNOME_VFS_ERROR_INTERNAL:
    case GNOME_VFS_ERROR_NOT_SUPPORTED:
#ifdef DEBUG
        g_warning ("Internal - un-mapped error");
#endif
        aException <<= io::IOException();
        break;
    case GNOME_VFS_ERROR_IO:
        if ( bWrite )
            aException <<=
                ucb::InteractiveNetworkWriteException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ),
                  task::InteractionClassification_ERROR,
                  message );
        else
            aException <<=
                ucb::InteractiveNetworkReadException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ),
                  task::InteractionClassification_ERROR,
                  message );
        break;
    case GNOME_VFS_ERROR_HOST_NOT_FOUND:
    case GNOME_VFS_ERROR_INVALID_HOST_NAME:
        aException <<=
            ucb::InteractiveNetworkResolveNameException
            ( rtl::OUString(),
              static_cast< cppu::OWeakObject * >( this ),
              task::InteractionClassification_ERROR,
              message );
        break;
    case GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE:
    case GNOME_VFS_ERROR_SERVICE_OBSOLETE:
    case GNOME_VFS_ERROR_PROTOCOL_ERROR:
    case GNOME_VFS_ERROR_NO_MASTER_BROWSER:
        aException <<=
            ucb::InteractiveNetworkConnectException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ),
                  task::InteractionClassification_ERROR,
                  message );
        break;

    case GNOME_VFS_ERROR_FILE_EXISTS:
        aException <<= ucb::NameClashException
                ( rtl::OUString(),
                  static_cast< cppu::OWeakObject * >( this ),
                  task::InteractionClassification_ERROR,
                  message );
        break;

    case GNOME_VFS_ERROR_INVALID_OPEN_MODE:
        aException <<= ucb::UnsupportedOpenModeException();
        break;

    case GNOME_VFS_ERROR_CORRUPTED_DATA:
    case GNOME_VFS_ERROR_WRONG_FORMAT:
    case GNOME_VFS_ERROR_BAD_FILE:
    case GNOME_VFS_ERROR_TOO_BIG:
    case GNOME_VFS_ERROR_NO_SPACE:
    case GNOME_VFS_ERROR_READ_ONLY:
    case GNOME_VFS_ERROR_INVALID_URI:
    case GNOME_VFS_ERROR_NOT_OPEN:
    case GNOME_VFS_ERROR_ACCESS_DENIED:
    case GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES:
    case GNOME_VFS_ERROR_NOT_A_DIRECTORY:
    case GNOME_VFS_ERROR_IN_PROGRESS:
    case GNOME_VFS_ERROR_INTERRUPTED:
    case GNOME_VFS_ERROR_LOOP:
    case GNOME_VFS_ERROR_NOT_PERMITTED:
    case GNOME_VFS_ERROR_IS_DIRECTORY:
    case GNOME_VFS_ERROR_NO_MEMORY:
    case GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS:
    case GNOME_VFS_ERROR_LOGIN_FAILED:
    case GNOME_VFS_ERROR_CANCELLED:
    case GNOME_VFS_ERROR_DIRECTORY_BUSY:
    case GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY:
    case GNOME_VFS_ERROR_TOO_MANY_LINKS:
    case GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM:
    case GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM:
    case GNOME_VFS_ERROR_NAME_TOO_LONG:
#ifdef DEBUG
        g_warning( "FIXME: Un-mapped VFS exception '%s' (%d)",
               gnome_vfs_result_to_string( result ), result );
#endif
    default:
        aException <<= ucb::InteractiveNetworkGeneralException
            ( rtl::OUString(),
              static_cast< cppu::OWeakObject * >( this ),
              task::InteractionClassification_ERROR );
        break;
    }

    return aException;
}

void Content::cancelCommandExecution(
    GnomeVFSResult result,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    sal_Bool bWrite /* = sal_False */ )
    throw ( uno::Exception )
{
    ucbhelper::cancelCommandExecution( mapVFSException( result, bWrite ), xEnv );
    // Unreachable
}

uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    static const beans::Property aGenericProperties[] = {
                beans::Property( rtl::OUString(  "ContentType"  ),
                 -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsDocument"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsFolder"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "Title"  ),
                 -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                 beans::PropertyAttribute::BOUND ),
        // Optional ...
        beans::Property( rtl::OUString(  "DateCreated"  ),
                 -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "DateModified"  ),
                 -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
// FIXME: Too expensive for now (?)
//                beans::Property( rtl::OUString(  "MediaType"  ),
//                 -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
//                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "Size"  ),
                 -1, getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsReadOnly"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsVolume"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsCompactDisk"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "IsHidden"  ),
                 -1, getCppuBooleanType(),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( rtl::OUString(  "CreatableContentsInfo"  ),
                 -1, getCppuType( static_cast< const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                 beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY )
    };

    const int nProps = sizeof (aGenericProperties) / sizeof (aGenericProperties[0]);

    return uno::Sequence< beans::Property > ( aGenericProperties, nProps );

}

uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    static ucb::CommandInfo aCommandInfoTable[] = {
        // Required commands
        ucb::CommandInfo
        ( rtl::OUString(  "getCommandInfo"  ),
          -1, getCppuVoidType() ),
        ucb::CommandInfo
        ( rtl::OUString(  "getPropertySetInfo"  ),
          -1, getCppuVoidType() ),
        ucb::CommandInfo
        ( rtl::OUString(  "getPropertyValues"  ),
          -1, getCppuType( static_cast<uno::Sequence< beans::Property > * >( 0 ) ) ),
        ucb::CommandInfo
        ( rtl::OUString(  "setPropertyValues"  ),
          -1, getCppuType( static_cast<uno::Sequence< beans::PropertyValue > * >( 0 ) ) ),

        // Optional standard commands
        ucb::CommandInfo
        ( rtl::OUString(  "delete"  ),
          -1, getCppuBooleanType() ),
        ucb::CommandInfo
        ( rtl::OUString(  "insert"  ),
          -1, getCppuType( static_cast<ucb::InsertCommandArgument * >( 0 ) ) ),
        ucb::CommandInfo
        ( rtl::OUString(  "open"  ),
          -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),

        // Folder Only, omitted if not a folder
        ucb::CommandInfo
        ( rtl::OUString(  "transfer"  ),
          -1, getCppuType( static_cast<ucb::TransferInfo * >( 0 ) ) ),
        ucb::CommandInfo
        ( rtl::OUString(  "createNewContent"  ),
          -1, getCppuType( static_cast<ucb::ContentInfo * >( 0 ) ) )
    };

    const int nProps
        = sizeof( aCommandInfoTable ) / sizeof( aCommandInfoTable[ 0 ] );
    return uno::Sequence< ucb::CommandInfo >(
        aCommandInfoTable, isFolder( xEnv ) ? nProps : nProps - 2 );
}

rtl::OUString
Content::getOUURI ()
{
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return m_xIdentifier->getContentIdentifier();
}

rtl::OString
Content::getOURI ()
{
    return rtl::OUStringToOString( getOUURI(), RTL_TEXTENCODING_UTF8 );
}

char *
Content::getURI ()
{
    return OUStringToGnome( getOUURI() );
}

void
Content::copyData( uno::Reference< io::XInputStream > xIn,
                   uno::Reference< io::XOutputStream > xOut )
{
    uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

    g_return_if_fail( xIn.is() && xOut.is() );

    while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
        xOut->writeBytes( theData );

    xOut->closeOutput();
}

// Inherits an authentication context
uno::Reference< io::XInputStream >
Content::createTempStream(
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
        throw( uno::Exception )
{
    GnomeVFSResult result;
    GnomeVFSHandle *handle = NULL;
    ::rtl::OString aURI = getOURI();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    // Something badly wrong happened - can't seek => stream to a temporary file
    uno::Reference < io::XOutputStream > xTempOut =
        uno::Reference < io::XOutputStream >
            ( io::TempFile::create(comphelper::getComponentContext(m_xSMgr)), uno::UNO_QUERY );

    if ( !xTempOut.is() )
        cancelCommandExecution( GNOME_VFS_ERROR_IO, xEnv );

    result = gnome_vfs_open
        ( &handle, aURI.getStr(), GNOME_VFS_OPEN_READ );
    if (result != GNOME_VFS_OK)
        cancelCommandExecution( result, xEnv );

    uno::Reference < io::XInputStream > pStream = new ::gvfs::Stream( handle, &m_info );
    copyData( pStream, xTempOut );

    return uno::Reference < io::XInputStream > ( xTempOut, uno::UNO_QUERY );
}

uno::Reference< io::XInputStream >
Content::createInputStream(
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
        throw( uno::Exception )
{
    GnomeVFSHandle *handle = NULL;
    GnomeVFSResult  result;
    uno::Reference<io::XInputStream > xIn;

    Authentication aAuth( xEnv );
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

    getInfo( xEnv );
    ::rtl::OString aURI = getOURI();

    if ( !(m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE) )
        return createTempStream( xEnv );

    result = gnome_vfs_open
        ( &handle, aURI.getStr(),
          (GnomeVFSOpenMode) (GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_RANDOM ) );

    if (result == GNOME_VFS_ERROR_INVALID_OPEN_MODE ||
        result == GNOME_VFS_ERROR_NOT_SUPPORTED)
        return createTempStream( xEnv );

    if (result != GNOME_VFS_OK)
        cancelCommandExecution( result, xEnv );

    // Try a seek just to make sure it's Random access: some lie.
    result = gnome_vfs_seek( handle, GNOME_VFS_SEEK_START, 0);
    if (result == GNOME_VFS_ERROR_NOT_SUPPORTED) {
        gnome_vfs_close( handle );
        return createTempStream( xEnv );
    }

    if (result != GNOME_VFS_OK)
        cancelCommandExecution( result, xEnv );

    if (handle != NULL)
        xIn = new ::gvfs::Stream( handle, &m_info );

    return xIn;
}

sal_Bool
Content::feedSink( uno::Reference< uno::XInterface > aSink,
                   const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    if ( !aSink.is() )
        return sal_False;

    uno::Reference< io::XOutputStream > xOut
        = uno::Reference< io::XOutputStream >(aSink, uno::UNO_QUERY );
    uno::Reference< io::XActiveDataSink > xDataSink
        = uno::Reference< io::XActiveDataSink >(aSink, uno::UNO_QUERY );

    if ( !xOut.is() && !xDataSink.is() )
        return sal_False;

    uno::Reference< io::XInputStream > xIn = createInputStream( xEnv );
    if ( !xIn.is() )
        return sal_False;

    if ( xOut.is() )
        copyData( xIn, xOut );

    if ( xDataSink.is() )
        xDataSink->setInputStream( xIn );

    return sal_True;
}

extern "C" {

#ifndef GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION
#  error "We require Gnome VFS 2.6.x to compile (will run fine with < 2.6)"
#endif

    static void
    vfs_authentication_callback (gconstpointer in_void,
                     gsize         in_size,
                     gpointer      out_void,
                     gsize         out_size,
                     gpointer      callback_data)
    {
        task::XInteractionHandler *xIH;

#if OSL_DEBUG_LEVEL > 1
        g_warning ("Full authentication callback (%p) ...", callback_data);
#endif

        if( !( xIH = (task::XInteractionHandler *) callback_data ) )
            return;

        const GnomeVFSModuleCallbackFullAuthenticationIn *in =
            (const GnomeVFSModuleCallbackFullAuthenticationIn *) in_void;
        GnomeVFSModuleCallbackFullAuthenticationOut *out =
            (GnomeVFSModuleCallbackFullAuthenticationOut *) out_void;

        g_return_if_fail (in != NULL && out != NULL);
        g_return_if_fail (sizeof (GnomeVFSModuleCallbackFullAuthenticationIn) == in_size &&
                  sizeof (GnomeVFSModuleCallbackFullAuthenticationOut) == out_size);

#if OSL_DEBUG_LEVEL > 1
#  define NNIL(x) (x?x:"<Null>")
        g_warning (" InComing data 0x%x uri '%s' prot '%s' server '%s' object '%s' "
                   "port %d auth_t '%s' user '%s' domain '%s' "
                   "def user '%s', def domain '%s'",
                   (int) in->flags, NNIL(in->uri), NNIL(in->protocol),
                   NNIL(in->server), NNIL(in->object),
                   (int) in->port, NNIL(in->authtype), NNIL(in->username), NNIL(in->domain),
                   NNIL(in->default_user), NNIL(in->default_domain));
#  undef NNIL
#endif

        ucbhelper::SimpleAuthenticationRequest::EntityType
                                   eDomain, eUserName, ePassword;
        ::rtl::OUString aHostName, aDomain, aUserName, aPassword;

        aHostName = GnomeToOUString( in->server );

        if (in->flags & GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_DOMAIN)
        {
            aDomain = GnomeToOUString( in->domain );
            eDomain = ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY;
            if (aDomain.isEmpty())
                aDomain = GnomeToOUString( in->default_domain );
        }
        else // no underlying capability to display realm otherwise
            eDomain = ucbhelper::SimpleAuthenticationRequest::ENTITY_NA;

        aUserName = GnomeToOUString( in->username );
        if (aUserName.isEmpty())
            aUserName = GnomeToOUString( in->default_user );
        eUserName = (in->flags & GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_USERNAME) ?
            ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY :
                (!aUserName.isEmpty() ?
                    ucbhelper::SimpleAuthenticationRequest::ENTITY_FIXED :
                    ucbhelper::SimpleAuthenticationRequest::ENTITY_NA);

        // No suggested password.
        ePassword = (in->flags & GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_PASSWORD) ?
            ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY :
            ucbhelper::SimpleAuthenticationRequest::ENTITY_FIXED;

        // Really, really bad things happen if we don't provide
        // the same user/password as was entered last time if
        // we failed to authenticate - infinite looping / flickering
        // madness etc. [ nice infrastructure ! ]
        static rtl::OUString aLastUserName, aLastPassword;
        if (in->flags & GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_PREVIOUS_ATTEMPT_FAILED)
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
            aUserName = aLastUserName;
            aPassword = aLastPassword;
        }

        rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
            = new ucbhelper::SimpleAuthenticationRequest (GnomeToOUString(in->uri),
                                                          aHostName, eDomain, aDomain,
                                                          eUserName, aUserName,
                                                          ePassword, aPassword);

        xIH->handle( xRequest.get() );

        rtl::Reference< ucbhelper::InteractionContinuation > xSelection
            = xRequest->getSelection();

        if ( xSelection.is() ) {
            // Handler handled the request.
            uno::Reference< task::XInteractionAbort > xAbort(xSelection.get(), uno::UNO_QUERY );
            if ( !xAbort.is() ) {
                const rtl::Reference<
                    ucbhelper::InteractionSupplyAuthentication > & xSupp
                    = xRequest->getAuthenticationSupplier();

                ::rtl::OUString aNewDomain, aNewUserName, aNewPassword;

                aNewUserName = xSupp->getUserName();
                if ( !aNewUserName.isEmpty() )
                    aUserName = aNewUserName;
                aNewDomain = xSupp->getRealm();
                if ( !aNewDomain.isEmpty() )
                    aDomain = aNewDomain;
                aNewPassword = xSupp->getPassword();
                if ( !aNewPassword.isEmpty() )
                    aPassword = aNewPassword;

                {
                    osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
                    aLastUserName = aUserName;
                    aLastPassword = aPassword;
                }

                out->username = OUStringToGnome( aUserName );
                out->domain   = OUStringToGnome( aDomain );
                out->password = OUStringToGnome( aPassword );
                out->save_password = xSupp->getRememberPasswordMode();

#if OSL_DEBUG_LEVEL > 1
                g_warning ("Got valid user/domain/password '%s' '%s' '%s', %s password",
                           out->username, out->domain, out->password,
                           out->save_password ? "save" : "don't save");
#endif
            }
            else
                out->abort_auth = TRUE;
        }
        else
            out->abort_auth = TRUE;
    }

    static void
    vfs_authentication_old_callback (gconstpointer in_void,
                                     gsize         in_size,
                                     gpointer      out_void,
                                     gsize         out_size,
                                     gpointer      callback_data)
    {
#ifdef DEBUG
        g_warning ("Old authentication callback (%p) [ UNTESTED ] ...", callback_data);
#endif
        const GnomeVFSModuleCallbackAuthenticationIn *in =
            (const GnomeVFSModuleCallbackAuthenticationIn *) in_void;
        GnomeVFSModuleCallbackAuthenticationOut *out =
            (GnomeVFSModuleCallbackAuthenticationOut *) out_void;

        g_return_if_fail (in != NULL && out != NULL);
        g_return_if_fail (sizeof (GnomeVFSModuleCallbackAuthenticationIn) == in_size &&
                  sizeof (GnomeVFSModuleCallbackAuthenticationOut) == out_size);

        GnomeVFSModuleCallbackFullAuthenticationIn mapped_in = {
                (GnomeVFSModuleCallbackFullAuthenticationFlags)
                (GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_PASSWORD |
                 GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_USERNAME |
                 GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_DOMAIN),
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        GnomeVFSModuleCallbackFullAuthenticationOut mapped_out = { 0, 0, 0, 0, 0, 0, 0, 0 };

        // Map the old style input auth. data to the new style structure.
        if (in->previous_attempt_failed)
            mapped_in.flags = (GnomeVFSModuleCallbackFullAuthenticationFlags)
                (mapped_in.flags |
                 GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_PREVIOUS_ATTEMPT_FAILED);

        GnomeVFSURI *pURI = NULL;
        // Urk - parse all this from the URL ...
        mapped_in.uri = in->uri;
        if (in->uri)
        {
            pURI = gnome_vfs_uri_new( in->uri );
            mapped_in.protocol = (char *) gnome_vfs_uri_get_scheme (pURI);
            mapped_in.server   = (char *) gnome_vfs_uri_get_host_name (pURI);
            mapped_in.port     = gnome_vfs_uri_get_host_port (pURI);
            mapped_in.username = (char *) gnome_vfs_uri_get_user_name (pURI);
        }
        mapped_in.domain = in->realm;
        mapped_in.default_user = mapped_in.username;
        mapped_in.default_domain = mapped_in.domain;

        vfs_authentication_callback ((gconstpointer) &mapped_in,
                                     sizeof (mapped_in),
                                     (gpointer) &mapped_out,
                                     sizeof (mapped_out),
                                     callback_data);

        if (pURI)
            gnome_vfs_uri_unref (pURI);

        // Map the new style auth. out data to the old style out structure.
        out->username = mapped_out.username;
        out->password = mapped_out.password;
        g_free (mapped_out.domain);
        g_free (mapped_out.keyring);
    }


    static void
    auth_destroy (gpointer data)
    {
        task::XInteractionHandler *xIH;
        if( ( xIH = ( task::XInteractionHandler * )data ) )
            xIH->release();
    }

    // This sucks, but gnome-vfs doesn't much like
    // repeated set / unsets - so we have to compensate.
    GPrivate *auth_queue = NULL;

    void auth_queue_destroy( gpointer data )
    {
        GList  *l;
        GQueue *vq = (GQueue *) data;

        for (l = vq->head; l; l = l->next)
            auth_destroy (l->data);
        g_queue_free (vq);
    }
}

static void
refresh_auth( GQueue *vq )
{
    GList *l;

    gnome_vfs_module_callback_pop( GNOME_VFS_MODULE_CALLBACK_AUTHENTICATION );
    gnome_vfs_module_callback_pop( GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION );

    for (l = vq->head; l; l = l->next) {
        if (l->data) {
            gnome_vfs_module_callback_push
                ( GNOME_VFS_MODULE_CALLBACK_AUTHENTICATION,
                  vfs_authentication_old_callback, l->data, NULL );
            gnome_vfs_module_callback_push
                ( GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION,
                  vfs_authentication_callback, l->data, NULL );
            break;
        }
    }
}

gvfs::Authentication::Authentication(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    GQueue *vq;
    uno::Reference< task::XInteractionHandler > xIH;

    if ( xEnv.is() )
        xIH = xEnv->getInteractionHandler();

    if ( xIH.is() )
        xIH->acquire();

    if( !(vq = (GQueue *)g_private_get( auth_queue ) ) ) {
        vq = g_queue_new();
        g_private_set( auth_queue, vq );
    }

    g_queue_push_head( vq, (gpointer) xIH.get() );
    refresh_auth( vq );
}

gvfs::Authentication::~Authentication()
{
    GQueue *vq;
    gpointer data;

    vq = (GQueue *)g_private_get( auth_queue );

    data = g_queue_pop_head( vq );
    auth_destroy (data);

    refresh_auth( vq );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
