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

#include<rtl/ustring.hxx>
#include <tools/shl.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/langtab.hxx>

#ifndef __RSC
#include <tools/errinf.hxx>
#endif
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/frame/XStorable.hpp>

#include <map>

#include <editeng/svxenum.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/edtdlg.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <editeng/editids.hrc>
#include <editeng/editerr.hxx>

#define WAIT_ON() if(pWin != NULL) { pWin->EnterWait(); }

#define WAIT_OFF() if(pWin != NULL) { pWin->LeaveWait(); }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;


// misc functions ---------------------------------------------

void SvxPrepareAutoCorrect( OUString &rOldText, const OUString &rNewText )
{
    // This function should be used to strip (or add) trailing '.' from
    // the strings before passing them on to the autocorrect function in
    // order that the autocorrect function will hopefully
    // works properly with normal words and abbreviations (with trailing '.')
    // independ of if they are at the end of the sentence or not.
    //
    // rOldText: text to be replaced
    // rNewText: replacement text

    xub_StrLen  nOldLen = rOldText.getLength(),
                nNewLen = rNewText.getLength();
    if (nOldLen && nNewLen)
    {
        bool bOldHasDot = '.' == rOldText[ nOldLen - 1 ],
             bNewHasDot = '.' == rNewText[ nNewLen - 1 ];
        if (bOldHasDot && !bNewHasDot
            /*this is: !(bOldHasDot && bNewHasDot) && bOldHasDot*/)
            rOldText = rOldText.copy( 0, nOldLen - 1 );
    }
}

#define SVX_LANG_NEED_CHECK         0
#define SVX_LANG_OK                 1
#define SVX_LANG_MISSING            2
#define SVX_LANG_MISSING_DO_WARN    3

struct lt_LanguageType
{
    bool operator()( LanguageType n1, LanguageType n2 ) const
    {
        return n1 < n2;
    }
};

typedef std::map< LanguageType, sal_uInt16, lt_LanguageType >   LangCheckState_map_t;

static LangCheckState_map_t & GetLangCheckState()
{
    static LangCheckState_map_t aLangCheckState;
    return aLangCheckState;
}

