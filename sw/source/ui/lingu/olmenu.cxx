/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: olmenu.cxx,v $
 * $Revision: 1.41 $
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
#include "precompiled_sw.hxx"

#include <hintids.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#include <svtools/lingucfg.hxx>
#include <svtools/linguprops.hxx>
#include <svtools/filter.hxx>
#include <svx/impgrf.hxx>
#include <svx/svxacorr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <osl/file.hxx>
#include <rtl/string.hxx>

#include <i18npool/mslangid.hxx>
#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/unolingu.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/linguistic2/SingleProofreadingError.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svx/dlgutil.hxx>
#include <svtools/itemset.hxx>
#include <svx/langitem.hxx>
#include <svx/splwrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/lingucfg.hxx>
#include <svx/acorrcfg.hxx>
#include <swmodule.hxx>
#include <cmdid.h>
#include <helpid.h>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>        //CheckSpellChanges
#include <olmenu.hxx>
#include <swundo.hxx>
#include <crsskip.hxx>
#include <ndtxt.hxx>
#include <olmenu.hrc>
#include <doc.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <comcore.hrc>
#include <undobj.hxx>
// <- #111827#

#include <unomid.h>
#include <svtools/languageoptions.hxx>
#include <map>
#include <svtools/langtab.hxx>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <edtwin.hxx>
#include <sfx2/sfxdlg.hxx>
#include "swabstdlg.hxx"
#include "chrdlg.hrc"
#include <svx/brshitem.hxx>
#include <svtools/stritem.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>

#include <wview.hxx>
#include <sfx2/request.hxx>

#include <vcl/msgbox.hxx>

#include <langhelper.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

extern void lcl_CharDialog( SwWrtShell &rWrtSh, BOOL bUseDialog, USHORT nSlot,const SfxItemSet *pArgs, SfxRequest *pReq );


/*--------------------------------------------------------------------------

---------------------------------------------------------------------------*/

