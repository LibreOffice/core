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

#pragma once


#include <com/sun/star/ucb/CheckinArgument.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include "providermap.hxx"
#include <ucbhelper/registerucb.hxx>

#include <memory>


namespace comphelper { class OInterfaceContainerHelper2; }

namespace com::sun::star::ucb {
    class XCommandInfo;
    struct GlobalTransferCommandArgument2;
}

class UniversalContentBroker :
                public cppu::WeakImplHelper<
                    css::ucb::XUniversalContentBroker,
                    css::lang::XServiceInfo,
                    css::lang::XInitialization,
                    css::util::XChangesListener>
{
public:
    explicit UniversalContentBroker( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~UniversalContentBroker() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL
    dispose() override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference<
                            css::lang::XEventListener >& Listener ) override;

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XContentProviderManager
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    registerContentProvider( const css::uno::Reference< css::ucb::XContentProvider >&  Provider,
                             const OUString& Scheme,
                             sal_Bool ReplaceExisting ) override;
    virtual void SAL_CALL
    deregisterContentProvider( const css::uno::Reference< css::ucb::XContentProvider >&  Provider,
                               const OUString& Scheme ) override;
    virtual css::uno::Sequence< css::ucb::ContentProviderInfo > SAL_CALL
    queryContentProviders() override;
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    queryContentProvider( const OUString& Identifier ) override;

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;
    virtual sal_Int32 SAL_CALL
    compareContentIds( const css::uno::Reference< css::ucb::XContentIdentifier >& Id1,
                       const css::uno::Reference< css::ucb::XContentIdentifier >& Id2 ) override;

    // XContentIdentifierFactory
    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    createContentIdentifier( const OUString& ContentId ) override;

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier() override;
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId ) override;

    // XCommandProcessor2
    virtual void SAL_CALL releaseCommandIdentifier(sal_Int32 aCommandId) override;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event ) override;

     // XEventListener ( base of XChangesLisetenr )
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

private:
    css::uno::Reference< css::ucb::XContentProvider >
    queryContentProvider( const OUString& Identifier,
                          bool bResolved );

    static css::uno::Reference< css::ucb::XCommandInfo >
    getCommandInfo();

    /// @throws css::uno::Exception
    void
    globalTransfer(
            const css::ucb::GlobalTransferCommandArgument2 & rArg,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Any checkIn( const css::ucb::CheckinArgument& rArg,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );


    /// @throws css::uno::RuntimeException
    void configureUcb();

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
    std::unique_ptr<comphelper::OInterfaceContainerHelper2> m_pDisposeEventListeners;
    sal_Int32 m_nCommandId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
