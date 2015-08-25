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

#include "hintids.hxx"
#include "flyfrm.hxx"
#include "viewopt.hxx"
#include "txtatr.hxx"
#include <tools/multisel.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/lrspitem.hxx>
#include <txtinet.hxx>
#include <fchrfmt.hxx>
#include <frmatr.hxx>
#include <sfx2/printer.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>

#include <EnhancedPDFExportHelper.hxx>

#include "flyfrms.hxx"
#include "viewsh.hxx"
#include "itrpaint.hxx"
#include "txtfrm.hxx"
#include "txtfly.hxx"
#include "swfont.hxx"
#include "txtpaint.hxx"
#include "portab.hxx"
#include "porfly.hxx"
#include "porfld.hxx"
#include "frmfmt.hxx"
#include "txatbase.hxx"
#include "charfmt.hxx"
#include "redlnitr.hxx"
#include "porrst.hxx"
#include "pormulti.hxx"

// Returns, if we have an underline breaking situation
// Adding some more conditions here means you also have to change them
// in SwTextPainter::CheckSpecialUnderline
bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt )
{
    return UNDERLINE_NONE == rFnt.GetUnderline() ||
           rPor.IsFlyPortion() || rPor.IsFlyCntPortion() ||
           rPor.IsBreakPortion() || rPor.IsMarginPortion() ||
           rPor.IsHolePortion() ||
          ( rPor.IsMultiPortion() && ! static_cast<const SwMultiPortion&>(rPor).IsBidi() ) ||
           rFnt.GetEscapement() < 0 || rFnt.IsWordLineMode() ||
           SVX_CASEMAP_KAPITAELCHEN == rFnt.GetCaseMap();
}

void SwTextPainter::CtorInitTextPainter( SwTextFrm *pNewFrm, SwTextPaintInfo *pNewInf )
{
    CtorInitTextCursor( pNewFrm, pNewInf );
    pInf = pNewInf;
    SwFont *pMyFnt = GetFnt();
    GetInfo().SetFont( pMyFnt );
#if OSL_DEBUG_LEVEL > 1
    if( ALIGN_BASELINE != pMyFnt->GetAlign() )
    {
        OSL_ENSURE( ALIGN_BASELINE == pMyFnt->GetAlign(),
                "+SwTextPainter::CTOR: font alignment revolution" );
        pMyFnt->SetAlign( ALIGN_BASELINE );
    }
#endif
    bPaintDrop = false;
}

