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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_HEADERMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_HEADERMENUCONTROLLER_HXX

#include <com/sun/star/frame/XModel.hpp>

#include <svtools/popupmenucontrollerbase.hxx>

namespace framework
{
    class HeaderMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            HeaderMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext, bool _bFooter = false );
            virtual ~HeaderMenuController() override;

            /* interface XServiceInfo */
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        private:
            void fillPopupMenu( const css::uno::Reference< css::frame::XModel >& rModel, css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );

            css::uno::Reference< css::frame::XModel > m_xModel;
            bool                                      m_bFooter;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_HEADERMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
