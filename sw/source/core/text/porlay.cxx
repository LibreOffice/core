/*************************************************************************
 *
 *  $RCSfile: porlay.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-29 08:09:49 $
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

#include "errhdl.hxx"   // ASSERT

#include "txtcfg.hxx"
#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "porexp.hxx"   // SwQuoVadisPortion
#include "blink.hxx"    // pBlink
#include "redlnitr.hxx" // SwRedlineItr
#include "porfly.hxx"   // SwFlyCntPortion
#ifndef _PORRST_HXX
#include <porrst.hxx>       // SwHangingPortion
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifdef BIDI
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SVX_CHARHIDDENITEM_HXX
#include <svx/charhiddenitem.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SVX_BLNKITEM_HXX
#include <svx/blnkitem.hxx>
#endif
#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>       // SwRedlineTbl
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>      // SwRedline
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

#ifdef BIDI
#include <unicode/ubidi.h>

/*************************************************************************
 *                 lcl_IsLigature
 *
 * Checks if cCh + cNectCh builds a ligature (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_IsLigature( xub_Unicode cCh, xub_Unicode cNextCh )
{
            // Lam + Alef
    return ( 0x644 == cCh && 0x627 == cNextCh ) ||
            // Beh + Reh
           ( 0x628 == cCh && 0x631 == cNextCh );
}

/*************************************************************************
 *                 lcl_ConnectToPrev
 *
 * Checks if cCh is connectable to cPrevCh (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_ConnectToPrev( xub_Unicode cCh, xub_Unicode cPrevCh )
{
    // Alef, Dal, Thal, Reh, Zain, and Waw do not connect to the left
    // Uh, there seem to be some more characters that are not connectable
    // to the left. So we look for the characters that are actually connectable
    // to the left. Here is the complete list of WH:
    sal_Bool bRet = 0x628 == cPrevCh ||
                    ( 0x62A <= cPrevCh && cPrevCh <= 0x62E ) ||
                    ( 0x633 <= cPrevCh && cPrevCh <= 0x643 ) ||
                    ( 0x645 <= cPrevCh && cPrevCh <= 0x647 ) ||
                    0x64A == cPrevCh ||
                    ( 0x678 <= cPrevCh && cPrevCh <= 0x687 ) ||
                    ( 0x69A <= cPrevCh && cPrevCh <= 0x6B4 ) ||
                    ( 0x6B9 <= cPrevCh && cPrevCh <= 0x6C0 ) ||
                    ( 0x6C3 <= cPrevCh && cPrevCh <= 0x6D3 );

    // check for ligatures cPrevChar + cChar
    if ( bRet )
        bRet = ! lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}

#endif


/*************************************************************************
 *                 SwLineLayout::~SwLineLayout()
 *
 * class SwLineLayout: Das Layout einer einzelnen Zeile. Dazu
 * gehoeren vor allen Dingen die Dimension, die Anzahl der
 * Character und der Wortzwischenraeume in der Zeile.
 * Zeilenobjekte werden in einem eigenen Pool verwaltet, um zu
 * erreichen, dass sie im Speicher moeglichst beeinander liegen
 * (d.h. zusammen gepaged werden und den Speicher nicht
 * fragmentieren).
 *************************************************************************/

SwLineLayout::~SwLineLayout()
{
    Truncate();
    if( GetNext() )
        delete GetNext();
    if( pBlink )
        pBlink->Delete( this );
    delete pSpaceAdd;
    if ( pKanaComp )
        delete pKanaComp;
}

/*************************************************************************
 *                virtual SwLineLayout::Insert()
 *************************************************************************/

SwLinePortion *SwLineLayout::Insert( SwLinePortion *pIns )
{
    // Erster Attributwechsel, Masse und Laengen
    // aus *pCurr in die erste Textportion kopieren.
    if( !pPortion )
    {
        if( GetLen() )
        {
            pPortion = new SwTxtPortion( *(SwLinePortion*)this );
            if( IsBlinking() && pBlink )
            {
                SetBlinking( sal_False );
                pBlink->Replace( this, pPortion );
            }
        }
        else
        {
            SetPortion( pIns );
            return pIns;
        }
    }
    // mit Skope aufrufen, sonst Rekursion !
    return pPortion->SwLinePortion::Insert( pIns );
}

/*************************************************************************
 *                virtual SwLineLayout::Append()
 *************************************************************************/

SwLinePortion *SwLineLayout::Append( SwLinePortion *pIns )
{
    // Erster Attributwechsel, Masse und Laengen
    // aus *pCurr in die erste Textportion kopieren.
    if( !pPortion )
        pPortion = new SwTxtPortion( *(SwLinePortion*)this );
    // mit Skope aufrufen, sonst Rekursion !
    return pPortion->SwLinePortion::Append( pIns );
}

/*************************************************************************
 *                virtual SwLineLayout::Format()
 *************************************************************************/

// fuer die Sonderbehandlung bei leeren Zeilen

sal_Bool SwLineLayout::Format( SwTxtFormatInfo &rInf )
{
    if( GetLen() )
        return SwTxtPortion::Format( rInf );
    else
    {
        Height( rInf.GetTxtHeight() );
        return sal_True;
    }
}

/*************************************************************************
 *                    SwLineLayout::CalcLeftMargin()
 *
 * Wir sammeln alle FlyPortions am Anfang der Zeile zu einer MarginPortion.
 *************************************************************************/

SwMarginPortion *SwLineLayout::CalcLeftMargin()
{
    SwMarginPortion *pLeft = (GetPortion() && GetPortion()->IsMarginPortion()) ?
        (SwMarginPortion *)GetPortion() : 0;
    if( !GetPortion() )
         SetPortion( new SwTxtPortion( *(SwLinePortion*)this ) );
    if( !pLeft )
    {
        pLeft = new SwMarginPortion( 0 );
        pLeft->SetPortion( GetPortion() );
        SetPortion( pLeft );
    }
    else
    {
        pLeft->Height( 0 );
        pLeft->Width( 0 );
        pLeft->SetLen( 0 );
        pLeft->SetAscent( 0 );
        pLeft->SetPortion( NULL );
        pLeft->SetFixWidth(0);
    }

    SwLinePortion *pPos = pLeft->GetPortion();
    while( pPos )
    {
        DBG_LOOP;
        if( pPos->IsFlyPortion() )
        {
            // Die FlyPortion wird ausgesogen ...
            pLeft->Join( (SwGluePortion*)pPos );
            pPos = pLeft->GetPortion();
            if( GetpKanaComp() )
                GetKanaComp().Remove( 0, 1 );
        }
        else
            pPos = 0;
    }
    return pLeft;
}

/*************************************************************************
 *                    SwLineLayout::CreateSpaceAdd()
 *************************************************************************/

void SwLineLayout::CreateSpaceAdd( const short nInit )
{
    pSpaceAdd = new SvShorts;
    pSpaceAdd->Insert( nInit, 0 );
}

