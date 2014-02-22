/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <uielement/langselectionmenucontroller.hxx>

#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/mnemonic.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

#include <i18nlangtag/mslangid.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <classes/fwlresid.hxx>

#include <classes/resource.hrc>

#include "helper/mischelper.hxx"
#include <osl/mutex.hxx>

#include <map>
#include <set>




using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   LanguageSelectionMenuController         ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_LANGUAGESELECTIONMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   LanguageSelectionMenuController, {} )

LanguageSelectionMenuController::LanguageSelectionMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bShowMenu( sal_True ),
    m_aLangGuessHelper( xContext )
{
}

LanguageSelectionMenuController::~LanguageSelectionMenuController()
{
}


void SAL_CALL LanguageSelectionMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xLanguageDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}


void SAL_CALL LanguageSelectionMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;

    m_bShowMenu = sal_True;
    m_nScriptType = LS_SCRIPT_LATIN | LS_SCRIPT_ASIAN | LS_SCRIPT_COMPLEX;  

    Sequence< OUString > aSeq;

    if ( Event.State >>= aSeq )
    {
        if ( aSeq.getLength() == 4 )
        {
            
            
            m_aCurLang          = aSeq[0];
            m_nScriptType       = static_cast< sal_Int16 >(aSeq[1].toInt32());
            m_aKeyboardLang     = aSeq[2];
            m_aGuessedTextLang  = aSeq[3];
        }
    }
    else if ( !Event.State.hasValue() )
    {
        m_bShowMenu = sal_False;    
    }
}


void LanguageSelectionMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Reference< XDispatch > xDispatch = _xDispatch;

    if ( aTargetURL.Complete == m_aMenuCommandURL_Font )
    {   
        xDispatch = m_xMenuDispatch_Font;
    }
    else if ( aTargetURL.Complete == m_aMenuCommandURL_Lang )
    {   
        xDispatch = m_xMenuDispatch_Lang;
    }
    else if ( aTargetURL.Complete == m_aMenuCommandURL_CharDlgForParagraph )
    {   
        xDispatch = m_xMenuDispatch_CharDlgForParagraph;
    }

    if ( !xDispatch.is() )
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    }

    if ( xDispatch.is() )
    {
        Sequence<PropertyValue>      aArgs;
        xDispatch->dispatch( aTargetURL, aArgs );
    }
}


void LanguageSelectionMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

    com::sun::star::util::URL aTargetURL;

    
    aTargetURL.Complete = m_aLangStatusCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xLanguageDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

    
    aTargetURL.Complete = m_aMenuCommandURL_Lang;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_Lang = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

    
    aTargetURL.Complete = m_aMenuCommandURL_Font;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_Font = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

    
    aTargetURL.Complete = m_aMenuCommandURL_CharDlgForParagraph;
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xMenuDispatch_CharDlgForParagraph = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
}

void LanguageSelectionMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu , const Mode eMode )
{
    VCLXPopupMenu* pVCLPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pPopupMenu    = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if (!m_bShowMenu)
        return;

    if ( pVCLPopupMenu )
        pPopupMenu = (PopupMenu *)pVCLPopupMenu->GetMenu();

    OUString aCmd;
    OUString aCmd_Dialog;
    OUString aCmd_Language;
    if( eMode == MODE_SetLanguageSelectionMenu )
    {
        aCmd_Dialog += ".uno:FontDialog?Language:string=*";
        aCmd_Language += ".uno:LanguageStatus?Language:string=Current_";
    }
    else if ( eMode == MODE_SetLanguageParagraphMenu )
    {
        aCmd_Dialog += ".uno:FontDialogForParagraph";
        aCmd_Language += ".uno:LanguageStatus?Language:string=Paragraph_";
    }
    else if ( eMode == MODE_SetLanguageAllTextMenu )
    {
        aCmd_Dialog += ".uno:LanguageStatus?Language:string=*";
        aCmd_Language += ".uno:LanguageStatus?Language:string=Default_";
    }

    SvtLanguageTable    aLanguageTable;

    
    std::set< OUString > aLangItems;
    FillLangItems( aLangItems, aLanguageTable, m_xFrame, m_aLangGuessHelper,
            m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedTextLang );

    
    
    

    sal_Int16 nItemId = 1;  
    const OUString sAsterisk("*");  
    const OUString sEmpty;  
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if (rStr != aLanguageTable.GetString( LANGUAGE_NONE ) &&
            rStr != sAsterisk &&
            rStr != sEmpty)
        {
            pPopupMenu->InsertItem( nItemId, rStr );
            aCmd = aCmd_Language;
            aCmd += rStr;
            pPopupMenu->SetItemCommand( nItemId, aCmd );
            if (rStr == m_aCurLang && eMode == MODE_SetLanguageSelectionMenu )
            {
                
                pPopupMenu->CheckItem( nItemId, true );
            }
            ++nItemId;
        }
    }

    
    ++nItemId;
    pPopupMenu->InsertItem( nItemId, FwlResId(STR_LANGSTATUS_NONE).toString() );
    aCmd = aCmd_Language + "LANGUAGE_NONE";
    pPopupMenu->SetItemCommand( nItemId, aCmd );

    
    ++nItemId;
    pPopupMenu->InsertItem( nItemId, FwlResId(STR_RESET_TO_DEFAULT_LANGUAGE).toString() );
    aCmd = aCmd_Language + "RESET_LANGUAGES";
    pPopupMenu->SetItemCommand( nItemId, aCmd );

    
    ++nItemId;
    pPopupMenu->InsertItem( nItemId, FwlResId(STR_LANGSTATUS_MORE).toString());
    pPopupMenu->SetItemCommand( nItemId, aCmd_Dialog );
}


void SAL_CALL LanguageSelectionMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    svt::PopupMenuControllerBase::updatePopupMenu();

    
    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< XDispatch > xDispatch( m_xLanguageDispatch );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aLangStatusCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
        xDispatch->removeStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
    }

    

    if ( m_aCommandURL == ".uno:SetLanguageSelectionMenu" )
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageSelectionMenu );
    }
    else if ( m_aCommandURL == ".uno:SetLanguageParagraphMenu" )
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageParagraphMenu );
    }
    else if ( m_aCommandURL == ".uno:SetLanguageAllTextMenu" )
    {
        fillPopupMenu(m_xPopupMenu, MODE_SetLanguageAllTextMenu );
    }
}


void SAL_CALL LanguageSelectionMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        svt::PopupMenuControllerBase::initialize(aArguments);

        if ( m_bInitialized )
        {
            m_aLangStatusCommandURL               = ".uno:LanguageStatus";
            m_aMenuCommandURL_Lang                = m_aLangStatusCommandURL;
            m_aMenuCommandURL_Font                = ".uno:FontDialog";
            m_aMenuCommandURL_CharDlgForParagraph = ".uno:FontDialogForParagraph";
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
