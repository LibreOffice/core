/*************************************************************************
 *
 *  $RCSfile: txtedt.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-14 10:09:29 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop


// So kann man die Linguistik-Statistik ( (Tmp-Path)\swlingu.stk ) aktivieren:
//#define LINGU_STATISTIK
#ifdef LINGU_STATISTIK
    #include <stdio.h>          // in SwLinguStatistik::DTOR
    #include <stdlib.h>         // getenv()
    #include <time.h>           // clock()
    #include "viewsh.hxx"       // ViewShell::GetHyphenator
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#ifndef _SPLARGS_HXX
#include <splargs.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // GetDoc()
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SPLARGS_HXX
#include <splargs.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>   // GetSystemLang
#endif
#ifndef _TXTTYPES_HXX
#include <txttypes.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _TXATRITR_HXX
#include <txatritr.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;


/*
 * Ein Zeichen wurde eingefuegt.
 */



SwTxtNode& SwTxtNode::Insert( xub_Unicode c, const SwIndex &rIdx )
{
    xub_StrLen nOrigLen = aText.Len();

    ASSERT( rIdx <= nOrigLen, "Array ueberindiziert." );
    ASSERT( nOrigLen < STRING_LEN, "USHRT_MAX ueberschritten." );

    if( nOrigLen == aText.Insert( c, rIdx.GetIndex() ).Len() )
        return *this;

    Update(rIdx,1);

    // leere Hints und Feldattribute an rIdx.GetIndex suchen
    if( pSwpHints )
    {
        USHORT* pEndIdx;
        for( USHORT i=0; i < pSwpHints->Count() &&
                rIdx >= *(*pSwpHints)[i]->GetStart(); ++i)
        {
            SwTxtAttr *pHt = pSwpHints->GetHt(i);
            if( 0 != ( pEndIdx = pHt->GetEnd()) )
            {
                // leere Hints an rIdx.GetIndex ?
                BOOL bEmpty = *pEndIdx == *pHt->GetStart()
                            && rIdx == *pHt->GetStart();

                if( bEmpty )
                {
                    pSwpHints->DeleteAtPos(i);
                    if( bEmpty )
                        *pHt->GetStart() -= 1;
                    else
                        *pEndIdx -= 1;
                    Insert(pHt);
                }
            }
        }
        if ( pSwpHints->CanBeDeleted() )
            DELETEZ( pSwpHints );
    }
    // den Frames Bescheid sagen
    SwInsChr aHint( rIdx.GetIndex()-1 );
    SwModify::Modify( 0, &aHint );
    return *this;
}

inline BOOL InRange(xub_StrLen nIdx, xub_StrLen nStart, xub_StrLen nEnd) {
    return ((nIdx >=nStart) && (nIdx <= nEnd));
}

/*
 * void SwTxtNode::RstAttr(const SwIndex &rIdx, USHORT nLen)
 *
 * loescht alle Attribute ab der Position rIdx ueber eine Laenge
 * von nLen.
 */

/* 5 Faelle:
 * 1) Das Attribut liegt vollstaendig im Bereich:
 *    -> loeschen
 * 2) Das Attributende liegt im Bereich:
 *    -> Loeschen, mit neuem Ende einfuegen
 * 3) Der Attributanfang liegt im Bereich:
 *    -> Loeschen, mit neuem Anfang einfuegen
 * 4) Das Attrib umfasst den Bereich:
 *       Aufsplitten, d.h.
 *    -> Loeschen, mit alten Anfang und Anfang des Bereiches einfuegen
 *    -> Neues Attribut mit Ende des Bereiches und altem Ende einfuegen
 * 5) Das Attribut liegt ausserhalb des Bereiches
 *     -> nichts tun.
 */



