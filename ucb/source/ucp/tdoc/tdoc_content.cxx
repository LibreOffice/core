/*************************************************************************
 *
 *  $RCSfile: tdoc_content.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:50:50 $
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

 - support for encrypted streams needed?
 - Content::storeData: remove dummyfile workaround for creation of new folders

 *************************************************************************/

#define STORAGE_CREATION_WORKAROUND 1

#include "osl/diagnose.h"
#include "osl/doublecheckedlocking.h"
#include "rtl/ustrbuf.hxx"

#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/embed/ElementModes.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/embed/XTransactedObject.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/io/XActiveDataStreamer.hpp"
#include "com/sun/star/lang/IllegalAccessException.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "com/sun/star/ucb/ContentInfoAttribute.hpp"
#include "com/sun/star/ucb/InsertCommandArgument.hpp"
#include "com/sun/star/ucb/InteractiveBadTransferURLException.hpp"
#include "com/sun/star/ucb/MissingInputStreamException.hpp"
#include "com/sun/star/ucb/MissingPropertiesException.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/NameClashException.hpp"
#include "com/sun/star/ucb/OpenCommandArgument2.hpp"
#include "com/sun/star/ucb/OpenMode.hpp"
#include "com/sun/star/ucb/TransferInfo.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "com/sun/star/ucb/UnsupportedDataSinkException.hpp"
#include "com/sun/star/ucb/UnsupportedNameClashException.hpp"
#include "com/sun/star/ucb/UnsupportedOpenModeException.hpp"
#include "com/sun/star/ucb/XCommandInfo.hpp"
#include "com/sun/star/ucb/XPersistentPropertySet.hpp"

#include "ucbhelper/cancelcommandexecution.hxx"
#include "ucbhelper/contentidentifier.hxx"
#include "ucbhelper/propertyvalueset.hxx"

#include "tdoc_content.hxx"
#include "tdoc_resultset.hxx"
#include "tdoc_passwordrequest.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace tdoc_ucp;

//=========================================================================
static ContentType lcl_getContentType( const rtl::OUString & rType )
{
    if ( rType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_ROOT_CONTENT_TYPE ) ) )
        return ROOT;
    else if ( rType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_DOCUMENT_CONTENT_TYPE ) ) )
        return DOCUMENT;
    else if ( rType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_FOLDER_CONTENT_TYPE ) ) )
        return FOLDER;
    else if ( rType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_STREAM_CONTENT_TYPE ) ) )
        return STREAM;
    else
    {
        OSL_ENSURE( sal_False,
                    "Content::Content - unsupported content type string" );
        return STREAM;
    }
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
    // Fail, if resource does not exist.
    ContentProperties aProps;
    if ( !Content::loadData( pProvider,
                             Uri( Identifier->getContentIdentifier() ),
                             aProps ) )
        return 0;

    return new Content( rxSMgr, pProvider, Identifier, aProps );
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

    if ( !Info.Type.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( TDOC_FOLDER_CONTENT_TYPE ) ) &&
         !Info.Type.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( TDOC_STREAM_CONTENT_TYPE ) ) )
    {
        OSL_ENSURE( sal_False, "Content::create - unsupported content type!" );
        return 0;
    }

#if 0
    // Fail, if content does exist.
    if ( Content::hasData( pProvider,
                           Uri( Identifier->getContentIdentifier() ) ) )
        return 0;
#endif

    return new Content( rxSMgr, pProvider, Identifier, Info );
}

//=========================================================================
Content::Content(
            const uno::Reference< lang::XMultiServiceFactory > & rxSMgr,
            ContentProvider * pProvider,
            const uno::Reference< star::ucb::XContentIdentifier > & Identifier,
            const ContentProperties & rProps )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_aProps( rProps ),
  m_eState( PERSISTENT ),
  m_pProvider( pProvider )
{
}

//=========================================================================
// ctor for a content just created via XContentCreator::createNewContent()
Content::Content(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const uno::Reference< star::ucb::XContentIdentifier >& Identifier,
            const star::ucb::ContentInfo& Info )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  m_aProps( lcl_getContentType( Info.Type ), rtl::OUString() ), // no Title (yet)
  m_eState( TRANSIENT ),
  m_pProvider( pProvider )
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
    uno::Any aRet = ContentImplHelper::queryInterface( rType );

    if ( !aRet.hasValue() )
    {
        aRet = cppu::queryInterface(
                rType, static_cast< star::ucb::XContentCreator * >( this ) );
        if ( aRet.hasValue() )
        {
            if ( !isContentCreator() )
                return uno::Any();
        }
    }

    return aRet;
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

    if ( isContentCreator() )
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
                "com.sun.star.comp.ucb.TransientDocumentsContent" );
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< rtl::OUString > aSNS( 1 );

    if ( m_aProps.getType() == STREAM )
        aSNS.getArray()[ 0 ] = rtl::OUString::createFromAscii(
                                TDOC_STREAM_CONTENT_SERVICE_NAME );
    else if ( m_aProps.getType() == FOLDER )
        aSNS.getArray()[ 0 ] = rtl::OUString::createFromAscii(
                                TDOC_FOLDER_CONTENT_SERVICE_NAME );
    else if ( m_aProps.getType() == DOCUMENT )
        aSNS.getArray()[ 0 ] = rtl::OUString::createFromAscii(
                                TDOC_DOCUMENT_CONTENT_SERVICE_NAME );
    else
        aSNS.getArray()[ 0 ] = rtl::OUString::createFromAscii(
                                TDOC_ROOT_CONTENT_SERVICE_NAME );

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
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return m_aProps.getContentType();
}

