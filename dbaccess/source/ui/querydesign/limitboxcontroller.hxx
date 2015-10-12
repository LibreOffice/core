/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_LIMITBOXCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_LIMITBOXCONTROLLER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <svtools/toolboxcontroller.hxx>
#include <rtl/ustring.hxx>
#include <vcl/vclptr.hxx>

#include "apitools.hxx"

namespace dbaui
{

class LimitBoxImpl;

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
        virtual ~LimitBoxController();

        /// XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        /// XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

        /// XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;

        /// XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        /// XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw (css::uno::RuntimeException, std::exception) override;

        void dispatchCommand( const css::uno::Sequence< css::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        VclPtr<LimitBoxImpl> m_pLimitBox;
};

} ///dbaui namespace

#endif /// INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_LIMITBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
