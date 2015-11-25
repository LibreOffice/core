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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_MACROSMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_MACROSMENUCONTROLLER_HXX

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/menu.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
    class MacrosMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        struct ExecuteInfo
        {
            css::uno::Reference< css::frame::XDispatch >     xDispatch;
            css::util::URL                                   aTargetURL;
         };
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::Reference< css::frame::XDispatchProvider >  m_xDispatchProvider;
        css::uno::Reference< css::container::XNameAccess >    m_xUICommandLabels;

        public:
            MacrosMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~MacrosMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        private:
            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu );
            OUString RetrieveLabelFromCommand(const OUString& rCmdURL);
            void addScriptItems( PopupMenu* pPopupMenu, sal_uInt16 startItemId );
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_MACROSMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
