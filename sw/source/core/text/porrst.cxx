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

#include <hintids.hxx>
#include <sfx2/printer.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/pgrditem.hxx>
#include <vcl/svapp.hxx>
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
#include "porfly.hxx"
#include "atrhndl.hxx"
#include <rootfrm.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <crsrsh.hxx>

SwTmpEndPortion::SwTmpEndPortion( const SwLinePortion &rPortion )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    SetWhichPor( PortionType::TempEnd );
}

void SwTmpEndPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if (rInf.OnWin() && rInf.GetOpt().IsParagraph())
    {
        const SwFont* pOldFnt = rInf.GetFont();

        SwFont aFont(*pOldFnt);
        aFont.SetColor(NON_PRINTING_CHARACTER_COLOR);
        const_cast<SwTextPaintInfo&>(rInf).SetFont(&aFont);

        // draw the pilcrow
        rInf.DrawText(OUString(CH_PAR), *this);

        const_cast<SwTextPaintInfo&>(rInf).SetFont(const_cast<SwFont*>(pOldFnt));
    }
}

SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion )
    : SwLinePortion( rPortion )
{
    nLineLength = TextFrameIndex(1);
    SetWhichPor( PortionType::Break );
}

TextFrameIndex SwBreakPortion::GetCursorOfst(const sal_uInt16) const
{
    return TextFrameIndex(0);
}

sal_uInt16 SwBreakPortion::GetViewWidth( const SwTextSizeInfo & ) const
{ return 0; }

SwLinePortion *SwBreakPortion::Compress()
{ return (GetNextPortion() && GetNextPortion()->InTextGrp() ? nullptr : this); }

void SwBreakPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsLineBreak() )
        rInf.DrawLineBreak( *this );
}

bool SwBreakPortion::Format( SwTextFormatInfo &rInf )
{
    const SwLinePortion *pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );
    if (rInf.GetIdx() + TextFrameIndex(1) == TextFrameIndex(rInf.GetText().getLength()))
        rInf.SetNewLine( true );
    return true;
}

void SwBreakPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

SwKernPortion::SwKernPortion( SwLinePortion &rPortion, short nKrn,
                              bool bBG, bool bGK ) :
    nKern( nKrn ), bBackground( bBG ), bGridKern( bGK )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    nLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Kern );
    if( nKern > 0 )
        Width( nKern );
    rPortion.Insert( this );
}

SwKernPortion::SwKernPortion( const SwLinePortion& rPortion ) :
    nKern( 0 ), bBackground( false ), bGridKern( true )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );

    nLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Kern );
}

void SwKernPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        // bBackground is set for Kerning Portions between two fields
        if ( bBackground )
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
}

void SwKernPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if ( bGridKern )
        return;

    if( rInf.GetLast() == this )
        rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
    if( nKern < 0 )
        Width( -nKern );
    else
        Width( 0 );
    rInf.GetLast()->FormatEOL( rInf );
}

SwArrowPortion::SwArrowPortion( const SwLinePortion &rPortion ) :
    bLeft( true )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    nLineLength = TextFrameIndex(0);
    SetWhichPor( PortionType::Arrow );
}

SwArrowPortion::SwArrowPortion( const SwTextPaintInfo &rInf )
    : bLeft( false )
{
    Height( static_cast<sal_uInt16>(rInf.GetTextFrame()->getFramePrintArea().Height()) );
    aPos.setX( rInf.GetTextFrame()->getFrameArea().Left() +
               rInf.GetTextFrame()->getFramePrintArea().Right() );
    aPos.setY( rInf.GetTextFrame()->getFrameArea().Top() +
               rInf.GetTextFrame()->getFramePrintArea().Bottom() );
    SetWhichPor( PortionType::Arrow );
}

void SwArrowPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    const_cast<SwArrowPortion*>(this)->aPos = rInf.GetPos();
}

SwLinePortion *SwArrowPortion::Compress() { return this; }

