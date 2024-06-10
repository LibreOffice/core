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

#include <SwRewriter.hxx>
#include <cmdid.h>
#include <strings.hrc>
#include <doc.hxx>
#include <edtwin.hxx>
#include <helpids.h>
#include <langhelper.hxx>
#include <bitmaps.hlst>
#include <olmenu.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <swundo.hxx>
#include <utility>
#include <view.hxx>
#include <wrtsh.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editview.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <rtl/string.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/voiditem.hxx>
#include <svtools/langtab.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <map>

#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

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
static bool lcl_checkScriptType( SvtScriptType nScriptType, LanguageType nLang )
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
    uno::Reference< css::frame::XController > xController = pWrtSh->GetView().GetViewFrame().GetFrame().GetFrameInterface()->getController();
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
        const uno::Sequence< lang::Locale > rLocales( xDocumentLanguages->getDocumentLanguages( static_cast<sal_Int16>(nScriptType), nMaxCount ) );
        for (const lang::Locale& rLocale : rLocales)
        {
            if (aLangItems.size() == size_t(nMaxCount))
                break;
            if (lcl_checkScriptType( nScriptType, SvtLanguageTable::GetLanguageType( rLocale.Language )))
                aLangItems.insert( rLocale.Language );
        }
    }

    sal_uInt16 nItemId = nLangItemIdStart;
    for (const OUString& aEntryText : aLangItems)
    {
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

    pPopupMenu->InsertItem( nLangItemIdStart + MN_NONE_OFFSET,  SwResId( STR_LANGSTATUS_NONE ), MenuItemBits::RADIOCHECK );
    if ( SvtLanguageTable::GetLanguageString( LANGUAGE_NONE ) == aCurLang )
        pPopupMenu->CheckItem( nLangItemIdStart + MN_NONE_OFFSET );

    pPopupMenu->InsertItem( nLangItemIdStart + MN_RESET_OFFSET, SwResId( STR_RESET_TO_DEFAULT_LANGUAGE ) );
    pPopupMenu->InsertItem( nLangItemIdStart + MN_MORE_OFFSET,  SwResId( STR_LANGSTATUS_MORE ) );
}