void SwTxtNode::RstAttr(const SwIndex &rIdx, xub_StrLen nLen, USHORT nWhich,
                        const SfxItemSet* pSet, BOOL bInclRefToxMark )
{
    // Attribute?
    if ( !GetpSwpHints() )
        return;

    USHORT i = 0;
    xub_StrLen nStart = rIdx.GetIndex();
    xub_StrLen nEnd = nStart + nLen;
    xub_StrLen *pAttrEnd;
    xub_StrLen nAttrStart;
    SwTxtAttr *pHt;

    BOOL    bChanged = FALSE;

    // nMin und nMax werden invers auf das Maximum bzw. Minimum gesetzt.
    xub_StrLen nMin = aText.Len();
    xub_StrLen nMax = nStart;

    const BOOL bNoLen = !nMin;

    // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
    // des Attributs hinter dem Bereich liegt
    while( (i < pSwpHints->Count()) &&
                ((( nAttrStart = *(*pSwpHints)[i]->GetStart()) < nEnd )
                    || nLen==0) )
    {
        pHt = pSwpHints->GetHt(i);

        // Attribute ohne Ende bleiben drin!
        if ( 0 == (pAttrEnd=pHt->GetEnd()) )
        {
            i++;
            continue;
        }

        // loesche alle TextAttribute die als Attribut im Set vorhanden sind
        if( pSet ? SFX_ITEM_SET != pSet->GetItemState( pHt->Which(), FALSE )
                 : ( nWhich ? nWhich != pHt->Which()
                            : (!bInclRefToxMark &&
                                ( RES_TXTATR_REFMARK == pHt->Which() ||
                                RES_TXTATR_TOXMARK == pHt->Which() ))))
        {
            // Es sollen nur Attribute mit nWhich beachtet werden
            i++;
            continue;
        }


        if( nStart <= nAttrStart )          // Faelle: 1,3,5
        {
            if( nEnd > nAttrStart
                || ( nEnd == *pAttrEnd && nEnd==nAttrStart ) )
            {
                // Faelle: 1,3
                if ( nMin > nAttrStart )
                    nMin = nAttrStart;
                if ( nMax < *pAttrEnd )
                    nMax = *pAttrEnd;
                // Falls wir nur ein nichtaufgespanntes Attribut entfernen,
                // tun wir mal so, als ob sich nichts geaendert hat.
                bChanged = bChanged || nEnd > nAttrStart || bNoLen;
                if( *pAttrEnd <= nEnd )     // Fall: 1
                {
                    pSwpHints->DeleteAtPos(i);
                    DestroyAttr( pHt );

                    // falls das letzte Attribut ein Field ist, loescht
                    // dieses das HintsArray !!!
                    if( !pSwpHints )
                        break;

                    //JP 26.11.96:
                    // beim DeleteAtPos wird ein Resort ausgefuehrt!!
                    // darum muessen wir wieder bei 0 anfangen!!!
                    // ueber den Fall 3 koennen Attribute nach hinten
                    // verschoben worden sein; damit stimmt jetzt das i
                    // nicht mehr!!!
                    i = 0;

                    continue;
                }
                else                        // Fall: 3
                {
                    pSwpHints->NoteInHistory( pHt );
                    *pHt->GetStart() = nEnd;
                    pSwpHints->NoteInHistory( pHt, TRUE );
                    bChanged = TRUE;
                }
            }
        }
        else                                // Faelle: 2,4,5
            if( *pAttrEnd > nStart )        // Faelle: 2,4
            {
                if( *pAttrEnd < nEnd )      // Fall: 2
                {
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = TRUE;
                    pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStart;
                    pSwpHints->NoteInHistory( pHt, TRUE );
                }
                else if( nLen )             // Fall: 4
                {       // bei Lange 0 werden beide Hints vom Insert(Ht)
                        // wieder zu einem zusammengezogen !!!!
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = TRUE;
                    xub_StrLen nTmpEnd = *pAttrEnd;
                    pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStart;
                    pSwpHints->NoteInHistory( pHt, TRUE );

                    if( nEnd < nTmpEnd &&
                        !pSwpHints->Forget( i, pHt->Which(), nEnd, nTmpEnd ) )
                    {
                        Insert( pHt->GetAttr(), nEnd, nTmpEnd,
                                SETATTR_NOHINTADJUST );
                        // jetzt kein i+1, weil das eingefuegte Attribut
                        // ein anderes auf die Position geschoben hat !
                        continue;
                    }
                }
            }
        ++i;
    }

    if ( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );
    if(bChanged)
    {
        if ( pSwpHints )
        {
            pSwpHints->ClearDummies( *this );
            ((SwpHintsArr*)pSwpHints)->Resort();
            pSwpHints->Merge( *this );
        }
        //TxtFrm's reagieren auf aHint, andere auf aNew
        SwUpdateAttr aHint( nMin, nMax, 0 );
        SwModify::Modify( 0, &aHint );
        SwFmtChg aNew( GetFmtColl() );
        SwModify::Modify( 0, &aNew );
    }
}