SwTwips SwTextFrame::EmptyHeight() const
{
    if (IsCollapse()) {
        SwViewShell *pSh = getRootFrame()->GetCurrShell();
        if ( dynamic_cast<const SwCursorShell*>( pSh ) !=  nullptr ) {
            SwCursorShell *pCrSh = static_cast<SwCursorShell*>(pSh);
            SwContentFrame *pCurrFrame=pCrSh->GetCurrFrame();
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
        pFnt->SetVertical( 2700 );

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
        const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
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
    if (HasFollow() || GetMergedPara() || GetTextNodeFirst()->GetpSwpHints() ||
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
        aSet.GetLRSpace().IsAutoFirst() ) )
        return false;

    SwTextFly aTextFly( this );
    SwRect aRect;
    bool bFirstFlyCheck = 0 != getFramePrintArea().Height();
    if ( !bCollapse && bFirstFlyCheck &&
            aTextFly.IsOn() && aTextFly.IsAnyObj( aRect ) )
        return false;

    SwTwips nHeight = EmptyHeight();

    if (aSet.GetParaGrid().GetValue() &&
            IsInDocBody() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(FindPageFrame()));
        if ( pGrid )
            nHeight = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
    }

    SwRectFnSet aRectFnSet(this);
    const SwTwips nChg = nHeight - aRectFnSet.GetHeight(getFramePrintArea());

    if( !nChg )
        SetUndersized( false );
    AdjustFrame( nChg );

    if( HasBlinkPor() )
    {
        ClearPara();
        ResetBlinkPor();
    }
    SetCacheIdx( USHRT_MAX );
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
                                pOut = pSh->GetWin();

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
                                    long nTmp = rSpace.GetPropLineSpace();
                                    if( nTmp < 50 )
                                        nTmp = nTmp ? 50 : 100;
                                    nTmp *= rRegDiff;
                                    nTmp /= 100;
                                    if( !nTmp )
                                        ++nTmp;
                                    rRegDiff = static_cast<sal_uInt16>(nTmp);
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
                const long nTmpDiff = pDesc->GetRegAscent() - rRegDiff;
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
    Color aCol( SwViewOption::GetFieldShadingsColor() );
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

void SwControlCharPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( Width() )  // is only set during prepaint mode
    {
        rInf.DrawViewOpt( *this, PortionType::ControlChar );

        if ( !rInf.GetOpt().IsPagePreview() &&
             !rInf.GetOpt().IsReadonly() &&
              SwViewOption::IsFieldShadings() &&
              CHAR_ZWNBSP != mcChar )
        {
            SwFont aTmpFont( *rInf.GetFont() );
            aTmpFont.SetEscapement( CHAR_ZWSP == mcChar ? DFLT_ESC_AUTO_SUB : -25 );
            const sal_uInt16 nProp = 40;
            aTmpFont.SetProportion( nProp );  // a smaller font
            SwFontSave aFontSave( rInf, &aTmpFont );

            OUString aOutString;

            switch ( mcChar )
            {
                case CHAR_ZWSP :
                    aOutString = "/"; break;
//                case CHAR_LRM :
//                    rText = sal_Unicode(0x2514); break;
//                case CHAR_RLM :
//                    rText = sal_Unicode(0x2518); break;
            }

            if ( !mnHalfCharWidth )
                mnHalfCharWidth = rInf.GetTextSize( aOutString ).Width() / 2;

            Point aOldPos = rInf.GetPos();
            Point aNewPos( aOldPos );
            aNewPos.setX( aNewPos.X() + ( Width() / 2 ) - mnHalfCharWidth );
            const_cast< SwTextPaintInfo& >( rInf ).SetPos( aNewPos );

            rInf.DrawText( aOutString, *this );

            const_cast< SwTextPaintInfo& >( rInf ).SetPos( aOldPos );
        }
    }
}

bool SwControlCharPortion::Format( SwTextFormatInfo &rInf )
{
    const SwLinePortion* pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );

    return false;
}

sal_uInt16 SwControlCharPortion::GetViewWidth( const SwTextSizeInfo& rInf ) const
{
    if( !mnViewWidth )
        mnViewWidth = rInf.GetTextSize(OUString(' ')).Width();

    return mnViewWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
