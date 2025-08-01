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
#include <editeng/pgrditem.hxx>
#include <editeng/lrspitem.hxx>
#include <tgrditem.hxx>
#include <paratr.hxx>

#include <fmtline.hxx>
#include <lineinfo.hxx>
#include <charfmt.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <txtfrm.hxx>
#include "itrpaint.hxx"
#include "txtpaint.hxx"
#include "txtcache.hxx"
#include <flyfrm.hxx>
#include "redlnitr.hxx"
#include <redline.hxx>
#include <swmodule.hxx>
#include <tabfrm.hxx>
#include <numrule.hxx>
#include <wrong.hxx>
#include <vcl/lineinfo.hxx>
#include <officecfg/Office/Writer.hxx>

#include <EnhancedPDFExportHelper.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>

#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

using namespace ::com::sun::star;

static bool bInitFont = true;

namespace {

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
    sal_Int32 m_nLineNr;
    sal_uInt16 m_nDivider;
    bool m_bGoLeft;
    bool IsClipChg() const { return m_aClip.IsChg(); }

    SwExtraPainter(const SwExtraPainter&) = delete;
    SwExtraPainter& operator=(const SwExtraPainter&) = delete;

public:
    SwExtraPainter( const SwTextFrame *pFrame, SwViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct,
        sal_Int16 eHor, bool bLnNm );
    SwFont* GetFont() const { return m_pFnt.get(); }
    void IncLineNr() { ++m_nLineNr; }
    bool HasNumber() const {
        assert( m_rLineInf.GetCountBy() != 0 );
        if( m_rLineInf.GetCountBy() == 0 )
            return false;
        return !( m_nLineNr % static_cast<sal_Int32>(m_rLineInf.GetCountBy()) );
    }
    bool HasDivider() const {
        assert( m_rLineInf.GetDividerCountBy() != 0 );
        if( !m_nDivider || m_rLineInf.GetDividerCountBy() == 0 )
            return false;
        return !(m_nLineNr % m_rLineInf.GetDividerCountBy());
    }

