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

#include <memory>
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
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <txtfrm.hxx>
#include "itrpaint.hxx"
#include "txtpaint.hxx"
#include "txtcache.hxx"
#include <flyfrm.hxx>
#include "redlnitr.hxx"
#include <swmodule.hxx>
#include <tabfrm.hxx>
#include <numrule.hxx>
#include <SwGrammarMarkUp.hxx>
#include <wrong.hxx>

#include <EnhancedPDFExportHelper.hxx>

#include <IDocumentStylePoolAccess.hxx>

#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

using namespace ::com::sun::star;

static bool bInitFont = true;

class SwExtraPainter
{
    SwSaveClip m_aClip;
    SwRect m_aRect;
    const SwTextFrame* m_pTextFrame;
    SwViewShell *m_pSh;
    std::unique_ptr<SwFont> m_pFnt;
    const SwLineNumberInfo &m_rLineInf;
    SwTwips m_nX;
    SwTwips m_nRedX;
    sal_uLong m_nLineNr;
    sal_uInt16 m_nDivider;
    bool m_bGoLeft;
    bool IsClipChg() { return m_aClip.IsChg(); }

    SwExtraPainter(const SwExtraPainter&) = delete;
    SwExtraPainter& operator=(const SwExtraPainter&) = delete;

public:
    SwExtraPainter( const SwTextFrame *pFrame, SwViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct,
        sal_Int16 eHor, bool bLnNm );
    SwFont* GetFont() const { return m_pFnt.get(); }
    void IncLineNr() { ++m_nLineNr; }
    bool HasNumber() { return !( m_nLineNr % m_rLineInf.GetCountBy() ); }
    bool HasDivider() {
        if( !m_nDivider ) return false;
        return !(m_nLineNr % m_rLineInf.GetDividerCountBy());
    }

    void PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed );
    void PaintRedline( SwTwips nY, long nMax );
};

SwExtraPainter::SwExtraPainter( const SwTextFrame *pFrame, SwViewShell *pVwSh,
                                const SwLineNumberInfo &rLnInf, const SwRect &rRct,
                                sal_Int16 eHor, bool bLineNum )
    : m_aClip( pVwSh->GetWin() || pFrame->IsUndersized() ? pVwSh->GetOut() : nullptr )
    , m_aRect( rRct )
    , m_pTextFrame( pFrame )
    , m_pSh( pVwSh )
    , m_rLineInf( rLnInf )
    , m_nX(0)
    , m_nRedX(0)
    , m_nLineNr( 1 )
    , m_nDivider(0)
    , m_bGoLeft(false)
{
    if( pFrame->IsUndersized() )
    {
        SwTwips nBottom = pFrame->getFrameArea().Bottom();
        if( m_aRect.Bottom() > nBottom )
            m_aRect.Bottom( nBottom );
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
        m_nDivider = !m_rLineInf.GetDivider().isEmpty() ? m_rLineInf.GetDividerCountBy() : 0;
        m_nX = pFrame->getFrameArea().Left();
        SwCharFormat* pFormat = m_rLineInf.GetCharFormat( const_cast<IDocumentStylePoolAccess&>(pFrame->GetDoc().getIDocumentStylePoolAccess()) );
        OSL_ENSURE( pFormat, "PaintExtraData without CharFormat" );
        m_pFnt.reset( new SwFont(&pFormat->GetAttrSet(), &pFrame->GetDoc().getIDocumentSettingAccess()) );
        m_pFnt->Invalidate();
        m_pFnt->ChgPhysFnt( m_pSh, *m_pSh->GetOut() );
        m_pFnt->SetVertical( 0, pFrame->IsVertical() );
        m_nLineNr += pFrame->GetAllLines() - pFrame->GetThisLines();
        LineNumberPosition ePos = m_rLineInf.GetPos();
        if( ePos != LINENUMBER_POS_LEFT && ePos != LINENUMBER_POS_RIGHT )
        {
            if( pFrame->FindPageFrame()->OnRightPage() )
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
            m_bGoLeft = true;
            m_nX -= m_rLineInf.GetPosFromLeft();
        }
        else
        {
            m_bGoLeft = false;
            m_nX += pFrame->getFrameArea().Width() + m_rLineInf.GetPosFromLeft();
        }
    }
    if( eHor != text::HoriOrientation::NONE )
    {
        if( text::HoriOrientation::INSIDE == eHor || text::HoriOrientation::OUTSIDE == eHor )
        {
            if( !nVirtPageNum )
                nVirtPageNum = pFrame->FindPageFrame()->OnRightPage() ? 1 : 2;
            if( nVirtPageNum % 2 )
                eHor = eHor == text::HoriOrientation::INSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
            else
                eHor = eHor == text::HoriOrientation::OUTSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
        }
        const SwFrame* pTmpFrame = pFrame->FindTabFrame();
        if( !pTmpFrame )
            pTmpFrame = pFrame;
        m_nRedX = text::HoriOrientation::LEFT == eHor ? pTmpFrame->getFrameArea().Left() - REDLINE_DISTANCE :
            pTmpFrame->getFrameArea().Right() + REDLINE_DISTANCE;
    }
}

