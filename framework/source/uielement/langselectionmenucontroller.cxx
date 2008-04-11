/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionmenucontroller.cxx,v $
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
#include <com/sun/star/util/XURLTransformer.hpp>
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
#include <classes/fwkresid.hxx>

#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//
using namespace ::rtl;
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
void SAL_CALL LanguageSelectionMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL LanguageSelectionMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    //xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is())//&& xDispatch.is()
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence< PropertyValue >    aArgs;
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

                // Command URL used to dispatch the selected font family name
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

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

            xURLTransformer->parseStrict( aTargetURL );
            if ( !xDispatch.is() )
            {
                Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
                if ( xDispatchProvider.is() )
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            }

            if ( xDispatch.is() )
            {
                xDispatch->dispatch( aTargetURL, aArgs );
            }
        }
    }
}

void SAL_CALL LanguageSelectionMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL LanguageSelectionMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL LanguageSelectionMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));


        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for language updates
        aTargetURL.Complete = m_aLangStatusCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xLanguageDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for setting languages and opening language dialog
        aTargetURL.Complete = m_aMenuCommandURL_Lang;
        xURLTransformer->parseStrict( aTargetURL );
        m_xMenuDispatch_Lang = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for opening character dialog
        aTargetURL.Complete = m_aMenuCommandURL_Font;
        xURLTransformer->parseStrict( aTargetURL );
        m_xMenuDispatch_Font = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for opening character dialog with preselected paragraph
        aTargetURL.Complete = m_aMenuCommandURL_CharDlgForParagraph;
        xURLTransformer->parseStrict( aTargetURL );
        m_xMenuDispatch_CharDlgForParagraph = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

//match ScriptType
bool lcl_checkScriptType(sal_Int16 nScriptType,LanguageType nLang)
{
    return 0 != (nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
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

    String aCmd=String::CreateFromAscii("");
    String aCmd_Dialog=String::CreateFromAscii("");
    String aCmd_Language=String::CreateFromAscii("");
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
    if(m_aCurLang!=OUString::createFromAscii(""))
    {
        LangItems[m_aCurLang]=m_aCurLang;
    }

    SvtLanguageTable aLangTable;
    //2--System
    const AllSettings& rAllSettings=Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguage();
    if(rSystemLanguage!=LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(m_nScriptType,rSystemLanguage ))
            LangItems[OUString(aLangTable.GetString(rSystemLanguage))]=OUString(aLangTable.GetString(rSystemLanguage));
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguage();
    if(rUILanguage!=LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(m_nScriptType, rUILanguage ))
            LangItems[OUString(aLangTable.GetString(rUILanguage))]=OUString(aLangTable.GetString(rUILanguage));
    }

    //4--guessed language
    if (m_xLanguageGuesser.is() && m_aGuessedText.getLength() > 0)
    {
        ::com::sun::star::lang::Locale aLocale(m_xLanguageGuesser->guessPrimaryLanguage( m_aGuessedText, 0, m_aGuessedText.getLength()) );
        LanguageType nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if ((nLang != LANGUAGE_DONTKNOW) && (nLang != LANGUAGE_NONE) && (nLang != LANGUAGE_SYSTEM)
            && (lcl_checkScriptType( m_nScriptType, nLang )))
            LangItems[aLangTable.GetString(nLang)]=aLangTable.GetString(nLang);
    }

    //5--keyboard language
    if(m_aKeyboardLang!=OUString::createFromAscii(""))
    {
        if (lcl_checkScriptType(m_nScriptType, aLanguageTable.GetType(m_aKeyboardLang)))
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
                if(lcl_checkScriptType(m_nScriptType, aLanguageTable.GetType(rLocale.Language)))
                    LangItems[OUString(rLocale.Language)]=OUString(rLocale.Language);
            }
        }
    }
    std::map< sal_Int16, ::rtl::OUString > LangTable;

    for(std::map<OUString, OUString>::const_iterator it = LangItems.begin(); it != LangItems.end(); ++it)
    {
        if(it->first != OUString( aLangTable.GetString( LANGUAGE_NONE ) )&&
           it->first != OUString::createFromAscii("*") &&
           it->first != OUString::createFromAscii(""))
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
    pPopupMenu->InsertItem( nItemId, String(FwkResId( STR_LANGSTATUS_NONE )) );
    aCmd=aCmd_Language;
    aCmd+=String::CreateFromAscii("LANGUAGE_NONE");
    pPopupMenu->SetItemCommand(nItemId,aCmd);

    //More...
    nItemId++;
    pPopupMenu->InsertItem( nItemId, String(FwkResId( STR_LANGSTATUS_MORE )));
    pPopupMenu->SetItemCommand(nItemId,aCmd_Dialog);
}


void SAL_CALL LanguageSelectionMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    PopupMenuControllerBase::updatePopupMenu();

    // Force status update to get information about the current languages
    ResetableGuard aLock( m_aLock );
    Reference< XDispatch > xDispatch( m_xLanguageDispatch );
    Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aLangStatusCommandURL;
    xURLTransformer->parseStrict( aTargetURL );
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
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            m_xFrame                              = xFrame;
            m_aCommandURL                         = aCommandURL;
            m_aBaseURL                            = determineBaseURL( aCommandURL );
            m_aLangStatusCommandURL               = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LanguageStatus" ));
            m_aMenuCommandURL_Lang                = m_aLangStatusCommandURL;
            m_aMenuCommandURL_Font                = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontDialog" ));
            m_aMenuCommandURL_CharDlgForParagraph = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontDialogForParagraph" ));
            m_bInitialized                        = true;
        }
    }
}

}