void SvxSpellWrapper::ShowLanguageErrors()
{
    // display message boxes for languages not available for
    // spellchecking or hyphenation
    LangCheckState_map_t &rLCS = GetLangCheckState();
    LangCheckState_map_t::iterator aIt( rLCS.begin() );
    while (aIt != rLCS.end())
    {
        LanguageType nLang = aIt->first;
        sal_uInt16   nVal  = aIt->second;
        sal_uInt16 nTmpSpell = nVal & 0x00FF;
        sal_uInt16 nTmpHyph  = (nVal >> 8) & 0x00FF;

        if (SVX_LANG_MISSING_DO_WARN == nTmpSpell)
        {
            OUString aErr( SvtLanguageTable::GetLanguageString( nLang ) );
            ErrorHandler::HandleError(
                *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
            nTmpSpell = SVX_LANG_MISSING;
        }
        if (SVX_LANG_MISSING_DO_WARN == nTmpHyph)
        {
            OUString aErr( SvtLanguageTable::GetLanguageString( nLang ) );
            ErrorHandler::HandleError(
                *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
            nTmpHyph = SVX_LANG_MISSING;
        }

        rLCS[ nLang ] = (nTmpHyph << 8) | nTmpSpell;
        ++aIt;
    }

}

SvxSpellWrapper::~SvxSpellWrapper()
{
}

/*--------------------------------------------------------------------
 *  Description: Constructor, the test sequence is determined
 *
 *  !bStart && !bOtherCntnt:    BODY_END,   BODY_START, OTHER
 *  !bStart && bOtherCntnt:     OTHER,      BODY
 *  bStart && !bOtherCntnt:     BODY_END,   OTHER
 *  bStart && bOtherCntnt:      OTHER
 *
 --------------------------------------------------------------------*/

SvxSpellWrapper::SvxSpellWrapper( Window* pWn,
    Reference< XSpellChecker1 >  &xSpellChecker,
    const sal_Bool bStart, const sal_Bool bIsAllRight,
    const sal_Bool bOther, const sal_Bool bRevAllow ) :

    pWin        ( pWn ),
    xSpell      ( xSpellChecker ),
    bOtherCntnt ( bOther ),
    bDialog     ( sal_False ),
    bHyphen     ( sal_False ),
    bAuto       ( sal_False ),
    bStartChk   ( bOther ),
    bRevAllowed ( bRevAllow ),
    bAllRight   ( bIsAllRight )
{
    Reference< linguistic2::XLinguProperties >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bWrapReverse = xProp.is() ? xProp->getIsWrapReverse() : sal_False;
    bReverse = bRevAllow && bWrapReverse;
    bStartDone = bOther || ( !bReverse && bStart );
    bEndDone   = bReverse && bStart && !bOther;
}

// -----------------------------------------------------------------------

SvxSpellWrapper::SvxSpellWrapper( Window* pWn,
        Reference< XHyphenator >  &xHyphenator,
        const sal_Bool bStart, const sal_Bool bOther ) :
    pWin        ( pWn ),
    xHyph       ( xHyphenator ),
    bOtherCntnt ( bOther ),
    bDialog     ( sal_False ),
    bHyphen     ( sal_False ),
    bAuto       ( sal_False ),
    bReverse    ( sal_False ),
    bStartDone  ( bOther || ( !bReverse && bStart ) ),
    bEndDone    ( bReverse && bStart && !bOther ),
    bStartChk   ( bOther ),
    bRevAllowed ( sal_False ),
    bAllRight   ( sal_True )
{
}

// -----------------------------------------------------------------------

sal_Int16 SvxSpellWrapper::CheckSpellLang(
        Reference< XSpellChecker1 > xSpell, sal_Int16 nLang)
{
    LangCheckState_map_t &rLCS = GetLangCheckState();

    LangCheckState_map_t::iterator aIt( rLCS.find( nLang ) );
    sal_uInt16 nVal = aIt == rLCS.end() ? SVX_LANG_NEED_CHECK : aIt->second;

    if (aIt == rLCS.end())
        rLCS[ nLang ] = nVal;

    if (SVX_LANG_NEED_CHECK == (nVal & 0x00FF))
    {
        sal_uInt16 nTmpVal = SVX_LANG_MISSING_DO_WARN;
        if (xSpell.is()  &&  xSpell->hasLanguage( nLang ))
            nTmpVal = SVX_LANG_OK;
        nVal &= 0xFF00;
        nVal |= nTmpVal;

        rLCS[ nLang ] = nVal;
    }

    return (sal_Int16) nVal;
}

sal_Int16 SvxSpellWrapper::CheckHyphLang(
        Reference< XHyphenator >  xHyph, sal_Int16 nLang)
{
    LangCheckState_map_t &rLCS = GetLangCheckState();

    LangCheckState_map_t::iterator aIt( rLCS.find( nLang ) );
    sal_uInt16 nVal = aIt == rLCS.end() ? 0 : aIt->second;

    if (aIt == rLCS.end())
        rLCS[ nLang ] = nVal;

    if (SVX_LANG_NEED_CHECK == ((nVal >> 8) & 0x00FF))
    {
        sal_uInt16 nTmpVal = SVX_LANG_MISSING_DO_WARN;
        if (xHyph.is()  &&  xHyph->hasLocale( LanguageTag::convertToLocale( nLang ) ))
            nTmpVal = SVX_LANG_OK;
        nVal &= 0x00FF;
        nVal |= nTmpVal << 8;

        rLCS[ nLang ] = nVal;
    }

    return (sal_Int16) nVal;
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SpellStart( SvxSpellArea /*eSpell*/ )
{ // Here, the necessary preparations be made for SpellContinue in the
} // given area.

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::HasOtherCnt()
{
    return sal_False; // Is there a special area?
}

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::SpellMore()
{
    return sal_False; // Should additional documents be examined?
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SpellEnd()
{   // Area is complete, tidy up if necessary

    // display error for last language not found
    ShowLanguageErrors();
}

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::SpellContinue()
{
    return sal_False;
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::AutoCorrect( const OUString&, const OUString& )
{
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ScrollArea()
{   // Set Scroll area
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ChangeWord( const OUString&, const sal_uInt16 )
{   // Insert Word
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ChangeThesWord( const OUString& )
{
    // replace word due to Thesaurus.
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::StartThesaurus( const OUString &rWord, sal_uInt16 nLanguage )
{
    Reference< XThesaurus >  xThes( SvxGetThesaurus() );
    if (!xThes.is())
    {
        InfoBox( pWin, EE_RESSTR( RID_SVXSTR_HMERR_THESAURUS ) ).Execute();
        return;
    }

    WAIT_ON();  // while looking up for initial word
    EditAbstractDialogFactory* pFact = EditAbstractDialogFactory::Create();
    AbstractThesaurusDialog* pDlg = pFact->CreateThesaurusDialog( pWin, xThes, rWord, nLanguage );
    WAIT_OFF();
    if ( pDlg->Execute()== RET_OK )
    {
        ChangeThesWord( pDlg->GetWord() );
    }
    delete pDlg;
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::ReplaceAll( const OUString &, sal_Int16 )
{   // Replace Word from the Replace list
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SetLanguage( const sal_uInt16 )
{   // Set Language
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::InsertHyphen( const sal_uInt16 )
{   // inserting and deleting Hyphae
}

// -----------------------------------------------------------------------
// Testing of the document areas in the order specified by the flags


void SvxSpellWrapper::SpellDocument( )
{
    if ( bOtherCntnt )
    {
        bReverse = sal_False;
        SpellStart( SVX_SPELL_OTHER );
    }
    else
    {
        bStartChk = bReverse;
        SpellStart( bReverse ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
    }

    if ( FindSpellError() )
    {
        Reference< XSpellAlternatives >     xAlt( GetLast(), UNO_QUERY );
        Reference< XHyphenatedWord >        xHyphWord( GetLast(), UNO_QUERY );

        Window *pOld = pWin;
        bDialog = sal_True;
        if (xHyphWord.is())
        {
            EditAbstractDialogFactory* pFact = EditAbstractDialogFactory::Create();
            AbstractHyphenWordDialog* pDlg = pFact->CreateHyphenWordDialog( pWin,
                            xHyphWord->getWord(),
                            LanguageTag( xHyphWord->getLocale() ).getLanguageType(),
                            xHyph, this );
            pWin = pDlg->GetWindow();
            pDlg->Execute();
            delete pDlg;
        }
        bDialog = sal_False;
        pWin = pOld;
    };
}

// -----------------------------------------------------------------------
// Select the next area


sal_Bool SvxSpellWrapper::SpellNext( )
{
    Reference< linguistic2::XLinguProperties >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bWrapReverse = xProp.is() ? xProp->getIsWrapReverse() : sal_False;
    sal_Bool bActRev = bRevAllowed && bWrapReverse;

    // bActRev is the direction after Spell checking, bReverse is the one
    // at the beginning.
    if( bActRev == bReverse )
    {                           // No change of direction, thus is the
        if( bStartChk )         // desired area ( bStartChk )
            bStartDone = sal_True;  // completely processed.
        else
            bEndDone = sal_True;
    }
    else if( bReverse == bStartChk ) //For a change of direction, an area can
    {                          // be processed during certain circumstances
        if( bStartChk )        // If the firdt part is spell checked in backwards
            bEndDone = sal_True;   // and this is reversed in the process, then
        else                   // then the end part is processed (and vice-versa).
            bStartDone = sal_True;
    }

    bReverse = bActRev;
    if( bOtherCntnt && bStartDone && bEndDone ) // Document has been fully checked?
    {
        if ( SpellMore() )  // spell check another document?
        {
            bOtherCntnt = sal_False;
            bStartDone = !bReverse;
            bEndDone  = bReverse;
            SpellStart( SVX_SPELL_BODY );
            return sal_True;
        }
        return sal_False;
    }

    sal_Bool bGoOn = sal_False;

    if ( bOtherCntnt )
    {
        bStartChk = sal_False;
        SpellStart( SVX_SPELL_BODY );
        bGoOn = sal_True;
    }
    else if ( bStartDone && bEndDone )
    {
        sal_Bool bIsSpellSpecial = xProp.is() ? xProp->getIsSpellSpecial() : sal_False;
        // Body area done, ask for special area
        if( !IsHyphen() && bIsSpellSpecial && HasOtherCnt() )
        {
            SpellStart( SVX_SPELL_OTHER );
            bOtherCntnt = bGoOn = sal_True;
        }
        else if ( SpellMore() )  // check another document?
        {
            bOtherCntnt = sal_False;
            bStartDone = !bReverse;
            bEndDone  = bReverse;
            SpellStart( SVX_SPELL_BODY );
            return sal_True;
        }
    }
    else
    {
        // a BODY_area done, ask for the other BODY_area
        WAIT_OFF();

        sal_uInt16 nResId = bReverse ? RID_SVXQB_BW_CONTINUE : RID_SVXQB_CONTINUE;
        QueryBox aBox( pWin, EditResId( nResId ) );
        if ( aBox.Execute() != RET_YES )
        {
            // sacrifice the other area if necessary ask for special area
            WAIT_ON();
            bStartDone = bEndDone = sal_True;
            return SpellNext();
        }
        else
        {
            bStartChk = !bStartDone;
            SpellStart( bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
            bGoOn = sal_True;
        }
        WAIT_ON();
    }
    return bGoOn;
}

// -----------------------------------------------------------------------

Reference< XDictionary >  SvxSpellWrapper::GetAllRightDic() const
{
    Reference< XDictionary >  xDic;

    Reference< XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
    {
        Sequence< Reference< XDictionary >  > aDics( xDicList->getDictionaries() );
        const Reference< XDictionary >  *pDic = aDics.getConstArray();
        sal_Int32 nCount = aDics.getLength();

        sal_Int32 i = 0;
        while (!xDic.is()  &&  i < nCount)
        {
            Reference< XDictionary >  xTmp( pDic[i], UNO_QUERY );
            if (xTmp.is())
            {
                if ( xTmp->isActive() &&
                     xTmp->getDictionaryType() != DictionaryType_NEGATIVE &&
                     LanguageTag( xTmp->getLocale() ).getLanguageType() == LANGUAGE_NONE )
                {
                    Reference< frame::XStorable >  xStor( xTmp, UNO_QUERY );
                    if (xStor.is() && xStor->hasLocation() && !xStor->isReadonly())
                    {
                        xDic = xTmp;
                    }
                }
            }
            ++i;
        }

        if (!xDic.is())
        {
            xDic = SvxGetOrCreatePosDic( xDicList );
            if (xDic.is())
                xDic->setActive( sal_True );
        }
    }

    return xDic;
}

// -----------------------------------------------------------------------

sal_Bool SvxSpellWrapper::FindSpellError()
{
    ShowLanguageErrors();

     Reference< XInterface >    xRef;

    WAIT_ON();
    sal_Bool bSpell = sal_True;

    Reference< XDictionary >  xAllRightDic;
    if (IsAllRight())
        xAllRightDic = GetAllRightDic();

    while ( bSpell )
    {
        SpellContinue();

        Reference< XSpellAlternatives >     xAlt( GetLast(), UNO_QUERY );
        Reference< XHyphenatedWord >        xHyphWord( GetLast(), UNO_QUERY );

        if (xAlt.is())
        {
            if (IsAllRight() && xAllRightDic.is())
            {
                xAllRightDic->add( xAlt->getWord(), sal_False, OUString() );
            }
            else
            {
                // look up in ChangeAllList for misspelled word
                Reference< XDictionary >    xChangeAllList(
                        SvxGetChangeAllList(), UNO_QUERY );
                Reference< XDictionaryEntry >   xEntry;
                if (xChangeAllList.is())
                    xEntry = xChangeAllList->getEntry( xAlt->getWord() );

                if (xEntry.is())
                {
                    // replace word without asking
                    ReplaceAll( xEntry->getReplacementText(),
                                LanguageTag( xAlt->getLocale() ).getLanguageType() );
                }
                else
                    bSpell = sal_False;
            }
        }
        else if (xHyphWord.is())
            bSpell = sal_False;
        else
        {
            SpellEnd();
            bSpell = SpellNext();
        }
    }
    WAIT_OFF();
    return GetLast().is();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
