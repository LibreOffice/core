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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument2.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include "identify.hxx"
#include "ucbcmds.hxx"

#include "ucb.hxx"

using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace ucb_impl;
using namespace com::sun::star;
using namespace ucbhelper;


namespace {

bool fillPlaceholders(OUString const & rInput,
                      uno::Sequence< uno::Any > const & rReplacements,
                      OUString * pOutput)
{
    sal_Unicode const * p = rInput.getStr();
    sal_Unicode const * pEnd = p + rInput.getLength();
    sal_Unicode const * pCopy = p;
    OUStringBuffer aBuffer;
    while (p != pEnd)
        switch (*p++)
        {
            case '&':
                if (pEnd - p >= 4
                    && p[0] == 'a' && p[1] == 'm' && p[2] == 'p'
                    && p[3] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append('&');
                    p += 4;
                    pCopy = p;
                }
                else if (pEnd - p >= 3
                         && p[0] == 'l' && p[1] == 't' && p[2] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append('<');
                    p += 3;
                    pCopy = p;
                }
                else if (pEnd - p >= 3
                         && p[0] == 'g' && p[1] == 't' && p[2] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append('>');
                    p += 3;
                    pCopy = p;
                }
                break;

            case '<':
                sal_Unicode const * q = p;
                while (q != pEnd && *q != '>')
                    ++q;
                if (q == pEnd)
                    break;
                OUString aKey(p, q - p);
                OUString aValue;
                bool bFound = false;
                for (sal_Int32 i = 2; i + 1 < rReplacements.getLength();
                     i += 2)
                {
                    OUString aReplaceKey;
                    if ((rReplacements[i] >>= aReplaceKey)
                        && aReplaceKey == aKey
                        && (rReplacements[i + 1] >>= aValue))
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                    return false;
                aBuffer.append(pCopy, p - 1 - pCopy);
                aBuffer.append(aValue);
                p = q + 1;
                pCopy = p;
                break;
        }
    aBuffer.append(pCopy, pEnd - pCopy);
    *pOutput = aBuffer.makeStringAndClear();
    return true;
}

void makeAndAppendXMLName(
                OUStringBuffer & rBuffer, const OUString & rIn )
{
    sal_Int32 nCount = rIn.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = rIn[ n ];
        switch ( c )
        {
            case '&':
                rBuffer.append( "&amp;" );
                break;

            case '"':
                rBuffer.append( "&quot;" );
                break;

            case '\'':
                rBuffer.append( "&apos;" );
                break;

            case '<':
                rBuffer.append( "&lt;" );
                break;

            case '>':
                rBuffer.append( "&gt;" );
                break;

            default:
                rBuffer.append( c );
                break;
        }
    }
}

bool createContentProviderData(
    const OUString & rProvider,
    const uno::Reference< container::XHierarchicalNameAccess >& rxHierNameAccess,
    ContentProviderData & rInfo)
{
    // Obtain service name.
    OUStringBuffer aKeyBuffer (rProvider);
    aKeyBuffer.append( "/ServiceName" );

    OUString aValue;
    try
    {
        if ( !( rxHierNameAccess->getByHierarchicalName(
                    aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
        {
            OSL_FAIL( "UniversalContentBroker::getContentProviderData - "
                        "Error getting item value!" );
        }
    }
    catch (const container::NoSuchElementException&)
    {
        return false;
    }

    rInfo.ServiceName = aValue;

    // Obtain URL Template.
    aKeyBuffer.append(rProvider);
    aKeyBuffer.append( "/URLTemplate" );

    if ( !( rxHierNameAccess->getByHierarchicalName(
                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
    {
        OSL_FAIL( "UniversalContentBroker::getContentProviderData - "
                    "Error getting item value!" );
    }

    rInfo.URLTemplate = aValue;

    // Obtain Arguments.
    aKeyBuffer.append(rProvider);
    aKeyBuffer.append( "/Arguments" );

    if ( !( rxHierNameAccess->getByHierarchicalName(
                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
    {
        OSL_FAIL( "UniversalContentBroker::getContentProviderData - "
                    "Error getting item value!" );
    }

    rInfo.Arguments = aValue;
    return true;
}

}


// UniversalContentBroker Implementation.


UniversalContentBroker::UniversalContentBroker(
    const Reference< css::uno::XComponentContext >& xContext )
: m_xContext( xContext ),
  m_pDisposeEventListeners( nullptr ),
  m_nCommandId( 0 )
{
    OSL_ENSURE( m_xContext.is(),
                "UniversalContentBroker ctor: No service manager" );
}


// virtual
UniversalContentBroker::~UniversalContentBroker()
{
    delete m_pDisposeEventListeners;
}


// XInterface methods.
void SAL_CALL UniversalContentBroker::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL UniversalContentBroker::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL UniversalContentBroker::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XUniversalContentBroker* >(this)),
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XComponent* >(this)),
                                               (static_cast< XServiceInfo* >(this)),
                                               (static_cast< XInitialization* >(this)),
                                               (static_cast< XContentProviderManager* >(this)),
                                               (static_cast< XContentProvider* >(this)),
                                               (static_cast< XContentIdentifierFactory* >(this)),
                                               (static_cast< XCommandProcessor* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_9( UniversalContentBroker,
                      XUniversalContentBroker,
                      XTypeProvider,
                      XComponent,
                      XServiceInfo,
                      XInitialization,
                      XContentProviderManager,
                      XContentProvider,
                      XContentIdentifierFactory,
                      XCommandProcessor );


// XComponent methods.


// virtual
void SAL_CALL UniversalContentBroker::dispose()
    throw( css::uno::RuntimeException, std::exception )
{
    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = (static_cast< XComponent* >(this));
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

     if ( m_xNotifier.is() )
        m_xNotifier->removeChangesListener( this );
}


// virtual
void SAL_CALL UniversalContentBroker::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( css::uno::RuntimeException, std::exception )
{
    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners = new OInterfaceContainerHelper2( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL UniversalContentBroker::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( css::uno::RuntimeException, std::exception )
{
    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );

    // Note: Don't want to delete empty container here -> performance.
}


// XServiceInfo methods.


XSERVICEINFO_IMPL_1_CTX( UniversalContentBroker,
                     OUString( "com.sun.star.comp.ucb.UniversalContentBroker" ),
                     UCB_SERVICE_NAME );


// Service factory implementation.


ONE_INSTANCE_SERVICE_FACTORY_IMPL( UniversalContentBroker );


// XInitialization methods.


// virtual
void SAL_CALL UniversalContentBroker::initialize( const css::uno::Sequence< Any >& aArguments )
    throw( css::uno::Exception,
           css::uno::RuntimeException, std::exception )
{
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_aArguments.getLength() != 0)
        {
            if (aArguments.getLength() != 0
                && !(m_aArguments.getLength() == 2
                     && aArguments.getLength() == 2
                     && m_aArguments[0] == aArguments[0]
                     && m_aArguments[1] == aArguments[1]))
            {
                throw IllegalArgumentException(
                    "UCB reinitialized with different arguments",
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
            return;
        }
        if (aArguments.getLength() == 0)
        {
            m_aArguments.realloc(2);
            m_aArguments[0] <<= OUString("Local");
            m_aArguments[1] <<= OUString("Office");
        }
        else
        {
            m_aArguments = aArguments;
        }
    }
    configureUcb();
}


// XContentProviderManager methods.


// virtual
Reference< XContentProvider > SAL_CALL
UniversalContentBroker::registerContentProvider(
                            const Reference< XContentProvider >& Provider,
                            const OUString& Scheme,
                            sal_Bool ReplaceExisting )
    throw( DuplicateProviderException, css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);

    ProviderMap_Impl::iterator aIt;
    try
    {
        aIt = m_aProviders.find(Scheme);
    }
    catch (const IllegalArgumentException&)
    {
        return nullptr; //@@@
    }

    Reference< XContentProvider > xPrevious;
    if (aIt == m_aProviders.end())
    {
        ProviderList_Impl aList;
        aList.push_front( ProviderListEntry_Impl(Provider) );
        try
        {
            m_aProviders.add(Scheme, aList);
        }
        catch (const IllegalArgumentException&)
        {
            return nullptr; //@@@
        }
    }
    else
    {
        if (!ReplaceExisting)
            throw DuplicateProviderException();

        ProviderList_Impl & rList = aIt->getValue();
        xPrevious = rList.front().getProvider();
        rList.push_front( ProviderListEntry_Impl(Provider) );
    }

    return xPrevious;
}


// virtual
void SAL_CALL UniversalContentBroker::deregisterContentProvider(
                              const Reference< XContentProvider >& Provider,
                            const OUString& Scheme )
    throw( css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);

    ProviderMap_Impl::iterator aMapIt;
    try
    {
        aMapIt = m_aProviders.find(Scheme);
    }
    catch (const IllegalArgumentException&)
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


// virtual
css::uno::Sequence< ContentProviderInfo > SAL_CALL
                            UniversalContentBroker::queryContentProviders()
    throw( css::uno::RuntimeException, std::exception )
{
    // Return a list with information about active(!) content providers.

    osl::MutexGuard aGuard(m_aMutex);

    css::uno::Sequence< ContentProviderInfo > aSeq( m_aProviders.size() );
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


// virtual
Reference< XContentProvider > SAL_CALL
        UniversalContentBroker::queryContentProvider( const OUString&
                                                          Identifier )
    throw( css::uno::RuntimeException, std::exception )
{
    return queryContentProvider( Identifier, false );
}


// XContentProvider methods.


// virtual
Reference< XContent > SAL_CALL UniversalContentBroker::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
    throw( IllegalIdentifierException, css::uno::RuntimeException, std::exception )
{

    // Let the content provider for the scheme given with the content
    // identifier create the XContent instance.


    if ( !Identifier.is() )
        return Reference< XContent >();

    Reference< XContentProvider > xProv =
        queryContentProvider( Identifier->getContentIdentifier(), true );
    if ( xProv.is() )
        return  xProv->queryContent( Identifier );

    return Reference< XContent >();
}


// virtual
sal_Int32 SAL_CALL UniversalContentBroker::compareContentIds(
                                const Reference< XContentIdentifier >& Id1,
                                const Reference< XContentIdentifier >& Id2 )
    throw( css::uno::RuntimeException, std::exception )
{
    OUString aURI1( Id1->getContentIdentifier() );
    OUString aURI2( Id2->getContentIdentifier() );

    Reference< XContentProvider > xProv1
                            = queryContentProvider( aURI1, true );
    Reference< XContentProvider > xProv2
                            = queryContentProvider( aURI2, true );

    // When both identifiers belong to the same provider, let that provider
    // compare them; otherwise, simply compare the URI strings (which must
    // be different):
    if ( xProv1.is() && ( xProv1 == xProv2 ) )
        return xProv1->compareContentIds( Id1, Id2 );
    else
        return aURI1.compareTo( aURI2 );
}


// XContentIdentifierFactory methods.


// virtual
Reference< XContentIdentifier > SAL_CALL
        UniversalContentBroker::createContentIdentifier(
                                            const OUString& ContentId )
    throw( css::uno::RuntimeException, std::exception )
{

    // Let the content provider for the scheme given with content
    // identifier create the XContentIdentifier instance, if he supports
    // the XContentIdentifierFactory interface. Otherwise create standard
    // implementation object for XContentIdentifier.


    Reference< XContentIdentifier > xIdentifier;

    Reference< XContentProvider > xProv
                            = queryContentProvider( ContentId, true );
    if ( xProv.is() )
    {
        Reference< XContentIdentifierFactory > xFac( xProv, UNO_QUERY );
        if ( xFac.is() )
            xIdentifier = xFac->createContentIdentifier( ContentId );
    }

    if ( !xIdentifier.is() )
        xIdentifier = new ContentIdentifier( ContentId );

    return xIdentifier;
}


// XCommandProcessor methods.


// virtual
sal_Int32 SAL_CALL UniversalContentBroker::createCommandIdentifier()
    throw( RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}


// virtual
Any SAL_CALL UniversalContentBroker::execute(
                          const Command& aCommand,
                          sal_Int32,
                          const Reference< XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException, std::exception )
{
    Any aRet;


    // Note: Don't forget to adapt ucb_commands::CommandProcessorInfo
    //       ctor in ucbcmds.cxx when adding new commands!


    if ( ( aCommand.Handle == GETCOMMANDINFO_HANDLE ) || aCommand.Name == GETCOMMANDINFO_NAME )
    {

        // getCommandInfo


        aRet <<= getCommandInfo();
    }
    else if ( ( aCommand.Handle == GLOBALTRANSFER_HANDLE ) || aCommand.Name == GLOBALTRANSFER_NAME )
    {

        // globalTransfer


        GlobalTransferCommandArgument2 aTransferArg;
        if ( !( aCommand.Argument >>= aTransferArg ) )
        {
            GlobalTransferCommandArgument aArg;
            if ( !( aCommand.Argument >>= aArg ) )
            {
                ucbhelper::cancelCommandExecution(
                    makeAny( IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                    Environment );
                // Unreachable
            }

            // Copy infos into the new structure
            aTransferArg.Operation = aArg.Operation;
            aTransferArg.SourceURL = aArg.SourceURL;
            aTransferArg.TargetURL = aArg.TargetURL;
            aTransferArg.NewTitle = aArg.NewTitle;
            aTransferArg.NameClash = aArg.NameClash;
        }

        globalTransfer( aTransferArg, Environment );
    }
    else if ( ( aCommand.Handle == CHECKIN_HANDLE ) || aCommand.Name == CHECKIN_NAME )
    {
        ucb::CheckinArgument aCheckinArg;
        if ( !( aCommand.Argument >>= aCheckinArg ) )
        {
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 ) ),
                Environment );
            // Unreachable
        }
        aRet <<= checkIn( aCheckinArg, Environment );
    }
    else
    {

        // Unknown command


        ucbhelper::cancelCommandExecution(
            makeAny( UnsupportedCommandException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}


// XCommandProcessor2 methods.


// virtual
void SAL_CALL UniversalContentBroker::releaseCommandIdentifier(sal_Int32 /*aCommandId*/)
    throw( RuntimeException, std::exception )
{
    // @@@ Not implemeted ( yet).
}


// virtual
void SAL_CALL UniversalContentBroker::abort( sal_Int32 )
    throw( RuntimeException, std::exception )
{
    // @@@ Not implemeted ( yet).
}


// XChangesListener methods


// virtual
void SAL_CALL UniversalContentBroker::changesOccurred( const util::ChangesEvent& Event )
        throw( uno::RuntimeException, std::exception )
{
    sal_Int32 nCount = Event.Changes.getLength();
    if ( nCount )
    {
        uno::Reference< container::XHierarchicalNameAccess > xHierNameAccess;
        Event.Base >>= xHierNameAccess;

        OSL_ASSERT( xHierNameAccess.is() );

        const util::ElementChange* pElementChanges
            = Event.Changes.getConstArray();

        ContentProviderDataList aData;
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const util::ElementChange& rElem = pElementChanges[ n ];
            OUString aKey;
            rElem.Accessor >>= aKey;

            ContentProviderData aInfo;

            // Removal of UCPs from the configuration leads to changesOccurred
            // notifications, too, but it is hard to tell for a given
            // ElementChange whether it is an addition or a removal, so as a
            // heuristic consider as removals those that cause a
            // NoSuchElementException in createContentProviderData.

            // For now, removal of UCPs from the configuration is simply ignored
            // (and not reflected in the UCB's data structures):
            if (createContentProviderData(aKey, xHierNameAccess, aInfo))
            {
                aData.push_back(aInfo);
            }
        }

        prepareAndRegister(aData);
    }
}


// XEventListener methods


// virtual
void SAL_CALL UniversalContentBroker::disposing(const lang::EventObject&)
    throw( uno::RuntimeException, std::exception )
{
    if ( m_xNotifier.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( m_xNotifier.is() )
            m_xNotifier.clear();
    }
}


// Non-interface methods


Reference< XContentProvider > UniversalContentBroker::queryContentProvider(
                                const OUString& Identifier,
                                bool bResolved )
{
    osl::MutexGuard aGuard( m_aMutex );

    ProviderList_Impl const * pList = m_aProviders.map( Identifier );
    return pList ? bResolved ? pList->front().getResolvedProvider()
                             : pList->front().getProvider()
                 : Reference< XContentProvider >();
}

void UniversalContentBroker::configureUcb()
    throw (uno::RuntimeException)
{
    OUString aKey1;
    OUString aKey2;
    if (m_aArguments.getLength() < 2
        || !(m_aArguments[0] >>= aKey1) || !(m_aArguments[1] >>= aKey2))
    {
        OSL_FAIL("UniversalContentBroker::configureUcb(): Bad arguments");
        return;
    }

    ContentProviderDataList aData;
    if (!getContentProviderData(aKey1, aKey2, aData))
    {
        OSL_TRACE("UniversalContentBroker::configureUcb(): No configuration");
        return;
    }

    prepareAndRegister(aData);
}

void UniversalContentBroker::prepareAndRegister(
    const ContentProviderDataList& rData)
{
    ContentProviderDataList::const_iterator aEnd(rData.end());
    for (ContentProviderDataList::const_iterator aIt(rData.begin());
         aIt != aEnd; ++aIt)
    {
        OUString aProviderArguments;
        if (fillPlaceholders(aIt->Arguments,
                             m_aArguments,
                             &aProviderArguments))
        {
            registerAtUcb(this,
                          m_xContext,
                          aIt->ServiceName,
                          aProviderArguments,
                          aIt->URLTemplate);

        }
        else
            OSL_FAIL("UniversalContentBroker::prepareAndRegister(): Bad argument placeholders");
    }
}


bool UniversalContentBroker::getContentProviderData(
            const OUString & rKey1,
            const OUString & rKey2,
            ContentProviderDataList & rListToFill )
{
    if ( !m_xContext.is() || rKey1.isEmpty() || rKey2.isEmpty() )
    {
        OSL_FAIL( "UniversalContentBroker::getContentProviderData - Invalid argument!" );
        return false;
    }

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProv =
                configuration::theDefaultProvider::get( m_xContext );

        OUStringBuffer aFullPath;
        aFullPath.append(
                "/org.openoffice.ucb.Configuration/ContentProviders"
                "/['" );
        makeAndAppendXMLName( aFullPath, rKey1 );
        aFullPath.append( "']/SecondaryKeys/['" );
        makeAndAppendXMLName( aFullPath, rKey2 );
        aFullPath.append( "']/ProviderData" );

        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue      aProperty;
        aProperty.Name = "nodepath";
        aProperty.Value <<= aFullPath.makeStringAndClear();
        aArguments[ 0 ] <<= aProperty;

        uno::Reference< uno::XInterface > xInterface(
                xConfigProv->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess",
                    aArguments ) );

        if ( !m_xNotifier.is() )
        {
            m_xNotifier.set( xInterface, uno::UNO_QUERY_THROW );

            m_xNotifier->addChangesListener( this );
        }

        uno::Reference< container::XNameAccess > xNameAccess(
                                            xInterface, uno::UNO_QUERY_THROW );

        uno::Sequence< OUString > aElems = xNameAccess->getElementNames();
        const OUString* pElems = aElems.getConstArray();
        sal_Int32 nCount = aElems.getLength();

        if ( nCount > 0 )
        {
            uno::Reference< container::XHierarchicalNameAccess >
                                xHierNameAccess( xInterface, uno::UNO_QUERY_THROW );

            // Iterate over children.
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {

                try
                {

                    ContentProviderData aInfo;

                    OUStringBuffer aElemBuffer;
                    aElemBuffer.append( "['" );
                    makeAndAppendXMLName( aElemBuffer, pElems[ n ] );
                    aElemBuffer.append( "']" );

                    OSL_VERIFY(
                        createContentProviderData(
                            aElemBuffer.makeStringAndClear(), xHierNameAccess,
                            aInfo));

                    rListToFill.push_back( aInfo );
                }
                catch (const container::NoSuchElementException&)
                {
                    // getByHierarchicalName
                    OSL_FAIL( "UniversalContentBroker::getContentProviderData - "
                                "caught NoSuchElementException!" );
                }
            }
        }
    }
    catch (const uno::RuntimeException&)
    {
        OSL_TRACE( "UniversalContentBroker::getContentProviderData - caught RuntimeException!" );
        return false;
    }
    catch (const uno::Exception&)
    {
        // createInstance, createInstanceWithArguments

        OSL_TRACE( "UniversalContentBroker::getContentProviderData - caught Exception!" );
        return false;
    }

    return true;
}


// ProviderListEntry_Impl implementation.


Reference< XContentProvider > const & ProviderListEntry_Impl::resolveProvider() const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
