/*************************************************************************
 *
 *  $RCSfile: itrpaint.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-30 09:58:34 $
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

#include "hintids.hxx"
#include "flyfrm.hxx"     // SwFlyInCntFrm
#include "viewopt.hxx"  // SwViewOptions
#include "errhdl.hxx"
#include "txtatr.hxx"  // SwINetFmt

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "flyfrms.hxx"
#include "viewsh.hxx"
#include "txtcfg.hxx"
#include "itrpaint.hxx"
#include "txtfrm.hxx"   // pFrm
#include "txtfly.hxx"
#include "swfont.hxx"
#include "txtpaint.hxx"
#include "portab.hxx"   // SwTabPortion::IsFilled
#include "porfly.hxx"     // SwFlyCntPortion
#include "porfld.hxx"   // SwGrfNumPortion
#include "frmfmt.hxx"   // LRSpace
#include "txatbase.hxx" // SwTxtAttr
#include "charfmt.hxx"  // SwFmtCharFmt
#include "redlnitr.hxx" // SwRedlineItr
#include "porrst.hxx"   // SwArrowPortion

/*************************************************************************
 *                  SwTxtPainter::CtorInit()
 *************************************************************************/
void SwTxtPainter::CtorInit( SwTxtFrm *pFrm, SwTxtPaintInfo *pNewInf )
{
    SwTxtCursor::CtorInit( pFrm, pNewInf );
    pInf = pNewInf;
    SwFont *pFnt = GetFnt();
    GetInfo().SetFont( pFnt );
#ifndef PRODUCT
    if( ALIGN_BASELINE != pFnt->GetAlign() )
    {
        ASSERT( ALIGN_BASELINE == pFnt->GetAlign(),
                "+SwTxtPainter::CTOR: font alignment revolution" );
        pFnt->SetAlign( ALIGN_BASELINE );
    }
#endif
    bPaintDrop = sal_False;
}


/*************************************************************************
 *                    SwTxtPainter::CalcPaintOfst()
 *************************************************************************/
SwLinePortion *SwTxtPainter::CalcPaintOfst( const SwRect &rPaint )
{
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    GetInfo().SetPaintOfst( 0 );
    SwTwips nPaintOfst = rPaint.Left();

    // nPaintOfst wurde exakt auf das Ende eingestellt, deswegen <=
    // nPaintOfst ist dokumentglobal, deswegen nLeftMar aufaddieren
    // const KSHORT nLeftMar = KSHORT(GetLeftMargin());
    // 8310: painten von LineBreaks in leeren Zeilen.
    if( nPaintOfst && pCurr->Width() )
    {
        SwLinePortion *pLast = 0;
        // 7529 und 4757: nicht <= nPaintOfst
        while( pPor && GetInfo().X() + pPor->Width() + (pPor->Height()/2)
                       < nPaintOfst )
        {
            DBG_LOOP;
            if( pPor->InSpaceGrp() && GetInfo().GetSpaceAdd() )
            {
                long nTmp = GetInfo().X() +pPor->Width() +
                    pPor->CalcSpacing( GetInfo().GetSpaceAdd(), GetInfo() );
                if( nTmp + (pPor->Height()/2) >= nPaintOfst )
                    break;
                GetInfo().X( nTmp );
                GetInfo().SetIdx( GetInfo().GetIdx() + pPor->GetLen() );
            }
            else
                pPor->Move( GetInfo() );
            pLast = pPor;
            pPor = pPor->GetPortion();
        }

#ifndef USED
        // 7529: bei PostIts auch pLast returnen.
        if( pLast && !pLast->Width() && pLast->IsPostItsPortion() )
        {
            pPor = pLast;
            GetInfo().SetIdx( GetInfo().GetIdx() - pPor->GetLen() );
        }
#endif
    }
    return pPor;
}

/*************************************************************************
 *                    SwTxtPainter::DrawTextLine()
 *
 * Es gibt zwei Moeglichkeiten bei transparenten Font auszugeben:
 * 1) DrawRect auf die ganze Zeile und die DrawText hinterher
 *    (objektiv schnell, subjektiv langsam).
 * 2) Fuer jede Portion ein DrawRect mit anschliessendem DrawText
 *    ausgefuehrt (objektiv langsam, subjektiv schnell).
 * Da der User in der Regel subjektiv urteilt, wird die 2. Methode
 * als Default eingestellt.
 *************************************************************************/
