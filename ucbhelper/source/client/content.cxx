/*************************************************************************
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-20 09:35:52 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_CONTENTCREATIONERROR_HPP_
#include <com/sun/star/ucb/ContentCreationError.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTACTION_HPP_
#include <com/sun/star/ucb/ContentAction.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_GLOBALTRANSFERCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTEVENTLISTENER_HPP_
#include <com/sun/star/ucb/XContentEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_ACTIVEDATASINK_HXX
#include <ucbhelper/activedatasink.hxx>
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

namespace ucb
{

//=========================================================================
//=========================================================================
//
// class ContentEventListener_Impl.
//
//=========================================================================
//=========================================================================

class ContentEventListener_Impl : public cppu::OWeakObject,
                                     public XContentEventListener
{
    Content_Impl& m_rContent;

public:
    ContentEventListener_Impl( Content_Impl& rContent )
    : m_rContent( rContent ) {}

     // XInterface
    XINTERFACE_DECL()

    // XContentEventListener
    virtual void SAL_CALL contentEvent( const ContentEvent& evt )
        throw( RuntimeException );

    // XEventListener ( base of XContentEventListener )
    virtual void SAL_CALL disposing( const EventObject& Source )
        throw( RuntimeException );
};

//=========================================================================
//=========================================================================
//
// class Content_Impl.
//
//=========================================================================
//=========================================================================

class Content_Impl : public vos::OReference
{
friend class ContentEventListener_Impl;

    mutable OUString                    m_aURL;
    Reference< XMultiServiceFactory >   m_xSMgr;
    Reference< XContent >               m_xContent;
    Reference< XCommandProcessor >      m_xCommandProcessor;
      Reference< XCommandEnvironment >      m_xEnv;
    Reference< XContentEventListener >  m_xContentEventListener;
    mutable vos::OMutex                 m_aMutex;
    sal_Int32                           m_nCommandId;

private:
    void reinit( const Reference< XContent >& xContent );

public:
    Content_Impl() : m_nCommandId( 0 ) {};
    Content_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                  const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv );

    virtual ~Content_Impl();

    const OUString&                getURL() const;
    Reference< XContent >          getContent() const { return m_xContent; }
    Reference< XCommandProcessor > getCommandProcessor();
    sal_Int32 getCommandId();

    Any  executeCommand( const Command& rCommand );
    void abortCommand();
    inline const Reference< XCommandEnvironment >& getEnvironment() const;
    inline void setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv );
};

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content()
: m_xImpl( new Content_Impl )
{
}

//=========================================================================
Content::Content( const OUString& rURL,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Broker!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_BROKER );

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    Reference< XContentIdentifierFactory > xIdFac
                        = pBroker->getContentIdentifierFactoryInterface();
    if ( !xIdFac.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Identifier factory!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_IDENTIFIER_FACTORY );

    Reference< XContentIdentifier > xId
                        = xIdFac->createContentIdentifier( rURL );
    if ( !xId.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Identifier!" ),
                    Reference< XInterface >(),
                    ContentCreationError_IDENTIFIER_CREATION_FAILED );

    Reference< XContentProvider > xProvider
        = pBroker->getContentProviderInterface();
    if ( !xProvider.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Provider!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_PROVIDER );

    Reference< XContent > xContent;
    try
    {
        xContent = xProvider->queryContent( xId );
    }
    catch ( IllegalIdentifierException& )
    {
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content!" ),
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );
    }

    if ( !xContent.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content!" ),
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContentIdentifier >& rId,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Broker!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_BROKER );

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    Reference< XContentProvider > xProvider
        = pBroker->getContentProviderInterface();
    if ( !xProvider.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Provider!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_PROVIDER );

    Reference< XContent > xContent;
    try
    {
        xContent = xProvider->queryContent( rId );
    }
    catch ( IllegalIdentifierException& )
    {
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content!" ),
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );
    }

    if ( !xContent.is() )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content!" ),
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        throw ContentCreationException(
                    OUString::createFromAscii( "No Content Broker!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_BROKER );

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), rContent, rEnv );
}

//=========================================================================
Content::Content( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
}

//=========================================================================
// static
sal_Bool Content::create( const OUString& rURL,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        return sal_False;

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    Reference< XContentIdentifierFactory > xIdFac
                        = pBroker->getContentIdentifierFactoryInterface();
    if ( !xIdFac.is() )
        return sal_False;

    Reference< XContentIdentifier > xId
                        = xIdFac->createContentIdentifier( rURL );
    if ( !xId.is() )
        return sal_False;

    Reference< XContentProvider > xProvider
        = pBroker->getContentProviderInterface();
    if ( !xProvider.is() )
        return sal_False;

    Reference< XContent > xContent;
    try
    {
        xContent = xProvider->queryContent( xId );
    }
//  catch ( IllegalIdentifierException& )
    catch ( Exception& )
    {
        return sal_False;
    }

    if ( !xContent.is() )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
    return sal_True;
}

//=========================================================================
// static
sal_Bool Content::create( const Reference< XContentIdentifier >& rId,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        return sal_False;

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    Reference< XContentProvider > xProvider
        = pBroker->getContentProviderInterface();
    if ( !xProvider.is() )
        return sal_False;

    Reference< XContent > xContent;
    try
    {
        xContent = xProvider->queryContent( rId );
    }
//  catch ( IllegalIdentifierException& )
    catch ( Exception& )
    {
        return sal_False;
    }

    if ( !xContent.is() )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
    return sal_True;
}

//=========================================================================
// static
sal_Bool Content::create( const Reference< XContent >& xContent,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        return sal_False;

    VOS_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                "Content Broker not configured (no providers)!" );

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
    return sal_True;
}

//=========================================================================
Content::~Content()
{
}

//=========================================================================
Content& Content::operator=( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
    return *this;
}

//=========================================================================
Reference< XContent > Content::get() const
{
    return m_xImpl->getContent();
}

//=========================================================================
const OUString& Content::getURL() const
{
    return m_xImpl->getURL();
}

//=========================================================================
const Reference< XCommandEnvironment >& Content::getCommandEnvironment() const
{
    return m_xImpl->getEnvironment();
}

//=========================================================================
void Content::setCommandEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv )
{
    m_xImpl->setEnvironment( xNewEnv );
}

//=========================================================================
Reference< XCommandInfo > Content::getCommands()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "getCommandInfo" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XCommandInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Reference< XPropertySetInfo > Content::getProperties()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "getPropertySetInfo" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XPropertySetInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Any Content::getPropertyValue( const OUString& rPropertyName )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rPropertyName;

    Sequence< Any > aRet = getPropertyValues( aNames );
    return aRet.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::getPropertyValue( sal_Int32 nPropertyHandle )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< sal_Int32 > aHandles( 1 );
    aHandles.getArray()[ 0 ] = nPropertyHandle;

    Sequence< Any > aRet = getPropertyValues( aHandles );
    return aRet.getConstArray()[ 0 ];
}

//=========================================================================
void Content::setPropertyValue( const OUString& rName,
                                  const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rName;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    setPropertyValues( aNames, aValues );
}

//=========================================================================
void Content::setPropertyValue( const sal_Int32 nPropertyHandle,
                                  const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< sal_Int32 > aHandles( 1 );
    aHandles.getArray()[ 0 ] = nPropertyHandle;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    setPropertyValues( aHandles, aValues );
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XRow > xRow = getPropertyValuesInterface( rPropertyNames );

    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Any > aValues( nCount );

    if ( xRow.is() )
    {
        Any* pValues = aValues.getArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
            pValues[ n ] = xRow->getObject( n + 1, Reference< XNameAccess >() );
    }

    return aValues;
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< sal_Int32 >& nPropertyHandles )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XRow > xRow = getPropertyValuesInterface( nPropertyHandles );

    sal_Int32 nCount = nPropertyHandles.getLength();
    Sequence< Any > aValues( nCount );

    if ( xRow.is() )
    {
        Any* pValues = aValues.getArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
            pValues[ n ] = xRow->getObject( n + 1, Reference< XNameAccess >() );
    }

    return aValues;
}

//=========================================================================
Reference< XRow > Content::getPropertyValuesInterface(
                            const Sequence< OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const OUString* pNames  = rPropertyNames.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = pNames[ n ];
        rProp.Handle     = -1; // n/a
//      rProp.Type       =
//      rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "getPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
Reference< XRow > Content::getPropertyValuesInterface(
                            const Sequence< sal_Int32 >& nPropertyHandles )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = nPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const sal_Int32* pHandles  = nPropertyHandles.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = OUString(); // n/a
        rProp.Handle     = pHandles[ n ];
//      rProp.Type       =
//      rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "getPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
void Content::setPropertyValues(
                            const Sequence< OUString >& rPropertyNames,
                               const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rPropertyNames.getLength() != rValues.getLength() )
        throw CommandAbortedException();

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const OUString* pNames  = rPropertyNames.getConstArray();
    const Any*      pValues = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
        rProp.Value  = pValues[ n ];
//      rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "setPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    m_xImpl->executeCommand( aCommand );
}

//=========================================================================
void Content::setPropertyValues(
                            const Sequence< sal_Int32 >& nPropertyHandles,
                               const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( nPropertyHandles.getLength() != rValues.getLength() )
        throw CommandAbortedException();

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const sal_Int32* pHandles = nPropertyHandles.getConstArray();
    const Any*       pValues  = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = OUString(); // n/a
        rProp.Handle = pHandles[ n ];
        rProp.Value  = pValues[ n ];
//      rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "setPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Any Content::executeCommand( const OUString& rCommandName,
                             const Any& rCommandArgument )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = rCommandName;
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = rCommandArgument;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Any Content::executeCommand( sal_Int32 nCommandHandle,
                             const Any& rCommandArgument )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = OUString(); // n/a
    aCommand.Handle   = nCommandHandle;
    aCommand.Argument = rCommandArgument;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
void Content::abortCommand()
{
    m_xImpl->abortCommand();
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isFolder() )
        return Reference< XResultSet >();

    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > xStaticSet;
    if ( aResult >>= xDynSet )
        xStaticSet = xDynSet->getStaticResultSet();

    VOS_ENSURE( xStaticSet.is(), "Content::createCursor - no cursor!" );

     if ( !xStaticSet.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aResult >>= xStaticSet;

        VOS_ENSURE( !xStaticSet.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return xStaticSet;
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isFolder() )
        return Reference< XResultSet >();

    sal_Int32 nCount = rPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const sal_Int32* pHandles = rPropertyHandles.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = OUString(); // n/a
        rProp.Handle = pHandles[ n ];
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > xStaticSet;
    if ( aResult >>= xDynSet )
        xStaticSet = xDynSet->getStaticResultSet();

    VOS_ENSURE( xStaticSet.is(), "Content::createCursor - no cursor!" );

    if ( !xStaticSet.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aResult >>= xStaticSet;

        VOS_ENSURE( !xStaticSet.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return xStaticSet;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isFolder() )
        return Reference< XDynamicResultSet >();

    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    Reference< XDynamicResultSet > xSet;
    m_xImpl->executeCommand( aCommand ) >>= xSet;

    VOS_ENSURE( xSet.is(), "Content::createDynamicCursor - no cursor!" );

    return xSet;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isFolder() )
        return Reference< XDynamicResultSet >();

    sal_Int32 nCount = rPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const sal_Int32* pHandles = rPropertyHandles.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = OUString(); // n/a
        rProp.Handle = pHandles[ n ];
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    Reference< XDynamicResultSet > xSet;
    m_xImpl->executeCommand( aCommand ) >>= xSet;

    VOS_ENSURE( xSet.is(), "Content::createDynamicCursor - no cursor!" );

    return xSet;
}

//=========================================================================
Reference< XInputStream > Content::openStream()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return Reference< XInputStream >();

    Reference< XActiveDataSink > xSink = new ActiveDataSink;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return xSink->getInputStream();
}

//=========================================================================
sal_Bool Content::openStream( const Reference< XActiveDataSink >& rSink )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return sal_False;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = rSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return sal_True;
}

//=========================================================================
sal_Bool Content::openStream( const Reference< XOutputStream >& rStream )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return sal_False;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = rStream;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return sal_True;
}

//=========================================================================
void Content::writeStream( const Reference< XInputStream >& rStream,
                            sal_Bool bReplaceExisting )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !rStream.is() )
        return;

    try
    {
        // Note: Throws an exception in case the resource does not exist,
        //       but writeStream must work for non-existing resources, too.
        if ( !isDocument() )
            return;
    }
    catch ( Exception& )
    {
    }

    InsertCommandArgument aArg;
    aArg.Data            = rStream;
    aArg.ReplaceExisting = bReplaceExisting;

    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "insert" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );
}

//=========================================================================
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                      const Sequence<   OUString >& rPropertyNames,
                                      const Sequence< Any >& rPropertyValues,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    return insertNewContent( rContentType,
                             rPropertyNames,
                             rPropertyValues,
                             Reference< XInputStream >(),
                             rNewContent );
}

//=========================================================================
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                      const Sequence< sal_Int32 >& nPropertyHandles,
                                      const Sequence< Any >& rPropertyValues,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    return insertNewContent( rContentType,
                             nPropertyHandles,
                             rPropertyValues,
                             Reference< XInputStream >(),
                             rNewContent );
}

//=========================================================================
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                      const Sequence<   OUString >& rPropertyNames,
                                      const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

    VOS_ENSURE( xCreator.is(),
                "Content::insertNewContent - Not a XContentCreator!" );

    if ( !xCreator.is() )
        return sal_False;

    ContentInfo aInfo;
    aInfo.Type = rContentType;
    aInfo.Attributes = 0;

    Reference< XContent > xNew = xCreator->createNewContent( aInfo );
    if ( !xNew.is() )
        return sal_False;

    Content aNewContent( xNew, m_xImpl->getEnvironment() );
    aNewContent.setPropertyValues( rPropertyNames, rPropertyValues );
    aNewContent.executeCommand( OUString::createFromAscii( "insert" ),
                                makeAny(
                                    InsertCommandArgument(
                                        rData,
                                        sal_False /* ReplaceExisting */ ) ) );
    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                      const Sequence< sal_Int32 >& nPropertyHandles,
                                      const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

    VOS_ENSURE( xCreator.is(),
                "Content::insertNewContent - Not a XContentCreator!" );

    if ( !xCreator.is() )
        return sal_False;

    ContentInfo aInfo;
    aInfo.Type = rContentType;
    aInfo.Attributes = 0;

    Reference< XContent > xNew = xCreator->createNewContent( aInfo );
    if ( !xNew.is() )
        return sal_False;

    Content aNewContent( xNew, m_xImpl->getEnvironment() );
    aNewContent.setPropertyValues( nPropertyHandles, rPropertyValues );
    aNewContent.executeCommand( OUString::createFromAscii( "insert" ),
                                makeAny(
                                    InsertCommandArgument(
                                        rData,
                                        sal_False /* ReplaceExisting */ ) ) );
    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::insertNewContent( const Content& rSourceContent,
                                    InsertOperation eOperation,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
        throw CommandAbortedException();

    Reference< XCommandProcessor > xCmdProc(
                                    pBroker->getCommandProcessorInterface() );
    if ( !xCmdProc.is() )
        throw CommandAbortedException();

    // Execute command "globalTransfer" at UCB.

    TransferCommandOperation eTransOp;
    switch ( eOperation )
    {
        case InsertOperation_COPY:
            eTransOp = TransferCommandOperation_COPY;
            break;

        case InsertOperation_MOVE:
            eTransOp = TransferCommandOperation_MOVE;
            break;

        case InsertOperation_LINK:
            eTransOp = TransferCommandOperation_LINK;
            break;

        default:
            VOS_ENSURE( sal_False,
                        "Content::insertNewContent - Unknown operation!" );
            throw CommandAbortedException();
    }

    GlobalTransferCommandArgument aTransferArg(
                                        eTransOp,
                                        rSourceContent.getURL(), // SourceURL
                                        getURL(),   // TargetURL,
                                        OUString(), // NewTitle
                                        NameClash::ERROR );
    Command aCommand;
    aCommand.Name     = OUString::createFromAscii( "globalTransfer" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aTransferArg;

    xCmdProc->execute( aCommand, 0, m_xImpl->getEnvironment() );
    return sal_True;
}

//=========================================================================
sal_Bool Content::isFolder()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bFolder = sal_False;
    if ( getPropertyValue( OUString::createFromAscii( "IsFolder" ) )
        >>= bFolder )
        return bFolder;

    throw CommandAbortedException();
}

