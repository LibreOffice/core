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

#ifndef INCLUDED_FORMS_SOURCE_SOLAR_COMPONENT_NAVBARCONTROL_HXX
#define INCLUDED_FORMS_SOURCE_SOLAR_COMPONENT_NAVBARCONTROL_HXX

#include "formnavigation.hxx"

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/wintypes.hxx>


namespace frm
{

    typedef ::cppu::ImplHelper  <   css::frame::XDispatchProviderInterception
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
        virtual ~ONavigationBarControl();

    public:
        // XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarControl, UnoControl )
        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XControl
        virtual void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& _rParent ) throw( css::uno::RuntimeException, std::exception ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()  throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( css::uno::RuntimeException, std::exception ) override;

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
    };

    class ONavigationBarPeer
                        :public VCLXWindow
                        ,public OFormNavigationHelper
    {
    public:
        /** factory method
            @return
                a new ONavigationBarPeer instance, which has been acquired once!
        */
        static ONavigationBarPeer* Create(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            vcl::Window* _pParentWindow,
            const css::uno::Reference< css::awt::XControlModel >& _rxModel
        );

    protected:
        explicit ONavigationBarPeer(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );
        virtual ~ONavigationBarPeer();

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( css::uno::RuntimeException, std::exception ) override;

        // XWindow2
        using VCLXWindow::isEnabled;

    protected:
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        void SAL_CALL dispose(  ) throw( css::uno::RuntimeException, std::exception ) override;

        // XVclWindowPeer
        void SAL_CALL setProperty( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw( css::uno::RuntimeException, std::exception ) override;
        css::uno::Any SAL_CALL getProperty( const OUString& _rPropertyName ) throw(css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

        // OFormNavigationHelper overriables
        virtual void    interceptorsChanged( ) override;
        virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled ) override;
        virtual void    allFeatureStatesChanged( ) override;
        virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) override;

        // IFeatureDispatcher overriables
        virtual bool    isEnabled( sal_Int16 _nFeatureId ) const override;
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_SOLAR_COMPONENT_NAVBARCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
