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

#ifndef INCLUDED_SCRIPTING_SOURCE_BASPROV_BASPROV_HXX
#define INCLUDED_SCRIPTING_SOURCE_BASPROV_BASPROV_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

class BasicManager;



namespace basprov
{



    //  class BasicProviderImpl


    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::script::provider::XScriptProvider,
        ::com::sun::star::script::browse::XBrowseNode > BasicProviderImpl_BASE;


    class BasicProviderImpl : public BasicProviderImpl_BASE
    {
    private:
        BasicManager*   m_pAppBasicManager;
        BasicManager*   m_pDocBasicManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >             m_xLibContainerApp;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >             m_xLibContainerDoc;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >                m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XScriptInvocationContext >    m_xInvocationContext;
        OUString  m_sScriptingContext;
        bool m_bIsAppScriptCtx;
        bool m_bIsUserCtx;

        bool isLibraryShared(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& rxLibContainer,
            const OUString& rLibName );

    public:
        BasicProviderImpl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
        virtual ~BasicProviderImpl();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XScriptProvider
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::script::provider::XScript > SAL_CALL getScript(
            const OUString& scriptURI )
            throw (  ::com::sun::star::script::provider::ScriptFrameworkErrorException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XBrowseNode
        virtual OUString SAL_CALL getName(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getType(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}   // namespace basprov


#endif // INCLUDED_SCRIPTING_SOURCE_BASPROV_BASPROV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
