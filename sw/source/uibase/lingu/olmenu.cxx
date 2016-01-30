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

#include "SwRewriter.hxx"
#include "chrdlg.hrc"
#include "cmdid.h"
#include "comcore.hrc"
#include "crsskip.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "edtwin.hxx"
#include "helpid.h"
#include "hintids.hxx"
#include "langhelper.hxx"
#include "ndtxt.hxx"
#include "olmenu.hrc"
#include "olmenu.hxx"
#include "swabstdlg.hxx"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "swundo.hxx"
#include "uitool.hxx"
#include "unomid.h"
#include "view.hxx"
#include "viewopt.hxx"
#include "wrtsh.hxx"
#include "wview.hxx"
#include "textsh.hxx"

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editview.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <osl/file.hxx>
#include <rtl/string.hxx>
#include <vcl/graphicfilter.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#include <svl/stritem.hxx>
#include <svtools/langtab.hxx>
#include <svx/dlgutil.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sal/macros.h>

#include <map>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/SingleProofreadingError.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>

using namespace ::com::sun::star;

/// @returns : the language for the selected text that is set for the
///     specified attribute (script type).
///     If there are more than one languages used LANGUAGE_DONTKNOW will be returned.
/// @param nLangWhichId : one of
///     RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
/// @returns: the language in use for the selected text.
///     'In use' means the language(s) matching the script type(s) of the
///     selected text. Or in other words, the language a spell checker would use.
///     If there is more than one language LANGUAGE_DONTKNOW will be returned.
// check if nScriptType includes the script type associated to nLang
static inline bool lcl_checkScriptType( SvtScriptType nScriptType, LanguageType nLang )
{
    return bool(nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

void SwSpellPopup::fillLangPopupMenu(
    PopupMenu *pPopupMenu,
    sal_uInt16 nLangItemIdStart,
    const uno::Sequence< OUString >& aSeq,
    SwWrtShell* pWrtSh,
    std::map< sal_Int16, OUString > &rLangTable )
{
    if (!pPopupMenu)
        return;

    // set of languages to be displayed in the sub menus
    std::set< OUString > aLangItems;

    OUString    aCurLang( aSeq[0] );
    SvtScriptType  nScriptType = static_cast<SvtScriptType>(aSeq[1].toInt32());
    OUString    aKeyboardLang( aSeq[2] );
    OUString    aGuessedTextLang( aSeq[3] );

    if (!aCurLang.isEmpty() &&
        LANGUAGE_DONTKNOW != SvtLanguageTable::GetLanguageType( aCurLang ))
        aLangItems.insert( aCurLang );

    //2--System
    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguageTag().getLanguageType();
    if (rSystemLanguage != LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType( nScriptType, rSystemLanguage ))
            aLangItems.insert( SvtLanguageTable::GetLanguageString(rSystemLanguage) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguageTag().getLanguageType();
    if (rUILanguage != LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(nScriptType, rUILanguage ))
            aLangItems.insert( SvtLanguageTable::GetLanguageString(rUILanguage) );
    }

    //4--guessed language
    if (!aGuessedTextLang.isEmpty())
    {
        if (lcl_checkScriptType(nScriptType, SvtLanguageTable::GetLanguageType(aGuessedTextLang)))
            aLangItems.insert( aGuessedTextLang );
    }

    //5--keyboard language
    if (!aKeyboardLang.isEmpty())
    {
        if (lcl_checkScriptType(nScriptType, SvtLanguageTable::GetLanguageType(aKeyboardLang)))
            aLangItems.insert( aKeyboardLang );
    }

    //6--all languages used in current document
    uno::Reference< css::frame::XModel > xModel;
    uno::Reference< css::frame::XController > xController( pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface()->getController(), uno::UNO_QUERY );
    if ( xController.is() )
        xModel = xController->getModel();
    uno::Reference< document::XDocumentLanguages > xDocumentLanguages( xModel, uno::UNO_QUERY );
    /*the description of nScriptType flags
      LATIN :   0x0001
      ASIAN :   0x0002
      COMPLEX:  0x0004
    */
    const sal_Int16 nMaxCount = 7;
    if (xDocumentLanguages.is())
    {
        uno::Sequence< lang::Locale > rLocales( xDocumentLanguages->getDocumentLanguages( static_cast<sal_Int16>(nScriptType), nMaxCount ) );
        if (rLocales.getLength() > 0)
        {
            for (sal_Int32 i = 0; i < rLocales.getLength(); ++i)
            {
                if (aLangItems.size() == (size_t)nMaxCount)
                    break;
                const lang::Locale& rLocale = rLocales[i];
                if (lcl_checkScriptType( nScriptType, SvtLanguageTable::GetLanguageType( rLocale.Language )))
                    aLangItems.insert( rLocale.Language );
            }
        }
    }

    sal_uInt16 nItemId = nLangItemIdStart;
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        OUString aEntryText( *it );
        if (aEntryText != SvtLanguageTable::GetLanguageString( LANGUAGE_NONE ) &&
            aEntryText != "*" && // multiple languages in current selection
            !aEntryText.isEmpty()) // 'no language found' from language guessing
        {
            OSL_ENSURE( nLangItemIdStart <= nItemId && nItemId <= nLangItemIdStart + MN_MAX_NUM_LANG,
                    "nItemId outside of expected range!" );
            pPopupMenu->InsertItem( nItemId, aEntryText, MenuItemBits::RADIOCHECK );
            if (aEntryText == aCurLang)
            {
                //make a check mark for the current language
                pPopupMenu->CheckItem( nItemId );
            }
            rLangTable[ nItemId ] = aEntryText;
            ++nItemId;
        }
    }

    pPopupMenu->InsertItem( nLangItemIdStart + MN_NONE_OFFSET,  OUString(SW_RES( STR_LANGSTATUS_NONE )), MenuItemBits::RADIOCHECK );
    if ( SvtLanguageTable::GetLanguageString( LANGUAGE_NONE ) == aCurLang )
        pPopupMenu->CheckItem( nLangItemIdStart + MN_NONE_OFFSET );

    pPopupMenu->InsertItem( nLangItemIdStart + MN_RESET_OFFSET, OUString(SW_RES( STR_RESET_TO_DEFAULT_LANGUAGE )) );
    pPopupMenu->InsertItem( nLangItemIdStart + MN_MORE_OFFSET,  OUString(SW_RES( STR_LANGSTATUS_MORE )) );
}

OUString RetrieveLabelFromCommand( const OUString& aCmdURL )
{
    OUString aLabel;
    if ( !aCmdURL.isEmpty() )
    {
        try
        {
            uno::Reference< container::XNameAccess > const xNameAccess(
                    frame::theUICommandDescription::get(
                        ::comphelper::getProcessComponentContext() ),
                    uno::UNO_QUERY_THROW );
            uno::Reference< container::XNameAccess > xUICommandLabels;
            uno::Any a = xNameAccess->getByName( "com.sun.star.text.TextDocument" );
            uno::Reference< container::XNameAccess > xUICommands;
            a >>= xUICommandLabels;
            OUString aStr;
            uno::Sequence< beans::PropertyValue > aPropSeq;
            a = xUICommandLabels->getByName( aCmdURL );
            if ( a >>= aPropSeq )
            {
                for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                {
                    if ( aPropSeq[i].Name == "Label" )
                    {
                        aPropSeq[i].Value >>= aStr;
                        break;
                    }
                }
            }
            aLabel = aStr;
        }
        catch (const uno::Exception&)
        {
        }
    }

    return aLabel;
}

SwSpellPopup::SwSpellPopup(
        SwWrtShell* pWrtSh,
        const uno::Reference< linguistic2::XSpellAlternatives >  &xAlt,
        const OUString &rParaText
)   : PopupMenu( SW_RES(MN_SPELL_POPUP) )
    , m_pSh( pWrtSh )
    , m_xSpellAlt(xAlt)
    , m_bGrammarResults(false)
{
    OSL_ENSURE(m_xSpellAlt.is(), "no spelling alternatives available");

    SetMenuFlags(MenuFlags::NoAutoMnemonics);
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    m_nCheckedLanguage = LANGUAGE_NONE;
    if (m_xSpellAlt.is())
    {
        m_nCheckedLanguage = LanguageTag( m_xSpellAlt->getLocale() ).getLanguageType();
        m_aSuggestions = m_xSpellAlt->getAlternatives();
    }
    sal_Int16 nStringCount = static_cast< sal_Int16 >( m_aSuggestions.getLength() );

    SvtLinguConfig aCfg;

    PopupMenu *pMenu = GetPopupMenu(MN_AUTOCORR);
    pMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);
    bool bEnable = false;
    if( nStringCount )
    {
        Image aImage;
        OUString aSuggestionImageUrl;

        if (bUseImagesInMenus)
        {
            uno::Reference< container::XNamed > xNamed( m_xSpellAlt, uno::UNO_QUERY );
            if (xNamed.is())
            {
                aSuggestionImageUrl = aCfg.GetSpellAndGrammarContextSuggestionImage( xNamed->getName() );
                aImage = Image( aSuggestionImageUrl );
            }
        }

        InsertSeparator(OString(), 0);
        bEnable = true;
        sal_uInt16 nAutoCorrItemId  = MN_AUTOCORR_START;
        sal_uInt16 nItemId          = MN_SUGGESTION_START;
        for (sal_uInt16 i = 0; i < nStringCount; ++i)
        {
            const OUString aEntry = m_aSuggestions[ i ];
            InsertItem(nItemId, aEntry, MenuItemBits::NONE, OString(), i);
            SetHelpId( nItemId, HID_LINGU_REPLACE);
            if (!aSuggestionImageUrl.isEmpty())
                SetItemImage( nItemId, aImage );

            pMenu->InsertItem( nAutoCorrItemId, aEntry );
            pMenu->SetHelpId( nAutoCorrItemId, HID_LINGU_AUTOCORR);

            ++nAutoCorrItemId;
            ++nItemId;
        }
    }

    OUString aIgnoreSelection( SW_RES( STR_IGNORE_SELECTION ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( ".uno:SpellingAndGrammarDialog" );
    SetItemText( MN_SPELLING_DLG, aSpellingAndGrammar );
    SetItemText(MN_AUTO_CORRECT_DLG, RetrieveLabelFromCommand(".uno:AutoCorrectDlg"));
    sal_uInt16 nItemPos = GetItemPos( MN_IGNORE_WORD );
    InsertItem(MN_IGNORE_SELECTION, aIgnoreSelection, MenuItemBits::NONE, OString(), nItemPos);
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, bEnable );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    m_nGuessLangWord = LANGUAGE_NONE;
    m_nGuessLangPara = LANGUAGE_NONE;
    if (m_xSpellAlt.is() && xLG.is())
    {
        m_nGuessLangWord = EditView::CheckLanguage( m_xSpellAlt->getWord(), ::GetSpellChecker(), xLG, false );
        m_nGuessLangPara = EditView::CheckLanguage( rParaText, ::GetSpellChecker(), xLG, true );
    }
    if (m_nGuessLangWord != LANGUAGE_NONE || m_nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (m_nGuessLangWord == LANGUAGE_NONE)
            m_nGuessLangWord = m_nGuessLangPara;
        if (m_nGuessLangPara == LANGUAGE_NONE)
            m_nGuessLangPara = m_nGuessLangWord;
    }

    pMenu = GetPopupMenu(MN_ADD_TO_DIC);
    pMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);     //! necessary to retrieve the correct dictionary name in 'Execute' below
    uno::Reference< linguistic2::XSearchableDictionaryList >    xDicList( SvxGetDictionaryList() );
    sal_uInt16 nItemId = MN_DICTIONARIES_START;
    if (xDicList.is())
    {
        // add the default positive dictionary to dic-list (if not already done).
        // This is to ensure that there is at least one dictionary to which
        // words could be added.
        uno::Reference< linguistic2::XDictionary >  xDic( SvxGetOrCreatePosDic( xDicList ) );
        if (xDic.is())
            xDic->setActive( sal_True );

        m_aDics = xDicList->getDictionaries();
        const uno::Reference< linguistic2::XDictionary >  *pDic = m_aDics.getConstArray();
        sal_uInt16 nDicCount = static_cast< sal_uInt16 >(m_aDics.getLength());

        for( sal_uInt16 i = 0; i < nDicCount; i++ )
        {
            uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
            if (!xDicTmp.is() || SvxGetIgnoreAllList() == xDicTmp)
                continue;

            uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
            LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
            if( xDicTmp->isActive()
                &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                && (m_nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                && (!xStor.is() || !xStor->isReadonly()) )
            {
                // the extra 1 is because of the (possible) external
                // linguistic entry above
                pMenu->InsertItem( nItemId, xDicTmp->getName() );
                m_aDicNameSingle = xDicTmp->getName();

                if (bUseImagesInMenus)
                {
                    uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                    if (xSvcInfo.is())
                    {
                        OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                                xSvcInfo->getImplementationName() ) );
                        if (!aDictionaryImageUrl.isEmpty())
                        {
                            Image aImage( aDictionaryImageUrl );
                            pMenu->SetItemImage( nItemId, aImage );
                        }
                    }
                }

                ++nItemId;
            }
        }
    }
    EnableItem( MN_ADD_TO_DIC, (nItemId - MN_DICTIONARIES_START) > 1 );
    EnableItem( MN_ADD_TO_DIC_SINGLE, (nItemId - MN_DICTIONARIES_START) == 1 );

    //ADD NEW LANGUAGE MENU ITEM

    OUString aScriptTypesInUse( OUString::number( static_cast<int>(pWrtSh->GetScriptType()) ) );

    // get keyboard language
    OUString aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

    // get the language that is in use
    OUString aCurrentLang("*");
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = SvtLanguageTable::GetLanguageString(m_nGuessLangWord);

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_SELECTION);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, m_aLangTable_Text );
    EnableItem( MN_SET_LANGUAGE_SELECTION );

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_PARAGRAPH);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, m_aLangTable_Paragraph );
    EnableItem( MN_SET_LANGUAGE_PARAGRAPH );

    if (bUseImagesInMenus)
    {
        uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface();
        Image rImg = ::GetImage( xFrame, ".uno:SpellingAndGrammarDialog", false );
        SetItemImage( MN_SPELLING_DLG, rImg );
    }

    checkRedline();
    RemoveDisabledEntries( true, true );
}

