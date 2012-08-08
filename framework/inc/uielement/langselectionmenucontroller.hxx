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

#ifndef __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_

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
            LanguageSelectionMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~LanguageSelectionMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        private:
            virtual void impl_setPopupMenu();
            virtual void impl_select(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aURL);
            enum Mode
            {
                MODE_SetLanguageSelectionMenu,
                MODE_SetLanguageParagraphMenu,
                MODE_SetLanguageAllTextMenu
            };

            sal_Bool                                                               m_bShowMenu;
            ::rtl::OUString                                                        m_aLangStatusCommandURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xLanguageDispatch;
            ::rtl::OUString                                                        m_aMenuCommandURL_Lang;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_Lang;
            ::rtl::OUString                                                        m_aMenuCommandURL_Font;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_Font;
            ::rtl::OUString                                                        m_aMenuCommandURL_CharDlgForParagraph;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_CharDlgForParagraph;
            ::rtl::OUString     m_aCurLang;
            sal_Int16           m_nScriptType;
            ::rtl::OUString     m_aKeyboardLang;
            ::rtl::OUString     m_aGuessedTextLang;
            LanguageGuessingHelper      m_aLangGuessHelper;

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu, const Mode rMode );
    };
}

#endif // __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
