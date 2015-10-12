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

#ifndef INCLUDED_SCRIPTING_SOURCE_DLGPROV_DLGPROV_HXX
#define INCLUDED_SCRIPTING_SOURCE_DLGPROV_DLGPROV_HXX

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XDialogProvider2.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/awt/XUnoControlDialog.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <memory>


namespace dlgprov
{



    // mutex


    ::osl::Mutex& getMutex();



    // class DialogProviderImpl

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > lcl_createControlModel(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_xContext);
    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager > lcl_getStringResourceManager(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_xContext,const OUString& i_sURL);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > lcl_createDialogModel(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_xContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInput,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel,
                const ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager >& xStringResourceManager,
                const ::com::sun::star::uno::Any &aDialogSourceURL) throw ( ::com::sun::star::uno::Exception );

    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::awt::XDialogProvider2,
        ::com::sun::star::awt::XContainerWindowProvider > DialogProviderImpl_BASE;

    class DialogProviderImpl : public DialogProviderImpl_BASE
    {
    private:
        struct BasicRTLParams
        {
             ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > mxInput;
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxDlgLib;
             ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > mxBasicRTLListener;
        };
        std::unique_ptr< BasicRTLParams > m_BasicInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;

    OUString msDialogLibName;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > createDialogModel( const OUString& sURL );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XUnoControlDialog > createDialogControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxDialogModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent );

        void attachControlEvents( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxControlContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxHandler,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > inspectHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxHandler );
    // helper methods
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createDialogModel(
                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInput,
                const ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager >& xStringResourceManager,
                const ::com::sun::star::uno::Any &aDialogSourceURL) throw ( ::com::sun::star::uno::Exception );
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > createDialogModelForBasic() throw ( ::com::sun::star::uno::Exception );

        // XDialogProvider / XDialogProvider2 impl method
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XControl > SAL_CALL createDialogImpl(
            const OUString& URL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
            bool bDialogProviderMode )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    public:
        DialogProviderImpl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~DialogProviderImpl();

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

        // XDialogProvider
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDialog > SAL_CALL createDialog(
            const OUString& URL )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XDialogProvider2
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDialog > SAL_CALL createDialogWithHandler(
            const OUString& URL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDialog > SAL_CALL createDialogWithArguments(
            const OUString& URL,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createContainerWindow(
            const OUString& URL, const OUString& WindowType,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
     };


}   // namespace dlgprov



// component helper namespace
namespace comp_DialogModelProvider {

// component and service helper functions:
OUString SAL_CALL _getImplementationName();
css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();

} // namespace comp_DialogModelProvider




#endif // INCLUDED_SCRIPTING_SOURCE_DLGPROV_DLGPROV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
