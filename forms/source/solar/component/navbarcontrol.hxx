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

#include <formnavigation.hxx>

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>


namespace frm
{

    typedef ::cppu::ImplHelper1 <   css::frame::XDispatchProviderInterception
                                >   ONavigationBarControl_Base;

    class ONavigationBarControl
                            :public UnoControl
                            ,public ONavigationBarControl_Base
    {
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
    public:
        explicit ONavigationBarControl(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

    protected:
        virtual ~ONavigationBarControl() override;

        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarControl, UnoControl )
        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

        // XControl
        virtual void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& _rParent ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) override;

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) override;
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) override;
    };

    class ONavigationBarPeer final
                        :public VCLXWindow
                        ,public OFormNavigationHelper
    {
    public:
        /** factory method
        */
        static rtl::Reference<ONavigationBarPeer> Create(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            vcl::Window* _pParentWindow,
            const css::uno::Reference< css::awt::XControlModel >& _rxModel
        );

        // XInterface
        DECLARE_XINTERFACE( )

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) override;

        // XWindow2
        using VCLXWindow::isEnabled;

    private:
        explicit ONavigationBarPeer(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );
        virtual ~ONavigationBarPeer() override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        void SAL_CALL dispose(  ) override;

        // XVclWindowPeer
        void SAL_CALL setProperty( const OUString& _rPropertyName, const css::uno::Any& _rValue ) override;
        css::uno::Any SAL_CALL getProperty( const OUString& _rPropertyName ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // OFormNavigationHelper overriables
        virtual void    interceptorsChanged( ) override;
        virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled ) override;
        virtual void    allFeatureStatesChanged( ) override;
        virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) override;

        // IFeatureDispatcher overriables
        virtual bool    isEnabled( sal_Int16 _nFeatureId ) const override;
    };


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