//=========================================================================
// virtual
uno::Reference< star::ucb::XContentIdentifier > SAL_CALL
Content::getIdentifier()
    throw( uno::RuntimeException )
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        // Transient?
        if ( m_eState == TRANSIENT )
        {
            // Transient contents have no identifier.
            return uno::Reference< star::ucb::XContentIdentifier >();
        }
    }
    return ContentImplHelper::getIdentifier();
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

        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

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
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert ( Supported by folders and streams only )
        //////////////////////////////////////////////////////////////////

        ContentType eType = m_aProps.getType();
        if ( ( eType != FOLDER ) && ( eType != STREAM ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( star::ucb::UnsupportedCommandException(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "insert command only supported by "
                                        "folders and streams!" ) ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            // Unreachable
        }

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
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete ( Supported by folders and streams only )
        //////////////////////////////////////////////////////////////////

        {
            osl::MutexGuard aGuard( m_aMutex );

            ContentType eType = m_aProps.getType();
            if ( ( eType != FOLDER ) && ( eType != STREAM ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( star::ucb::UnsupportedCommandException(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "delete command only supported by "
                                            "folders and streams!" ) ),
                                    static_cast< cppu::OWeakObject * >(
                                        this ) ) ),
                    Environment );
                // Unreachable
            }
        }

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
        // transfer ( Supported by document and folders only )
        //////////////////////////////////////////////////////////////////

        {
            osl::MutexGuard aGuard( m_aMutex );

            ContentType eType = m_aProps.getType();
            if ( ( eType != FOLDER ) && ( eType != DOCUMENT ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( star::ucb::UnsupportedCommandException(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "transfer command only supported "
                                            "by folders and documents!" ) ),
                                    static_cast< cppu::OWeakObject * >(
                                        this ) ) ),
                    Environment );
                // Unreachable
            }
        }

        star::ucb::TransferInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
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
    if ( isContentCreator() )
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
            = rtl::OUString::createFromAscii( TDOC_FOLDER_CONTENT_TYPE );
        aSeq.getArray()[ 0 ].Attributes
            = star::ucb::ContentInfoAttribute::KIND_FOLDER;
        aSeq.getArray()[ 0 ].Properties = aProps;

        // Stream.
        aSeq.getArray()[ 1 ].Type
            = rtl::OUString::createFromAscii( TDOC_STREAM_CONTENT_TYPE );
        aSeq.getArray()[ 1 ].Attributes
            = star::ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
              | star::ucb::ContentInfoAttribute::KIND_DOCUMENT;
        aSeq.getArray()[ 1 ].Properties = aProps;

        return aSeq;
    }
    else
    {
        OSL_ENSURE( sal_False,
                    "queryCreatableContentsInfo called on non-contentcreator "
                    "object!" );

        return uno::Sequence< star::ucb::ContentInfo >( 0 );
    }
}

//=========================================================================
// virtual
uno::Reference< star::ucb::XContent > SAL_CALL
Content::createNewContent( const star::ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    if ( isContentCreator() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !Info.Type.getLength() )
            return uno::Reference< star::ucb::XContent >();

        sal_Bool bCreateFolder =
            Info.Type.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_FOLDER_CONTENT_TYPE ) );

        if ( !bCreateFolder &&
             !Info.Type.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_STREAM_CONTENT_TYPE ) ) )
        {
            OSL_ENSURE( sal_False,
                        "Content::createNewContent - unsupported type!" );
            return uno::Reference< star::ucb::XContent >();
        }

        rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

        OSL_ENSURE( aURL.getLength() > 0,
                    "Content::createNewContent - empty identifier!" );

        if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
            aURL += rtl::OUString::createFromAscii( "/" );

        if ( bCreateFolder )
            aURL += rtl::OUString::createFromAscii( "New_Folder" );
        else
            aURL += rtl::OUString::createFromAscii( "New_Stream" );

        uno::Reference< star::ucb::XContentIdentifier > xId
            = new ::ucb::ContentIdentifier( m_xSMgr, aURL );

        return create( m_xSMgr, m_pProvider, xId, Info );
    }
    else
    {
        OSL_ENSURE( sal_False,
                    "createNewContent called on non-contentcreator object!" );
        return uno::Reference< star::ucb::XContent >();
    }
}

//=========================================================================
// virtual
rtl::OUString Content::getParentURL()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    Uri aUri( m_xIdentifier->getContentIdentifier() );
    return aUri.getParentUri();
}

