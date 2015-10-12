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

#ifndef INCLUDED_SCRIPTING_SOURCE_PROVIDER_MASTERSCRIPTPROVIDER_HXX
#define INCLUDED_SCRIPTING_SOURCE_PROVIDER_MASTERSCRIPTPROVIDER_HXX

#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#include "ProviderCache.hxx"

namespace func_provider
{

 typedef ::cppu::WeakImplHelper<
     css::script::provider::XScriptProvider,
     css::script::browse::XBrowseNode, css::lang::XServiceInfo,
     css::lang::XInitialization,
     css::container::XNameContainer > t_helper;

class MasterScriptProvider :
            public t_helper
{
public:
    MasterScriptProvider(
        const css::uno::Reference< css::uno::XComponentContext >
        & xContext ) throw( css::uno::RuntimeException );
    virtual ~MasterScriptProvider();

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName( )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XBrowseNode implementation
    virtual OUString SAL_CALL getName()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::script::browse::XBrowseNode > > SAL_CALL getChildNodes()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasChildNodes()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL getType()
        throw ( css::uno::RuntimeException, std::exception ) override;
    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) throw ( css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw ( css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw ( css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw ( css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw ( css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XScriptProvider implementation
    virtual css::uno::Reference < css::script::provider::XScript > SAL_CALL
        getScript( const OUString& scriptURI )
        throw( css::script::provider::ScriptFrameworkErrorException,
               css::uno::RuntimeException, std::exception ) override;

    /**
     *  XInitialise implementation
     *
     * @param args expected to contain a single OUString
     * containing the URI
     */
    virtual void SAL_CALL initialize( const css::uno::Sequence < css::uno::Any > & args )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // Public method to return all Language Providers in this MasterScriptProviders
    // context.
    css::uno::Sequence< css::uno::Reference< css::script::provider::XScriptProvider > > SAL_CALL
        getAllProviders() throw ( css::uno::RuntimeException );

    bool isPkgProvider() { return m_bIsPkgMSP; }
    css::uno::Reference< css::script::provider::XScriptProvider > getPkgProvider() { return m_xMSPPkg; }
    // returns context string for this provider, eg
    OUString getContextString() { return m_sCtxString; }

private:
    static OUString parseLocationName( const OUString& location );
    void  createPkgProvider();
    bool  isValid() { return m_bIsValid;}

    ProviderCache* providerCache();
    /* to obtain other services if needed */
    css::uno::Reference< css::uno::XComponentContext >              m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory >        m_xMgr;
    css::uno::Reference< css::frame::XModel >                       m_xModel;
    css::uno::Reference< css::document::XScriptInvocationContext >  m_xInvocationContext;
    css::uno::Sequence< css::uno::Any >                             m_sAargs;
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
    bool m_bIsPkgMSP;
    css::uno::Reference< css::script::provider::XScriptProvider > m_xMSPPkg;
    ProviderCache* m_pPCache;
    osl::Mutex m_mutex;
    OUString m_sCtxString;
};

OUString SAL_CALL mspf_getImplementationName() ;
css::uno::Reference< css::uno::XInterface > SAL_CALL mspf_create( css::uno::Reference< css::uno::XComponentContext > const & xComponentContext );
css::uno::Sequence< OUString > SAL_CALL mspf_getSupportedServiceNames();

}

namespace browsenodefactory
{
OUString SAL_CALL bnf_getImplementationName() ;
css::uno::Reference< css::uno::XInterface > SAL_CALL bnf_create( css::uno::Reference< css::uno::XComponentContext > const & xComponentContext );
css::uno::Sequence< OUString > SAL_CALL bnf_getSupportedServiceNames();
}

#endif // INCLUDED_SCRIPTING_SOURCE_PROVIDER_MASTERSCRIPTPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
