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

#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/fontitem.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/scopeguard.hxx>

#include <viewsh.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <paratr.hxx>
#include <SwPortionHandler.hxx>
#include "porrst.hxx"
#include "inftxt.hxx"
#include "txtpaint.hxx"
#include <swfntcch.hxx>
#include <tgrditem.hxx>
#include <pagedesc.hxx>
#include <frmatr.hxx>
#include "redlnitr.hxx"
#include "atrhndl.hxx"
#include <rootfrm.hxx>
#include <formatlinebreak.hxx>
#include <txatbase.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <crsrsh.hxx>
#include <swtypes.hxx>
#include <strings.hrc>
#include <flyfrms.hxx>
#include <bodyfrm.hxx>

SwTmpEndPortion::SwTmpEndPortion( const SwLinePortion &rPortion,
                const FontLineStyle eUL,
                const FontStrikeout eStrkout,
                const Color& rCol ) :
    m_eUnderline( eUL ), m_eStrikeout( eStrkout ), m_aColor( rCol )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    SetWhichPor( PortionType::TempEnd );
}

void SwTmpEndPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if (!(rInf.OnWin() && rInf.GetOpt().IsParagraph()))
        return;

    const SwFont* pOldFnt = rInf.GetFont();

    SwFont aFont(*pOldFnt);

    const SwDoc& rDoc = rInf.GetTextFrame()->GetDoc();
    if (aFont.IsSymbol(rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()))
    {
        const SvxFontItem& rFontItem = rDoc.GetDefault(RES_CHRATR_FONT);
        aFont.SetName( rFontItem.GetFamilyName(), SwFontScript::Latin );
        aFont.SetStyleName( rFontItem.GetStyleName(), SwFontScript::Latin );
        aFont.SetFamily( rFontItem.GetFamily(), SwFontScript::Latin );
        aFont.SetPitch( rFontItem.GetPitch(), SwFontScript::Latin );
        aFont.SetCharSet( rFontItem.GetCharSet(), SwFontScript::Latin );
    }
    // Paint strikeout/underline based on redline color and settings
    // (with an extra pilcrow in the background, because there is
    // no SetStrikeoutColor(), also SetUnderColor() doesn't work()).
    if ( m_eUnderline != LINESTYLE_NONE || m_eStrikeout != STRIKEOUT_NONE )
    {
        aFont.SetColor( m_aColor );
        aFont.SetUnderline( m_eUnderline );
        aFont.SetStrikeout( m_eStrikeout );

        const_cast<SwTextPaintInfo&>(rInf).SetFont(&aFont);

        // draw the pilcrow with strikeout/underline in redline color
        rInf.DrawText(CH_PAR, *this);

    }

    aFont.SetColor( NON_PRINTING_CHARACTER_COLOR );
    aFont.SetStrikeout( STRIKEOUT_NONE );
    aFont.SetUnderline( LINESTYLE_NONE );
    const_cast<SwTextPaintInfo&>(rInf).SetFont(&aFont);

    // draw the pilcrow
    rInf.DrawText(CH_PAR, *this);

    const_cast<SwTextPaintInfo&>(rInf).SetFont(const_cast<SwFont*>(pOldFnt));
}

SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion, const SwTextAttr* pAttr )
    : SwLinePortion( rPortion )
{
    mnLineLength = TextFrameIndex(1);
    m_eRedline = RedlineType::None;
    SetWhichPor( PortionType::Break );

    m_eClear = SwLineBreakClear::NONE;
    if (pAttr && pAttr->Which() == RES_TXTATR_LINEBREAK)
    {
        m_eClear = pAttr->GetLineBreak().GetValue();
    }
    m_nTextHeight = 0;
}

TextFrameIndex SwBreakPortion::GetModelPositionForViewPoint(const SwTwips) const
{
    return TextFrameIndex(0);
}

SwTwips SwBreakPortion::GetViewWidth(const SwTextSizeInfo&) const { return 0; }

SwLinePortion *SwBreakPortion::Compress()
{ return (GetNextPortion() && GetNextPortion()->InTextGrp() ? nullptr : this); }

void SwBreakPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !(rInf.OnWin() && rInf.GetOpt().IsLineBreak()) )
        return;

    // Reduce height to text height for the duration of the print, so the vertical height will look
    // correct for the line break character, even for clearing breaks.
    SwTwips nHeight = Height();
    SwTwips nVertPosOffset = (nHeight - m_nTextHeight) / 2;
    auto pPortion = const_cast<SwBreakPortion*>(this);
    pPortion->Height(m_nTextHeight, false);
    if (rInf.GetTextFrame()->IsVertical())
    {
        // Compensate for the offset done in SwTextCursor::AdjustBaseLine() for the vertical case.
        const_cast<SwTextPaintInfo&>(rInf).Y(rInf.Y() + nVertPosOffset);
    }
    comphelper::ScopeGuard g(
        [pPortion, nHeight, &rInf, nVertPosOffset]
        {
            if (rInf.GetTextFrame()->IsVertical())
            {
                const_cast<SwTextPaintInfo&>(rInf).Y(rInf.Y() - nVertPosOffset);
            }
            pPortion->Height(nHeight, false);
        });

    rInf.DrawLineBreak( *this );

    // paint redlining
    if (m_eRedline == RedlineType::None)
        return;

    sal_Int16 nNoBreakWidth = rInf.GetTextSize(S_NOBREAK_FOR_REDLINE).Width();
    if ( nNoBreakWidth > 0 )
    {
        // approximate portion size with multiple no-break spaces
        // and draw these spaces (at least a single one) by DrawText
        // painting the requested redline underline/strikeout
        sal_Int16 nSpaces = (LINE_BREAK_WIDTH + nNoBreakWidth/2) / nNoBreakWidth;
        OUStringBuffer aBuf(S_NOBREAK_FOR_REDLINE);
        for (sal_Int16 i = 1; i < nSpaces; ++i)
            aBuf.append(S_NOBREAK_FOR_REDLINE);

        const SwFont* pOldFnt = rInf.GetFont();

        SwFont aFont(*pOldFnt);

        if (m_eRedline == RedlineType::Delete)
            aFont.SetUnderline( LINESTYLE_NONE );
        else
            aFont.SetStrikeout( STRIKEOUT_NONE );

        const_cast<SwTextPaintInfo&>(rInf).SetFont(&aFont);

        rInf.DrawText(aBuf.makeStringAndClear(), *this);

        const_cast<SwTextPaintInfo&>(rInf).SetFont(const_cast<SwFont*>(pOldFnt));
    }
}

bool SwBreakPortion::Format( SwTextFormatInfo &rInf )
{
    const SwLinePortion *pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    m_nTextHeight = Height();

    // See if this is a clearing break. If so, calculate how much we need to "jump down" so the next
    // line can again use the full text width.
    SwLineBreakClear eClear = m_eClear;
    if (rInf.GetTextFrame()->IsRightToLeft() && eClear != SwLineBreakClear::ALL)
    {
        // RTL ignores left/right breaks.
        eClear = SwLineBreakClear::NONE;
    }
    if (eClear != SwLineBreakClear::NONE)
    {
        SwTextFly& rTextFly = rInf.GetTextFly();
        if (rTextFly.IsOn())
        {
            SwTwips nHeight = rTextFly.GetMaxBottom(*this, rInf) - rInf.Y();
            if (nHeight > Height())
            {
                Height(nHeight, /*bText=*/false);
            }
        }
    }

    SetAscent( pRoot->GetAscent() );
    if (rInf.GetIdx() + TextFrameIndex(1) == TextFrameIndex(rInf.GetText().getLength()))
        rInf.SetNewLine( true );
    return true;
}

void SwBreakPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

void SwBreakPortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText, TextFrameIndex&
        nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwBreakPortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("text-height"),
                                      BAD_CAST(OString::number(m_nTextHeight).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

SwLineBreakClear SwBreakPortion::GetClear() const { return m_eClear; }

SwKernPortion::SwKernPortion( SwLinePortion &rPortion, short nKrn,
                              bool bBG, bool bGK ) :
    m_nKern( nKrn ), m_bBackground( bBG ), m_bGridKern( bGK )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    mnLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Kern );
    if( m_nKern > 0 )
        Width( m_nKern );
    rPortion.Insert( this );
}

SwKernPortion::SwKernPortion( const SwLinePortion& rPortion ) :
    m_nKern( 0 ), m_bBackground( false ), m_bGridKern( true )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );

    mnLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Kern );
}

void SwKernPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !Width() )
        return;

    // bBackground is set for Kerning Portions between two fields
    if ( m_bBackground )
        rInf.DrawViewOpt( *this, PortionType::Field );

    rInf.DrawBackBrush( *this );
    if (GetJoinBorderWithNext() ||GetJoinBorderWithPrev())
        rInf.DrawBorder( *this );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && mpNextPortion && !mpNextPortion->Width() )
        mpNextPortion->PrePaint( rInf, this );

    if( rInf.GetFont()->IsPaintBlank() )
    {
        SwRect aClipRect;
        rInf.CalcRect( *this, &aClipRect );
        SwSaveClip aClip( const_cast<OutputDevice*>(rInf.GetOut()) );
        aClip.ChgClip( aClipRect );
        rInf.DrawText("  ", *this, TextFrameIndex(0), TextFrameIndex(2), true );
    }
}

void SwKernPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if ( m_bGridKern )
        return;

    if( rInf.GetLast() == this )
        rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
    if( m_nKern < 0 )
        Width( -m_nKern );
    else
        Width( 0 );
    rInf.GetLast()->FormatEOL( rInf );
}

SwArrowPortion::SwArrowPortion( const SwLinePortion &rPortion ) :
    m_bLeft( true )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    mnLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Arrow );
}

SwArrowPortion::SwArrowPortion( const SwTextPaintInfo &rInf )
    : m_bLeft( false )
{
    Height(rInf.GetTextFrame()->getFramePrintArea().Height());
    m_aPos.setX( rInf.GetTextFrame()->getFrameArea().Left() +
               rInf.GetTextFrame()->getFramePrintArea().Right() );
    m_aPos.setY( rInf.GetTextFrame()->getFrameArea().Top() +
               rInf.GetTextFrame()->getFramePrintArea().Bottom() );
    SetWhichPor( PortionType::Arrow );
}

void SwArrowPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    const_cast<SwArrowPortion*>(this)->m_aPos = rInf.GetPos();
}

SwLinePortion *SwArrowPortion::Compress() { return this; }

SwTwips SwTextFrame::EmptyHeight() const
{
    if (IsCollapse()) {
        SwViewShell *pSh = getRootFrame()->GetCurrShell();
        if ( auto pCrSh = dynamic_cast<SwCursorShell*>( pSh ) ) {
            // this is called during formatting so avoid recursive layout
            SwContentFrame const*const pCurrFrame = pCrSh->GetCurrFrame(false);
            if (pCurrFrame==static_cast<SwContentFrame const *>(this)) {
                // do nothing
            } else {
                return 1;
            }
        } else {
            return 1;
        }
    }
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTextFrame::EmptyHeight with swapped frame" );

    std::unique_ptr<SwFont> pFnt;
    const SwTextNode& rTextNode = *GetTextNodeForParaProps();
    const IDocumentSettingAccess* pIDSA = rTextNode.getIDocumentSettingAccess();
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( rTextNode.HasSwAttrSet() )
    {
        const SwAttrSet *pAttrSet = &( rTextNode.GetSwAttrSet() );
        pFnt.reset(new SwFont( pAttrSet, pIDSA ));
    }
    else
    {
        SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), pSh);
        pFnt.reset(new SwFont( aFontAccess.Get()->GetFont() ));
        pFnt->CheckFontCacheId( pSh, pFnt->GetActual() );
    }

    if ( IsVertical() )
        pFnt->SetVertical( 2700_deg10 );

    OutputDevice* pOut = pSh ? pSh->GetOut() : nullptr;
    if ( !pOut || !pSh->GetViewOptions()->getBrowseMode() ||
         pSh->GetViewOptions()->IsPrtFormat() )
    {
        pOut = rTextNode.getIDocumentDeviceAccess().getReferenceDevice(true);
    }

    const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();
    if (IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineFlags())
        && !getRootFrame()->IsHideRedlines())
    {
        const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, RedlineType::Any );
        if( SwRedlineTable::npos != nRedlPos )
        {
            SwAttrHandler aAttrHandler;
            aAttrHandler.Init(rTextNode.GetSwAttrSet(),
                              *rTextNode.getIDocumentSettingAccess());
            SwRedlineItr aRedln( rTextNode, *pFnt, aAttrHandler,
                                 nRedlPos, SwRedlineItr::Mode::Show);
        }
    }

    SwTwips nRet;
    if( !pOut )
        nRet = IsVertical() ?
               getFramePrintArea().SSize().Width() + 1 :
               getFramePrintArea().SSize().Height() + 1;
    else
    {
        pFnt->SetFntChg( true );
        pFnt->ChgPhysFnt( pSh, *pOut );
        nRet = pFnt->GetHeight( pSh, *pOut );
    }
    return nRet;
}