//=========================================================================
uno::Reference< star::ucb::XContentIdentifier >
Content::makeNewIdentifier( const rtl::OUString& rTitle )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Assemble new content identifier...
    Uri aUri( m_xIdentifier->getContentIdentifier() );
    rtl::OUStringBuffer aNewURL = aUri.getParentUri();
    aNewURL.append( Uri::encodeSegment( rTitle ) );

    return
        uno::Reference< star::ucb::XContentIdentifier >(
            new ::ucb::ContentIdentifier(
                m_xSMgr, aNewURL.makeStringAndClear() ) );
}

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Only folders (root, documents, folders) have children.
    if ( !m_aProps.getIsFolder() )
        return;

    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucb::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += rtl::OUString::createFromAscii( "/" );
    }

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
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back(
                    ContentRef(
                        static_cast< Content * >(
                            xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
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

    // Only folders and streams can be renamed -> exchange identity.
    ContentType eType = m_aProps.getType();
    if ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
    {
        OSL_ENSURE( sal_False, "Content::exchangeIdentity - "
                               "Not supported by root or document!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
    if ( !hasData( Uri( xNewId->getContentIdentifier() ) ) )
    {
        rtl::OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            if ( eType == FOLDER )
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
                "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
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
    if ( loadData( pProvider, rContentId, aData ) )
    {
        return getPropertyValues(
            rSMgr, rProperties, aData, pProvider, rContentId );
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
                ContentProvider* pProvider,
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
                xRow->appendString ( rProp, rData.getContentType() );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                xRow->appendString ( rProp, rData.getTitle() );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                xRow->appendBoolean( rProp, rData.getIsDocument() );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                xRow->appendBoolean( rProp, rData.getIsFolder() );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Storage" ) ) )
            {
                // Storage is only supported by folders.
                ContentType eType = rData.getType();
                if ( eType == FOLDER )
                    xRow->appendObject(
                        rProp,
                        uno::makeAny(
                            pProvider->queryStorageClone( rContentId ) ) );
                else
                    xRow->appendVoid( rProp );
            }
            else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DocumentModel" ) ) )
            {
                // DocumentModel is only supported by documents.
                ContentType eType = rData.getType();
                if ( eType == DOCUMENT )
                    xRow->appendObject(
                        rProp,
                        uno::makeAny(
                            pProvider->queryDocumentModel( rContentId ) ) );
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
                            pProvider->getAdditionalPropertySet( rContentId,
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
            beans::Property( rtl::OUString::createFromAscii( "ContentType" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getContentType() );

        ContentType eType = rData.getType();

        xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Title" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      // Title is read-only for root and documents.
                      beans::PropertyAttribute::BOUND ||
                      ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
                        ? beans::PropertyAttribute::READONLY
                        : 0 ),
            rData.getTitle() );
        xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsDocument" ),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsDocument() );
        xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsFolder" ),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsFolder() );

        // Storage is only supported by folders.
        if ( eType == FOLDER )
            xRow->appendObject(
                beans::Property( rtl::OUString::createFromAscii( "Storage" ),
                          -1,
                          getCppuType(
                            static_cast<
                                const uno::Reference< embed::XStorage > * >( 0 ) ),
                          beans::PropertyAttribute::BOUND
                            | beans::PropertyAttribute::READONLY ),
                uno::makeAny( pProvider->queryStorageClone( rContentId ) ) );

        // DocumentModel is only supported by documents.
        if ( eType == DOCUMENT )
            xRow->appendObject(
                beans::Property( rtl::OUString::createFromAscii( "DocumentModel" ),
                          -1,
                          getCppuType(
                            static_cast<
                                const uno::Reference< frame::XModel > * >( 0 ) ),
                          beans::PropertyAttribute::BOUND
                            | beans::PropertyAttribute::READONLY ),
                uno::makeAny(
                    pProvider->queryDocumentModel( rContentId ) ) );

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
            pProvider->getAdditionalPropertySet( rContentId, sal_False ),
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
                              m_pProvider,
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
    rtl::OUString aOldTitle;
    sal_Int32 nTitlePos = -1;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(  "ContentType" ) ) )
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
            // Title is read-only for root and documents.
            ContentType eType = m_aProps.getType();
            if ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
            {
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
                        if ( aNewValue != m_aProps.getTitle() )
                        {
                            // modified title -> modified URL -> exchange !
                            if ( m_eState == PERSISTENT )
                                bExchange = sal_True;

                            aOldTitle = m_aProps.getTitle();
                            m_aProps.setTitle( aNewValue );

                            // property change event will be sent later...

                            // remember position within sequence of values
                            // (for error handling).
                            nTitlePos = n;
                        }
                    }
                    else
                    {
                        aRet[ n ] <<= lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Empty Title not allowed!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                    "Title Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Storage" ) ) )
        {
            ContentType eType = m_aProps.getType();
            if ( eType == FOLDER )
            {
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                // Storage is only supported by folders.
                aRet[ n ] <<= beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Storage property only supported by folders" ),
                            static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else if ( rValue.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DocumentModel" ) ) )
        {
            ContentType eType = m_aProps.getType();
            if ( eType == DOCUMENT )
            {
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                // Storage is only supported by folders.
                aRet[ n ] <<= beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "DocumentModel property only supported by "
                                "documents" ),
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
                    uno::Any aOldValue = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
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
        uno::Reference< star::ucb::XContentIdentifier > xOldId
            = m_xIdentifier;
        uno::Reference< star::ucb::XContentIdentifier > xNewId
            = makeNewIdentifier( m_aProps.getTitle() );

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
            // Roll-back.
            m_aProps.setTitle( aOldTitle );
            aOldTitle = rtl::OUString();

            // Set error .
            aRet[ nTitlePos ] <<= uno::Exception(
                    rtl::OUString::createFromAscii( "Exchange failed!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    if ( aOldTitle.getLength() )
    {
        aEvent.PropertyName = rtl::OUString::createFromAscii( "Title" );
        aEvent.OldValue     = uno::makeAny( aOldTitle );
        aEvent.NewValue     = uno::makeAny( m_aProps.getTitle() );

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        // Save changes, if content was already made persistent.
        if ( !bExchange && ( m_eState == PERSISTENT ) )
        {
            if ( !storeData( uno::Reference< io::XInputStream >(), xEnv ) )
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

        aChanges.realloc( nChanged );

        aGuard.clear();
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
            = new DynamicResultSet( m_xSMgr, this, rArg );
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

        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

        uno::Reference< io::XActiveDataStreamer > xDataStreamer(
                                        rArg.Sink, uno::UNO_QUERY );
        if ( xDataStreamer.is() )
        {
            // May throw CommandFailedException, DocumentPasswordRequest!
            uno::Reference< io::XStream > xStream = getStream( xEnv );
            if ( !xStream.is() )
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
            xDataStreamer->setStream( xStream );
        }
        else
        {
            uno::Reference< io::XOutputStream > xOut( rArg.Sink, uno::UNO_QUERY );
            if ( xOut.is() )
            {
                // PUSH: write data into xOut

                // May throw CommandFailedException, DocumentPasswordRequest!
                uno::Reference< io::XInputStream > xIn = getInputStream( xEnv );
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

                    // May throw CommandFailedException, DocumentPasswordRequest!
                    uno::Reference< io::XInputStream > xIn = getInputStream( xEnv );
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
    }

    return uno::Any();
}

//=========================================================================
void Content::insert( const uno::Reference< io::XInputStream >& xData,
                      sal_Int32 nNameClashResolve,
                      const uno::Reference<
                        star::ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();

    OSL_ENSURE( ( eType == FOLDER ) || ( eType == STREAM ),
                "insert command only supported by streams and folders!" );

    Uri aUri( m_xIdentifier->getContentIdentifier() );

    // Check, if all required properties were set.
    if ( eType == FOLDER )
    {
        // Required: Title

        if ( m_aProps.getTitle().getLength() == 0 )
            m_aProps.setTitle( aUri.getDecodedName() );
    }
    else // stream
    {
        // Required: data

        if ( !xData.is() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( star::ucb::MissingInputStreamException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                xEnv );
            // Unreachable
        }

        // Required: Title

        if ( m_aProps.getTitle().getLength() == 0 )
            m_aProps.setTitle( aUri.getDecodedName() );
    }

    rtl::OUStringBuffer aNewURL = aUri.getParentUri();
    aNewURL.append( m_aProps.getTitle() );
    Uri aNewUri( aNewURL.makeStringAndClear() );

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
                                    m_aProps.getTitle() ) ),
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
                    rtl::OUStringBuffer aNew = aNewUri.getUri();
                    aNew.appendAscii( "_" );
                    aNew.append( rtl::OUString::valueOf( ++nTry ) );
                    aNewUri.setUri( aNew.makeStringAndClear() );
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
                    rtl::OUStringBuffer aNewTitle = m_aProps.getTitle();
                    aNewTitle.appendAscii( "_" );
                    aNewTitle.append( rtl::OUString::valueOf( ++nTry ) );
                    m_aProps.setTitle( aNewTitle.makeStringAndClear() );
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
    sal_Bool bNewId = ( aUri != aNewUri );

    if ( bNewId )
    {
        m_xIdentifier
            = new ::ucb::ContentIdentifier( m_xSMgr, aNewUri.getUri() );
    }

    if ( !storeData( xData, xEnv ) )
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
        //loadData( m_pProvider, m_aUri, m_aProps );

        aGuard.clear();
        inserted();
    }
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical,
                       const uno::Reference<
                        star::ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();

    OSL_ENSURE( ( eType == FOLDER ) || ( eType == STREAM ),
                "delete command only supported by streams and folders!" );

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

    if ( eType == FOLDER )
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
void Content::notifyDocumentClosed()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    m_eState = DEAD;

    // @@@ anything else to reset or such?

    // callback follows!
    aGuard.clear();

    // Propagate destruction to content event listeners
    // Remove this from provider's content list.
    deleted();
}

//=========================================================================
uno::Reference< star::ucb::XContent >
Content::queryChildContent( const rtl::OUString & rRelativeChildUri )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    const rtl::OUString aMyId = getIdentifier()->getContentIdentifier();
    rtl::OUStringBuffer aBuf( aMyId );
    if ( aMyId.getStr()[ aMyId.getLength() - 1 ] != sal_Unicode( '/' ) )
        aBuf.appendAscii( "/" );
    if ( rRelativeChildUri.getStr()[ 0 ] != sal_Unicode( '/' ) )
        aBuf.append( rRelativeChildUri );
    else
        aBuf.append( rRelativeChildUri.copy( 1 ) );

    uno::Reference< star::ucb::XContentIdentifier > xChildId
        = new ::ucb::ContentIdentifier( m_xSMgr, aBuf.makeStringAndClear() );

    uno::Reference< star::ucb::XContent > xChild;
    try
    {
        xChild = m_pProvider->queryContent( xChildId );
    }
    catch ( star::ucb::IllegalIdentifierException const & )
    {
        // handled below.
    }

    OSL_ENSURE( xChild.is(),
                "Content::queryChildContent - unable to create child content!" );
    return xChild;
}

//=========================================================================
void Content::notifyChildRemoved( const rtl::OUString & rRelativeChildUri )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Ugly! Need to create child content object, just to fill event properly.
    uno::Reference< star::ucb::XContent > xChild
        = queryChildContent( rRelativeChildUri );

    if ( xChild.is() )
    {
        // callback follows!
        aGuard.clear();

        // Notify "REMOVED" event.
        star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( this ),
            star::ucb::ContentAction::REMOVED,
            xChild,
            getIdentifier() );
        notifyContentEvent( aEvt );
    }
}

//=========================================================================
void Content::notifyChildInserted( const rtl::OUString & rRelativeChildUri )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Ugly! Need to create child content object, just to fill event properly.
    uno::Reference< star::ucb::XContent > xChild
        = queryChildContent( rRelativeChildUri );

    if ( xChild.is() )
    {
        // callback follows!
        aGuard.clear();

        // Notify "INSERTED" event.
        star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( this ),
            star::ucb::ContentAction::INSERTED,
            xChild,
            getIdentifier() );
        notifyContentEvent( aEvt );
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

    // Does source URI scheme match? Only vnd.sun.star.tdoc is supported.

    if ( ( rInfo.SourceURL.getLength() < TDOC_URL_SCHEME_LENGTH + 2 ) )
    {
        // Invaild length (to short).
        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::InteractiveBadTransferURLException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        // Unreachable
    }

    rtl::OUString aScheme
        = rInfo.SourceURL.copy( 0, TDOC_URL_SCHEME_LENGTH + 2 )
            .toAsciiLowerCase();
    if ( !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( TDOC_URL_SCHEME ":/" ) ) )
    {
        // Invaild scheme.
        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::InteractiveBadTransferURLException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        // Unreachable
    }

    // Does source URI describe a tdoc folder or stream?
    Uri aSourceUri( rInfo.SourceURL );
    if ( !aSourceUri.isValid() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( lang::IllegalArgumentException(
                                rtl::OUString::createFromAscii(
                                    "Invalid source URI! Syntax!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 ) ),
            xEnv );
        // Unreachable
    }

    if ( aSourceUri.isRoot() || aSourceUri.isDocument() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( lang::IllegalArgumentException(
                                rtl::OUString::createFromAscii(
                                    "Invalid source URI! "
                                    "Must describe a folder or stream!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 ) ),
            xEnv );
        // Unreachable
    }

    // Is source not a parent of me / not me?
    rtl::OUString aId = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aId.lastIndexOf( '/' );
    if ( nPos != ( aId.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aId += rtl::OUString::createFromAscii( "/" );
    }

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
                                      uno::makeAny( rInfo.SourceURL ),
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

    /////////////////////////////////////////////////////////////////////////
    // Copy data.
    /////////////////////////////////////////////////////////////////////////

    rtl::OUString aNewName( rInfo.NewTitle.getLength() > 0
                                ? rInfo.NewTitle
                                : aSourceUri.getDecodedName() );

    if ( !copyData( aSourceUri, aNewName ) )
    {
        uno::Any aProps
            = uno::makeAny(
                     beans::PropertyValue(
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                           "Uri")),
                         -1,
                         uno::makeAny( rInfo.SourceURL ),
                         beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "Cannot copy data!" ) ),
            this );
        // Unreachable
    }

    /////////////////////////////////////////////////////////////////////////
    // Copy own and all children's Additional Core Properties.
    /////////////////////////////////////////////////////////////////////////

    rtl::OUString aTargetUri = m_xIdentifier->getContentIdentifier();
    if ( ( aTargetUri.lastIndexOf( '/' ) + 1 ) != aTargetUri.getLength() )
        aTargetUri += rtl::OUString::createFromAscii( "/" );

    if ( rInfo.NewTitle.getLength() > 0 )
        aTargetUri += Uri::encodeSegment( rInfo.NewTitle );
    else
        aTargetUri += aSourceUri.getName();

    if ( !copyAdditionalPropertySet(
            aSourceUri.getUri(), aTargetUri, sal_True ) )
    {
        uno::Any aProps
            = uno::makeAny(
                     beans::PropertyValue(
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                           "Uri")),
                         -1,
                         uno::makeAny( rInfo.SourceURL ),
                         beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "Cannot copy additional properties!" ) ),
            this );
        // Unreachable
    }

    /////////////////////////////////////////////////////////////////////////
    // Propagate new content.
    /////////////////////////////////////////////////////////////////////////

    rtl::Reference< Content > xTarget;
    try
    {
        uno::Reference< star::ucb::XContentIdentifier > xTargetId
            = new ::ucb::ContentIdentifier( m_xSMgr, aTargetUri );

        // Note: The static cast is okay here, because its sure that
        //       m_xProvider is always the WebDAVContentProvider.
        xTarget = static_cast< Content * >(
            m_pProvider->queryContent( xTargetId ).get() );

    }
    catch ( star::ucb::IllegalIdentifierException const & )
    {
        // queryContent
    }

    if ( !xTarget.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Uri")),
                                  -1,
                                  uno::makeAny( aTargetUri ),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Cannot instanciate target object!" ),
            this );
        // Unreachable
    }

    // Announce transfered content in its new folder.
    xTarget->inserted();

    /////////////////////////////////////////////////////////////////////////
    // Remove source, if requested
    /////////////////////////////////////////////////////////////////////////

    if ( rInfo.MoveData )
    {
        rtl::Reference< Content > xSource;
        try
        {
            uno::Reference< star::ucb::XContentIdentifier >
                xSourceId = new ::ucb::ContentIdentifier(
                    m_xSMgr, rInfo.SourceURL );

            // Note: The static cast is okay here, because its sure
            //       that m_xProvider is always the ContentProvider.
            xSource = static_cast< Content * >(
                m_xProvider->queryContent( xSourceId ).get() );
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
                                      uno::makeAny( rInfo.SourceURL ),
                                      beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_READ,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Cannot instanciate target object!" ),
                this );
            // Unreachable
        }

        // Propagate destruction (recursively).
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
                             uno::makeAny( rInfo.SourceURL ),
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
        if ( !xSource->removeAdditionalPropertySet( sal_True ) )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny( rInfo.SourceURL ),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Cannot remove additional properties of source object!" ),
                this );
            // Unreachable
        }

    } // rInfo.MoveData
}