void SwExtraPainter::PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed )
{
  // Line number is stronger than the divider
    const OUString aTmp( HasNumber() ? m_rLineInf.GetNumType().GetNumStr( m_nLineNr )
                                : m_rLineInf.GetDivider() );

    // Get script type of line numbering:
    m_pFnt->SetActual( SwScriptInfo::WhichFont(0, aTmp) );

    SwDrawTextInfo aDrawInf( m_pSh, *m_pSh->GetOut(), aTmp, 0, aTmp.getLength() );
    aDrawInf.SetSpace( 0 );
    aDrawInf.SetWrong( nullptr );
    aDrawInf.SetGrammarCheck( nullptr );
    aDrawInf.SetSmartTags( nullptr );
    aDrawInf.SetFrame( m_pTextFrame );
    aDrawInf.SetFont( m_pFnt.get() );
    aDrawInf.SetSnapToGrid( false );
    aDrawInf.SetIgnoreFrameRTL( true );

    bool bTooBig = m_pFnt->GetSize( m_pFnt->GetActual() ).Height() > nMax &&
                m_pFnt->GetHeight( m_pSh, *m_pSh->GetOut() ) > nMax;
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
    Point aTmpPos( m_nX, nY );
    aTmpPos.AdjustY(nAsc );
    bool bPaint = true;
    if( !IsClipChg() )
    {
        Size aSize = pTmpFnt->GetTextSize_( aDrawInf );
        if( m_bGoLeft )
            aTmpPos.AdjustX( -(aSize.Width()) );
        // calculate rectangle containing the line number
        SwRect aRct( Point( aTmpPos.X(),
                         aTmpPos.Y() - pTmpFnt->GetAscent( m_pSh, *m_pSh->GetOut() )
                          ), aSize );
        if( !m_aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( m_aRect ).IsEmpty() )
                bPaint = false;
            else
                m_aClip.ChgClip( m_aRect, m_pTextFrame );
        }
    }
    else if( m_bGoLeft )
        aTmpPos.AdjustX( -(pTmpFnt->GetTextSize_( aDrawInf ).Width()) );
    aDrawInf.SetPos( aTmpPos );
    if( bPaint )
        pTmpFnt->DrawText_( aDrawInf );

    if( bTooBig )
        delete pTmpFnt;
    if( bRed )
    {
        long nDiff = m_bGoLeft ? m_nRedX - m_nX : m_nX - m_nRedX;
        if( nDiff > REDLINE_MINDIST )
            PaintRedline( nY, nMax );
    }
}

void SwExtraPainter::PaintRedline( SwTwips nY, long nMax )
{
    Point aStart( m_nRedX, nY );
    Point aEnd( m_nRedX, nY + nMax );

    if( !IsClipChg() )
    {
        SwRect aRct( aStart, aEnd );
        if( !m_aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( m_aRect ).IsEmpty() )
                return;
            m_aClip.ChgClip( m_aRect, m_pTextFrame );
        }
    }
    const Color aOldCol( m_pSh->GetOut()->GetLineColor() );
    m_pSh->GetOut()->SetLineColor( SW_MOD()->GetRedlineMarkColor() );

    if ( m_pTextFrame->IsVertical() )
    {
        m_pTextFrame->SwitchHorizontalToVertical( aStart );
        m_pTextFrame->SwitchHorizontalToVertical( aEnd );
    }

    m_pSh->GetOut()->DrawLine( aStart, aEnd );
    m_pSh->GetOut()->SetLineColor( aOldCol );
}

