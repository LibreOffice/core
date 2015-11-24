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

#ifndef INCLUDED_UCB_SOURCE_CORE_UCB_HXX
#define INCLUDED_UCB_SOURCE_CORE_UCB_HXX


#include <com/sun/star/ucb/CheckinArgument.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <ucbhelper/macros.hxx>
#include "providermap.hxx"
#include <ucbhelper/registerucb.hxx>

#include <vector>


#define UCB_SERVICE_NAME "com.sun.star.ucb.UniversalContentBroker"



namespace cppu { class OInterfaceContainerHelper; }

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandInfo;
    struct GlobalTransferCommandArgument2;
} } } }

class UniversalContentBroker :
                public cppu::OWeakObject,
                public css::ucb::XUniversalContentBroker,
                public css::lang::XTypeProvider,
                public css::lang::XServiceInfo,
                public css::lang::XInitialization,
                public css::util::XChangesListener
{
public:
    explicit UniversalContentBroker( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~UniversalContentBroker();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference<
                            css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException, std::exception ) override;

    // XContentProviderManager
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    registerContentProvider( const css::uno::Reference< css::ucb::XContentProvider >&  Provider,
                             const OUString& Scheme,
                             sal_Bool ReplaceExisting )
        throw( css::ucb::DuplicateProviderException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    deregisterContentProvider( const css::uno::Reference< css::ucb::XContentProvider >&  Provider,
                               const OUString& Scheme )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::ucb::ContentProviderInfo > SAL_CALL
    queryContentProviders()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    queryContentProvider( const OUString& Identifier )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
        throw( css::ucb::IllegalIdentifierException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    compareContentIds( const css::uno::Reference< css::ucb::XContentIdentifier >& Id1,
                       const css::uno::Reference< css::ucb::XContentIdentifier >& Id2 )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XContentIdentifierFactory
    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    createContentIdentifier( const OUString& ContentId )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XCommandProcessor2
    virtual void SAL_CALL releaseCommandIdentifier(sal_Int32 aCommandId)
        throw( css::uno::RuntimeException, std::exception ) override;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event )
        throw( css::uno::RuntimeException, std::exception ) override;

     // XEventListener ( base of XChangesLisetenr )
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException, std::exception ) override;

private:
    css::uno::Reference< css::ucb::XContentProvider >
    queryContentProvider( const OUString& Identifier,
                          bool bResolved );

    static css::uno::Reference< css::ucb::XCommandInfo >
    getCommandInfo();

    void
    globalTransfer(
            const css::ucb::GlobalTransferCommandArgument2 & rArg,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw( css::uno::Exception, std::exception );

    css::uno::Any checkIn( const css::ucb::CheckinArgument& rArg,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv ) throw( css::uno::Exception );


    bool configureUcb()
        throw ( css::uno::RuntimeException);

    bool getContentProviderData(
            const OUString & rKey1,
            const OUString & rKey2,
            ucbhelper::ContentProviderDataList & rListToFill);

    void prepareAndRegister( const ucbhelper::ContentProviderDataList& rData);

    css::uno::Reference<
        css::uno::XComponentContext > m_xContext;

    css::uno::Reference<
        css::util::XChangesNotifier > m_xNotifier;

    css::uno::Sequence< css::uno::Any > m_aArguments;
    ProviderMap_Impl m_aProviders;
    osl::Mutex m_aMutex;
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    sal_Int32 m_nCommandId;
};

#endif // INCLUDED_UCB_SOURCE_CORE_UCB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