/*************************************************************************
 *                SwTxtNode::GetCurWord()
 *
 * Aktuelles Wort zurueckliefern:
 * Wir suchen immer von links nach rechts, es wird also das Wort
 * vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
 * Absatzes, dann wird das erste Wort zurueckgeliefert.
 * Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
 * einen leeren String.
 *************************************************************************/



XubString SwTxtNode::GetCurWord( xub_StrLen nPos )
{
    ASSERT( nPos<=aText.Len() , "SwTxtNode::GetCurWord: Pos hinter String?");
    if( !aText.Len() )
        return aText;

    Boundary aBndry;
    if( pBreakIt->xBreak.is() )
        aBndry = pBreakIt->xBreak->getWordBoundary(
                    aText, nPos, pBreakIt->GetLocale( GetLang( nPos ) ),
                    WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/, TRUE );

    if( aBndry.endPos != aBndry.startPos &&
        IsSymbol( (xub_StrLen)aBndry.startPos ) )
        aBndry.endPos = aBndry.startPos;
    return aText.Copy( (xub_StrLen)aBndry.startPos,
                        (xub_StrLen)(aBndry.endPos - aBndry.startPos) );
}


void SwTxtNode::SetWrong( SwWrongList *pNew )
{
    delete pWrong;
    pWrong = pNew;
}

/*************************************************************************
 *                class SwScanner
 * Hilfsklasse, die beim Spellen die Worte im gewuenschten Bereich
 * nacheinander zur Verfuegung stellt.
 *************************************************************************/

class SwScanner
{
    XubString aWord;
    const SwWrongList* pWrong;
    SwTxtNode* pNode;
    xub_StrLen nEndPos;
    xub_StrLen nBegin;
    xub_StrLen nLen;
    BOOL bReverse;
    BOOL bStart;
public:
    SwScanner( SwTxtNode* pNd, const SwWrongList* pWrng, xub_StrLen nStart,
                xub_StrLen nEnde, BOOL bRev );
    BOOL NextWord( LanguageType aLang );
    const XubString& GetWord() const    { return aWord; }
    xub_StrLen GetBegin() const         { return nBegin; }
    xub_StrLen GetEnd() const           { return nBegin + nLen; }
    xub_StrLen GetLen() const           { return nLen; }
};



SwScanner::SwScanner( SwTxtNode* pNd, const SwWrongList* pWrng,
                        xub_StrLen nStart, xub_StrLen nEnde, BOOL bRev )
    : pNode( pNd ), pWrong( pWrng ), nLen( 0 ), bReverse( bRev ), bStart( TRUE )
{
    ASSERT( pNode->GetTxt().Len(), "SwScanner: EmptyString" );
    if( bReverse )
    {
        nBegin = nEnde;
        nEndPos = nStart;
    }
    else
    {
        nBegin = nStart;
        nEndPos = nEnde;
    }
}

