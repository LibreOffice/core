/*************************************************************************
 *
 *  $RCSfile: itrpaint.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:56:25 $
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
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>      // SwField
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
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
#include "pormulti.hxx"

/*************************************************************************
 *                  IsUnderlineBreak
 *
 * Returns, if we have an underline breaking situation
 * Adding some more conditions here means you also have to change them
 * in SwTxtPainter::CheckSpecialUnderline
 *************************************************************************/
sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt )
{
    return UNDERLINE_NONE == rFnt.GetUnderline() ||
           rPor.IsFlyPortion() || rPor.IsFlyCntPortion() ||
           rPor.IsBreakPortion() || rPor.IsMarginPortion() ||
           rPor.IsHolePortion() ||
          ( rPor.IsMultiPortion() && ! ((SwMultiPortion&)rPor).IsBidi() ) ||
           rFnt.GetEscapement() < 0 || rFnt.IsWordLineMode() ||
           SVX_CASEMAP_KAPITAELCHEN == rFnt.GetCaseMap();
}

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
#if OSL_DEBUG_LEVEL > 1
//    USHORT nFntHeight = GetInfo().GetFont()->GetHeight( GetInfo().GetVsh(), GetInfo().GetOut() );
//    USHORT nFntAscent = GetInfo().GetFont()->GetAscent( GetInfo().GetVsh(), GetInfo().GetOut() );
#endif

    // Adjustierung ggf. nachholen
    GetAdjusted();
    GetInfo().SetSpaceAdd( pCurr->GetpSpaceAdd() );
    GetInfo().ResetSpaceIdx();
    GetInfo().SetKanaComp( pCurr->GetpKanaComp() );
    GetInfo().ResetKanaIdx();
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
            rClip.ChgClip( rPaint, pFrm, pCurr->HasUnderscore() );
        }
#if OSL_DEBUG_LEVEL > 1
        static sal_Bool bClipAlways = sal_False;
        if( bClip && bClipAlways )
        {   bClip = sal_False;
            rClip.ChgClip( rPaint );
        }
