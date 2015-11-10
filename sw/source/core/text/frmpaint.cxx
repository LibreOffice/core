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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <hintids.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/lrspitem.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <paratr.hxx>

#include <fmtline.hxx>
#include <lineinfo.hxx>
#include <charfmt.hxx>
#include "rootfrm.hxx"
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <txtfrm.hxx>
#include <itrpaint.hxx>
#include <txtpaint.hxx>
#include <txtcache.hxx>
#include <flyfrm.hxx>
#include <redlnitr.hxx>
#include <swmodule.hxx>
#include <tabfrm.hxx>
#include <numrule.hxx>
#include <SwGrammarMarkUp.hxx>

#include <EnhancedPDFExportHelper.hxx>

#include <IDocumentStylePoolAccess.hxx>

#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

using namespace ::com::sun::star;

static bool bInitFont = true;

class SwExtraPainter
{
    SwSaveClip aClip;
    SwRect aRect;
    const SwTextFrm* pTextFrm;
    SwViewShell *pSh;
    SwFont* pFnt;
    const SwLineNumberInfo &rLineInf;
    SwTwips nX;
    SwTwips nRedX;
    sal_uLong nLineNr;
    sal_uInt16 nDivider;
    bool bGoLeft;
    bool bLineNum;
    inline bool IsClipChg() { return aClip.IsChg(); }
public:
    SwExtraPainter( const SwTextFrm *pFrm, SwViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct,
        sal_Int16 eHor, bool bLnNm );
    ~SwExtraPainter() { delete pFnt; }
    inline SwFont* GetFont() const { return pFnt; }
    inline void IncLineNr() { ++nLineNr; }
    inline bool HasNumber() { return !( nLineNr % rLineInf.GetCountBy() ); }
    inline bool HasDivider() { if( !nDivider ) return false;
        return !(nLineNr % rLineInf.GetDividerCountBy()); }

    void PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed );
    void PaintRedline( SwTwips nY, long nMax );
};

SwExtraPainter::SwExtraPainter( const SwTextFrm *pFrm, SwViewShell *pVwSh,
                                const SwLineNumberInfo &rLnInf, const SwRect &rRct,
                                sal_Int16 eHor, bool bLnNm )
    : aClip( pVwSh->GetWin() || pFrm->IsUndersized() ? pVwSh->GetOut() : nullptr )
    , aRect( rRct )
    , pTextFrm( pFrm )
    , pSh( pVwSh )
    , pFnt( nullptr )
    , rLineInf( rLnInf )
    , nX(0)
    , nRedX(0)
    , nLineNr( 1L )
    , nDivider(0)
    , bGoLeft(false)
    , bLineNum( bLnNm )
{
    if( pFrm->IsUndersized() )
    {
        SwTwips nBottom = pFrm->Frm().Bottom();
        if( aRect.Bottom() > nBottom )
            aRect.Bottom( nBottom );
    }
    int nVirtPageNum = 0;
    if( bLineNum )
    {
        /* Initializes the Members necessary for line numbering:

            nDivider,   how often do we want a substring; 0 == never
            nX,         line number's x position
            pFnt,       line number's font
            nLineNr,    the first line number
            bLineNum is set back to false if the numbering is completely
            outside of the paint rect
        */
        nDivider = !rLineInf.GetDivider().isEmpty() ? rLineInf.GetDividerCountBy() : 0;
        nX = pFrm->Frm().Left();
        SwCharFormat* pFormat = rLineInf.GetCharFormat( const_cast<IDocumentStylePoolAccess&>(pFrm->GetNode()->getIDocumentStylePoolAccess()) );
        OSL_ENSURE( pFormat, "PaintExtraData without CharFormat" );
        pFnt = new SwFont( &pFormat->GetAttrSet(), pFrm->GetTextNode()->getIDocumentSettingAccess() );
        pFnt->Invalidate();
        pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
        pFnt->SetVertical( 0, pFrm->IsVertical() );
        nLineNr += pFrm->GetAllLines() - pFrm->GetThisLines();
        LineNumberPosition ePos = rLineInf.GetPos();
        if( ePos != LINENUMBER_POS_LEFT && ePos != LINENUMBER_POS_RIGHT )
        {
            if( pFrm->FindPageFrm()->OnRightPage() )
            {
                nVirtPageNum = 1;
                ePos = ePos == LINENUMBER_POS_INSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            }
            else
            {
                nVirtPageNum = 2;
                ePos = ePos == LINENUMBER_POS_OUTSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            }
        }
        if( LINENUMBER_POS_LEFT == ePos )
        {
            bGoLeft = true;
            nX -= rLineInf.GetPosFromLeft();
            if( nX < aRect.Left() )
                bLineNum = false;
        }
        else
        {
            bGoLeft = false;
            nX += pFrm->Frm().Width() + rLineInf.GetPosFromLeft();
            if( nX > aRect.Right() )
                bLineNum = false;
        }
    }
    if( eHor != text::HoriOrientation::NONE )
    {
        if( text::HoriOrientation::INSIDE == eHor || text::HoriOrientation::OUTSIDE == eHor )
        {
            if( !nVirtPageNum )
                nVirtPageNum = pFrm->FindPageFrm()->OnRightPage() ? 1 : 2;
            if( nVirtPageNum % 2 )
                eHor = eHor == text::HoriOrientation::INSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
            else
                eHor = eHor == text::HoriOrientation::OUTSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
        }
        const SwFrm* pTmpFrm = pFrm->FindTabFrm();
        if( !pTmpFrm )
            pTmpFrm = pFrm;
        nRedX = text::HoriOrientation::LEFT == eHor ? pTmpFrm->Frm().Left() - REDLINE_DISTANCE :
            pTmpFrm->Frm().Right() + REDLINE_DISTANCE;
    }
}