void SwTxtPainter::DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                                 const sal_Bool bUnderSz )
{
    // Adjustierung ggf. nachholen
    GetAdjusted();
    GetInfo().SetSpaceAdd( pCurr->GetpSpaceAdd() );
    GetInfo().ResetSpaceIdx();
    // Die Groesse des Frames
    GetInfo().SetIdx( GetStart() );
    GetInfo().SetPos( GetTopLeft() );

    const sal_Bool bDrawInWindow = GetInfo().OnWin();

    // 6882: Leerzeilen duerfen nicht wegoptimiert werden bei Paragraphzeichen.
    const sal_Bool bEndPor = GetInfo().GetOpt().IsParagraph() && !GetInfo().GetTxt().Len();

    SwLinePortion *pPor = bEndPor ? pCurr->GetFirstPortion() : CalcPaintOfst( rPaint );

    // Optimierung!
    const SwTwips nMaxRight = Min( rPaint.Right(), Right() );
    const SwTwips nTmpLeft = GetInfo().X();
    if( !bEndPor && nTmpLeft >= nMaxRight )
        return;

    // DropCaps!
    // 7538: natuerlich auch auf dem Drucker
    if( !bPaintDrop )
    {
        // 8084: Optimierung, weniger Painten.
        // AMA: Durch 8084 wurde 7538 wiederbelebt!
        // bDrawInWindow entfernt, damit DropCaps auch gedruckt werden
        bPaintDrop = pPor == pCurr->GetFirstPortion()
                     && GetDropLines() >= GetLineNr();
    }

    KSHORT nTmpHeight, nTmpAscent;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );

    // bClip entscheidet darueber, ob geclippt werden muss.
    // Das Ganze muss vor der Retusche stehen

    sal_Bool bClip = ( bDrawInWindow || bUnderSz ) && !rClip.IsChg();
    if( bClip && pPor )
    {
        // Wenn TopLeft oder BottomLeft der Line ausserhalb liegen,
        // muss geclippt werden. Die Ueberpruefung auf Right() erfolgt
        // in der folgenden Ausgabeschleife...

        if( GetInfo().GetPos().X() < rPaint.Left() ||
            GetInfo().GetPos().Y() < rPaint.Top() ||
            GetInfo().GetPos().Y() + nTmpHeight > rPaint.Top() + rPaint.Height() )
        {
            bClip = sal_False;
            rClip.ChgClip( rPaint );
        }
#ifdef DEBUG
        static sal_Bool bClipAlways = sal_False;
        if( bClip && bClipAlways )
        {   bClip = sal_False;
            rClip.ChgClip( rPaint );
        }
#endif
    }

    // Alignment:
    sal_Bool bPlus = sal_False;
    OutputDevice *pOut = GetInfo().GetOut();
    Point aPnt1( nTmpLeft, GetInfo().GetPos().Y() );
    if ( aPnt1.X() < rPaint.Left() )
        aPnt1.X() = rPaint.Left();
    if ( aPnt1.Y() < rPaint.Top() )
        aPnt1.Y() = rPaint.Top();
    Point aPnt2( GetInfo().GetPos().X() + nMaxRight - GetInfo().X(),
                 GetInfo().GetPos().Y() + nTmpHeight );
    if ( aPnt2.X() > rPaint.Right() )
        aPnt2.X() = rPaint.Right();
    if ( aPnt2.Y() > rPaint.Bottom() )
    {
        aPnt2.Y() = rPaint.Bottom();
        bPlus = sal_True;
    }

    const SwRect aLineRect( aPnt1, aPnt2 );

    if( pCurr->IsClipping() )
    {
        rClip.ChgClip( aLineRect );
        bClip = sal_False;
    }

    if( !pPor && !bEndPor )
    {
#ifdef DBGTXT
        aDbstream << "PAINTER: done nothing" << endl;
#endif
        return;
    }

    // Baseline-Ausgabe auch bei nicht-TxtPortions (vgl. TabPor mit Fill)
    const SwTwips nY = GetInfo().GetPos().Y() + nTmpAscent;
    GetInfo().Y( nY );

    // 7529: PostIts prepainten
    if( GetInfo().OnWin() && pPor && !pPor->Width() )
    {
        SeekAndChg( GetInfo() );
        pPor->PrePaint( GetInfo(), pPor );
    }

    // 7923: EndPortions geben auch Zeichen aus, deswegen den Fnt wechseln!
    if( bEndPor )
        SeekStartAndChg( GetInfo() );

    sal_Bool bRest = pCurr->IsRest();
    sal_Bool bFirst = sal_True;

    SwArrowPortion *pArrow = NULL;

    while( pPor )
    {
        DBG_LOOP;
        sal_Bool bSeeked = sal_True;
        GetInfo().SetLen( pPor->GetLen() );
        GetInfo().SetSpecialUnderline( sal_False );
        // Ein Sonderfall sind GluePortions, die Blanks ausgeben.

        // 6168: Der Rest einer FldPortion zog sich die Attribute der naechsten
        // Portion an, dies wird durch SeekAndChgBefore vermieden:
        if( ( bRest && pPor->InFldGrp() && !pPor->GetLen() ) )
            SeekAndChgBefore( GetInfo() );
        else if ( pPor->IsQuoVadisPortion() )
        {
            xub_StrLen nOffset = GetInfo().GetIdx();
            SeekStartAndChg( GetInfo(), sal_True );
            if( GetRedln() && pCurr->HasRedline() )
                GetRedln()->Seek( *pFnt, nOffset, 0 );
        }
        else if( pPor->InTxtGrp() || pPor->InFldGrp() || pPor->InTabGrp() )
            SeekAndChg( GetInfo() );
        else if ( !bFirst && pPor->IsBreakPortion() && GetInfo().GetOpt().IsParagraph() )
        {
            // Paragraphzeichen sollten den gleichen Font wie das Zeichen vor
            // haben, es sei denn, es gibt Redlining in dem Absatz.
            if( GetRedln() )
                SeekAndChg( GetInfo() );
            else
                SeekAndChgBefore( GetInfo() );
        }
        else
            bSeeked = sal_False;

//      bRest = sal_False;

        ASSERT( GetInfo().Y() == nY, "DrawTextLine: Y() has changed" );

        // Wenn das Ende der Portion hinausragt, wird geclippt.
        // Es wird ein Sicherheitsabstand von Height-Halbe aufaddiert,
        // damit die TTF-"f" nicht im Seitenrand haengen...
        if(bClip && GetInfo().X() + pPor->Width() + (pPor->Height()/2) > nMaxRight)
        {   bClip = sal_False;
            rClip.ChgClip( rPaint );
        }

        // Portions, die "unter" dem Text liegen wie PostIts
        SwLinePortion *pNext = pPor->GetPortion();
        if(GetInfo().OnWin() && pNext && !pNext->Width() )
        {
            // Fix 11289: Felder waren hier ausgeklammert wg. Last!=Owner beim
            // Laden von Brief.sdw. Jetzt sind die Felder wieder zugelassen,
            // durch bSeeked wird Last!=Owner vermieden.
            if ( !bSeeked )
                SeekAndChg( GetInfo() );
            pNext->PrePaint( GetInfo(), pPor );
        }

        if( pFnt->GetEscapement() && UNDERLINE_NONE != pFnt->GetUnderline() )
            CheckSpecialUnderline();

        if( pPor->IsMultiPortion() )
            PaintMultiPortion( rPaint, (SwMultiPortion&)*pPor );
        else
            pPor->Paint( GetInfo() );

        if( GetFnt()->IsURL() && pPor->InTxtGrp() )
            GetInfo().NotifyURL( *pPor );

        bFirst &= !pPor->GetLen();
        if( pNext || !pPor->IsMarginPortion() )
            pPor->Move( GetInfo() );
        if( pPor->IsArrowPortion() && GetInfo().OnWin() && !pArrow )
            pArrow = (SwArrowPortion*)pPor;

        pPor = !bDrawInWindow && GetInfo().X() > nMaxRight ? 0 : pNext;
    }
    if( bDrawInWindow )
    {
        if( !GetNextLine() &&
            GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreView() &&
            GetInfo().GetOpt().IsParagraph() && !GetTxtFrm()->GetFollow() &&
            GetInfo().GetIdx() >= GetInfo().GetTxt().Len() )
        {
            SwTmpEndPortion aEnd( *pCurr->GetFirstPortion() );
            aEnd.Paint( GetInfo() );
        }
        if( bUnderSz )
        {
            if( GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreView() )
            {
                if( pArrow )
                    pArrow->PaintIt( pOut );
                if( !GetTxtFrm()->GetFollow() )
                {
                    SwTwips nDiff = GetInfo().Y() + nTmpHeight - nTmpAscent - GetTxtFrm()->Frm().Bottom();
                    if( nDiff > 0 && ( GetEnd() < GetInfo().GetTxt().Len() ||
                        ( nDiff > nTmpHeight/2 && GetPrevLine() ) ) )
                    {
                        SwArrowPortion aArrow( GetInfo() );
                        aArrow.PaintIt( pOut );
                    }
                }
            }
        }
    }
    if( pCurr->IsClipping() )
        rClip.ChgClip( rPaint );
}