void SwTextFrame::PaintExtraData( const SwRect &rRect ) const
{
    if( getFrameArea().Top() > rRect.Bottom() || getFrameArea().Bottom() < rRect.Top() )
        return;

    SwDoc const& rDoc(GetDoc());
    const IDocumentRedlineAccess& rIDRA = rDoc.getIDocumentRedlineAccess();
    const SwLineNumberInfo &rLineInf = rDoc.GetLineNumberInfo();
    const SwFormatLineNumber &rLineNum = GetAttrSet()->GetLineNumber();
    bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    sal_Int16 eHor = static_cast<sal_Int16>(SW_MOD()->GetRedlineMarkPos());
    if (eHor != text::HoriOrientation::NONE
        && (!IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineFlags())
            || getRootFrame()->IsHideRedlines()))
    {
        eHor = text::HoriOrientation::NONE;
    }
    bool bRedLine = eHor != text::HoriOrientation::NONE;
    if ( !bLineNum && !bRedLine )
        return;

    if( IsLocked() || IsHiddenNow() || !getFramePrintArea().Height() )
        return;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();

    SwSwapIfNotSwapped swap(const_cast<SwTextFrame *>(this));
    SwRect rOldRect( rRect );

    if ( IsVertical() )
        SwitchVerticalToHorizontal( const_cast<SwRect&>(rRect) );

    SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
    aLayoutModeModifier.Modify( false );

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pSh->GetOut() );

    SwExtraPainter aExtra( this, pSh, rLineInf, rRect, eHor, bLineNum );

    if( HasPara() )
    {
        TextFrameLockGuard aLock(const_cast<SwTextFrame*>(this));

        SwTextLineAccess aAccess( this );
        aAccess.GetPara();

        SwTextPaintInfo aInf( const_cast<SwTextFrame*>(this), rRect );

        aLayoutModeModifier.Modify( false );

        SwTextPainter  aLine( const_cast<SwTextFrame*>(this), &aInf );
        bool bNoDummy = !aLine.GetNext(); // Only one empty line!

        while( aLine.Y() + aLine.GetLineHeight() <= rRect.Top() )
        {
            if( !aLine.GetCurr()->IsDummy() &&
                ( rLineInf.IsCountBlankLines() ||
                  aLine.GetCurr()->HasContent() ) )
                aExtra.IncLineNr();
            if( !aLine.Next() )
            {
                const_cast<SwRect&>(rRect) = rOldRect;
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
        if (!GetMergedPara() &&
            SwRedlineTable::npos == rIDRA.GetRedlinePos(*GetTextNodeFirst(), USHRT_MAX))
        {
            bRedLine = false;
        }

        if( bLineNum && rLineInf.IsCountBlankLines() &&
            ( aExtra.HasNumber() || aExtra.HasDivider() ) )
        {
            aExtra.PaintExtra( getFrameArea().Top()+getFramePrintArea().Top(), aExtra.GetFont()
                ->GetAscent( pSh, *pSh->GetOut() ), getFramePrintArea().Height(), bRedLine );
        }
        else if( bRedLine )
            aExtra.PaintRedline( getFrameArea().Top()+getFramePrintArea().Top(), getFramePrintArea().Height() );
    }

    const_cast<SwRect&>(rRect) = rOldRect;

}

SwRect SwTextFrame::GetPaintSwRect()
{
    // finger layout
    OSL_ENSURE( isFrameAreaPositionValid(), "+SwTextFrame::GetPaintSwRect: no Calc()" );

    SwRect aRet( getFramePrintArea() );
    if ( IsEmpty() || !HasPara() )
        aRet += getFrameArea().Pos();
    else
    {
        // We return the right paint rect. Use the calculated PaintOfst as the
        // left margin
        SwRepaint& rRepaint = GetPara()->GetRepaint();
        long l;

        if ( IsVertLR() ) // mba: the following line was added, but we don't need it for the existing directions; kept for IsVertLR(), but should be checked
            rRepaint.Chg( GetUpper()->getFrameArea().Pos() + GetUpper()->getFramePrintArea().Pos(), GetUpper()->getFramePrintArea().SSize() );

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
        SwLayoutFrame* pBodyFrame = GetUpper();
        if (pBodyFrame->IsBodyFrame() && aRet.Left() == (pBodyFrame->getFrameArea().Left() + pBodyFrame->getFramePrintArea().Left()))
            if (SwLayoutFrame* pPageFrame = pBodyFrame->GetUpper())
                aRet.Left(pPageFrame->getFrameArea().Left());

        if ( IsRightToLeft() )
            SwitchLTRtoRTL( aRet );

        if ( IsVertical() )
            SwitchHorizontalToVertical( aRet );
    }
    ResetRepaint();

    return aRet;
}

bool SwTextFrame::PaintEmpty( const SwRect &rRect, bool bCheck ) const
{
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
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
            std::unique_ptr<SwFont> pFnt;
            const SwTextNode& rTextNode = *GetTextNodeForParaProps();
            if ( rTextNode.HasSwAttrSet() )
            {
                const SwAttrSet *pAttrSet = &( rTextNode.GetSwAttrSet() );
                pFnt.reset(new SwFont( pAttrSet, rTextNode.getIDocumentSettingAccess() ));
            }
            else
            {
                SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), pSh );
                pFnt.reset(new SwFont( aFontAccess.Get()->GetFont() ));
            }

            const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();
            if (IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineFlags())
                && !getRootFrame()->IsHideRedlines())
            {
                const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
                if( SwRedlineTable::npos != nRedlPos )
                {
                    SwAttrHandler aAttrHandler;
                    aAttrHandler.Init(  rTextNode.GetSwAttrSet(),
                                       *rTextNode.getIDocumentSettingAccess() );
                    SwRedlineItr aRedln(rTextNode, *pFnt, aAttrHandler, nRedlPos, SwRedlineItr::Mode::Show);
                }
            }

            if( pSh->GetViewOptions()->IsParagraph() && getFramePrintArea().Height() )
            {
                if( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet( SwFontScript::Latin ) &&
                    pFnt->GetName( SwFontScript::Latin ) != numfunc::GetDefBulletFontname() )
                {
                    pFnt->SetFamily( FAMILY_DONTKNOW, SwFontScript::Latin );
                    pFnt->SetName( numfunc::GetDefBulletFontname(), SwFontScript::Latin );
                    pFnt->SetStyleName(OUString(), SwFontScript::Latin);
                    pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, SwFontScript::Latin );
                }
                pFnt->SetVertical( 0, IsVertical() );
                SwFrameSwapper aSwapper( this, true );
                SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
                aLayoutModeModifier.Modify( IsRightToLeft() );

                pFnt->Invalidate();
                pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
                Point aPos = getFrameArea().Pos() + getFramePrintArea().Pos();

                const SvxLRSpaceItem &rSpace =
                    GetTextNodeForParaProps()->GetSwAttrSet().GetLRSpace();

                if ( rSpace.GetTextFirstLineOfst() > 0 )
                    aPos.AdjustX(rSpace.GetTextFirstLineOfst() );

                std::unique_ptr<SwSaveClip> pClip;
                if( IsUndersized() )
                {
                    pClip.reset(new SwSaveClip( pSh->GetOut() ));
                    pClip->ChgClip( rRect );
                }

                aPos.AdjustY(pFnt->GetAscent( pSh, *pSh->GetOut() ) );

                if (GetTextNodeForParaProps()->GetSwAttrSet().GetParaGrid().GetValue() &&
                     IsInDocBody() )
                {
                    SwTextGridItem const*const pGrid(GetGridItem(FindPageFrame()));
                    if ( pGrid )
                    {
                        // center character in grid line
                        aPos.AdjustY(( pGrid->GetBaseHeight() -
                                      pFnt->GetHeight( pSh, *pSh->GetOut() ) ) / 2 );

                        if ( ! pGrid->GetRubyTextBelow() )
                            aPos.AdjustY(pGrid->GetRubyHeight() );
                    }
                }

                // Don't show the paragraph mark for collapsed paragraphs, when they are hidden
                if ( EmptyHeight( ) > 1 )
                {
                    const OUString aTmp( CH_PAR );
                    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), aTmp, 0, 1 );
                    aDrawInf.SetPos( aPos );
                    aDrawInf.SetSpace( 0 );
                    aDrawInf.SetKanaComp( 0 );
                    aDrawInf.SetWrong( nullptr );
                    aDrawInf.SetGrammarCheck( nullptr );
                    aDrawInf.SetSmartTags( nullptr );
                    aDrawInf.SetFrame( this );
                    aDrawInf.SetFont( pFnt.get() );
                    aDrawInf.SetSnapToGrid( false );

                    pFnt->SetColor(NON_PRINTING_CHARACTER_COLOR);
                    pFnt->DrawText_( aDrawInf );
                }
            }
            return true;
        }
    }
    else
        return true;
    return false;
}

void SwTextFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    ResetRepaint();

    // #i16816# tagged pdf support
    SwViewShell *pSh = getRootFrame()->GetCurrShell();

    Num_Info aNumInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperNumbering( &aNumInfo, nullptr, nullptr, rRenderContext );

    Frame_Info aFrameInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperParagraph( nullptr, &aFrameInfo, nullptr, rRenderContext );

    if( IsEmpty() && PaintEmpty( rRect, true ) )
        return;

    if( IsLocked() || IsHiddenNow() || ! getFramePrintArea().HasArea() )
        return;

    // It can happen that the IdleCollector withdrew my cached information
    if( !HasPara() )
    {
        OSL_ENSURE( isFrameAreaPositionValid(), "+SwTextFrame::PaintSwFrame: no Calc()" );

        // #i29062# pass info that we are currently
        // painting.
        const_cast<SwTextFrame*>(this)->GetFormatted( true );
        if( IsEmpty() )
        {
            PaintEmpty( rRect, false );
            return;
        }
        if( !HasPara() )
        {
            OSL_ENSURE( false, "+SwTextFrame::PaintSwFrame: missing format information" );
            return;
        }
    }

    // We don't want to be interrupted while painting.
    // Do that after thr Format()!
    TextFrameLockGuard aLock(const_cast<SwTextFrame*>(this));

    // We only paint the part of the TextFrame which changed, is within the
    // range and was requested to paint.
    // One could think that the area rRect _needs_ to be painted, although
    // rRepaint is set. Indeed, we cannot avoid this problem from a formal
    // perspective. Luckily we can assume rRepaint to be empty when we need
    // paint the while Frame.
    SwTextLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();

    SwRepaint &rRepaint = pPara->GetRepaint();

    // Switch off recycling when in the FlyContentFrame.
    // A DrawRect is called for repainting the line anyways.
    if( rRepaint.GetOfst() )
    {
        const SwFlyFrame *pFly = FindFlyFrame();
        if( pFly && pFly->IsFlyInContentFrame() )
            rRepaint.SetOfst( 0 );
    }

    // Ge the String for painting. The length is of special interest.

    // Rectangle
    OSL_ENSURE( ! IsSwapped(), "A frame is swapped before Paint" );
    SwRect aOldRect( rRect );

    {
        SwSwapIfNotSwapped swap(const_cast<SwTextFrame *>(this));

        if ( IsVertical() )
            SwitchVerticalToHorizontal( const_cast<SwRect&>(rRect) );

        if ( IsRightToLeft() )
            SwitchRTLtoLTR( const_cast<SwRect&>(rRect) );

        SwTextPaintInfo aInf( const_cast<SwTextFrame*>(this), rRect );
        sw::WrongListIterator iterWrong(*this, &SwTextNode::GetWrong);
        sw::WrongListIterator iterGrammar(*this, &SwTextNode::GetGrammarCheck);
        sw::WrongListIterator iterSmartTags(*this, &SwTextNode::GetSmartTags);
        if (iterWrong.LooksUseful())
        {
            aInf.SetWrongList( &iterWrong );
        }
        if (iterGrammar.LooksUseful())
        {
            aInf.SetGrammarCheckList( &iterGrammar );
        }
        if (iterSmartTags.LooksUseful())
        {
            aInf.SetSmartTags( &iterSmartTags );
        }
        aInf.GetTextFly().SetTopRule();

        SwTextPainter  aLine( const_cast<SwTextFrame*>(this), &aInf );
        // Optimization: if no free flying Frame overlaps into our line, the
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

    const_cast<SwRect&>(rRect) = aOldRect;

    OSL_ENSURE( ! IsSwapped(), "A frame is swapped after Paint" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
