/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <uielement/langselectionstatusbarcontroller.hxx>
#include <classes/fwkresid.hxx>
#include <services.h>
#include <classes/resource.hrc>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/status.hxx>
#include <toolkit/unohlp.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/PopupMenu.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <svtools/langtab.hxx>
#include "sal/types.h"
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <i18npool/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <classes/fwkresid.hxx>
#include <classes/resource.hrc>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/unohlp.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Command.hpp>
#include <svl/languageoptions.hxx>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

#include "helper/mischelper.hxx"

#include <map>
#include <set>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::document;

using ::rtl::OUString;


namespace framework
{


DEFINE_XSERVICEINFO_MULTISERVICE        (   LangSelectionStatusbarController            ,
                                            OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLER         ,
                                            IMPLEMENTATIONNAME_LANGSELECTIONSTATUSBARCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   LangSelectionStatusbarController, {} )

LangSelectionStatusbarController::LangSelectionStatusbarController( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    svt::StatusbarController( xServiceManager, uno::Reference< frame::XFrame >(), OUString(), 0 ),
    m_bShowMenu( sal_True ),
    m_nScriptType( LS_SCRIPT_LATIN | LS_SCRIPT_ASIAN | LS_SCRIPT_COMPLEX ),
    m_aLangGuessHelper( xServiceManager )
{
}

// XInterface
Any SAL_CALL LangSelectionStatusbarController::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    return svt::StatusbarController::queryInterface( rType );
}

void SAL_CALL LangSelectionStatusbarController::acquire() throw ()
{
    svt::StatusbarController::acquire();
}

void SAL_CALL LangSelectionStatusbarController::release() throw ()
{
    svt::StatusbarController::release();
}

void SAL_CALL LangSelectionStatusbarController::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::initialize" );
    SolarMutexGuard aSolarMutexGuard;

    svt::StatusbarController::initialize( aArguments );

    if ( m_xParentWindow.is() && m_nID > 0 )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
        if ( pWindow && ( pWindow->GetType() == WINDOW_STATUSBAR ))
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            pStatusBar->SetItemText( m_nID, FwkResId( STR_LANGSTATUS_MULTIPLE_LANGUAGES ) );
        }
    }
}

// XComponent
void SAL_CALL LangSelectionStatusbarController::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::dispose" );
    svt::StatusbarController::dispose();
}

// XEventListener
void SAL_CALL LangSelectionStatusbarController::disposing( const com::sun::star::lang::EventObject& Source )
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::disposing" );
    svt::StatusbarController::disposing( Source );
}

// XStatusbarController
::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseButtonDown(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::mouseButtonDown" );
    return sal_False;
}

::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseMove(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::mouseMove" );
    return sal_False;
}

::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseButtonUp(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::mouseButtonUp" );
    return sal_False;
}