SwSpellPopup::SwSpellPopup(
    SwWrtShell *pWrtSh,
    const linguistic2::ProofreadingResult &rResult,
    sal_Int32 nErrorInResult,
    const uno::Sequence< OUString > &rSuggestions,
    const OUString &rParaText ) :
PopupMenu( SW_RES(MN_SPELL_POPUP) ),
m_pSh( pWrtSh ),
m_xGrammarResult( rResult ),
m_aSuggestions( rSuggestions ),
m_sExplanationLink( ),
m_bGrammarResults( true ),
m_aInfo16( SW_RES(IMG_INFO_16) )
{
    m_nCheckedLanguage = LanguageTag::convertToLanguageType( rResult.aLocale );
    m_nGrammarError = nErrorInResult;
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    sal_uInt16 nPos = 0;
    OUString aMessageText( rResult.aErrors[ nErrorInResult ].aShortComment );
    InsertSeparator(OString(), nPos++);
    InsertItem(MN_SHORT_COMMENT, aMessageText, MenuItemBits::NOSELECT, OString(), nPos++);
    if (bUseImagesInMenus)
        SetItemImage( MN_SHORT_COMMENT, m_aInfo16 );

    // Add an item to show detailed infos if the FullCommentURL property is defined
    beans::PropertyValues  aProperties = rResult.aErrors[ nErrorInResult ].aProperties;
    {
        sal_Int32 i = 0;
        while ( m_sExplanationLink.isEmpty() && i < aProperties.getLength() )
        {
            if ( aProperties[i].Name == "FullCommentURL" )
            {
                uno::Any aValue = aProperties[i].Value;
                aValue >>= m_sExplanationLink;
            }
            ++i;
        }
    }

    if ( !m_sExplanationLink.isEmpty( ) )
    {
        InsertItem(MN_EXPLANATION_LINK, SW_RESSTR(STR_EXPLANATION_LINK), MenuItemBits::TEXT | MenuItemBits::HELP, OString(), nPos++);
    }

    SetMenuFlags(MenuFlags::NoAutoMnemonics);

    InsertSeparator(OString(), nPos++);
    sal_Int32 nStringCount = m_aSuggestions.getLength();
    if ( nStringCount )     // suggestions available...
    {
        Image aImage;
        OUString aSuggestionImageUrl;

        if (bUseImagesInMenus)
        {
            uno::Reference< lang::XServiceInfo > xInfo( rResult.xProofreader, uno::UNO_QUERY );
            if (xInfo.is())
            {
                aSuggestionImageUrl = SvtLinguConfig().GetSpellAndGrammarContextSuggestionImage( xInfo->getImplementationName() );
                aImage = Image( aSuggestionImageUrl );
            }
        }

        sal_uInt16 nItemId = MN_SUGGESTION_START;
        for (sal_Int32 i = 0;  i < nStringCount;  ++i)
        {
            const OUString aEntry = m_aSuggestions[ i ];
            InsertItem(nItemId, aEntry, MenuItemBits::NONE, OString(), nPos++);
            SetHelpId( nItemId, HID_LINGU_REPLACE );
            if (!aSuggestionImageUrl.isEmpty())
                SetItemImage( nItemId, aImage );

            ++nItemId;
        }
        InsertSeparator(OString(), nPos++);
    }

    OUString aIgnoreSelection( SW_RES( STR_IGNORE_SELECTION ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( ".uno:SpellingAndGrammarDialog" );
    SetItemText( MN_SPELLING_DLG, aSpellingAndGrammar );
    sal_uInt16 nItemPos = GetItemPos( MN_IGNORE_WORD );
    InsertItem( MN_IGNORE_SELECTION, aIgnoreSelection, MenuItemBits::NONE, OString(), nItemPos );
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, false );
    EnableItem( MN_AUTO_CORRECT_DLG, false );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    m_nGuessLangWord = LANGUAGE_NONE;
    m_nGuessLangPara = LANGUAGE_NONE;
    if (xLG.is())
    {
        m_nGuessLangPara = EditView::CheckLanguage( rParaText, ::GetSpellChecker(), xLG, true );
    }
    if (m_nGuessLangWord != LANGUAGE_NONE || m_nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (m_nGuessLangWord == LANGUAGE_NONE)
            m_nGuessLangWord = m_nGuessLangPara;
        if (m_nGuessLangPara == LANGUAGE_NONE)
            m_nGuessLangPara = m_nGuessLangWord;
    }

    EnableItem( MN_ADD_TO_DIC, false );
    EnableItem( MN_ADD_TO_DIC_SINGLE, false );

    //ADD NEW LANGUAGE MENU ITEM

    OUString aScriptTypesInUse( OUString::number( static_cast<int>(pWrtSh->GetScriptType()) ) );

    // get keyboard language
    OUString aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

    // get the language that is in use
    OUString aCurrentLang("*");
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = SvtLanguageTable::GetLanguageString(m_nGuessLangWord);

    PopupMenu *pMenu = GetPopupMenu(MN_SET_LANGUAGE_SELECTION);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, m_aLangTable_Text );
    EnableItem( MN_SET_LANGUAGE_SELECTION );

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_PARAGRAPH);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, m_aLangTable_Paragraph );
    EnableItem( MN_SET_LANGUAGE_PARAGRAPH );

    if (bUseImagesInMenus)
    {
        uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface();
        Image rImg = ::GetImage( xFrame, ".uno:SpellingAndGrammarDialog", false );
        SetItemImage( MN_SPELLING_DLG, rImg );
    }

    checkRedline();
    RemoveDisabledEntries( true, true );
}

