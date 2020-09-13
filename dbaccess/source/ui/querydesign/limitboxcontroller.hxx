/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <connectivity/CommonTools.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <rtl/ustring.hxx>
#include <vcl/vclptr.hxx>

namespace dbaui
{

class LimitBox;

/**
 * A ToolboxController to paste LimitBox onto the Query Design Toolbar
 * It is communicating with querycontroller and this channel make enable
 * to set\get the value of limitbox when switching between views
 */
class LimitBoxController: public svt::ToolboxController,
                          public css::lang::XServiceInfo
{
    public:
        explicit LimitBoxController(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~LimitBoxController() override;

        /// XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        /// XServiceInfo
        DECLARE_SERVICE_INFO();

        /// XComponent
        virtual void SAL_CALL dispose() override;

        /// XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        /// XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;
        virtual void SAL_CALL click() override;
        virtual void SAL_CALL doubleClick() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

        void dispatchCommand( const css::uno::Sequence< css::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        VclPtr<LimitBox> m_xLimitBox;
};

} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