void SwTxtPainter::CheckSpecialUnderline()
{
    sal_Bool bSpecial = sal_False;
    if( HasHints() )
    {
        sal_Bool bINet = sal_False;
        MSHORT nTmp = 0;
        Range aRange( 0, GetInfo().GetTxt().Len() );
        MultiSelection aUnderMulti( aRange );
        if( bUnderPara )
            aUnderMulti.SelectAll();
        MultiSelection aEscMulti( aRange );
        if( bEscPara )
            aEscMulti.SelectAll();
        SwTxtAttr* pTxtAttr;
        sal_Bool bUnder = sal_False;
        sal_Bool bEsc = sal_False;
        while( nTmp < pHints->GetStartCount() )
        {
            pTxtAttr = pHints->GetStart( nTmp++ );
            sal_Bool bUnderSelect;
            sal_Bool bEscSelect;
            switch ( pTxtAttr->Which() )
            {
                case RES_CHRATR_UNDERLINE:
                {
                    bUnder = sal_True;
                    bUnderSelect = UNDERLINE_NONE != pTxtAttr->GetUnderline().
                                                          GetUnderline();
                }
                break;
                case RES_CHRATR_ESCAPEMENT:
                {
                    bEsc = sal_True;
                    bEscSelect = 0 != pTxtAttr->GetEscapement().GetEsc();
                }
                break;
                case RES_TXTATR_FTN:
                {
                    xub_StrLen nStrt = *pTxtAttr->GetStart();
                    Range aRg( nStrt, nStrt + 1 );
                    aEscMulti.Select( aRg );
                }
                break;
                case RES_TXTATR_INETFMT: bINet = sal_True;
                case RES_TXTATR_CHARFMT:
                {
                    SwCharFmt* pFmt;
                    const SfxPoolItem* pItem;
                    if( bINet )
                    {
                        pFmt = ((SwTxtINetFmt*)pTxtAttr)->GetCharFmt();
                        bINet = sal_False;
                    }
                    else
                        pFmt = pTxtAttr->GetCharFmt().GetCharFmt();
                    if ( pFmt )
                    {
                        if( SFX_ITEM_SET == pFmt->GetAttrSet().
                            GetItemState( RES_CHRATR_ESCAPEMENT, sal_True, &pItem) )
                        {
                            bEscSelect = 0 !=
                                ((SvxEscapementItem *)pItem)->GetEsc();
                            bEsc = sal_True;
                        }
                        if( SFX_ITEM_SET == pFmt->GetAttrSet().
                            GetItemState( RES_CHRATR_UNDERLINE, sal_True, &pItem ) )
                        {
                            bUnderSelect = UNDERLINE_NONE !=
                                 ((SvxUnderlineItem*)pItem)->GetUnderline();
                            bUnder = sal_True;
                        }
                    }
                }
                break;
            }
            if( bUnder || bEsc )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    if( bUnder )
                        aUnderMulti.Select( aTmp, bUnderSelect );
                    if( bEsc )
                        aEscMulti.Select( aTmp, bEscSelect );
                }
                bUnder = sal_False;
                bEsc = sal_False;
            }
        }
        MSHORT i;
        xub_StrLen nEscStart = 0;
        xub_StrLen nEscEnd = 0;
        xub_StrLen nIndx = GetInfo().GetIdx();
        MSHORT nCnt = (MSHORT)aEscMulti.GetRangeCount();
        for( i = 0; i < nCnt; ++i )
        {
            const Range& rRange = aEscMulti.GetRange( i );
            if( nEscEnd == rRange.Min() )
                nEscEnd = rRange.Max();
            else if( nIndx >= rRange.Min() )
            {
                nEscStart = rRange.Min();
                nEscEnd = rRange.Max();
            }
            else
                break;
        }
        xub_StrLen nUnderStart = 0;
        xub_StrLen nUnderEnd = 0;
        nCnt = (MSHORT)aUnderMulti.GetRangeCount();
        for( i = 0; i < nCnt; ++i )
        {
            const Range& rRange = aUnderMulti.GetRange( i );
            if( nUnderEnd == rRange.Min() )
                nUnderEnd = rRange.Max();
            else if( nIndx >= rRange.Min() )
            {
                nUnderStart = rRange.Min();
                nUnderEnd = rRange.Max();
            }
            else
                break;
        }
        bSpecial = nEscStart > nUnderStart || nEscEnd < nUnderEnd;
    }
    if( bSpecial || ( GetRedln() && GetRedln()->ChkSpecialUnderline() ) )
        GetInfo().SetSpecialUnderline( sal_True );
}

