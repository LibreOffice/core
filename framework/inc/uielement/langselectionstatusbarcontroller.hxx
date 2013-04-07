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

#ifndef __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <svtools/statusbarcontroller.hxx>
#include <rtl/ustring.hxx>

#include "helper/mischelper.hxx"

#include <set>

class SvtLanguageTable;


// component helper namespace
namespace framework {

class LangSelectionStatusbarController : public svt::StatusbarController
{
    public:
        explicit LangSelectionStatusbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        // XServiceInfo
        DECLARE_XSERVICEINFO

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusbarController
        virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       ::sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nItemId, ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);

    private:
        virtual ~LangSelectionStatusbarController() {}
        LangSelectionStatusbarController(LangSelectionStatusbarController &); // not defined
        void operator =(LangSelectionStatusbarController &); // not defined


        sal_Bool            m_bShowMenu;        // if the menu is to be displayed or not (depending on the selected object/text)
        sal_Int16           m_nScriptType;      // the flags for the different script types available in the selection, LATIN = 0x0001, ASIAN = 0x0002, COMPLEX = 0x0004
        OUString     m_aCurLang;         // the language of the current selection, "*" if there are more than one languages
        OUString     m_aKeyboardLang;    // the keyboard language
        OUString     m_aGuessedTextLang;     // the 'guessed' language for the selection, "" if none could be guessed
        LanguageGuessingHelper      m_aLangGuessHelper;

        void LangMenu() throw (::com::sun::star::uno::RuntimeException);
};

} // framework namespace

#endif // __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
