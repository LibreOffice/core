/*************************************************************************
 *
 *  $RCSfile: pkgcontent.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:14:53 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************
 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include "osl/doublecheckedlocking.h"

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALACCESSEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalAccessException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_MISSINGINPUTSTREAMEXCEPTION_HPP_
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDNAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDOPENMODEEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif

#ifndef _PKGCONTENT_HXX
#include "pkgcontent.hxx"
#endif
#ifndef _PKGPROVIDER_HXX
#include "pkgprovider.hxx"
#endif
#ifndef _PKGRESULTSET_HXX
#include "pkgresultset.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;
using namespace package_ucp;

#define NONE_MODIFIED           sal_uInt32( 0x00 )
#define MEDIATYPE_MODIFIED      sal_uInt32( 0x01 )
#define COMPRESSED_MODIFIED     sal_uInt32( 0x02 )
#define ENCRYPTED_MODIFIED      sal_uInt32( 0x04 )
#define ENCRYPTIONKEY_MODIFIED  sal_uInt32( 0x08 )

//=========================================================================
//=========================================================================
//
// ContentProperties Implementation.
//
//=========================================================================
//=========================================================================

ContentProperties::ContentProperties( const rtl::OUString& rContentType )
: aContentType( rContentType ),
  nSize( 0 ),
  bCompressed( sal_True ),
  bEncrypted( sal_False ),
  bHasEncryptedEntries( sal_False )
{
    bIsFolder = rContentType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PACKAGE_FOLDER_CONTENT_TYPE ) )
                || rContentType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PACKAGE_ZIP_FOLDER_CONTENT_TYPE ) );
    bIsDocument = !bIsFolder;

    OSL_ENSURE( bIsFolder ||
                rContentType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PACKAGE_STREAM_CONTENT_TYPE ) ) ||
                rContentType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PACKAGE_ZIP_STREAM_CONTENT_TYPE ) ),
                "ContentProperties::ContentProperties - Unknown type!" );
}

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

// static ( "virtual" ctor )
Content* Content::create(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const uno::Reference< star::ucb::XContentIdentifier >& Identifier )
{
    rtl::OUString aURL = Identifier->getContentIdentifier();
    PackageUri aURI( aURL );
    ContentProperties aProps;
    uno::Reference< container::XHierarchicalNameAccess > xPackage;

    if ( loadData( pProvider, aURI, aProps, xPackage ) )
    {
        // resource exists

        sal_Int32 nLastSlash = aURL.lastIndexOf( '/' );
        if ( ( nLastSlash + 1 ) == aURL.getLength() )
        {
            // Client explicitely requested a folder!
            if ( !aProps.bIsFolder )
                return 0;
        }

        uno::Reference< star::ucb::XContentIdentifier > xId
            = new ::ucb::ContentIdentifier( rxSMgr, aURI.getUri() );
        return new Content( rxSMgr, pProvider, xId, xPackage, aURI, aProps );
    }
    else
    {
        // resource doesn't exist

        sal_Bool bFolder = sal_False;

        // Guess type according to URI.
        sal_Int32 nLastSlash = aURL.lastIndexOf( '/' );
        if ( ( nLastSlash + 1 ) == aURL.getLength() )
            bFolder = sal_True;

        uno::Reference< star::ucb::XContentIdentifier > xId
            = new ::ucb::ContentIdentifier( rxSMgr, aURI.getUri() );

        star::ucb::ContentInfo aInfo;
        if ( bFolder || aURI.isRootFolder() )
            aInfo.Type = GetContentType( aURI.getScheme(), sal_True );
        else
            aInfo.Type = GetContentType( aURI.getScheme(), sal_False );

        return new Content( rxSMgr, pProvider, xId, xPackage, aURI, aInfo );
    }
}

//=========================================================================
// static ( "virtual" ctor )
Content* Content::create(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const uno::Reference< star::ucb::XContentIdentifier >& Identifier,
            const star::ucb::ContentInfo& Info )
{
    if ( !Info.Type.getLength() )
        return 0;

    PackageUri aURI( Identifier->getContentIdentifier() );

    if ( !Info.Type.equalsIgnoreAsciiCase(
                GetContentType( aURI.getScheme(), sal_True ) ) &&
         !Info.Type.equalsIgnoreAsciiCase(
                GetContentType( aURI.getScheme(), sal_False ) ) )
        return 0;

    uno::Reference< container::XHierarchicalNameAccess > xPackage;

#if 0
    // Fail, if content does exist.
    if ( hasData( pProvider, aURI, xPackage ) )
        return 0;
#else
    xPackage = pProvider->createPackage( aURI.getPackage(), aURI.getParam() );
#endif

    uno::Reference< star::ucb::XContentIdentifier > xId
        = new ::ucb::ContentIdentifier( rxSMgr, aURI.getUri() );
    return new Content( rxSMgr, pProvider, xId, xPackage, aURI, Info );
}

//=========================================================================
// static
::rtl::OUString Content::GetContentType( const ::rtl::OUString& aScheme, sal_Bool bFolder )
{
    return
        ( rtl::OUString::createFromAscii( "application/" )
        + aScheme
        + ( bFolder ? rtl::OUString::createFromAscii( "-folder" ) : rtl::OUString::createFromAscii( "-stream" ) ) );
}

//=========================================================================
Content::Content(
        const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        ContentProvider* pProvider,
        const uno::Reference< star::ucb::XContentIdentifier >& Identifier,
        const uno::Reference< container::XHierarchicalNameAccess > & Package,
        const PackageUri& rUri,
        const ContentProperties& rProps )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_xPackage( Package ),
  m_aUri( rUri ),
  m_aProps( rProps ),
  m_eState( PERSISTENT ),
  m_pProvider( pProvider ),
  m_nModifiedProps( NONE_MODIFIED )
{
}

//=========================================================================
Content::Content(
        const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        ContentProvider* pProvider,
        const uno::Reference< star::ucb::XContentIdentifier >& Identifier,
        const uno::Reference< container::XHierarchicalNameAccess > & Package,
        const PackageUri& rUri,
        const star::ucb::ContentInfo& Info )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  m_xPackage( Package ),
  m_aUri( rUri ),
  m_aProps( Info.Type ),
  m_eState( TRANSIENT ),
  m_pProvider( pProvider ),
  m_nModifiedProps( NONE_MODIFIED )
{
}

//=========================================================================
// virtual
Content::~Content()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire()
    throw( )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;

    if ( isFolder() )
        aRet = cppu::queryInterface(
                rType, static_cast< star::ucb::XContentCreator * >( this ) );

     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    cppu::OTypeCollection * pCollection = 0;

    if ( isFolder() )
    {
        static cppu::OTypeCollection* pFolderTypes = 0;

        pCollection = pFolderTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pFolderTypes;
            if ( !pCollection )
            {
                static cppu::OTypeCollection aCollection(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( star::ucb::XContent ),
                    CPPU_TYPE_REF( star::ucb::XCommandProcessor ),
                    CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                    CPPU_TYPE_REF( star::ucb::XCommandInfoChangeNotifier ),
                    CPPU_TYPE_REF( beans::XPropertyContainer ),
                    CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                    CPPU_TYPE_REF( container::XChild ),
                    CPPU_TYPE_REF( star::ucb::XContentCreator ) ); // !!
                pCollection = &aCollection;
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pFolderTypes = pCollection;
            }
        }
        else
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    else
    {
        static cppu::OTypeCollection* pDocumentTypes = 0;

        pCollection = pDocumentTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pDocumentTypes;
            if ( !pCollection )
            {
                static cppu::OTypeCollection aCollection(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( star::ucb::XContent ),
                    CPPU_TYPE_REF( star::ucb::XCommandProcessor ),
                    CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                    CPPU_TYPE_REF( star::ucb::XCommandInfoChangeNotifier ),
                    CPPU_TYPE_REF( beans::XPropertyContainer ),
                    CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                    CPPU_TYPE_REF( container::XChild ) );
                pCollection = &aCollection;
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pDocumentTypes = pCollection;
            }
        }
        else
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    return rtl::OUString::createFromAscii(
                            "com.sun.star.comp.ucb.PackageContent" );
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    if ( isFolder() )
        aSNS.getArray()[ 0 ]
            = rtl::OUString::createFromAscii(
                PACKAGE_FOLDER_CONTENT_SERVICE_NAME );
    else
        aSNS.getArray()[ 0 ]
            = rtl::OUString::createFromAscii(
                PACKAGE_STREAM_CONTENT_SERVICE_NAME );

    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
    return m_aProps.aContentType;
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const star::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const uno::Reference< star::ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           star::ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "No properties!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // open
        //////////////////////////////////////////////////////////////////

        star::ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet = open( aOpenCommand, Environment );
    }
    else if ( !m_aUri.isRootFolder()
              && aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        star::ucb::InsertCommandArgument aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Int32 nNameClash = aArg.ReplaceExisting
                             ? star::ucb::NameClash::OVERWRITE
                             : star::ucb::NameClash::ERROR;
        insert( aArg.Data, nNameClash, Environment );
    }
    else if ( !m_aUri.isRootFolder()
              && aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical, Environment );

        // Remove own and all children's persistent data.
        if ( !removeData() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny(m_xIdentifier->
                                              getContentIdentifier()),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                Environment,
                rtl::OUString::createFromAscii(
                    "Cannot remove persistent data!" ),
                this );
            // Unreachable
        }

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "transfer" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // transfer
        //  ( Not available at stream objects )
        //////////////////////////////////////////////////////////////////

        star::ucb::TransferInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        transfer( aInfo, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "flush" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // flush
        //  ( Not available at stream objects )
        //////////////////////////////////////////////////////////////////

        if( !flushData() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny(m_xIdentifier->
                                              getContentIdentifier()),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                Environment,
                rtl::OUString::createFromAscii(
                    "Cannot write file to disk!" ),
                this );
            // Unreachable
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::UnsupportedCommandException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( uno::RuntimeException )
{
    // @@@ Implement logic to abort running commands, if this makes
    //     sense for your content.
}

//=========================================================================
//
// XContentCreator methods.
//
//=========================================================================

// virtual
uno::Sequence< star::ucb::ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
    throw( uno::RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        uno::Sequence< beans::Property > aProps( 1 );
        aProps.getArray()[ 0 ] = beans::Property(
                    rtl::OUString::createFromAscii( "Title" ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND );

        uno::Sequence< star::ucb::ContentInfo > aSeq( 2 );

        // Folder.
        aSeq.getArray()[ 0 ].Type
            = GetContentType( m_aUri.getScheme(), sal_True );
        aSeq.getArray()[ 0 ].Attributes
            = star::ucb::ContentInfoAttribute::KIND_FOLDER;
        aSeq.getArray()[ 0 ].Properties = aProps;

        // Stream.
        aSeq.getArray()[ 1 ].Type
            = GetContentType( m_aUri.getScheme(), sal_False );
        aSeq.getArray()[ 1 ].Attributes
            = star::ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
              | star::ucb::ContentInfoAttribute::KIND_DOCUMENT;
        aSeq.getArray()[ 1 ].Properties = aProps;

        return aSeq;
    }
    else
    {
        OSL_ENSURE( sal_False,
                    "queryCreatableContentsInfo called on non-folder object!" );

        return uno::Sequence< star::ucb::ContentInfo >( 0 );
    }
}

//=========================================================================
// virtual
uno::Reference< star::ucb::XContent > SAL_CALL
Content::createNewContent( const star::ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !Info.Type.getLength() )
            return uno::Reference< star::ucb::XContent >();

        if ( !Info.Type.equalsIgnoreAsciiCase(
                GetContentType( m_aUri.getScheme(), sal_True ) ) &&
             !Info.Type.equalsIgnoreAsciiCase(
                GetContentType( m_aUri.getScheme(), sal_False ) ) )
            return uno::Reference< star::ucb::XContent >();

        rtl::OUString aURL = m_aUri.getUri();
        aURL += rtl::OUString::createFromAscii( "/" );

        if ( Info.Type.equalsIgnoreAsciiCase(
                GetContentType( m_aUri.getScheme(), sal_True ) ) )
            aURL += rtl::OUString::createFromAscii( "New_Folder" );
        else
            aURL += rtl::OUString::createFromAscii( "New_Stream" );

        uno::Reference< star::ucb::XContentIdentifier > xId(
                        new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );

        return create( m_xSMgr, m_pProvider, xId, Info );
    }
    else
    {
        OSL_ENSURE( sal_False,
                    "createNewContent called on non-folder object!" );
        return uno::Reference< star::ucb::XContent >();
    }
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// virtual
rtl::OUString Content::getParentURL()
{
    return m_aUri.getParentUri();
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
                const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
                const uno::Sequence< beans::Property >& rProperties,
                ContentProvider* pProvider,
                const rtl::OUString& rContentId )
{
    ContentProperties aData;
    uno::Reference< container::XHierarchicalNameAccess > xPackage;
    if ( loadData( pProvider, PackageUri( rContentId ), aData, xPackage ) )
    {
        return getPropertyValues( rSMgr,
                                  rProperties,
                                  aData,
                                  rtl::Reference<
                                    ::ucb::ContentProviderImplHelper >(
                                        pProvider ),
                                  rContentId );
    }
    else
    {
        rtl::Reference< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

        sal_Int32 nCount = rProperties.getLength();
        if ( nCount )
        {
            const beans::Property* pProps = rProperties.getConstArray();
            for ( sal_Int32 n = 0; n < nCount; ++n )
                xRow->appendVoid( pProps[ n ] );
        }

        return uno::Reference< sdbc::XRow >( xRow.get() );
    }
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
        const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
        const uno::Sequence< beans::Property >& rProperties,
        const ContentProperties& rData,
        const rtl::Reference< ::ucb::ContentProviderImplHelper >& rProvider,
        const rtl::OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
                xRow->appendString ( rProp, rData.aContentType );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                xRow->appendString ( rProp, rData.aTitle );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsDocument );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsFolder );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
            {
                xRow->appendString ( rProp, rData.aMediaType );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
            {
                // Property only available for streams.
                if ( rData.bIsDocument )
                    xRow->appendLong( rProp, rData.nSize );
                else
                    xRow->appendVoid( rProp );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Compressed" ) ) )
            {
                // Property only available for streams.
                if ( rData.bIsDocument )
                    xRow->appendBoolean( rProp, rData.bCompressed );
                else
                    xRow->appendVoid( rProp );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Encrypted" ) ) )
            {
                // Property only available for streams.
                if ( rData.bIsDocument )
                    xRow->appendBoolean( rProp, rData.bEncrypted );
                else
                    xRow->appendVoid( rProp );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "HasEncryptedEntries" ) ) )
            {
                // Property only available for root folder.
                PackageUri aURI( rContentId );
                if ( aURI.isRootFolder() )
                    xRow->appendBoolean( rProp, rData.bHasEncryptedEntries );
                else
                    xRow->appendVoid( rProp );
            }
            else
            {
                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        // Append empty entry.
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            beans::Property(
                rtl::OUString::createFromAscii( "ContentType" ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY ),
            rData.aContentType );
        xRow->appendString(
            beans::Property(
                rtl::OUString::createFromAscii( "Title" ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            rData.aTitle );
        xRow->appendBoolean(
            beans::Property(
                rtl::OUString::createFromAscii( "IsDocument" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY ),
            rData.bIsDocument );
        xRow->appendBoolean(
            beans::Property(
                rtl::OUString::createFromAscii( "IsFolder" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY ),
            rData.bIsFolder );
        xRow->appendString(
            beans::Property(
                rtl::OUString::createFromAscii( "MediaType" ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            rData.aMediaType );

        // Properties only available for streams.
        if ( rData.bIsDocument )
        {
            xRow->appendLong(
                beans::Property(
                    rtl::OUString::createFromAscii( "Size" ),
                    -1,
                    getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
                rData.nSize );

            xRow->appendBoolean(
                beans::Property(
                    rtl::OUString::createFromAscii( "Compressed" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND ),
                rData.bCompressed );

            xRow->appendBoolean(
                beans::Property(
                    rtl::OUString::createFromAscii( "Encrypted" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND ),
                rData.bEncrypted );
        }

        // Properties only available for root folder.
        PackageUri aURI( rContentId );
        if ( aURI.isRootFolder() )
        {
            xRow->appendBoolean(
                beans::Property(
                    rtl::OUString::createFromAscii( "HasEncryptedEntries" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
                rData.bHasEncryptedEntries );
        }

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
                        const uno::Sequence< beans::Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              rtl::Reference<
                                ::ucb::ContentProviderImplHelper >(
                                    m_xProvider.getBodyPtr() ),
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
        const uno::Sequence< beans::PropertyValue >& rValues,
        const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw( ::com::sun::star::uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< star::ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;
    sal_Bool bExchange = sal_False;
    sal_Bool bStore    = sal_False;
    rtl::OUString aNewTitle;
    sal_Int32 nTitlePos = -1;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            if ( m_aUri.isRootFolder() )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                rtl::OUString aNewValue;
                if ( rValue.Value >>= aNewValue )
                {
                    // No empty titles!
                    if ( aNewValue.getLength() > 0 )
                    {
                        if ( aNewValue != m_aProps.aTitle )
                        {
                            // modified title -> modified URL -> exchange !
                            if ( m_eState == PERSISTENT )
                                bExchange = sal_True;

                            // new value will be set later...
                            aNewTitle = aNewValue;

                            // remember position within sequence of values
                            // (for error handling).
                            nTitlePos = n;
                        }
                    }
                    else
                    {
                        aRet[ n ] <<=
                            lang::IllegalArgumentException(
                                rtl::OUString::createFromAscii(
                                    "Empty title not allowed!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<=
                        beans::IllegalTypeException(
                            rtl::OUString::createFromAscii(
                                "Property value has wrong type!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
                }
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        {
            rtl::OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_aProps.aMediaType )
                {
                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = uno::makeAny( m_aProps.aMediaType );
                    aEvent.NewValue     = uno::makeAny( aNewValue );

                    m_aProps.aMediaType = aNewValue;
                      nChanged++;
                    bStore = sal_True;
                    m_nModifiedProps |= MEDIATYPE_MODIFIED;
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                    "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Compressed" ) ) )
        {
            // Property only available for streams.
            if ( m_aProps.bIsDocument )
            {
                sal_Bool bNewValue;
                if ( rValue.Value >>= bNewValue )
                {
                    if ( bNewValue != m_aProps.bCompressed )
                    {
                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue = uno::makeAny( m_aProps.bCompressed );
                        aEvent.NewValue = uno::makeAny( bNewValue );

                        m_aProps.bCompressed = bNewValue;
                          nChanged++;
                        bStore = sal_True;
                        m_nModifiedProps |= COMPRESSED_MODIFIED;
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                        "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
            else
            {
                aRet[ n ] <<= beans::UnknownPropertyException(
                                rtl::OUString::createFromAscii(
                                    "Compressed only supported by streams!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Encrypted" ) ) )
        {
            // Property only available for streams.
            if ( m_aProps.bIsDocument )
            {
                sal_Bool bNewValue;
                if ( rValue.Value >>= bNewValue )
                {
                    if ( bNewValue != m_aProps.bEncrypted )
                    {
                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue = uno::makeAny( m_aProps.bEncrypted );
                        aEvent.NewValue = uno::makeAny( bNewValue );

                        m_aProps.bEncrypted = bNewValue;
                          nChanged++;
                        bStore = sal_True;
                        m_nModifiedProps |= ENCRYPTED_MODIFIED;
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                        "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
            else
            {
                aRet[ n ] <<= beans::UnknownPropertyException(
                                rtl::OUString::createFromAscii(
                                    "Encrypted only supported by streams!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "HasEncryptedEntries" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "EncryptionKey" ) ) )
        {
            // @@@ This is a temporary solution. In the future submitting
            //     the key should be done using an interaction handler!

            // Write-Only property. Only supported by root folder and streams
            // (all non-root folders of a package have the same encryption key).
            if ( m_aUri.isRootFolder() || m_aProps.bIsDocument )
            {
                uno::Sequence < sal_Int8 > aNewValue;
                if ( rValue.Value >>= aNewValue )
                {
                    if ( aNewValue != m_aProps.aEncryptionKey )
                    {
                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = uno::makeAny(
                                                m_aProps.aEncryptionKey );
                        aEvent.NewValue     = uno::makeAny( aNewValue );

                        m_aProps.aEncryptionKey = aNewValue;
                        nChanged++;
                        bStore = sal_True;
                        m_nModifiedProps |= ENCRYPTIONKEY_MODIFIED;
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                    "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
            else
            {
                aRet[ n ] <<= beans::UnknownPropertyException(
                        rtl::OUString::createFromAscii(
                            "EncryptionKey not supported by non-root folder!" ),
                        static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else
        {
            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    uno::Any aOldValue
                        = xAdditionalPropSet->getPropertyValue( rValue.Name );
                    if ( aOldValue != rValue.Value )
                    {
                        xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                }
                catch ( beans::UnknownPropertyException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRet[ n ] <<= e;
                }
            }
            else
            {
                aRet[ n ] <<= uno::Exception(
                                rtl::OUString::createFromAscii(
                                    "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
    }

    if ( bExchange )
    {
        uno::Reference< star::ucb::XContentIdentifier > xOldId = m_xIdentifier;

        // Assemble new content identifier...
        rtl::OUString aNewURL = m_aUri.getParentUri();
        aNewURL += rtl::OUString::createFromAscii( "/" );
        aNewURL += PackageUri::encodeSegment( aNewTitle );
        uno::Reference< star::ucb::XContentIdentifier > xNewId
            = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );

        aGuard.clear();
        if ( exchangeIdentity( xNewId ) )
        {
            // Adapt persistent data.
            renameData( xOldId, xNewId );

            // Adapt Additional Core Properties.
            renameAdditionalPropertySet( xOldId->getContentIdentifier(),
                                         xNewId->getContentIdentifier(),
                                         sal_True );
        }
        else
        {
            // Do not set new title!
            aNewTitle = rtl::OUString();

            // Set error .
            aRet[ nTitlePos ] <<= uno::Exception(
                    rtl::OUString::createFromAscii( "Exchange failed!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    if ( aNewTitle.getLength() )
    {
        aEvent.PropertyName = rtl::OUString::createFromAscii( "Title" );
        aEvent.OldValue     = uno::makeAny( m_aProps.aTitle );
        aEvent.NewValue     = uno::makeAny( aNewTitle );

        m_aProps.aTitle = aNewTitle;

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        // Save changes, if content was already made persistent.
        if ( ( m_nModifiedProps & ENCRYPTIONKEY_MODIFIED ) ||
             ( bStore && ( m_eState == PERSISTENT ) ) )
        {
            if ( !storeData( uno::Reference< io::XInputStream >() ) )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Uri")),
                                 -1,
                                 uno::makeAny(m_xIdentifier->
                                                  getContentIdentifier()),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    star::ucb::IOErrorCode_CANT_WRITE,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    xEnv,
                    rtl::OUString::createFromAscii(
                        "Cannot store persistent data!" ),
                    this );
                // Unreachable
            }
        }

        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

//=========================================================================
uno::Any Content::open(
                const star::ucb::OpenCommandArgument2& rArg,
                const uno::Reference< star::ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    if ( rArg.Mode == star::ucb::OpenMode::ALL ||
         rArg.Mode == star::ucb::OpenMode::FOLDERS ||
         rArg.Mode == star::ucb::OpenMode::DOCUMENTS )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////

        uno::Reference< star::ucb::XDynamicResultSet > xSet
            = new DynamicResultSet( m_xSMgr, this, rArg, xEnv );
        return uno::makeAny( xSet );
      }
    else
    {
        //////////////////////////////////////////////////////////////////
        // open command for a document content
        //////////////////////////////////////////////////////////////////

        if ( ( rArg.Mode == star::ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
             ( rArg.Mode == star::ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
        {
            // Currently(?) unsupported.
            ucbhelper::cancelCommandExecution(
                uno::makeAny( star::ucb::UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( rArg.Mode ) ) ),
                xEnv );
            // Unreachable
        }

        rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
        uno::Reference< io::XOutputStream > xOut( rArg.Sink, uno::UNO_QUERY );
        if ( xOut.is() )
          {
            // PUSH: write data into xOut

            uno::Reference< io::XInputStream > xIn = getInputStream();
            if ( !xIn.is() )
            {
                // No interaction if we are not persistent!
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Uri")),
                                 -1,
                                 uno::makeAny(m_xIdentifier->
                                                  getContentIdentifier()),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    star::ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    m_eState == PERSISTENT
                        ? xEnv
                        : uno::Reference< star::ucb::XCommandEnvironment >(),
                    rtl::OUString::createFromAscii( "Got no data stream!" ),
                    this );
                // Unreachable
            }

            try
            {
                uno::Sequence< sal_Int8 > aBuffer;
                sal_Int32  nRead = xIn->readSomeBytes( aBuffer, 65536 );

                while ( nRead > 0 )
                {
                    aBuffer.realloc( nRead );
                    xOut->writeBytes( aBuffer );
                    aBuffer.realloc( 0 );
                    nRead = xIn->readSomeBytes( aBuffer, 65536 );
                }

                xOut->closeOutput();
            }
            catch ( io::NotConnectedException const & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
            catch ( io::BufferSizeExceededException const & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
            catch ( io::IOException const & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
          }
        else
          {
            uno::Reference< io::XActiveDataSink > xDataSink(
                                            rArg.Sink, uno::UNO_QUERY );
              if ( xDataSink.is() )
            {
                  // PULL: wait for client read

                uno::Reference< io::XInputStream > xIn = getInputStream();
                if ( !xIn.is() )
                {
                    // No interaction if we are not persistent!
                    uno::Any aProps
                        = uno::makeAny(
                                 beans::PropertyValue(
                                     rtl::OUString(
                                         RTL_CONSTASCII_USTRINGPARAM("Uri")),
                                     -1,
                                     uno::makeAny(m_xIdentifier->
                                                      getContentIdentifier()),
                                     beans::PropertyState_DIRECT_VALUE));
                    ucbhelper::cancelCommandExecution(
                        star::ucb::IOErrorCode_CANT_READ,
                        uno::Sequence< uno::Any >(&aProps, 1),
                        m_eState == PERSISTENT
                            ? xEnv
                            : uno::Reference<
                                  star::ucb::XCommandEnvironment >(),
                        rtl::OUString::createFromAscii(
                            "Got no data stream!" ),
                        this );
                    // Unreachable
                }

                // Done.
                xDataSink->setInputStream( xIn );
            }
              else
            {
                // Note: aOpenCommand.Sink may contain an XStream
                //       implementation. Support for this type of
                //       sink is optional...
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        star::ucb::UnsupportedDataSinkException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                rArg.Sink ) ),
                    xEnv );
                // Unreachable
            }
          }
    }

    return uno::Any();
}

//=========================================================================
void Content::insert(
            const uno::Reference< io::XInputStream >& xStream,
            sal_Int32 nNameClashResolve,
            const uno::Reference< star::ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( isFolder() )
    {
        // Required: Title

        if ( !m_aProps.aTitle.getLength() )
            m_aProps.aTitle = m_aUri.getName();
    }
    else
    {
        // Required: rArg.Data

        if ( !xStream.is() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( star::ucb::MissingInputStreamException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                xEnv );
            // Unreachable
        }

        // Required: Title

        if ( !m_aProps.aTitle.getLength() )
            m_aProps.aTitle = m_aUri.getName();
    }

    rtl::OUString aNewURL = m_aUri.getParentUri();
    if (1 + aNewURL.lastIndexOf('/') != aNewURL.getLength())
        aNewURL += rtl::OUString::createFromAscii( "/" );
    aNewURL += PackageUri::encodeSegment( m_aProps.aTitle );
    PackageUri aNewUri( aNewURL );

    // Handle possible name clash...
    switch ( nNameClashResolve )
    {
        // fail.
        case star::ucb::NameClash::ERROR:
            if ( hasData( aNewUri ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( star::ucb::NameClashException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    task::InteractionClassification_ERROR,
                                    m_aProps.aTitle ) ),
                    xEnv );
                // Unreachable
            }
            break;

        // replace (possibly) existing object.
        case star::ucb::NameClash::OVERWRITE:
            break;

        // "invent" a new valid title.
        case star::ucb::NameClash::RENAME:
            if ( hasData( aNewUri ) )
            {
                sal_Int32 nTry = 0;

                do
                {
                    rtl::OUString aNew = aNewUri.getUri();
                    aNew += rtl::OUString::createFromAscii( "_" );
                    aNew += rtl::OUString::valueOf( ++nTry );
                    aNewUri.setUri( aNew );
                }
                while ( hasData( aNewUri ) && ( nTry < 1000 ) );

                if ( nTry == 1000 )
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            star::ucb::UnsupportedNameClashException(
                                rtl::OUString::createFromAscii(
                                    "Unable to resolve name clash!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                nNameClashResolve ) ),
                    xEnv );
                    // Unreachable
                }
                else
                {
                    m_aProps.aTitle += rtl::OUString::createFromAscii( "_" );
                    m_aProps.aTitle += rtl::OUString::valueOf( nTry );
                }
            }
            break;

        case star::ucb::NameClash::KEEP: // deprecated
        case star::ucb::NameClash::ASK:
        default:
            if ( hasData( aNewUri ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        star::ucb::UnsupportedNameClashException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            nNameClashResolve ) ),
                    xEnv );
                // Unreachable
            }
            break;
    }

    // Identifier changed?
    sal_Bool bNewId = ( m_aUri.getUri() != aNewUri.getUri() );

    if ( bNewId )
    {
        m_xIdentifier = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );
        m_aUri = aNewUri;
    }

    if ( !storeData( xStream ) )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(m_xIdentifier->
                                                   getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii( "Cannot store persistent data!" ),
            this );
        // Unreachable
    }

    m_eState = PERSISTENT;

    if ( bNewId )
    {
        // Take over correct default values from underlying packager...
        uno::Reference< container::XHierarchicalNameAccess > xXHierarchicalNameAccess;
        loadData( m_pProvider,
                  m_aUri,
                  m_aProps,
                  xXHierarchicalNameAccess );

        aGuard.clear();
        inserted();
    }
}

//=========================================================================
void Content::destroy(
                sal_Bool bDeletePhysical,
                const uno::Reference< star::ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< star::ucb::XContent > xThis = this;

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::UnsupportedCommandException(
                                rtl::OUString::createFromAscii(
                                    "Not persistent!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        // Unreachable
    }

    m_eState = DEAD;

    aGuard.clear();
    deleted();

    if ( isFolder() )
    {
        // Process instanciated children...

        ContentRefList aChildren;
        queryChildren( aChildren );

        ContentRefList::const_iterator it  = aChildren.begin();
        ContentRefList::const_iterator end = aChildren.end();

        while ( it != end )
        {
            (*it)->destroy( bDeletePhysical, xEnv );
            ++it;
        }
    }
}

//=========================================================================
void Content::transfer(
            const star::ucb::TransferInfo& rInfo,
            const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::UnsupportedCommandException(
                                rtl::OUString::createFromAscii(
                                    "Not persistent!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        // Unreachable
    }

    // Is source a package content?
    if ( ( rInfo.SourceURL.getLength() == 0 ) ||
         ( rInfo.SourceURL.compareTo(
            m_aUri.getUri(), PACKAGE_URL_SCHEME_LENGTH + 3 ) != 0 ) )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::InteractiveBadTransferURLException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        // Unreachable
    }

    // Is source not a parent of me / not me?
    rtl::OUString aId = m_aUri.getParentUri();
    aId += rtl::OUString::createFromAscii( "/" );

    if ( rInfo.SourceURL.getLength() <= aId.getLength() )
    {
        if ( aId.compareTo(
                rInfo.SourceURL, rInfo.SourceURL.getLength() ) == 0 )
        {
            uno::Any aProps
                = uno::makeAny(beans::PropertyValue(
                                      rtl::OUString(
                                          RTL_CONSTASCII_USTRINGPARAM("Uri")),
                                      -1,
                                      uno::makeAny(rInfo.SourceURL),
                                      beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_RECURSIVE,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Target is equal to or is a child of source!" ),
                this );
            // Unreachable
        }
    }

    //////////////////////////////////////////////////////////////////////
    // 0) Obtain content object for source.
    //////////////////////////////////////////////////////////////////////

    uno::Reference< star::ucb::XContentIdentifier > xId
        = new ::ucb::ContentIdentifier( m_xSMgr, rInfo.SourceURL );

    // Note: The static cast is okay here, because its sure that
    //       m_xProvider is always the PackageContentProvider.
    rtl::Reference< Content > xSource;

    try
    {
        xSource = static_cast< Content * >(
                        m_xProvider->queryContent( xId ).get() );
    }
    catch ( star::ucb::IllegalIdentifierException const & )
    {
        // queryContent
    }

    if ( !xSource.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(xId->getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Cannot instanciate source object!" ),
            this );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    // 1) Create new child content.
    //////////////////////////////////////////////////////////////////////

    rtl::OUString aType = xSource->isFolder()
            ? GetContentType( m_aUri.getScheme(), sal_True )
            : GetContentType( m_aUri.getScheme(), sal_False );
    star::ucb::ContentInfo aInfo;
    aInfo.Type = aType;
    aInfo.Attributes = 0;

    // Note: The static cast is okay here, because its sure that
    //       createNewContent always creates a Content.
    rtl::Reference< Content > xTarget
        = static_cast< Content * >( createNewContent( aInfo ).get() );
    if ( !xTarget.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Folder")),
                                  -1,
                                  uno::makeAny(aId),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "XContentCreator::createNewContent failed!" ),
            this );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    // 2) Copy data from source content to child content.
    //////////////////////////////////////////////////////////////////////

    uno::Sequence< beans::Property > aProps
                    = xSource->getPropertySetInfo( xEnv )->getProperties();
    sal_Int32 nCount = aProps.getLength();

    if ( nCount )
    {
        sal_Bool bHadTitle = ( rInfo.NewTitle.getLength() == 0 );

        // Get all source values.
        uno::Reference< sdbc::XRow > xRow
            = xSource->getPropertyValues( aProps );

        uno::Sequence< beans::PropertyValue > aValues( nCount );
        beans::PropertyValue* pValues = aValues.getArray();

        const beans::Property* pProps = aProps.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp  = pProps[ n ];
            beans::PropertyValue&  rValue = pValues[ n ];

            rValue.Name   = rProp.Name;
            rValue.Handle = rProp.Handle;

            if ( !bHadTitle && rProp.Name.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                // Set new title instead of original.
                bHadTitle = sal_True;
                rValue.Value <<= rInfo.NewTitle;
            }
            else
                rValue.Value
                    = xRow->getObject( n + 1,
                                       uno::Reference<
                                            container::XNameAccess >() );

            rValue.State = beans::PropertyState_DIRECT_VALUE;

            if ( rProp.Attributes & beans::PropertyAttribute::REMOVABLE )
            {
                // Add Additional Core Property.
                try
                {
                    xTarget->addProperty( rProp.Name,
                                          rProp.Attributes,
                                          rValue.Value );
                }
                catch ( beans::PropertyExistException const & )
                {
                }
                catch ( beans::IllegalTypeException const & )
                {
                }
                catch ( lang::IllegalArgumentException const & )
                {
                }
            }
        }

        // Set target values.
        xTarget->setPropertyValues( aValues, xEnv );
    }

    //////////////////////////////////////////////////////////////////////
    // 3) Commit (insert) child.
    //////////////////////////////////////////////////////////////////////

    xTarget->insert( xSource->getInputStream(), rInfo.NameClash, xEnv );

    //////////////////////////////////////////////////////////////////////
    // 4) Transfer (copy) children of source.
    //////////////////////////////////////////////////////////////////////

    if ( xSource->isFolder() )
    {
        uno::Reference< container::XEnumeration > xIter
            = xSource->getIterator();
        if ( xIter.is() )
        {
            while ( xIter->hasMoreElements() )
            {
                try
                {
                    uno::Reference< container::XNamed > xNamed;
                    xIter->nextElement() >>= xNamed;

                    if ( !xNamed.is() )
                    {
                        OSL_ENSURE( sal_False,
                                    "Content::transfer - Got no XNamed!" );
                        break;
                    }

                    rtl::OUString aName = xNamed->getName();

                    if ( !aName.getLength() )
                    {
                        OSL_ENSURE( sal_False,
                                    "Content::transfer - Empty name!" );
                        break;
                    }

                    rtl::OUString aChildId = xId->getContentIdentifier();
                    if ( ( aChildId.lastIndexOf( '/' ) + 1 )
                                                != aChildId.getLength() )
                        aChildId += rtl::OUString::createFromAscii( "/" );

                    aChildId += PackageUri::encodeSegment( aName );

                    star::ucb::TransferInfo aInfo;
                    aInfo.MoveData  = sal_False;
                    aInfo.NewTitle  = rtl::OUString();
                    aInfo.SourceURL = aChildId;
                    aInfo.NameClash = rInfo.NameClash;

                    // Transfer child to target.
                    xTarget->transfer( aInfo, xEnv );
                }
                catch ( container::NoSuchElementException const & )
                {
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // 5) Destroy source ( when moving only ) .
    //////////////////////////////////////////////////////////////////////

    if ( rInfo.MoveData )
    {
        xSource->destroy( sal_True, xEnv );

        // Remove all persistent data of source and its children.
        if ( !xSource->removeData() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny(
                                 xSource->m_xIdentifier->
                                              getContentIdentifier()),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Cannot remove persistent data of source object!" ),
                this );
            // Unreachable
        }

        // Remove own and all children's Additional Core Properties.
        xSource->removeAdditionalPropertySet( sal_True );
    }
}

//=========================================================================
sal_Bool Content::exchangeIdentity(
            const uno::Reference< star::ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< star::ucb::XContent > xThis = this;

    // Already persistent?
    if ( m_eState != PERSISTENT )
    {
        OSL_ENSURE( sal_False,
                    "Content::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
    PackageUri aNewUri( xNewId->getContentIdentifier() );
    if ( !hasData( aNewUri ) )
    {
        rtl::OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            m_aUri = aNewUri;
            if ( isFolder() )
            {
                // Process instanciated children...

                ContentRefList aChildren;
                queryChildren( aChildren );

                ContentRefList::const_iterator it  = aChildren.begin();
                ContentRefList::const_iterator end = aChildren.end();

                while ( it != end )
                {
                    ContentRef xChild = (*it);

                    // Create new content identifier for the child...
                    uno::Reference< star::ucb::XContentIdentifier > xOldChildId
                        = xChild->getIdentifier();
                    rtl::OUString aOldChildURL
                        = xOldChildId->getContentIdentifier();
                    rtl::OUString aNewChildURL
                        = aOldChildURL.replaceAt(
                                        0,
                                        aOldURL.getLength(),
                                        xNewId->getContentIdentifier() );
                    uno::Reference< star::ucb::XContentIdentifier > xNewChildId
                        = new ::ucb::ContentIdentifier( m_xSMgr, aNewChildURL );

                    if ( !xChild->exchangeIdentity( xNewChildId ) )
                        return sal_False;

                    ++it;
                }
            }
            return sal_True;
        }
    }

    OSL_ENSURE( sal_False,
            "Content::exchangeIdentity - Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucb::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    OSL_ENSURE( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ),
                "Content::queryChildren - Invalid URL!" );

    aURL += rtl::OUString::createFromAscii( "/" );

    sal_Int32 nLen = aURL.getLength();

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucb::ContentImplHelperRef xChild = (*it);
        rtl::OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            if ( aChildURL.indexOf( '/', nLen ) == -1 )
            {
                // No further slashes. It's a child!
                rChildren.push_back(
                    ContentRef(
                        static_cast< Content * >( xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
uno::Reference< container::XHierarchicalNameAccess > Content::getPackage(
                                                const PackageUri& rURI )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< container::XHierarchicalNameAccess > xPackage;
    if ( rURI.getPackage() == m_aUri.getPackage() )
    {
        if ( !m_xPackage.is() )
            m_xPackage = m_pProvider->createPackage( m_aUri.getPackage(), m_aUri.getParam() );

        return m_xPackage;
    }

    return m_pProvider->createPackage( rURI.getPackage(), rURI.getParam() );
}

//=========================================================================
uno::Reference< container::XHierarchicalNameAccess > Content::getPackage()
{
    return getPackage( m_aUri );
}

//=========================================================================
// static
sal_Bool Content::hasData(
            ContentProvider* pProvider,
            const PackageUri& rURI,
            uno::Reference< container::XHierarchicalNameAccess > & rxPackage )
{
    rxPackage = pProvider->createPackage( rURI.getPackage(), rURI.getParam() );
    if ( !rxPackage.is() )
        return sal_False;

    return rxPackage->hasByHierarchicalName( rURI.getPath() );
}

//=========================================================================
sal_Bool Content::hasData( const PackageUri& rURI )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< container::XHierarchicalNameAccess > xPackage;
    if ( rURI.getPackage() == m_aUri.getPackage() )
    {
        xPackage = getPackage();
        if ( !xPackage.is() )
            return sal_False;

        return xPackage->hasByHierarchicalName( rURI.getPath() );
    }

    return hasData( m_pProvider, rURI, xPackage );
}

//=========================================================================
//static
sal_Bool Content::loadData(
            ContentProvider* pProvider,
            const PackageUri& rURI,
            ContentProperties& rProps,
            uno::Reference< container::XHierarchicalNameAccess > & rxPackage )
{
    rxPackage = pProvider->createPackage( rURI.getPackage(), rURI.getParam() );
    if ( !rxPackage.is() )
        return sal_False;

    if ( rURI.isRootFolder() )
    {
        // Properties available only from package
        uno::Reference< beans::XPropertySet > xPackagePropSet(
                                                rxPackage, uno::UNO_QUERY );

        OSL_ENSURE( xPackagePropSet.is(),
                    "Content::loadData - "
                    "Got no XPropertySet interface from package!" );

        if ( xPackagePropSet.is() )
        {
            // HasEncryptedEntries ( only avalibale at root folder )
            try
            {
                uno::Any aHasEncryptedEntries
                    = xPackagePropSet->getPropertyValue(
                        rtl::OUString::createFromAscii(
                            "HasEncryptedEntries" ) );
                if ( !( aHasEncryptedEntries >>= rProps.bHasEncryptedEntries ) )
                {
                    OSL_ENSURE( sal_False,
                                "Content::loadData - "
                                "Got no HasEncryptedEntries value!" );
                    return sal_False;
                }
            }
            catch ( beans::UnknownPropertyException const & )
            {
                OSL_ENSURE( sal_False,
                            "Content::loadData - "
                            "Got no HasEncryptedEntries value!" );
                return sal_False;
            }
            catch ( lang::WrappedTargetException const & )
            {
                OSL_ENSURE( sal_False,
                            "Content::loadData - "
                            "Got no HasEncryptedEntries value!" );
                return sal_False;
            }
        }
    }

    if ( !rxPackage->hasByHierarchicalName( rURI.getPath() ) )
        return sal_False;

    try
    {
        uno::Any aEntry = rxPackage->getByHierarchicalName( rURI.getPath() );
        if ( aEntry.hasValue() )
        {
            uno::Reference< beans::XPropertySet > xPropSet;
            aEntry >>= xPropSet;

            if ( !xPropSet.is() )
            {
                OSL_ENSURE( sal_False,
                        "Content::loadData - Got no XPropertySet interface!" );
                return sal_False;
            }

            // Title
            rProps.aTitle = rURI.getName();

            // MediaType
            try
            {
                uno::Any aMediaType
                    = xPropSet->getPropertyValue(
                        rtl::OUString::createFromAscii( "MediaType" ) );
                if ( !( aMediaType >>= rProps.aMediaType ) )
                {
                    OSL_ENSURE( sal_False,
                                "Content::loadData - Got no MediaType value!" );
                    return sal_False;
                }
            }
            catch ( beans::UnknownPropertyException const & )
            {
                OSL_ENSURE( sal_False,
                            "Content::loadData - Got no MediaType value!" );
                return sal_False;
            }
            catch ( lang::WrappedTargetException const & )
            {
                OSL_ENSURE( sal_False,
                            "Content::loadData - Got no MediaType value!" );
                return sal_False;
            }

            uno::Reference< container::XEnumerationAccess > xEnumAccess;
            aEntry >>= xEnumAccess;

            // ContentType / IsFolder / IsDocument
            if ( xEnumAccess.is() )
            {
                // folder
                rProps.aContentType = GetContentType( rURI.getScheme(), sal_True );
                rProps.bIsDocument = sal_False;
                rProps.bIsFolder = sal_True;
            }
            else
            {
                // stream
                rProps.aContentType = GetContentType( rURI.getScheme(), sal_False );
                rProps.bIsDocument = sal_True;
                rProps.bIsFolder = sal_False;
            }

            if ( rProps.bIsDocument )
            {
                // Size ( only available for streams )
                try
                {
                    uno::Any aSize
                        = xPropSet->getPropertyValue(
                            rtl::OUString::createFromAscii( "Size" ) );
                    if ( !( aSize >>= rProps.nSize ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "Content::loadData - Got no Size value!" );
                        return sal_False;
                    }
                }
                catch ( beans::UnknownPropertyException const & )
                {
                    OSL_ENSURE( sal_False,
                                "Content::loadData - Got no Size value!" );
                    return sal_False;
                }
                catch ( lang::WrappedTargetException const & )
                {
                    OSL_ENSURE( sal_False,
                                "Content::loadData - Got no Size value!" );
                    return sal_False;
                }

                // Compressed ( only available for streams )
                try
                {
                    uno::Any aCompressed
                        = xPropSet->getPropertyValue(
                            rtl::OUString::createFromAscii( "Compressed" ) );
                    if ( !( aCompressed >>= rProps.bCompressed ) )
                    {
                        OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Compressed value!" );
                        return sal_False;
                    }
                }
                catch ( beans::UnknownPropertyException const & )
                {
                    OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Compressed value!" );
                    return sal_False;
                }
                catch ( lang::WrappedTargetException const & )
                {
                    OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Compressed value!" );
                    return sal_False;
                }

                // Encrypted ( only available for streams )
                try
                {
                    uno::Any aEncrypted
                        = xPropSet->getPropertyValue(
                            rtl::OUString::createFromAscii( "Encrypted" ) );
                    if ( !( aEncrypted >>= rProps.bEncrypted ) )
                    {
                        OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Encrypted value!" );
                        return sal_False;
                    }
                }
                catch ( beans::UnknownPropertyException const & )
                {
                    OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Encrypted value!" );
                    return sal_False;
                }
                catch ( lang::WrappedTargetException const & )
                {
                    OSL_ENSURE( sal_False,
                            "Content::loadData - Got no Encrypted value!" );
                    return sal_False;
                }
            }
            return sal_True;
        }
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName
    }

    return sal_False;
}

//=========================================================================
sal_Bool Content::renameData(
            const uno::Reference< star::ucb::XContentIdentifier >& xOldId,
            const uno::Reference< star::ucb::XContentIdentifier >& xNewId )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    PackageUri aURI( xOldId->getContentIdentifier() );
    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage(
                                                                        aURI );
    if ( !xNA.is() )
        return sal_False;

    if ( !xNA->hasByHierarchicalName( aURI.getPath() ) )
        return sal_False;

    try
    {
        uno::Any aEntry = xNA->getByHierarchicalName( aURI.getPath() );
        uno::Reference< container::XNamed > xNamed;
        aEntry >>= xNamed;

        if ( !xNamed.is() )
        {
            OSL_ENSURE( sal_False,
                        "Content::renameData - Got no XNamed interface!" );
            return sal_False;
        }

        PackageUri aNewURI( xNewId->getContentIdentifier() );

        // No success indicator!? No return value / exceptions specified.
        xNamed->setName( aNewURI.getName() );

        return sal_True;
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName
    }

    return sal_False;
}

//=========================================================================
sal_Bool Content::storeData( const uno::Reference< io::XInputStream >& xStream )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPackagePropSet(
                                                    xNA, uno::UNO_QUERY );
    OSL_ENSURE( xPackagePropSet.is(),
                "Content::storeData - "
                "Got no XPropertySet interface from package!" );

    if ( !xPackagePropSet.is() )
        return sal_False;

    if ( m_nModifiedProps & ENCRYPTIONKEY_MODIFIED )
    {
        if ( m_aUri.isRootFolder() )
        {
            // Property available only from package and from streams (see below)
            try
            {
                xPackagePropSet->setPropertyValue(
                        rtl::OUString::createFromAscii( "EncryptionKey" ),
                        uno::makeAny( m_aProps.aEncryptionKey ) );
                m_nModifiedProps &= ~ENCRYPTIONKEY_MODIFIED;
            }
            catch ( beans::UnknownPropertyException const & )
            {
                // setPropertyValue
            }
            catch ( beans::PropertyVetoException const & )
            {
                // setPropertyValue
            }
            catch ( lang::IllegalArgumentException const & )
            {
                // setPropertyValue
            }
            catch ( lang::WrappedTargetException const & )
            {
                // setPropertyValue
            }
        }
    }

    if ( !xNA->hasByHierarchicalName( m_aUri.getPath() ) )
    {
//      if ( !bCreate )
//          return sal_True;

        try
        {
            // Create new resource...
            uno::Reference< lang::XSingleServiceFactory > xFac(
                                                    xNA, uno::UNO_QUERY );
            if ( !xFac.is() )
            {
                OSL_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XSingleServiceFactory interface!" );
                return sal_False;
            }

            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= isFolder();

            uno::Reference< uno::XInterface > xNew
                = xFac->createInstanceWithArguments( aArgs );

            if ( !xNew.is() )
            {
                OSL_ENSURE( sal_False,
                            "Content::storeData - createInstance failed!" );
                return sal_False;
            }

            PackageUri aParentUri( getParentURL() );
            uno::Any aEntry
                = xNA->getByHierarchicalName( aParentUri.getPath() );
            uno::Reference< container::XNameContainer > xParentContainer;
            aEntry >>= xParentContainer;

            if ( !xParentContainer.is() )
            {
                OSL_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XNameContainer interface!" );
                return sal_False;
            }

            xParentContainer->insertByName( m_aProps.aTitle,
                                            uno::makeAny( xNew ) );
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( lang::IllegalArgumentException const & )
        {
            // insertByName
            OSL_ENSURE( sal_False,
                        "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( container::ElementExistException const & )
        {
            // insertByName
            OSL_ENSURE( sal_False,
                        "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( lang::WrappedTargetException const & )
        {
            // insertByName
            OSL_ENSURE( sal_False,
                        "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( container::NoSuchElementException const & )
        {
            // getByHierarchicalName
            OSL_ENSURE( sal_False,
                        "Content::storeData - getByHierarchicalName failed!" );
            return sal_False;
        }
        catch ( uno::Exception const & )
        {
            // createInstanceWithArguments
            OSL_ENSURE( sal_False, "Content::storeData - Error!" );
            return sal_False;
        }
    }

    if ( !xNA->hasByHierarchicalName( m_aUri.getPath() ) )
        return sal_False;

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet;
        xNA->getByHierarchicalName( m_aUri.getPath() ) >>= xPropSet;

        if ( !xPropSet.is() )
        {
            OSL_ENSURE( sal_False,
                        "Content::storeData - Got no XPropertySet interface!" );
            return sal_False;
        }

        //////////////////////////////////////////////////////////////////
        // Store property values...
        //////////////////////////////////////////////////////////////////

        if ( m_nModifiedProps & MEDIATYPE_MODIFIED )
        {
            xPropSet->setPropertyValue(
                                rtl::OUString::createFromAscii( "MediaType" ),
                                uno::makeAny( m_aProps.aMediaType ) );
            m_nModifiedProps &= ~MEDIATYPE_MODIFIED;
        }

        if ( m_nModifiedProps & COMPRESSED_MODIFIED )
        {
            if ( !isFolder() )
                xPropSet->setPropertyValue(
                                rtl::OUString::createFromAscii( "Compressed" ),
                                uno::makeAny( m_aProps.bCompressed ) );

            m_nModifiedProps &= ~COMPRESSED_MODIFIED;
        }

        if ( m_nModifiedProps & ENCRYPTED_MODIFIED )
        {
            if ( !isFolder() )
                xPropSet->setPropertyValue(
                                rtl::OUString::createFromAscii( "Encrypted" ),
                                uno::makeAny( m_aProps.bEncrypted ) );

            m_nModifiedProps &= ~ENCRYPTED_MODIFIED;
        }

        if ( m_nModifiedProps & ENCRYPTIONKEY_MODIFIED )
        {
            if ( !isFolder() )
                xPropSet->setPropertyValue(
                            rtl::OUString::createFromAscii( "EncryptionKey" ),
                            uno::makeAny( m_aProps.aEncryptionKey ) );

            m_nModifiedProps &= ~ENCRYPTIONKEY_MODIFIED;
        }

        //////////////////////////////////////////////////////////////////
        // Store data stream...
        //////////////////////////////////////////////////////////////////

        if ( xStream.is() && !isFolder() )
        {
            uno::Reference< io::XActiveDataSink > xSink(
                                                xPropSet, uno::UNO_QUERY );

            if ( !xSink.is() )
            {
                OSL_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XActiveDataSink interface!" );
                return sal_False;
            }

            xSink->setInputStream( xStream );
        }

        return sal_True;
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName
    }
    catch ( beans::UnknownPropertyException const & )
    {
        // setPropertyValue
    }
    catch ( beans::PropertyVetoException const & )
    {
        // setPropertyValue
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // setPropertyValue
    }
    catch ( lang::WrappedTargetException const & )
    {
        // setPropertyValue
    }

    OSL_ENSURE( sal_False, "Content::storeData - Error!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::removeData()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    PackageUri aParentUri( getParentURL() );
    if ( !xNA->hasByHierarchicalName( aParentUri.getPath() ) )
        return sal_False;

    try
    {
        uno::Any aEntry = xNA->getByHierarchicalName( aParentUri.getPath() );
        uno::Reference< container::XNameContainer > xContainer;
        aEntry >>= xContainer;

        if ( !xContainer.is() )
        {
            OSL_ENSURE( sal_False,
                        "Content::removeData - "
                        "Got no XNameContainer interface!" );
            return sal_False;
        }

        xContainer->removeByName( m_aUri.getName() );
        return sal_True;
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName, removeByName
    }
    catch ( lang::WrappedTargetException const & )
    {
        // removeByName
    }

    OSL_ENSURE( sal_False, "Content::removeData - Error!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::flushData()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Note: XChangesBatch is only implemented by the package itself, not
    //       by the single entries. Maybe this has to change...

    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    uno::Reference< util::XChangesBatch > xBatch( xNA, uno::UNO_QUERY );
    if ( !xBatch.is() )
    {
        OSL_ENSURE( sal_False,
                    "Content::flushData - Got no XChangesBatch interface!" );
        return sal_False;
    }

    try
    {
        xBatch->commitChanges();
        return sal_True;
    }
    catch ( lang::WrappedTargetException const & )
    {
    }

    OSL_ENSURE( sal_False, "Content::flushData - Error!" );
    return sal_False;
}

//=========================================================================
uno::Reference< io::XInputStream > Content::getInputStream()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< io::XInputStream > xStream;
    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return xStream;

    if ( !xNA->hasByHierarchicalName( m_aUri.getPath() ) )
        return xStream;

    try
    {
        uno::Any aEntry = xNA->getByHierarchicalName( m_aUri.getPath() );
        uno::Reference< io::XActiveDataSink > xSink;
        aEntry >>= xSink;

        if ( !xSink.is() )
        {
            OSL_ENSURE( sal_False,
                        "Content::getInputStream - "
                        "Got no XActiveDataSink interface!" );
            return xStream;
        }

        xStream = xSink->getInputStream();

        OSL_ENSURE( xStream.is(),
                    "Content::getInputStream - Got no stream!" );
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName
    }

    return xStream;
}

//=========================================================================
uno::Reference< container::XEnumeration > Content::getIterator()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< container::XEnumeration > xIter;
    uno::Reference< container::XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return xIter;

    if ( !xNA->hasByHierarchicalName( m_aUri.getPath() ) )
        return xIter;

    try
    {
        uno::Any aEntry = xNA->getByHierarchicalName( m_aUri.getPath() );
        uno::Reference< container::XEnumerationAccess > xIterFac;
        aEntry >>= xIterFac;

        if ( !xIterFac.is() )
        {
            OSL_ENSURE( sal_False,
                        "Content::getIterator - "
                        "Got no XEnumerationAccess interface!" );
            return xIter;
        }

        xIter = xIterFac->createEnumeration();

        OSL_ENSURE( xIter.is(),
                    "Content::getIterator - Got no iterator!" );
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName
    }

    return xIter;
}