void SwExtraPainter::PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed )
{
  // Line number is stronger than the divider
    const OUString aTmp( HasNumber() ? rLineInf.GetNumType().GetNumStr( nLineNr )
                                : rLineInf.GetDivider() );

    // Get script type of line numbering:
    pFnt->SetActual( SwScriptInfo::WhichFont( 0, &aTmp, nullptr ) );

    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), nullptr, aTmp, 0, aTmp.getLength() );
    aDrawInf.SetSpace( 0 );
    aDrawInf.SetWrong( nullptr );
    aDrawInf.SetGrammarCheck( nullptr );
    aDrawInf.SetSmartTags( nullptr );
    aDrawInf.SetLeft( 0 );
    aDrawInf.SetRight( LONG_MAX );
    aDrawInf.SetFrm( pTextFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( false );
    aDrawInf.SetIgnoreFrmRTL( true );

    bool bTooBig = pFnt->GetSize( pFnt->GetActual() ).Height() > nMax &&
                pFnt->GetHeight( pSh, *pSh->GetOut() ) > nMax;
    SwFont* pTmpFnt;
    if( bTooBig )
    {
        pTmpFnt = new SwFont( *GetFont() );
        if( nMax >= 20 )
        {
            nMax *= 17;
            nMax /= 20;
        }
        pTmpFnt->SetSize( Size( 0, nMax ), pTmpFnt->GetActual() );
    }
    else
        pTmpFnt = GetFont();
    Point aTmpPos( nX, nY );
    aTmpPos.Y() += nAsc;
    bool bPaint = true;
    if( !IsClipChg() )
    {
        Size aSize = pTmpFnt->_GetTextSize( aDrawInf );
        if( bGoLeft )
            aTmpPos.X() -= aSize.Width();
        // calculate rectangle containing the line number
        SwRect aRct( Point( aTmpPos.X(),
                         aTmpPos.Y() - pTmpFnt->GetAscent( pSh, *pSh->GetOut() )
                          ), aSize );
        if( !aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( aRect ).IsEmpty() )
                bPaint = false;
            else
                aClip.ChgClip( aRect, pTextFrm );
        }
    }
    else if( bGoLeft )
        aTmpPos.X() -= pTmpFnt->_GetTextSize( aDrawInf ).Width();
    aDrawInf.SetPos( aTmpPos );
    if( bPaint )
        pTmpFnt->_DrawText( aDrawInf );

    if( bTooBig )
        delete pTmpFnt;
    if( bRed )
    {
        long nDiff = bGoLeft ? nRedX - nX : nX - nRedX;
        if( nDiff > REDLINE_MINDIST )
            PaintRedline( nY, nMax );
    }
}

