/*************************************************************************
 *
 *  $RCSfile: splwrap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#pragma hdrstop

// include ---------------------------------------------------------------

#ifndef _RTL_USTRING_HXX_
#include<rtl/ustring.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif


#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#ifndef _SVARRAY_HXX
#include <svtools/svstdarr.hxx>
#endif

#ifndef __RSC
#include <tools/errinf.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svxerr.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic/XDictionary1.hpp>
#endif



#include "svxenum.hxx"
#include "spldlg.hxx"       // Der SpellDialog
#include "hyphen.hxx"       // Der HyphenDialog
#include "splwrap.hxx"      // Der Wrapper
#include "thesdlg.hxx"      // ThesaurusDlg
#include "dialmgr.hxx"

#include "dialogs.hrc"

#define WAIT_ON()   pWin->EnterWait()
#define WAIT_OFF()  pWin->LeaveWait()

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic;
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
    bRevAllowed ( bRevAllow )
{
    Reference< beans::XPropertySet >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bWrapReverse = xProp.is() ?
        *(sal_Bool*)xProp->getPropertyValue(
            ::rtl::OUString::createFromAscii(UPN_IS_WRAP_REVERSE) ).getValue()
        : sal_False;
    bReverse = bRevAllow && bWrapReverse;
    bAllRight = bIsAllRight;
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
    bStartDone  ( bOther || ( !bReverse && bStart ) ),
    bEndDone    ( bReverse && bStart && !bOther ),
    bStartChk   ( bOther ),
    bRevAllowed ( sal_False ),
    bReverse    ( sal_False )
{
}

// -----------------------------------------------------------------------

#define SVX_LANG_NEED_CHECK         0
#define SVX_LANG_OK                 1
#define SVX_LANG_MISSING            2
#define SVX_LANG_MISSING_DO_WARN    3

#define SVX_FLAGS_NEW

static SvUShortsSort    _aLanguages(16, 16);
static SvUShorts        _aState(16, 16);    // lowerbyte spell values,
                                            // higherbyte hyph values

sal_Int16 SvxSpellWrapper::CheckSpellLang(
        Reference< XSpellChecker1 > xSpell, sal_Int16 nLang)
{
    DBG_ASSERT( _aLanguages.Count() == _aState.Count(), "inconsistent arrays");

    sal_uInt16 nPos;
    sal_Bool bFound = _aLanguages.Seek_Entry( nLang, &nPos );
    sal_uInt16 nVal = bFound ? _aState.GetObject( nPos ) : 0;

    if (!bFound)
    {
        _aLanguages.Insert( (sal_uInt16) nLang, nPos );
        _aState.Insert( nVal , nPos );
    }

    if (SVX_LANG_NEED_CHECK == (nVal & 0x00FF))
    {
        sal_uInt16 nTmpVal = SVX_LANG_MISSING_DO_WARN;
        if (xSpell.is()  &&  xSpell->hasLanguage( nLang ))
            nTmpVal = SVX_LANG_OK;
        nVal &= 0xFF00;
        nVal |= nTmpVal;

        _aState.Replace( nVal , nPos );
    }

    return (sal_Int16) _aState.GetObject( nPos );
}

sal_Int16 SvxSpellWrapper::CheckHyphLang(
        Reference< XHyphenator >  xHyph, sal_Int16 nLang)
{
    DBG_ASSERT( _aLanguages.Count() == _aState.Count(), "inconsistent arrays");

    sal_uInt16 nPos;
    sal_Bool bFound = _aLanguages.Seek_Entry( nLang, &nPos );
    sal_uInt16 nVal = bFound ? _aState.GetObject( nPos ) : 0;

    if (!bFound)
    {
        _aLanguages.Insert( (sal_uInt16) nLang, nPos );
        _aState.Insert( nVal , nPos );
    }

    if (SVX_LANG_NEED_CHECK == ((nVal >> 8) & 0x00FF))
    {
        sal_uInt16 nTmpVal = SVX_LANG_MISSING_DO_WARN;
        if (xHyph.is()  &&  xHyph->hasLocale( SvxCreateLocale( nLang ) ))
            nTmpVal = SVX_LANG_OK;
        nVal &= 0x00FF;
        nVal |= nTmpVal << 8;

        _aState.Replace( nVal , nPos );
    }

    return (sal_Int16) _aState.GetObject( nPos );
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SpellStart( SvxSpellArea eSpell )
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
    DBG_ASSERT( _aLanguages.Count() == _aState.Count(), "inconsistent arrays");

    sal_uInt16 nCount = _aLanguages.Count();
    for (sal_uInt16 i = 0;  i < nCount;  ++i)
    {
        sal_Int16 nLang = (sal_Int16) _aLanguages.GetObject( i );
        sal_uInt16 nVal = _aState.GetObject( i );
        sal_uInt16 nTmpSpell = nVal & 0x00FF;
        sal_uInt16 nTmpHyph  = (nVal >> 8) & 0x00FF;

        if (SVX_LANG_MISSING_DO_WARN == nTmpSpell)
        {
            String aErr( ::GetLanguageString( nLang ) );
            ErrorHandler::HandleError(
                *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
            nTmpSpell = SVX_LANG_MISSING;
        }
        if (SVX_LANG_MISSING_DO_WARN == nTmpHyph)
        {
            String aErr( ::GetLanguageString( nLang ) );
            ErrorHandler::HandleError(
                *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
            nTmpHyph = SVX_LANG_MISSING;
        }

        _aState.Replace( (nTmpHyph << 8) | nTmpSpell, i );
    }
}

// -----------------------------------------------------------------------


sal_Bool SvxSpellWrapper::SpellContinue()
{
    return sal_False;
}

// -----------------------------------------------------------------------

void SvxSpellWrapper::AutoCorrect( const String& rAktWord,
                                   const String& rNewWord )
{
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ScrollArea()
{   // Scrollarea einstellen
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::ChangeWord( const String& rNewWord, const sal_uInt16 nLang )
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

    String sErr( SVX_RES( RID_SVXSTR_HMERR_THESAURUS ) );

    Reference< XThesaurus >  xThes( SvxGetThesaurus() );
    if (!xThes.is())
    {
        InfoBox( pWin, sErr ).Execute();
        return;
    }

    WAIT_ON();  // while looking up for initial word
    SvxThesaurusDialog aDlg(pWin, xThes, rWord, nLanguage);
    WAIT_OFF();

    if ( aDlg.Execute()== RET_OK )
    {
        if ( bDialog )
        {
            SvxSpellCheckDialog* pSplDlg = (SvxSpellCheckDialog*)GetWin();
            pSplDlg->SetNewEditWord( aDlg.GetWord() );
            pSplDlg->SetLanguage( aDlg.GetLanguage() );
        }
        else
        {
            ChangeThesWord( aDlg.GetWord() );
        }
    }

}

// -----------------------------------------------------------------------

void SvxSpellWrapper::ReplaceAll( const String &rNewText, sal_Int16 nLanguage )
{   // Wort aus der Replace-Liste ersetzen
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::SetLanguage( const sal_uInt16 nLang )
{   // Sprache aendern
}

// -----------------------------------------------------------------------


void SvxSpellWrapper::InsertHyphen( const sal_uInt16 nPos )
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
            DBG_ASSERT(xHyphWord.is(), "NULL pointer");
            SvxHyphenWordDialog* pDlg =
                new SvxHyphenWordDialog( xHyphWord->getWord(),
                            SvxLocaleToLanguage( xHyphWord->getLocale() ),
                            pWin, xHyph, this );
            pWin = pDlg;
            pDlg->Execute();
            delete pDlg;
        }
        else
        {
            SvxSpellCheckDialog* pDlg =
                new SvxSpellCheckDialog( pWin, xSpell, this );

            if ( !bAuto )
                pDlg->HideAutoCorrect();
            pWin = pDlg;
            // Visarea richtig setzen
            ScrollArea();

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

    ResMgr* pMgr = DIALOG_MGR();
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
        QueryBox aBox( pWin, ResId( nResId, pMgr ) );
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

Reference< XDictionary1 >  SvxSpellWrapper::GetAllRightDic() const
{
    Reference< XDictionary1 >  xDic;

    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
    {
        Sequence< Reference< XDictionary >  > aDics( xDicList->getDictionaries() );
        const Reference< XDictionary >  *pDic = aDics.getConstArray();
        sal_Int32 nCount = aDics.getLength();

        sal_Int32 i = 0;
        while (!xDic.is()  &&  i < nCount)
        {
            Reference< XDictionary1 >  xTmp( pDic[i], UNO_QUERY );
            if (xTmp.is())
            {
                if ( xTmp->isActive() &&
                     xTmp->getDictionaryType() != DictionaryType_NEGATIVE &&
                     xTmp->getLanguage() == LANGUAGE_NONE )
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
     Reference< XInterface >    xRef;

    WAIT_ON();
    sal_Bool bSpell = sal_True;

    Reference< XDictionary1 >  xAllRightDic;
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
                xAllRightDic->add( xAlt->getWord(), sal_False,
                            ::rtl::OUString(), LANGUAGE_NONE );
            }
            else
            {
                // look up in ChangeAllList for misspelled word
                Reference< XDictionary1 >   xChangeAllList(
                        SvxGetChangeAllList(), UNO_QUERY );
                Reference< XDictionaryEntry >   xEntry;
                if (xChangeAllList.is())
                    xEntry = xChangeAllList->getEntry( xAlt->getWord() );

                if (xEntry.is())
                {
                    // replace word without asking
                    sal_Int16 nRplcLang = SvxLocaleToLanguage(
                            xEntry->getReplacementLocale() );
                    ReplaceAll( xEntry->getReplacementText(), nRplcLang );
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