//=========================================================================
bool Content::isContentCreator()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return
        ( m_aProps.getType() == FOLDER ) || ( m_aProps.getType() == DOCUMENT );
}

//=========================================================================
//static
bool Content::hasData( ContentProvider* pProvider, const Uri & rUri )
{
    if ( rUri.isRoot() )
    {
        return true; // root has no storage
    }
    else if ( rUri.isDocument() )
    {
        uno::Reference< embed::XStorage > xStorage
            = pProvider->queryStorage( rUri.getUri(), READ );
        return xStorage.is();
    }
    else
    {
        // folder or stream

        // Ask parent storage. In case that rUri describes a stream,
        // ContentProvider::queryStorage( rUri ) would return null.

        uno::Reference< embed::XStorage > xStorage
            = pProvider->queryStorage( rUri.getParentUri(), READ );

        if ( !xStorage.is() )
            return false;

        uno::Reference< container::XNameAccess > xParentNA(
            xStorage, uno::UNO_QUERY );

        OSL_ENSURE( xParentNA.is(), "Got no css.container.XNameAccess!" );

        return xParentNA->hasByName( rUri.getDecodedName() );
    }
}

//=========================================================================
//static
bool Content::loadData( ContentProvider* pProvider,
                        const Uri & rUri,
                        ContentProperties& rProps )
{
    if ( rUri.isRoot() ) // root has no storage, but can always be created
    {
        rProps
            = ContentProperties(
                ROOT, pProvider->queryStorageTitle( rUri.getUri() ) );
    }
    else if ( rUri.isDocument() ) // document must have storage
    {
        uno::Reference< embed::XStorage > xStorage
            = pProvider->queryStorage( rUri.getUri(), READ );

        if ( !xStorage.is() )
            return false;

        rProps
            = ContentProperties(
                DOCUMENT, pProvider->queryStorageTitle( rUri.getUri() ) );
    }
    else // stream or folder; stream has no storage; folder has storage
    {
        uno::Reference< embed::XStorage > xStorage
            = pProvider->queryStorage( rUri.getParentUri(), READ );

        if ( !xStorage.is() )
            return false;

        // Check whether exists at all, is stream or folder
        try
        {
            // return: true  -> folder
            // return: false -> stream
            // NoSuchElementException -> neither folder nor stream
            bool bIsFolder
                = xStorage->isStorageElement( rUri.getDecodedName() );

            rProps
                = ContentProperties(
                    bIsFolder ? FOLDER : STREAM,
                    pProvider->queryStorageTitle( rUri.getUri() ) );
        }
        catch ( container::NoSuchElementException const & )
        {
            // there is no element with such name
            //OSL_ENSURE( false, "Caught NoSuchElementException!" );
            return false;
        }
        catch ( lang::IllegalArgumentException const & )
        {
            // an illegal argument is provided
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
            return false;
        }
        catch ( embed::InvalidStorageException const & )
        {
            // this storage is in invalid state for any reason
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
            return false;
        }
    }
    return true;
}