void LangSelectionStatusbarController::LangMenu()
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::LangMenu" );
    if (!m_bShowMenu)
        return;

    //add context menu
    Reference< awt::XPopupMenu > xPopupMenu( awt::PopupMenu::create( comphelper::getComponentContext(m_xServiceManager) ) );
    //sub menu that contains all items except the last two items: Separator + Set Language for Paragraph
    Reference< awt::XPopupMenu > subPopupMenu( awt::PopupMenu::create( comphelper::getComponentContext(m_xServiceManager) ) );

    SvtLanguageTable    aLanguageTable;

    // get languages to be displayed in the menu
    std::set< OUString > aLangItems;
    FillLangItems( aLangItems, aLanguageTable, m_xFrame, m_aLangGuessHelper,
            m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedTextLang );

    // add first few entries to main menu
    sal_Int16 nItemId = static_cast< sal_Int16 >(MID_LANG_SEL_1);
    const OUString sAsterix(RTL_CONSTASCII_USTRINGPARAM("*"));  // multiple languages in current selection
    const OUString sEmpty;  // 'no language found' from language guessing
    std::map< sal_Int16, OUString > aLangMap;
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if ( rStr != OUString( aLanguageTable.GetString( LANGUAGE_NONE ) ) &&
             rStr != sAsterix &&
             rStr != sEmpty)
        {
            DBG_ASSERT( MID_LANG_SEL_1 <= nItemId && nItemId <= MID_LANG_SEL_9,
                    "nItemId outside of expected range!" );
            xPopupMenu->insertItem( nItemId, rStr, css::awt::MenuItemStyle::RADIOCHECK, nItemId );
            if ( rStr == m_aCurLang )
            {
                //make a sign for the current language
                xPopupMenu->checkItem( nItemId, sal_True );
            }
            aLangMap[ nItemId ] = rStr;
            ++nItemId;
        }
    }
    xPopupMenu->insertItem( MID_LANG_SEL_NONE,  String( FwkResId( STR_LANGSTATUS_NONE )), css::awt::MenuItemStyle::RADIOCHECK, MID_LANG_SEL_NONE );
    if ( aLanguageTable.GetString( LANGUAGE_NONE ) == m_aCurLang )
        xPopupMenu->checkItem( MID_LANG_SEL_NONE, sal_True );

    xPopupMenu->insertItem( MID_LANG_SEL_RESET, String( FwkResId( STR_RESET_TO_DEFAULT_LANGUAGE )), 0, MID_LANG_SEL_RESET );
    xPopupMenu->insertItem( MID_LANG_SEL_MORE,  String( FwkResId( STR_LANGSTATUS_MORE )), 0, MID_LANG_SEL_MORE );

    // add entries to submenu ('set language for paragraph')
    nItemId = static_cast< sal_Int16 >(MID_LANG_PARA_1);
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if( rStr != OUString( aLanguageTable.GetString( LANGUAGE_NONE ) )&&
            rStr != sAsterix &&
            rStr != sEmpty)
        {
            DBG_ASSERT( MID_LANG_PARA_1 <= nItemId && nItemId <= MID_LANG_PARA_9,
                    "nItemId outside of expected range!" );
            subPopupMenu->insertItem( nItemId, rStr, 0, nItemId );
            aLangMap[nItemId] = rStr;
            ++nItemId;
        }
    }
    subPopupMenu->insertItem( MID_LANG_PARA_NONE,  String( FwkResId( STR_LANGSTATUS_NONE )), 0, MID_LANG_PARA_NONE );
    subPopupMenu->insertItem( MID_LANG_PARA_RESET, String( FwkResId( STR_RESET_TO_DEFAULT_LANGUAGE )), 0, MID_LANG_PARA_RESET );
    subPopupMenu->insertItem( MID_LANG_PARA_MORE,  String( FwkResId( STR_LANGSTATUS_MORE )), 0, MID_LANG_PARA_MORE );

    // add last two entries to main menu
    xPopupMenu->insertSeparator( MID_LANG_PARA_SEPERATOR );
    xPopupMenu->insertItem( MID_LANG_PARA_STRING, String( FwkResId( STR_SET_LANGUAGE_FOR_PARAGRAPH )), 0, MID_LANG_PARA_STRING );
    xPopupMenu->setPopupMenu( MID_LANG_PARA_STRING, subPopupMenu );


    // now display the popup menu and execute every command ...

    Reference< awt::XWindowPeer > xParent( m_xParentWindow, UNO_QUERY );

    com::sun::star::awt::Rectangle aRectangle;
    Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    const Point mMousePos = pWindow->GetPointerPosPixel();
    aRectangle.X = mMousePos.X();
    aRectangle.Y = mMousePos.Y();
    sal_Int16 nId = xPopupMenu->execute( xParent, aRectangle, com::sun::star::awt::PopupMenuDirection::EXECUTE_UP+16 );
    //click "More..."
    if ( nId && m_xFrame.is() )
    {
        uno::Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        util::URL aURL;

        if (MID_LANG_SEL_1 <= nId && nId <= MID_LANG_SEL_9)
        {
            //set selected language as current language for selection
            String aSelectedLang = aLangMap[nId];
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Current_"));
            aURL.Complete += aSelectedLang;
        }
        else if (nId == MID_LANG_SEL_NONE)
        {
            //set None as current language for selection
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE"));
        }
        else if (nId == MID_LANG_SEL_RESET)
        {
            // reset language attributes for selection
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Current_RESET_LANGUAGES"));
        }
        else if (nId == MID_LANG_SEL_MORE)
        {
            //open the dialog "format/character" for current selection
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FontDialog?Language:string=*"));
        }
        else if (MID_LANG_PARA_1 <= nId && nId <= MID_LANG_PARA_9)
        {
            //set selected language for current paragraph
            String aSelectedLang = aLangMap[nId];
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Paragraph_"));
            aURL.Complete += aSelectedLang;
        }
        else if (nId == MID_LANG_PARA_NONE)
        {
            //set None as language for current paragraph
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE"));
        }
        else if (nId == MID_LANG_PARA_RESET)
        {
            // reset language attributes for paragraph
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES"));
        }
        else if (nId == MID_LANG_PARA_MORE)
        {
            //open the dialog "format/character" for current paragraph
            aURL.Complete += OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FontDialogForParagraph"));
        }

        uno::Reference< util::XURLTransformer > xURLTransformer( util::URLTransformer::create(::comphelper::getComponentContext(m_xServiceManager)) );
        xURLTransformer->parseStrict( aURL );
        uno::Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);
        if( xDispatch.is() )
        {
            uno::Sequence< beans::PropertyValue > aPV;
            xDispatch->dispatch( aURL, aPV);
        }
    }
}