SwSpellPopup::SwSpellPopup(
        SwWrtShell* pWrtSh,
        uno::Reference< linguistic2::XSpellAlternatives > xAlt,
        const OUString &rParaText)
    : m_aBuilder(nullptr, AllSettings::GetUIRootDir(), u"modules/swriter/ui/spellmenu.ui"_ustr, u""_ustr)
    , m_xPopupMenu(m_aBuilder.get_menu(u"menu"))
    , m_nIgnoreWordId(m_xPopupMenu->GetItemId(u"ignoreall"))
    , m_nAddMenuId(m_xPopupMenu->GetItemId(u"addmenu"))
    , m_nAddId(m_xPopupMenu->GetItemId(u"add"))
    , m_nSpellDialogId(m_xPopupMenu->GetItemId(u"spelldialog"))
    , m_nCorrectMenuId(m_xPopupMenu->GetItemId(u"correctmenu"))
    , m_nCorrectDialogId(m_xPopupMenu->GetItemId(u"correctdialog"))
    , m_nLangSelectionMenuId(m_xPopupMenu->GetItemId(u"langselection"))
    , m_nLangParaMenuId(m_xPopupMenu->GetItemId(u"langpara"))
    , m_nRedlineAcceptId(m_xPopupMenu->GetItemId(u"accept"))
    , m_nRedlineRejectId(m_xPopupMenu->GetItemId(u"reject"))
    , m_nRedlineNextId(m_xPopupMenu->GetItemId(u"next"))
    , m_nRedlinePrevId(m_xPopupMenu->GetItemId(u"prev"))
    , m_pSh( pWrtSh )
    , m_xSpellAlt(std::move(xAlt))
    , m_bGrammarResults(false)
{
    OSL_ENSURE(m_xSpellAlt.is(), "no spelling alternatives available");

    m_xPopupMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    m_nCheckedLanguage = LANGUAGE_NONE;
    css::uno::Sequence< OUString > aSuggestions;
    if (m_xSpellAlt.is())
    {
        m_nCheckedLanguage = LanguageTag( m_xSpellAlt->getLocale() ).getLanguageType();
        aSuggestions = m_xSpellAlt->getAlternatives();
    }
    sal_Int16 nStringCount = static_cast< sal_Int16 >( aSuggestions.getLength() );

    SvtLinguConfig aCfg;

    PopupMenu *pMenu = m_xPopupMenu->GetPopupMenu(m_nCorrectMenuId);
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

        m_xPopupMenu->InsertSeparator({}, 0);
        bEnable = true;
        sal_uInt16 nAutoCorrItemId  = MN_AUTOCORR_START;
        sal_uInt16 nItemId          = MN_SUGGESTION_START;
        for (sal_uInt16 i = 0; i < nStringCount; ++i)
        {
            const OUString aEntry = aSuggestions[ i ];
            m_xPopupMenu->InsertItem(nItemId, aEntry, MenuItemBits::NONE, {}, i);
            m_xPopupMenu->SetHelpId(nItemId, HID_LINGU_REPLACE);
            if (!aSuggestionImageUrl.isEmpty())
                m_xPopupMenu->SetItemImage(nItemId, aImage);

            pMenu->InsertItem( nAutoCorrItemId, aEntry );
            pMenu->SetHelpId( nAutoCorrItemId, HID_LINGU_AUTOCORR);

            ++nAutoCorrItemId;
            ++nItemId;
        }
    }

    uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame().GetFrame().GetFrameInterface();
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));

    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:SpellingAndGrammarDialog"_ustr, aModuleName);
        m_xPopupMenu->SetItemText(m_nSpellDialogId,
            vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties));
    }
    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:AutoCorrectDlg"_ustr, aModuleName);
        m_xPopupMenu->SetItemText(m_nCorrectDialogId,
            vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties));
    }

    sal_uInt16 nItemPos = m_xPopupMenu->GetItemPos(m_nIgnoreWordId);
    OUString aIgnoreSelection( SwResId( STR_IGNORE_SELECTION ) );
    m_xPopupMenu->InsertItem(MN_IGNORE_SELECTION, aIgnoreSelection, MenuItemBits::NONE, {}, nItemPos);
    m_xPopupMenu->SetHelpId(MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    m_xPopupMenu->EnableItem(m_nCorrectMenuId, bEnable);

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    LanguageType nGuessLangWord = LANGUAGE_NONE;
    LanguageType nGuessLangPara = LANGUAGE_NONE;
    if (m_xSpellAlt.is() && xLG.is())
    {
        nGuessLangWord = EditView::CheckLanguage( m_xSpellAlt->getWord(), ::GetSpellChecker(), xLG, false );
        nGuessLangPara = EditView::CheckLanguage( rParaText, ::GetSpellChecker(), xLG, true );
    }
    if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (nGuessLangWord == LANGUAGE_NONE)
            nGuessLangWord = nGuessLangPara;
        if (nGuessLangPara == LANGUAGE_NONE)
            nGuessLangPara = nGuessLangWord;
    }

    pMenu = m_xPopupMenu->GetPopupMenu(m_nAddMenuId);
    pMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);     //! necessary to retrieve the correct dictionary name in 'Execute' below
    uno::Reference< linguistic2::XSearchableDictionaryList >    xDicList( LinguMgr::GetDictionaryList() );
    sal_uInt16 nItemId = MN_DICTIONARIES_START;
    if (xDicList.is())
    {
        // add the default positive dictionary to dic-list (if not already done).
        // This is to ensure that there is at least one dictionary to which
        // words could be added.
        uno::Reference< linguistic2::XDictionary >  xDic( LinguMgr::GetStandardDic() );
        if (xDic.is())
            xDic->setActive( true );

        m_aDics = xDicList->getDictionaries();

        for (const uno::Reference<linguistic2::XDictionary>& rDic : m_aDics)
        {
            uno::Reference< linguistic2::XDictionary >  xDicTmp = rDic;
            if (!xDicTmp.is() || LinguMgr::GetIgnoreAllList() == xDicTmp)
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
    m_xPopupMenu->EnableItem(m_nAddMenuId, (nItemId - MN_DICTIONARIES_START) > 1);
    m_xPopupMenu->EnableItem(m_nAddId, (nItemId - MN_DICTIONARIES_START) == 1);

    //ADD NEW LANGUAGE MENU ITEM

    OUString aScriptTypesInUse( OUString::number( static_cast<int>(pWrtSh->GetScriptType()) ) );

    // get keyboard language
    OUString aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

    // get the language that is in use
    OUString aCurrentLang(u"*"_ustr);
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq{ aCurrentLang,
                                    aScriptTypesInUse,
                                    aKeyboardLang,
                                    SvtLanguageTable::GetLanguageString(nGuessLangWord) };

    pMenu = m_xPopupMenu->GetPopupMenu(m_nLangSelectionMenuId);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, m_aLangTable_Text );
    m_xPopupMenu->EnableItem(m_nLangSelectionMenuId);

    pMenu = m_xPopupMenu->GetPopupMenu(m_nLangParaMenuId);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, m_aLangTable_Paragraph );
    m_xPopupMenu->EnableItem(m_nLangParaMenuId);

    if (bUseImagesInMenus)
        m_xPopupMenu->SetItemImage(m_nSpellDialogId,
            vcl::CommandInfoProvider::GetImageForCommand(u".uno:SpellingAndGrammarDialog"_ustr, xFrame));

    checkRedline();
    m_xPopupMenu->RemoveDisabledEntries( true );

    InitItemCommands(aSuggestions);
}