bool SwTextFrame::FormatEmpty()
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTextFrame::FormatEmpty with swapped frame" );

    bool bCollapse = EmptyHeight( ) == 1 && IsCollapse( );

    // sw_redlinehide: just disable FormatEmpty optimisation for now
    // Split fly frames: non-last parts of the anchor want this optimization to clear the old
    // content.
    SwFlyAtContentFrame* pNonLastSplitFlyDrawObj = HasNonLastSplitFlyDrawObj();
    bool bHasNonLastSplitFlyDrawObj = pNonLastSplitFlyDrawObj != nullptr;

    if (pNonLastSplitFlyDrawObj && pNonLastSplitFlyDrawObj->IsWrapOnAllPages())
    {
        // Split fly: the anchor is non-empty on all pages in the "wrap on all pages" case.
        bHasNonLastSplitFlyDrawObj = false;
    }

    if ((HasFollow() && !bHasNonLastSplitFlyDrawObj) || GetMergedPara() || (GetTextNodeFirst()->GetpSwpHints() && !bHasNonLastSplitFlyDrawObj) ||
        nullptr != GetTextNodeForParaProps()->GetNumRule() ||
        GetTextNodeFirst()->HasHiddenCharAttribute(true) ||
         IsInFootnote() || ( HasPara() && GetPara()->IsPrepMustFit() ) )
        return false;
    const SwAttrSet& aSet = GetTextNodeForParaProps()->GetSwAttrSet();
    const SvxAdjust nAdjust = aSet.GetAdjust().GetAdjust();
    if( !bCollapse && ( ( ( ! IsRightToLeft() && ( SvxAdjust::Left != nAdjust ) ) ||
          (   IsRightToLeft() && ( SvxAdjust::Right != nAdjust ) ) ) ||
          aSet.GetRegister().GetValue() ) )
        return false;
    const SvxLineSpacingItem &rSpacing = aSet.GetLineSpacing();
    if( !bCollapse && ( SvxLineSpaceRule::Min == rSpacing.GetLineSpaceRule() ||
        SvxLineSpaceRule::Fix == rSpacing.GetLineSpaceRule() ||
        aSet.GetFirstLineIndent().IsAutoFirst()))
    {
        return false;
    }

    SwTextFly aTextFly( this );
    SwRect aRect;
    bool bFirstFlyCheck = 0 != getFramePrintArea().Height();
    if ( !bCollapse && bFirstFlyCheck &&
            aTextFly.IsOn() && aTextFly.IsAnyObj( aRect ) && !bHasNonLastSplitFlyDrawObj )
        return false;

    if (IsEmptyWithSplitFly())
    {
        // We don't want this optimization in case the paragraph is not really empty, because it has
        // a fly frame and it also needs space for the empty paragraph in a next line.
        return false;
    }

    // only need to check one node because of early return on GetMerged()
    for (SwContentIndex const* pIndex = GetTextNodeFirst()->GetFirstIndex();
         pIndex; pIndex = pIndex->GetNext())
    {
        sw::mark::IMark const*const pMark = pIndex->GetMark();
        if (dynamic_cast<const sw::mark::IBookmark*>(pMark) != nullptr)
        {   // need bookmark portions!
            return false;
        }
    }

    SwTwips nHeight = EmptyHeight();

    if (aSet.GetParaGrid().GetValue() &&
            IsInDocBody() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(FindPageFrame()));
        if ( pGrid )
            nHeight = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
    }

    SwRectFnSet aRectFnSet(this);
    SwTwips nChg = nHeight - aRectFnSet.GetHeight(getFramePrintArea());
    const SwBodyFrame* pBody = FindBodyFrame();
    if (pNonLastSplitFlyDrawObj && pBody)
    {
        // See if we need to increase the text frame height due to split flys. This is necessary for
        // anchors of inner floating tables, where moving to a next page moves indirectly, so we
        // want a correct text frame height.
        SwTwips nFrameBottom = aRectFnSet.GetBottom(getFrameArea()) + nChg;
        SwTwips nFlyBottom = aRectFnSet.GetBottom(pNonLastSplitFlyDrawObj->getFrameArea());
        SwTwips nBodyBottom = aRectFnSet.GetBottom(pBody->getFrameArea());
        if (nFlyBottom > nBodyBottom)
        {
            // This is the legacy case where flys may overlap with footer frames.
            nFlyBottom = nBodyBottom;
        }
        if (pNonLastSplitFlyDrawObj->isFrameAreaPositionValid() && nFlyBottom > nFrameBottom)
        {
            nChg += (nFlyBottom - nFrameBottom);
        }
    }

    if( !nChg )
        SetUndersized( false );
    AdjustFrame( nChg );

    if (GetHasRotatedPortions())
    {
        ClearPara();
        SetHasRotatedPortions(false);
    }

    RemoveFromCache();
    if( !IsEmpty() )
    {
        SetEmpty( true );
        SetCompletePaint();
    }
    if( !bCollapse && !bFirstFlyCheck &&
            aTextFly.IsOn() && aTextFly.IsAnyObj( aRect ) )
        return false;

    // #i35635# - call method <HideAndShowObjects()>
    // to assure that objects anchored at the empty paragraph are
    // correctly visible resp. invisible.
    HideAndShowObjects();
    return true;
}