void SwExtraPainter::PaintRedline( SwTwips nY, long nMax )
{
    Point aStart( nRedX, nY );
    Point aEnd( nRedX, nY + nMax );

    if( !IsClipChg() )
    {
        SwRect aRct( aStart, aEnd );
        if( !aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( aRect ).IsEmpty() )
                return;
            aClip.ChgClip( aRect, pTextFrm );
        }
    }
    const Color aOldCol( pSh->GetOut()->GetLineColor() );
    pSh->GetOut()->SetLineColor( SW_MOD()->GetRedlineMarkColor() );

    if ( pTextFrm->IsVertical() )
    {
        pTextFrm->SwitchHorizontalToVertical( aStart );
        pTextFrm->SwitchHorizontalToVertical( aEnd );
    }

    pSh->GetOut()->DrawLine( aStart, aEnd );
    pSh->GetOut()->SetLineColor( aOldCol );
}

void SwTextFrm::PaintExtraData( const SwRect &rRect ) const
{
    if( Frm().Top() > rRect.Bottom() || Frm().Bottom() < rRect.Top() )
        return;

    const SwTextNode& rTextNode = *GetTextNode();
    const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();
    const SwLineNumberInfo &rLineInf = rTextNode.GetDoc()->GetLineNumberInfo();
    const SwFormatLineNumber &rLineNum = GetAttrSet()->GetLineNumber();
    bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    sal_Int16 eHor = (sal_Int16)SW_MOD()->GetRedlineMarkPos();
    if( eHor != text::HoriOrientation::NONE && !IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
        eHor = text::HoriOrientation::NONE;
    bool bRedLine = eHor != text::HoriOrientation::NONE;
    if ( bLineNum || bRedLine )
    {
        if( IsLocked() || IsHiddenNow() || !Prt().Height() )
            return;
        SwViewShell *pSh = getRootFrm()->GetCurrShell();

        SwSwapIfNotSwapped swap(const_cast<SwTextFrm *>(this));
        SwRect rOldRect( rRect );

        if ( IsVertical() )
            SwitchVerticalToHorizontal( (SwRect&)rRect );

        SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
        aLayoutModeModifier.Modify( false );

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pSh->GetOut() );

        SwExtraPainter aExtra( this, pSh, rLineInf, rRect, eHor, bLineNum );

        if( HasPara() )
        {
            TextFrmLockGuard aLock(const_cast<SwTextFrm*>(this));

            SwTextLineAccess aAccess( this );
            aAccess.GetPara();

            SwTextPaintInfo aInf( const_cast<SwTextFrm*>(this), rRect );

            aLayoutModeModifier.Modify( false );

            SwTextPainter  aLine( const_cast<SwTextFrm*>(this), &aInf );
            bool bNoDummy = !aLine.GetNext(); // Only one empty line!

            while( aLine.Y() + aLine.GetLineHeight() <= rRect.Top() )
            {
                if( !aLine.GetCurr()->IsDummy() &&
                    ( rLineInf.IsCountBlankLines() ||
                      aLine.GetCurr()->HasContent() ) )
                    aExtra.IncLineNr();
                if( !aLine.Next() )
                {
                    (SwRect&)rRect = rOldRect;
                    return;
                }
            }

            long nBottom = rRect.Bottom();

            bool bNoPrtLine = 0 == GetMinPrtLine();
            if( !bNoPrtLine )
            {
                while ( aLine.Y() < GetMinPrtLine() )
                {
                    if( ( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasContent() )
                        && !aLine.GetCurr()->IsDummy() )
                        aExtra.IncLineNr();
                    if( !aLine.Next() )
                        break;
                }
                bNoPrtLine = aLine.Y() >= GetMinPrtLine();
            }
            if( bNoPrtLine )
            {
                do
                {
                    if( bNoDummy || !aLine.GetCurr()->IsDummy() )
                    {
                        bool bRed = bRedLine && aLine.GetCurr()->HasRedline();
                        if( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasContent() )
                        {
                            if( bLineNum &&
                                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
                            {
                                sal_uInt16 nTmpHeight, nTmpAscent;
                                aLine.CalcAscentAndHeight( nTmpAscent, nTmpHeight );
                                aExtra.PaintExtra( aLine.Y(), nTmpAscent,
                                    nTmpHeight, bRed );
                                bRed = false;
                            }
                            aExtra.IncLineNr();
                        }
                        if( bRed )
                            aExtra.PaintRedline( aLine.Y(), aLine.GetLineHeight() );
                    }
                } while( aLine.Next() && aLine.Y() <= nBottom );
            }
        }
        else
        {
            if ( USHRT_MAX == rIDRA.GetRedlinePos(rTextNode, USHRT_MAX) )
                bRedLine = false;

            if( bLineNum && rLineInf.IsCountBlankLines() &&
                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
            {
                aExtra.PaintExtra( Frm().Top()+Prt().Top(), aExtra.GetFont()
                    ->GetAscent( pSh, *pSh->GetOut() ), Prt().Height(), bRedLine );
            }
            else if( bRedLine )
                aExtra.PaintRedline( Frm().Top()+Prt().Top(), Prt().Height() );
        }

        (SwRect&)rRect = rOldRect;
    }
}

SwRect SwTextFrm::Paint()
{
#if OSL_DEBUG_LEVEL > 1
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
#endif

    // finger layout
    OSL_ENSURE( GetValidPosFlag(), "+SwTextFrm::Paint: no Calc()" );

    SwRect aRet( Prt() );
    if ( IsEmpty() || !HasPara() )
        aRet += Frm().Pos();
    else
    {
        // We return the right paint rect. Use the calculated PaintOfst as the
        // left margin
        SwRepaint& rRepaint = GetPara()->GetRepaint();
        long l;

        if ( IsVertLR() ) // mba: the following line was added, but we don't need it for the existing directions; kept for IsVertLR(), but should be checked
            rRepaint.Chg( ( GetUpper()->Frm() ).Pos() + ( GetUpper()->Prt() ).Pos(), ( GetUpper()->Prt() ).SSize() );

        if( rRepaint.GetOfst() )
            rRepaint.Left( rRepaint.GetOfst() );

        l = rRepaint.GetRightOfst();
        if( l && l > rRepaint.Right() )
             rRepaint.Right( l );
        rRepaint.SetOfst( 0 );
        aRet = rRepaint;

        // In case our left edge is the same as the body frame's left edge,
        // then extend the rectangle to include the page margin as well,
        // otherwise some font will be clipped.
        SwLayoutFrm* pBodyFrm = GetUpper();
        if (pBodyFrm->IsBodyFrm() && aRet.Left() == (pBodyFrm->Frm().Left() + pBodyFrm->Prt().Left()))
            if (SwLayoutFrm* pPageFrm = pBodyFrm->GetUpper())
                aRet.Left(pPageFrm->Frm().Left());

        if ( IsRightToLeft() )
            SwitchLTRtoRTL( aRet );

        if ( IsVertical() )
            SwitchHorizontalToVertical( aRet );
    }
    ResetRepaint();

    return aRet;
}

bool SwTextFrm::PaintEmpty( const SwRect &rRect, bool bCheck ) const
{
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && ( pSh->GetViewOptions()->IsParagraph() || bInitFont ) )
    {
        bInitFont = false;
        SwTextFly aTextFly( this );
        aTextFly.SetTopRule();
        SwRect aRect;
        if( bCheck && aTextFly.IsOn() && aTextFly.IsAnyObj( aRect ) )
            return false;
        else if( pSh->GetWin() )
        {
            SwFont *pFnt;
            const SwTextNode& rTextNode = *GetTextNode();
            if ( rTextNode.HasSwAttrSet() )
            {
                const SwAttrSet *pAttrSet = &( rTextNode.GetSwAttrSet() );
                pFnt = new SwFont( pAttrSet, rTextNode.getIDocumentSettingAccess() );
            }
            else
            {
                SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), pSh );
                pFnt = new SwFont( aFontAccess.Get()->GetFont() );
            }

            const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();
            if( IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
            {
                const sal_uInt16 nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
                if( USHRT_MAX != nRedlPos )
                {
                    SwAttrHandler aAttrHandler;
                    aAttrHandler.Init(  rTextNode.GetSwAttrSet(),
                                       *rTextNode.getIDocumentSettingAccess(), nullptr );
                    SwRedlineItr aRedln( rTextNode, *pFnt, aAttrHandler, nRedlPos, true );
                }
            }

            if( pSh->GetViewOptions()->IsParagraph() && Prt().Height() )
            {
                if( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet( SW_LATIN ) &&
                    pFnt->GetName( SW_LATIN ) != numfunc::GetDefBulletFontname() )
                {
                    pFnt->SetFamily( FAMILY_DONTKNOW, SW_LATIN );
                    pFnt->SetName( numfunc::GetDefBulletFontname(), SW_LATIN );
                    pFnt->SetStyleName( aEmptyOUStr, SW_LATIN );
                    pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, SW_LATIN );
                }
                pFnt->SetVertical( 0, IsVertical() );
                SwFrmSwapper aSwapper( this, true );
                SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
                aLayoutModeModifier.Modify( IsRightToLeft() );

                pFnt->Invalidate();
                pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
                Point aPos = Frm().Pos() + Prt().Pos();

                const SvxLRSpaceItem &rSpace =
                    GetTextNode()->GetSwAttrSet().GetLRSpace();

                if ( rSpace.GetTextFirstLineOfst() > 0 )
                    aPos.X() += rSpace.GetTextFirstLineOfst();

                SwSaveClip *pClip;
                if( IsUndersized() )
                {
                    pClip = new SwSaveClip( pSh->GetOut() );
                    pClip->ChgClip( rRect );
                }
                else
                    pClip = nullptr;

                aPos.Y() += pFnt->GetAscent( pSh, *pSh->GetOut() );

                if ( GetTextNode()->GetSwAttrSet().GetParaGrid().GetValue() &&
                     IsInDocBody() )
                {
                    SwTextGridItem const*const pGrid(GetGridItem(FindPageFrm()));
                    if ( pGrid )
                    {
                        // center character in grid line
                        aPos.Y() += ( pGrid->GetBaseHeight() -
                                      pFnt->GetHeight( pSh, *pSh->GetOut() ) ) / 2;

                        if ( ! pGrid->GetRubyTextBelow() )
                            aPos.Y() += pGrid->GetRubyHeight();
                    }
                }

                // Don't show the paragraph mark for collapsed paragraphs, when they are hidden
                if ( EmptyHeight( ) > 1 )
                {
                    const OUString aTmp( CH_PAR );
                    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), nullptr, aTmp, 0, 1 );
                    aDrawInf.SetLeft( rRect.Left() );
                    aDrawInf.SetRight( rRect.Right() );
                    aDrawInf.SetPos( aPos );
                    aDrawInf.SetSpace( 0 );
                    aDrawInf.SetKanaComp( 0 );
                    aDrawInf.SetWrong( nullptr );
                    aDrawInf.SetGrammarCheck( nullptr );
                    aDrawInf.SetSmartTags( nullptr );
                    aDrawInf.SetFrm( this );
                    aDrawInf.SetFont( pFnt );
                    aDrawInf.SetSnapToGrid( false );

                    pFnt->SetColor(NON_PRINTING_CHARACTER_COLOR);
                    pFnt->_DrawText( aDrawInf );
                }
                delete pClip;
            }
            delete pFnt;
            return true;
        }
    }
    else
        return true;
    return false;
}

