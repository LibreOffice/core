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
#include "docsh.hxx"        //CheckSpellChanges
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
#include <vcl/msgbox.hxx>
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
#include <com/sun/star/frame/UICommandDescription.hpp>


using namespace ::com::sun::star;

extern void sw_CharDialog( SwWrtShell &rWrtSh, bool bUseDialog, sal_uInt16 nSlot,const SfxItemSet *pArgs, SfxRequest *pReq );


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
static inline bool lcl_checkScriptType( sal_Int16 nScriptType, LanguageType nLang )
{
    return 0 != (nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

void SwSpellPopup::fillLangPopupMenu(
    PopupMenu *pPopupMenu,
    sal_uInt16 nLangItemIdStart,
    uno::Sequence< OUString > aSeq,
    SwWrtShell* pWrtSh,
    std::map< sal_Int16, OUString > &rLangTable )
{
    if (!pPopupMenu)
        return;

    SvtLanguageTable    aLanguageTable;

    // set of languages to be displayed in the sub menus
    std::set< OUString > aLangItems;

    OUString    aCurLang( aSeq[0] );
    sal_uInt16      nScriptType = static_cast< sal_Int16 >(aSeq[1].toInt32());
    OUString    aKeyboardLang( aSeq[2] );
    OUString    aGuessedTextLang( aSeq[3] );

    if (!aCurLang.isEmpty() &&
        LANGUAGE_DONTKNOW != aLanguageTable.GetType( aCurLang ))
        aLangItems.insert( aCurLang );

    //2--System
    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguageTag().getLanguageType();
    if (rSystemLanguage != LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType( nScriptType, rSystemLanguage ))
            aLangItems.insert( aLanguageTable.GetString(rSystemLanguage) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguageTag().getLanguageType();
    if (rUILanguage != LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(nScriptType, rUILanguage ))
            aLangItems.insert( aLanguageTable.GetString(rUILanguage) );
    }

    //4--guessed language
    if (!aGuessedTextLang.isEmpty())
    {
        if (lcl_checkScriptType(nScriptType, aLanguageTable.GetType(aGuessedTextLang)))
            aLangItems.insert( aGuessedTextLang );
    }

    //5--keyboard language
    if (!aKeyboardLang.isEmpty())
    {
        if (lcl_checkScriptType(nScriptType, aLanguageTable.GetType(aKeyboardLang)))
            aLangItems.insert( aKeyboardLang );
    }

    //6--all languages used in current document
    uno::Reference< com::sun::star::frame::XModel > xModel;
    uno::Reference< com::sun::star::frame::XController > xController( pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface()->getController(), uno::UNO_QUERY );
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
        uno::Sequence< lang::Locale > rLocales( xDocumentLanguages->getDocumentLanguages( nScriptType, nMaxCount ) );
        if (rLocales.getLength() > 0)
        {
            for (sal_uInt16 i = 0; i < rLocales.getLength(); ++i)
            {
                if (aLangItems.size() == (size_t)nMaxCount)
                    break;
                const lang::Locale& rLocale = rLocales[i];
                if (lcl_checkScriptType( nScriptType, aLanguageTable.GetType( rLocale.Language )))
                    aLangItems.insert( rLocale.Language );
            }
        }
    }


    sal_uInt16 nItemId = nLangItemIdStart;
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        OUString aEntryTxt( *it );
        if (aEntryTxt != OUString( aLanguageTable.GetString( LANGUAGE_NONE ) )&&
            aEntryTxt != "*" && // multiple languages in current selection
            !aEntryTxt.isEmpty()) // 'no language found' from language guessing
        {
            OSL_ENSURE( nLangItemIdStart <= nItemId && nItemId <= nLangItemIdStart + MN_MAX_NUM_LANG,
                    "nItemId outside of expected range!" );
            pPopupMenu->InsertItem( nItemId, aEntryTxt, MIB_RADIOCHECK );
            if (aEntryTxt == aCurLang)
            {
                //make a check mark for the current language
                pPopupMenu->CheckItem( nItemId, sal_True );
            }
            rLangTable[ nItemId ] = aEntryTxt;
            ++nItemId;
        }
    }

    pPopupMenu->InsertItem( nLangItemIdStart + MN_NONE_OFFSET,  String(SW_RES( STR_LANGSTATUS_NONE )), MIB_RADIOCHECK );
    if ( aLanguageTable.GetString( LANGUAGE_NONE ) == aCurLang )
        pPopupMenu->CheckItem( nLangItemIdStart + MN_NONE_OFFSET, sal_True );

    pPopupMenu->InsertItem( nLangItemIdStart + MN_RESET_OFFSET, String(SW_RES( STR_RESET_TO_DEFAULT_LANGUAGE )), 0 );
    pPopupMenu->InsertItem( nLangItemIdStart + MN_MORE_OFFSET,  String(SW_RES( STR_LANGSTATUS_MORE )), 0 );
}