//=========================================================================

#ifdef STORAGE_CREATION_WORKAROUND
static uno::Reference< io::XOutputStream > lcl_getTruncatedOutputStream(
                const rtl::OUString & rUri,
                ContentProvider * pProvider,
                const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest );
#endif /* STORAGE_CREATION_WORKAROUND */

bool Content::storeData( const uno::Reference< io::XInputStream >& xData,
                         const uno::Reference<
                            star::ucb::XCommandEnvironment >& xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();
    if ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
    {
        OSL_ENSURE( false, "storeData not supported by root and documents!" );
        return false;
    }

    Uri aUri( m_xIdentifier->getContentIdentifier() );

    if ( eType == FOLDER )
    {
        uno::Reference< embed::XStorage > xStorage
            = m_pProvider->queryStorage( aUri.getUri(), READ_WRITE_CREATE );

        if ( !xStorage.is() )
            return false;

        uno::Reference< beans::XPropertySet > xPropSet(
            xStorage, uno::UNO_QUERY );
        OSL_ENSURE( xPropSet.is(),
                    "Content::storeData - Got no XPropertySet interface!" );
        if ( !xPropSet.is() )
            return false;

        try
        {
            // According to MBA, if no mediatype is set, folder and all
            // its contents will be lost on save of the document!!!
            xPropSet->setPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                uno::makeAny(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        // @@@ better mediatype
                        "application/binary" ) ) ) );
        }
        catch ( beans::UnknownPropertyException const & )
        {
            OSL_ENSURE( false, "Property MediaType not supported!" );
            return false;
        }
        catch ( beans::PropertyVetoException const & )
        {
            OSL_ENSURE( false, "Caught PropertyVetoException!" );
            return false;
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
            return false;
        }
        catch ( lang::WrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught WrappedTargetException!" );
            return false;
        }

