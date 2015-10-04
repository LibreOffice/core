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
#include <cppuhelper/implbase.hxx>

class SfxStatusBarControl;
namespace rptui
{
    typedef ::cppu::ImplHelper < css::lang::XServiceInfo > OStatusbarController_BASE;
    class OStatusbarController : public ::svt::StatusbarController,
                                 public OStatusbarController_BASE
    {
        css::uno::Reference< css::frame::XStatusbarController >  m_rController;
        sal_uInt16      m_nSlotId;
        sal_uInt16      m_nId;
    public:
        OStatusbarController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

    private:
        void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
        // XInterface
        DECLARE_XINTERFACE( )
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
        // need by registration

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XStatusbarController
        virtual sal_Bool SAL_CALL mouseButtonDown( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseMove( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseButtonUp( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL command( const css::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       sal_Bool bMouseEvent,
                                       const css::uno::Any& aData ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL paint( const css::uno::Reference< css::awt::XGraphics >& xGraphics,
                                     const css::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception) override;
    };
}
#endif // DBAUI_STATUSBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
