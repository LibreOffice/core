/*************************************************************************
 *
 *  $RCSfile: ucb.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kso $ $Date: 2001-04-05 09:49:55 $
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

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_GLOBALTRANSFERCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERSUPPLIER_HPP_
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#endif
#ifndef _UCBHELPER_CONFIGUREUCB_HXX_
#include <ucbhelper/configureucb.hxx>
#endif

#ifndef _IDENTIFY_HXX
#include "identify.hxx"
#endif
#ifndef _UCBCMDS_HXX
#include "ucbcmds.hxx"
#endif

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
    VOS_ENSURE( m_xSMgr.is(),
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
        ::ucb::configureUcb(this, m_xSMgr, aArguments, 0);
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
        UniversalContentBroker::queryContentProvider( const OUString& Scheme )
    throw( com::sun::star::uno::RuntimeException )
{
    return queryContentProvider( Scheme, sal_False );
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
                          sal_Int32 CommandId,
                          const Reference< XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    //////////////////////////////////////////////////////////////////////
    // Note: Don't forget to adapt ucb_commands::CommandProcessorInfo
    //       ctor in ucbcmds.cxx when adding new commands!
    //////////////////////////////////////////////////////////////////////

    if ( ( aCommand.Handle == GETCOMMANDINFO_HANDLE ) ||
         ( aCommand.Name.compareToAscii( GETCOMMANDINFO_NAME ) == 0 ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getCommandInfo();
    }
    else if ( ( aCommand.Handle == GLOBALTRANSFER_HANDLE ) ||
              ( aCommand.Name.compareToAscii( GLOBALTRANSFER_NAME ) == 0 ) )
    {
        //////////////////////////////////////////////////////////////////
        // globalTransfer
        //////////////////////////////////////////////////////////////////

        GlobalTransferCommandArgument aTransferArg;
        if ( aCommand.Argument >>= aTransferArg )
        {
            globalTransfer( aTransferArg, Environment );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "UniversalContentBroker::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unknown command
        //////////////////////////////////////////////////////////////////

        VOS_ENSURE( sal_False,
                    "UniversalContentBroker::execute - unknown command!" );
        throw CommandAbortedException();
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL UniversalContentBroker::abort( sal_Int32 CommandId )
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
                                const OUString& Scheme, sal_Bool bResolved )
{
    osl::MutexGuard aGuard( m_aMutex );

    ProviderList_Impl const * pList = m_aProviders.map( Scheme );
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