void SAL_CALL LangSelectionStatusbarController::command(
    const ::com::sun::star::awt::Point& /*aPos*/,
    ::sal_Int32 nCommand,
    ::sal_Bool /*bMouseEvent*/,
    const ::com::sun::star::uno::Any& /*aData*/ )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::command" );
    if ( nCommand & ::awt::Command::CONTEXTMENU )
    {
        LangMenu();
    }
}

void SAL_CALL LangSelectionStatusbarController::paint(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
    const ::com::sun::star::awt::Rectangle& rOutputRectangle,
    ::sal_Int32 nItemId,
    ::sal_Int32 nStyle )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::paint" );
    svt::StatusbarController::paint( xGraphics, rOutputRectangle, nItemId, nStyle );
}

void SAL_CALL LangSelectionStatusbarController::click()
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::click" );
    LangMenu();
}

void SAL_CALL LangSelectionStatusbarController::doubleClick()
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::doubleClick" );
    svt::StatusbarController::doubleClick();
}

// XStatusListener
void SAL_CALL LangSelectionStatusbarController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    // This function will be called when observed data changes,
    // for example the selection or keyboard language.
    // - It displays the language in use in the status bar
    // - and it stores the relevant data for creating the menu
    //   at some later point in the member variables
    //      m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedText

    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::statusChanged" );
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    m_bShowMenu = sal_True;

    m_nScriptType = LS_SCRIPT_LATIN | LS_SCRIPT_ASIAN | LS_SCRIPT_COMPLEX;  //set the default value
    Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR && m_nID != 0 )
    {
        OUString               aStrValue;
        Sequence< OUString > aSeq;

        StatusBar*    pStatusBar = (StatusBar *)pWindow;
        if ( Event.State >>= aStrValue )
            pStatusBar->SetItemText( m_nID, aStrValue );
        else if ( Event.State >>= aSeq )
        {
            if ( aSeq.getLength() == 4 )
            {
                const String aMultipleLangText( FwkResId( STR_LANGSTATUS_MULTIPLE_LANGUAGES ) );
                OUString aStatusText = aSeq[0];
                if ( 0 == aStatusText.compareToAscii( "*" ))
                    aStatusText = aMultipleLangText;
                pStatusBar->SetItemText( m_nID, aStatusText );

                // Retrieve all other values from the sequence and
                // store it members!
                m_aCurLang      = aSeq[0];
                m_nScriptType   = static_cast< sal_Int16 >( aSeq[1].toInt32() );
                m_aKeyboardLang = aSeq[2];
                m_aGuessedTextLang  = aSeq[3];
            }
        }
        else if ( !Event.State.hasValue() )
        {
            pStatusBar->SetItemText( m_nID, String() );
            m_bShowMenu = sal_False;    // no language -> no menu
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