BOOL SwScanner::NextWord( LanguageType aLang )
{
    const XubString& rText = pNode->GetTxt();
    if( bReverse )
    {
        if( nBegin )
        {
            --nBegin;
            if( pNode->GetpSwpHints() )
            {
                while( CH_TXTATR_BREAKWORD == rText.GetChar( nBegin ) ||
                        CH_TXTATR_INWORD == rText.GetChar( nBegin ) )
                {
                    if( pNode->GetTxtAttr( nBegin ) )
                    {
                        if( nBegin )
                            --nBegin;
                        else
                            return FALSE;
                    }
                    else
                        break;
                }
            }
        }
        else
            return FALSE;
    }
    else if( nBegin + nLen >= rText.Len() )
        return FALSE;

    if( pWrong )
    {
        nBegin = bReverse ? pWrong->LastWrong( nBegin )
                          : pWrong->NextWrong( nBegin );
        if( STRING_LEN == nBegin )
            return FALSE;
    }


    Boundary aBound;
    if( bStart )
    {
        aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
            pBreakIt->GetLocale( aLang ), WordType::DICTIONARY_WORD, !bReverse );
        bStart = aBound.startPos != aBound.endPos;
        if( bStart )
        {
            if( bReverse )
            {
                if( nEndPos > aBound.startPos )
                    nEndPos = (xub_StrLen)aBound.startPos;
            }
            else if( nEndPos < aBound.endPos && nEndPos > aBound.startPos )
                nEndPos = (xub_StrLen)aBound.endPos;
        }
    }
    if( !bStart )
    {
        if( bReverse )
            aBound = pBreakIt->xBreak->previousWord( rText, nBegin,
                    pBreakIt->GetLocale( aLang ), WordType::DICTIONARY_WORD );
        else
            aBound = pBreakIt->xBreak->nextWord( rText, nBegin,
                    pBreakIt->GetLocale( aLang ), WordType::DICTIONARY_WORD );
    }
    else
        bStart = FALSE;

    nBegin = (xub_StrLen)aBound.startPos;
    nLen = aBound.endPos - nBegin;
    if( !nLen )
        return FALSE;

    aWord = rText.Copy( nBegin, nLen );
    if( bReverse )
    {
        if( nBegin < nEndPos )
            return FALSE;
    }
    else
    {
        if( nBegin + nLen > nEndPos )
            return FALSE;
    }
    return TRUE;
}

USHORT SwTxtNode::Spell(SwSpellArgs* pArgs)
{
    // Die Aehnlichkeiten zu SwTxtFrm::_AutoSpell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!

    LanguageType eFmtLang = GetSwAttrSet().GetLanguage().GetLanguage();

    BOOL bCheck = FALSE;
    BOOL bNoLang = FALSE;
    Reference<beans::XPropertySet> xProp( GetLinguPropertySet() );
    BOOL bReverse = xProp.is() ?
        *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_WRAP_REVERSE) ).getValue() : FALSE;

    xub_StrLen nBegin, nEnd;

    if ( pArgs->pStartNode != this )
        nBegin = 0;
    else
        nBegin = pArgs->rStartIdx.GetIndex();

    if ( pArgs->pEndNode != this )
        nEnd = aText.Len();
    else
        nEnd = pArgs->rEndIdx.GetIndex();

    pArgs->xSpellAlt = NULL;

    if( ( IsWrongDirty() || GetWrong() ) && aText.Len() )
    {
        if( nBegin > aText.Len() )
            nBegin = aText.Len();
        if( nEnd > aText.Len() )
            nEnd = aText.Len();
        LanguageType eActLang = GetLang( nBegin );
        SwScanner aScanner( this, GetWrong(), nBegin, nEnd, bReverse );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord( eActLang ) )
        {
            const XubString& rWord = aScanner.GetWord();

            if ( (USHORT)(eActLang=(LanguageType)GetLang(aScanner.GetBegin(),
                          rWord.Len())) == USHRT_MAX )
                eActLang=eFmtLang;

            if( eActLang == LANGUAGE_SYSTEM )
                eActLang = ::GetSystemLang();

            if( rWord.Len() > 1 )
            {
                // Sobald bCheck gesetzt ist, hat eine echte Pruefung stattgefunden,
                // solange dies nicht der Fall ist, bedeutet ein gesetztes bNoLang,
                // dass ein Wort mit [Keine] "ueberprueft" wurde. Sollte dieses Flag
                // am Ende des zu pruefenden Abschnitts immer noch gesetzt sein, wird
                // der Anwender auf seinen Fehler hingewiesen.
                if ( !bCheck )
                {
                    if ( eActLang == LANGUAGE_NONE )
                        bNoLang = TRUE;
                    else
                    {
                        bCheck = TRUE;
                        bNoLang = FALSE;
                    }
                }

                if (pArgs->xSpeller.is())
                {
                    SvxSpellWrapper::CheckSpellLang( pArgs->xSpeller, eActLang );
                    pArgs->xSpellAlt = pArgs->xSpeller->spell( rWord, eActLang,
                                            Sequence< PropertyValue >() );
                }
                if( (pArgs->xSpellAlt).is() )
                {
                    if( IsSymbol( aScanner.GetBegin() ) )
                    {
                        pArgs->xSpellAlt = NULL;
                    }
                    else
                    {
                        pArgs->pStartNode = this;
                        pArgs->pEndNode = this;
                        pArgs->rStartIdx.Assign(this, aScanner.GetEnd() );
                        pArgs->rEndIdx.Assign(this, aScanner.GetBegin() );
                    }
                }
            }
        }
    }

    return pArgs->xSpellAlt.is() ? 1 : 0;
}