static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );
    Graphic aGraphic;
    const String aFilterName( IMP_PNG );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}


OUString RetrieveLabelFromCommand( const OUString& aCmdURL )
{
    OUString aLabel;
    if ( !aCmdURL.isEmpty() )
    {
        try
        {
            uno::Reference< container::XNameAccess > const xNameAccess(
                    frame::UICommandDescription::create(
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
                    if ( aPropSeq[i].Name == "Name" )
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
        const String &rParaText
)   : PopupMenu( SW_RES(MN_SPELL_POPUP) )
    , pSh( pWrtSh )
    , xSpellAlt(xAlt)
    , bGrammarResults(false)
{
    OSL_ENSURE(xSpellAlt.is(), "no spelling alternatives available");

    SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    nCheckedLanguage = LANGUAGE_NONE;
    if (xSpellAlt.is())
    {
        nCheckedLanguage = LanguageTag( xSpellAlt->getLocale() ).getLanguageType();
        aSuggestions = xSpellAlt->getAlternatives();
    }
    sal_Int16 nStringCount = static_cast< sal_Int16 >( aSuggestions.getLength() );

    SvtLinguConfig aCfg;

    PopupMenu *pMenu = GetPopupMenu(MN_AUTOCORR);
    pMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
    sal_Bool bEnable = sal_False;
    if( nStringCount )
    {
        Image aImage;
        OUString aSuggestionImageUrl;

        if (bUseImagesInMenus)
        {
            uno::Reference< container::XNamed > xNamed( xSpellAlt, uno::UNO_QUERY );
            if (xNamed.is())
            {
                aSuggestionImageUrl = aCfg.GetSpellAndGrammarContextSuggestionImage( xNamed->getName() );
                aImage = Image( lcl_GetImageFromPngUrl( aSuggestionImageUrl ) );
            }
        }

        InsertSeparator(OString(), 0);
        bEnable = sal_True;
        sal_uInt16 nAutoCorrItemId  = MN_AUTOCORR_START;
        sal_uInt16 nItemId          = MN_SUGGESTION_START;
        for (sal_uInt16 i = 0; i < nStringCount; ++i)
        {
            const OUString aEntry = aSuggestions[ i ];
            InsertItem(nItemId, aEntry, 0, OString(), i);
            SetHelpId( nItemId, HID_LINGU_REPLACE);
            if (!aSuggestionImageUrl.isEmpty())
                SetItemImage( nItemId, aImage );

            pMenu->InsertItem( nAutoCorrItemId, aEntry );
            pMenu->SetHelpId( nAutoCorrItemId, HID_LINGU_AUTOCORR);

            ++nAutoCorrItemId;
            ++nItemId;
        }
    }

    OUString aIgnoreSelection( String( SW_RES( STR_IGNORE_SELECTION ) ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( ".uno:SpellingAndGrammarDialog" );
    SetItemText( MN_SPELLING_DLG, aSpellingAndGrammar );
    sal_uInt16 nItemPos = GetItemPos( MN_IGNORE_WORD );
    InsertItem(MN_IGNORE_SELECTION, aIgnoreSelection, 0, OString(), nItemPos);
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, bEnable );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    nGuessLangWord = LANGUAGE_NONE;
    nGuessLangPara = LANGUAGE_NONE;
    if (xSpellAlt.is() && xLG.is())
    {
        nGuessLangWord = EditView::CheckLanguage( xSpellAlt->getWord(), ::GetSpellChecker(), xLG, false );
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

    pMenu = GetPopupMenu(MN_ADD_TO_DIC);
    pMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);     //! necessary to retrieve the correct dictionary name in 'Execute' below
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

        aDics = xDicList->getDictionaries();
        const uno::Reference< linguistic2::XDictionary >  *pDic = aDics.getConstArray();
        sal_uInt16 nDicCount = static_cast< sal_uInt16 >(aDics.getLength());

        for( sal_uInt16 i = 0; i < nDicCount; i++ )
        {
            uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
            if (!xDicTmp.is() || SvxGetIgnoreAllList() == xDicTmp)
                continue;

            uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
            LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
            if( xDicTmp->isActive()
                &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                && (nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                && (!xStor.is() || !xStor->isReadonly()) )
            {
                // the extra 1 is because of the (possible) external
                // linguistic entry above
                pMenu->InsertItem( nItemId, xDicTmp->getName() );
                aDicNameSingle = xDicTmp->getName();

                if (bUseImagesInMenus)
                {
                    uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                    if (xSvcInfo.is())
                    {
                        OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                                xSvcInfo->getImplementationName() ) );
                        if (!aDictionaryImageUrl.isEmpty())
                        {
                            Image aImage( lcl_GetImageFromPngUrl( aDictionaryImageUrl ) );
                            pMenu->SetItemImage( nItemId, aImage );
                        }
                    }
                }

                ++nItemId;
            }
        }
    }
    EnableItem( MN_ADD_TO_DIC, ((nItemId - MN_DICTIONARIES_START) > 1)?sal_True:sal_False );
    EnableItem( MN_ADD_TO_DIC_SINGLE, ((nItemId - MN_DICTIONARIES_START) == 1)?sal_True:sal_False );

    //ADD NEW LANGUAGE MENU ITEM
    ///////////////////////////////////////////////////////////////////////////
    String aScriptTypesInUse( OUString::number( pWrtSh->GetScriptType() ) );
    SvtLanguageTable aLanguageTable;

    // get keyboard language
    String aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = aLanguageTable.GetString( nLang );

    // get the language that is in use
    String aCurrentLang = OUString("*");
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = aLanguageTable.GetString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = aLanguageTable.GetString(nGuessLangWord);

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_SELECTION);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, aLangTable_Text );
    EnableItem( MN_SET_LANGUAGE_SELECTION, true );

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_PARAGRAPH);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, aLangTable_Paragraph );
    EnableItem( MN_SET_LANGUAGE_PARAGRAPH, true );

    if (bUseImagesInMenus)
    {
        uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface();
        Image rImg = ::GetImage( xFrame, ".uno:SpellingAndGrammarDialog", sal_False );
        SetItemImage( MN_SPELLING_DLG, rImg );
    }

    //////////////////////////////////////////////////////////////////////////////////

    RemoveDisabledEntries( sal_True, sal_True );
}



