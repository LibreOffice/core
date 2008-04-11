/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionstatusbarcontroller.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uielement/langselectionstatusbarcontroller.hxx>
#include <classes/fwkresid.hxx>
#include <services.h>
#include <classes/resource.hrc>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/status.hxx>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <toolkit/helper/vclunohelper.hxx>
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

#include <map>
#include <set>
#include <classes/fwkresid.hxx>
#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/unohlp.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Command.hpp>
#include <svtools/languageoptions.hxx>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::document;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   LangSelectionStatusbarController            ,
                                            OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLER         ,
                                            IMPLEMENTATIONNAME_LANGSELECTIONSTATUSBARCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   LangSelectionStatusbarController, {} )

LangSelectionStatusbarController::LangSelectionStatusbarController( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    svt::StatusbarController( xServiceManager, uno::Reference< frame::XFrame >(), rtl::OUString(), 0 ),
    m_bShowMenu( sal_True ),
    m_nScriptType( 7 )
{
    if (!m_xLanguageGuesser.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr ( comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            m_xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    xMgr->createInstance(
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
    }
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
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

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
    svt::StatusbarController::dispose();
}

// XEventListener
void SAL_CALL LangSelectionStatusbarController::disposing( const com::sun::star::lang::EventObject& Source )
throw ( RuntimeException )
{
    svt::StatusbarController::disposing( Source );
}

// XStatusbarController
::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseButtonDown(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseMove(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL LangSelectionStatusbarController::mouseButtonUp(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

//match ScriptType
bool checkScriptType( sal_Int16 nScriptType, LanguageType nLang )
{
    return 0 != ( nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

void LangSelectionStatusbarController::LangMenu()throw (::com::sun::star::uno::RuntimeException)
{
    if (!m_bShowMenu)
        return;

    //add context menu
    Reference< awt::XPopupMenu > xPopupMenu( m_xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.awt.PopupMenu" ) ), UNO_QUERY );
    //sub menu that contains all items except the last two items: Separator + Set Language for Paragraph
    Reference< awt::XPopupMenu > subPopupMenu(m_xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.awt.PopupMenu" ) ), UNO_QUERY );

    std::set< ::rtl::OUString > LangItems;

    SvtLanguageTable aLanguageTable;
    USHORT nItemId=1;

    //1--add current language
    if( m_aCurLang != OUString::createFromAscii( "" ) &&
        LANGUAGE_DONTKNOW != aLanguageTable.GetType( m_aCurLang ))
        LangItems.insert( m_aCurLang );

    //2--System
    SvtLanguageTable aLangTable;
    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguage();
    if( rSystemLanguage != LANGUAGE_DONTKNOW )
    {
        if ( checkScriptType( m_nScriptType, rSystemLanguage ))
            LangItems.insert( OUString( aLangTable.GetString( rSystemLanguage )) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguage();
    if( rUILanguage != LANGUAGE_DONTKNOW )
    {
        if ( checkScriptType( m_nScriptType, rUILanguage ))
            LangItems.insert( OUString( aLangTable.GetString( rUILanguage )) );
    }

    //4--guessed language
    if ( m_xLanguageGuesser.is() && m_aGuessedText.getLength() > 0)
    {
        ::com::sun::star::lang::Locale aLocale(m_xLanguageGuesser->guessPrimaryLanguage( m_aGuessedText, 0, m_aGuessedText.getLength()) );
        LanguageType nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if (( nLang != LANGUAGE_DONTKNOW ) && ( nLang != LANGUAGE_NONE ) && (nLang != LANGUAGE_SYSTEM)
            && ( checkScriptType( m_nScriptType, nLang )))
            LangItems.insert( aLangTable.GetString( nLang ));
    }

    //5--keyboard language
    if( m_aKeyboardLang != OUString::createFromAscii( "" ))
    {
        if ( checkScriptType( m_nScriptType, aLanguageTable.GetType( m_aKeyboardLang )))
            LangItems.insert( m_aKeyboardLang );
    }

    //6--all languages used in current document
    Reference< com::sun::star::frame::XModel > xModel;
    if ( m_xFrame.is() )
    {
       Reference< com::sun::star::frame::XController > xController( m_xFrame->getController(), UNO_QUERY );
       if ( xController.is() )
           xModel = xController->getModel();
    }
    Reference< document::XDocumentLanguages > xDocumentLanguages( xModel, UNO_QUERY );
    /*the description of m_nScriptType
      LATIN : 1
      ASIAN : 2
      COMPLEX:4
      LATIN  + ASIAN : 3
      LATIN  + COMPLEX : 5
      ASIAN + COMPLEX : 6
      LATIN + ASIAN + COMPLEX : 7
    */

    sal_Int16 nCount=7;
    if ( xDocumentLanguages.is() )
    {
        Sequence< Locale > rLocales( xDocumentLanguages->getDocumentLanguages( m_nScriptType, nCount ));
        if ( rLocales.getLength() > 0 )
        {
            for ( USHORT i = 0; i<rLocales.getLength();++i )
            {
                if ( LangItems.size() == 7 )
                    break;
                const Locale& rLocale=rLocales[i];
                if( checkScriptType( m_nScriptType, aLangTable.GetType( rLocale.Language )))
                    LangItems.insert( OUString( rLocale.Language ) );
            }
        }
    }
    std::map< sal_Int16, ::rtl::OUString > LangTable;

    for( std::set< OUString >::const_iterator it = LangItems.begin(); it != LangItems.end(); ++it )
    {
        if ( *it != OUString( aLangTable.GetString( LANGUAGE_NONE ) )&&
             *it != OUString::createFromAscii( "*" ) &&
             *it != OUString::createFromAscii( ""  ))
        {
            //nItemId = xPopupMenu->getItemCount()+1;
            nItemId++;
            xPopupMenu->insertItem( nItemId, *it, css::awt::MenuItemStyle::RADIOCHECK, nItemId );
            LangTable[nItemId]=*it;
            if( *it == m_aCurLang )
            {
                //make a sign for the current language
                xPopupMenu->checkItem( nItemId, TRUE );
            }
        }
    }

    //7--none
    nItemId++;
    xPopupMenu->insertItem( nItemId, String( FwkResId( STR_LANGSTATUS_NONE )), css::awt::MenuItemStyle::RADIOCHECK, nItemId );
    //More...
    nItemId++;
    xPopupMenu->insertItem( nItemId, String( FwkResId( STR_LANGSTATUS_MORE )), css::awt::MenuItemStyle::RADIOCHECK, nItemId );

    for( ::std::set< OUString >::const_iterator it = LangItems.begin(); it != LangItems.end(); ++it )
    {
        if( *it != OUString( aLangTable.GetString( LANGUAGE_NONE ) )&&
            *it != OUString::createFromAscii( "*" ) &&
            *it != OUString::createFromAscii( ""  ))
        {
            nItemId++;
            subPopupMenu->insertItem( nItemId, *it, css::awt::MenuItemStyle::RADIOCHECK, nItemId );
            LangTable[nItemId]=*it;
        }
    }
    //7--none
    nItemId++;
    subPopupMenu->insertItem( nItemId, String( FwkResId( STR_LANGSTATUS_NONE )), css::awt::MenuItemStyle::RADIOCHECK, nItemId );
    //More
    nItemId++;
    subPopupMenu->insertItem( nItemId, String( FwkResId( STR_LANGSTATUS_MORE )), css::awt::MenuItemStyle::RADIOCHECK, nItemId );

    nItemId++;
    xPopupMenu->insertSeparator(nItemId);

    nItemId++;
    xPopupMenu->insertItem( nItemId, String( FwkResId( STR_SET_LANGUAGE_FOR_PARAGRAPH )), css::awt::MenuItemStyle::RADIOCHECK, nItemId );
    xPopupMenu->setPopupMenu( nItemId, subPopupMenu );

    //display the popup menu and execute every command

    Reference< awt::XWindowPeer > xParent( m_xParentWindow, UNO_QUERY );

    com::sun::star::awt::Rectangle mRectangle;
    Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    const Point mMousePos = pWindow->GetPointerPosPixel();
    mRectangle.X = mMousePos.X();
    mRectangle.Y = mMousePos.Y();
    sal_Int16 nId = xPopupMenu->execute( xParent, mRectangle, com::sun::star::awt::PopupMenuDirection::EXECUTE_UP+16 );
    //click "More..."
    if ( m_xFrame.is() )
    {
        uno::Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        util::URL aURL;

        if ( nId < nItemId-3-subPopupMenu->getItemCount() )
        {
            //1..7
            //set selected language as current language for selection
            String SelectedLang = LangTable[nId];
            aURL.Complete+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Current_");
            aURL.Complete+=SelectedLang;
        }
        else if ( nId == nItemId-3-subPopupMenu->getItemCount() )
        {
            //8
            //set None as current language for selection
            aURL.Complete+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE");
        }
        else if ( nId == nItemId-2-subPopupMenu->getItemCount() )
        {
            //9 (more)...
            //open the dialog "format/character" for current selection
            aURL.Complete+=String::CreateFromAscii(".uno:FontDialog?Language:string=*");
        }
        else if ( nId < nItemId-3 && nId>nItemId-2-subPopupMenu->getItemCount() )
        {
            //1..7 para
            //set selected language for current paragraph
            String SelectedLang = LangTable[nId];
            aURL.Complete+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Paragraph_");
            aURL.Complete+=SelectedLang;
        }
        else if ( nId==nItemId-3 )
        {
            //8 para
            //set None as language for current paragraph
            aURL.Complete+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE");
        }
        else if ( nId==nItemId-2 )
        {
            //9 (more) para...
            //open the dialog "format/character" for current paragraph
            aURL.Complete+=String::CreateFromAscii(".uno:FontDialogForParagraph");
        }

        uno::Reference< util::XURLTransformer > xURLTransformer( m_xServiceManager->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), uno::UNO_QUERY );
        xURLTransformer->parseStrict( aURL );
        uno::Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL,::rtl::OUString(),0);
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
    svt::StatusbarController::paint( xGraphics, rOutputRectangle, nItemId, nStyle );
}

void SAL_CALL LangSelectionStatusbarController::click()
throw (::com::sun::star::uno::RuntimeException)
{
    LangMenu();
}

void SAL_CALL LangSelectionStatusbarController::doubleClick()
throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::doubleClick();
}

// XStatusListener
void SAL_CALL LangSelectionStatusbarController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( m_bDisposed )
        return;

    m_bShowMenu = sal_True;

    m_nScriptType=7;//set the default value
    Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR && m_nID != 0 )
    {
        rtl::OUString               aStrValue;
        Sequence< ::rtl::OUString > aSeq;

        StatusBar*    pStatusBar = (StatusBar *)pWindow;
        if ( Event.State >>= aStrValue )
        {
            pStatusBar->SetItemText( m_nID, aStrValue );
            m_aCurrentLanguage = aStrValue;
        }
        else if ( Event.State >>= aSeq )
        {
            if ( aSeq.getLength() == 4 )
            {
                const String aMultipleLangText( FwkResId( STR_LANGSTATUS_MULTIPLE_LANGUAGES ) );
                ::rtl::OUString aStatusText = aSeq[0];
                if ( 0 == aStatusText.compareToAscii( "*" ))
                    aStatusText = aMultipleLangText;
                pStatusBar->SetItemText( m_nID, aStatusText );

                // Retrieve all other values from the sequence and
                // store it members!
                m_aCurLang      = aSeq[0];
                m_nScriptType   = static_cast< sal_Int16 >( aSeq[1].toInt32() );
                m_aKeyboardLang = aSeq[2];
                m_aGuessedText  = aSeq[3];
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