//=========================================================================
sal_Bool Content::isDocument()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bDoc = sal_False;
    if ( getPropertyValue( OUString::createFromAscii( "IsDocument" ) )
        >>= bDoc )
        return bDoc;

    throw CommandAbortedException();
}

//=========================================================================
//=========================================================================
//
// Content_Impl Implementation.
//
//=========================================================================
//=========================================================================

Content_Impl::Content_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                              const Reference< XContent >& rContent,
                              const Reference< XCommandEnvironment >& rEnv )
: m_xSMgr( rSMgr ),
  m_xContent( rContent ),
  m_xEnv( rEnv ),
  m_nCommandId( 0 )
{
    if ( m_xContent.is() )
    {
        m_xContentEventListener = new ContentEventListener_Impl( *this );
        m_xContent->addContentEventListener( m_xContentEventListener );

#ifdef DEBUG
        // Only done on demand in product version, but a nice debug helper.
        getURL();
#endif
    }
}

//=========================================================================
void Content_Impl::reinit( const Reference< XContent >& xContent )
{
    vos::OGuard aGuard( m_aMutex );

    m_xCommandProcessor = 0;
    m_nCommandId = 0;
       m_aURL = OUString();

    if ( xContent.is() )
    {
        m_xContent->removeContentEventListener( m_xContentEventListener );
        m_xContent = xContent;
        m_xContent->addContentEventListener( m_xContentEventListener );

#ifdef DEBUG
        // Only done on demand in product version, but a nice debug helper.
        getURL();
#endif
    }
    else
    {
        m_xContent->removeContentEventListener( m_xContentEventListener );
        m_xContent = 0;
    }
}