SwRect SwTxtFrm::_AutoSpell( SwCntntNode* pActNode, xub_StrLen nActPos )
{
    SwRect aRect;
#ifdef DEBUG
    static BOOL bStop = FALSE;
    if ( bStop )
        return aRect;
#endif
    // Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwAutoCompleteWord& rACW = pNode->GetDoc()->GetAutoCompleteWords();

    LanguageType eFmtLang = pNode->GetSwAttrSet().GetLanguage().GetLanguage();

    const XubString& rTxt = pNode->aText;

    xub_StrLen nBegin;
    xub_StrLen nEnd;
    xub_StrLen nLen;
    xub_StrLen nInsertPos = pNode->aText.Len();
    xub_StrLen nChgStart = STRING_LEN;
    xub_StrLen nChgEnd = 0;
    xub_StrLen nInvStart = STRING_LEN;
    xub_StrLen nInvEnd = 0;

    BOOL bAddAutoCmpl = pNode->IsAutoCompleteWordDirty() &&
                        GetShell()->GetViewOptions()->IsAutoCompleteWords();
    BOOL bIncInsertPos = FALSE;

    if( pNode->GetWrong() )
    {
        if( STRING_LEN != ( nBegin = pNode->GetWrong()->GetBeginInv() ) )
        {
            nBegin = pNode->GetWrong()->GetBeginInv();
            nEnd = pNode->GetWrong()->GetEndInv();
            if ( nEnd > nInsertPos )
                nEnd = nInsertPos;
        }
        else
            nEnd = nInsertPos;
        nInsertPos = pNode->GetWrong()->GetPos( nBegin );
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
            bIncInsertPos = TRUE;
    }
    else
    {
        nBegin = 0;
        nEnd = nInsertPos;
        nInsertPos = 0;
    }

    Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );

    BOOL bFresh = nBegin < nEnd;
    BOOL bACWDirty = FALSE;

    if( nBegin < nEnd )
    {
        LanguageType eActLang = pNode->GetLang( nBegin );
        SwScanner aScanner( pNode, NULL, nBegin, nEnd, FALSE );
        while( aScanner.NextWord( eActLang ) )
        {
            if( bIncInsertPos )
            {
                bIncInsertPos = FALSE;
                if( aScanner.GetBegin() >= nBegin )
                    ++nInsertPos;
            }
            const XubString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();

            if ( (USHORT)(eActLang=(LanguageType)pNode->GetLang(nBegin, nLen))
                == USHRT_MAX )
                eActLang = eFmtLang;

            BOOL bSpell = TRUE;
            BOOL bSoft = FALSE;
            bSpell = xSpell.is() ? xSpell->hasLanguage( eActLang ) : FALSE;
            if( bSpell && rWord.Len() > 1 )
            {
                // check for: bAlter => xHyphWord.is()
                DBG_ASSERT(!bSpell || xSpell.is(), "NULL pointer");

                if( !xSpell->isValid( rWord, eActLang, Sequence< PropertyValue >() ) )
                {
                    bACWDirty = TRUE;
                    if( !pNode->GetWrong() )
                    {
                        pNode->SetWrong( new SwWrongList() );
                        pNode->GetWrong()->SetInvalid( 0, nEnd );
                    }
                    if( pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                        nBegin, nLen, nInsertPos, nActPos ) )
                        pNode->GetWrong()->Insert( nBegin, nLen, nInsertPos++ );
                    else
                    {
                        nInvStart = nBegin;
                        nInvEnd = nBegin + nLen;
                    }
                }
                else if( bAddAutoCmpl && rACW.GetMinWordLen() <= rWord.Len() )
                    rACW.InsertWord( rWord );
            }
        }
    }
    if( pNode->GetWrong() )
    {
        if( bFresh )
            pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                                      nEnd, 0, nInsertPos, nActPos );
        ViewShell *pSh = GetShell();
        if( nChgStart < nChgEnd &&
            (pSh && !GetShell()->GetViewOptions()->IsHideSpell()) )
        {
            SwNodeIndex aNdIdx( *pNode );
            SwPosition aPos( aNdIdx, SwIndex( pNode, nChgEnd ) );
            GetCharRect( aRect, aPos );
            SwRect aTmp;
            aPos = SwPosition( aNdIdx, SwIndex( pNode, nChgStart ) );
            SwCrsrMoveState aTmpState( MV_NONE );
            GetCharRect( aTmp, aPos, &aTmpState );
            BOOL bSameFrame = TRUE;
            SwTxtFrm* pStartFrm = this;
            if( HasFollow() )
            {
                while( pStartFrm->HasFollow() &&
                       nChgStart >= pStartFrm->GetFollow()->GetOfst() )
                    pStartFrm = pStartFrm->GetFollow();
                SwTxtFrm *pEndFrm = pStartFrm;
                while( pEndFrm->HasFollow() &&
                       nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
                    pEndFrm = pEndFrm->GetFollow();
                if( pEndFrm != pStartFrm )
                {
                    bSameFrame = FALSE;
                    SwRect aStFrm( pStartFrm->PaintArea() );
                    aTmp.Left( aStFrm.Left() );
                    aTmp.Right( aStFrm.Right() );
                    aTmp.Bottom( aStFrm.Bottom() );
                    aStFrm = pEndFrm->PaintArea();
                    aRect.Top( aStFrm.Top() );
                    aRect.Left( aStFrm.Left() );
                    aRect.Right( aStFrm.Right() );
                    aRect.Union( aTmp );
                    while( TRUE )
                    {
                        pStartFrm = pStartFrm->GetFollow();
                        if( pStartFrm == pEndFrm )
                            break;
                        aRect.Union( pStartFrm->PaintArea() );
                    }
                }
            }
            if( bSameFrame )
            {
                if( aTmp.Top() == aRect.Top() )
                    aRect.Left( aTmp.Left() );
                else
                {
                    SwRect aStFrm( pStartFrm->PaintArea() );
                    aRect.Left( aStFrm.Left() );
                    aRect.Right( aStFrm.Right() );
                    aRect.Top( aTmp.Top() );
                }
                if( aTmp.Height() > aRect.Height() )
                    aRect.Height( aTmp.Height() );
            }
        }
        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty( STRING_LEN != pNode->GetWrong()->GetBeginInv() );
        if( !pNode->GetWrong()->Count() )
            pNode->SetWrong( NULL );
    }
    else
        pNode->SetWrongDirty( FALSE );

    if( bAddAutoCmpl )
        pNode->SetAutoCompleteWordDirty( FALSE );
    return aRect;
}