/*************************************************************************
 *                    SwLineLayout::CalcLine()
 *
 * Aus FormatLine() ausgelagert.
 *************************************************************************/

void SwLineLayout::CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    const KSHORT nLineWidth = rInf.RealWidth();

    KSHORT nFlyAscent;
    KSHORT nFlyHeight;
    KSHORT nFlyDescent;
    sal_Bool bOnlyPostIts = sal_True;
    SetHanging( sal_False );

    sal_Bool bTmpDummy = ( 0 == GetLen() );
    SwFlyCntPortion* pFlyCnt = 0;
    if( bTmpDummy )
    {
        nFlyAscent = 0;
        nFlyHeight = 0;
        nFlyDescent = 0;
    }

    if( pPortion )
    {
        SetCntnt( sal_False );
        if( pPortion->IsBreakPortion() )
        {
            SetLen( pPortion->GetLen() );
            if( GetLen() )
                bTmpDummy = sal_False;
        }
        else
        {
            Init( GetPortion() );
            SwLinePortion *pPos = pPortion;
            SwLinePortion *pLast = this;
            KSHORT nMaxDescent = 0;

            //  Eine Gruppe ist ein Abschnitt in der Portion-Kette von
            //  pCurr oder einer Fix-Portion bis zum Ende bzw. zur naechsten
            //  Fix-Portion.
            while( pPos )
            {
                DBG_LOOP;
                ASSERT( POR_LIN != pPos->GetWhichPor(),
                        "SwLineLayout::CalcLine: don't use SwLinePortions !" );
                // Null-Portions werden eliminiert. Sie koennen entstehen,
                // wenn zwei FlyFrms ueberlappen.
                if( !pPos->Compress() )
                {
                    // 8110: Hoehe und Ascent nur uebernehmen, wenn sonst in der
                    // Zeile nichts mehr los ist.
                    if( !pPos->GetPortion() )
                    {
                        if( !Height() )
                            Height( pPos->Height() );
                        if( !GetAscent() )
                            SetAscent( pPos->GetAscent() );
                    }
                    delete pLast->Cut( pPos );
                    pPos = pLast->GetPortion();
                    continue;
                }

                // Es gab Attributwechsel: Laengen und Masse aufaddieren;
                // bzw.Maxima bilden.

                nLineLength += pPos->GetLen();

                KSHORT nPosHeight = pPos->Height();
                KSHORT nPosAscent = pPos->GetAscent();
                AddPrtWidth( pPos->Width() );

                ASSERT( nPosHeight >= nPosAscent,
                        "SwLineLayout::CalcLine: bad ascent or height" );
                if( pPos->IsHangingPortion() )
                {
                    SetHanging( sal_True );
                    rInf.GetParaPortion()->SetMargin( sal_True );
                }

                // Damit ein Paragraphende-Zeichen nicht durch ein Descent zu einer
                // geaenderten Zeilenhoehe und zum Umformatieren fuehrt.
                if ( !pPos->IsBreakPortion() || !Height() )
                {
                    bOnlyPostIts &= pPos->IsPostItsPortion();
                    if( bTmpDummy && !nLineLength )
                    {
                        if( pPos->IsFlyPortion() )
                        {
                            if( nFlyHeight < nPosHeight )
                                nFlyHeight = nPosHeight;
                            if( nFlyAscent < nPosAscent )
                                nFlyAscent = nPosAscent;
                            if( nFlyDescent < nPosHeight - nPosAscent )
                                nFlyDescent = nPosHeight - nPosAscent;
                        }
                        else
                        {
                            if( pPos->InNumberGrp() )
                            {
                                KSHORT nTmp = rInf.GetFont()->GetAscent(
                                                rInf.GetVsh(), *rInf.GetOut() );
                                if( nTmp > nPosAscent )
                                {
                                    nPosHeight += nTmp - nPosAscent;
                                    nPosAscent = nTmp;
                                }
                                nTmp = rInf.GetFont()->GetHeight( rInf.GetVsh(),
                                                                 *rInf.GetOut() );
                                if( nTmp > nPosHeight )
                                    nPosHeight = nTmp;
                            }
                            Height( nPosHeight );
                            nAscent = nPosAscent;
                            nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                    else if( !pPos->IsFlyPortion() )
                    {
                        if( Height() < nPosHeight )
                            Height( nPosHeight );
                        if( pPos->IsFlyCntPortion() || ( pPos->IsMultiPortion()
                            && ((SwMultiPortion*)pPos)->HasFlyInCntnt() ) )
                            rLine.SetFlyInCntBase();
                        if( pPos->IsFlyCntPortion() &&
                            ((SwFlyCntPortion*)pPos)->GetAlign() )
                        {
                            ((SwFlyCntPortion*)pPos)->SetMax( sal_False );
                            if( !pFlyCnt || pPos->Height() > pFlyCnt->Height() )
                                pFlyCnt = (SwFlyCntPortion*)pPos;
                        }
                        else
                        {
                            if( nAscent < nPosAscent )
                                nAscent = nPosAscent;
                            if( nMaxDescent < nPosHeight - nPosAscent )
                                nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                }
                else if( pPos->GetLen() )
                    bTmpDummy = sal_False;
                if( !HasCntnt() && !pPos->InNumberGrp() )
                {
                    if ( pPos->InExpGrp() )
                    {
                        XubString aTxt;
                        if( pPos->GetExpTxt( rInf, aTxt ) && aTxt.Len() )
                            SetCntnt( sal_True );
                    }
                    else if( ( pPos->InTxtGrp() || pPos->IsMultiPortion() ) &&
                             pPos->GetLen() )
                        SetCntnt( sal_True );
                }
                bTmpDummy = bTmpDummy && !HasCntnt() &&
                            ( !pPos->Width() || pPos->IsFlyPortion() );

                pLast = pPos;
                pPos = pPos->GetPortion();
            }
            if( pFlyCnt )
            {
                if( pFlyCnt->Height() == Height() )
                {
                    pFlyCnt->SetMax( sal_True );
                    if( Height() > nMaxDescent + nAscent )
                    {
                        if( 3 == pFlyCnt->GetAlign() ) // Bottom
                            nAscent = Height() - nMaxDescent;
                        else if( 2 == pFlyCnt->GetAlign() ) // Center
                            nAscent = ( Height() + nAscent - nMaxDescent ) / 2;
                    }
                    pFlyCnt->SetAscent( nAscent );
                }
            }
            if( bTmpDummy && nFlyHeight )
            {
                nAscent = nFlyAscent;
                if( nFlyDescent > nFlyHeight - nFlyAscent )
                    Height( nFlyHeight + nFlyDescent );
                else
                    Height( nFlyHeight );
            }
            else if( nMaxDescent > Height() - nAscent )
                Height( nMaxDescent + nAscent );
            if( bOnlyPostIts )
            {
                Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), *rInf.GetOut() ) );
                nAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), *rInf.GetOut() );
            }
        }
    }
    else
        SetCntnt( !bTmpDummy );
    // Robust:
    if( nLineWidth < Width() )
        Width( nLineWidth );
    ASSERT( nLineWidth >= Width(), "SwLineLayout::CalcLine: line is bursting" );
    SetDummy( bTmpDummy );
    SetRedline( rLine.GetRedln() &&
        rLine.GetRedln()->CheckLine( rLine.GetStart(), rLine.GetEnd() ) );
}