#ifdef STORAGE_CREATION_WORKAROUND
        // @@@ workaround(!) - current Storage API implementation does not
        // support empty substorages (<-- MAV). Final solution will do.
        // As a workaround we currently put a dummy file into the newly
        // created substorage.
        uno::Reference< io::XOutputStream > xOut;
        if ( m_eState == TRANSIENT )
        {
            rtl::OUStringBuffer aDummyStreamUri( aUri.getUri() );
            if ( aUri.getUri()[ aUri.getUri().getLength() - 1 ]
                    != sal_Unicode( '/' ) )
                aDummyStreamUri.appendAscii( "/" );

            aDummyStreamUri.appendAscii(
                "this_is_a_dummy_stream_just_there_as_a_workaround_for_a_"
                "temporary_limitation_of_the_storage_api_implementation" );

            // May throw CommandFailedException, DocumentPasswordRequest!
            xOut = lcl_getTruncatedOutputStream(
                    aDummyStreamUri.makeStringAndClear(), m_pProvider, xEnv );

            OSL_ENSURE( xOut.is(), "No target data stream!" );

            closeOutputStream( xOut );
        }
#endif /* STORAGE_CREATION_WORKAROUND */

        if ( !commitStorage( xStorage ) )
            return false;
    }
    else if ( eType == STREAM )
    {
        // stream

        // Important: Parent storage and output stream must be kept alive until
        //            changes have been committed!
        uno::Reference< embed::XStorage > xStorage
            = m_pProvider->queryStorage(
                aUri.getParentUri(), READ_WRITE_CREATE );
        uno::Reference< io::XOutputStream > xOut;

        if ( !xStorage.is() )
            return false;

        if ( xData.is() )
        {
            // May throw CommandFailedException, DocumentPasswordRequest!
            xOut = getTruncatedOutputStream( xEnv );

            OSL_ENSURE( xOut.is(), "No target data stream!" );

            try
            {
                uno::Sequence< sal_Int8 > aBuffer;
                sal_Int32 nRead = xData->readSomeBytes( aBuffer, 65536 );

                while ( nRead > 0 )
                {
                    aBuffer.realloc( nRead );
                    xOut->writeBytes( aBuffer );
                    aBuffer.realloc( 0 );
                    nRead = xData->readSomeBytes( aBuffer, 65536 );
                }

                closeOutputStream( xOut );
            }
            catch ( io::NotConnectedException const & )
            {
                // readSomeBytes, writeBytes
                OSL_ENSURE( false, "Caught NotConnectedException!" );
                closeOutputStream( xOut );
                return false;
            }
            catch ( io::BufferSizeExceededException const & )
            {
                // readSomeBytes, writeBytes
                OSL_ENSURE( false, "Caught BufferSizeExceededException!" );
                closeOutputStream( xOut );
                return false;
            }
            catch ( io::IOException const & )
            {
                // readSomeBytes, writeBytes
                OSL_ENSURE( false, "Caught IOException!" );
                closeOutputStream( xOut );
                return false;
            }
            catch ( ... )
            {
                closeOutputStream( xOut );
                throw;
            }
        }

        // Commit changes.
        if ( !commitStorage( xStorage ) )
            return false;
    }
    else
    {
        OSL_ENSURE( false, "Unknown content type!" );
        return false;
    }
    return true;
}

