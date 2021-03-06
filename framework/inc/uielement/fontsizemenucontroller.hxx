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

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <rtl/ustring.hxx>
#include <tools/long.hxx>

#include <memory>

namespace framework
{
    class FontSizeMenuController final : public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            FontSizeMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~FontSizeMenuController() override;

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
            virtual void impl_setPopupMenu() override;
            void setCurHeight( tools::Long nHeight, css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
            OUString retrievePrinterName( css::uno::Reference< css::frame::XFrame > const & rFrame );

            std::unique_ptr<tools::Long[]>                   m_pHeightArray;
            css::awt::FontDescriptor                         m_aFontDescriptor;
            css::frame::status::FontHeight                   m_aFontHeight;
            css::uno::Reference< css::frame::XDispatch >     m_xCurrentFontDispatch;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