void SwLineLayout::MaxAscentDescent( SwTwips& _orAscent,
                                     SwTwips& _orDescent,
                                     SwTwips& _orObjAscent,
                                     SwTwips& _orObjDescent,
                                     const SwLinePortion* _pDontConsiderPortion ) const
{
    _orAscent = 0;
    _orDescent = 0;
    _orObjAscent = 0;
    _orObjDescent = 0;

    const SwLinePortion* pPortion = this;
    if ( !pPortion->GetLen() && pPortion->GetPortion() )
    {
        pPortion = pPortion->GetPortion();
    }

    while ( pPortion )
    {
        if ( !pPortion->IsBreakPortion() && !pPortion->IsFlyPortion() )
        {
            SwTwips nPortionAsc = static_cast<SwTwips>(pPortion->GetAscent());
            SwTwips nPortionDesc = static_cast<SwTwips>(pPortion->Height()) -
                                   nPortionAsc;

            sal_Bool bFlyCmp = pPortion->IsFlyCntPortion()
                               ? static_cast<const SwFlyCntPortion*>(pPortion)->IsMax()
                               : ( pPortion != _pDontConsiderPortion );
            if ( bFlyCmp )
            {
                _orObjAscent = Max( _orObjAscent, nPortionAsc );
                _orObjDescent = Max( _orObjDescent, nPortionDesc );
            }

            if ( !pPortion->IsFlyCntPortion() && !pPortion->IsGrfNumPortion() )
            {
                _orAscent = Max( _orAscent, nPortionAsc );
                _orDescent = Max( _orDescent, nPortionDesc );
            }
        }
        pPortion = pPortion->GetPortion();
    }
}

/*************************************************************************
 *                      class SwCharRange
 *************************************************************************/

SwCharRange &SwCharRange::operator+=(const SwCharRange &rRange)
{
    if(0 != rRange.nLen ) {
        if(0 == nLen) {
            nStart = rRange.nStart;
            nLen = rRange.nLen ;
        }
        else {
            if(rRange.nStart + rRange.nLen > nStart + nLen) {
                nLen = rRange.nStart + rRange.nLen - nStart;
            }
            if(rRange.nStart < nStart) {
                nLen += nStart - rRange.nStart;
                nStart = rRange.nStart;
            }
        }
    }
    return *this;
}

/*************************************************************************
 *                      WhichFont()
 *
 * Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
 * Sw Script Types (SW_LATIN, SW_CJK, SW_CTL), used to identify the font
 *************************************************************************/

BYTE SwScriptInfo::WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI )
{
    ASSERT( pTxt || pSI,"How should I determine the script type?" );
    USHORT nScript;

    // First we try to use our SwScriptInfo
    if ( pSI )
        nScript = pSI->ScriptType( nIdx );
    else
        // Ok, we have to ask the break iterator
        nScript = pBreakIt->GetRealScriptOfText( *pTxt, nIdx );

    switch ( nScript ) {
        case i18n::ScriptType::LATIN : return SW_LATIN;
        case i18n::ScriptType::ASIAN : return SW_CJK;
        case i18n::ScriptType::COMPLEX : return SW_CTL;
    }

    ASSERT( sal_False, "Somebody tells lies about the script type!" );
    return SW_LATIN;
}