void SwTextFrm::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    ResetRepaint();

    // #i16816# tagged pdf support
    SwViewShell *pSh = getRootFrm()->GetCurrShell();

    Num_Info aNumInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperNumbering( &aNumInfo, nullptr, nullptr, rRenderContext );

    Frm_Info aFrmInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperParagraph( nullptr, &aFrmInfo, nullptr, rRenderContext );

    if( !IsEmpty() || !PaintEmpty( rRect, true ) )
    {
#if OSL_DEBUG_LEVEL > 1
        const SwTwips nDbgY = Frm().Top();
        (void)nDbgY;
#endif

        if( IsLocked() || IsHiddenNow() || ! Prt().HasArea() )
            return;

        // It can happen that the IdleCollector withdrew my cached information
        if( !HasPara() )
        {
            OSL_ENSURE( GetValidPosFlag(), "+SwTextFrm::Paint: no Calc()" );

            // #i29062# pass info that we are currently
            // painting.
            const_cast<SwTextFrm*>(this)->GetFormatted( true );
            if( IsEmpty() )
            {
                PaintEmpty( rRect, false );
                return;
            }
            if( !HasPara() )
            {
                OSL_ENSURE( false, "+SwTextFrm::Paint: missing format information" );
                return;
            }
        }

        // We don't want to be interrupted while painting.
        // Do that after thr Format()!
        TextFrmLockGuard aLock(const_cast<SwTextFrm*>(this));

        // We only paint the part of the TextFrm which changed, is within the
        // range and was requested to paint.
        // One could think that the area rRect _needs_ to be painted, although
        // rRepaint is set. Indeed, we cannot avoid this problem from a formal
        // perspective. Luckily we can assume rRepaint to be empty when we need
        // paint the while Frm.
        SwTextLineAccess aAccess( this );
        SwParaPortion *pPara = aAccess.GetPara();

        SwRepaint &rRepaint = pPara->GetRepaint();

        // Switch off recycling when in the FlyCntFrm.
        // A DrawRect is called for repainting the line anyways.
        if( rRepaint.GetOfst() )
        {
            const SwFlyFrm *pFly = FindFlyFrm();
            if( pFly && pFly->IsFlyInCntFrm() )
                rRepaint.SetOfst( 0 );
        }

        // Ge the String for painting. The length is of special interest.

        // Rectangle
        OSL_ENSURE( ! IsSwapped(), "A frame is swapped before Paint" );
        SwRect aOldRect( rRect );

        {
            SwSwapIfNotSwapped swap(const_cast<SwTextFrm *>(this));

            if ( IsVertical() )
                SwitchVerticalToHorizontal( (SwRect&)rRect );

            if ( IsRightToLeft() )
                SwitchRTLtoLTR( (SwRect&)rRect );

            SwTextPaintInfo aInf( const_cast<SwTextFrm*>(this), rRect );
            aInf.SetWrongList( const_cast<SwTextNode*>(GetTextNode())->GetWrong() );
            aInf.SetGrammarCheckList( const_cast<SwTextNode*>(GetTextNode())->GetGrammarCheck() );
            aInf.SetSmartTags( const_cast<SwTextNode*>(GetTextNode())->GetSmartTags() );
            aInf.GetTextFly().SetTopRule();

            SwTextPainter  aLine( const_cast<SwTextFrm*>(this), &aInf );
            // Optimization: if no free flying Frm overlaps into our line, the
            // SwTextFly just switches off
            aInf.GetTextFly().Relax();

            OutputDevice* pOut = aInf.GetOut();
            const bool bOnWin = pSh->GetWin() != nullptr;

            SwSaveClip aClip( bOnWin || IsUndersized() ? pOut : nullptr );

            // Output loop: For each Line ... (which is still visible) ...
            //   adapt rRect (Top + 1, Bottom - 1)
            // Because the Iterator attaches the Lines without a gap to each other
            aLine.TwipsToLine( rRect.Top() + 1 );
            long nBottom = rRect.Bottom();

            bool bNoPrtLine = 0 == GetMinPrtLine();
            if( !bNoPrtLine )
            {
                while ( aLine.Y() < GetMinPrtLine() && aLine.Next() )
                    ;
                bNoPrtLine = aLine.Y() >= GetMinPrtLine();
            }
            if( bNoPrtLine )
            {
                do
                {
                    aLine.DrawTextLine( rRect, aClip, IsUndersized() );

                } while( aLine.Next() && aLine.Y() <= nBottom );
            }

            // Once is enough:
            if( aLine.IsPaintDrop() )
                aLine.PaintDropPortion();

            if( rRepaint.HasArea() )
                rRepaint.Clear();
        }

        (SwRect&)rRect = aOldRect;

        OSL_ENSURE( ! IsSwapped(), "A frame is swapped after Paint" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