SwSpellPopup::SwSpellPopup(
    SwWrtShell *pWrtSh,
    const linguistic2::ProofreadingResult &rResult,
    sal_Int32 nErrorInResult,
    const uno::Sequence< OUString > &rSuggestions,
    const OUString &rParaText )
    : m_aBuilder(nullptr, AllSettings::GetUIRootDir(), u"modules/swriter/ui/spellmenu.ui"_ustr, u""_ustr)
    , m_xPopupMenu(m_aBuilder.get_menu(u"menu"))
    , m_nIgnoreWordId(m_xPopupMenu->GetItemId(u"ignoreall"))
    , m_nAddMenuId(m_xPopupMenu->GetItemId(u"addmenu"))
    , m_nAddId(m_xPopupMenu->GetItemId(u"add"))
    , m_nSpellDialogId(m_xPopupMenu->GetItemId(u"spelldialog"))
    , m_nCorrectMenuId(m_xPopupMenu->GetItemId(u"correctmenu"))
    , m_nCorrectDialogId(m_xPopupMenu->GetItemId(u"correctdialog"))
    , m_nLangSelectionMenuId(m_xPopupMenu->GetItemId(u"langselection"))
    , m_nLangParaMenuId(m_xPopupMenu->GetItemId(u"langpara"))
    , m_nRedlineAcceptId(m_xPopupMenu->GetItemId(u"accept"))
    , m_nRedlineRejectId(m_xPopupMenu->GetItemId(u"reject"))
    , m_nRedlineNextId(m_xPopupMenu->GetItemId(u"next"))
    , m_nRedlinePrevId(m_xPopupMenu->GetItemId(u"prev"))
    , m_pSh(pWrtSh)
    , m_bGrammarResults(true)
{
    m_nCheckedLanguage = LanguageTag::convertToLanguageType( rResult.aLocale );
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    sal_uInt16 nPos = 0;
    OUString aMessageText( rResult.aErrors[ nErrorInResult ].aShortComment );
    m_xPopupMenu->InsertSeparator({}, nPos++);
    m_xPopupMenu->InsertItem(MN_SHORT_COMMENT, aMessageText, MenuItemBits::NOSELECT, {}, nPos++);
    if (bUseImagesInMenus)
        m_xPopupMenu->SetItemImage(MN_SHORT_COMMENT, Image(StockImage::Yes, BMP_INFO_16));

    // Add an item to show detailed infos if the FullCommentURL property is defined
    const beans::PropertyValues  aProperties = rResult.aErrors[ nErrorInResult ].aProperties;
    for ( const auto& rProp : aProperties )
    {
        if ( rProp.Name == "FullCommentURL" )
        {
            uno::Any aValue = rProp.Value;
            aValue >>= m_sExplanationLink;

            if ( !m_sExplanationLink.isEmpty( ) )
                break;
        }
    }

    if ( !m_sExplanationLink.isEmpty( ) )
    {
        m_xPopupMenu->InsertItem(MN_EXPLANATION_LINK, SwResId(STR_EXPLANATION_LINK), MenuItemBits::TEXT | MenuItemBits::HELP, {}, nPos++);
    }

    m_xPopupMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);

    m_xPopupMenu->InsertSeparator({}, nPos++);
    if ( rSuggestions.hasElements() )     // suggestions available...
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
        for (const OUString& aEntry : rSuggestions)
        {
            m_xPopupMenu->InsertItem(nItemId, aEntry, MenuItemBits::NONE, {}, nPos++);
            m_xPopupMenu->SetHelpId(nItemId, HID_LINGU_REPLACE);
            if (!aSuggestionImageUrl.isEmpty())
                m_xPopupMenu->SetItemImage(nItemId, aImage);

            ++nItemId;
        }
        m_xPopupMenu->InsertSeparator({}, nPos++);
    }

    uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame().GetFrame().GetFrameInterface();
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));

    OUString aIgnoreSelection( SwResId( STR_IGNORE_SELECTION ) );
    auto aCommandProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:SpellingAndGrammarDialog"_ustr, aModuleName);
    m_xPopupMenu->SetItemText(m_nSpellDialogId,
        vcl::CommandInfoProvider::GetPopupLabelForCommand(aCommandProperties));
    sal_uInt16 nItemPos = m_xPopupMenu->GetItemPos(m_nIgnoreWordId);
    m_xPopupMenu->InsertItem(MN_IGNORE_SELECTION, aIgnoreSelection, MenuItemBits::NONE, {}, nItemPos);
    m_xPopupMenu->SetHelpId(MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    m_xPopupMenu->EnableItem(m_nCorrectMenuId, false);
    m_xPopupMenu->EnableItem(m_nCorrectDialogId, false);

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    LanguageType nGuessLangWord = LANGUAGE_NONE;
    LanguageType nGuessLangPara = LANGUAGE_NONE;
    if (xLG.is())
    {
        nGuessLangPara = EditView::CheckLanguage( rParaText, ::GetSpellChecker(), xLG, true );
    }
    if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (nGuessLangWord == LANGUAGE_NONE)
            nGuessLangWord = nGuessLangPara;
        if (nGuessLangPara == LANGUAGE_NONE)
            nGuessLangPara = nGuessLangWord;
    }

    m_xPopupMenu->EnableItem(m_nAddMenuId, false);
    m_xPopupMenu->EnableItem(m_nAddId, false);

    //ADD NEW LANGUAGE MENU ITEM

    OUString aScriptTypesInUse( OUString::number( static_cast<int>(pWrtSh->GetScriptType()) ) );

    // get keyboard language
    OUString aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

    // get the language that is in use
    OUString aCurrentLang(u"*"_ustr);
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq{ aCurrentLang,
                                    aScriptTypesInUse,
                                    aKeyboardLang,
                                    SvtLanguageTable::GetLanguageString(nGuessLangWord) };

    PopupMenu *pMenu = m_xPopupMenu->GetPopupMenu(m_nLangSelectionMenuId);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, m_aLangTable_Text );
    m_xPopupMenu->EnableItem(m_nLangSelectionMenuId);

    pMenu = m_xPopupMenu->GetPopupMenu(m_nLangParaMenuId);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, m_aLangTable_Paragraph );
    m_xPopupMenu->EnableItem(m_nLangParaMenuId);

    if (bUseImagesInMenus)
        m_xPopupMenu->SetItemImage(m_nSpellDialogId,
            vcl::CommandInfoProvider::GetImageForCommand(u".uno:SpellingAndGrammarDialog"_ustr, xFrame));

    checkRedline();
    m_xPopupMenu->RemoveDisabledEntries(true);

    SvtLinguConfig().SetProperty( UPN_IS_GRAMMAR_INTERACTIVE, uno::Any( true ));

    InitItemCommands(rSuggestions);
}

