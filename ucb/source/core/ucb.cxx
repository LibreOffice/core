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
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/DuplicateProviderException.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument2.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/macros.hxx>
#include <tools/diagnose_ex.h>
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

    OUString aValue;
    try
    {
        if ( !( rxHierNameAccess->getByHierarchicalName(
                    rProvider +  "/ServiceName" ) >>= aValue ) )
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

    if ( !( rxHierNameAccess->getByHierarchicalName(
                rProvider + "/URLTemplate" ) >>= aValue ) )
    {
        OSL_FAIL( "UniversalContentBroker::getContentProviderData - "
                    "Error getting item value!" );
    }

    rInfo.URLTemplate = aValue;

    // Obtain Arguments.

    if ( !( rxHierNameAccess->getByHierarchicalName(
                rProvider + "/Arguments" ) >>= aValue ) )
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
  m_nCommandId( 0 )
{
    OSL_ENSURE( m_xContext.is(),
                "UniversalContentBroker ctor: No service manager" );
}


// virtual
UniversalContentBroker::~UniversalContentBroker()
{
}


// XComponent methods.


// virtual
void SAL_CALL UniversalContentBroker::dispose()
{
    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent* >(this);
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_xNotifier.is() )
        m_xNotifier->removeChangesListener( this );
}


// virtual
void SAL_CALL UniversalContentBroker::addEventListener(
                            const Reference< XEventListener >& Listener )
{
    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners.reset( new OInterfaceContainerHelper2( m_aMutex ) );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL UniversalContentBroker::removeEventListener(
                            const Reference< XEventListener >& Listener )
{
    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );

    // Note: Don't want to delete empty container here -> performance.
}


// XServiceInfo methods.

OUString SAL_CALL UniversalContentBroker::getImplementationName()
{
    return "com.sun.star.comp.ucb.UniversalContentBroker";
}
sal_Bool SAL_CALL UniversalContentBroker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL UniversalContentBroker::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.UniversalContentBroker" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_UniversalContentBroker_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new UniversalContentBroker(context)));
}


// XInitialization methods.


// virtual
void SAL_CALL UniversalContentBroker::initialize( const css::uno::Sequence< Any >& aArguments )
{
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_aArguments.hasElements())
        {
            if (aArguments.hasElements()
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
        if (!aArguments.hasElements())
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

        auto aListIt = std::find_if(rList.begin(), rList.end(),
            [&Provider](const ProviderListEntry_Impl& rEntry) { return rEntry.getProvider() == Provider; });
        if (aListIt != rList.end())
            rList.erase(aListIt);

        if (rList.empty())
            m_aProviders.erase(aMapIt);
    }
}


// virtual
css::uno::Sequence< ContentProviderInfo > SAL_CALL
                            UniversalContentBroker::queryContentProviders()
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
{
    return queryContentProvider( Identifier, false );
}


// XContentProvider methods.


// virtual
Reference< XContent > SAL_CALL UniversalContentBroker::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
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
                                    "Wrong argument type!",
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
                                "Wrong argument type!",
                                static_cast< cppu::OWeakObject * >( this ),
                                -1 ) ),
                Environment );
            // Unreachable
        }
        aRet = checkIn( aCheckinArg, Environment );
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
{
    // @@@ Not implemented ( yet).
}


// virtual
void SAL_CALL UniversalContentBroker::abort( sal_Int32 )
{
    // @@@ Not implemented ( yet).
}


// XChangesListener methods


// virtual
void SAL_CALL UniversalContentBroker::changesOccurred( const util::ChangesEvent& Event )
{
    if ( !Event.Changes.hasElements() )
        return;

    uno::Reference< container::XHierarchicalNameAccess > xHierNameAccess;
    Event.Base >>= xHierNameAccess;

    OSL_ASSERT( xHierNameAccess.is() );

    ContentProviderDataList aData;
    for ( const util::ElementChange& rElem : Event.Changes )
    {
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


// XEventListener methods


// virtual
void SAL_CALL UniversalContentBroker::disposing(const lang::EventObject&)
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
        SAL_WARN( "ucb", "No configuration");
        return;
    }

    prepareAndRegister(aData);
}

void UniversalContentBroker::prepareAndRegister(
    const ContentProviderDataList& rData)
{
    for (const auto& rContentProviderData : rData)
    {
        OUString aProviderArguments;
        if (fillPlaceholders(rContentProviderData.Arguments,
                             m_aArguments,
                             &aProviderArguments))
        {
            registerAtUcb(this,
                          m_xContext,
                          rContentProviderData.ServiceName,
                          aProviderArguments,
                          rContentProviderData.URLTemplate);

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

        OUStringBuffer aFullPath(128);
        aFullPath.append(
                "/org.openoffice.ucb.Configuration/ContentProviders"
                "/['" );
        makeAndAppendXMLName( aFullPath, rKey1 );
        aFullPath.append( "']/SecondaryKeys/['" );
        makeAndAppendXMLName( aFullPath, rKey2 );
        aFullPath.append( "']/ProviderData" );

        uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(aFullPath.makeStringAndClear())}
        }));

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

        const uno::Sequence< OUString > aElems = xNameAccess->getElementNames();

        if ( aElems.hasElements() )
        {
            uno::Reference< container::XHierarchicalNameAccess >
                                xHierNameAccess( xInterface, uno::UNO_QUERY_THROW );

            // Iterate over children.
            for ( const auto& rElem : aElems )
            {

                try
                {

                    ContentProviderData aInfo;

                    OUStringBuffer aElemBuffer;
                    aElemBuffer.append( "['" );
                    makeAndAppendXMLName( aElemBuffer, rElem );
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
        TOOLS_WARN_EXCEPTION( "ucb", "" );
        return false;
    }
    catch (const uno::Exception&)
    {
        // createInstance, createInstanceWithArguments

        TOOLS_WARN_EXCEPTION( "ucb", "" );
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
