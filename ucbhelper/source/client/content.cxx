/*************************************************************************
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:36:37 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
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
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
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
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
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
#ifndef _UCBHELPER_INTERACTIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

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

class Content_Impl : public salhelper::SimpleReferenceObject
{
friend class ContentEventListener_Impl;

    mutable rtl::OUString               m_aURL;
    Reference< XMultiServiceFactory >   m_xSMgr;
    Reference< XContent >               m_xContent;
    Reference< XCommandProcessor >      m_xCommandProcessor;
      Reference< XCommandEnvironment >      m_xEnv;
    Reference< XContentEventListener >  m_xContentEventListener;
    mutable osl::Mutex                  m_aMutex;
    sal_Int32                           m_nCommandId;

private:
    void reinit( const Reference< XContent >& xContent );
    void disposing(const EventObject& Source);

public:
    Content_Impl() : m_nCommandId( 0 ) {};
    Content_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                  const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv );

    virtual ~Content_Impl();

    const rtl::OUString&           getURL() const;
    Reference< XContent >          getContent();
    Reference< XCommandProcessor > getCommandProcessor();
    sal_Int32 getCommandId();
    Reference< XMultiServiceFactory > getServiceManager() { return m_xSMgr; }

    Any  executeCommand( const Command& rCommand );
    void abortCommand();
    inline const Reference< XCommandEnvironment >& getEnvironment() const;
    inline void setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv );
};

//=========================================================================
// Helpers.
//=========================================================================

static ucb::ContentBroker* getContentBroker( bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();

    if ( !pBroker )
    {
        if ( bThrow )
            throw ContentCreationException(
                    rtl::OUString::createFromAscii( "No Content Broker!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_BROKER );
    }
    else
        OSL_ENSURE( pBroker->getContentProviderManagerInterface()
                                    ->queryContentProviders().getLength(),
                    "Content Broker not configured (no providers)!" );

    return pBroker;
}

//=========================================================================
static Reference< XContentIdentifier > getContentIdentifier(
                                    const ucb::ContentBroker & rBroker,
                                    const rtl::OUString & rURL,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentIdentifierFactory > xIdFac
                        = rBroker.getContentIdentifierFactoryInterface();
    if ( xIdFac.is() )
    {
        Reference< XContentIdentifier > xId
            = xIdFac->createContentIdentifier( rURL );

        if ( xId.is() )
            return xId;

        if ( bThrow )
            throw ContentCreationException(
                rtl::OUString::createFromAscii( "No Content Identifier!" ),
                Reference< XInterface >(),
                ContentCreationError_IDENTIFIER_CREATION_FAILED );
    }
    else
    {
        if ( bThrow )
            throw ContentCreationException(
                        rtl::OUString::createFromAscii(
                            "No Content Identifier factory!" ),
                        Reference< XInterface >(),
                        ContentCreationError_NO_IDENTIFIER_FACTORY );
    }

    return Reference< XContentIdentifier >();
}

//=========================================================================
static Reference< XContent > getContent(
                                    const ucb::ContentBroker & rBroker,
                                    const Reference< XContentIdentifier > & xId,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentProvider > xProvider
        = rBroker.getContentProviderInterface();
    if ( xProvider.is() )
    {
        Reference< XContent > xContent;
        try
        {
            xContent = xProvider->queryContent( xId );
        }
        catch ( IllegalIdentifierException const & )
        {
        }

        if ( xContent.is() )
            return xContent;

        if ( bThrow )
            throw ContentCreationException(
                    rtl::OUString::createFromAscii( "No Content!" ),
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );
    }
    else
    {
        if ( bThrow )
            throw ContentCreationException(
                    rtl::OUString::createFromAscii( "No Content Provider!" ),
                    Reference< XInterface >(),
                    ContentCreationError_NO_CONTENT_PROVIDER );
    }

    return Reference< XContent >();
}

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
Content::Content( const rtl::OUString& rURL,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = getContentBroker( true );

    Reference< XContentIdentifier > xId
        = getContentIdentifier( *pBroker, rURL, true );

    Reference< XContent > xContent = getContent( *pBroker, xId, true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContentIdentifier >& rId,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = getContentBroker( true );

    Reference< XContent > xContent = getContent( *pBroker, rId, true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ucb::ContentBroker* pBroker = getContentBroker( true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), rContent, rEnv );
}

//=========================================================================
Content::Content( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
}

//=========================================================================
// static
sal_Bool Content::create( const rtl::OUString& rURL,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ucb::ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

    Reference< XContentIdentifier > xId
        = getContentIdentifier( *pBroker, rURL, false );
    if ( !xId.is() )
        return sal_False;

    Reference< XContent > xContent = getContent( *pBroker, xId, false );
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
    ucb::ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

    Reference< XContent > xContent = getContent( *pBroker, rId, false );
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
    ucb::ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

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
const rtl::OUString& Content::getURL() const
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
    aCommand.Name     = rtl::OUString::createFromAscii( "getCommandInfo" );
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
    aCommand.Name     = rtl::OUString::createFromAscii( "getPropertySetInfo" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XPropertySetInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Any Content::getPropertyValue( const rtl::OUString& rPropertyName )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< rtl::OUString > aNames( 1 );
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
Any Content::setPropertyValue( const rtl::OUString& rName,
                                  const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< rtl::OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rName;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    Sequence< Any > aErrors = setPropertyValues( aNames, aValues );
    return aErrors.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::setPropertyValue( const sal_Int32 nPropertyHandle,
                                  const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< sal_Int32 > aHandles( 1 );
    aHandles.getArray()[ 0 ] = nPropertyHandle;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    Sequence< Any > aErrors = setPropertyValues( aHandles, aValues );
    return aErrors.getConstArray()[ 0 ];
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< rtl::OUString >& rPropertyNames )
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
                            const Sequence< rtl::OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const rtl::OUString* pNames  = rPropertyNames.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = pNames[ n ];
        rProp.Handle     = -1; // n/a
//      rProp.Type       =
//      rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "getPropertyValues" );
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

        rProp.Name       = rtl::OUString(); // n/a
        rProp.Handle     = pHandles[ n ];
//      rProp.Type       =
//      rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "getPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
Sequence< Any > Content::setPropertyValues(
                            const Sequence< rtl::OUString >& rPropertyNames,
                               const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rPropertyNames.getLength() != rValues.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            makeAny( IllegalArgumentException(
                        rtl::OUString::createFromAscii(
                            "Length of property names sequence and value "
                            "sequence are unequal!" ),
                        get(),
                        -1 ) ),
            m_xImpl->getEnvironment() );
        // Unreachable
    }

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const rtl::OUString* pNames  = rPropertyNames.getConstArray();
    const Any* pValues = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
        rProp.Value  = pValues[ n ];
//      rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "setPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Sequence< Any > aErrors;
    aResult >>= aErrors;
    return aErrors;
}

//=========================================================================
Sequence< Any > Content::setPropertyValues(
                            const Sequence< sal_Int32 >& nPropertyHandles,
                               const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( nPropertyHandles.getLength() != rValues.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            makeAny( IllegalArgumentException(
                        rtl::OUString::createFromAscii(
                            "Length of property handles sequence and value "
                            "sequence are unequal!" ),
                        get(),
                        -1 ) ),
            m_xImpl->getEnvironment() );
        // Unreachable
    }

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const sal_Int32* pHandles = nPropertyHandles.getConstArray();
    const Any*       pValues  = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = rtl::OUString(); // n/a
        rProp.Handle = pHandles[ n ];
        rProp.Value  = pValues[ n ];
//      rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "setPropertyValues" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Sequence< Any > aErrors;
    aResult >>= aErrors;
    return aErrors;
}

//=========================================================================
Any Content::executeCommand( const rtl::OUString& rCommandName,
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
    aCommand.Name     = rtl::OUString(); // n/a
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
Any Content::createCursorAny( const Sequence< rtl::OUString >& rPropertyNames,
                              ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const rtl::OUString* pNames = rPropertyNames.getConstArray();
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
    aCommand.Name     = rtl::OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Any Content::createCursorAny( const Sequence< sal_Int32 >& rPropertyHandles,
                              ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const sal_Int32* pHandles = rPropertyHandles.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = rtl::OUString(); // n/a
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
    aCommand.Name     = rtl::OUString::createFromAscii( "open" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Any aCursorAny = createCursorAny( rPropertyNames, eMode );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > aResult;

    aCursorAny >>= xDynSet;
    if ( xDynSet.is() )
        aResult = xDynSet->getStaticResultSet();

    OSL_ENSURE( aResult.is(), "Content::createCursor - no cursor!" );

     if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Any aCursorAny = createCursorAny( rPropertyHandles, eMode );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > aResult;

    aCursorAny >>= xDynSet;
    if ( xDynSet.is() )
        aResult = xDynSet->getStaticResultSet();

    OSL_ENSURE( aResult.is(), "Content::createCursor - no cursor!" );

     if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    createCursorAny( rPropertyNames, eMode ) >>= aResult;

    OSL_ENSURE( aResult.is(), "Content::createDynamicCursor - no cursor!" );

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    createCursorAny( rPropertyHandles, eMode ) >>= aResult;

    OSL_ENSURE( aResult.is(), "Content::createDynamicCursor - no cursor!" );

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createSortedDynamicCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    Reference< XDynamicResultSet > aOrigCursor = createDynamicCursor( rPropertyNames, eMode );

    if( aOrigCursor.is() )
    {
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString::createFromAscii( "com.sun.star.ucb.SortedDynamicResultSetFactory" )),
                                UNO_QUERY );

            aResult = aSortFactory->createSortedDynamicResultSet( aOrigCursor,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aResult.is(), "Content::createSortedDynamicCursor - no sorted cursor!\n" );

        if( !aResult.is() )
            aResult = aOrigCursor;
    }

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createSortedDynamicCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    Reference< XDynamicResultSet > aOrigCursor = createDynamicCursor( rPropertyHandles, eMode );

    if( aOrigCursor.is() )
    {
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString::createFromAscii( "com.sun.star.ucb.SortedDynamicResultSetFactory" )),
                                UNO_QUERY );

            aResult = aSortFactory->createSortedDynamicResultSet( aOrigCursor,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aResult.is(), "Content::createSortedDynamicCursor - no sorted cursor!\n" );

        if( !aResult.is() )
            aResult = aOrigCursor;
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createSortedCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XResultSet > aResult;
    Reference< XDynamicResultSet > aDynSet;

    Any aCursorAny = createCursorAny( rPropertyNames, eMode );

    aCursorAny >>= aDynSet;

    if( aDynSet.is() )
    {
        Reference< XDynamicResultSet > aDynResult;
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString::createFromAscii( "com.sun.star.ucb.SortedDynamicResultSetFactory" )),
                                UNO_QUERY );

            aDynResult = aSortFactory->createSortedDynamicResultSet( aDynSet,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aDynResult.is(), "Content::createSortedCursor - no sorted cursor!\n" );

        if( aDynResult.is() )
            aResult = aDynResult->getStaticResultSet();
        else
            aResult = aDynSet->getStaticResultSet();
    }

    OSL_ENSURE( aResult.is(), "Content::createSortedCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createSortedCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XResultSet > aResult;
    Reference< XDynamicResultSet > aDynSet;

    Any aCursorAny = createCursorAny( rPropertyHandles, eMode );

    aCursorAny >>= aDynSet;

    if( aDynSet.is() )
    {
        Reference< XDynamicResultSet > aDynResult;
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString::createFromAscii( "com.sun.star.ucb.SortedDynamicResultSetFactory" )),
                                UNO_QUERY );

            aDynResult = aSortFactory->createSortedDynamicResultSet( aDynSet,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aDynResult.is(), "Content::createSortedCursor - no sorted cursor!\n" );

        if( aDynResult.is() )
            aResult = aDynResult->getStaticResultSet();
        else
            aResult = aDynSet->getStaticResultSet();
    }

    OSL_ENSURE( aResult.is(), "Content::createSortedCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
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
    aCommand.Name     = rtl::OUString::createFromAscii( "open" );
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
    aCommand.Name     = rtl::OUString::createFromAscii( "open" );
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
    aCommand.Name     = rtl::OUString::createFromAscii( "open" );
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

    InsertCommandArgument aArg;
    aArg.Data            = rStream;
    aArg.ReplaceExisting = bReplaceExisting;

    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "insert" );
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< rtl::OUString >&
                                        rPropertyNames,
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
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< sal_Int32 >&
                                        nPropertyHandles,
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
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< rtl::OUString >&
                                        rPropertyNames,
                                      const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

    OSL_ENSURE( xCreator.is(),
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
    aNewContent.executeCommand( rtl::OUString::createFromAscii( "insert" ),
                                makeAny(
                                    InsertCommandArgument(
                                        rData,
                                        sal_False /* ReplaceExisting */ ) ) );
    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< sal_Int32 >&
                                        nPropertyHandles,
                                      const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                      Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

    OSL_ENSURE( xCreator.is(),
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
    aNewContent.executeCommand( rtl::OUString::createFromAscii( "insert" ),
                                makeAny(
                                    InsertCommandArgument(
                                        rData,
                                        sal_False /* ReplaceExisting */ ) ) );
    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::transferContent( const Content& rSourceContent,
                                   InsertOperation eOperation,
                                   const rtl::OUString & rTitle,
                                     const sal_Int32 nNameClashAction )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    ucb::ContentBroker* pBroker = ucb::ContentBroker::get();
    if ( !pBroker )
    {
        OSL_ENSURE( sal_False,
                    "Content::transferContent - No Content Broker!" );
        return sal_False;
    }

    Reference< XCommandProcessor > xCmdProc(
                                    pBroker->getCommandProcessorInterface() );
    if ( !xCmdProc.is() )
    {
        OSL_ENSURE( sal_False,
                    "Content::transferContent - No XCommandProcessor!" );
        return sal_False;
    }

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
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                            rtl::OUString::createFromAscii(
                                "Unknown transfer operation!" ),
                            get(),
                            -1 ) ),
                         m_xImpl->getEnvironment() );
            // Unreachable
    }

    GlobalTransferCommandArgument aTransferArg(
                                        eTransOp,
                                        rSourceContent.getURL(), // SourceURL
                                        getURL(),   // TargetFolderURL,
                                        rTitle,
                                        nNameClashAction );
    Command aCommand;
    aCommand.Name     = rtl::OUString::createFromAscii( "globalTransfer" );
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
    if ( getPropertyValue( rtl::OUString::createFromAscii( "IsFolder" ) )
        >>= bFolder )
        return bFolder;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    rtl::OUString::createFromAscii(
                        "Unable to retreive value of property 'IsFolder'!" ),
                    get() ) ),
         m_xImpl->getEnvironment() );

    // Unreachable - cancelCommandExecution always throws an exception.
    // But some compilers complain...
    return sal_False;
}