    void PaintExtra( SwTwips nY, tools::Long nAsc, tools::Long nMax, bool bRed, const OUString* pRedlineText = nullptr );
    void PaintRedline( SwTwips nY, tools::Long nMax, sal_Int16 nWordSpacing = 0 );
};

}

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
    std::optional<bool> oIsRightPage;
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
        assert(pFormat && "PaintExtraData without CharFormat");
        m_pFnt.reset( new SwFont(&pFormat->GetAttrSet(), &pFrame->GetDoc().getIDocumentSettingAccess()) );
        m_pFnt->Invalidate();
        m_pFnt->ChgPhysFnt( m_pSh, *m_pSh->GetOut() );
        m_pFnt->SetVertical( 0_deg10, pFrame->IsVertical() );
    }

    if( bLineNum )
    {
        m_nLineNr += pFrame->GetAllLines() - pFrame->GetThisLines();
        LineNumberPosition ePos = m_rLineInf.GetPos();
        if( ePos != LINENUMBER_POS_LEFT && ePos != LINENUMBER_POS_RIGHT )
        {
            if( pFrame->FindPageFrame()->OnRightPage() )
            {
                oIsRightPage = true;
                ePos = ePos == LINENUMBER_POS_INSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            }
            else
            {
                oIsRightPage = false;
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
    if( eHor == text::HoriOrientation::NONE )
        return;

    if( text::HoriOrientation::INSIDE == eHor || text::HoriOrientation::OUTSIDE == eHor )
    {
        if (!oIsRightPage.has_value())
            oIsRightPage = pFrame->FindPageFrame()->OnRightPage();
        if (*oIsRightPage)
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

void SwExtraPainter::PaintExtra( SwTwips nY, tools::Long nAsc, tools::Long nMax, bool bRed, const OUString* pRedlineText )
{
    const OUString aTmp( pRedlineText
                             // Tracked change is stronger than the line number
                             ? *pRedlineText
                             : ( HasNumber()
                                 // Line number is stronger than the divider
                                 ? m_rLineInf.GetNumType().GetNumStr( m_nLineNr )
                                 : m_rLineInf.GetDivider() ) );

    // Get script type of line numbering:
    m_pFnt->SetActual( SwScriptInfo::WhichFont(0, aTmp) );

    if ( pRedlineText )
    {
        m_pFnt->SetColor(m_pSh->GetViewOptions()->GetNonPrintingCharacterColor());
        // don't strike out text in Insertions In Margin mode
        if ( !m_pSh->GetViewOptions()->IsShowChangesInMargin2() )
            m_pFnt->SetStrikeout( STRIKEOUT_SINGLE );
        m_pFnt->SetSize( Size( 0, 200), m_pFnt->GetActual() );
    }

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
    if ( pRedlineText )
    {
        Size aSize = pTmpFnt->GetTextSize_( aDrawInf );
        aTmpPos.AdjustX( -(aSize.Width()) - 200 );
    }
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
        if( !m_aRect.Contains( aRct ) )
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
        tools::Long nDiff = m_bGoLeft ? m_nRedX - m_nX : m_nX - m_nRedX;
        if( nDiff > REDLINE_MINDIST )
            PaintRedline( nY, nMax );
    }
}

// paint redline or word spacing indicator
void SwExtraPainter::PaintRedline( SwTwips nY, tools::Long nMax, sal_Int16 nWordSpacing )
{
    Point aStart( m_nRedX, nY );
    Point aEnd( m_nRedX, nY + nMax );

    if( !IsClipChg() )
    {
        SwRect aRct( aStart, aEnd );
        if( !m_aRect.Contains( aRct ) )
        {
            if( aRct.Intersection( m_aRect ).IsEmpty() )
                return;
            m_aClip.ChgClip( m_aRect, m_pTextFrame );
        }
    }
    m_pSh->GetOut()->Push(vcl::PushFlags::LINECOLOR);
    m_pSh->GetOut()->SetLineColor(nWordSpacing ? COL_LIGHTRED : SwModule::get()->GetRedlineMarkColor());

    if ( nWordSpacing )
    {
        LineInfo aLineInfo;
        aLineInfo.SetStyle(LineStyle::Solid);
        aLineInfo.SetWidth( nWordSpacing * 2540/1440 );

        m_pSh->GetOut()->DrawLine( aStart, aEnd, aLineInfo );
    }
    else
        m_pSh->GetOut()->DrawLine( aStart, aEnd );
    m_pSh->GetOut()->Pop();
}

void SwTextFrame::PaintExtraData( const SwRect &rRect ) const
{
    if( getFrameArea().Top() > rRect.Bottom() || getFrameArea().Bottom() < rRect.Top() )
        return;

    PaintOutlineContentVisibilityButton();

    SwDoc const& rDoc(GetDoc());
    const IDocumentRedlineAccess& rIDRA = rDoc.getIDocumentRedlineAccess();
    const SwLineNumberInfo &rLineInf = rDoc.GetLineNumberInfo();
    const SwFormatLineNumber &rLineNum = GetAttrSet()->GetLineNumber();
    bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    sal_Int16 eHor = static_cast<sal_Int16>(SwModule::get()->GetRedlineMarkPos());
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    bool bWordSpacingIndicator = officecfg::Office::Writer::Content::Display::ShowWordSpacingIndicator::get()
        && pSh->GetViewOptions()->IsViewMetaChars();
    if (eHor != text::HoriOrientation::NONE
        && !bWordSpacingIndicator
        && (!IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineFlags())
            || getRootFrame()->IsHideRedlines()))
    {
        eHor = text::HoriOrientation::NONE;
    }
    bool bRedLine = eHor != text::HoriOrientation::NONE;
    if ( !bLineNum && !bRedLine && !bWordSpacingIndicator )
        return;

    if( IsLocked() || IsHiddenNow() || !getFramePrintArea().Height() )
        return;

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

        tools::Long nBottom = rRect.Bottom();

        const bool bIsShowChangesInMargin = pSh->GetViewOptions()->IsShowChangesInMargin();
        do
        {
            // A comment from SwTextFormatter::CalcRealHeight:
            // The dummy flag is set on lines that only contain flyportions. Unfortunately an empty
            // line can be at the end of a paragraph (empty paragraphs or behind a Shift-Return).
            if (!aLine.GetCurr()->IsDummy()
                || (!aLine.GetCurr()->GetNext()
                    && aLine.GetStart()
                           >= TextFrameIndex(aLine.GetTextFrame()->GetText().getLength())))
            {
                SwTwips nExtraSpaceSize = aLine.GetCurr()->GetFirstPortion()->ExtraSpaceSize();
                if ( nExtraSpaceSize && bWordSpacingIndicator )
                    aExtra.PaintRedline( aLine.Y(), aLine.GetLineHeight(), nExtraSpaceSize );

                bool bRed = bRedLine && aLine.GetCurr()->HasRedline();
                if( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasContent() )
                {
                    bool bRedInMargin = bIsShowChangesInMargin && bRed;
                    bool bNum = bLineNum && ( aExtra.HasNumber() || aExtra.HasDivider() );
                    if( bRedInMargin || bNum )
                    {
                        SwTwips nTmpHeight, nTmpAscent;
                        aLine.CalcAscentAndHeight( nTmpAscent, nTmpHeight );
                        if ( bRedInMargin )
                        {
                            const OUString* pRedlineText = aLine.GetCurr()->GetRedlineText();
                            if( !pRedlineText->isEmpty() )
                            {
                                aExtra.PaintExtra( aLine.Y(), nTmpAscent,
                                    nTmpHeight, bRed, pRedlineText );
                                bRed = false;
                                bNum = false;
                            }
                        }
                        if ( bNum )
                        {
                            aExtra.PaintExtra( aLine.Y(), nTmpAscent, nTmpHeight, bRed );
                            bRed = false;
                        }
                    }
                    aExtra.IncLineNr();
                }
                if( bRed )
                    aExtra.PaintRedline( aLine.Y(), aLine.GetLineHeight() );
            }
        } while( aLine.Next() && aLine.Y() <= nBottom );
    }
    else
    {
        if (SwRedlineTable::npos == rIDRA.GetRedlinePos(*GetTextNodeFirst(), RedlineType::Any))
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
        tools::Long l;

        if ( IsVertLR() && !IsVertLRBT()) // mba: the following line was added, but we don't need it for the existing directions; kept for IsVertLR(), but should be checked
            rRepaint.Chg( GetUpper()->getFrameArea().Pos() + GetUpper()->getFramePrintArea().Pos(), GetUpper()->getFramePrintArea().SSize() );

        if( rRepaint.GetOffset() )
            rRepaint.Left( rRepaint.GetOffset() );

        l = rRepaint.GetRightOfst();
        if( l && l > rRepaint.Right() )
             rRepaint.Right( l );
        rRepaint.SetOffset( 0 );
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
    PaintParagraphStylesHighlighting();

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
            RedlineType eRedline = RedlineType::None;
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
                const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, RedlineType::Any );
                if( SwRedlineTable::npos != nRedlPos )
                {
                    SwAttrHandler aAttrHandler;
                    aAttrHandler.Init(  rTextNode.GetSwAttrSet(),
                                       *rTextNode.getIDocumentSettingAccess() );
                    SwRedlineItr aRedln(rTextNode, *pFnt, aAttrHandler, nRedlPos, SwRedlineItr::Mode::Show);
                    const SwRangeRedline* pRedline = rIDRA.GetRedlineTable()[nRedlPos];
                    // show redlining only on the inserted/deleted empty paragraph, but not on the next one
                    if ( rTextNode.GetIndex() != pRedline->End()->GetNodeIndex() )
                        eRedline = pRedline->GetType();
                    // except if the next empty paragraph starts a new redline (e.g. deletion after insertion)
                    else if ( nRedlPos + 1 < rIDRA.GetRedlineTable().size() )
                    {
                        const SwRangeRedline* pNextRedline = rIDRA.GetRedlineTable()[nRedlPos + 1];
                        if ( rTextNode.GetIndex() == pNextRedline->Start()->GetNodeIndex() )
                            eRedline = pNextRedline->GetType();
                    }
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
                pFnt->SetVertical( 0_deg10, IsVertical() );
                SwFrameSwapper aSwapper( this, true );
                SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
                aLayoutModeModifier.Modify( IsRightToLeft() );

                pFnt->Invalidate();
                pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
                Point aPos = getFrameArea().Pos() + getFramePrintArea().Pos();

                const SvxFirstLineIndentItem& rFirstLine(
                    GetTextNodeForParaProps()->GetSwAttrSet().GetFirstLineIndent());

                if (0.0 < rFirstLine.GetTextFirstLineOffset().m_dValue)
                {
                    aPos.AdjustX(rFirstLine.ResolveTextFirstLineOffset({}));
                }

                std::unique_ptr<SwSaveClip, o3tl::default_delete<SwSaveClip>> xClip;
                if( IsUndersized() )
                {
                    xClip.reset(new SwSaveClip( pSh->GetOut() ));
                    xClip->ChgClip( rRect );
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
                // No paragraph marker in the non-last part of a split fly anchor, either.
                if ( EmptyHeight( ) > 1 && !HasNonLastSplitFlyDrawObj() )
                {
                    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), CH_PAR, 0, 1 );
                    aDrawInf.SetPos( aPos );
                    aDrawInf.SetSpace( 0 );
                    aDrawInf.SetKanaComp( 0 );
                    aDrawInf.SetWrong( nullptr );
                    aDrawInf.SetGrammarCheck( nullptr );
                    aDrawInf.SetSmartTags( nullptr );
                    aDrawInf.SetFrame( this );
                    aDrawInf.SetFont( pFnt.get() );
                    aDrawInf.SetSnapToGrid( false );

                    // show redline color and settings drawing a background pilcrow,
                    // but keep also other formattings (with neutral pilcrow color)
                    if ( eRedline != RedlineType::None )
                    {
                        pFnt->DrawText_( aDrawInf );
                        if ( eRedline == RedlineType::Delete )
                            pFnt->SetStrikeout( STRIKEOUT_NONE );
                        else
                            pFnt->SetUnderline( LINESTYLE_NONE );
                    }

                    pFnt->SetColor(pSh->GetViewOptions()->GetNonPrintingCharacterColor());
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

void SwTextFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, PaintFrameMode) const
{
    ResetRepaint();

    // #i16816# tagged pdf support
    SwViewShell *pSh = getRootFrame()->GetCurrShell();

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

    // tdf140219-2.odt text frame with only fly portions and a follow is not
    // actually a paragraph - delay creating all structured elements to follow.
    bool const isPDFTaggingEnabled(!HasFollow() || GetPara()->HasContentPortions());
    ::std::optional<SwTaggedPDFHelper> oTaggedPDFHelperNumbering;
    if (isPDFTaggingEnabled)
    {
        Num_Info aNumInfo(*this);
        oTaggedPDFHelperNumbering.emplace(&aNumInfo, nullptr, nullptr, rRenderContext);
    }

    // Lbl unfortunately must be able to contain multiple numbering portions
    // that may be on multiple lines of text (but apparently always in the
    // master frame), so it gets complicated.
    ::std::optional<SwTaggedPDFHelper> oTaggedLabel;
    // Paragraph tag - if there is a list label, opening should be delayed.
    ::std::optional<SwTaggedPDFHelper> oTaggedParagraph;

    if (isPDFTaggingEnabled
        && (GetTextNodeForParaProps()->IsOutline()
            || !GetPara()->HasNumberingPortion(SwParaPortion::FootnoteToo)))
    {   // no Lbl needed => open paragraph tag now
        Frame_Info aFrameInfo(*this, false);
        oTaggedParagraph.emplace(nullptr, &aFrameInfo, nullptr, rRenderContext);
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
    if( rRepaint.GetOffset() )
    {
        const SwFlyFrame *pFly = FindFlyFrame();
        if( pFly && pFly->IsFlyInContentFrame() )
            rRepaint.SetOffset( 0 );
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
        tools::Long nBottom = rRect.Bottom();

        do
        {
            aLine.DrawTextLine(rRect, aClip, IsUndersized(), oTaggedLabel, oTaggedParagraph, isPDFTaggingEnabled);

        } while( aLine.Next() && aLine.Y() <= nBottom );

        // Once is enough:
        if( aLine.IsPaintDrop() )
            aLine.PaintDropPortion();

        if( rRepaint.HasArea() )
            rRepaint.Clear();
    }

    PaintParagraphStylesHighlighting();

    const_cast<SwRect&>(rRect) = aOldRect;

    OSL_ENSURE( ! IsSwapped(), "A frame is swapped after Paint" );

    assert(!oTaggedLabel); // must have been closed if opened
    assert(!isPDFTaggingEnabled || oTaggedParagraph || rRect.GetIntersection(getFrameArea()) != getFrameArea()); // must have been created during complete paint (PDF export is always complete paint)
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