bool SwTextFrame::FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff )
{
    const SwFrame *pFrame = this;
    rRegDiff = 0;
    while( !( ( SwFrameType::Body | SwFrameType::Fly )
           & pFrame->GetType() ) && pFrame->GetUpper() )
        pFrame = pFrame->GetUpper();
    if( ( SwFrameType::Body| SwFrameType::Fly ) & pFrame->GetType() )
    {
        SwRectFnSet aRectFnSet(pFrame);
        rRegStart = aRectFnSet.GetPrtTop(*pFrame);
        pFrame = pFrame->FindPageFrame();
        if( pFrame->IsPageFrame() )
        {
            SwPageDesc* pDesc = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(pFrame))->FindPageDesc();
            if( pDesc )
            {
                rRegDiff = pDesc->GetRegHeight();
                if( !rRegDiff )
                {
                    const SwTextFormatColl *pFormat = pDesc->GetRegisterFormatColl();
                    if( pFormat )
                    {
                        const SvxLineSpacingItem &rSpace = pFormat->GetLineSpacing();
                        if( SvxLineSpaceRule::Fix == rSpace.GetLineSpaceRule() )
                        {
                            rRegDiff = rSpace.GetLineHeight();
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( ( 4 * rRegDiff ) / 5 );
                        }
                        else
                        {
                            SwViewShell *pSh = getRootFrame()->GetCurrShell();
                            SwFontAccess aFontAccess( pFormat, pSh );
                            SwFont aFnt( aFontAccess.Get()->GetFont() );

                            OutputDevice *pOut = nullptr;
                            if( !pSh || !pSh->GetViewOptions()->getBrowseMode() ||
                                pSh->GetViewOptions()->IsPrtFormat() )
                                pOut = GetDoc().getIDocumentDeviceAccess().getReferenceDevice( true );

                            if( pSh && !pOut )
                                pOut = pSh->GetWin()->GetOutDev();

                            if( !pOut )
                                pOut = Application::GetDefaultDevice();

                            MapMode aOldMap( pOut->GetMapMode() );
                            pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

                            aFnt.ChgFnt( pSh, *pOut );
                            rRegDiff = aFnt.GetHeight( pSh, *pOut );
                            sal_uInt16 nNetHeight = rRegDiff;

                            switch( rSpace.GetLineSpaceRule() )
                            {
                                case SvxLineSpaceRule::Auto:
                                break;
                                case SvxLineSpaceRule::Min:
                                {
                                    if( rRegDiff < rSpace.GetLineHeight() )
                                        rRegDiff = rSpace.GetLineHeight();
                                    break;
                                }
                                default:
                                    OSL_FAIL( ": unknown LineSpaceRule" );
                            }
                            switch( rSpace.GetInterLineSpaceRule() )
                            {
                                case SvxInterLineSpaceRule::Off:
                                break;
                                case SvxInterLineSpaceRule::Prop:
                                {
                                    tools::Long nTmp = rSpace.GetPropLineSpace();
                                    if( nTmp < 50 )
                                        nTmp = nTmp ? 50 : 100;
                                    nTmp *= rRegDiff;
                                    nTmp /= 100;
                                    if( !nTmp )
                                        ++nTmp;
                                    rRegDiff = o3tl::narrowing<sal_uInt16>(nTmp);
                                    nNetHeight = rRegDiff;
                                    break;
                                }
                                case SvxInterLineSpaceRule::Fix:
                                {
                                    rRegDiff = rRegDiff + rSpace.GetInterLineSpace();
                                    nNetHeight = rRegDiff;
                                    break;
                                }
                                default: OSL_FAIL( ": unknown InterLineSpaceRule" );
                            }
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( rRegDiff - nNetHeight +
                                                 aFnt.GetAscent( pSh, *pOut ) );
                            pOut->SetMapMode( aOldMap );
                        }
                    }
                }
                const tools::Long nTmpDiff = pDesc->GetRegAscent() - rRegDiff;
                if ( aRectFnSet.IsVert() )
                    rRegStart -= nTmpDiff;
                else
                    rRegStart += nTmpDiff;
            }
        }
    }
    return ( 0 != rRegDiff );
}