// Wird vom CollectAutoCmplWords gerufen
void SwTxtFrm::CollectAutoCmplWrds( SwCntntNode* pActNode, xub_StrLen nActPos,
                                    BOOL bIsVisArea )
{
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    const XubString& rTxt = pNode->aText;
    SwAutoCompleteWord& rACW = pNode->GetDoc()->GetAutoCompleteWords();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->aText.Len();
    xub_StrLen nLen;
    BOOL bACWDirty = FALSE, bAnyWrd = FALSE;


    if( nBegin < nEnd )
    {
        SwScanner aScanner( pNode, NULL, nBegin, nEnd, FALSE );
        while( aScanner.NextWord( pNode->GetLang( nBegin ) ) )
        {
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();
            if( rACW.GetMinWordLen() <= nLen )
            {
                const XubString& rWord = aScanner.GetWord();

                if( nActPos < nBegin || ( nBegin + nLen ) < nActPos )
                {
// !!! ---> ggfs. das Flag bIsVisarea auswerten
                    if( rACW.GetMinWordLen() <= rWord.Len() )
                        rACW.InsertWord( rWord );
// !!! ---> ggfs. das Flag bIsVisarea auswerten
                    bAnyWrd = TRUE;
                }
                else
                    bACWDirty = TRUE;
            }
        }
    }

    if( bAnyWrd && !bACWDirty )
        pNode->SetAutoCompleteWordDirty( FALSE );
}