/*************************************************************************
 *                      SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode )
{
    InitScriptInfo( rNode, nDefaultDir == UBIDI_RTL );
}

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL )
{
    if( !pBreakIt->xBreak.is() )
        return;

    const String& rTxt = rNode.GetTxt();

    //
    // HIDDEN TEXT INFORMATION
    //
    Range aRange( 0, rTxt.Len() ? rTxt.Len() - 1 : 0 );
    MultiSelection aHiddenMulti( aRange );
    CalcHiddenRanges( rNode, aHiddenMulti );

    aHiddenChg.Remove( 0, aHiddenChg.Count() );
    USHORT nHiddenIdx = 0;
    USHORT i = 0;
    for( i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
    {
        const Range& rRange = aHiddenMulti.GetRange( i );
        const xub_StrLen nStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nEnd = (xub_StrLen)rRange.Max() + 1;

        aHiddenChg.Insert( nStart, nHiddenIdx++ );
        aHiddenChg.Insert( nEnd, nHiddenIdx++ );
    }

    //
    // SCRIPT AND SCRIPT RELATED INFORMATION
    //

    xub_StrLen nChg = nInvalidityPos;

    // STRING_LEN means the data structure is up to date
    nInvalidityPos = STRING_LEN;

    // this is the default direction
    nDefaultDir = bRTL ? UBIDI_RTL : UBIDI_LTR;

    // counter for script info arrays
    USHORT nCnt = 0;
    // counter for compression information arrays
    USHORT nCntComp = 0;
    // counter for kashida array
    USHORT nCntKash = 0;

    BYTE nScript;

    // compression type
    const SwCharCompressType aCompEnum = rNode.GetDoc()->GetCharCompressType();

    // justification type
    const sal_Bool bAdjustBlock = SVX_ADJUST_BLOCK ==
                                  rNode.GetSwAttrSet().GetAdjust().GetAdjust();

    //
    // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:
    //

    if( nChg )
    {
        // if change position = 0 we do not use any data from the arrays
        // because by deleting all characters of the first group at the beginning
        // of a paragraph nScript is set to a wrong value
        ASSERT( CountScriptChg(), "Where're my changes of script?" );
        while( nCnt < CountScriptChg() )
        {
            if ( nChg > GetScriptChg( nCnt ) )
                nCnt++;
            else
            {
                nScript = GetScriptType( nCnt );
                break;
            }
        }
        if( CHARCOMPRESS_NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg > GetCompStart( nCntComp ) )
                    nCntComp++;
                else
                    break;
            }
        }
        if ( bAdjustBlock )
        {
            while( nCntKash < CountKashida() )
            {
                if ( nChg > GetKashida( nCntKash ) )
                    nCntKash++;
                else
                    break;
            }
        }
    }

    //
    // ADJUST nChg VALUE:
    //

    // by stepping back one position we know that we are inside a group
    // declared as an nScript group
    if ( nChg )
        --nChg;

    const xub_StrLen nGrpStart = nCnt ? GetScriptChg( nCnt - 1 ) : 0;

    // we go back in our group until we reach the first character of
    // type nScript
    while ( nChg > nGrpStart &&
            nScript != pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
        --nChg;

    // If we are at the start of a group, we do not trust nScript,
    // we better get nScript from the breakiterator:
    if ( nChg == nGrpStart )
        nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg );

    //
    // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:
    //

    // remove invalid entries from script information arrays
    const USHORT nScriptRemove = aScriptChg.Count() - nCnt;
    aScriptChg.Remove( nCnt, nScriptRemove );
    aScriptType.Remove( nCnt, nScriptRemove );

    // get the start of the last compression group
    USHORT nLastCompression = nChg;
    if( nCntComp )
    {
        --nCntComp;
        nLastCompression = GetCompStart( nCntComp );
        if( nChg >= nLastCompression + GetCompLen( nCntComp ) )
        {
            nLastCompression = nChg;
            ++nCntComp;
        }
    }

    // remove invalid entries from compression information arrays
    const USHORT nCompRemove = aCompChg.Count() - nCntComp;
    aCompChg.Remove( nCntComp, nCompRemove );
    aCompLen.Remove( nCntComp, nCompRemove );
    aCompType.Remove( nCntComp, nCompRemove );

    // get the start of the last kashida group
    USHORT nLastKashida = nChg;
    if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
    {
        --nCntKash;
        nLastKashida = GetKashida( nCntKash );
    }

    // remove invalid entries from kashida array
    aKashida.Remove( nCntKash, aKashida.Count() - nCntKash );

    //
    // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
    // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH
    //

    if( WEAK == pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
    {
        // If the beginning of the current group is weak, this means that
        // all of the characters in this grounp are weak. We have to assign
        // the scripts to these characters depending on the fonts which are
        // set for these characters to display them.
        xub_StrLen nEnd =
                (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, WEAK );

        if( nEnd > rTxt.Len() )
            nEnd = rTxt.Len();

        nScript = (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );

        ASSERT( i18n::ScriptType::LATIN == nScript ||
                i18n::ScriptType::ASIAN == nScript ||
                i18n::ScriptType::COMPLEX == nScript, "Wrong default language" );

        nChg = nEnd;

        // Get next script type or set to weak in order to exit
        BYTE nNextScript = ( nEnd < rTxt.Len() ) ?
           (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nEnd ) :
           (BYTE)WEAK;

        if ( nScript != nNextScript )
        {
            aScriptChg.Insert( nEnd, nCnt );
            aScriptType.Insert( nScript, nCnt++ );
            nScript = nNextScript;
        }
    }

    //
    // UPDATE THE SCRIPT INFO ARRAYS:
    //

    while ( nChg < rTxt.Len() || ( !aScriptChg.Count() && !rTxt.Len() ) )
    {
        ASSERT( i18n::ScriptType::WEAK != nScript,
                "Inserting WEAK into SwScriptInfo structure" );
        ASSERT( STRING_LEN != nChg, "65K? Strange length of script section" );

        nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );

        if ( nChg > rTxt.Len() )
            nChg = rTxt.Len();

        aScriptChg.Insert( nChg, nCnt );
        aScriptType.Insert( nScript, nCnt++ );

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CHARCOMPRESS_NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            BYTE ePrevState = NONE;
            BYTE eState;
            USHORT nPrevChg = nLastCompression;

            while ( nLastCompression < nChg )
            {
                xub_Unicode cChar = rTxt.GetChar( nLastCompression );

                // examine current character
                switch ( cChar )
                {
                // Left punctuation found
                case 0x3008: case 0x300A: case 0x300C: case 0x300E:
                case 0x3010: case 0x3014: case 0x3016: case 0x3018:
                case 0x301A: case 0x301D:
                    eState = SPECIAL_LEFT;
                    break;
                // Right punctuation found
                case 0x3001: case 0x3002: case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                    eState = SPECIAL_RIGHT;
                    break;
                default:
                    eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ?
                               KANA :
                               NONE;
                }

                // insert range of compressable characters
                if( ePrevState != eState )
                {
                    if ( ePrevState != NONE )
                    {
                        // insert start and type
                        if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                             ePrevState != KANA )
                        {
                            aCompChg.Insert( nPrevChg, nCntComp );
                            BYTE nTmpType = ePrevState;
                            aCompType.Insert( nTmpType, nCntComp );
                            aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                        }
                    }

                    ePrevState = eState;
                    nPrevChg = nLastCompression;
                }

                nLastCompression++;
            }

            // we still have to examine last entry
            if ( ePrevState != NONE )
            {
                // insert start and type
                if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                     ePrevState != KANA )
                {
                    aCompChg.Insert( nPrevChg, nCntComp );
                    BYTE nTmpType = ePrevState;
                    aCompType.Insert( nTmpType, nCntComp );
                    aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                }
            }
        }

        // we search for connecting opportunities (kashida)
        else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
        {
            SwScanner aScanner( rNode, NULL,
                                ::com::sun::star::i18n::WordType::DICTIONARY_WORD,
                                nLastKashida, nChg, sal_False, sal_False );

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const XubString& rWord = aScanner.GetWord();

                xub_StrLen nIdx = 0;
                xub_StrLen nKashidaPos = STRING_LEN;
                xub_Unicode cCh;
                xub_Unicode cPrevCh = 0;

                while ( nIdx < rWord.Len() )
                {
                    cCh = rWord.GetChar( nIdx );

                    // 1. Priority:
                    // after user inserted kashida
                    if ( 0x640 == cCh )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        break;
                    }

                    // 2. Priority:
                    // after a Seen or Sad
                    if ( nIdx + 1 < rWord.Len() &&
                         ( 0x633 == cCh || 0x635 == cCh ) )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        break;
                    }

                    // 3. Priority:
                    // before final form of Teh Marbuta, Hah, Dal
                    // 4. Priority:
                    // before final form of Alef, Lam or Kaf
                    if ( nIdx && nIdx + 1 == rWord.Len() &&
                         ( 0x629 == cCh || 0x62D == cCh || 0x62F == cCh ||
                           0x627 == cCh || 0x644 == cCh || 0x643 == cCh ) )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if character is connectable to previous character,
                        if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                        {
                            nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                            break;
                        }
                    }

                    // 5. Priority:
                    // before media Bah
                    if ( nIdx && nIdx + 1 < rWord.Len() && 0x628 == cCh )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if next character is Reh, Yeh or Alef Maksura
                        xub_Unicode cNextCh = rWord.GetChar( nIdx + 1 );

                        if ( 0x631 == cNextCh || 0x64A == cNextCh ||
                             0x649 == cNextCh )
                        {
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                        }
                    }

                    // 6. Priority:
                    // other connecting possibilities
                    if ( nIdx && nIdx + 1 == rWord.Len() &&
                         0x60C <= cCh && 0x6FE >= cCh )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if character is connectable to previous character,
                        if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                        {
                            // only choose this position if we did not find
                            // a better one:
                            if ( STRING_LEN == nKashidaPos )
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                            break;
                        }
                    }

                    // Do not consider Fathatan, Dammatan, Kasratan, Fatha,
                    // Damma, Kasra, Shadda and Sukun when checking if
                    // a character can be connected to previous character.
                    if ( cCh < 0x64B || cCh > 0x652 )
                        cPrevCh = cCh;

                    ++nIdx;
                } // end of current word

                if ( STRING_LEN != nKashidaPos )
                    aKashida.Insert( nKashidaPos, nCntKash++ );
            } // end of kashida search
        }

        if ( nChg < rTxt.Len() )
            nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg );

        nLastCompression = nChg;
        nLastKashida = nChg;
    };

#ifndef PRODUCT
    // check kashida data
    long nTmpKashidaPos = -1;
    sal_Bool bWrongKash = sal_False;
    for (i = 0; i < aKashida.Count(); ++i )
    {
        long nCurrKashidaPos = GetKashida( i );
        if ( nCurrKashidaPos <= nTmpKashidaPos )
        {
            bWrongKash = sal_True;
            break;
        }
        nTmpKashidaPos = nCurrKashidaPos;
    }
    ASSERT( ! bWrongKash, "Kashida array contains wrong data" )
#endif

    // remove invalid entries from direction information arrays
    const USHORT nDirRemove = aDirChg.Count();
    aDirChg.Remove( 0, nDirRemove );
    aDirType.Remove( 0, nDirRemove );

    // Perform Unicode Bidi Algorithm for text direction information
    bool bPerformUBA = UBIDI_LTR != nDefaultDir;
    nCnt = 0;
    while( !bPerformUBA && nCnt < CountScriptChg() )
    {
        if ( i18n::ScriptType::COMPLEX == GetScriptType( nCnt++ ) )
            bPerformUBA = true;
    }

    // do not call the unicode bidi algorithm if not required
    if ( bPerformUBA )
    {
        UpdateBidiInfo( rTxt );

        // #i16354# Change script type for RTL text to CTL.
        for ( USHORT nDirIdx = 0; nDirIdx < aDirChg.Count(); ++nDirIdx )
        {
            if ( GetDirType( nDirIdx ) == UBIDI_RTL )
            {
                // nStart ist start of RTL run:
                const xub_StrLen nStart = nDirIdx > 0 ? GetDirChg( nDirIdx - 1 ) : 0;
                // nEnd is end of RTL run:
                const xub_StrLen nEnd = GetDirChg( nDirIdx );
                // nScriptIdx points into the ScriptArrays:
                USHORT nScriptIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                // Make nScriptIdx become the index of the script group with
                // 1. nStartPosOfGroup <= nStart and
                // 2. nEndPosOfGroup > nStart
                while ( GetScriptChg( nScriptIdx ) <= nStart )
                    ++nScriptIdx;

                xub_StrLen nEndPosOfGroup = GetScriptChg( nScriptIdx );
                xub_StrLen nStartPosOfGroup = nScriptIdx ? GetScriptChg( nScriptIdx - 1 ) : 0;
                BYTE nScriptTypeOfGroup = GetScriptType( nScriptIdx );

                ASSERT( nStartPosOfGroup <= nStart && nEndPosOfGroup > nStart,
                        "Script override with CTL font trouble" )

                // Check if we have to insert a new script change at
                // position nStart. If nStartPosOfGroup < nStart,
                // we have to insert a new script change:
                if ( nStart > 0 && nStartPosOfGroup < nStart )
                {
                    aScriptChg.Insert( nStart, nScriptIdx );
                    aScriptType.Insert( nScriptTypeOfGroup, nScriptIdx );
                    ++nScriptIdx;
                }

                // Remove entries in ScriptArray which end inside the RTL run:
                while ( nScriptIdx < aScriptChg.Count() && GetScriptChg( nScriptIdx ) <= nEnd )
                {
                    aScriptChg.Remove( nScriptIdx, 1 );
                    aScriptType.Remove( nScriptIdx, 1 );
                }

                // Insert a new entry in ScriptArray for the end of the RTL run:
                aScriptChg.Insert( nEnd, nScriptIdx );
                aScriptType.Insert( i18n::ScriptType::COMPLEX, nScriptIdx );

#if OSL_DEBUG_LEVEL > 1
                BYTE nScriptType;
                BYTE nLastScriptType = i18n::ScriptType::WEAK;
                xub_StrLen nScriptChg;
                xub_StrLen nLastScriptChg = 0;

                for ( int i = 0; i < aScriptChg.Count(); ++i )
                {
                    nScriptChg = GetScriptChg( i );
                    nScriptType = GetScriptType( i );
                    ASSERT( nLastScriptType != nScriptType &&
                            nLastScriptChg < nScriptChg,
                            "Heavy InitScriptType() confusion" )
                }
#endif
            }
        }
    }
}

void SwScriptInfo::UpdateBidiInfo( const String& rTxt )
{
    // remove invalid entries from direction information arrays
    const USHORT nDirRemove = aDirChg.Count();
    aDirChg.Remove( 0, nDirRemove );
    aDirType.Remove( 0, nDirRemove );

    //
    // Bidi functions from icu 2.0
    //
    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( rTxt.Len(), 0, &nError );
    nError = U_ZERO_ERROR;

    ubidi_setPara( pBidi, rTxt.GetBuffer(), rTxt.Len(),
                   nDefaultDir, NULL, &nError );
    nError = U_ZERO_ERROR;
    long nCount = ubidi_countRuns( pBidi, &nError );
    int32_t nStart = 0;
    int32_t nEnd;
    UBiDiLevel nCurrDir;
    // counter for direction information arrays
    USHORT nCntDir = 0;

    for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
    {
        ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
        aDirChg.Insert( (USHORT)nEnd, nCntDir );
        aDirType.Insert( (BYTE)nCurrDir, nCntDir++ );
        nStart = nEnd;
    }

    ubidi_close( pBidi );
}


/*************************************************************************
 *                        SwScriptInfo::NextScriptChg(..)
 * returns the position of the next character which belongs to another script
 * than the character of the actual (input) position.
 * If there's no script change until the end of the paragraph, it will return
 * STRING_LEN.
 * Scripts are Asian (Chinese, Japanese, Korean),
 *             Latin ( English etc.)
 *         and Complex ( Hebrew, Arabian )
 *************************************************************************/

