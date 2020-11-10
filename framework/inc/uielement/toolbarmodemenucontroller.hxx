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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMODEMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMODEMENUCONTROLLER_HXX

#include <svtools/popupmenucontrollerbase.hxx>

namespace framework
{
    class ToolbarModeMenuController final : public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            ToolbarModeMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~ToolbarModeMenuController() override;

            /* interface XServiceInfo */
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const css::uno::Reference< css::awt::XPopupMenu >& PopupMenu ) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

            // XMenuListener
            virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;
            virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        private:
            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );

            css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMODEMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
