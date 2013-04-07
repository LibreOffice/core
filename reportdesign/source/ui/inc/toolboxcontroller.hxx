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
#ifndef RPTUI_TOOLBOXCONTROLLER_HXX
#define RPTUI_TOOLBOXCONTROLLER_HXX

#include <svtools/toolboxcontroller.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/implementationreference.hxx>

class SfxToolBoxControl;
namespace rptui
{
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::lang::XServiceInfo,
                                    ::com::sun::star::frame::XSubToolbarController> TToolboxController_BASE;
    typedef ::comphelper::ImplementationReference<SfxToolBoxControl,::com::sun::star::frame::XToolbarController> TToolbarHelper;

    class OToolboxController : public ::svt::ToolboxController
                              ,public TToolboxController_BASE
    {
        DECLARE_STL_USTRINGACCESS_MAP(sal_Bool,TCommandState);
        TCommandState   m_aStates;
        TToolbarHelper  m_pToolbarController;
        sal_uInt16      m_nToolBoxId;
        sal_uInt16      m_nSlotId;
        OToolboxController(const OToolboxController&);
        void operator =(const OToolboxController&);
    public:
        OToolboxController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        virtual ~OToolboxController();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
        virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
        // XToolbarController
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);

        // XSubToolbarController
        virtual ::sal_Bool SAL_CALL opensSubToolbar(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getSubToolbarName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL functionSelected( const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateImage(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);
    };
//..........................................................................
} // rptui
//..........................................................................
#endif //RPTUI_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