#endif
    }

    // Alignment:
    sal_Bool bPlus = sal_False;
    OutputDevice* pOut = GetInfo().GetOut();
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
        rClip.ChgClip( aLineRect, pFrm );
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
    // if no special vertical alignment is used,
    // we calculate Y value for the whole line
    GETGRID( GetTxtFrm()->FindPageFrm() )
    const sal_Bool bAdjustBaseLine =
        GetLineInfo().HasSpecialAlign( GetTxtFrm()->IsVertical() ) ||
        ( 0 != pGrid );
    const SwTwips nLineBaseLine = GetInfo().GetPos().Y() + nTmpAscent;
    if ( ! bAdjustBaseLine )
        GetInfo().Y( nLineBaseLine );

    // 7529: PostIts prepainten
    if( GetInfo().OnWin() && pPor && !pPor->Width() )
    {
        SeekAndChg( GetInfo() );

        if( bAdjustBaseLine )
        {
            const SwTwips nOldY = GetInfo().Y();

            GetInfo().Y( GetInfo().GetPos().Y() + AdjustBaseLine( *pCurr, 0,
                GetInfo().GetFont()->GetHeight( GetInfo().GetVsh(), *pOut ),
                GetInfo().GetFont()->GetAscent( GetInfo().GetVsh(), *pOut )
            ) );

            pPor->PrePaint( GetInfo(), pPor );
            GetInfo().Y( nOldY );
        }
        else
            pPor->PrePaint( GetInfo(), pPor );
    }

    // 7923: EndPortions geben auch Zeichen aus, deswegen den Fnt wechseln!
    if( bEndPor )
        SeekStartAndChg( GetInfo() );

    sal_Bool bRest = pCurr->IsRest();
    sal_Bool bFirst = sal_True;

    SwArrowPortion *pArrow = NULL;
    // Reference portion for the paragraph end portion
    SwLinePortion* pEndTempl = pCurr->GetFirstPortion();

    while( pPor )
    {
        DBG_LOOP;
        sal_Bool bSeeked = sal_True;
        GetInfo().SetLen( pPor->GetLen() );

        const SwTwips nOldY = GetInfo().Y();

        if ( bAdjustBaseLine )
        {
            GetInfo().Y( GetInfo().GetPos().Y() + AdjustBaseLine( *pCurr, pPor ) );

            // we store the last portion, because a possible paragraph
            // end character has the same font as this portion
            // (only in special vertical alignment case, otherwise the first
            // portion of the line is used)
            if ( pPor->Width() && pPor->InTxtGrp() )
                pEndTempl = pPor;
        }

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

        // Wenn das Ende der Portion hinausragt, wird geclippt.
        // Es wird ein Sicherheitsabstand von Height-Halbe aufaddiert,
        // damit die TTF-"f" nicht im Seitenrand haengen...
        if( bClip &&
            GetInfo().X() + pPor->Width() + ( pPor->Height() / 2 ) > nMaxRight )
        {
            bClip = sal_False;
            rClip.ChgClip( rPaint, pFrm, pCurr->HasUnderscore() );
        }

        // Portions, die "unter" dem Text liegen wie PostIts
        SwLinePortion *pNext = pPor->GetPortion();
        if( GetInfo().OnWin() && pNext && !pNext->Width() )
        {
            // Fix 11289: Felder waren hier ausgeklammert wg. Last!=Owner beim
            // Laden von Brief.sdw. Jetzt sind die Felder wieder zugelassen,
            // durch bSeeked wird Last!=Owner vermieden.
            if ( !bSeeked )
                SeekAndChg( GetInfo() );
            pNext->PrePaint( GetInfo(), pPor );
        }

        // We calculate a separate font for underlining.
        CheckSpecialUnderline( pPor, bAdjustBaseLine ? nOldY : 0 );
        SwUnderlineFont* pUnderLineFnt = GetInfo().GetUnderFnt();
        if ( pUnderLineFnt )
        {
            const Point aTmpPoint( GetInfo().X(),
                                   bAdjustBaseLine ?
                                   pUnderLineFnt->GetPos().Y() :
                                   nLineBaseLine );
            pUnderLineFnt->SetPos( aTmpPoint );
        }


        // in extended input mode we do not want a common underline font.
        SwUnderlineFont* pOldUnderLineFnt = 0;
        if ( GetRedln() && GetRedln()->ExtOn() )
        {
            pOldUnderLineFnt = GetInfo().GetUnderFnt();
            GetInfo().SetUnderFnt( 0 );
        }

        if( pPor->IsMultiPortion() )
            PaintMultiPortion( rPaint, (SwMultiPortion&)*pPor );
        else
            pPor->Paint( GetInfo() );

        // reset underline font
        if ( pOldUnderLineFnt )
            GetInfo().SetUnderFnt( pOldUnderLineFnt );

        // reset (for special vertical alignment)
        GetInfo().Y( nOldY );

        if( GetFnt()->IsURL() && pPor->InTxtGrp() )
            GetInfo().NotifyURL( *pPor );

        bFirst &= !pPor->GetLen();
        if( pNext || !pPor->IsMarginPortion() )
            pPor->Move( GetInfo() );
        if( pPor->IsArrowPortion() && GetInfo().OnWin() && !pArrow )
            pArrow = (SwArrowPortion*)pPor;

        pPor = !bDrawInWindow && GetInfo().X() > nMaxRight ? 0 : pNext;
    }

    // delete underline font
    delete GetInfo().GetUnderFnt();
    GetInfo().SetUnderFnt( 0 );

    // paint remaining stuff
    if( bDrawInWindow )
    {
        // If special vertical alignment is enabled, GetInfo().Y() is the
        // top of the current line. Therefore is has to be adjusted for
        // the painting of the remaining stuff. We first store the old value.
        const SwTwips nOldY = GetInfo().Y();

        if( !GetNextLine() &&
            GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreView() &&
            GetInfo().GetOpt().IsParagraph() && !GetTxtFrm()->GetFollow() &&
            GetInfo().GetIdx() >= GetInfo().GetTxt().Len() )
        {
            const SwTmpEndPortion aEnd( *pEndTempl );
            GetFnt()->ChgPhysFnt( GetInfo().GetVsh(), *pOut );

            if ( bAdjustBaseLine )
                GetInfo().Y( GetInfo().GetPos().Y()
                           + AdjustBaseLine( *pCurr, &aEnd ) );

            aEnd.Paint( GetInfo() );
            GetInfo().Y( nOldY );
        }
        if( GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreView() )
        {
            const sal_Bool bNextUndersized =
                ( GetTxtFrm()->GetNext() &&
                  0 == GetTxtFrm()->GetNext()->Prt().Height() &&
                  GetTxtFrm()->GetNext()->IsTxtFrm() &&
                  ((SwTxtFrm*)GetTxtFrm()->GetNext())->IsUndersized() ) ;

            if( bUnderSz || bNextUndersized )
            {
                if ( bAdjustBaseLine )
                    GetInfo().Y( GetInfo().GetPos().Y() + pCurr->GetAscent() );

                if( pArrow )
                    GetInfo().DrawRedArrow( *pArrow );

                // GetInfo().Y() must be current baseline.
                SwTwips nDiff = GetInfo().Y() + nTmpHeight - nTmpAscent - GetTxtFrm()->Frm().Bottom();
                if( ( nDiff > 0 &&
                      ( GetEnd() < GetInfo().GetTxt().Len() ||
                        ( nDiff > nTmpHeight/2 && GetPrevLine() ) ) ) ||
                    nDiff >= 0 && bNextUndersized )

                {
                    SwArrowPortion aArrow( GetInfo() );
                    GetInfo().DrawRedArrow( aArrow );
                }

                GetInfo().Y( nOldY );
            }
        }
    }

    if( pCurr->IsClipping() )
        rClip.ChgClip( rPaint, pFrm );
}

