/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#include "ProviderCache.hxx"
#include <memory>
#include <mutex>

namespace func_provider
{

 typedef ::cppu::WeakImplHelper<
     css::script::provider::XScriptProvider,
     css::script::browse::XBrowseNode, css::lang::XServiceInfo,
     css::lang::XInitialization > t_helper;

class MasterScriptProvider :
            public t_helper
{
public:
    /// @throws cpo::uno::RuntimeException
    explicit MasterScriptProvider(
        const cpo::uno::Reference< cpo::uno::XComponentContext >
        & xContext );
    virtual ~MasterScriptProvider() override;

    // XServiceInfo implementation
    virtual OUString getImplementationName( ) override;

    // XBrowseNode implementation
    virtual OUString getName() override;
    virtual cpo::uno::Sequence< cpo::uno::Reference< css::script::browse::XBrowseNode > > getChildNodes() override;
    virtual bool hasChildNodes() override;
    virtual sal_Int16 getType() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames( ) override;

    // XScriptProvider implementation
    virtual cpo::uno::Reference < css::script::provider::XScript >
        getScript( const OUString& scriptURI ) override;

    /**
     *  XInitialise implementation
     *
     * @param args expected to contain a single OUString
     * containing the URI
     */
    virtual void initialize( const cpo::uno::Sequence < cpo::uno::Any > & args ) override;

    // returns context string for this provider, eg
    const OUString& getContextString() const { return m_sCtxString; }

private:
    static OUString parseLocationName( const OUString& location );

    ProviderCache* providerCache();
    /* to obtain other services if needed */
    cpo::uno::Reference< cpo::uno::XComponentContext >              m_xContext;
    cpo::uno::Reference< css::lang::XMultiComponentFactory >        m_xMgr;
    cpo::uno::Reference< css::frame::XModel >                       m_xModel;
    cpo::uno::Reference< css::document::XScriptInvocationContext >  m_xInvocationContext;
    cpo::uno::Sequence< cpo::uno::Any >                             m_sAargs;
    OUString                                                 m_sNodeName;

    // This component supports XInitialization, it can be created
    // using createInstanceXXX() or createInstanceWithArgumentsXXX using
    // the service Manager.
    // Need to detect proper initialisation and validity
    // for the object, so m_bIsValid indicates that the object is valid is set in ctor
    // in case of createInstanceWithArgumentsXXX() called m_bIsValid is set to reset
    // and then set to true when initialisation is complete
    bool m_bIsValid;
    // m_bInitialised ensure initialisation only takes place once.
    bool m_bInitialised;
    std::unique_ptr<ProviderCache> m_pPCache;
    std::mutex m_mutex;
    OUString m_sCtxString;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
