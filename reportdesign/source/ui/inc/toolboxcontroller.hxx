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

class SfxToolBoxControl;
namespace rptui
{
    typedef ::cppu::ImplHelper2 <   css::lang::XServiceInfo,
                                    css::frame::XSubToolbarController> TToolboxController_BASE;
    typedef rtl::Reference<SfxToolBoxControl> TToolbarHelper;

    class OToolboxController : public ::svt::ToolboxController
                              ,public TToolboxController_BASE
    {
        typedef std::map<OUString, sal_Bool> TCommandState;
        TCommandState   m_aStates;
        TToolbarHelper  m_pToolbarController;
        sal_uInt16      m_nToolBoxId;
        sal_uInt16      m_nSlotId;
        OToolboxController(const OToolboxController&) = delete;
        void operator =(const OToolboxController&) = delete;
    public:
        OToolboxController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
        virtual ~OToolboxController();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
        // need by registration
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
        // XToolbarController
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;

        // XSubToolbarController
        virtual sal_Bool SAL_CALL opensSubToolbar(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSubToolbarName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL functionSelected( const OUString& aCommand ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateImage(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw (css::uno::RuntimeException, std::exception) override;
    };

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