SwSpellPopup::~SwSpellPopup() {}

void SwSpellPopup::InitItemCommands(const css::uno::Sequence< OUString >& aSuggestions)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    // None is added only for LOK, it means there is no need to execute anything
    m_xPopupMenu->SetItemCommand(MN_SHORT_COMMENT, u".uno:None"_ustr);
    m_xPopupMenu->SetItemCommand(m_nSpellDialogId, u".uno:SpellingAndGrammarDialog"_ustr);
    if(m_bGrammarResults)
        m_xPopupMenu->SetItemCommand(m_nIgnoreWordId, u".uno:SpellCheckIgnoreAll?Type:string=Grammar"_ustr);
    else
        m_xPopupMenu->SetItemCommand(m_nIgnoreWordId, u".uno:SpellCheckIgnoreAll?Type:string=Spelling"_ustr);
    if(m_bGrammarResults)
        m_xPopupMenu->SetItemCommand(MN_IGNORE_SELECTION, u".uno:SpellCheckIgnore?Type:string=Grammar"_ustr);
    else
        m_xPopupMenu->SetItemCommand(MN_IGNORE_SELECTION, u".uno:SpellCheckIgnore?Type:string=Spelling"_ustr);

    for(int i = 0; i < aSuggestions.getLength(); ++i)
    {
        sal_uInt16 nItemId = MN_SUGGESTION_START + i;
        OUString sCommandString = u".uno:SpellCheckApplySuggestion?ApplyRule:string="_ustr;
        if(m_bGrammarResults)
            sCommandString += "Grammar_";
        else if (m_xSpellAlt.is())
            sCommandString += "Spelling_";
        sCommandString += m_xPopupMenu->GetItemText(nItemId);
        m_xPopupMenu->SetItemCommand(nItemId, sCommandString);
    }

    PopupMenu *pMenu = m_xPopupMenu->GetPopupMenu(m_nLangSelectionMenuId);
    m_xPopupMenu->SetItemCommand(m_nLangSelectionMenuId, u".uno:SetSelectionLanguageMenu"_ustr);
    if(pMenu)
    {
        for (const auto& item : m_aLangTable_Text)
        {
            OUString sCommandString = ".uno:LanguageStatus?Language:string=Current_" + item.second;
            pMenu->SetItemCommand(item.first, sCommandString);
        }

        pMenu->SetItemCommand(MN_SET_SELECTION_NONE, u".uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE"_ustr);
        pMenu->SetItemCommand(MN_SET_SELECTION_RESET, u".uno:LanguageStatus?Language:string=Current_RESET_LANGUAGES"_ustr);
        pMenu->SetItemCommand(MN_SET_SELECTION_MORE, u".uno:FontDialog?Page:string=font"_ustr);
    }

    pMenu = m_xPopupMenu->GetPopupMenu(m_nLangParaMenuId);
    m_xPopupMenu->SetItemCommand(m_nLangParaMenuId, u".uno:SetParagraphLanguageMenu"_ustr);
    if(pMenu)
    {
        for (const auto& item : m_aLangTable_Paragraph)
        {
            OUString sCommandString = ".uno:LanguageStatus?Language:string=Paragraph_" + item.second;
            pMenu->SetItemCommand(item.first, sCommandString);
        }

        pMenu->SetItemCommand(MN_SET_PARA_NONE, u".uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE"_ustr);
        pMenu->SetItemCommand(MN_SET_PARA_RESET, u".uno:LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES"_ustr);
        pMenu->SetItemCommand(MN_SET_PARA_MORE, u".uno:FontDialogForParagraph"_ustr);
    }
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
    SfxItemSetFixed<FN_REDLINE_ACCEPT_DIRECT, FN_REDLINE_PREV_CHANGE> aSet(pDoc->GetAttrPool());
    for (sal_uInt16 nWhich : pRedlineIds)
    {
        aSet.Put(SfxVoidItem(nWhich));
    }
    m_pSh->GetView().GetState(aSet);

    // Enable/disable items based on if the which id of the void items are
    // cleared or not.
    for (sal_uInt16 nWhich : pRedlineIds)
    {
        sal_uInt16 nId(0);
        if (nWhich == FN_REDLINE_ACCEPT_DIRECT)
            nId = m_nRedlineAcceptId;
        else if (nWhich == FN_REDLINE_REJECT_DIRECT)
            nId = m_nRedlineRejectId;
        else if (nWhich == FN_REDLINE_NEXT_CHANGE)
            nId = m_nRedlineNextId;
        else if (nWhich == FN_REDLINE_PREV_CHANGE)
            nId = m_nRedlinePrevId;
        m_xPopupMenu->EnableItem(nId, aSet.Get(nWhich).Which() != 0);
    }
}