void SwHiddenTextPortion::Paint( const SwTextPaintInfo & rInf) const
{
#ifdef DBG_UTIL
    OutputDevice* pOut = const_cast<OutputDevice*>(rInf.GetOut());
    Color aCol( rInf.GetOpt().GetFieldShadingsColor() );
    Color aOldColor( pOut->GetFillColor() );
    pOut->SetFillColor( aCol );
    Point aPos( rInf.GetPos() );
    aPos.AdjustY( -150 );
    aPos.AdjustX( -25 );
    SwRect aRect( aPos, Size( 100, 200 ) );
    pOut->DrawRect( aRect.SVRect() );
    pOut->SetFillColor( aOldColor );
#else
    (void)rInf;
#endif
}

bool SwHiddenTextPortion::Format( SwTextFormatInfo &rInf )
{
    Width( 0 );
    rInf.GetTextFrame()->HideFootnotes( rInf.GetIdx(), rInf.GetIdx() + GetLen() );

    return false;
};

bool SwControlCharPortion::DoPaint(SwTextPaintInfo const& rTextPaintInfo,
        OUString & rOutString, SwFont & rTmpFont, int &) const
{
    if (mcChar == CHAR_WJ || !rTextPaintInfo.GetOpt().IsViewMetaChars())
    {
        return false;
    }

    switch (mcChar)
    {
        case CHAR_ZWSP:
            rOutString = "/"; break;
//      case CHAR_LRM :
//          rText = sal_Unicode(0x2514); break;
//      case CHAR_RLM :
//          rText = sal_Unicode(0x2518); break;
        default:
            assert(false);
            break;
    }

    rTmpFont.SetEscapement( CHAR_ZWSP == mcChar ? DFLT_ESC_AUTO_SUB : -25 );
    rTmpFont.SetColor( NON_PRINTING_CHARACTER_COLOR );
    const sal_uInt16 nProp = 40;
    rTmpFont.SetProportion( nProp );  // a smaller font

    return true;
}