SwSpellPopup::SwSpellPopup(
    SwWrtShell *pWrtSh,
    const linguistic2::ProofreadingResult &rResult,
    sal_Int32 nErrorInResult,
    const uno::Sequence< OUString > &rSuggestions,
    const String &rParaText ) :
PopupMenu( SW_RES(MN_SPELL_POPUP) ),
pSh( pWrtSh ),
xGrammarResult( rResult ),
aSuggestions( rSuggestions ),
sExplanationLink( ),
bGrammarResults( true ),
aInfo16( SW_RES(IMG_INFO_16) )
{
    nCheckedLanguage = LanguageTag::convertToLanguageType( rResult.aLocale );
    nGrammarError = nErrorInResult;
    bool bUseImagesInMenus = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();

    sal_uInt16 nPos = 0;
    OUString aMessageText( rResult.aErrors[ nErrorInResult ].aShortComment );
    InsertSeparator(OString(), nPos++);
    InsertItem(MN_SHORT_COMMENT, aMessageText, MIB_NOSELECT, OString(), nPos++);
    if (bUseImagesInMenus)
        SetItemImage( MN_SHORT_COMMENT, aInfo16 );

    // Add an item to show detailled infos if the FullCommentURL property is defined
    beans::PropertyValues  aProperties = rResult.aErrors[ nErrorInResult ].aProperties;
    {
        sal_Int32 i = 0;
        while ( sExplanationLink.isEmpty() && i < aProperties.getLength() )
        {
            if ( aProperties[i].Name == "FullCommentURL" )
            {
                uno::Any aValue = aProperties[i].Value;
                aValue >>= sExplanationLink;
            }
            ++i;
        }
    }

    if ( !sExplanationLink.isEmpty( ) )
    {
        InsertItem(MN_EXPLANATION_LINK, SW_RESSTR(STR_EXPLANATION_LINK), MIB_TEXT | MIB_HELP, OString(), nPos++);
    }

    SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);

    InsertSeparator(OString(), nPos++);
    sal_Int32 nStringCount = aSuggestions.getLength();
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
                aImage = Image( lcl_GetImageFromPngUrl( aSuggestionImageUrl ) );
            }
        }

        sal_uInt16 nItemId = MN_SUGGESTION_START;
        for (sal_uInt16 i = 0;  i < nStringCount;  ++i)
        {
            const OUString aEntry = aSuggestions[ i ];
            InsertItem(nItemId, aEntry, 0, OString(), nPos++);
            SetHelpId( nItemId, HID_LINGU_REPLACE );
            if (!aSuggestionImageUrl.isEmpty())
                SetItemImage( nItemId, aImage );

            ++nItemId;
        }
        InsertSeparator(OString(), nPos++);
    }

    OUString aIgnoreSelection( String( SW_RES( STR_IGNORE_SELECTION ) ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( ".uno:SpellingAndGrammarDialog" );
    SetItemText( MN_SPELLING_DLG, aSpellingAndGrammar );
    sal_uInt16 nItemPos = GetItemPos( MN_IGNORE_WORD );
    InsertItem( MN_IGNORE_SELECTION, aIgnoreSelection, 0, OString(), nItemPos );
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, false );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    nGuessLangWord = LANGUAGE_NONE;
    nGuessLangPara = LANGUAGE_NONE;
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

    EnableItem( MN_IGNORE_WORD, false );
    EnableItem( MN_ADD_TO_DIC, false );
    EnableItem( MN_ADD_TO_DIC_SINGLE, false );

    //ADD NEW LANGUAGE MENU ITEM
    ///////////////////////////////////////////////////////////////////////////
    String aScriptTypesInUse( OUString::number( pWrtSh->GetScriptType() ) );
    SvtLanguageTable aLanguageTable;

    // get keyboard language
    String aKeyboardLang;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    LanguageType nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = aLanguageTable.GetString( nLang );

    // get the language that is in use
    String aCurrentLang = OUString("*");
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = aLanguageTable.GetString( nLang );

    // build sequence for status value
    uno::Sequence< OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = aLanguageTable.GetString(nGuessLangWord);

    PopupMenu *pMenu = GetPopupMenu(MN_SET_LANGUAGE_SELECTION);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_SELECTION_START, aSeq, pWrtSh, aLangTable_Text );
    EnableItem( MN_SET_LANGUAGE_SELECTION, true );

    pMenu = GetPopupMenu(MN_SET_LANGUAGE_PARAGRAPH);
    fillLangPopupMenu( pMenu, MN_SET_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, aLangTable_Paragraph );
    EnableItem( MN_SET_LANGUAGE_PARAGRAPH, true );

    if (bUseImagesInMenus)
    {
        uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface();
        Image rImg = ::GetImage( xFrame, ".uno:SpellingAndGrammarDialog", sal_False );
        SetItemImage( MN_SPELLING_DLG, rImg );
    }

    //////////////////////////////////////////////////////////////////////////////////

    RemoveDisabledEntries( sal_True, sal_True );
}