xub_StrLen SwScriptInfo::NextScriptChg( const xub_StrLen nPos )  const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    }

    return STRING_LEN;
}

/*************************************************************************
 *                        SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

BYTE SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    // the default is the application language script
    return (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
}

xub_StrLen SwScriptInfo::NextDirChg( const xub_StrLen nPos,
                                     const BYTE* pLevel )  const
{
    BYTE nCurrDir = pLevel ? *pLevel : 62;
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) &&
            ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
            return GetDirChg( nX );
    }

    return STRING_LEN;
}

BYTE SwScriptInfo::DirType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) )
            return GetDirType( nX );
    }

    return 0;
}

/*************************************************************************
 *                        SwScriptInfo::MaskHiddenRanges(..)
 * Takes a string and replaced the hidden ranges with cChar.
 **************************************************************************/

USHORT SwScriptInfo::MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
                                       const xub_StrLen nStt, const xub_StrLen nEnd,
                                       const xub_Unicode cChar )
{
    ASSERT( rNode.GetTxt().Len() == rText.Len(), "MaskHiddenRanges, string len mismatch" )

    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    USHORT nNumOfHiddenChars = 0;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        if ( nHiddenEnd < nStt || nHiddenStart > nEnd )
            continue;

        while ( nHiddenStart < nHiddenEnd && nHiddenStart < nEnd )
        {
            if ( nHiddenStart >= nStt && nHiddenStart < nEnd )
            {
                rText.SetChar( nHiddenStart, cChar );
                ++nNumOfHiddenChars;
            }
            ++nHiddenStart;
        }
    }

    return nNumOfHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::DeleteHiddenRanges(..)
 * Takes a SwTxtNode and deletes the hidden ranges from the node.
 **************************************************************************/