// tries to determine the language of 'rText'
//
LanguageType lcl_CheckLanguage(
    const OUString &rText,
    uno::Reference< linguistic2::XSpellChecker1 > xSpell,
    uno::Reference< linguistic2::XLanguageGuessing > xLangGuess,
    sal_Bool bIsParaText )
{
    LanguageType  nLang = LANGUAGE_NONE;
    if (bIsParaText)    // check longer texts with language-guessing...
    {
        if (!xLangGuess.is())
            return nLang;

        lang::Locale aLocale( xLangGuess->guessPrimaryLanguage( rText, 0, rText.getLength()) );

        // get language as from "Tools/Options - Language Settings - Languages: Locale setting"
        LanguageType nTmpLang = Application::GetSettings().GetLanguage();

        // if the result from language guessing does not provide a 'Country' part
        // try to get it by looking up the locale setting of the office.
        if (aLocale.Country.getLength() == 0)
        {
            lang::Locale aTmpLocale = SvxCreateLocale( nTmpLang );
            if (aTmpLocale.Language == aLocale.Language)
                nLang = nTmpLang;
        }
        if (nLang == LANGUAGE_NONE) // language not found by looking up the system language...
            nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if (nLang == LANGUAGE_SYSTEM)
            nLang = nTmpLang;
        if (nLang == LANGUAGE_DONTKNOW)
            nLang = LANGUAGE_NONE;
    }
    else    // check single word
    {
            if (!xSpell.is())
            return nLang;

        //
        // build list of languages to check
        //
        LanguageType aLangList[4];
        const AllSettings& rSettings  = Application::GetSettings();
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig().GetOptions( aLinguOpt );
        // The default document language from "Tools/Options - Language Settings - Languages: Western"
        aLangList[0] = aLinguOpt.nDefaultLanguage;
        // The one from "Tools/Options - Language Settings - Languages: User interface"
        aLangList[1] = rSettings.GetUILanguage();
        // The one from "Tools/Options - Language Settings - Languages: Locale setting"
        aLangList[2] = rSettings.GetLanguage();
        // en-US
        aLangList[3] = LANGUAGE_ENGLISH_US;
#ifdef DEBUG
        lang::Locale a0( SvxCreateLocale( aLangList[0] ) );
        lang::Locale a1( SvxCreateLocale( aLangList[1] ) );
        lang::Locale a2( SvxCreateLocale( aLangList[2] ) );
        lang::Locale a3( SvxCreateLocale( aLangList[3] ) );
#endif

        INT32   nCount = sizeof(aLangList) / sizeof(aLangList[0]);
        for (INT32 i = 0;  i < nCount;  i++)
        {
            INT16 nTmpLang = aLangList[i];
            if (nTmpLang != LANGUAGE_NONE  &&  nTmpLang != LANGUAGE_DONTKNOW)
            {
                if (xSpell->hasLanguage( nTmpLang ) &&
                    xSpell->isValid( rText, nTmpLang, uno::Sequence< beans::PropertyValue >() ))
                {
                    nLang = nTmpLang;
                    break;
                }
            }
        }
    }

    return nLang;
}


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
inline bool lcl_checkScriptType( sal_Int16 nScriptType, LanguageType nLang )
{
    return 0 != (nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

USHORT SwSpellPopup::fillLangPopupMenu(
    PopupMenu *pPopupMenu,
    USHORT Lang_Start,
    uno::Sequence< ::rtl::OUString > aSeq,
    SwWrtShell* pWrtSh,
    USHORT nLangTable )
{
    if (!pPopupMenu)
        return 0;

    //Reference< awt::XMenuExtended > m_xMenuExtended( m_xPopupMenu, UNO_QUERY );
    std::map< ::rtl::OUString, ::rtl::OUString > LangItems;

    SvtLanguageTable    aLanguageTable;
    USHORT nItemId              = Lang_Start;
    rtl::OUString curLang       = aSeq[0];
    USHORT nScriptType          = static_cast< sal_Int16 >(aSeq[1].toInt32());
    rtl::OUString keyboardLang  = aSeq[2];
    rtl::OUString guessLang     = aSeq[3];

    //1--add current language
    if(curLang!=OUString::createFromAscii(""))
    {
        LangItems[curLang]=curLang;
    }

    SvtLanguageTable aLangTable;
    //2--System
    const AllSettings& rAllSettings=Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguage();
    if(rSystemLanguage!=LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(nScriptType,rSystemLanguage ))
            LangItems[OUString(aLangTable.GetString(rSystemLanguage))]=OUString(aLangTable.GetString(rSystemLanguage));
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguage();
    if(rUILanguage!=LANGUAGE_DONTKNOW)
    {
        if (lcl_checkScriptType(nScriptType, rUILanguage ))
            LangItems[OUString(aLangTable.GetString(rUILanguage))]=OUString(aLangTable.GetString(rUILanguage));
    }

    //4--guessed language
    if(guessLang!=OUString::createFromAscii(""))
    {
        if (lcl_checkScriptType(nScriptType, aLanguageTable.GetType(guessLang)))
            LangItems[guessLang]=guessLang;
    }


    //5--keyboard language
    if(keyboardLang!=OUString::createFromAscii(""))
    {
        if (lcl_checkScriptType(nScriptType, aLanguageTable.GetType(keyboardLang)))
            LangItems[keyboardLang]=keyboardLang;
    }

    //6--all languages used in current document
    uno::Reference< com::sun::star::frame::XModel > xModel;
    uno::Reference< com::sun::star::frame::XController > xController( pWrtSh->GetView().GetViewFrame()->GetFrame()->GetFrameInterface()->getController(), uno::UNO_QUERY );
    if ( xController.is() )
        xModel = xController->getModel();

    uno::Reference< document::XDocumentLanguages > xDocumentLanguages( xModel, uno::UNO_QUERY );
    /*the description of nScriptType
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
        uno::Sequence< lang::Locale > rLocales(xDocumentLanguages->getDocumentLanguages(nScriptType,nCount));
        if(rLocales.getLength()>0)
        {
            for(USHORT i = 0; i<rLocales.getLength();++i)
            {
                if (LangItems.size()==7)
                    break;
                const lang::Locale& rLocale=rLocales[i];
                if(lcl_checkScriptType(nScriptType, aLanguageTable.GetType(rLocale.Language)))
                    LangItems[ rtl::OUString(rLocale.Language)]=OUString(rLocale.Language);
            }
        }
    }

    for (std::map< rtl::OUString, rtl::OUString >::const_iterator it = LangItems.begin(); it != LangItems.end(); ++it)
    {
        rtl::OUString aEntryTxt( it->first );
        if (aEntryTxt != rtl::OUString( aLangTable.GetString( LANGUAGE_NONE ) )&&
            aEntryTxt != rtl::OUString::createFromAscii("*") &&
            aEntryTxt.getLength() > 0)
        {
            ++nItemId;
            if (nLangTable == 0)        // language for selection
                aLangTable_Text[nItemId]      = aEntryTxt;
            else if (nLangTable == 1)   // language for paragraph
                aLangTable_Paragraph[nItemId] = aEntryTxt;
            else if (nLangTable == 2)   // language for document
                aLangTable_Document[nItemId]  = aEntryTxt;

            pPopupMenu->InsertItem( nItemId, aEntryTxt, MIB_RADIOCHECK );
            if (aEntryTxt == curLang)
            {
                //make a check mark for the current language
                pPopupMenu->CheckItem( nItemId, TRUE );
            }
        }
    }

    //7--none
    nItemId++;
    pPopupMenu->InsertItem( nItemId, String(SW_RES( STR_LANGSTATUS_NONE )), MIB_RADIOCHECK );

    //More...
    nItemId++;
    pPopupMenu->InsertItem( nItemId, String(SW_RES( STR_LANGSTATUS_MORE )), MIB_RADIOCHECK );

    return nItemId - Lang_Start;    // return number of inserted entries
}


static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );
//    ::rtl::OString aPath = OString( aTmp.getStr(), aTmp.getLength(), osl_getThreadTextEncoding() );
#if defined(WNT)
//    aTmp = lcl_Win_GetShortPathName( aTmp );
#endif
    Graphic aGraphic;
    const String aFilterName( RTL_CONSTASCII_USTRINGPARAM( IMP_PNG ) );
    if( GRFILTER_OK == LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}


::rtl::OUString RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL )
{
    ::rtl::OUString aLabel;
    if ( aCmdURL.getLength() )
    {
        try
        {
            uno::Reference< container::XNameAccess > xNameAccess( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.frame.UICommandDescription") ), uno::UNO_QUERY );
            if ( xNameAccess.is() )
            {
                uno::Reference< container::XNameAccess > xUICommandLabels;
                const ::rtl::OUString aModule( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ) );
                uno::Any a = xNameAccess->getByName( aModule );
                uno::Reference< container::XNameAccess > xUICommands;
                a >>= xUICommandLabels;
                rtl::OUString aStr;
                uno::Sequence< beans::PropertyValue > aPropSeq;
                a = xUICommandLabels->getByName( aCmdURL );
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( "Name" ))
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( uno::Exception& )
        {
        }
    }

    return aLabel;
}


SwSpellPopup::SwSpellPopup(
        SwWrtShell* pWrtSh,
        const uno::Reference< linguistic2::XSpellAlternatives >  &xAlt,
        const String &rParaText ) :
PopupMenu( SW_RES(MN_SPELL_POPUP) ),
pSh( pWrtSh ),
xSpellAlt(xAlt),
bGrammarResults(false)
{
    DBG_ASSERT(xSpellAlt.is(), "no spelling alternatives available");

    CreateAutoMnemonics();
    nCheckedLanguage = LANGUAGE_NONE;
    if (xSpellAlt.is())
    {
        nCheckedLanguage = SvxLocaleToLanguage( xSpellAlt->getLocale() );
        aSuggestions = xSpellAlt->getAlternatives();
    }
    sal_Int16 nStringCount = static_cast< sal_Int16 >( aSuggestions.getLength() );

    SvtLinguConfig aCfg;
    const bool bIsDark = Application::GetSettings().GetStyleSettings().GetWindowColor().IsDark();

    PopupMenu *pMenu = GetPopupMenu(MN_AUTOCORR);
    pMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
    sal_Bool bEnable = sal_False;
    if( nStringCount )
    {
        Image aImage;
        OUString aSuggestionImageUrl;
        uno::Reference< container::XNamed > xNamed( xSpellAlt, uno::UNO_QUERY );
        if (xNamed.is())
        {
            aSuggestionImageUrl = aCfg.GetSpellAndGrammarContextSuggestionImage( xNamed->getName(), bIsDark );
            aImage = Image( lcl_GetImageFromPngUrl( aSuggestionImageUrl ) );
        }

        InsertSeparator(0);
        bEnable = sal_True;
        for( sal_uInt16 i = 0, nPos = 1, nId = MN_AUTOCORR_START + 1;
            i < nStringCount; ++i, ++nPos, ++nId )
        {
            const String aEntry = aSuggestions[ i ];
            InsertItem( nPos, aEntry, 0, i );
            SetHelpId( nPos, HID_LINGU_REPLACE);

            if (aSuggestionImageUrl.getLength() > 0)
                SetItemImage( nPos, aImage );

            pMenu->InsertItem( nId, aEntry );
            pMenu->SetHelpId( nPos, HID_LINGU_AUTOCORR);
        }
    }

    OUString aIgnoreSelection( String( SW_RES( STR_IGNORE_SELECTION ) ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( C2U(".uno:SpellingAndGrammarDialog") );
    SetItemText( MN_SPELLING, aSpellingAndGrammar );
    USHORT nItemPos = GetItemPos( MN_IGNORE );
    InsertItem( MN_IGNORE_SELECTION, aIgnoreSelection, 0, nItemPos );
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, bEnable );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    nGuessLangWord = LANGUAGE_NONE;
    nGuessLangPara = LANGUAGE_NONE;
    if (xSpellAlt.is() && xLG.is())
    {
        nGuessLangWord = lcl_CheckLanguage( xSpellAlt->getWord(), ::GetSpellChecker(), xLG, sal_False );
        nGuessLangPara = lcl_CheckLanguage( rParaText, ::GetSpellChecker(), xLG, sal_True );
    }
    if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (nGuessLangWord == LANGUAGE_NONE)
            nGuessLangWord = nGuessLangPara;
        if (nGuessLangPara == LANGUAGE_NONE)
            nGuessLangPara = nGuessLangWord;
    }

    pMenu = GetPopupMenu(MN_INSERT);

    bEnable = FALSE;    // enable MN_INSERT?

    pMenu->CreateAutoMnemonics();
    uno::Reference< linguistic2::XDictionaryList >    xDicList( SvxGetDictionaryList() );
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
        USHORT nDicCount = static_cast< USHORT >(aDics.getLength());

        for( USHORT i = 0; i < nDicCount; i++ )
        {
            uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
            if (!xDicTmp.is() || SvxGetIgnoreAllList() == xDicTmp)
                continue;

            uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
            LanguageType nActLanguage = SvxLocaleToLanguage( xDicTmp->getLocale() );
            if( xDicTmp->isActive()
                &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                && (nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                && (!xStor.is() || !xStor->isReadonly()) )
            {
                // the extra 1 is because of the (possible) external
                // linguistic entry above
                USHORT nPos = MN_INSERT_START + i + 1;
                pMenu->InsertItem( nPos, xDicTmp->getName() );
                bEnable = sal_True;

                uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                if (xSvcInfo.is())
                {
                    OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                            xSvcInfo->getImplementationName(), bIsDark) );
                    if (aDictionaryImageUrl.getLength() > 0)
                    {
                        Image aImage( lcl_GetImageFromPngUrl( aDictionaryImageUrl ) );
                        pMenu->SetItemImage( nPos, aImage );
                    }
                }
            }
        }
    }
    EnableItem( MN_INSERT, bEnable );

    //ADD NEW LANGUAGE MENU ITEM
    ///////////////////////////////////////////////////////////////////////////
    String aScriptTypesInUse( String::CreateFromInt32( pWrtSh->GetScriptType() ) );
    SvtLanguageTable aLangTable;

    // get keyboard language
    String aKeyboardLang;
    LanguageType nLang = LANGUAGE_DONTKNOW;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = aLangTable.GetString( nLang );

    // get the language that is in use
    const String aMultipleLanguages = String::CreateFromAscii("*");
    String aCurrentLang = aMultipleLanguages;
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = aLangTable.GetString( nLang );

    // build sequence for status value
    uno::Sequence< ::rtl::OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = aLangTable.GetString(nGuessLangWord);

    pMenu = GetPopupMenu(MN_LANGUAGE_SELECTION);
    nNumLanguageTextEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_SELECTION_START, aSeq, pWrtSh, 0 );
    EnableItem( MN_LANGUAGE_SELECTION, true );

    pMenu = GetPopupMenu(MN_LANGUAGE_PARAGRAPH);
    nNumLanguageParaEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, 1 );
    EnableItem( MN_LANGUAGE_PARAGRAPH, true );
/*
    pMenu = GetPopupMenu(MN_LANGUAGE_ALL_TEXT);
    nNumLanguageDocEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_ALL_TEXT_START, aSeq, pWrtSh, 2 );
    EnableItem( MN_LANGUAGE_ALL_TEXT, true );
*/
    uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame()->GetFrameInterface();
    Image rImg = ::GetImage( xFrame,
            ::rtl::OUString::createFromAscii(".uno:SpellingAndGrammarDialog"), sal_False,
            Application::GetSettings().GetStyleSettings().GetWindowColor().IsDark() );
    SetItemImage( MN_SPELLING, rImg );
    //////////////////////////////////////////////////////////////////////////////////

    RemoveDisabledEntries( TRUE, TRUE );
    SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
}

/*--------------------------------------------------------------------------

---------------------------------------------------------------------------*/

SwSpellPopup::SwSpellPopup(
    SwWrtShell *pWrtSh,
    const linguistic2::ProofreadingResult &rResult,
    sal_Int32 nErrorInResult,
    const uno::Sequence< rtl::OUString > &rSuggestions,
    const String &rParaText ) :
PopupMenu( SW_RES(MN_SPELL_POPUP) ),
pSh( pWrtSh ),
aSuggestions( rSuggestions ),
bGrammarResults( true ),
aInfo16( SW_RES(IMG_INFO_16) )
{
    nCheckedLanguage = SvxLocaleToLanguage( rResult.aLocale );

    sal_Int16 nItemId = 1;
    sal_Int16 nPos    = 0;
    OUString aMessageText( rResult.aErrors[ nErrorInResult ].aShortComment );
    InsertSeparator( nPos++ );
    InsertItem( nItemId, aMessageText, MIB_NOSELECT, nPos++ );
    SetItemImage( nItemId, aInfo16 );
    ++nItemId;

    CreateAutoMnemonics();

    InsertSeparator( nPos++ );
    sal_Int32 nStringCount = aSuggestions.getLength();
    if ( nStringCount )     // suggestions available...
    {
        Image aImage;
        OUString aSuggestionImageUrl;
        uno::Reference< lang::XServiceInfo > xInfo( rResult.xProofreader, uno::UNO_QUERY );
        if (xInfo.is())
        {
            aSuggestionImageUrl = SvtLinguConfig().GetSpellAndGrammarContextSuggestionImage( xInfo->getImplementationName() );
            aImage = Image( lcl_GetImageFromPngUrl( aSuggestionImageUrl ) );
        }

        for (sal_uInt16 i = 0;  i < nStringCount;  ++i)
        {
            const String aEntry = aSuggestions[ i ];
            InsertItem( nItemId, aEntry, 0, nPos++ );
            SetHelpId( nItemId, HID_LINGU_REPLACE );

            if (aSuggestionImageUrl.getLength() > 0)
                SetItemImage( nItemId, aImage );

            ++nItemId;
        }
        InsertSeparator( nPos++ );
    }

    OUString aIgnoreSelection( String( SW_RES( STR_IGNORE_SELECTION ) ) );
    OUString aSpellingAndGrammar = RetrieveLabelFromCommand( C2U(".uno:SpellingAndGrammarDialog") );
    SetItemText( MN_SPELLING, aSpellingAndGrammar );
    USHORT nItemPos = GetItemPos( MN_IGNORE );
    InsertItem( MN_IGNORE_SELECTION, aIgnoreSelection, 0, nItemPos );
    SetHelpId( MN_IGNORE_SELECTION, HID_LINGU_IGNORE_SELECTION);

    EnableItem( MN_AUTOCORR, false );

    uno::Reference< linguistic2::XLanguageGuessing > xLG = SW_MOD()->GetLanguageGuesser();
    nGuessLangWord = LANGUAGE_NONE;
    nGuessLangPara = LANGUAGE_NONE;
    if (xLG.is())
    {
//        nGuessLangWord = lcl_CheckLanguage( xSpellAlt->getWord(), ::GetSpellChecker(), xLG, sal_False );
        nGuessLangPara = lcl_CheckLanguage( rParaText, ::GetSpellChecker(), xLG, sal_True );
    }
    if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (nGuessLangWord == LANGUAGE_NONE)
            nGuessLangWord = nGuessLangPara;
        if (nGuessLangPara == LANGUAGE_NONE)
            nGuessLangPara = nGuessLangWord;
    }

    EnableItem( MN_IGNORE, false );
    EnableItem( MN_INSERT, false );

    //ADD NEW LANGUAGE MENU ITEM
    ///////////////////////////////////////////////////////////////////////////
    String aScriptTypesInUse( String::CreateFromInt32( pWrtSh->GetScriptType() ) );
    SvtLanguageTable aLangTable;

    // get keyboard language
    String aKeyboardLang;
    LanguageType nLang = LANGUAGE_DONTKNOW;
    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
    nLang = rEditWin.GetInputLanguage();
    if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
        aKeyboardLang = aLangTable.GetString( nLang );

    // get the language that is in use
    const String aMultipleLanguages = String::CreateFromAscii("*");
    String aCurrentLang = aMultipleLanguages;
    nLang = SwLangHelper::GetCurrentLanguage( *pWrtSh );
    if (nLang != LANGUAGE_DONTKNOW)
        aCurrentLang = aLangTable.GetString( nLang );

    // build sequence for status value
    uno::Sequence< ::rtl::OUString > aSeq( 4 );
    aSeq[0] = aCurrentLang;
    aSeq[1] = aScriptTypesInUse;
    aSeq[2] = aKeyboardLang;
    aSeq[3] = aLangTable.GetString(nGuessLangWord);

    PopupMenu *pMenu = GetPopupMenu(MN_LANGUAGE_SELECTION);
    nNumLanguageTextEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_SELECTION_START, aSeq, pWrtSh, 0 );
    EnableItem( MN_LANGUAGE_SELECTION, true );

    pMenu = GetPopupMenu(MN_LANGUAGE_PARAGRAPH);
    nNumLanguageParaEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_PARAGRAPH_START, aSeq, pWrtSh, 1 );
    EnableItem( MN_LANGUAGE_PARAGRAPH, true );
