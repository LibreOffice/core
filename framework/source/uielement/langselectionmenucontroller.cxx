/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionmenucontroller.cxx,v $
 * $Revision: 1.6.40.1 $
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
#include <uielement/langselectionmenucontroller.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <com/sun/star/awt/XMenuExtended.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <map>
#include <i18npool/mslangid.hxx>
#include <svtools/languageoptions.hxx>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <svtools/langtab.hxx>
#include <classes/fwlresid.hxx>

#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <dispatch/uieventloghelper.hxx>

#include "helper/mischelper.hxx"

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//
using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   LanguageSelectionMenuController         ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_LANGUAGESELECTIONMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   LanguageSelectionMenuController, {} )

LanguageSelectionMenuController::LanguageSelectionMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_bShowMenu( sal_True )
    ,m_aLangGuessHelper(xServiceManager)
{
}

LanguageSelectionMenuController::~LanguageSelectionMenuController()
{
}

// XEventListener
void SAL_CALL LanguageSelectionMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xLanguageDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL LanguageSelectionMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( m_bDisposed )
        return;

    m_bShowMenu = sal_True;
    m_nScriptType=7;//set the default value

    rtl::OUString               aStrValue;
    Sequence< ::rtl::OUString > aSeq;

    if ( Event.State >>= aStrValue )
    {
        m_aCurrentLanguage=aStrValue;
    }
    else if ( Event.State >>= aSeq )
    {
        if ( aSeq.getLength() == 4 )
        {
            // Retrieve all other values from the sequence and
            // store it members!
            m_aCurLang=aSeq[0];
            m_nScriptType= static_cast< sal_Int16 >(aSeq[1].toInt32());
            m_aKeyboardLang=aSeq[2];
            m_aGuessedText=aSeq[3];
        }
    }
    else if ( !Event.State.hasValue() )
    {
        m_bShowMenu = sal_False;    // no language -> no sub-menu entries -> disable menu
    }
}

// XMenuListener
void LanguageSelectionMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Reference< XDispatch > xDispatch = _xDispatch;

    if ( aTargetURL.Complete == m_aMenuCommandURL_Font )
    {   //open format/character dialog for current selection
        xDispatch = m_xMenuDispatch_Font;
    }
    else if ( aTargetURL.Complete == m_aMenuCommandURL_Lang )
    {   //open language tab-page in tools/options dialog
        xDispatch = m_xMenuDispatch_Lang;
    }
    else if ( aTargetURL.Complete == m_aMenuCommandURL_CharDlgForParagraph )
    {   //open format/character dialog for current selection
        xDispatch = m_xMenuDispatch_CharDlgForParagraph;
    }

    if ( !xDispatch.is() )
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
    }

    if ( xDispatch.is() )
    {
        Sequence<PropertyValue>      aArgs;
        if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
            UiEventLogHelper(::rtl::OUString::createFromAscii("LanguageSelectionMenuController")).log(m_xServiceManager, m_xFrame, aTargetURL, aArgs);
        xDispatch->dispatch( aTargetURL, aArgs );
    }
}

// XPopupMenuController
void LanguageSelectionMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

    com::sun::star::util::URL aTargetURL;

    // Register for language updates
    aTargetURL.Complete = m_aLangStatusCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xLanguageDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

    // Register for setting languages and opening language dialog
    aTargetURL.Complete = m_aMenuCommandURL_Lang;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_Lang = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

    // Register for opening character dialog
    aTargetURL.Complete = m_aMenuCommandURL_Font;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_Font = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

    // Register for opening character dialog with preselected paragraph
    aTargetURL.Complete = m_aMenuCommandURL_CharDlgForParagraph;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_CharDlgForParagraph = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
}

void LanguageSelectionMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu , const Mode eMode )
{
    VCLXPopupMenu* pVCLPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pPopupMenu    = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if (!m_bShowMenu)
        return;

    if ( pVCLPopupMenu )
        pPopupMenu = (PopupMenu *)pVCLPopupMenu->GetMenu();

    String aCmd;
    String aCmd_Dialog;
    String aCmd_Language;
    if( eMode == MODE_SetLanguageSelectionMenu )
    {
        aCmd_Dialog+=String::CreateFromAscii(".uno:FontDialog?Language:string=*");
        aCmd_Language+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Current_");
    }
    else if ( eMode == MODE_SetLanguageParagraphMenu )
    {
        aCmd_Dialog+=String::CreateFromAscii(".uno:FontDialogForParagraph");
        aCmd_Language+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Paragraph_");
    }
    else if ( eMode == MODE_SetLanguageAllTextMenu )
    {
        aCmd_Dialog+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=*");
        aCmd_Language+=String::CreateFromAscii(".uno:LanguageStatus?Language:string=Default_");
    }

    //Reference< awt::XMenuExtended > m_xMenuExtended( m_xPopupMenu, UNO_QUERY );
    std::map< ::rtl::OUString, ::rtl::OUString > LangItems;

    SvtLanguageTable aLanguageTable;
    USHORT nItemId = 1;

    //1--add current language
    if(m_aCurLang.getLength())
    {
        LangItems[m_aCurLang]=m_aCurLang;
    }

    SvtLanguageTable aLangTable;
    //2--System
    const AllSettings& rAllSettings=Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguage();
    if(rSystemLanguage!=LANGUAGE_DONTKNOW)
    {
        if (IsScriptTypeMatchingToLanguage(m_nScriptType,rSystemLanguage ))
            LangItems[::rtl::OUString(aLangTable.GetString(rSystemLanguage))]=::rtl::OUString(aLangTable.GetString(rSystemLanguage));
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguage();
    if(rUILanguage!=LANGUAGE_DONTKNOW)
    {
        if (IsScriptTypeMatchingToLanguage(m_nScriptType, rUILanguage ))
            LangItems[::rtl::OUString(aLangTable.GetString(rUILanguage))]=::rtl::OUString(aLangTable.GetString(rUILanguage));
    }

    //4--guessed language
    uno::Reference< linguistic2::XLanguageGuessing > xLangGuesser( m_aLangGuessHelper.GetGuesser() );
    if (xLangGuesser.is() && m_aGuessedText.getLength() > 0)
    {
        ::com::sun::star::lang::Locale aLocale(xLangGuesser->guessPrimaryLanguage( m_aGuessedText, 0, m_aGuessedText.getLength()) );
        LanguageType nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_NONE && nLang != LANGUAGE_SYSTEM
            && IsScriptTypeMatchingToLanguage( m_nScriptType, nLang ))
            LangItems[aLangTable.GetString(nLang)]=aLangTable.GetString(nLang);
    }

    //5--keyboard language
    if(m_aKeyboardLang!=::rtl::OUString::createFromAscii(""))
    {
        if (IsScriptTypeMatchingToLanguage(m_nScriptType, aLanguageTable.GetType(m_aKeyboardLang)))
            LangItems[m_aKeyboardLang] = m_aKeyboardLang;
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
    if(xDocumentLanguages.is())
    {
        Sequence< Locale > rLocales(xDocumentLanguages->getDocumentLanguages(m_nScriptType,nCount));
        if(rLocales.getLength()>0)
        {
            for(USHORT i = 0; i<rLocales.getLength();++i)
            {
                if (LangItems.size()==7)
                    break;
                const Locale& rLocale=rLocales[i];
                if(IsScriptTypeMatchingToLanguage(m_nScriptType, aLanguageTable.GetType(rLocale.Language)))
                    LangItems[rLocale.Language] = rLocale.Language;
            }
        }
    }
    std::map< sal_Int16, ::rtl::OUString > LangTable;

    const ::rtl::OUString sAsterix(RTL_CONSTASCII_USTRINGPARAM("*"));
    for(std::map< ::rtl::OUString, ::rtl::OUString >::const_iterator it = LangItems.begin(); it != LangItems.end(); ++it)
    {
        if(it->first != ::rtl::OUString( aLangTable.GetString( LANGUAGE_NONE ) )&&
           it->first != sAsterix &&
           it->first.getLength())
        {
            ++nItemId;
            pPopupMenu->InsertItem( nItemId,it->first);
            LangTable[nItemId] = it->first;
            if(it->first == m_aCurLang && eMode == MODE_SetLanguageSelectionMenu )
            {
                //make a sign for the current language
                pPopupMenu->CheckItem(nItemId,TRUE);
            }
            aCmd=aCmd_Language;
            aCmd+=(String)it->first;
            pPopupMenu->SetItemCommand(nItemId,aCmd);
        }
    }

    //7--none
    nItemId++;
    pPopupMenu->InsertItem( nItemId, String(FwlResId( STR_LANGSTATUS_NONE )) );
    aCmd=aCmd_Language;
    aCmd+=String::CreateFromAscii("LANGUAGE_NONE");
    pPopupMenu->SetItemCommand(nItemId,aCmd);

    //More...
    nItemId++;
    pPopupMenu->InsertItem( nItemId, String(FwlResId( STR_LANGSTATUS_MORE )));
    pPopupMenu->SetItemCommand(nItemId,aCmd_Dialog);
}


void SAL_CALL LanguageSelectionMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    PopupMenuControllerBase::updatePopupMenu();

    // Force status update to get information about the current languages
    ResetableGuard aLock( m_aLock );
    Reference< XDispatch > xDispatch( m_xLanguageDispatch );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aLangStatusCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
        xDispatch->removeStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
    }

    // TODO: Fill menu with the information retrieved by the status update

    if( m_aCommandURL.equalsAscii( ".uno:SetLanguageSelectionMenu" ))
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageSelectionMenu );
    }
    else if( m_aCommandURL.equalsAscii( ".uno:SetLanguageParagraphMenu" ))
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageParagraphMenu );
    }
    else if( m_aCommandURL.equalsAscii( ".uno:SetLanguageAllTextMenu" ))
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageAllTextMenu );
    }
}

// XInitialization
void SAL_CALL LanguageSelectionMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PopupMenuControllerBase::initialize(aArguments);

        if ( m_bInitialized )
        {
            m_aLangStatusCommandURL               = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LanguageStatus" ));
            m_aMenuCommandURL_Lang                = m_aLangStatusCommandURL;
            m_aMenuCommandURL_Font                = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontDialog" ));
            m_aMenuCommandURL_CharDlgForParagraph = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontDialogForParagraph" ));
        }
    }
}

}