void SwSpellPopup::Execute( const tools::Rectangle& rWordPos, vcl::Window* pWin )
{
    sal_uInt16 nRet = m_xPopupMenu->Execute(pWin, pWin->LogicToPixel(rWordPos));
    Execute( nRet );
}

void SwSpellPopup::Execute( sal_uInt16 nId )
{
    if (nId == USHRT_MAX)
        return;

    if (/*m_bGrammarResults && */nId == MN_SHORT_COMMENT)
        return;     // nothing to do since it is the error message (short comment)

    if (MN_SUGGESTION_START <= nId && nId <= MN_SUGGESTION_END)
    {
        OUString sApplyRule(u""_ustr);
        if(m_bGrammarResults)
            sApplyRule += "Grammar_";
        else if (m_xSpellAlt.is())
            sApplyRule += "Spelling_";
        sApplyRule += m_xPopupMenu->GetItemText(nId);

        SfxStringItem aApplyItem(FN_PARAM_1, sApplyRule);
        m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_SPELLCHECK_APPLY_SUGGESTION, SfxCallMode::SYNCHRON, { &aApplyItem });
    }
    else if(MN_AUTOCORR_START <= nId && nId <= MN_AUTOCORR_END)
    {
        if (m_xSpellAlt.is())
        {
            bool bOldIns = m_pSh->IsInsMode();
            m_pSh->SetInsMode();

            PopupMenu* pMenu = m_xPopupMenu->GetPopupMenu(m_nCorrectMenuId);
            assert(pMenu);
            OUString aTmp( pMenu->GetItemText(nId) );
            OUString aOrig( m_xSpellAlt->getWord() );

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
            aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));

            OUString aTmpStr = SwResId(STR_START_QUOTE) +
                aTmp + SwResId(STR_END_QUOTE);
            aRewriter.AddRule(UndoArg3, aTmpStr);

            m_pSh->StartUndo(SwUndoId::UI_REPLACE, &aRewriter);
            m_pSh->StartAction();

            m_pSh->Replace(aTmp, false);

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/

            // record only if it's NOT already present in autocorrection
            SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();

            OUString aOrigWord( m_xSpellAlt->getWord() ) ;
            OUString aNewWord( pMenu->GetItemText(nId) );
            SvxPrepareAutoCorrect( aOrigWord, aNewWord );

            pACorr->PutText( aOrigWord, aNewWord, m_nCheckedLanguage );

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/
            m_pSh->EndAction();
            m_pSh->EndUndo();

            m_pSh->SetInsMode( bOldIns );
        }
    }
    else if (nId == m_nSpellDialogId)
    {
        m_pSh->Left(SwCursorSkipMode::Chars, false, 1, false );
        {
            m_pSh->GetView().GetViewFrame().GetDispatcher()->
                Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON );
        }
    }
    else if (nId == m_nCorrectDialogId)
    {
        m_pSh->GetView().GetViewFrame().GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
    }
    else if (nId == MN_IGNORE_SELECTION)
    {
        SfxStringItem aIgnoreString(FN_PARAM_1, m_bGrammarResults ? u"Grammar"_ustr : u"Spelling"_ustr);
        m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_SPELLCHECK_IGNORE, SfxCallMode::SYNCHRON, { &aIgnoreString });
    }
    else if (nId == m_nIgnoreWordId)
    {
        SfxStringItem aIgnoreString(FN_PARAM_1, m_bGrammarResults ? u"Grammar"_ustr : u"Spelling"_ustr);
        m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_SPELLCHECK_IGNORE_ALL, SfxCallMode::SYNCHRON, { &aIgnoreString });
    }
    else if ((MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END) || nId == m_nAddId)
    {
        OUString sWord( m_xSpellAlt->getWord() );
        OUString aDicName;

        if (MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END)
        {
            PopupMenu *pMenu = m_xPopupMenu->GetPopupMenu(m_nAddMenuId);
            aDicName = pMenu->GetItemText(nId);
        }
        else
            aDicName = m_aDicNameSingle;

        uno::Reference< linguistic2::XDictionary >      xDic;
        uno::Reference< linguistic2::XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
        if (xDicList.is())
            xDic = xDicList->getDictionaryByName( aDicName );

        if (xDic.is())
        {
            linguistic::DictionaryError nAddRes = linguistic::AddEntryToDic(xDic, sWord, false, OUString());
            // save modified user-dictionary if it is persistent
            uno::Reference< frame::XStorable >  xSavDic( xDic, uno::UNO_QUERY );
            if (xSavDic.is())
                xSavDic->store();

            if (linguistic::DictionaryError::NONE != nAddRes && !xDic->getEntry(sWord).is())
            {
                SvxDicError(m_pSh->GetView().GetFrameWeld(), nAddRes);
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
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pSh->GetView().GetFrameWeld(),
                                                      VclMessageType::Warning, VclButtonsType::Ok, msg));
            xBox->set_title(u"Explanations"_ustr);
            xBox->run();
        }
    }
    else if (nId == m_nRedlineAcceptId || nId == m_nRedlineRejectId
            || nId == m_nRedlineNextId || nId == m_nRedlinePrevId)
    {
        if (nId == m_nRedlineAcceptId)
            nId = FN_REDLINE_ACCEPT_DIRECT;
        else if (nId == m_nRedlineRejectId)
            nId = FN_REDLINE_REJECT_DIRECT;
        else if (nId == m_nRedlineNextId)
            nId = FN_REDLINE_NEXT_CHANGE;
        else if (nId == m_nRedlinePrevId)
            nId = FN_REDLINE_PREV_CHANGE;
        // Let SwView::Execute() handle the redline actions.
        SfxRequest aReq(m_pSh->GetView().GetViewFrame(), nId);
        m_pSh->GetView().Execute(aReq);
    }
    else
    {
        if (MN_SET_LANGUAGE_SELECTION_START <= nId && nId <= MN_SET_LANGUAGE_SELECTION_END)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, "Current_" + m_aLangTable_Text[nId]);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_SELECTION_NONE)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Current_LANGUAGE_NONE"_ustr);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_SELECTION_RESET)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Current_RESET_LANGUAGES"_ustr);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_SELECTION_MORE)
        {
            SfxStringItem aDlgString(FN_PARAM_1, u"font"_ustr);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_CHAR_DLG, SfxCallMode::SYNCHRON, { &aDlgString });
        }
        else if (MN_SET_LANGUAGE_PARAGRAPH_START <= nId && nId <= MN_SET_LANGUAGE_PARAGRAPH_END)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, "Paragraph_" + m_aLangTable_Paragraph[nId]);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_PARA_NONE)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Paragraph_LANGUAGE_NONE"_ustr);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_PARA_RESET)
        {
            SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Paragraph_RESET_LANGUAGES"_ustr);
            m_pSh->GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON, { &aLangString });
        }
        else if (nId == MN_SET_PARA_MORE)
        {
            m_pSh->GetView().GetViewFrame().GetDispatcher()->Execute( SID_CHAR_DLG_FOR_PARAGRAPH );
        }
    }

    m_pSh->EnterStdMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
