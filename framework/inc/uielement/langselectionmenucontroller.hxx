/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionmenucontroller.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <helper/popupmenucontrollerbase.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
    class LanguageSelectionMenuController :  public PopupMenuControllerBase
    {
        public:
            LanguageSelectionMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~LanguageSelectionMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL highlight( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL deactivate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        private:
            enum Mode
            {
                MODE_SetLanguageSelectionMenu,
                MODE_SetLanguageParagraphMenu,
                MODE_SetLanguageAllTextMenu
            };

            sal_Bool                                                               m_bShowMenu;
            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing >    m_xLanguageGuesser;
            ::rtl::OUString                                                        m_aLangStatusCommandURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xLanguageDispatch;
            ::rtl::OUString                                                        m_aMenuCommandURL_Lang;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_Lang;
            ::rtl::OUString                                                        m_aMenuCommandURL_Font;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_Font;
            ::rtl::OUString                                                        m_aMenuCommandURL_CharDlgForParagraph;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xMenuDispatch_CharDlgForParagraph;
            ::rtl::OUString     m_aCurrentLanguage;
            ::rtl::OUString     m_aCurLang;
            sal_Int16           m_nScriptType;
            ::rtl::OUString     m_aKeyboardLang;
            ::rtl::OUString     m_aGuessedText;

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu, const Mode rMode );
    };
}

#endif // __FRAMEWORK_UIELEMENT_LANGUAGESELECTIONMENUCONTROLLER_HXX_
