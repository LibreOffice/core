/*************************************************************************
 *
 *  $RCSfile: txtedt.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 15:05:35 $
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


// So kann man die Linguistik-Statistik ( (Tmp-Path)\swlingu.stk ) aktivieren:
//#define LINGU_STATISTIK
#ifdef LINGU_STATISTIK
    #include <stdio.h>          // in SwLinguStatistik::DTOR
    #include <stdlib.h>         // getenv()
    #include <time.h>           // clock()
    #include "viewsh.hxx"       // ViewShell::GetHyphenator
    #include <tools/stream.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
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

#ifndef _DLELSTNR_HXX_
#include <dlelstnr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>    // needed for SW_MOD() macro
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
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
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
#ifndef _REDLINE_HXX
#include <redline.hxx>      // SwRedline
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>       // SwRedlineTbl
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx> // SwDrawTextInfo
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

bool lcl_IsSkippableWhiteSpace( xub_Unicode cCh )
{
    return 0x3000 == cCh ||
           ' ' == cCh ||
           '\t' == cCh ||
           0x0a == cCh;
}

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

    // We have to remember the "new" attributes, which have
    // been introduced by splitting surrounding attributes (case 4).
    // They may not be forgotten inside the "Forget" function
    std::vector< const SwTxtAttr* > aNewAttributes;

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
                        ! pSwpHints->Forget( &aNewAttributes, i, pHt->Which(),
                                             nEnd, nTmpEnd ) )
                    {
                        const SwTxtAttr* pNewAttr =
                                Insert( pHt->GetAttr(), nEnd, nTmpEnd,
                                        SETATTR_NOHINTADJUST );

                        aNewAttributes.push_back( pHt );
                        aNewAttributes.push_back( pNewAttr );

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
    if (!aText.Len())
        return aText;

    Boundary aBndry;
    const Reference< XBreakIterator > &rxBreak = pBreakIt->xBreak;
    if (rxBreak.is())
    {
        sal_Int16 nWordType = WordType::DICTIONARY_WORD;
        lang::Locale aLocale( pBreakIt->GetLocale( GetLang( nPos ) ) );
#ifdef DEBUG
        BOOL bBegin = rxBreak->isBeginWord( aText, nPos, aLocale, nWordType );
        BOOL bEnd   = rxBreak->isEndWord  ( aText, nPos, aLocale, nWordType );
#endif
        aBndry = rxBreak->getWordBoundary( aText, nPos, aLocale, nWordType, TRUE );

        // if no word was found use previous word (if any)
        if (aBndry.startPos == aBndry.endPos)
            aBndry = rxBreak->previousWord( aText, nPos, aLocale, nWordType );
    }

    // check if word was found and if it uses a symbol font, if so
    // enforce returning an empty string
    if (aBndry.endPos != aBndry.startPos && IsSymbol( (xub_StrLen)aBndry.startPos ))
        aBndry.endPos = aBndry.startPos;

    return aText.Copy( (xub_StrLen) aBndry.startPos,
                       (xub_StrLen) (aBndry.endPos - aBndry.startPos) );
}


void SwTxtNode::SetWrong( SwWrongList *pNew )
{
    delete pWrong;
    pWrong = pNew;
}

SwScanner::SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng,
                      USHORT nType, xub_StrLen nStart, xub_StrLen nEnde,
                      BOOL bRev, BOOL bOS )
    : rNode( rNd ), pWrong( pWrng ), nWordType( nType ), nLen( 0 ),
      bReverse( bRev ), bStart( TRUE ), bIsOnlineSpell( bOS )
{
    ASSERT( rNd.GetTxt().Len(), "SwScanner: EmptyString" );
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

    aCurrLang = rNd.GetLang( nBegin );
}


BOOL SwScanner::NextWord()
{
    ASSERT( ! bReverse,
            "SwScanner::NextWord() currently not implemented for reverse mode" )

    nBegin += nLen;

    // first we have to skip some whitespace characters
    const XubString& rText = rNode.GetTxt();
    while ( nBegin < rText.Len() &&
            lcl_IsSkippableWhiteSpace( rText.GetChar( nBegin ) ) )
        ++nBegin;

    if ( nBegin >= rText.Len() || nBegin >= nEndPos )
        return FALSE;

    // get next language in order to find next or previous word
    const USHORT nNextScript =
            pBreakIt->xBreak->getScriptType( rText, nBegin );
    if ( nNextScript != GetI18NScriptTypeOfLanguage( aCurrLang ) )
    {
        LanguageType aNextLang = rNode.GetLang( nBegin, nNextScript );
        aCurrLang = aNextLang;
    }

    // get the word boundaries
    Boundary aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
            pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );

    // we have to differenciate between these cases:
    if ( aBound.startPos <= nBegin )
    {
        ASSERT( aBound.endPos >= nBegin, "Unexpected aBound result" )

        // restrict boundaries to script boundaries and nEndPos
        const USHORT nCurrScript =
                pBreakIt->xBreak->getScriptType( rText, nBegin );

        XubString aTmpWord = rText.Copy( nBegin, aBound.endPos - nBegin );
        const long nScriptEnd = nBegin +
            pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
        const long nEnd = Min( aBound.endPos, nScriptEnd );

        // restrict word start to last script change position
        long nScriptBegin = 0;
        if ( aBound.startPos < nBegin )
        {
            // search from nBegin backwards until the next script change
            aTmpWord = rText.Copy( aBound.startPos, nBegin - aBound.startPos + 1 );
            nScriptBegin = aBound.startPos +
                pBreakIt->xBreak->beginOfScript( aTmpWord, nBegin - aBound.startPos,
                                                nCurrScript );
        }

        nBegin = (xub_StrLen)Max( aBound.startPos, nScriptBegin );
        nLen = (xub_StrLen)(nEnd - nBegin);
    }
    else
    {
        const USHORT nCurrScript =
                pBreakIt->xBreak->getScriptType( rText, aBound.startPos );
        XubString aTmpWord = rText.Copy( aBound.startPos, aBound.endPos - aBound.startPos );
        const long nScriptEnd = aBound.startPos +
            pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
        const long nEnd = Min( aBound.endPos, nScriptEnd );
        nBegin = (xub_StrLen)aBound.startPos;
        nLen = (xub_StrLen)(nEnd - nBegin);
    }

    if( ! nLen )
        return FALSE;

    aWord = rText.Copy( nBegin, nLen );

    return TRUE;
}


BOOL SwScanner::NextWord( LanguageType aLang )
{
    const XubString& rText = rNode.GetTxt();
    if( bReverse )
    {
        if( nBegin )
        {
            --nBegin;
            if( rNode.GetpSwpHints() )
            {
                while( CH_TXTATR_BREAKWORD == rText.GetChar( nBegin ) ||
                        CH_TXTATR_INWORD == rText.GetChar( nBegin ) )
                {
                    if( rNode.GetTxtAttr( nBegin ) )
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
        xub_StrLen nOldBegin = nBegin;
        nBegin = bReverse ? pWrong->LastWrong( nBegin )
                          : pWrong->NextWrong( nBegin );

        if( STRING_LEN == nBegin )
            return FALSE;

        // We make sure that we do not step backwards in order to avoid
        // endless loops.
        if ( ( bReverse && nBegin > nOldBegin ) ||
             ( ! bReverse && nBegin < nOldBegin ) )
             nBegin = nOldBegin;

        // if we jumped over a range marked as valid, we have to adjust
        // the word boundaries
        if ( nBegin != nOldBegin )
            bStart = TRUE;
    }

    Boundary aBound;
    if( bStart )
    {
        aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
            pBreakIt->GetLocale( aLang ), nWordType, !bReverse );
        bStart = aBound.startPos != aBound.endPos;
    }
    if( !bStart )
    {
        if( bReverse )
            aBound = pBreakIt->xBreak->previousWord( rText, nBegin,
                    pBreakIt->GetLocale( aLang ), nWordType );
        else
            aBound = pBreakIt->xBreak->nextWord( rText, nBegin,
                    pBreakIt->GetLocale( aLang ), nWordType );
    }
    else
        bStart = FALSE;

    nBegin = (xub_StrLen)aBound.startPos;
    nLen = (xub_StrLen)(aBound.endPos - nBegin);
    if( !nLen )
        return FALSE;

    // only in online spelling mode we want to consider the last word
    // surrounding nEndPos
    if( bReverse )
    {
        if( nBegin + ( bIsOnlineSpell ? nLen : 0 ) < nEndPos )
            return FALSE;
    }
    else
    {
        if( nBegin + ( bIsOnlineSpell ? 0 : nLen ) > nEndPos )
            return FALSE;
    }

    aWord = rText.Copy( nBegin, nLen );

    return TRUE;
}

USHORT SwTxtNode::Spell(SwSpellArgs* pArgs)
{
    // Die Aehnlichkeiten zu SwTxtFrm::_AutoSpell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!

    //!! please check SwTxtNode::Convert when modifying this one !!

    Reference<beans::XPropertySet> xProp( GetLinguPropertySet() );
    BOOL bReverse = xProp.is() ?
        *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_WRAP_REVERSE) ).getValue() : FALSE;

    xub_StrLen nBegin, nEnd;

    // modify string according to redline information
    const SwDoc* pDoc = GetDoc();
    const XubString aOldTxt( aText );
    const sal_Bool bShowChg = ::IsShowChanges( pDoc->GetRedlineMode() );
    if ( bShowChg )
    {
        USHORT nAct = pDoc->GetRedlinePos( *this );

        for ( ; nAct < pDoc->GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = pDoc->GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > GetIndex() )
                break;

            if( REDLINE_DELETE == pRed->GetType() )
            {
                USHORT nStart, nEnd;
                pRed->CalcStartEnd( GetIndex(), nStart, nEnd );

                while ( nStart < nEnd && nStart < aText.Len() )
                    aText.SetChar( nStart++, CH_TXTATR_INWORD );
            }
        }
    }

    if ( pArgs->pStartNode != this )
        nBegin = 0;
    else
        nBegin = pArgs->rStartIdx.GetIndex();

    if ( pArgs->pEndNode != this )
        nEnd = aText.Len();
    else
        nEnd = pArgs->rEndIdx.GetIndex();

    pArgs->xSpellAlt = NULL;

    // 4 cases:
    //
    // 1. IsWrongDirty = 0 and GetWrong = 0
    //      Everything is checked and correct
    // 2. IsWrongDirty = 0 and GetWrong = 1
    //      Everything is checked and errors are identified in the wrong list
    // 3. IsWrongDirty = 1 and GetWrong = 0
    //      Nothing has been checked
    // 4. IsWrongDirty = 1 and GetWrong = 1
    //      Text has been checked but there is an invalid range in the wrong list
    //
    // Nothing has to be done for case 1.
    if( ( IsWrongDirty() || GetWrong() ) && aText.Len() )
    {
        if( nBegin > aText.Len() )
            nBegin = aText.Len();
        if( nEnd > aText.Len() )
            nEnd = aText.Len();

        LanguageType eActLang = GetLang( nBegin );

        // In case 2. we pass the wrong list to the scanned, because only
        // the words in the wrong list have to be checked
        SwScanner aScanner( *this, IsWrongDirty() ? NULL : GetWrong(),
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd, bReverse, FALSE );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord( eActLang ) )
        {
            const XubString& rWord = aScanner.GetWord();

            // get next language for next word, consider language attributes
            // within the word
            eActLang = GetLang( aScanner.GetBegin(), rWord.Len() );

            if( rWord.Len() > 1 && LANGUAGE_NONE != eActLang )
            {
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
                        // make sure the selection build later from the
                        // data below does not include footnotes and other
                        // "in word" character to the left and right in order
                        // to preserve those. Therefore count those "in words"
                        // in order to modify the selection accordingly.
                        const sal_Unicode* pChar = rWord.GetBuffer();
                        xub_StrLen nLeft = 0;
                        while (pChar && *pChar++ == CH_TXTATR_INWORD)
                            ++nLeft;
                        pChar = rWord.Len() ? rWord.GetBuffer() + rWord.Len() - 1 : 0;
                        xub_StrLen nRight = 0;
                        while (pChar && *pChar-- == CH_TXTATR_INWORD)
                            ++nRight;

                        pArgs->pStartNode = this;
                        pArgs->pEndNode = this;
                        pArgs->rStartIdx.Assign(this, aScanner.GetEnd() - nRight );
                        pArgs->rEndIdx.Assign(this, aScanner.GetBegin() + nLeft );
                    }
                }
            }

            // get next language in order to find next or previous word
            xub_StrLen nNextBegin;
            short nInc;

            if ( bReverse )
            {
                nNextBegin = aScanner.GetBegin() ? aScanner.GetBegin() - 1 : 0;
                nInc = -1;
            }
            else
            {
                nNextBegin = aScanner.GetBegin() + rWord.Len();
                nInc = 1;
            }

            // first we have to skip some whitespace characters
            while ( ( bReverse ? nNextBegin : ( nNextBegin < aText.Len() ) ) &&
                    lcl_IsSkippableWhiteSpace( aText.GetChar( nNextBegin ) ) )
            {
                nNextBegin += nInc;
            }

            if ( nNextBegin < aText.Len() )
                eActLang = GetLang( nNextBegin );
            else
                break;
        }
    }

    // reset original text
    if ( bShowChg )
        aText = aOldTxt;

    return pArgs->xSpellAlt.is() ? 1 : 0;
}

USHORT SwTxtNode::Convert( SwConversionArgs &rArgs )
{
    //!! mofified version of SwTxtNode::Spell.          !!
    //!! please check the above when modifying this one !!

    xub_StrLen nBegin, nEnd;

    // modify string according to redline information
    const SwDoc* pDoc = GetDoc();
    const XubString aOldTxt( aText );
    const sal_Bool bShowChg = ::IsShowChanges( pDoc->GetRedlineMode() );
    if ( bShowChg )
    {
        USHORT nAct = pDoc->GetRedlinePos( *this );

        for ( ; nAct < pDoc->GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = pDoc->GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > GetIndex() )
                break;

            if( REDLINE_DELETE == pRed->GetType() )
            {
                USHORT nStart, nEnd;
                pRed->CalcStartEnd( GetIndex(), nStart, nEnd );

                while ( nStart < nEnd && nStart < aText.Len() )
                    aText.SetChar( nStart++, CH_TXTATR_INWORD );
            }
        }
    }

    if ( rArgs.pStartNode != this )
        nBegin = 0;
    else
        nBegin = rArgs.rStartIdx.GetIndex();

    if ( rArgs.pEndNode != this )
        nEnd = aText.Len();
    else
        nEnd = rArgs.rEndIdx.GetIndex();

    rArgs.bConvTextFound = sal_False;

    if(aText.Len() )
    {
        if( nBegin > aText.Len() )
            nBegin = aText.Len();
        if( nEnd > aText.Len() )
            nEnd = aText.Len();

        LanguageType eActLang = GetLang( nBegin );

        // In case 2. we pass the wrong list to the scanned, because only
        // the words in the wrong list have to be checked
        SwScanner aScanner( *this, NULL,
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd, FALSE, TRUE );
        while( !rArgs.bConvTextFound && aScanner.NextWord() )
        {
            const XubString& rWord = aScanner.GetWord();

            // get next language for next word, consider language attributes
            // within the word
            eActLang = GetLang( aScanner.GetBegin(), rWord.Len() );

            if( rWord.Len() > 0 && LANGUAGE_KOREAN == eActLang )
            {
                // clip result to provided begin and end (that may be
                // obtained from a selection) in order to restrict the
                // results to that selection
                xub_StrLen nRealBegin = aScanner.GetBegin();
                xub_StrLen nRealEnd   = aScanner.GetEnd();
                if (nRealBegin < nBegin)
                    nRealBegin = nBegin;
                if (nRealEnd > nEnd)
                    nRealEnd = nEnd;

                rArgs.bConvTextFound = sal_True;
                xub_StrLen nCpStart, nCpLen;
                nCpStart = nRealBegin - aScanner.GetBegin();
                nCpLen = nRealEnd - nRealBegin;
                rArgs.aConvText = rWord.Copy( nCpStart, nCpLen );
                rArgs.pStartNode = this;
                rArgs.pEndNode = this;
                rArgs.rStartIdx.Assign(this, nRealEnd );
                rArgs.rEndIdx.Assign(this, nRealBegin );
            }

            // get next language in order to find next or previous word
            xub_StrLen nNextBegin = aScanner.GetBegin() + rWord.Len();

            // first we have to skip some whitespace characters
            short nInc = 1;
            while ( nNextBegin < aText.Len() &&
                    lcl_IsSkippableWhiteSpace( aText.GetChar( nNextBegin ) ) )
            {
                nNextBegin += nInc;
            }

            if ( nNextBegin < aText.Len() )
                eActLang = GetLang( nNextBegin );
            else
                break;
        }
    }

    // reset original text
    if ( bShowChg )
        aText = aOldTxt;

    return rArgs.bConvTextFound ? 1 : 0;
}


SwRect SwTxtFrm::_AutoSpell( SwCntntNode* pActNode, xub_StrLen nActPos )
{
    SwRect aRect;
#if OSL_DEBUG_LEVEL > 1
    static BOOL bStop = FALSE;
    if ( bStop )
        return aRect;
#endif
    // Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    // modify string according to redline information
    USHORT nAct = pDoc->GetRedlinePos( *pNode );
    const XubString aOldTxt( pNode->aText );

    const sal_Bool bShowChg = ::IsShowChanges( pDoc->GetRedlineMode() );
    if ( bShowChg )
    {
        for ( ; nAct < pDoc->GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = pDoc->GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > pNode->GetIndex() )
                break;

            if( REDLINE_DELETE == pRed->GetType() )
            {
                USHORT nStart, nEnd;
                pRed->CalcStartEnd( pNode->GetIndex(), nStart, nEnd );

                while ( nStart < nEnd && nStart < pNode->aText.Len() )
                    pNode->aText.SetChar( nStart++, CH_TXTATR_INWORD );
            }
        }
    }

    // a change of data indicates that at least one word has been modified
    sal_Bool bRedlineChg = ( pNode->aText.GetBuffer() != aOldTxt.GetBuffer() );

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

        // get word around nBegin, we start at nBegin - 1
        if ( STRING_LEN != nBegin )
        {
            if ( nBegin )
                --nBegin;

            LanguageType eActLang = pNode->GetLang( nBegin );
            Boundary aBound = pBreakIt->xBreak->getWordBoundary( pNode->aText, nBegin,
                            pBreakIt->GetLocale( eActLang ), WordType::DICTIONARY_WORD, TRUE );
            nBegin = xub_StrLen(aBound.startPos);
        }

        // get the position in the wrong list
        nInsertPos = pNode->GetWrong()->GetPos( nBegin );

        // sometimes we have to skip one entry
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
                nInsertPos++;
    }
    else
    {
        nBegin = 0;
        nEnd = nInsertPos;
        nInsertPos = 0;
    }

    BOOL bFresh = nBegin < nEnd;
    BOOL bACWDirty = FALSE;

    if( nBegin < nEnd )
    {
        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );

        LanguageType eActLang = pNode->GetLang( nBegin );

        SwScanner aScanner( *pNode, NULL, WordType::DICTIONARY_WORD,
                            nBegin, nEnd, FALSE, TRUE );
        while( aScanner.NextWord( eActLang ) )
        {
            const XubString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();

            // get next language for next word, consider language attributes
            // within the word
            eActLang = pNode->GetLang( aScanner.GetBegin(), rWord.Len() );

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
                {
                    if ( bRedlineChg )
                    {
                        XubString rNewWord( rWord );
                        rACW.InsertWord( rNewWord, *pDoc );
                    }
                    else
                        rACW.InsertWord( rWord, *pDoc );
                }
            }

            // get next language in order to find next word
            xub_StrLen nNextBegin = aScanner.GetBegin() + rWord.Len();
            // first we have to skip some whitespace characters
            while ( nNextBegin < pNode->aText.Len() &&
                    lcl_IsSkippableWhiteSpace( pNode->aText.GetChar( nNextBegin ) ) )
                nNextBegin++;

            if ( nNextBegin < pNode->aText.Len() )
                eActLang = pNode->GetLang( nNextBegin );
            else
                break;
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
            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.b2Lines = sal_True;
            GetCharRect( aRect, aPos, &aTmpState );
            // information about end of repaint area
            Sw2LinesPos* pEnd2Pos = aTmpState.p2Lines;

            SwTxtFrm* pStartFrm = this;

            while( pStartFrm->HasFollow() &&
                   nChgStart >= pStartFrm->GetFollow()->GetOfst() )
                pStartFrm = pStartFrm->GetFollow();

            SwTxtFrm *pEndFrm = pStartFrm;

            while( pEndFrm->HasFollow() &&
                   nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
                pEndFrm = pEndFrm->GetFollow();

            if ( pEnd2Pos )
            {
                // we are inside a special portion, take left border
                SWRECTFN( pEndFrm )
                (aRect.*fnRect->fnSetTop)( (pEnd2Pos->aLine.*fnRect->fnGetTop)() );
                if ( pEndFrm->IsRightToLeft() )
                    (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetLeft)() );
                else
                    (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetRight)() );
                (aRect.*fnRect->fnSetWidth)( 1 );
                (aRect.*fnRect->fnSetHeight)( (pEnd2Pos->aLine.*fnRect->fnGetHeight)() );
                delete pEnd2Pos;
            }

            aTmpState.p2Lines = NULL;
            SwRect aTmp;
            aPos = SwPosition( aNdIdx, SwIndex( pNode, nChgStart ) );
            GetCharRect( aTmp, aPos, &aTmpState );
            // information about start of repaint area
            Sw2LinesPos* pSt2Pos = aTmpState.p2Lines;
            if ( pSt2Pos )
            {
                // we are inside a special portion, take right border
                SWRECTFN( pStartFrm )
                (aTmp.*fnRect->fnSetTop)( (pSt2Pos->aLine.*fnRect->fnGetTop)() );
                if ( pStartFrm->IsRightToLeft() )
                    (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
                else
                    (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
                (aTmp.*fnRect->fnSetWidth)( 1 );
                (aTmp.*fnRect->fnSetHeight)( (pSt2Pos->aLine.*fnRect->fnGetHeight)() );
                delete pSt2Pos;
            }

            BOOL bSameFrame = TRUE;

            if( HasFollow() )
            {
                if( pEndFrm != pStartFrm )
                {
                    bSameFrame = FALSE;
                    SwRect aStFrm( pStartFrm->PaintArea() );
                    {
                        SWRECTFN( pStartFrm )
                        (aTmp.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                        (aTmp.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
                        (aTmp.*fnRect->fnSetBottom)( (aStFrm.*fnRect->fnGetBottom)() );
                    }
                    aStFrm = pEndFrm->PaintArea();
                    {
                        SWRECTFN( pEndFrm )
                        (aRect.*fnRect->fnSetTop)( (aStFrm.*fnRect->fnGetTop)() );
                        (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                        (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
                    }
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
                SWRECTFN( pStartFrm )
                if( (aTmp.*fnRect->fnGetTop)() == (aRect.*fnRect->fnGetTop)() )
                    (aRect.*fnRect->fnSetLeft)( (aTmp.*fnRect->fnGetLeft)() );
                else
                {
                    SwRect aStFrm( pStartFrm->PaintArea() );
                    (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                    (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
                    (aRect.*fnRect->fnSetTop)( (aTmp.*fnRect->fnGetTop)() );
                }

                if( aTmp.Height() > aRect.Height() )
                    aRect.Height( aTmp.Height() );
            }
        }
        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty( STRING_LEN != pNode->GetWrong()->GetBeginInv() );
        if( !pNode->GetWrong()->Count() && ! pNode->IsWrongDirty() )
            pNode->SetWrong( NULL );
    }
    else
        pNode->SetWrongDirty( FALSE );

    // reset original text
    if ( bShowChg )
        pNode->aText = aOldTxt;

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
    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->aText.Len();
    xub_StrLen nLen;
    BOOL bACWDirty = FALSE, bAnyWrd = FALSE;


    if( nBegin < nEnd )
    {
        USHORT nCnt = 200;
        SwScanner aScanner( *pNode, NULL, WordType::DICTIONARY_WORD,
                            nBegin, nEnd, FALSE, FALSE );
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
                        rACW.InsertWord( rWord, *pDoc );
// !!! ---> ggfs. das Flag bIsVisarea auswerten
                    bAnyWrd = TRUE;
                }
                else
                    bACWDirty = TRUE;
            }
            if( !--nCnt )
            {
                if ( Application::AnyInput( INPUT_ANY ) )
                    return;
                nCnt = 100;
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
#if OSL_DEBUG_LEVEL > 1
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
                snprintf( pLogName, sizeof(pLogName), "%s%s", pPath, pName );
            else
                snprintf( pLogName, sizeof(pLogName), "%s/%s", pPath, pName );
        }
    }
    SvFileStream aStream( String::CreateFromAscii(pLogName), (bFirstOpen
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
    //pThes = NULL;
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
            if( !aText.Equals( sChgd, nStart, nLen ) )
            {
                if( pUndo )
                    pUndo->AddChanges( *this, nStart, nLen, aOffsets );
                ReplaceTextOnly( nStart, nLen, sChgd, aOffsets );
            }
            nStart = nEndPos;
        } while( nEndPos < nEnd && pIter && pIter->Next() );
        delete pIter;
    }
}

void SwTxtNode::ReplaceTextOnly( xub_StrLen nPos, xub_StrLen nLen,
                                const XubString& rText,
                                const Sequence<long>& rOffsets )
{
    aText.Replace( nPos, nLen, rText );

    xub_StrLen nTLen = rText.Len();
    const long* pOffsets = rOffsets.getConstArray();
    // now look for no 1-1 mapping -> move the indizies!
    xub_StrLen nI, nMyOff;
    for( nI = 0, nMyOff = nPos; nI < nTLen; ++nI, ++nMyOff )
    {
        xub_StrLen nOff = (xub_StrLen)pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            // something is inserted
            xub_StrLen nCnt = 1;
            while( nI + nCnt < nTLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt, FALSE );
            nMyOff = nOff;
            //nMyOff -= nCnt;
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            // something is deleted
            Update( SwIndex( this, nMyOff+1 ), nOff - nMyOff, TRUE );
            nMyOff = nOff;
        }
    }
    if( nMyOff < nLen )
        // something is deleted at the end
        Update( SwIndex( this, nMyOff ), nLen - nMyOff, TRUE );

    // notify the layout!
    SwDelTxt aDelHint( nPos, nTLen );
    SwModify::Modify( 0, &aDelHint );

    SwInsTxt aHint( nPos, nTLen );
    SwModify::Modify( 0, &aHint );
}