//=========================================================================
// virtual
Content_Impl::~Content_Impl()
{
    if ( m_xContent.is() )
        m_xContent->removeContentEventListener( m_xContentEventListener );
}

//=========================================================================
const OUString& Content_Impl::getURL() const
{
    vos::OGuard aGuard( m_aMutex );

    if ( !m_aURL.getLength() && m_xContent.is() )
    {
        Reference< XContentIdentifier > xId = m_xContent->getIdentifier();
        if ( xId.is() )
            m_aURL = xId->getContentIdentifier();
    }

    return m_aURL;
}

//=========================================================================
Reference< XCommandProcessor > Content_Impl::getCommandProcessor()
{
    vos::OGuard aGuard( m_aMutex );

    if ( !m_xCommandProcessor.is() )
        m_xCommandProcessor
            = Reference< XCommandProcessor >( m_xContent, UNO_QUERY );

    return m_xCommandProcessor;
}

//=========================================================================
sal_Int32 Content_Impl::getCommandId()
{
    vos::OGuard aGuard( m_aMutex );

    if ( m_nCommandId == 0 )
    {
        Reference< XCommandProcessor > xProc = getCommandProcessor();
        if ( xProc.is() )
            m_nCommandId = xProc->createCommandIdentifier();
    }

    return m_nCommandId;
}