void SwTxtPainter::CheckSpecialUnderline( const SwLinePortion* pPor,
                                          long nAdjustBaseLine )
{
    // Check if common underline should not be continued.
    if ( IsUnderlineBreak( *pPor, *pFnt ) )
    {
        // delete underline font
        delete GetInfo().GetUnderFnt();
        GetInfo().SetUnderFnt( 0 );
        return;
    }

    // If current underline matches the common underline font, we continue
    // to use the common underline font.
    if ( GetInfo().GetUnderFnt() &&
         GetInfo().GetUnderFnt()->GetFont().GetUnderline() ==
         GetFnt()->GetUnderline() )
         return;

    // calculate the new common underline font
    SwFont* pUnderlineFnt = 0;
    Point aCommonBaseLine;

    Range aRange( 0, GetInfo().GetTxt().Len() );
    MultiSelection aUnderMulti( aRange );

    ASSERT( GetFnt() && UNDERLINE_NONE != GetFnt()->GetUnderline(),
            "CheckSpecialUnderline without underlined font" )
    const SwFont* pParaFnt = GetAttrHandler().GetFont();
    if( pParaFnt && pParaFnt->GetUnderline() == GetFnt()->GetUnderline() )
        aUnderMulti.SelectAll();

    SwTxtAttr* pTxtAttr;
    if( HasHints() )
    {
        sal_Bool bINet = sal_False;
        sal_Bool bUnder = sal_False;
        MSHORT nTmp = 0;

        while( nTmp < pHints->GetStartCount() )
        {
            pTxtAttr = pHints->GetStart( nTmp++ );
            sal_Bool bUnderSelect;
            switch ( pTxtAttr->Which() )
            {
                case RES_CHRATR_UNDERLINE:
                {
                    bUnder = sal_True;
                    bUnderSelect = pFnt->GetUnderline() == pTxtAttr->GetUnderline().
                                                           GetUnderline();
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
                            GetItemState( RES_CHRATR_UNDERLINE, sal_True, &pItem ) )
                        {
                            bUnderSelect = pFnt->GetUnderline() ==
                                 ((SvxUnderlineItem*)pItem)->GetUnderline();
                            bUnder = sal_True;
                        }
                    }
                }
                break;
            }
            if( bUnder )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    if( bUnder )
                        aUnderMulti.Select( aTmp, bUnderSelect );
                }
                bUnder = sal_False;
            }
        }
    }

    MSHORT i;
    xub_StrLen nIndx = GetInfo().GetIdx();
    long nUnderStart = 0;
    long nUnderEnd = 0;
    MSHORT nCnt = (MSHORT)aUnderMulti.GetRangeCount();

    // find the underline range the current portion is contained in
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

    // restrict start and end to current line
    if ( GetStart() > nUnderStart )
        nUnderStart = GetStart();

    if ( GetEnd() && GetEnd() <= nUnderEnd )
        nUnderEnd = GetEnd() - 1;


    // check, if underlining is not isolated
    if ( nIndx + GetInfo().GetLen() < nUnderEnd + 1 )
    {
        //
        // here starts the algorithm for calculating the underline font
        //
        SwScriptInfo& rScriptInfo = GetInfo().GetParaPortion()->GetScriptInfo();
        SwAttrIter aIter( *(SwTxtNode*)GetInfo().GetTxtFrm()->GetTxtNode(),
                          rScriptInfo );

        xub_StrLen nTmpIdx = nIndx;
        ULONG nSumWidth = 0;
        ULONG nSumHeight = 0;
        ULONG nBold = 0;
        const ULONG nPorWidth = pPor->Width();
        USHORT nMaxBaseLineOfst = 0;

        while( nTmpIdx <= nUnderEnd && pPor )
        {
            if ( pPor->IsFlyPortion() || pPor->IsFlyCntPortion() ||
                pPor->IsBreakPortion() || pPor->IsMarginPortion() ||
                pPor->IsHolePortion() ||
                ( pPor->IsMultiPortion() && ! ((SwMultiPortion*)pPor)->IsBidi() ) )
                break;

            aIter.Seek( nTmpIdx );

            if ( aIter.GetFnt()->GetEscapement() < 0 || pFnt->IsWordLineMode() ||
                 SVX_CASEMAP_KAPITAELCHEN == pFnt->GetCaseMap() )
                break;

            if ( ! aIter.GetFnt()->GetEscapement() )
            {
                nSumWidth += pPor->Width();
                const ULONG nFontHeight = aIter.GetFnt()->GetHeight();

                // If we do not have a common baseline we take the baseline
                // and the font of the lowest portion.
                if ( nAdjustBaseLine )
                {
                    USHORT nTmpBaseLineOfst = AdjustBaseLine( *pCurr, pPor );
                    if ( nMaxBaseLineOfst < nTmpBaseLineOfst )
                    {
                        nMaxBaseLineOfst = nTmpBaseLineOfst;
                        nSumHeight = nFontHeight;
                    }
                }
                // in horizontal layout we build a weighted sum of the heights
                else
                    nSumHeight += pPor->Width() * nFontHeight;

                if ( WEIGHT_NORMAL != aIter.GetFnt()->GetWeight() )
                    nBold += pPor->Width();
            }

            nTmpIdx += pPor->GetLen();
            pPor = pPor->GetPortion();
        }

        // resulting height
        if ( nSumWidth && nSumWidth != nPorWidth )
        {
            const ULONG nNewFontHeight = nAdjustBaseLine ?
                                         nSumHeight :
                                         nSumHeight / nSumWidth;

            pUnderlineFnt = new SwFont( *GetInfo().GetFont() );

            // font height
            const BYTE nActual = pUnderlineFnt->GetActual();
            pUnderlineFnt->SetSize( Size( pUnderlineFnt->GetSize( nActual ).Width(),
                                          nNewFontHeight ), nActual );

            // font weight
            if ( 2 * nBold > nSumWidth )
                pUnderlineFnt->SetWeight( WEIGHT_BOLD, nActual );
            else
                pUnderlineFnt->SetWeight( WEIGHT_NORMAL, nActual );

            // common base line
            aCommonBaseLine.Y() = nAdjustBaseLine + nMaxBaseLineOfst;
        }
    }

    // an escaped redlined portion should also have a special underlining
    if( ! pUnderlineFnt && pFnt->GetEscapement() > 0 && GetRedln() &&
        GetRedln()->ChkSpecialUnderline() )
        pUnderlineFnt = new SwFont( *pFnt );

    delete GetInfo().GetUnderFnt();

    if ( pUnderlineFnt )
    {
        pUnderlineFnt->SetProportion( 100 );
        pUnderlineFnt->SetEscapement( 0 );
        pUnderlineFnt->SetStrikeout( STRIKEOUT_NONE );
        const Color aFillColor( COL_TRANSPARENT );
        pUnderlineFnt->SetFillColor( aFillColor );

        GetInfo().SetUnderFnt( new SwUnderlineFont( *pUnderlineFnt,
                                                     aCommonBaseLine ) );
    }
    else
        // I'm sorry, we do not have a special underlining font for you.
        GetInfo().SetUnderFnt( 0 );
}