/*************************************************************************
 *                      SwTxtNode::Hyphenate
 *************************************************************************/
// Findet den TxtFrm und sucht dessen CalcHyph



BOOL SwTxtNode::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    // Abkuerzung: am Absatz ist keine Sprache eingestellt:
    if( LANGUAGE_NONE == USHORT( GetSwAttrSet().GetLanguage().GetLanguage() ) &&
        USHRT_MAX == GetLang( 0, aText.Len() ) )
    {
        if( !rHyphInf.IsCheck() )
            rHyphInf.SetNoLang( TRUE );
        return FALSE;
    }

    if( pLinguNode != this )
    {
        pLinguNode = this;
        pLinguFrm = (SwTxtFrm*)GetFrm( (Point*)(rHyphInf.GetCrsrPos()) );
    }
    SwTxtFrm *pFrm = pLinguFrm;
    if( pFrm )
        pFrm = pFrm->GetFrmAtOfst( rHyphInf.nStart );
    else
    {
        // 4935: Seit der Trennung ueber Sonderbereiche sind Faelle
        // moeglich, in denen kein Frame zum Node vorliegt.
        // Also kein ASSERT!
#ifdef DEBUG
        ASSERT( pFrm, "!SwTxtNode::Hyphenate: can't find any frame" );
#endif
        return FALSE;
    }

    while( pFrm )
    {
        if( pFrm->Hyphenate( rHyphInf ) )
        {
            // Das Layout ist nicht robust gegen "Direktformatierung"
            // (7821, 7662, 7408); vgl. layact.cxx,
            // SwLayAction::_TurboAction(), if( !pCnt->IsValid() ...
            pFrm->SetCompletePaint();
            return TRUE;
        }
        pFrm = (SwTxtFrm*)(pFrm->GetFollow());
        if( pFrm )
        {
            rHyphInf.nLen = rHyphInf.nLen - (pFrm->GetOfst() - rHyphInf.nStart);
            rHyphInf.nStart = pFrm->GetOfst();
        }
    }
    return FALSE;
}

#ifdef LINGU_STATISTIK

// globale Variable
SwLinguStatistik aSwLinguStat;


void SwLinguStatistik::Flush()
{
    if ( !nWords )
        return ;

#ifndef MAC
    static char *pLogName = 0;
    const BOOL bFirstOpen = pLogName ? FALSE : TRUE;
    if( bFirstOpen )
    {
        char *pPath = getenv( "TEMP" );
        char *pName = "swlingu.stk";
        if( !pPath )
            pLogName = pName;
        else
        {
            const int nLen = strlen(pPath);
            // fuer dieses new wird es kein delete geben.
            pLogName = new char[nLen + strlen(pName) + 3];
            if(nLen && (pPath[nLen-1] == '\\') || (pPath[nLen-1] == '/'))
                sprintf( pLogName, "%s%s", pPath, pName );
            else
                sprintf( pLogName, "%s/%s", pPath, pName );
        }
    }
    SvFileStream aStream( pLogName, (bFirstOpen
                                        ? STREAM_WRITE | STREAM_TRUNC
                                        : STREAM_WRITE ));

    if( !aStream.GetError() )
    {
        if ( bFirstOpen )
            aStream << "\nLinguistik-Statistik\n";
        aStream << endl << ++nFlushCnt << ". Messung\n";
        aStream << "Rechtschreibung\n";
        aStream << "gepruefte Worte: \t" << nWords << endl;
        aStream << "als fehlerhaft erkannt:\t" << nWrong << endl;
        aStream << "Alternativvorschlaege:\t" << nAlter << endl;
        if ( nWrong )
            aStream << "Durchschnitt:\t\t" << nAlter*1.0 / nWrong << endl;
        aStream << "Dauer (msec):\t\t" << nSpellTime << endl;
        aStream << "\nThesaurus\n";
        aStream << "Synonyme gesamt:\t" << nSynonym << endl;
        if ( nSynonym )
            aStream << "Synonym-Durchschnitt:\t" <<
                            nSynonym*1.0 / ( nWords - nNoSynonym ) << endl;
        aStream << "ohne Synonyme:\t\t" << nNoSynonym << endl;
        aStream << "Bedeutungen gesamt:\t" << nSynonym << endl;
        aStream << "keine Bedeutungen:\t"<< nNoSynonym << endl;
        aStream << "Dauer (msec):\t\t" << nTheTime << endl;
        aStream << "\nHyphenator\n";
        aStream << "Trennstellen gesamt:\t" << nHyphens << endl;
        if ( nHyphens )
            aStream << "Hyphen-Durchschnitt:\t" <<
                    nHyphens*1.0 / ( nWords - nNoHyph - nHyphErr ) << endl;
        aStream << "keine Trennstellen:\t" << nNoHyph << endl;
        aStream << "Trennung verweigert:\t" << nHyphErr << endl;
        aStream << "Dauer (msec):\t\t" << nHyphTime << endl;
        aStream << "---------------------------------------------\n";
    }
    nWords = nWrong = nAlter = nSynonym = nNoSynonym =
    nHyphens = nNoHyph = nHyphErr = nSpellTime = nTheTime =
    nHyphTime = 0;
    pThes = NULL;
#endif
}