void SwScriptInfo::DeleteHiddenRanges( SwTxtNode& rNode )
{
    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        SwPaM aPam( rNode, nHiddenStart, rNode, nHiddenEnd );
        rNode.GetDoc()->Delete( aPam );
    }
}


/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos,
                                           xub_StrLen& rnStartPos, xub_StrLen& rnEndPos,
                                           PositionList* pList )
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    bool bNewContainsHiddenChars = false;

    //
    // Optimization: First examine the flags at the text node:
    //
    if ( !rNode.IsCalcHiddenCharFlags() )
    {
        bool bWholePara = rNode.HasHiddenCharAttribute( true );
        bool bContainsHiddenChars = rNode.HasHiddenCharAttribute( false );
        if ( !bContainsHiddenChars )
            return false;

        if ( bWholePara )
        {
            if ( pList )
            {
                pList->push_back( 0 );
                pList->push_back( rNode.GetTxt().Len() );
            }

            rnStartPos = 0;
            rnEndPos = rNode.GetTxt().Len();
            return true;
        }
    }

    const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rNode );
    if ( pSI )
    {
        //
        // Check first, if we have a valid SwScriptInfo object for this text node:
        //
        bNewContainsHiddenChars = pSI->GetBoundsOfHiddenRange( nPos, rnStartPos, rnEndPos, pList );
        const bool bNewHiddenCharsHidePara = ( rnStartPos == 0 && rnEndPos >= rNode.GetTxt().Len() );
        rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
    }
    else
    {
        //
        // No valid SwScriptInfo Object, we have to do it the hard way:
        //
        Range aRange( 0, rNode.GetTxt().Len() ? rNode.GetTxt().Len() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        SwScriptInfo::CalcHiddenRanges( rNode, aHiddenMulti );
        for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
            const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;

            if ( nHiddenStart > nPos )
                break;
            else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
            {
                rnStartPos = nHiddenStart;
                rnEndPos   = Min( nHiddenEnd, rNode.GetTxt().Len() );
                break;
            }
        }

        if ( pList )
        {
            for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                pList->push_back( (xub_StrLen)rRange.Min() );
                pList->push_back( (xub_StrLen)rRange.Max() + 1 );
            }
        }

        bNewContainsHiddenChars = aHiddenMulti.GetRangeCount() > 0;
    }

    return bNewContainsHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * non-static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( xub_StrLen nPos, xub_StrLen& rnStartPos,
                                           xub_StrLen& rnEndPos, PositionList* pList ) const
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    USHORT nEnd = CountHiddenChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        const xub_StrLen nHiddenStart = GetHiddenChg( nX++ );
        const xub_StrLen nHiddenEnd = GetHiddenChg( nX );

        if ( nHiddenStart > nPos )
            break;
        else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
        {
            rnStartPos = nHiddenStart;
            rnEndPos   = nHiddenEnd;
            break;
        }
    }

    if ( pList )
    {
        for( USHORT nX = 0; nX < nEnd; ++nX )
        {
            pList->push_back( GetHiddenChg( nX++ ) );
            pList->push_back( GetHiddenChg( nX ) );
        }
    }

    return CountHiddenChg() > 0;
}

/*************************************************************************
 *                        SwScriptInfo::IsInHiddenRange()
 **************************************************************************/

bool SwScriptInfo::IsInHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos )
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nPos, nStartPos, nEndPos );
    return nStartPos != STRING_LEN;
}


/*************************************************************************
 *                        SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/

BYTE SwScriptInfo::CompType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountCompChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        xub_StrLen nChg = GetCompStart( nX );

        if ( nPos < nChg )
            return NONE;

        if( nPos < nChg + GetCompLen( nX ) )
            return GetCompType( nX );
    }
    return NONE;
}

/*************************************************************************
 *                      SwScriptInfo::HasKana()
 * returns, if there are compressable kanas or specials
 * betwenn nStart and nEnd
 *************************************************************************/

USHORT SwScriptInfo::HasKana( xub_StrLen nStart, const xub_StrLen nLen ) const
{
    USHORT nCnt = CountCompChg();
    xub_StrLen nEnd = nStart + nLen;

    for( USHORT nX = 0; nX < nCnt; ++nX )
    {
        xub_StrLen nKanaStart  = GetCompStart( nX );
        xub_StrLen nKanaEnd = nKanaStart + GetCompLen( nX );

        if ( nKanaStart >= nEnd )
            return USHRT_MAX;

        if ( nStart < nKanaEnd )
            return nX;
    }

    return USHRT_MAX;
}

/*************************************************************************
 *                      SwScriptInfo::Compress()
 *************************************************************************/

