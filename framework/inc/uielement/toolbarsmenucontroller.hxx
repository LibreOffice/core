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

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <rtl/ustring.hxx>
#include <unotools/intlwrapper.hxx>

#include <vector>

namespace framework
{
    class ToolbarsMenuController final : public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            ToolbarsMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~ToolbarsMenuController() override;

            /* interface XServiceInfo */
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const css::uno::Reference< css::awt::XPopupMenu >& PopupMenu ) override;

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

            // XMenuListener
            virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;
            virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

            struct ExecuteInfo
            {
                css::uno::Reference< css::frame::XDispatch >     xDispatch;
                css::util::URL                                                aTargetURL;
                css::uno::Sequence< css::beans::PropertyValue >  aArgs;
            };

            DECL_STATIC_LINK( ToolbarsMenuController, ExecuteHdl_Impl, void*, void );

        private:
            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
            css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > getLayoutManagerToolbars( const css::uno::Reference< css::frame::XLayoutManager >& rLayoutManager );
            css::uno::Reference< css::frame::XDispatch > getDispatchFromCommandURL( const OUString& rCommandURL );
            void addCommand( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu, const OUString& rCommandURL, const OUString& aLabel );

            css::uno::Reference< css::uno::XComponentContext >        m_xContext;
            css::uno::Reference< css::container::XNameAccess >        m_xPersistentWindowState;
            css::uno::Reference< css::ui::XUIConfigurationManager >   m_xModuleCfgMgr;
            css::uno::Reference< css::ui::XUIConfigurationManager >   m_xDocCfgMgr;
            bool                                                      m_bResetActive;
            std::vector< OUString >                                   m_aCommandVector;
            IntlWrapper                                               m_aIntlWrapper;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
