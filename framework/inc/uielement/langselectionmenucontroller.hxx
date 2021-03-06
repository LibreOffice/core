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

#include <svtools/popupmenucontrollerbase.hxx>
#include <rtl/ustring.hxx>

#include <helper/mischelper.hxx>

namespace framework
{
    class LanguageSelectionMenuController final : public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            LanguageSelectionMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~LanguageSelectionMenuController() override;

            /* interface XServiceInfo */
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() override;

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

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

            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu, const Mode rMode );
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
