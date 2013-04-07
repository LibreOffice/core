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

#ifndef FORMS_NAVBARCONTROL_HXX
#define FORMS_NAVBARCONTROL_HXX

#include "formnavigation.hxx"

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <tools/wintypes.hxx>

//.........................................................................
namespace frm
{
//.........................................................................

    //==================================================================
    // ONavigationBarControl
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProviderInterception
                                >   ONavigationBarControl_Base;

    class ONavigationBarControl
                            :public UnoControl
                            ,public ONavigationBarControl_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    public:
        ONavigationBarControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

    protected:
        ~ONavigationBarControl();

    public:
        // XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarControl, UnoControl );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw ( ::com::sun::star::uno::RuntimeException );

        // XControl
        virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rParent ) throw( ::com::sun::star::uno::RuntimeException );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( ::com::sun::star::uno::RuntimeException );

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
    };

    //==================================================================
    // ONavigationBarPeer
    //==================================================================
    class ONavigationBarPeer
                        :public VCLXWindow
                        ,public OFormNavigationHelper
    {
    public:
        /** factory method
            @return
                a new ONavigationBarPeer instance, which has been aquired once!
        */
        static ONavigationBarPeer* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pParentWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel
        );

    protected:
        ONavigationBarPeer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );
        ~ONavigationBarPeer();

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( ::com::sun::star::uno::RuntimeException );

        // XWindow2
        using VCLXWindow::isEnabled;

    protected:
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        void SAL_CALL dispose(  ) throw( ::com::sun::star::uno::RuntimeException );

        // XVclWindowPeer
        void SAL_CALL setProperty( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw( ::com::sun::star::uno::RuntimeException );
        ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& _rPropertyName ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // OFormNavigationHelper overriables
        virtual void    interceptorsChanged( );
        virtual void    featureStateChanged( sal_Int16 _nFeatureId, sal_Bool _bEnabled );
        virtual void    allFeatureStatesChanged( );
        virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds );

        // IFeatureDispatcher overriables
        virtual bool    isEnabled( sal_Int16 _nFeatureId ) const;
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_NAVBARCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