//=========================================================================
Any Content_Impl::executeCommand( const Command& rCommand )
{
    Reference< XCommandProcessor > xProc = getCommandProcessor();
    if ( !xProc.is() )
        return Any();

    // Execute command
    return xProc->execute( rCommand, getCommandId(), m_xEnv );
}

//=========================================================================
void Content_Impl::abortCommand()
{
    if ( ( m_nCommandId != 0 ) && m_xCommandProcessor.is() )
        m_xCommandProcessor->abort( m_nCommandId );
}

//=========================================================================
inline const Reference< XCommandEnvironment >&
                                        Content_Impl::getEnvironment() const
{
    return m_xEnv;
}

//=========================================================================
inline void Content_Impl::setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv )
{
    vos::OGuard aGuard( m_aMutex );
    m_xEnv = xNewEnv;
}

//=========================================================================
//=========================================================================
//
// ContentEventListener_Impl Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( ContentEventListener_Impl,
                   XContentEventListener,
                   XEventListener ); /* base of XContentEventListener */

//=========================================================================
//
// XContentEventListener methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentEventListener_Impl::contentEvent( const ContentEvent& evt )
    throw( RuntimeException )
{
    if ( evt.Source == m_rContent.m_xContent )
    {
        switch ( evt.Action )
        {
            case ContentAction::DELETED:
                m_rContent.reinit( Reference< XContent >() );
                break;

            case ContentAction::EXCHANGED:
                m_rContent.reinit( evt.Content );
                break;

            default:
                break;
        }
    }
}

//=========================================================================
//
// XEventListenr methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentEventListener_Impl::disposing( const EventObject& Source )
    throw( RuntimeException )
{
    if ( Source.Source == m_rContent.m_xContent )
        m_rContent.reinit( Reference< XContent >() );
}

} /* namespace ucb */