SwLinePortion *SwTextPainter::CalcPaintOfst( const SwRect &rPaint )
{
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    GetInfo().SetPaintOfst( 0 );
    SwTwips nPaintOfst = rPaint.Left();

    // nPaintOfst wurde exakt auf das Ende eingestellt, deswegen <=
    // nPaintOfst ist dokumentglobal, deswegen nLeftMar aufaddieren
    // const sal_uInt16 nLeftMar = sal_uInt16(GetLeftMargin());
    // 8310: painten von LineBreaks in leeren Zeilen.
    if( nPaintOfst && pCurr->Width() )
    {
        SwLinePortion *pLast = 0;
        // 7529 und 4757: nicht <= nPaintOfst
        while( pPor && GetInfo().X() + pPor->Width() + (pPor->Height()/2)
                       < nPaintOfst )
        {
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

        // 7529: bei PostIts auch pLast returnen.
        if( pLast && !pLast->Width() && pLast->IsPostItsPortion() )
        {
            pPor = pLast;
            GetInfo().SetIdx( GetInfo().GetIdx() - pPor->GetLen() );
        }
    }
    return pPor;
}

// Es gibt zwei Moeglichkeiten bei transparenten Font auszugeben:
// 1) DrawRect auf die ganze Zeile und die DrawText hinterher
//    (objektiv schnell, subjektiv langsam).
// 2) Fuer jede Portion ein DrawRect mit anschliessendem DrawText
//    ausgefuehrt (objektiv langsam, subjektiv schnell).
// Da der User in der Regel subjektiv urteilt, wird die 2. Methode
// als Default eingestellt.
void SwTextPainter::DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                                 const bool bUnderSz )
{
#if OSL_DEBUG_LEVEL > 1
//    sal_uInt16 nFntHeight = GetInfo().GetFont()->GetHeight( GetInfo().GetVsh(), GetInfo().GetOut() );
//    sal_uInt16 nFntAscent = GetInfo().GetFont()->GetAscent( GetInfo().GetVsh(), GetInfo().GetOut() );
#endif

    // Adjustierung ggf. nachholen
    GetAdjusted();
    GetInfo().SetpSpaceAdd( pCurr->GetpLLSpaceAdd() );
    GetInfo().ResetSpaceIdx();
    GetInfo().SetKanaComp( pCurr->GetpKanaComp() );
    GetInfo().ResetKanaIdx();
    // The size of the frame
    GetInfo().SetIdx( GetStart() );
    GetInfo().SetPos( GetTopLeft() );

    const bool bDrawInWindow = GetInfo().OnWin();

    // 6882: Leerzeilen duerfen nicht wegoptimiert werden bei Paragraphzeichen.
    const bool bEndPor = GetInfo().GetOpt().IsParagraph() && GetInfo().GetText().isEmpty();

    SwLinePortion *pPor = bEndPor ? pCurr->GetFirstPortion() : CalcPaintOfst( rPaint );

    // Optimization!
    const SwTwips nMaxRight = std::min( rPaint.Right(), Right() );
    const SwTwips nTmpLeft = GetInfo().X();
    if( !bEndPor && nTmpLeft >= nMaxRight )
        return;

    // DropCaps!
    // 7538: of course for the printer, too
    if( !bPaintDrop )
    {
        // 8084: Optimization, less painting
        // AMA: By 8084 7538 has been revived
        // bDrawInWindow entfernt, damit DropCaps auch gedruckt werden
        bPaintDrop = pPor == pCurr->GetFirstPortion()
                     && GetDropLines() >= GetLineNr();
    }

    sal_uInt16 nTmpHeight, nTmpAscent;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );

    // bClip decides if there's a need to clip
    // Das Ganze muss vor der Retusche stehen

    bool bClip = ( bDrawInWindow || bUnderSz ) && !rClip.IsChg();
    if( bClip && pPor )
    {
        // Wenn TopLeft oder BottomLeft der Line ausserhalb liegen,
        // muss geclippt werden. Die Ueberpruefung auf Right() erfolgt
        // in der folgenden Ausgabeschleife...

        if( GetInfo().GetPos().X() < rPaint.Left() ||
            GetInfo().GetPos().Y() < rPaint.Top() ||
            GetInfo().GetPos().Y() + nTmpHeight > rPaint.Top() + rPaint.Height() )
        {
            bClip = false;
            rClip.ChgClip( rPaint, pFrm, pCurr->HasUnderscore() );
        }
#if OSL_DEBUG_LEVEL > 1
        static bool bClipAlways = false;
        if( bClip && bClipAlways )
        {   bClip = false;
            rClip.ChgClip( rPaint );
        }
#endif
    }

    // Alignment
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
        aPnt2.Y() = rPaint.Bottom();

    const SwRect aLineRect( aPnt1, aPnt2 );

    if( pCurr->IsClipping() )
    {
        rClip.ChgClip( aLineRect, pFrm );
        bClip = false;
    }

    if( !pPor && !bEndPor )
        return;

    // Baseline-Ausgabe auch bei nicht-TextPortions (vgl. TabPor mit Fill)
    // if no special vertical alignment is used,
    // we calculate Y value for the whole line
    SwTextGridItem const*const pGrid(GetGridItem(GetTextFrm()->FindPageFrm()));
    const bool bAdjustBaseLine =
        ( !GetTextFrm()->IsVertical() || GetLineInfo().HasSpecialAlign( true ) ) && (! GetTextFrm()->IsInFly()) &&
        ( 0 != pGrid );
    const SwTwips nLineBaseLine = GetInfo().GetPos().Y() + nTmpAscent;
    if ( ! bAdjustBaseLine )
        GetInfo().Y( nLineBaseLine );

    // 7529: Pre-paint post-its
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

    // 7923: EndPortions output chars, too, that's why we change the font
    if( bEndPor )
        SeekStartAndChg( GetInfo() );

    const bool bRest = pCurr->IsRest();
    bool bFirst = true;

    SwArrowPortion *pArrow = NULL;
    // Reference portion for the paragraph end portion
    SwLinePortion* pEndTempl = pCurr->GetFirstPortion();

    while( pPor )
    {
        bool bSeeked = true;
        GetInfo().SetLen( pPor->GetLen() );

        const SwTwips nOldY = GetInfo().Y();

        if ( bAdjustBaseLine )
        {
            GetInfo().Y( GetInfo().GetPos().Y() + AdjustBaseLine( *pCurr, pPor ) );

            // we store the last portion, because a possible paragraph
            // end character has the same font as this portion
            // (only in special vertical alignment case, otherwise the first
            // portion of the line is used)
            if ( pPor->Width() && pPor->InTextGrp() )
                pEndTempl = pPor;
        }

        // Ein Sonderfall sind GluePortions, die Blanks ausgeben.

        // 6168: Der Rest einer FieldPortion zog sich die Attribute der naechsten
        // Portion an, dies wird durch SeekAndChgBefore vermieden:
        if( ( bRest && pPor->InFieldGrp() && !pPor->GetLen() ) )
            SeekAndChgBefore( GetInfo() );
        else if ( pPor->IsQuoVadisPortion() )
        {
            sal_Int32 nOffset = GetInfo().GetIdx();
            SeekStartAndChg( GetInfo(), true );
            if( GetRedln() && pCurr->HasRedline() )
                GetRedln()->Seek( *pFnt, nOffset, 0 );
        }
        else if( pPor->InTextGrp() || pPor->InFieldGrp() || pPor->InTabGrp() )
            SeekAndChg( GetInfo() );
        else if ( !bFirst && pPor->IsBreakPortion() && GetInfo().GetOpt().IsParagraph() )
        {
            // Paragraph symbols should have the same fon as the paragraph in fron of them,
            // except for the case that there's redlining in the paragraph
            if( GetRedln() )
                SeekAndChg( GetInfo() );
            else
                SeekAndChgBefore( GetInfo() );
        }
        else
            bSeeked = false;

        // bRest = false;

        // Wenn das Ende der Portion hinausragt, wird geclippt.
        // Es wird ein Sicherheitsabstand von Height-Halbe aufaddiert,
        // damit die TTF-"f" nicht im Seitenrand haengen...
        if( bClip &&
            GetInfo().X() + pPor->Width() + ( pPor->Height() / 2 ) > nMaxRight )
        {
            bClip = false;
            rClip.ChgClip( rPaint, pFrm, pCurr->HasUnderscore() );
        }

        // Portions, which lay "below" the text like post-its
        SwLinePortion *pNext = pPor->GetPortion();
        if( GetInfo().OnWin() && pNext && !pNext->Width() )
        {
            // Fix 11289: Fields were omitted here because of Last!=Owner during
            // loading Brief.sdw. Now the fields are allowed again,
            // by bSeeked Last!=Owner is being avoided.
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

        {
            // #i16816# tagged pdf support
            Por_Info aPorInfo( *pPor, *this );
            SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, &aPorInfo, *pOut );

            if( pPor->IsMultiPortion() )
                PaintMultiPortion( rPaint, static_cast<SwMultiPortion&>(*pPor) );
            else
                pPor->Paint( GetInfo() );
        }

        // reset underline font
        if ( pOldUnderLineFnt )
            GetInfo().SetUnderFnt( pOldUnderLineFnt );

        // reset (for special vertical alignment)
        GetInfo().Y( nOldY );

        if( GetFnt()->IsURL() && pPor->InTextGrp() )
            GetInfo().NotifyURL( *pPor );

        bFirst &= !pPor->GetLen();
        if( pNext || !pPor->IsMarginPortion() )
            pPor->Move( GetInfo() );
        if( pPor->IsArrowPortion() && GetInfo().OnWin() && !pArrow )
            pArrow = static_cast<SwArrowPortion*>(pPor);

        pPor = bDrawInWindow || GetInfo().X() <= nMaxRight ||
               // #i16816# tagged pdf support
               ( GetInfo().GetVsh() &&
                 GetInfo().GetVsh()->GetViewOptions()->IsPDFExport() &&
                 pNext && pNext->IsHolePortion() ) ?
               pNext :
               0;
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
            GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreview() &&
            GetInfo().GetOpt().IsParagraph() && !GetTextFrm()->GetFollow() &&
            GetInfo().GetIdx() >= GetInfo().GetText().getLength() )
        {
            const SwTmpEndPortion aEnd( *pEndTempl );
            GetFnt()->ChgPhysFnt( GetInfo().GetVsh(), *pOut );

            if ( bAdjustBaseLine )
                GetInfo().Y( GetInfo().GetPos().Y()
                           + AdjustBaseLine( *pCurr, &aEnd ) );

            aEnd.Paint( GetInfo() );
            GetInfo().Y( nOldY );
        }
        if( GetInfo().GetVsh() && !GetInfo().GetVsh()->IsPreview() )
        {
            const bool bNextUndersized =
                ( GetTextFrm()->GetNext() &&
                  0 == GetTextFrm()->GetNext()->Prt().Height() &&
                  GetTextFrm()->GetNext()->IsTextFrm() &&
                  static_cast<SwTextFrm*>(GetTextFrm()->GetNext())->IsUndersized() ) ;

            if( bUnderSz || bNextUndersized )
            {
                if ( bAdjustBaseLine )
                    GetInfo().Y( GetInfo().GetPos().Y() + pCurr->GetAscent() );

                if( pArrow )
                    GetInfo().DrawRedArrow( *pArrow );

                // GetInfo().Y() must be current baseline
                SwTwips nDiff = GetInfo().Y() + nTmpHeight - nTmpAscent - GetTextFrm()->Frm().Bottom();
                if( ( nDiff > 0 &&
                      ( GetEnd() < GetInfo().GetText().getLength() ||
                        ( nDiff > nTmpHeight/2 && GetPrevLine() ) ) ) ||
                    (nDiff >= 0 && bNextUndersized) )

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

void SwTextPainter::CheckSpecialUnderline( const SwLinePortion* pPor,
                                          long nAdjustBaseLine )
{
    // Check if common underline should not be continued
    if ( IsUnderlineBreak( *pPor, *pFnt ) )
    {
        // delete underline font
        delete GetInfo().GetUnderFnt();
        GetInfo().SetUnderFnt( 0 );
        return;
    }

    // If current underline matches the common underline font, we continue
    // to use the common underline font.
    // Bug 120769:Color of underline display wrongly
    if ( GetInfo().GetUnderFnt() &&
        GetInfo().GetUnderFnt()->GetFont().GetUnderline() == GetFnt()->GetUnderline() &&
        GetInfo().GetFont() && GetInfo().GetFont()->GetUnderColor() != Color(COL_AUTO) )
        return;
    //Bug 120769(End)

    OSL_ENSURE( GetFnt() && UNDERLINE_NONE != GetFnt()->GetUnderline(),
            "CheckSpecialUnderline without underlined font" );
    MultiSelection aUnderMulti( Range( 0, GetInfo().GetText().getLength() ) );
    const SwFont* pParaFnt = GetAttrHandler().GetFont();
    if( pParaFnt && pParaFnt->GetUnderline() == GetFnt()->GetUnderline() )
        aUnderMulti.SelectAll();

    if( HasHints() )
    {
        for ( size_t nTmp = 0; nTmp < pHints->GetStartCount(); ++nTmp )
        {
            SwTextAttr* const pTextAttr = pHints->GetStart( nTmp );

            const SvxUnderlineItem* pItem =
                    static_cast<const SvxUnderlineItem*>(CharFormat::GetItem( *pTextAttr, RES_CHRATR_UNDERLINE ));

            if ( pItem )
            {
                const sal_Int32 nSt = pTextAttr->GetStart();
                const sal_Int32 nEnd = *pTextAttr->GetEnd();
                if( nEnd > nSt )
                {
                    const bool bUnderSelect = pFnt->GetUnderline() == pItem->GetLineStyle();
                    aUnderMulti.Select( Range( nSt, nEnd - 1 ), bUnderSelect );
                }
            }
        }
    }

    const sal_Int32 nIndx = GetInfo().GetIdx();
    long nUnderEnd = 0;
    const size_t nCnt = aUnderMulti.GetRangeCount();

    // find the underline range the current portion is contained in
    for( size_t i = 0; i < nCnt; ++i )
    {
        const Range& rRange = aUnderMulti.GetRange( i );
        if( nUnderEnd == rRange.Min() )
            nUnderEnd = rRange.Max();
        else if( nIndx >= rRange.Min() )
        {
            nUnderEnd = rRange.Max();
        }
        else
            break;
    }

    if ( GetEnd() && GetEnd() <= nUnderEnd )
        nUnderEnd = GetEnd() - 1;

    // calculate the new common underline font
    SwFont* pUnderlineFnt = 0;
    Point aCommonBaseLine;

    // check, if underlining is not isolated
    if ( nIndx + GetInfo().GetLen() < nUnderEnd + 1 )
    {
        // here starts the algorithm for calculating the underline font
        SwScriptInfo& rScriptInfo = GetInfo().GetParaPortion()->GetScriptInfo();
        SwAttrIter aIter( *GetInfo().GetTextFrm()->GetTextNode(),
                          rScriptInfo );

        sal_Int32 nTmpIdx = nIndx;
        sal_uLong nSumWidth = 0;
        sal_uLong nSumHeight = 0;
        sal_uLong nBold = 0;
        sal_uInt16 nMaxBaseLineOfst = 0;
        int nNumberOfPortions = 0;

        while( nTmpIdx <= nUnderEnd && pPor )
        {
            if ( pPor->IsFlyPortion() || pPor->IsFlyCntPortion() ||
                pPor->IsBreakPortion() || pPor->IsMarginPortion() ||
                pPor->IsHolePortion() ||
                ( pPor->IsMultiPortion() && ! static_cast<const SwMultiPortion*>(pPor)->IsBidi() ) )
                break;

            aIter.Seek( nTmpIdx );

            if ( aIter.GetFnt()->GetEscapement() < 0 || pFnt->IsWordLineMode() ||
                 SVX_CASEMAP_KAPITAELCHEN == pFnt->GetCaseMap() )
                break;

            if ( !aIter.GetFnt()->GetEscapement() )
            {
                nSumWidth += pPor->Width();
                const sal_uLong nFontHeight = aIter.GetFnt()->GetHeight();

                // If we do not have a common baseline we take the baseline
                // and the font of the lowest portion.
                if ( nAdjustBaseLine )
                {
                    const sal_uInt16 nTmpBaseLineOfst = AdjustBaseLine( *pCurr, pPor );
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

            ++nNumberOfPortions;

            nTmpIdx += pPor->GetLen();
            pPor = pPor->GetPortion();
        }

        // resulting height
        if ( nNumberOfPortions > 1 && nSumWidth )
        {
            const sal_uLong nNewFontHeight = nAdjustBaseLine ?
                                         nSumHeight :
                                         nSumHeight / nSumWidth;

            pUnderlineFnt = new SwFont( *GetInfo().GetFont() );

            // font height
            const sal_uInt8 nActual = pUnderlineFnt->GetActual();
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
        pUnderlineFnt->SetOverline( UNDERLINE_NONE );
        const Color aFillColor( COL_TRANSPARENT );
        pUnderlineFnt->SetFillColor( aFillColor );

        GetInfo().SetUnderFnt( new SwUnderlineFont( *pUnderlineFnt,
                                                     aCommonBaseLine ) );
    }
    else
        // I'm sorry, we do not have a special underlining font for you.
        GetInfo().SetUnderFnt( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