#endif

// change text to Upper/Lower/Hiragana/Katagana/...
void SwTxtNode::TransliterateText( utl::TransliterationWrapper& rTrans,
        xub_StrLen nStart, xub_StrLen nEnd, SwUndoTransliterate* pUndo )
{
    if( nStart < nEnd )
    {
        SwLanguageIterator* pIter;
        if( rTrans.needLanguageForTheMode() )
            pIter = new SwLanguageIterator( *this, nStart );
        else
            pIter = 0;

        xub_StrLen nEndPos;
        sal_uInt16 nLang;
        do {
            if( pIter )
            {
                nLang = pIter->GetLanguage();
                nEndPos = pIter->GetChgPos();
                if( nEndPos > nEnd )
                    nEndPos = nEnd;
            }
            else
            {
                nLang = LANGUAGE_SYSTEM;
                nEndPos = nEnd;
            }
            xub_StrLen nLen = nEndPos - nStart;

            Sequence <long> aOffsets;
            String sChgd( rTrans.transliterate( aText, nLang, nStart, nLen,
                                                    &aOffsets ));

            ASSERT( nLen == aOffsets.getLength(),
                    "transliterate add/remove characters" );
            if( nLen == aOffsets.getLength() &&
                !aText.Equals( sChgd, nStart, nLen ) )
            {
                if( pUndo )
                    pUndo->AddChanges( *this, nStart, sChgd.Len(), aOffsets );
                ReplaceTextOnly( nStart, sChgd, aOffsets );
            }
            nStart = nEndPos;
        } while( nEndPos < nEnd && pIter && pIter->Next() );
        delete pIter;
    }
}

void SwTxtNode::ReplaceTextOnly( xub_StrLen nPos, const XubString& rText,
                                    const Sequence<long>& rOffsets )
{
    xub_StrLen nLen = (xub_StrLen)rOffsets.getLength();
    aText.Replace( nPos, nLen, rText );

    const long* pOffsets = rOffsets.getConstArray();
    // now look for no 1-1 mapping -> move the indizies!
    xub_StrLen nI, nMyOff;
    for( nI = 0, nMyOff = nPos; nI < nLen; ++nI, ++nMyOff )
    {
        xub_StrLen nOff = (xub_StrLen)pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            // something is deleted
            xub_StrLen nCnt = 1;
            while( nI + nCnt < nLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt, TRUE );
            nMyOff = nOff;
            //nMyOff -= nCnt;
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            // something is inserted
            Update( SwIndex( this, nMyOff ), nOff - nMyOff, FALSE );
            nMyOff = nOff;
        }
    }
    if( nMyOff < rText.Len() )
        // something is inserted at the end
        Update( SwIndex( this, nMyOff ), rText.Len()- nMyOff, FALSE );

    // notify the layout!
    SwDelTxt aDelHint( nPos, nLen );
    SwModify::Modify( 0, &aDelHint );

    SwInsTxt aHint( nPos, nLen );
    SwModify::Modify( 0, &aHint );
}