long SwScriptInfo::Compress( long* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                             const USHORT nCompress, const USHORT nFontHeight,
                             Point* pPoint ) const
{
    ASSERT( nCompress, "Compression without compression?!" );
    ASSERT( nLen, "Compression without text?!" );
    USHORT nCompCount = CountCompChg();

    // In asian typography, there are full width and half width characters.
    // Full width punctuation characters can be compressed by 50 %
    // to determine this, we compare the font width with 75 % of its height
    USHORT nMinWidth = ( 3 * nFontHeight ) / 4;

    USHORT nCompIdx = HasKana( nIdx, nLen );

    if ( USHRT_MAX == nCompIdx )
        return 0;

    xub_StrLen nChg = GetCompStart( nCompIdx );
    xub_StrLen nCompLen = GetCompLen( nCompIdx );
    USHORT nI = 0;
    nLen += nIdx;

    if( nChg > nIdx )
    {
        nI = nChg - nIdx;
        nIdx = nChg;
    }
    else if( nIdx < nChg + nCompLen )
        nCompLen -= nIdx - nChg;

    if( nIdx > nLen || nCompIdx >= nCompCount )
        return 0;

    long nSub = 0;
    long nLast = nI ? pKernArray[ nI - 1 ] : 0;
    do
    {
        USHORT nType = GetCompType( nCompIdx );
        ASSERT( nType == CompType( nIdx ), "Gimme the right type!" );
        nCompLen += nIdx;
        if( nCompLen > nLen )
            nCompLen = nLen;

        // are we allowed to compress the character?
        if ( pKernArray[ nI ] - nLast < nMinWidth )
        {
            nIdx++; nI++;
        }
        else
        {
            while( nIdx < nCompLen )
            {
                ASSERT( SwScriptInfo::NONE != nType, "None compression?!" );

                // nLast is width of current character
                nLast -= pKernArray[ nI ];

                nLast *= nCompress;
                long nMove = 0;
                if( SwScriptInfo::KANA != nType )
                {
                    nLast /= 20000;
                    if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
                    {
                        if( nI )
                            nMove = nLast;
                        else
                        {
                            pPoint->X() += nLast;
                            nLast = 0;
                        }
                    }
                }
                else
                    nLast /= 100000;
                nSub -= nLast;
                nLast = pKernArray[ nI ];
                if( nMove )
                    pKernArray[ nI - 1 ] += nMove;
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }

        if( nIdx < nLen )
        {
            xub_StrLen nChg;
            if( ++nCompIdx < nCompCount )
            {
                nChg = GetCompStart( nCompIdx );
                if( nChg > nLen )
                    nChg = nLen;
                nCompLen = GetCompLen( nCompIdx );
            }
            else
                nChg = nLen;
            while( nIdx < nChg )
            {
                nLast = pKernArray[ nI ];
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }
        else
            break;
    } while( nIdx < nLen );
    return nSub;
}

/*************************************************************************
 *                      SwScriptInfo::KashidaJustify()
 *************************************************************************/

USHORT SwScriptInfo::KashidaJustify( long* pKernArray, long* pScrArray,
                                     xub_StrLen nStt, xub_StrLen nLen,
                                     USHORT nSpace ) const
{
    ASSERT( nLen, "Kashida justification without text?!" )

    // evaluate kashida informatin in collected in SwScriptInfo

    USHORT nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    if ( ! pKernArray )
    {
        USHORT nCntKashEnd = nCntKash;
        while ( nCntKashEnd < CountKashida() )
        {
            if ( nEnd <= GetKashida( nCntKashEnd ) )
                break;
            else
                nCntKashEnd++;
        }

        return nCntKashEnd - nCntKash;
    }

    // do nothing if there is no more kashida
    if ( nCntKash < CountKashida() )
    {
        xub_StrLen nKashidaPos = GetKashida( nCntKash );
        xub_StrLen nIdx = nKashidaPos;
        USHORT nSpaceAdd = nSpace;

        while ( nIdx < nEnd )
        {
            USHORT nArrayPos = nIdx - nStt;

            // next kashida position
            nIdx = ++nCntKash  < CountKashida() ? GetKashida( nCntKash ) : nEnd;
            if ( nIdx > nEnd )
                nIdx = nEnd;

            const USHORT nArrayEnd = nIdx - nStt;

            while ( nArrayPos < nArrayEnd )
            {
                pKernArray[ nArrayPos ] += nSpaceAdd;
                if ( pScrArray )
                   pScrArray[ nArrayPos ] += nSpaceAdd;
                ++nArrayPos;
            }

            nSpaceAdd += nSpace;
        }
    }

    return 0;
}

/*************************************************************************
 *                      SwScriptInfo::IsArabicLanguage()
 *************************************************************************/

sal_Bool SwScriptInfo::IsArabicLanguage( LanguageType aLang )
{
    return LANGUAGE_ARABIC == aLang || LANGUAGE_ARABIC_SAUDI_ARABIA == aLang ||
           LANGUAGE_ARABIC_IRAQ == aLang || LANGUAGE_ARABIC_EGYPT == aLang ||
           LANGUAGE_ARABIC_LIBYA == aLang || LANGUAGE_ARABIC_ALGERIA == aLang ||
           LANGUAGE_ARABIC_MOROCCO == aLang || LANGUAGE_ARABIC_TUNISIA == aLang ||
           LANGUAGE_ARABIC_OMAN == aLang || LANGUAGE_ARABIC_YEMEN == aLang ||
           LANGUAGE_ARABIC_SYRIA == aLang || LANGUAGE_ARABIC_JORDAN == aLang ||
           LANGUAGE_ARABIC_LEBANON == aLang || LANGUAGE_ARABIC_KUWAIT == aLang ||
           LANGUAGE_ARABIC_UAE == aLang || LANGUAGE_ARABIC_BAHRAIN == aLang ||
           LANGUAGE_ARABIC_QATAR == aLang;
}

/*************************************************************************
 *                      SwScriptInfo::ThaiJustify()
 *************************************************************************/

USHORT SwScriptInfo::ThaiJustify( const XubString& rTxt, long* pKernArray,
                                  long* pScrArray, xub_StrLen nStt,
                                  xub_StrLen nLen, USHORT nSpace )
{
    ASSERT( nStt + nLen <= rTxt.Len(), "String in ThaiJustify too small" )

    long nSpaceSum = 0;
    USHORT nCnt = 0;

    for ( USHORT nI = 0; nI < nLen; ++nI )
    {
        const xub_Unicode cCh = rTxt.GetChar( nStt + nI );

        // check if character is not above or below base
        if ( ( 0xE34 > cCh || cCh > 0xE3A ) &&
             ( 0xE47 > cCh || cCh > 0xE4E ) && cCh != 0xE31 )
        {
            nSpaceSum += nSpace;
            ++nCnt;
        }

        if ( pKernArray ) pKernArray[ nI ] += nSpaceSum;
        if ( pScrArray ) pScrArray[ nI ] += nSpaceSum;
    }

    return nCnt;
}

/*************************************************************************
 *                      SwScriptInfo::GetScriptInfo()
 *************************************************************************/

SwScriptInfo* SwScriptInfo::GetScriptInfo( const SwTxtNode& rTNd,
                                           sal_Bool bAllowInvalid )
{
    SwClientIter aClientIter( (SwTxtNode&)rTNd );
    SwClient* pLast = aClientIter.GoStart();
    SwScriptInfo* pScriptInfo = 0;

    while( pLast )
    {
        if ( pLast->ISA( SwTxtFrm ) )
        {
            pScriptInfo = (SwScriptInfo*)((SwTxtFrm*)pLast)->GetScriptInfo();
            if ( pScriptInfo )
            {
                if ( !bAllowInvalid && STRING_LEN != pScriptInfo->GetInvalidity() )
                    pScriptInfo = 0;
                else break;
            }
        }
        pLast = ++aClientIter;
    }

    return pScriptInfo;
}



/*************************************************************************
 *                      class SwParaPortion
 *************************************************************************/

SwParaPortion::SwParaPortion()
{
    FormatReset();
    bFlys = bFtnNum = bMargin = sal_False;
    SetWhichPor( POR_PARA );
}

/*************************************************************************
 *                      SwParaPortion::GetParLen()
 *************************************************************************/

xub_StrLen SwParaPortion::GetParLen() const
{
    xub_StrLen nLen = 0;
    const SwLineLayout *pLay = this;
    while( pLay )
    {
        DBG_LOOP;
        nLen += pLay->GetLen();
        pLay = pLay->GetNext();
    }
    return nLen;
}

/*************************************************************************
 *                      SwParaPortion::FindDropPortion()
 *************************************************************************/

const SwDropPortion *SwParaPortion::FindDropPortion() const
{
    const SwLineLayout *pLay = this;
    while( pLay && pLay->IsDummy() )
        pLay = pLay->GetNext();
    while( pLay )
    {
        const SwLinePortion *pPos = pLay->GetPortion();
        while ( pPos && !pPos->GetLen() )
            pPos = pPos->GetPortion();
        if( pPos && pPos->IsDropPortion() )
            return (SwDropPortion *)pPos;
        pLay = pLay->GetLen() ? NULL : pLay->GetNext();
    }
    return NULL;
}

/*************************************************************************
 *                      SwLineLayout::Init()
 *************************************************************************/

void SwLineLayout::Init( SwLinePortion* pNextPortion )
{
    Height( 0 );
    Width( 0 );
    SetLen( 0 );
    SetAscent( 0 );
    SetRealHeight( 0 );
    SetPortion( pNextPortion );
}

/*-----------------16.11.00 11:04-------------------
 * HangingMargin()
 * looks for hanging punctuation portions in the paragraph
 * and return the maximum right offset of them.
 * If no such portion is found, the Margin/Hanging-flags will be atualized.
 * --------------------------------------------------*/

SwTwips SwLineLayout::_GetHangingMargin() const
{
    SwLinePortion* pPor = GetPortion();
    BOOL bFound = sal_False;
    SwTwips nDiff = 0;
    while( pPor)
    {
        if( pPor->IsHangingPortion() )
        {
            nDiff = ((SwHangingPortion*)pPor)->GetInnerWidth() - pPor->Width();
            if( nDiff )
                bFound = sal_True;
        }
        // the last post its portion
        else if ( pPor->IsPostItsPortion() && ! pPor->GetPortion() )
            nDiff = nAscent;

        pPor = pPor->GetPortion();
    }
    if( !bFound ) // actualize the hanging-flag
        ((SwLineLayout*)this)->SetHanging( sal_False );
    return nDiff;
}

SwTwips SwTxtFrm::HangingMargin() const
{
    ASSERT( HasPara(), "Don't call me without a paraportion" );
    if( !GetPara()->IsMargin() )
        return 0;
    const SwLineLayout* pLine = GetPara();
    SwTwips nRet = 0;
    do
    {
        SwTwips nDiff = pLine->GetHangingMargin();
        if( nDiff > nRet )
            nRet = nDiff;
        pLine = pLine->GetNext();
    } while ( pLine );
    if( !nRet ) // actualize the margin-flag
        ((SwParaPortion*)GetPara())->SetMargin( sal_False );
    return nRet;
}


/*************************************************************************
 *                      SwScriptInfo::CalcHiddenRanges()
 *
 * Returns a MultiSection indicating the hidden ranges.
 *************************************************************************/

void SwScriptInfo::CalcHiddenRanges( const SwTxtNode& rNode, MultiSelection& rHiddenMulti )
{
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState( RES_CHRATR_HIDDEN, TRUE, &pItem ) &&
        ((SvxCharHiddenItem*)pItem)->GetValue() )
    {
        rHiddenMulti.SelectAll();
    }

    const SwpHints* pHints = rNode.GetpSwpHints();
    const SwTxtAttr* pTxtAttr = 0;
    bool bHidden = false;
    bool bHiddenSelect = false;

    if( pHints )
    {
        MSHORT nTmp = 0;

        while( nTmp < pHints->GetStartCount() )
        {
            pTxtAttr = pHints->GetStart( nTmp++ );
            switch ( pTxtAttr->Which() )
            {
                case RES_CHRATR_HIDDEN:
                {
                    bHidden = sal_True;
                    bHiddenSelect = pTxtAttr->GetCharHidden().GetValue();
                }
                break;
                case RES_TXTATR_CHARFMT:
                {
                    SwCharFmt* pFmt;
                    const SfxPoolItem* pItem;
                    pFmt = pTxtAttr->GetCharFmt().GetCharFmt();
                    if ( pFmt )
                    {
                        if( SFX_ITEM_SET == pFmt->GetAttrSet().
                            GetItemState( RES_CHRATR_HIDDEN, sal_True, &pItem ) )
                        {
                            bHidden = sal_True;
                            bHiddenSelect = ((SvxCharHiddenItem*)pItem)->GetValue();
                        }
                    }
                }
                break;
            }
            if( bHidden )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    if( bHidden )
                        rHiddenMulti.Select( aTmp, bHiddenSelect );
                }
                bHidden = sal_False;
            }
        }
    }

    // If there are any hidden ranges in the current text node, we have
    // to unhide the redlining ranges:
    const SwDoc& rDoc = *rNode.GetDoc();
    if ( rHiddenMulti.GetRangeCount() && ::IsShowChanges( rDoc.GetRedlineMode() ) )
    {
        USHORT nAct = rDoc.GetRedlinePos( rNode );

        for ( ; nAct < rDoc.GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = rDoc.GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > rNode.GetIndex() )
                break;

            xub_StrLen nRedlStart;
            xub_StrLen nRedlnEnd;
            pRed->CalcStartEnd( rNode.GetIndex(), nRedlStart, nRedlnEnd );
            if ( nRedlnEnd > nRedlStart )
            {
                Range aTmp( nRedlStart, nRedlnEnd - 1 );
                rHiddenMulti.Select( aTmp, false );
            }
        }
    }

    //
    // We calculated a lot of stuff. Finally we can update the flags at the text node.
    //
    const bool bNewContainsHiddenChars = rHiddenMulti.GetRangeCount() > 0;
    bool bNewHiddenCharsHidePara = false;
    if ( bNewContainsHiddenChars )
    {
        const Range& rRange = rHiddenMulti.GetRange( 0 );
        const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;
        bNewHiddenCharsHidePara = ( nHiddenStart == 0 && nHiddenEnd >= rNode.GetTxt().Len() );
    }
    rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
}

void SwTxtNode::CalcHiddenCharFlags() const
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    // Update of the flags is done inside GetBoundsOfHiddenRange()
    SwScriptInfo::GetBoundsOfHiddenRange( *this, 0, nStartPos, nEndPos );
}