void SwSpellPopup::checkRedline()
{
    // Let SwView::GetState() already has the logic on when to disable the
    // accept/reject and the next/prev change items, let it do the decision.

    // Build an item set that contains a void item for each menu entry. The
    // WhichId of each item is set, so SwView may clear it.
    static const sal_uInt16 pRedlineIds[] = {
        FN_REDLINE_ACCEPT_DIRECT,
        FN_REDLINE_REJECT_DIRECT,
        FN_REDLINE_NEXT_CHANGE,
        FN_REDLINE_PREV_CHANGE
    };
    SwDoc *pDoc = m_pSh->GetDoc();
    SfxItemSet aSet(pDoc->GetAttrPool(), FN_REDLINE_ACCEPT_DIRECT, FN_REDLINE_PREV_CHANGE);
    for (size_t i = 0; i < SAL_N_ELEMENTS(pRedlineIds); ++i)
    {
        const sal_uInt16 nWhich = pRedlineIds[i];
        aSet.Put(SfxVoidItem(nWhich), nWhich);
    }
    m_pSh->GetView().GetState(aSet);

    // Enable/disable items based on if the which id of the void items are
    // cleared or not.
    for (size_t i = 0; i < SAL_N_ELEMENTS(pRedlineIds); ++i)
    {
        const sal_uInt16 nWhich = pRedlineIds[i];
        EnableItem(nWhich, aSet.Get(nWhich).Which());
    }
}