bool SwBookmarkPortion::DoPaint(SwTextPaintInfo const& rTextPaintInfo,
        OUString & rOutString, SwFont & rFont, int & rDeltaY) const
{
    // custom color is visible without field shading, too
    if (!rTextPaintInfo.GetOpt().IsShowBookmarks())
    {
        return false;
    }

    rOutString = OUStringChar(mcChar);

    // init font: we want OpenSymbol to ensure it doesn't look too crazy;
    // thin and a bit higher than the surrounding text
    auto const nOrigAscent(rFont.GetAscent(rTextPaintInfo.GetVsh(), *rTextPaintInfo.GetOut()));
    rFont.SetName("OpenSymbol", rFont.GetActual());
    Size aSize(rFont.GetSize(rFont.GetActual()));
    // use also the external leading (line gap) of the portion, but don't use
    // 100% of it because i can't figure out how to baseline align that
    assert(aSize.Height() != 0);
    auto const nFactor = aSize.Height() > 0 ? (Height() * 95) / aSize.Height() : Height();
    rFont.SetProportion(nFactor);
    rFont.SetWeight(WEIGHT_THIN, rFont.GetActual());
    rFont.SetColor(rTextPaintInfo.GetOpt().GetFieldShadingsColor());
    // reset these to default...
    rFont.SetAlign(ALIGN_BASELINE);
    rFont.SetUnderline(LINESTYLE_NONE);
    rFont.SetOverline(LINESTYLE_NONE);
    rFont.SetStrikeout(STRIKEOUT_NONE);
    rFont.SetOutline(false);
    rFont.SetShadow(false);
    rFont.SetTransparent(false);
    rFont.SetEmphasisMark(FontEmphasisMark::NONE);
    rFont.SetEscapement(0);
    rFont.SetPitch(PITCH_DONTKNOW, rFont.GetActual());
    rFont.SetRelief(FontRelief::NONE);

    // adjust Y position to account for different baselines of the fonts
    auto const nOSAscent(rFont.GetAscent(rTextPaintInfo.GetVsh(), *rTextPaintInfo.GetOut()));
    rDeltaY = nOSAscent - nOrigAscent;

    return true;
}

void SwControlCharPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( !Width() )  // is only set during prepaint mode
        return;

    rInf.DrawViewOpt(*this, GetWhichPor());

    int deltaY(0);
    SwFont aTmpFont( *rInf.GetFont() );
    OUString aOutString;

    if (!(rInf.OnWin()
        && !rInf.GetOpt().IsPagePreview()
        && !rInf.GetOpt().IsReadonly()
        && DoPaint(rInf, aOutString, aTmpFont, deltaY)))
        return;

    SwFontSave aFontSave( rInf, &aTmpFont );

    if ( !mnHalfCharWidth )
        mnHalfCharWidth = rInf.GetTextSize( aOutString ).Width() / 2;

    Point aOldPos = rInf.GetPos();
    Point aNewPos( aOldPos );
    auto const deltaX((Width() / 2) - mnHalfCharWidth);
    switch (rInf.GetFont()->GetOrientation(rInf.GetTextFrame()->IsVertical()).get())
    {
        case 0:
            aNewPos.AdjustX(deltaX);
            aNewPos.AdjustY(deltaY);
            break;
        case 900:
            aNewPos.AdjustY(-deltaX);
            aNewPos.AdjustX(deltaY);
            break;
        case 2700:
            aNewPos.AdjustY(deltaX);
            aNewPos.AdjustX(-deltaY);
            break;
        default:
            assert(false);
            break;
    }
    const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );

    rInf.DrawText( aOutString, *this );

    const_cast< SwTextPaintInfo& >( rInf ).SetPos( aOldPos );
}

void SwBookmarkPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( !Width() )  // is only set during prepaint mode
        return;

    rInf.DrawViewOpt(*this, GetWhichPor());

    int deltaY(0);
    SwFont aTmpFont( *rInf.GetFont() );
    OUString aOutString;

    if (!(rInf.OnWin()
        && !rInf.GetOpt().IsPagePreview()
        && !rInf.GetOpt().IsReadonly()
        && DoPaint(rInf, aOutString, aTmpFont, deltaY)))
        return;

    SwFontSave aFontSave( rInf, &aTmpFont );

    if ( !mnHalfCharWidth )
        mnHalfCharWidth = rInf.GetTextSize( aOutString ).Width() / 2;

    Point aOldPos = rInf.GetPos();
    Point aNewPos( aOldPos );
    auto const deltaX((Width() / 2) - mnHalfCharWidth);
    switch (rInf.GetFont()->GetOrientation(rInf.GetTextFrame()->IsVertical()).get())
    {
        case 0:
            aNewPos.AdjustX(deltaX);
            aNewPos.AdjustY(deltaY);
            break;
        case 900:
            aNewPos.AdjustY(-deltaX);
            aNewPos.AdjustX(deltaY);
            break;
        case 2700:
            aNewPos.AdjustY(deltaX);
            aNewPos.AdjustX(-deltaY);
            break;
        default:
            assert(false);
            break;
    }

    // draw end marks before the character position
    if ( m_nStart == 0 || m_nEnd == 0 )
    {
        // single type boundary marks are there outside of the bookmark text
        // some |text| here
        //     [[    ]]
        if (m_nStart > 1)
            aNewPos.AdjustX(mnHalfCharWidth * -2 * (m_aColors.size() - 1));
    }
    else if ( m_nStart != 0 && m_nEnd != 0 )
        // both end and start boundary marks: adjust them around the bookmark position
        // |te|xt|
        //  ]] [[
        aNewPos.AdjustX(mnHalfCharWidth * -(2 * m_nEnd - 1 + m_nPoint) );

    const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );

    for ( const auto& it : m_aColors )
    {
        // set bold for custom colored bookmark symbol
        // and draw multiple symbols showing all custom colors
        aTmpFont.SetWeight( COL_TRANSPARENT == std::get<1>(it) ? WEIGHT_THIN : WEIGHT_BOLD, aTmpFont.GetActual() );
        aTmpFont.SetColor( COL_TRANSPARENT == std::get<1>(it) ? rInf.GetOpt().GetFieldShadingsColor() : std::get<1>(it) );
        aOutString = OUString(std::get<0>(it) == SwScriptInfo::MarkKind::Start ? '[' : ']');

        // MarkKind::Point: drawn I-beam (e.g. U+2336) as overlapping ][
        if ( std::get<0>(it) == SwScriptInfo::MarkKind::Point )
        {
            aNewPos.AdjustX(-mnHalfCharWidth * 5/16);
            const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );
            rInf.DrawText( aOutString, *this );

            // when the overlapping vertical lines are 50 pixel width on the screen,
            // this distance (half width * 5/8) still results precise overlapping
            aNewPos.AdjustX(mnHalfCharWidth * 5/8);
            const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );
            aOutString = OUString('[');
        }
        rInf.DrawText( aOutString, *this );
        // place the next symbol after the previous one
        // TODO: fix orientation and start/end
        aNewPos.AdjustX(mnHalfCharWidth * 2);
        const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );
    }

    const_cast< SwTextPaintInfo& >( rInf ).SetPos( aOldPos );
}

void SwBookmarkPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    OUStringBuffer aStr;
    for ( const auto& it : m_aColors )
    {
        aStr.append("#" + std::get<2>(it) + " " + SwResId(STR_BOOKMARK_DEF_NAME));
        switch (std::get<0>(it))
        {
            case SwScriptInfo::MarkKind::Point:
                break;
            case SwScriptInfo::MarkKind::Start:
                aStr.append(" " + SwResId(STR_CAPTION_BEGINNING));
                break;
            case SwScriptInfo::MarkKind::End:
                aStr.append(" " + SwResId(STR_CAPTION_END));
                break;
        }
    }

    rPH.Special( GetLen(), aStr.makeStringAndClear(), GetWhichPor() );
}

void SwBookmarkPortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText, TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwBookmarkPortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    if (!m_aColors.empty())
    {
        OUStringBuffer aStr;
        for (const auto& rColor : m_aColors)
        {
            aStr.append("#" + std::get<2>(rColor) + " " + SwResId(STR_BOOKMARK_DEF_NAME));
            switch (std::get<0>(rColor))
            {
                case SwScriptInfo::MarkKind::Point:
                    break;
                case SwScriptInfo::MarkKind::Start:
                    aStr.append(" " + SwResId(STR_CAPTION_BEGINNING));
                    break;
                case SwScriptInfo::MarkKind::End:
                    aStr.append(" " + SwResId(STR_CAPTION_END));
                    break;
            }
        }
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("colors"),
                                          BAD_CAST(aStr.makeStringAndClear().toUtf8().getStr()));
    }

    (void)xmlTextWriterEndElement(pWriter);
}

bool SwControlCharPortion::Format( SwTextFormatInfo &rInf )
{
    const SwLinePortion* pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );

    return false;
}

SwTwips SwControlCharPortion::GetViewWidth(const SwTextSizeInfo& rInf) const
{
    if( !mnViewWidth )
        mnViewWidth = rInf.GetTextSize(OUString(' ')).Width();

    return mnViewWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