//=========================================================================
bool Content::renameData(
            const uno::Reference< star::ucb::XContentIdentifier >& xOldId,
            const uno::Reference< star::ucb::XContentIdentifier >& xNewId )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();
    if ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
    {
        OSL_ENSURE( false, "renameData not supported by root and documents!" );
        return false;
    }

    Uri aOldUri( xOldId->getContentIdentifier() );
    uno::Reference< embed::XStorage > xStorage
        = m_pProvider->queryStorage(
            aOldUri.getParentUri(), READ_WRITE_NOCREATE );

    if ( !xStorage.is() )
        return false;

    try
    {
        Uri aNewUri( xNewId->getContentIdentifier() );
        xStorage->renameElement(
            aOldUri.getDecodedName(), aNewUri.getDecodedName() );
    }
    catch ( embed::InvalidStorageException const & )
    {
        // this storage is in invalid state for eny reason
        OSL_ENSURE( false, "Caught InvalidStorageException!" );
        return false;
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // an illegal argument is provided
        OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        return false;
    }
    catch ( container::NoSuchElementException const & )
    {
        // there is no element with old name in this storage
        OSL_ENSURE( false, "Caught NoSuchElementException!" );
        return false;
    }
    catch ( container::ElementExistException const & )
    {
        // an element with new name already exists in this storage
        OSL_ENSURE( false, "Caught ElementExistException!" );
        return false;
    }
    catch ( io::IOException const & )
    {
        // in case of io errors during renaming
        OSL_ENSURE( false, "Caught IOException!" );
        return false;
    }
    catch ( embed::StorageWrappedTargetException const & )
    {
        // wraps other exceptions
        OSL_ENSURE( false, "Caught StorageWrappedTargetException!" );
        return false;
    }

    return commitStorage( xStorage );
}

//=========================================================================
bool Content::removeData()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();
    if ( ( eType == ROOT ) || ( eType == DOCUMENT ) )
    {
        OSL_ENSURE( false, "removeData not supported by root and documents!" );
        return false;
    }

    Uri aUri( m_xIdentifier->getContentIdentifier() );
    uno::Reference< embed::XStorage > xStorage
        = m_pProvider->queryStorage(
            aUri.getParentUri(), READ_WRITE_NOCREATE );

    if ( !xStorage.is() )
        return false;

    try
    {
        xStorage->removeElement( aUri.getDecodedName() );
    }
    catch ( embed::InvalidStorageException const & )
    {
        // this storage is in invalid state for eny reason
        OSL_ENSURE( false, "Caught InvalidStorageException!" );
        return false;
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // an illegal argument is provided
        OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        return false;
    }
    catch ( container::NoSuchElementException const & )
    {
        // there is no element with this name in this storage
        OSL_ENSURE( false, "Caught NoSuchElementException!" );
        return false;
    }
    catch ( io::IOException const & )
    {
        // in case of io errors during renaming
        OSL_ENSURE( false, "Caught IOException!" );
        return false;
    }
    catch ( embed::StorageWrappedTargetException const & )
    {
        // wraps other exceptions
        OSL_ENSURE( false, "Caught StorageWrappedTargetException!" );
        return false;
    }

    return commitStorage( xStorage );
}

//=========================================================================
bool Content::copyData( const Uri & rSourceUri, const rtl::OUString & rNewName )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    ContentType eType = m_aProps.getType();
    if ( ( eType == ROOT ) || ( eType == STREAM ) )
    {
        OSL_ENSURE( false, "copyData not supported by root and streams!" );
        return false;
    }

    Uri aDestUri( m_xIdentifier->getContentIdentifier() );
    uno::Reference< embed::XStorage > xDestStorage
        = m_pProvider->queryStorage( aDestUri.getUri(), READ_WRITE_NOCREATE );

    if ( !xDestStorage.is() )
        return false;

    uno::Reference< embed::XStorage > xSourceStorage
        = m_pProvider->queryStorage( rSourceUri.getParentUri(), READ );

    if ( !xSourceStorage.is() )
        return false;

    try
    {
        xSourceStorage->copyElementTo( rSourceUri.getDecodedName(),
                                       xDestStorage,
                                       rNewName );
    }
    catch ( embed::InvalidStorageException const & )
    {
        // this storage is in invalid state for eny reason
        OSL_ENSURE( false, "Caught InvalidStorageException!" );
        return false;
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // an illegal argument is provided
        OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        return false;
    }
    catch ( container::NoSuchElementException const & )
    {
        // there is no element with this name in this storage
        OSL_ENSURE( false, "Caught NoSuchElementException!" );
        return false;
    }
    catch ( container::ElementExistException const & )
    {
        // there is no element with this name in this storage
        OSL_ENSURE( false, "Caught ElementExistException!" );
        return false;
    }
    catch ( io::IOException const & )
    {
        // in case of io errors during renaming
        OSL_ENSURE( false, "Caught IOException!" );
        return false;
    }
    catch ( embed::StorageWrappedTargetException const & )
    {
        // wraps other exceptions
        OSL_ENSURE( false, "Caught StorageWrappedTargetException!" );
        return false;
    }

    return commitStorage( xDestStorage );
}