sal_uInt16  SwSpellPopup::Execute( const Rectangle& rWordPos, Window* pWin )
{
    sal_uInt16 nRet = PopupMenu::Execute(pWin, pWin->LogicToPixel(rWordPos));
    Execute( nRet );
    return nRet;
}

void SwSpellPopup::Execute( sal_uInt16 nId )
{
    if (nId == USHRT_MAX)
        return;

    if (/*bGrammarResults && */nId == MN_SHORT_COMMENT)
        return;     // nothing to do since it is the error message (short comment)

    if ((MN_SUGGESTION_START <= nId && nId <= MN_SUGGESTION_END) ||
        (MN_AUTOCORR_START <= nId && nId <= MN_AUTOCORR_END))
    {
        sal_Int32 nAltIdx = (MN_SUGGESTION_START <= nId && nId <= MN_SUGGESTION_END) ?
                nId - MN_SUGGESTION_START : nId - MN_AUTOCORR_START;
        OSL_ENSURE( 0 <= nAltIdx && nAltIdx < aSuggestions.getLength(), "index out of range" );
        if (0 <= nAltIdx && nAltIdx < aSuggestions.getLength() && (bGrammarResults || xSpellAlt.is()))
        {
            sal_Bool bOldIns = pSh->IsInsMode();
            pSh->SetInsMode( sal_True );

            String aTmp( aSuggestions[ nAltIdx ] );
            String aOrig( bGrammarResults ? OUString() : xSpellAlt->getWord() );

            // if orginal word has a trailing . (likely the end of a sentence)
            // and the replacement text hasn't, then add it to the replacement
            if (aTmp.Len() && aOrig.Len() &&
                '.' == aOrig.GetChar( aOrig.Len() - 1) && /* !IsAlphaNumeric ??*/
                '.' != aTmp.GetChar( aTmp.Len() - 1))
            {
                aTmp += '.';
            }

            // #111827#
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pSh->GetCrsrDescr());
            aRewriter.AddRule(UndoArg2, String(SW_RES(STR_YIELDS)));

            String aTmpStr( SW_RES(STR_START_QUOTE) );
            aTmpStr += aTmp;
            aTmpStr += String(SW_RES(STR_END_QUOTE));
            aRewriter.AddRule(UndoArg3, aTmpStr);

            pSh->StartUndo(UNDO_UI_REPLACE, &aRewriter);
            pSh->StartAction();
            pSh->DelLeft();

            pSh->Insert( aTmp );

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/

            // record only if it's NOT already present in autocorrection
            SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();

            String aOrigWord( bGrammarResults ? OUString() : xSpellAlt->getWord() ) ;
            String aNewWord( aSuggestions[ nAltIdx ] );
            SvxPrepareAutoCorrect( aOrigWord, aNewWord );

            if (MN_AUTOCORR_START <= nId && nId <= MN_AUTOCORR_END)
                pACorr->PutText( aOrigWord, aNewWord, nCheckedLanguage );

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/
            pSh->EndAction();
            pSh->EndUndo();

            pSh->SetInsMode( bOldIns );
        }
    }
    else if (nId == MN_SPELLING_DLG)
    {
        if (bGrammarResults)
        {
            SvtLinguConfig().SetProperty( UPN_IS_GRAMMAR_INTERACTIVE, uno::makeAny( sal_True ));
        }
        pSh->Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
        {
            uno::Reference<linguistic2::XSearchableDictionaryList> xDictionaryList( SvxGetDictionaryList() );
            SvxDicListChgClamp aClamp( xDictionaryList );
            pSh->GetView().GetViewFrame()->GetDispatcher()->
                Execute( FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON );
        }
    }
    else if (nId == MN_IGNORE_SELECTION)
    {
        SwPaM *pPaM = pSh->GetCrsr();
        if (pPaM) {
            if (bGrammarResults) {
                try
                {
                    xGrammarResult.xProofreader->ignoreRule(
                        xGrammarResult.aErrors[ nGrammarError ].aRuleIdentifier,
                            xGrammarResult.aLocale );
                }
                catch( const uno::Exception& )
                {
                }
            }
            pSh->IgnoreGrammarErrorAt( *pPaM );
        }
    }
    else if (nId == MN_IGNORE_WORD)
    {
        uno::Reference< linguistic2::XDictionary > xDictionary( SvxGetIgnoreAllList(), uno::UNO_QUERY );
        linguistic::AddEntryToDic( xDictionary,
                xSpellAlt->getWord(), sal_False, aEmptyStr, LANGUAGE_NONE );
    }
    else if ((MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END) || nId == MN_ADD_TO_DIC_SINGLE)
    {
            OUString aWord( xSpellAlt->getWord() );
            String aDicName;

            if (MN_DICTIONARIES_START <= nId && nId <= MN_DICTIONARIES_END)
            {
                PopupMenu *pMenu = GetPopupMenu(MN_ADD_TO_DIC);
                aDicName = pMenu->GetItemText(nId);
            }
            else
                aDicName = aDicNameSingle;

            uno::Reference< linguistic2::XDictionary >      xDic;
            uno::Reference< linguistic2::XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );
            if (xDicList.is())
                xDic = xDicList->getDictionaryByName( aDicName );

            if (xDic.is())
            {
                sal_Int16 nAddRes = linguistic::AddEntryToDic( xDic, aWord, sal_False, aEmptyStr, LANGUAGE_NONE );
                // save modified user-dictionary if it is persistent
                uno::Reference< frame::XStorable >  xSavDic( xDic, uno::UNO_QUERY );
                if (xSavDic.is())
                    xSavDic->store();

                if (DIC_ERR_NONE != nAddRes
                    && !xDic->getEntry( aWord ).is())
                {
                    SvxDicError(
                        &pSh->GetView().GetViewFrame()->GetWindow(),
                        nAddRes );
                }
            }
    }
    else if ( nId == MN_EXPLANATION_LINK && !sExplanationLink.isEmpty() )
    {
        try
        {
            uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                com::sun::star::system::SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );
            xSystemShellExecute->execute( sExplanationLink, OUString(),
                    com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
        }
        catch (const uno::Exception&)
        {
            uno::Any exc( ::cppu::getCaughtException() );
            OUString msg( ::comphelper::anyToString( exc ) );
            const SolarMutexGuard guard;
            ErrorBox aErrorBox( NULL, WB_OK, msg );
            aErrorBox.SetText( "Explanations" );
            aErrorBox.Execute();
        }
    }
    else
    {
        // Set language for selection or for paragraph...

        SfxItemSet aCoreSet( pSh->GetView().GetPool(),
                    RES_CHRATR_LANGUAGE,        RES_CHRATR_LANGUAGE,
                    RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CTL_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE,
                    0 );
        String aNewLangTxt;

        if (MN_SET_LANGUAGE_SELECTION_START <= nId && nId <= MN_SET_LANGUAGE_SELECTION_END)
        {
            //Set language for current selection
            aNewLangTxt = aLangTable_Text[nId];
            SwLangHelper::SetLanguage( *pSh, aNewLangTxt, true, aCoreSet );
        }
        else if (nId == MN_SET_SELECTION_NONE)
        {
            //Set Language_None for current selection
            SwLangHelper::SetLanguage_None( *pSh, true, aCoreSet );
        }
        else if (nId == MN_SET_SELECTION_RESET)
        {
            //reset languages for current selection
            SwLangHelper::ResetLanguages( *pSh, true );
        }
        else if (nId == MN_SET_SELECTION_MORE)
        {
            //Open Format/Character Dialog
            sw_CharDialog( *pSh, true, nId, 0, 0 );
        }
        else if (MN_SET_LANGUAGE_PARAGRAPH_START <= nId && nId <= MN_SET_LANGUAGE_PARAGRAPH_END)
        {
            //Set language for current paragraph
            aNewLangTxt = aLangTable_Paragraph[nId];
            pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *pSh );
            SwLangHelper::SetLanguage( *pSh, aNewLangTxt, true, aCoreSet );
            pSh->Pop( sal_False );  // restore cursor
        }
        else if (nId == MN_SET_PARA_NONE)
        {
            //Set Language_None for current paragraph
            pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *pSh );
            SwLangHelper::SetLanguage_None( *pSh, true, aCoreSet );
            pSh->Pop( sal_False );  // restore cursor
        }
        else if (nId == MN_SET_PARA_RESET)
        {
            //reset languages for current paragraph
            pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *pSh );
            SwLangHelper::ResetLanguages( *pSh, true );
            pSh->Pop( sal_False );  // restore cursor
        }
        else if (nId == MN_SET_PARA_MORE)
        {
            pSh->Push();        // save cursor
            SwLangHelper::SelectCurrentPara( *pSh );
            //Open Format/Character Dialog
            sw_CharDialog( *pSh, true, nId, 0, 0 );
            pSh->Pop( sal_False );  // restore cursor
        }
    }

    pSh->EnterStdMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