//=========================================================================
sal_Bool Content::isDocument()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bDoc = sal_False;
    if ( getPropertyValue( rtl::OUString::createFromAscii( "IsDocument" ) )
        >>= bDoc )
        return bDoc;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    rtl::OUString::createFromAscii(
                        "Unable to retreive value of property 'IsDocument'!" ),
                    get() ) ),
         m_xImpl->getEnvironment() );

    // Unreachable - cancelCommandExecution always throws an exception,
    // But some compilers complain...
    return sal_False;
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

#if OSL_DEBUG_LEVEL > 1
        // Only done on demand in product version for performance reasons,
        // but a nice debug helper.
        getURL();
#endif
    }
}

//=========================================================================
void Content_Impl::reinit( const Reference< XContent >& xContent )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_xCommandProcessor = 0;
    m_nCommandId = 0;

    // #92581# - Don't reset m_aURL!!!

    if ( m_xContent.is() )
    {
        try
        {
            m_xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }

    if ( xContent.is() )
    {
        m_xContent = xContent;
        m_xContent->addContentEventListener( m_xContentEventListener );

#if OSL_DEBUG_LEVEL > 1
        // Only done on demand in product version for performance reasons,
        // but a nice debug helper.
        getURL();
#endif
    }
    else
    {
        // We need m_xContent's URL in order to be able to create the
        // content object again if demanded ( --> Content_Impl::getContent() )
        getURL();

        m_xContent = 0;
    }
}