/*
    pMenu = GetPopupMenu(MN_LANGUAGE_ALL_TEXT);
    nNumLanguageDocEntries = fillLangPopupMenu( pMenu, MN_LANGUAGE_ALL_TEXT_START, aSeq, pWrtSh, 2 );
    EnableItem( MN_LANGUAGE_ALL_TEXT, true );
*/
    uno::Reference< frame::XFrame > xFrame = pWrtSh->GetView().GetViewFrame()->GetFrame()->GetFrameInterface();
    Image rImg = ::GetImage( xFrame,
            ::rtl::OUString::createFromAscii(".uno:SpellingAndGrammarDialog"), sal_False,
            Application::GetSettings().GetStyleSettings().GetWindowColor().IsDark() );
    SetItemImage( MN_SPELLING, rImg );

    //////////////////////////////////////////////////////////////////////////////////

    RemoveDisabledEntries( TRUE, TRUE );
    SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
}

/*--------------------------------------------------------------------------

---------------------------------------------------------------------------*/
sal_uInt16  SwSpellPopup::Execute( const Rectangle& rWordPos, Window* pWin )
{
//    SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
    sal_uInt16 nRet = PopupMenu::Execute(pWin, pWin->LogicToPixel(rWordPos));
    Execute( nRet );
    return nRet;
}
/*-- 19.01.2006 08:15:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellPopup::Execute( USHORT nId )
{
    if (bGrammarResults && nId == 1)
        return;     // nothing to do since it is the error message (short comment)

    sal_Bool bAutoCorr = sal_False;
    if( nId > MN_AUTOCORR_START && nId < MN_LANGUAGE_SELECTION_START && nId != USHRT_MAX )
    {
        nId -= MN_AUTOCORR_START;
        bAutoCorr = sal_True;
    }

    if( nId && nId != USHRT_MAX)
    {
        int nAltIdx = bGrammarResults ? nId - 2 : nId - 1;
        if ( nAltIdx >= 0 && nAltIdx < aSuggestions.getLength() && (bGrammarResults || xSpellAlt.is()) )
        {
            sal_Bool bOldIns = pSh->IsInsMode();
            pSh->SetInsMode( sal_True );

            DBG_ASSERT( 0 <= nAltIdx && nAltIdx <= aSuggestions.getLength(), "index out of range");
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

            aRewriter.AddRule(UNDO_ARG1, pSh->GetCrsrDescr());
            aRewriter.AddRule(UNDO_ARG2, String(SW_RES(STR_YIELDS)));

            {
                String aTmpStr;

                aTmpStr += String(SW_RES(STR_START_QUOTE));
                aTmpStr += aTmp;
                aTmpStr += String(SW_RES(STR_END_QUOTE));
                aRewriter.AddRule(UNDO_ARG3, aTmpStr);
            }

            pSh->StartUndo(UNDO_UI_REPLACE, &aRewriter);
            pSh->StartAction();
            pSh->DelLeft();

            pSh->Insert( aTmp );
            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/


            // nur aufnehmen, wenn es NICHT schon in der Autokorrektur vorhanden ist
            SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get()->GetAutoCorrect();

            String aOrigWord( bGrammarResults ? OUString() : xSpellAlt->getWord() ) ;
            String aNewWord;
            if( nId )
                aNewWord = aSuggestions[ nAltIdx ];
            else
                aNewWord = aOrigWord;
            SvxPrepareAutoCorrect( aOrigWord, aNewWord );

            if( bAutoCorr )
            {
                pACorr->PutText( aOrigWord, aNewWord, nCheckedLanguage );
            }

            /* #102505# EndAction/EndUndo moved down since insertion
               of temporary auto correction is now undoable two and
               must reside in the same undo group.*/
            pSh->EndAction();
            pSh->EndUndo(UNDO_UI_REPLACE);

            pSh->SetInsMode( bOldIns );
        }
        else
        {
            if (nId < MN_LANGUAGE_SELECTION_START)
            {
                switch( nId )
                {
                    case MN_SPELLING:
                    {
                        if (bGrammarResults)
                        {
                            SvtLinguConfig().SetProperty( A2OU( UPN_IS_GRAMMAR_INTERACTIVE ), uno::makeAny( sal_True ));
                        }
                        pSh->Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
                        {
                            uno::Reference<linguistic2::XDictionaryList> xDictionaryList( SvxGetDictionaryList() );
                            SvxDicListChgClamp aClamp( xDictionaryList );
                            pSh->GetView().GetViewFrame()->GetDispatcher()->
                                Execute( FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON );
                        }
                    }
                    break;
                    case MN_IGNORE_SELECTION :
                    {
                        SwPaM *pPaM = pSh->GetCrsr();
                        if (pPaM)
                            pSh->IgnoreGrammarErrorAt( *pPaM );
                    }
                    break;
                    case MN_IGNORE :
                    {
                        uno::Reference< linguistic2::XDictionary > xDictionary( SvxGetIgnoreAllList(), uno::UNO_QUERY );
                        linguistic::AddEntryToDic(
                                xDictionary,
                                xSpellAlt->getWord(), sal_False,
                                aEmptyStr, LANGUAGE_NONE );
                    }
                    break;
                    case MN_INSERT:
                        DBG_ERROR("geht noch nicht!");
                    break;
                    case MN_LANGUAGE_WORD:
                    case MN_LANGUAGE_PARA:
                    {
                        pSh->StartAction();

                        if( MN_LANGUAGE_PARA == nId )
                        {
                            if( !pSh->IsSttPara() )
                                pSh->MovePara( fnParaCurr, fnParaStart );
                            pSh->SwapPam();
                            if( !pSh->IsEndPara() )
                                pSh->MovePara( fnParaCurr,  fnParaEnd );
                        }

                        LanguageType nLangToUse = (MN_LANGUAGE_PARA == nId) ? nGuessLangPara : nGuessLangWord;
                        sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );
                        USHORT nResId = 0;
                        switch (nScriptType)
                        {
                            case SCRIPTTYPE_COMPLEX     : nResId = RES_CHRATR_CTL_LANGUAGE; break;
                            case SCRIPTTYPE_ASIAN       : nResId = RES_CHRATR_CJK_LANGUAGE; break;
                            default /*SCRIPTTYPE_LATIN*/: nResId = RES_CHRATR_LANGUAGE; break;
                        }
                        SfxItemSet aSet(pSh->GetAttrPool(), nResId, nResId );
                        aSet.Put( SvxLanguageItem( nLangToUse, nResId ) );
                        pSh->SetAttr( aSet );

                        pSh->EndAction();
                    }
                    break;
                    default:
                        if(nId >= MN_INSERT_START )
                        {
                            OUString aWord( xSpellAlt->getWord() );
                            INT32 nDicIdx = nId - MN_INSERT_START - 1;
                            DBG_ASSERT( nDicIdx < aDics.getLength(),
                                        "dictionary index out of range" );
                            uno::Reference< linguistic2::XDictionary > xDic =
                                aDics.getConstArray()[nDicIdx];
                            INT16 nAddRes = linguistic::AddEntryToDic( xDic,
                                aWord, FALSE, aEmptyStr, LANGUAGE_NONE );
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
            }
            else
            {
                SfxItemSet aCoreSet( pSh->GetView().GetPool(),
                            RES_CHRATR_LANGUAGE,        RES_CHRATR_LANGUAGE,
                            RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,
                            RES_CHRATR_CTL_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE,
                            0 );
                String aNewLangTxt;

//              pSh->StartAction();

                if (nId >= MN_LANGUAGE_SELECTION_START && nId < MN_LANGUAGE_SELECTION_START + nNumLanguageTextEntries - 1)
                {
                    //Set language for current selection
                    aNewLangTxt=aLangTable_Text[nId];
                    SwLangHelper::SetLanguage( *pSh, aNewLangTxt, true, aCoreSet );
                }
                else if (nId == MN_LANGUAGE_SELECTION_START + nNumLanguageTextEntries - 1)
                {
                    //Set Language_None for current selection
                    SwLangHelper::SetLanguage_None( *pSh, true, aCoreSet );
                }
                else if (nId == MN_LANGUAGE_SELECTION_START + nNumLanguageTextEntries)
                {
                    //Open Format/Character Dialog
                    lcl_CharDialog( *pSh, true, nId, 0, 0 );
                }
                else if (nId >= MN_LANGUAGE_PARAGRAPH_START && nId < MN_LANGUAGE_PARAGRAPH_START + nNumLanguageParaEntries - 1)
                {
                    //Set language for current paragraph
                    aNewLangTxt=aLangTable_Paragraph[nId];
                    pSh->Push();        // save cursor
                    SwLangHelper::SelectCurrentPara( *pSh );
                    SwLangHelper::SetLanguage( *pSh, aNewLangTxt, true, aCoreSet );
                    pSh->Pop( FALSE );  // restore cursor
                }
                else if (nId == MN_LANGUAGE_PARAGRAPH_START + nNumLanguageParaEntries - 1)
                {
                    //Set Language_None for current paragraph
                    pSh->Push();        // save cursor
                    SwLangHelper::SelectCurrentPara( *pSh );
                    SwLangHelper::SetLanguage_None( *pSh, true, aCoreSet );
                    pSh->Pop( FALSE );  // restore cursor
                }
                else if (nId == MN_LANGUAGE_PARAGRAPH_START + nNumLanguageParaEntries)
                {
                    pSh->Push();        // save cursor
                    SwLangHelper::SelectCurrentPara( *pSh );
                    //Open Format/Character Dialog
                    lcl_CharDialog( *pSh, true, nId, 0, 0 );
                    pSh->Pop( FALSE );  // restore cursor
                }
                else if (nId >= MN_LANGUAGE_ALL_TEXT_START && nId < MN_LANGUAGE_ALL_TEXT_START + nNumLanguageDocEntries - 1)
                {
                    //Set selected language as the default language
                    aNewLangTxt=aLangTable_Document[nId];
                    SwLangHelper::SetLanguage( *pSh, aNewLangTxt, false, aCoreSet );
                }
                else if (nId == MN_LANGUAGE_ALL_TEXT_START + nNumLanguageDocEntries - 1)
                {
                    //Set Language_None as the default language
                    SwLangHelper::SetLanguage_None( *pSh, false, aCoreSet );
                }
                else if (nId == MN_LANGUAGE_ALL_TEXT_START + nNumLanguageDocEntries)
                {
                    // open the dialog "Tools/Options/Language Settings - Language"
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    if (pFact)
                    {
                        VclAbstractDialog* pDlg = pFact->CreateVclDialog( pSh->GetView().GetWindow(), SID_LANGUAGE_OPTIONS );
                        pDlg->Execute();
                        delete pDlg;
                    }
                }

//              pSh->EndAction();
            }
        }
    }

    pSh->EnterStdMode();
}
