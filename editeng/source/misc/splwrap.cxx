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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include<rtl/ustring.hxx>
#include <tools/shl.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>
#include <svtools/langtab.hxx>

#ifndef __RSC
#include <tools/errinf.hxx>
#endif
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/frame/XStorable.hpp>

#include <map>

#include <editeng/svxenum.hxx>
#include <editeng/splwrap.hxx>      // Der Wrapper
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

void SvxPrepareAutoCorrect( String &rOldText, String &rNewText )
{
    // This function should be used to strip (or add) trailing '.' from
    // the strings before passing them on to the autocorrect function in
    // order that the autocorrect function will hopefully
    // works properly with normal words and abbreviations (with trailing '.')
    // independ of if they are at the end of the sentence or not.
    //
    // rOldText: text to be replaced
    // rNewText: replacement text

    xub_StrLen  nOldLen = rOldText.Len(),
                nNewLen = rNewText.Len();
    if (nOldLen && nNewLen)
    {
        sal_Bool bOldHasDot = sal_Unicode( '.' ) == rOldText.GetChar( nOldLen - 1 ),
             bNewHasDot = sal_Unicode( '.' ) == rNewText.GetChar( nNewLen - 1 );
        if (bOldHasDot && !bNewHasDot
            /*this is: !(bOldHasDot && bNewHasDot) && bOldHasDot*/)
            rOldText.Erase( nOldLen - 1 );
    }
}

// -----------------------------------------------------------------------

#define SVX_LANG_NEED_CHECK         0
#define SVX_LANG_OK                 1
#define SVX_LANG_MISSING            2
#define SVX_LANG_MISSING_DO_WARN    3

#define SVX_FLAGS_NEW


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
            String aErr( SvtLanguageTable::GetLanguageString( nLang ) );
            ErrorHandler::HandleError(
                *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
            nTmpSpell = SVX_LANG_MISSING;
        }
        if (SVX_LANG_MISSING_DO_WARN == nTmpHyph)
        {
            String aErr( SvtLanguageTable::GetLanguageString( nLang ) );
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
 *  Beschreibung: Ctor, die Pruefreihenfolge wird festgelegt
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
    Reference< beans::XPropertySet >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bWrapReverse = xProp.is() ?
        *(sal_Bool*)xProp->getPropertyValue(
            ::rtl::OUString::createFromAscii(UPN_IS_WRAP_REVERSE) ).getValue()
        : sal_False;
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
        if (xHyph.is()  &&  xHyph->hasLocale( SvxCreateLocale( nLang ) ))
            nTmpVal = SVX_LANG_OK;
        nVal &= 0x00FF;
        nVal |= nTmpVal << 8;

        rLCS[ nLang ] = nVal;
    }

    return (sal_Int16) nVal;
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SpellStart( SvxSpellArea /*eSpell*/ )
{   // Hier muessen die notwendigen Vorbereitungen fuer SpellContinue
}   // im uebergebenen Bereich getroffen werden.

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::HasOtherCnt()
{
    return sal_False; // Gibt es ueberhaupt einen Sonderbereich?
}

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::SpellMore()
{
    return sal_False; // Sollen weitere Dokumente geprueft werden?
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SpellEnd()
{   // Bereich ist abgeschlossen, ggf. Aufraeumen

    // display error for last language not found
    ShowLanguageErrors();
}

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::SpellContinue()
{
    return sal_False;
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::AutoCorrect( const String&, const String& )
{
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ScrollArea()
{   // Scrollarea einstellen
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ChangeWord( const String&, const sal_uInt16 )
{   // Wort ersetzen
}

// -----------------------------------------------------------------------


String SvxSpellWrapper::GetThesWord()
{
    // Welches Wort soll nachgeschlagen werden?
    return String();
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ChangeThesWord( const String& )
{
    // Wort wg. Thesaurus ersetzen
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::StartThesaurus( const String &rWord, sal_uInt16 nLanguage )
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

void SvxSpellWrapper::ReplaceAll( const String &, sal_Int16 )
{   // Wort aus der Replace-Liste ersetzen
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SetLanguage( const sal_uInt16 )
{   // Sprache aendern
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::InsertHyphen( const sal_uInt16 )
{   // Hyphen einfuegen bzw. loeschen
}

// -----------------------------------------------------------------------
// Pruefung der Dokumentbereiche in der durch die Flags angegebenen Reihenfolge


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
                            SvxLocaleToLanguage( xHyphWord->getLocale() ),
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
// Naechsten Bereich auswaehlen


sal_Bool SvxSpellWrapper::SpellNext( )
{
    Reference< beans::XPropertySet >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bWrapReverse = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue(
                ::rtl::OUString::createFromAscii(UPN_IS_WRAP_REVERSE) ).getValue()
            : sal_False;
    sal_Bool bActRev = bRevAllowed && bWrapReverse;

    // bActRev ist die Richtung nach dem Spellen, bReverse die am Anfang.
    if( bActRev == bReverse )
    {                           // Keine Richtungsaenderung, also ist
        if( bStartChk )         // der gewuenschte Bereich ( bStartChk )
            bStartDone = sal_True;  // vollstaendig abgearbeitet.
        else
            bEndDone = sal_True;
    }
    else if( bReverse == bStartChk ) // Bei einer Richtungsaenderung kann
    {                          // u.U. auch ein Bereich abgearbeitet sein.
        if( bStartChk )        // Sollte der vordere Teil rueckwaerts gespellt
            bEndDone = sal_True;   // werden und wir kehren unterwegs um, so ist
        else                   // der hintere Teil abgearbeitet (und umgekehrt).
            bStartDone = sal_True;
    }

    bReverse = bActRev;
    if( bOtherCntnt && bStartDone && bEndDone ) // Dokument komplett geprueft?
    {
        if ( SpellMore() )  // ein weiteres Dokument pruefen?
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
        sal_Bool bIsSpellSpecial = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue(
                ::rtl::OUString::createFromAscii(UPN_IS_SPELL_SPECIAL) ).getValue()
            : sal_False;
        // Bodybereich erledigt, Frage nach Sonderbereich
        if( !IsHyphen() && bIsSpellSpecial && HasOtherCnt() )
        {
            SpellStart( SVX_SPELL_OTHER );
            bOtherCntnt = bGoOn = sal_True;
        }
        else if ( SpellMore() )  // ein weiteres Dokument pruefen?
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
        // Ein BODY_Bereich erledigt, Frage nach dem anderen BODY_Bereich
        WAIT_OFF();

// Sobald im Dialog das DontWrapAround gesetzt werden kann, kann der
// folgende #ifdef-Zweig aktiviert werden ...
#ifdef USED
        sal_Bool bDontWrapAround = IsHyphen() ?
            pSpell->GetOptions() & DONT_WRAPAROUND :
            pSpell->GetHyphOptions() & HYPH_DONT_WRAPAROUND;
        if( bDontWrapAround )
#else
        sal_uInt16 nResId = bReverse ? RID_SVXQB_BW_CONTINUE : RID_SVXQB_CONTINUE;
        QueryBox aBox( pWin, EditResId( nResId ) );
        if ( aBox.Execute() != RET_YES )
#endif

        {
            // Verzicht auf den anderen Bereich, ggf. Frage nach Sonderbereich
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

    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
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
                     SvxLocaleToLanguage( xTmp->getLocale() ) == LANGUAGE_NONE )
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
                xAllRightDic->add( xAlt->getWord(), sal_False, ::rtl::OUString() );
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
                                SvxLocaleToLanguage( xAlt->getLocale() ) );
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



