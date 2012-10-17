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

#ifndef _UCB_HXX
#define _UCB_HXX


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
//=========================================================================

#define UCB_SERVICE_NAME "com.sun.star.ucb.UniversalContentBroker"

//=========================================================================

namespace cppu { class OInterfaceContainerHelper; }

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandInfo;
    struct GlobalTransferCommandArgument2;
} } } }

class UniversalContentBroker :
                public cppu::OWeakObject,
                public com::sun::star::ucb::XUniversalContentBroker,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XInitialization,
                public com::sun::star::util::XChangesListener
{
public:
    UniversalContentBroker( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UniversalContentBroker();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL
    initialize( const com::sun::star::uno::Sequence<
                        com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception,
               com::sun::star::uno::RuntimeException );

    // XContentProviderManager
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > SAL_CALL
    registerContentProvider( const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XContentProvider >&
                                    Provider,
                             const rtl::OUString& Scheme,
                             sal_Bool ReplaceExisting )
        throw( com::sun::star::ucb::DuplicateProviderException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    deregisterContentProvider( const com::sun::star::uno::Reference<
                                    com::sun::star::ucb::XContentProvider >&
                                        Provider,
                               const rtl::OUString& Scheme )
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentProviderInfo > SAL_CALL
    queryContentProviders()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > SAL_CALL
    queryContentProvider( const rtl::OUString& Identifier )
        throw( com::sun::star::uno::RuntimeException );

    // XContentProvider
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( com::sun::star::ucb::IllegalIdentifierException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    compareContentIds( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& Id1,
                       const com::sun::star::uno::Reference<
                           com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( com::sun::star::uno::RuntimeException );

    // XContentIdentifierFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentIdentifier > SAL_CALL
    createContentIdentifier( const rtl::OUString& ContentId )
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor2
    virtual void SAL_CALL releaseCommandIdentifier(sal_Int32 aCommandId)
        throw( com::sun::star::uno::RuntimeException );

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const com::sun::star::util::ChangesEvent& Event )
        throw( com::sun::star::uno::RuntimeException );

     // XEventListener ( base of XChangesLisetenr )
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
    queryContentProvider( const rtl::OUString& Identifier,
                          sal_Bool bResolved );

    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo >
    getCommandInfo();

    void
    globalTransfer(
            const com::sun::star::ucb::GlobalTransferCommandArgument2 & rArg,
            const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( com::sun::star::uno::Exception );


    bool configureUcb()
        throw ( com::sun::star::uno::RuntimeException);

    bool getContentProviderData(
            const rtl::OUString & rKey1,
            const rtl::OUString & rKey2,
            ucbhelper::ContentProviderDataList & rListToFill);

    void prepareAndRegister( const ucbhelper::ContentProviderDataList& rData);

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;

    com::sun::star::uno::Reference<
        com::sun::star::util::XChangesNotifier > m_xNotifier;

    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_aArguments;
    ProviderMap_Impl m_aProviders;
    osl::Mutex m_aMutex;
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    oslInterlockedCount m_nInitCount; //@@@ see initialize() method
    sal_Int32 m_nCommandId;
};

#endif /* !_UCB_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
