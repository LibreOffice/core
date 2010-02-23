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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#include <ucbhelper/configureucb.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include "identify.hxx"
#include "ucbcmds.hxx"

#include "ucb.hxx"

// Definitions for ProviderMap_Impl (Solaris wouldn't find explicit template
// instantiations for these in another compilation unit...):
#ifndef _UCB_REGEXPMAP_TPT_
#include <regexpmap.tpt>
#endif

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace ucb_impl;

//=========================================================================
//
// UniversalContentBroker Implementation.
//
//=========================================================================

UniversalContentBroker::UniversalContentBroker(
    const Reference< com::sun::star::lang::XMultiServiceFactory >& rXSMgr )
: m_xSMgr( rXSMgr ),
  m_pDisposeEventListeners( NULL ),
  m_nInitCount( 0 ), //@@@ see initialize() method
  m_nCommandId( 0 )
{
    OSL_ENSURE( m_xSMgr.is(),
                "UniversalContentBroker ctor: No service manager" );
}

//=========================================================================
// virtual
UniversalContentBroker::~UniversalContentBroker()
{
    delete m_pDisposeEventListeners;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_8( UniversalContentBroker,
                   XTypeProvider,
                   XComponent,
                   XServiceInfo,
                   XInitialization,
                   XContentProviderManager,
                   XContentProvider,
                   XContentIdentifierFactory,
                   XCommandProcessor );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_8( UniversalContentBroker,
                         XTypeProvider,
                      XComponent,
                         XServiceInfo,
                      XInitialization,
                         XContentProviderManager,
                         XContentProvider,
                      XContentIdentifierFactory,
                      XCommandProcessor );

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
void SAL_CALL UniversalContentBroker::dispose()
    throw( com::sun::star::uno::RuntimeException )
{
    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = SAL_STATIC_CAST( XComponent*, this );
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
}

//=========================================================================
// virtual
void SAL_CALL UniversalContentBroker::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( com::sun::star::uno::RuntimeException )
{
    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners = new OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL UniversalContentBroker::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( com::sun::star::uno::RuntimeException )
{
    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );

    // Note: Don't want to delete empty container here -> performance.
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( UniversalContentBroker,
                     OUString::createFromAscii(
                         "com.sun.star.comp.ucb.UniversalContentBroker" ),
                     OUString::createFromAscii(
                         UCB_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( UniversalContentBroker );

//=========================================================================
//
// XInitialization methods.
//
//=========================================================================

// virtual
void SAL_CALL UniversalContentBroker::initialize(
                    const com::sun::star::uno::Sequence< Any >& aArguments )
    throw( com::sun::star::uno::Exception,
           com::sun::star::uno::RuntimeException )
{
    //@@@ At the moment, there's a problem when one (non-one-instance) factory
    // 'wraps' another (one-instance) factory, causing this method to be
    // called several times:
    oslInterlockedCount nCount = osl_incrementInterlockedCount(&m_nInitCount);
    if (nCount == 1)
        ::ucbhelper::configureUcb(this, m_xSMgr, aArguments, 0);
    else
        osl_decrementInterlockedCount(&m_nInitCount);
            // make the possibility of overflow less likely...
}

//=========================================================================
//
// XContentProviderManager methods.
//
//=========================================================================

// virtual
Reference< XContentProvider > SAL_CALL
UniversalContentBroker::registerContentProvider(
                            const Reference< XContentProvider >& Provider,
                            const OUString& Scheme,
                            sal_Bool ReplaceExisting )
    throw( DuplicateProviderException, com::sun::star::uno::RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);

    ProviderMap_Impl::iterator aIt;
    try
    {
        aIt = m_aProviders.find(Scheme);
    }
    catch (IllegalArgumentException const &)
    {
        return 0; //@@@
    }

    Reference< XContentProvider > xPrevious;
    if (aIt == m_aProviders.end())
    {
        ProviderList_Impl aList;
        aList.push_front(Provider);
        try
        {
            m_aProviders.add(Scheme, aList, false);
        }
        catch (IllegalArgumentException const &)
        {
            return 0; //@@@
        }
    }
    else
    {
        if (!ReplaceExisting)
            throw DuplicateProviderException();

        ProviderList_Impl & rList = aIt->getValue();
        xPrevious = rList.front().getProvider();
        rList.push_front(Provider);
    }

    return xPrevious;
}

//=========================================================================
// virtual
void SAL_CALL UniversalContentBroker::deregisterContentProvider(
                              const Reference< XContentProvider >& Provider,
                            const OUString& Scheme )
    throw( com::sun::star::uno::RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);

    ProviderMap_Impl::iterator aMapIt;
    try
    {
        aMapIt = m_aProviders.find(Scheme);
    }
    catch (IllegalArgumentException const &)
    {
        return; //@@@
    }

    if (aMapIt != m_aProviders.end())
    {
        ProviderList_Impl & rList = aMapIt->getValue();

        ProviderList_Impl::iterator aListEnd(rList.end());
        for (ProviderList_Impl::iterator aListIt(rList.begin());
             aListIt != aListEnd; ++aListIt)
        {
            if ((*aListIt).getProvider() == Provider)
            {
                rList.erase(aListIt);
                break;
            }
        }

        if (rList.empty())
            m_aProviders.erase(aMapIt);
    }
}

//=========================================================================
// virtual
com::sun::star::uno::Sequence< ContentProviderInfo > SAL_CALL
                            UniversalContentBroker::queryContentProviders()
    throw( com::sun::star::uno::RuntimeException )
{
    // Return a list with information about active(!) content providers.

    osl::MutexGuard aGuard(m_aMutex);

    com::sun::star::uno::Sequence< ContentProviderInfo > aSeq(
                                                    m_aProviders.size() );
    ContentProviderInfo* pInfo = aSeq.getArray();

    ProviderMap_Impl::const_iterator end = m_aProviders.end();
    for (ProviderMap_Impl::const_iterator it(m_aProviders.begin()); it != end;
         ++it)
    {
        // Note: Active provider is always the first list element.
        pInfo->ContentProvider = it->getValue().front().getProvider();
        pInfo->Scheme = it->getRegexp();
        ++pInfo;
    }

    return aSeq;
}

//=========================================================================
// virtual
Reference< XContentProvider > SAL_CALL
        UniversalContentBroker::queryContentProvider( const OUString&
                                                          Identifier )
    throw( com::sun::star::uno::RuntimeException )
{
    return queryContentProvider( Identifier, sal_False );
}

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference< XContent > SAL_CALL UniversalContentBroker::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
    throw( IllegalIdentifierException, com::sun::star::uno::RuntimeException )
{
    //////////////////////////////////////////////////////////////////////
    // Let the content provider for the scheme given with the content
    // identifier create the XContent instance.
    //////////////////////////////////////////////////////////////////////

    if ( !Identifier.is() )
        return Reference< XContent >();

    Reference< XContentProvider > xProv =
        queryContentProvider( Identifier->getContentIdentifier(), sal_True );
    if ( xProv.is() )
        return  xProv->queryContent( Identifier );

    return Reference< XContent >();
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL UniversalContentBroker::compareContentIds(
                                const Reference< XContentIdentifier >& Id1,
                                const Reference< XContentIdentifier >& Id2 )
    throw( com::sun::star::uno::RuntimeException )
{
    OUString aURI1( Id1->getContentIdentifier() );
    OUString aURI2( Id2->getContentIdentifier() );

    Reference< XContentProvider > xProv1
                            = queryContentProvider( aURI1, sal_True );
    Reference< XContentProvider > xProv2
                            = queryContentProvider( aURI2, sal_True );

    // When both identifiers belong to the same provider, let that provider
    // compare them; otherwise, simply compare the URI strings (which must
    // be different):
    if ( xProv1.is() && ( xProv1 == xProv2 ) )
        return xProv1->compareContentIds( Id1, Id2 );
    else
        return aURI1.compareTo( aURI2 );
}

//=========================================================================
//
// XContentIdentifierFactory methods.
//
//=========================================================================

// virtual
Reference< XContentIdentifier > SAL_CALL
        UniversalContentBroker::createContentIdentifier(
                                            const OUString& ContentId )
    throw( com::sun::star::uno::RuntimeException )
{
    //////////////////////////////////////////////////////////////////////
    // Let the content provider for the scheme given with content
    // identifier create the XContentIdentifier instance, if he supports
    // the XContentIdentifierFactory interface. Otherwise create standard
    // implementation object for XContentIdentifier.
    //////////////////////////////////////////////////////////////////////

    Reference< XContentIdentifier > xIdentifier;

    Reference< XContentProvider > xProv
                            = queryContentProvider( ContentId, sal_True );
    if ( xProv.is() )
    {
        Reference< XContentIdentifierFactory > xFac( xProv, UNO_QUERY );
        if ( xFac.is() )
            xIdentifier = xFac->createContentIdentifier( ContentId );
    }

    if ( !xIdentifier.is() )
        xIdentifier = new ContentIdentifier( m_xSMgr, ContentId );

    return xIdentifier;
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL UniversalContentBroker::createCommandIdentifier()
    throw( RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}

//=========================================================================
// virtual
Any SAL_CALL UniversalContentBroker::execute(
                          const Command& aCommand,
                          sal_Int32,
                          const Reference< XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    //////////////////////////////////////////////////////////////////////
    // Note: Don't forget to adapt ucb_commands::CommandProcessorInfo
    //       ctor in ucbcmds.cxx when adding new commands!
    //////////////////////////////////////////////////////////////////////

    if ( ( aCommand.Handle == GETCOMMANDINFO_HANDLE ) ||
         aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( GETCOMMANDINFO_NAME ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getCommandInfo();
    }
    else if ( ( aCommand.Handle == GLOBALTRANSFER_HANDLE ) ||
              aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(GLOBALTRANSFER_NAME ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // globalTransfer
        //////////////////////////////////////////////////////////////////

        GlobalTransferCommandArgument aTransferArg;
        if ( !( aCommand.Argument >>= aTransferArg ) )
        {
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 ) ),
                Environment );
            // Unreachable
        }

        globalTransfer( aTransferArg, Environment );
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unknown command
        //////////////////////////////////////////////////////////////////

        ucbhelper::cancelCommandExecution(
            makeAny( UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL UniversalContentBroker::abort( sal_Int32 )
    throw( RuntimeException )
{
    // @@@ Not implemeted ( yet).
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

Reference< XContentProvider > UniversalContentBroker::queryContentProvider(
                                const OUString& Identifier,
                                sal_Bool bResolved )
{
    osl::MutexGuard aGuard( m_aMutex );

    ProviderList_Impl const * pList = m_aProviders.map( Identifier );
    return pList ? bResolved ? pList->front().getResolvedProvider()
                             : pList->front().getProvider()
                 : Reference< XContentProvider >();
}

//=========================================================================
//
// ProviderListEntry_Impl implementation.
//
//=========================================================================

Reference< XContentProvider > ProviderListEntry_Impl::resolveProvider() const
{
    if ( !m_xResolvedProvider.is() )
    {
        Reference< XContentProviderSupplier > xSupplier(
                                                    m_xProvider, UNO_QUERY );
        if ( xSupplier.is() )
            m_xResolvedProvider = xSupplier->getContentProvider();

        if ( !m_xResolvedProvider.is() )
            m_xResolvedProvider = m_xProvider;
    }

    return m_xResolvedProvider;
}

