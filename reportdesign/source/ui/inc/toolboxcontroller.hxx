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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX

#include <sal/config.h>

#include <map>

#include <rtl/ref.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <cppuhelper/implbase2.hxx>

class SvxColorToolBoxControl;
namespace rptui
{
    typedef ::cppu::ImplHelper2 <   css::lang::XServiceInfo,
                                    css::frame::XSubToolbarController> TToolboxController_BASE;
    typedef rtl::Reference<SvxColorToolBoxControl> TToolbarHelper;

    class OToolboxController : public ::svt::ToolboxController
                              ,public TToolboxController_BASE
    {
        typedef std::map<OUString, sal_Bool> TCommandState;
        TCommandState   m_aStates;
        TToolbarHelper  m_pToolbarController;
        OToolboxController(const OToolboxController&) = delete;
        void operator =(const OToolboxController&) = delete;
    public:
        OToolboxController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
        virtual ~OToolboxController() override;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;
        // need by registration
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static();
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;
        // XToolbarController
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

        // XSubToolbarController
        virtual sal_Bool SAL_CALL opensSubToolbar(  ) override;
        virtual OUString SAL_CALL getSubToolbarName(  ) override;
        virtual void SAL_CALL functionSelected( const OUString& aCommand ) override;
        virtual void SAL_CALL updateImage(  ) override;
    };

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