//=========================================================================
// virtual
Content_Impl::~Content_Impl()
{
    if ( m_xContent.is() )
    {
        try
        {
            m_xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }
}


void Content_Impl::disposing( const EventObject& Source )
{
    Reference<XContent> xContent;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if(Source.Source != m_xContent)
            return;

        xContent = m_xContent;

        m_nCommandId = 0;
        m_aURL = rtl::OUString();
        m_xCommandProcessor = 0;
        m_xContent = 0;
    }

    if ( xContent.is() )
    {
        try
        {
            xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }
}

//=========================================================================
const rtl::OUString& Content_Impl::getURL() const
{
    if ( !m_aURL.getLength() && m_xContent.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_aURL.getLength() && m_xContent.is() )
        {
            Reference< XContentIdentifier > xId = m_xContent->getIdentifier();
            if ( xId.is() )
                m_aURL = xId->getContentIdentifier();
        }
    }

    return m_aURL;
}

//=========================================================================
Reference< XContent > Content_Impl::getContent()
{
    if ( !m_xContent.is() && m_aURL.getLength() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xContent.is() && m_aURL.getLength() )
        {
            ucb::ContentBroker* pBroker = ucb::ContentBroker::get();

            OSL_ENSURE( pBroker, "No Content Broker!" );

            if ( pBroker )
            {
                OSL_ENSURE( pBroker->getContentProviderManagerInterface()
                                        ->queryContentProviders().getLength(),
                            "Content Broker not configured (no providers)!" );

                Reference< XContentIdentifierFactory > xIdFac
                            = pBroker->getContentIdentifierFactoryInterface();

                OSL_ENSURE( xIdFac.is(), "No Content Identifier factory!" );

                if ( xIdFac.is() )
                {
                    Reference< XContentIdentifier > xId
                                = xIdFac->createContentIdentifier( m_aURL );

                    OSL_ENSURE( xId.is(), "No Content Identifier!" );

                    if ( xId.is() )
                    {
                        Reference< XContentProvider > xProvider
                            = pBroker->getContentProviderInterface();

                        OSL_ENSURE( xProvider.is(), "No Content Provider!" );

                        if ( xProvider.is() )
                        {
                            try
                            {
                                m_xContent = xProvider->queryContent( xId );
                            }
                            catch ( IllegalIdentifierException const & )
                            {
                            }

                            if ( m_xContent.is() )
                                m_xContent->addContentEventListener(
                                                m_xContentEventListener );
                        }
                    }
                }
            }
        }
    }

    return m_xContent;
}

//=========================================================================
Reference< XCommandProcessor > Content_Impl::getCommandProcessor()
{
    if ( !m_xCommandProcessor.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xCommandProcessor.is() )
            m_xCommandProcessor
                = Reference< XCommandProcessor >( getContent(), UNO_QUERY );
    }

    return m_xCommandProcessor;
}

//=========================================================================
sal_Int32 Content_Impl::getCommandId()
{
    if ( m_nCommandId == 0 )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_nCommandId == 0 )
        {
            Reference< XCommandProcessor > xProc = getCommandProcessor();
            if ( xProc.is() )
                m_nCommandId = xProc->createCommandIdentifier();
        }
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
    sal_Int32 nCommandId;
    Reference< XCommandProcessor > xCommandProcessor;
    {
        osl::MutexGuard aGuard( m_aMutex );
        nCommandId = m_nCommandId;
        xCommandProcessor = m_xCommandProcessor;
    }

    if ( ( nCommandId != 0 ) && xCommandProcessor.is() )
        xCommandProcessor->abort( nCommandId );
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
    osl::MutexGuard aGuard( m_aMutex );
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
    m_rContent.disposing(Source);
}

} /* namespace ucb */

