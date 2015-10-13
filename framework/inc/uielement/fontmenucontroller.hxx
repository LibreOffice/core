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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_FONTMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_FONTMENUCONTROLLER_HXX

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
    class FontMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            FontMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~FontMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() throw (css::uno::RuntimeException, std::exception) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

            // XMenuListener
            virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        private:
            virtual void impl_setPopupMenu() override;
            void fillPopupMenu( const css::uno::Sequence< OUString >& rFontNameSeq, css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu );

            OUString                                        m_aFontFamilyName;
            css::uno::Reference< css::frame::XDispatch >    m_xFontListDispatch;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_FONTMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
