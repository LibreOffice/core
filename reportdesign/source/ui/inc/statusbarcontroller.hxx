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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STATUSBARCONTROLLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STATUSBARCONTROLLER_HXX

#include <svtools/statusbarcontroller.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>

namespace rptui
{
    typedef ::cppu::ImplHelper1 <   css::lang::XServiceInfo> OStatusbarController_BASE;
    class OStatusbarController : public ::svt::StatusbarController,
                                 public OStatusbarController_BASE
    {
        css::uno::Reference< css::frame::XStatusbarController >  m_rController;
        sal_uInt16      m_nSlotId;
        sal_uInt16      m_nId;
    public:
        OStatusbarController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

    private:
        void SAL_CALL dispose() override;
        // XInterface
        DECLARE_XINTERFACE( )
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;
        // need by registration

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XUpdatable
        virtual void SAL_CALL update() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XStatusbarController
        virtual sal_Bool SAL_CALL mouseButtonDown( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual sal_Bool SAL_CALL mouseMove( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual sal_Bool SAL_CALL mouseButtonUp( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual void SAL_CALL command( const css::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       sal_Bool bMouseEvent,
                                       const css::uno::Any& aData ) override;
        virtual void SAL_CALL paint( const css::uno::Reference< css::awt::XGraphics >& xGraphics,
                                     const css::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) override;
        virtual void SAL_CALL click( const css::awt::Point& aPos ) override;
        virtual void SAL_CALL doubleClick( const css::awt::Point& aPos ) override;
    };
}
#endif // DBAUI_STATUSBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
