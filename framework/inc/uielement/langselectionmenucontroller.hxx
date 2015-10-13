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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_LANGSELECTIONMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_LANGSELECTIONMENUCONTROLLER_HXX

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
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

#include "helper/mischelper.hxx"

namespace framework
{
    class LanguageSelectionMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            LanguageSelectionMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~LanguageSelectionMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() throw (css::uno::RuntimeException, std::exception) override;

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        private:
            virtual void impl_setPopupMenu() override;
            enum Mode
            {
                MODE_SetLanguageSelectionMenu,
                MODE_SetLanguageParagraphMenu,
                MODE_SetLanguageAllTextMenu
            };

            bool                                             m_bShowMenu;
            OUString                                         m_aLangStatusCommandURL;
            css::uno::Reference< css::frame::XDispatch >     m_xLanguageDispatch;
            OUString                                         m_aMenuCommandURL_Lang;
            css::uno::Reference< css::frame::XDispatch >     m_xMenuDispatch_Lang;
            OUString                                         m_aMenuCommandURL_Font;
            css::uno::Reference< css::frame::XDispatch >     m_xMenuDispatch_Font;
            OUString                                         m_aMenuCommandURL_CharDlgForParagraph;
            css::uno::Reference< css::frame::XDispatch >     m_xMenuDispatch_CharDlgForParagraph;
            OUString                                         m_aCurLang;
            SvtScriptType                                    m_nScriptType;
            OUString                                         m_aKeyboardLang;
            OUString                                         m_aGuessedTextLang;
            LanguageGuessingHelper                           m_aLangGuessHelper;

            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu, const Mode rMode );
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_LANGSELECTIONMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