sal_uInt16  SwSpellPopup::Execute( const Rectangle& rWordPos, vcl::Window* pWin )
{
    sal_uInt16 nRet = PopupMenu::Execute(pWin, pWin->LogicToPixel(rWordPos));
    Execute( nRet );
    return nRet;
}

void SwSpellPopup::Execute( sal_uInt16 nId )
{
    if (nId == USHRT_MAX)
        return;

    if (/*m_bGrammarResults && */nId == MN_SHORT_COMMENT)
        return;     // nothing to do since it is the error message (short comment)

    if ((MN_SUGGESTION_START <= nId && nId <= MN_SUGGESTION_END) ||
        (MN_AUTOCORR_START <= nId && nId <= MN_AUTOCORR_END))
    {
        sal_Int32 nAltIdx = (MN_SUGGESTION_START <= nId && nId <= MN_SUGGESTION_END) ?
                nId - MN_SUGGESTION_START : nId - MN_AUTOCORR_START;
        OSL_ENSURE( 0 <= nAltIdx && nAltIdx < m_aSuggestions.getLength(), "index out of range" );
        if (0 <= nAltIdx && nAltIdx < m_aSuggestions.getLength() && (m_bGrammarResults || m_xSpellAlt.is()))
        {
            bool bOldIns = m_pSh->IsInsMode();
            m_pSh->SetInsMode();

            OUString aTmp( m_aSuggestions[ nAltIdx ] );
            OUString aOrig( m_bGrammarResults ? OUString() : m_xSpellAlt->getWord() );

            // if original word has a trailing . (likely the end of a sentence)
            // and the replacement text hasn't, then add it to the replacement
            if (!aTmp.isEmpty() && !aOrig.isEmpty() &&
                aOrig.endsWith(".") && /* !IsAlphaNumeric ??*/
                !aTmp.endsWith("."))
            {
                aTmp += ".";
            }

            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, m_pSh->GetCursorDescr());
            aRewriter.AddRule(UndoArg2, OUString(SW_RES(STR_YIELDS)));

            OUString aTmpStr( SW_RES(STR_START_QUOTE) );
            aTmpStr += aTmp;
            aTmpStr += OUString(SW_RES(STR_END_QUOTE));
            aRewriter.AddRule(UndoArg3, aTmpStr);

            m_pSh->StartUndo(UNDO_UI_REPLACE, &aRewriter);
            m_pSh->StartAction();
            m_pSh->DelLeft();

            m_pSh->Insert( aTmp );

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/

            // record only if it's NOT already present in autocorrection
            SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();

            OUString aOrigWord( m_bGrammarResults ? OUString() : m_xSpellAlt->getWord() ) ;
            OUString aNewWord( m_aSuggestions[ nAltIdx ] );
            SvxPrepareAutoCorrect( aOrigWord, aNewWord );

            if (MN_AUTOCORR_START <= nId && nId <= MN_AUTOCORR_END)
                pACorr->PutText( aOrigWord, aNewWord, m_nCheckedLanguage );

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/
            m_pSh->EndAction();
            m_pSh->EndUndo();

            m_pSh->SetInsMode( bOldIns );
        }
    }
    else if (nId == MN_SPELLING_DLG)
    {
        if (m_bGrammarResults)
        {
            SvtLinguConfig().SetProperty( UPN_IS_GRAMMAR_INTERACTIVE, uno::makeAny( true ));
        }
        m_pSh->Left(CRSR_SKIP_CHARS, false, 1, false );
        {
            uno::Reference<linguistic2::XSearchableDictionaryList> xDictionaryList( SvxGetDictionaryList() );
            SvxDicListChgClamp aClamp( xDictionaryList );
            m_pSh->GetView().GetViewFrame()->GetDispatcher()->
                Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON );
        }
    }
    else if (nId == MN_AUTO_CORRECT_DLG)
    {
        m_pSh->GetView().GetViewFrame()->GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
    }
    else if (nId == MN_IGNORE_SELECTION)
    {
        SwPaM *pPaM = m_pSh->GetCursor();
        if (pPaM)
            SwEditShell::IgnoreGrammarErrorAt( *pPaM );
    }
    else if (nId == MN_IGNORE_WORD)
    {
        uno::Reference< linguistic2::XDictionary > xDictionary( SvxGetIgnoreAllList(), uno::UNO_QUERY );
        if (m_bGrammarResults) {
            try
            {
                m_xGrammarResult.xProofreader->ignoreRule(
                    m_xGrammarResult.aErrors[ m_nGrammarError ].aRuleIdentifier,
                        m_xGrammarResult.aLocale );
                // refresh the layout of the actual paragraph (faster)
                SwPaM *pPaM = m_pSh->GetCursor();
                if (pPaM)
                    SwEditShell::IgnoreGrammarErrorAt( *pPaM );
                // refresh the layout of all paragraphs (workaround to launch a dictionary event)
                xDictionary->setActive(sal_False);
                xDictionary->setActive(sal_True);
            }
            catch( const uno::Exception& )
            {
            }
        } else {
            OUString sWord(m_xSpellAlt->getWord());
            linguistic::DictionaryError nAddRes = linguistic::AddEntryToDic( xDictionary,
                    sWord, false, OUString(), LANGUAGE_NONE );
            if (linguistic::DictionaryError::NONE != nAddRes && !xDictionary->getEntry(sWord).is())
            {
                SvxDicError(&m_pSh->GetView().GetViewFrame()->GetWindow(), nAddRes);
            }
        }
    }
    else if ((MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END) || nId == MN_ADD_TO_DIC_SINGLE)
    {
        OUString sWord( m_xSpellAlt->getWord() );
        OUString aDicName;

        if (MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END)
        {
            PopupMenu *pMenu = GetPopupMenu(MN_ADD_TO_DIC);
            aDicName = pMenu->GetItemText(nId);
        }
        else
            aDicName = m_aDicNameSingle;

        uno::Reference< linguistic2::XDictionary >      xDic;
        uno::Reference< linguistic2::XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );
        if (xDicList.is())
            xDic = xDicList->getDictionaryByName( aDicName );

        if (xDic.is())
        {
            linguistic::DictionaryError nAddRes = linguistic::AddEntryToDic(xDic, sWord, false, OUString(), LANGUAGE_NONE);
            // save modified user-dictionary if it is persistent
            uno::Reference< frame::XStorable >  xSavDic( xDic, uno::UNO_QUERY );
            if (xSavDic.is())
                xSavDic->store();

            if (linguistic::DictionaryError::NONE != nAddRes && !xDic->getEntry(sWord).is())
            {
                SvxDicError(&m_pSh->GetView().GetViewFrame()->GetWindow(), nAddRes);
            }
        }
    }
    else if ( nId == MN_EXPLANATION_LINK && !m_sExplanationLink.isEmpty() )
    {
        try
        {
            uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute(
                css::system::SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );
            xSystemShellExecute->execute( m_sExplanationLink, OUString(),
                    css::system::SystemShellExecuteFlags::URIS_ONLY );
        }
        catch (const uno::Exception&)
        {
            uno::Any exc( ::cppu::getCaughtException() );
            OUString msg( ::comphelper::anyToString( exc ) );
            const SolarMutexGuard guard;
            ScopedVclPtrInstance< MessageDialog > aErrorBox(nullptr, msg);
            aErrorBox->SetText( "Explanations" );
            aErrorBox->Execute();
        }
    }
    else if (nId == FN_REDLINE_ACCEPT_DIRECT || nId == FN_REDLINE_REJECT_DIRECT
            || nId == FN_REDLINE_NEXT_CHANGE || nId == FN_REDLINE_PREV_CHANGE)
    {
        // Let SwView::Execute() handle the redline actions.
        SfxRequest aReq(m_pSh->GetView().GetViewFrame(), nId);
        m_pSh->GetView().Execute(aReq);
    }
    else
    {
        // Set language for selection or for paragraph...

        SfxItemSet aCoreSet( m_pSh->GetView().GetPool(),
                    RES_CHRATR_LANGUAGE,        RES_CHRATR_LANGUAGE,
                    RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CTL_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE,
                    0 );
        OUString aNewLangText;

        if (MN_SET_LANGUAGE_SELECTION_START <= nId && nId <= MN_SET_LANGUAGE_SELECTION_END)
        {
            //Set language for current selection
            aNewLangText = m_aLangTable_Text[nId];
            SwLangHelper::SetLanguage( *m_pSh, aNewLangText, true, aCoreSet );
        }
        else if (nId == MN_SET_SELECTION_NONE)
        {
            //Set Language_None for current selection
            SwLangHelper::SetLanguage_None( *m_pSh, true, aCoreSet );
        }
        else if (nId == MN_SET_SELECTION_RESET)
        {
            //reset languages for current selection
            SwLangHelper::ResetLanguages( *m_pSh, true );
        }
        else if (nId == MN_SET_SELECTION_MORE)
        {
            //Open Format/Character Dialog
            sw_CharDialog( *m_pSh, true, SID_ATTR_CHAR_FONT, nullptr, nullptr );
        }
        else if (MN_SET_LANGUAGE_PARAGRAPH_START <= nId && nId <= MN_SET_LANGUAGE_PARAGRAPH_END)
        {
            //Set language for current paragraph
            aNewLangText = m_aLangTable_Paragraph[nId];
            m_pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *m_pSh );
            SwLangHelper::SetLanguage( *m_pSh, aNewLangText, true, aCoreSet );
            m_pSh->Pop( false );  // restore cursor
        }
        else if (nId == MN_SET_PARA_NONE)
        {
            //Set Language_None for current paragraph
            m_pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *m_pSh );
            SwLangHelper::SetLanguage_None( *m_pSh, true, aCoreSet );
            m_pSh->Pop( false );  // restore cursor
        }
        else if (nId == MN_SET_PARA_RESET)
        {
            //reset languages for current paragraph
            m_pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *m_pSh );
            SwLangHelper::ResetLanguages( *m_pSh, true );
            m_pSh->Pop( false );  // restore cursor
        }
        else if (nId == MN_SET_PARA_MORE)
        {
            m_pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *m_pSh );
            //Open Format/Character Dialog
            sw_CharDialog( *m_pSh, true, SID_ATTR_CHAR_FONT, nullptr, nullptr );
            m_pSh->Pop( false );  // restore cursor
        }
    }

    m_pSh->EnterStdMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