//=========================================================================
// static
bool Content::commitStorage( const uno::Reference< embed::XStorage > & xStorage )
{
    // Commit changes
    uno::Reference< embed::XTransactedObject > xTO( xStorage, uno::UNO_QUERY );

    OSL_ENSURE( xTO.is(),
                "Required interface css.embed.XTransactedObject missing!" );
    try
    {
        xTO->commit();
    }
    catch ( io::IOException const & )
    {
        OSL_ENSURE( false, "Caught IOException!" );
        return false;
    }
    catch ( lang::WrappedTargetException const & )
    {
        OSL_ENSURE( false, "Caught WrappedTargetException!" );
        return false;
    }

    return true;
}

//=========================================================================
// static
bool Content::closeOutputStream(
    const uno::Reference< io::XOutputStream > & xOut )
{
    if ( xOut.is() )
    {
        try
        {
            xOut->closeOutput();
            return true;
        }
        catch ( io::NotConnectedException const & )
        {
            OSL_ENSURE( false, "Caught NotConnectedException!" );
        }
        catch ( io::BufferSizeExceededException const & )
        {
            OSL_ENSURE( false, "Caught BufferSizeExceededException!" );
        }
        catch ( io::IOException const & )
        {
            OSL_ENSURE( false, "Caught IOException!" );
        }
    }
    return false;
}

//=========================================================================
static rtl::OUString obtainPassword(
        const rtl::OUString & rName,
        task::PasswordRequestMode eMode,
        const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    rtl::Reference< DocumentPasswordRequest > xRequest
        = new DocumentPasswordRequest( eMode, rName );

    if ( xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH
            = xEnv->getInteractionHandler();
        if ( xIH.is() )
        {
            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
            {
                // Handler handled the request.
                uno::Reference< task::XInteractionAbort > xAbort(
                    xSelection.get(), uno::UNO_QUERY );
                if ( xAbort.is() )
                {
                    throw star::ucb::CommandFailedException(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "Abort requested by Interaction Handler." ) ),
                        uno::Reference< uno::XInterface >(),
                        xRequest->getRequest() );
                }

                uno::Reference< task::XInteractionPassword > xPassword(
                    xSelection.get(), uno::UNO_QUERY );
                if ( xPassword.is() )
                {
                    return xPassword->getPassword();
                }

                // Unknown selection. Should never happen.
                throw star::ucb::CommandFailedException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "Interaction Handler selected unknown continuation!" ) ),
                    uno::Reference< uno::XInterface >(),
                    xRequest->getRequest() );
            }
        }
    }

    // No IH or IH did not handle exception.
    task::DocumentPasswordRequest aRequest;
    xRequest->getRequest() >>= aRequest;
    throw aRequest;
}

//=========================================================================
uno::Reference< io::XInputStream > Content::getInputStream(
        const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    rtl::OUString aUri;
    rtl::OUString aPassword;
    bool bPasswordRequested = false;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        OSL_ENSURE( m_aProps.getType() == STREAM,
                    "Content::getInputStream - content is no stream!" );

        aUri = Uri( m_xIdentifier->getContentIdentifier() ).getUri();
    }

    for ( ;; )
    {
        try
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            return uno::Reference< io::XInputStream >(
                m_pProvider->queryInputStream( aUri, aPassword ) );
        }
        catch ( packages::WrongPasswordException const & )
        {
            // Obtain (new) password.
            aPassword
                = obtainPassword( aUri, /* @@@ find better title */
                                  bPasswordRequested
                                   ? task::PasswordRequestMode_PASSWORD_REENTER
                                   : task::PasswordRequestMode_PASSWORD_ENTER,
                                   xEnv );
            bPasswordRequested = true;
        }
    }
}

//=========================================================================
static uno::Reference< io::XOutputStream > lcl_getTruncatedOutputStream(
                const rtl::OUString & rUri,
                ContentProvider * pProvider,
                const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    rtl::OUString aPassword;
    bool bPasswordRequested = false;
    for ( ;; )
    {
        try
        {
            return uno::Reference< io::XOutputStream >(
                pProvider->queryOutputStream(
                    rUri, aPassword, true /* truncate */ ) );
        }
        catch ( packages::WrongPasswordException const & )
        {
            // Obtain (new) password.
            aPassword
                = obtainPassword( rUri, /* @@@ find better title */
                                  bPasswordRequested
                                   ? task::PasswordRequestMode_PASSWORD_REENTER
                                   : task::PasswordRequestMode_PASSWORD_ENTER,
                                   xEnv );
            bPasswordRequested = true;
        }
    }
}

//=========================================================================
uno::Reference< io::XOutputStream > Content::getTruncatedOutputStream(
        const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    OSL_ENSURE( m_aProps.getType() == STREAM,
                "Content::getTruncatedOutputStream - content is no stream!" );

    return lcl_getTruncatedOutputStream(
            Uri( m_xIdentifier->getContentIdentifier() ).getUri(),
            m_pProvider,
            xEnv );
}

//=========================================================================
uno::Reference< io::XStream > Content::getStream(
        const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw ( star::ucb::CommandFailedException,
            task::DocumentPasswordRequest )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    OSL_ENSURE( m_aProps.getType() == STREAM,
                "Content::getStream - content is no stream!" );

    rtl::OUString aUri( Uri( m_xIdentifier->getContentIdentifier() ).getUri() );
    rtl::OUString aPassword;
    bool bPasswordRequested = false;
    for ( ;; )
    {
        try
        {
            return uno::Reference< io::XStream >(
                m_pProvider->queryStream(
                    aUri, aPassword, false /* no truncate */ ) );
        }
        catch ( packages::WrongPasswordException const & )
        {
            // Obtain (new) password.
            aPassword
                = obtainPassword( aUri, /* @@@ find better title */
                                  bPasswordRequested
                                   ? task::PasswordRequestMode_PASSWORD_REENTER
                                   : task::PasswordRequestMode_PASSWORD_ENTER,
                                   xEnv );
            bPasswordRequested = true;
        }
    }
}
