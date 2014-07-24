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
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <paratr.hxx>
#include <SwPortionHandler.hxx>
#include <porrst.hxx>
#include <inftxt.hxx>
#include <txtpaint.hxx>
#include <swfntcch.hxx>
#include <tgrditem.hxx>
#include <pagedesc.hxx>
#include <frmatr.hxx>
#include <redlnitr.hxx>
#include <porfly.hxx>
#include <atrhndl.hxx>
#include "rootfrm.hxx"

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <crsrsh.hxx>

SwTmpEndPortion::SwTmpEndPortion( const SwLinePortion &rPortion )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    SetWhichPor( POR_TMPEND );
}

void SwTmpEndPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if (rInf.OnWin() && rInf.GetOpt().IsParagraph())
    {
        #ifdef LEGACY_NON_PRINTING_CHARACTER_COLOR_FUNCTIONALITY
        const OUString aTmp( CH_PAR );
        rInf.DrawText( aTmp, *this );
        #else
        const SwFont* pOldFnt = rInf.GetFont();

        SwFont aFont(*pOldFnt);
        aFont.SetColor(NON_PRINTING_CHARACTER_COLOR);
        const_cast<SwTxtPaintInfo&>(rInf).SetFont(&aFont);

        // draw the pilcrow
        rInf.DrawText(OUString(CH_PAR), *this);

        const_cast<SwTxtPaintInfo&>(rInf).SetFont(const_cast<SwFont*>(pOldFnt));
        #endif
    }
}

SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion )
    : SwLinePortion( rPortion )
{
    nLineLength = 1;
    SetWhichPor( POR_BRK );
}

sal_Int32 SwBreakPortion::GetCrsrOfst( const KSHORT ) const
{ return 0; }

KSHORT SwBreakPortion::GetViewWidth( const SwTxtSizeInfo & ) const
{ return 0; }

SwLinePortion *SwBreakPortion::Compress()
{ return (GetPortion() && GetPortion()->InTxtGrp() ? 0 : this); }

void SwBreakPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsLineBreak() )
        rInf.DrawLineBreak( *this );
}

bool SwBreakPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion *pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );
    if ( rInf.GetIdx()+1 == rInf.GetTxt().getLength() )
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
    nLineLength = 0;
    SetWhichPor( POR_KERN );
    if( nKern > 0 )
        Width( nKern );
     rPortion.Insert( this );
}

SwKernPortion::SwKernPortion( const SwLinePortion& rPortion ) :
    nKern( 0 ), bBackground( false ), bGridKern( true )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );

    nLineLength = 0;
    SetWhichPor( POR_KERN );
}

void SwKernPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        // bBackground is set for Kerning Portions between two fields
        if ( bBackground )
            rInf.DrawViewOpt( *this, POR_FLD );

        rInf.DrawBackBrush( *this );

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

        if( rInf.GetFont()->IsPaintBlank() )
        {
            OUString aTxtDouble("  ");

            SwRect aClipRect;
            rInf.CalcRect( *this, &aClipRect, 0 );
            SwSaveClip aClip( (OutputDevice*)rInf.GetOut() );
            aClip.ChgClip( aClipRect, 0 );
            rInf.DrawText( aTxtDouble, *this, 0, 2, true );
        }
    }
}

void SwKernPortion::FormatEOL( SwTxtFormatInfo &rInf )
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
    nLineLength = 0;
    SetWhichPor( POR_ARROW );
}

SwArrowPortion::SwArrowPortion( const SwTxtPaintInfo &rInf )
    : bLeft( false )
{
    Height( (sal_uInt16)(rInf.GetTxtFrm()->Prt().Height()) );
    aPos.X() = rInf.GetTxtFrm()->Frm().Left() +
               rInf.GetTxtFrm()->Prt().Right();
    aPos.Y() = rInf.GetTxtFrm()->Frm().Top() +
               rInf.GetTxtFrm()->Prt().Bottom();
    SetWhichPor( POR_ARROW );
}

void SwArrowPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    ((SwArrowPortion*)this)->aPos = rInf.GetPos();
}

SwLinePortion *SwArrowPortion::Compress() { return this; }

SwTwips SwTxtFrm::EmptyHeight() const
{
    if (IsCollapse()) {
        SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( pSh->IsA( TYPE(SwCrsrShell) ) ) {
            SwCrsrShell *pCrSh=(SwCrsrShell*)pSh;
            SwCntntFrm *pCurrFrm=pCrSh->GetCurrFrm();
            if (pCurrFrm==(SwCntntFrm*)this) {
                // do nothing
            } else {
                return 1;
            }
        } else {
            return 1;
        }
    }
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::EmptyHeight with swapped frame" );

    SwFont *pFnt;
    const SwTxtNode& rTxtNode = *GetTxtNode();
    const IDocumentSettingAccess* pIDSA = rTxtNode.getIDocumentSettingAccess();
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( rTxtNode.HasSwAttrSet() )
    {
        const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
        pFnt = new SwFont( pAttrSet, pIDSA );
    }
    else
    {
        SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh);
        pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }

    if ( IsVertical() )
        pFnt->SetVertical( 2700 );

    OutputDevice* pOut = pSh ? pSh->GetOut() : 0;
    if ( !pOut || !pSh->GetViewOptions()->getBrowseMode() ||
         pSh->GetViewOptions()->IsPrtFormat() )
    {
        pOut = rTxtNode.getIDocumentDeviceAccess()->getReferenceDevice(true);
    }

    const IDocumentRedlineAccess* pIDRA = rTxtNode.getIDocumentRedlineAccess();
    if( IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
    {
        MSHORT nRedlPos = pIDRA->GetRedlinePos( rTxtNode, USHRT_MAX );
        if( MSHRT_MAX != nRedlPos )
        {
            SwAttrHandler aAttrHandler;
            aAttrHandler.Init(  GetTxtNode()->GetSwAttrSet(),
                               *GetTxtNode()->getIDocumentSettingAccess(), NULL );
            SwRedlineItr aRedln( rTxtNode, *pFnt, aAttrHandler,
                                 nRedlPos, true );
        }
    }

    SwTwips nRet;
    if( !pOut )
        nRet = IsVertical() ?
               Prt().SSize().Width() + 1 :
               Prt().SSize().Height() + 1;
    else
    {
        pFnt->SetFntChg( true );
        pFnt->ChgPhysFnt( pSh, *pOut );
        nRet = pFnt->GetHeight( pSh, *pOut );
    }
    delete pFnt;
    return nRet;
}

bool SwTxtFrm::FormatEmpty()
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::FormatEmpty with swapped frame" );

    bool bCollapse = EmptyHeight( ) == 1 && this->IsCollapse( );

    if ( HasFollow() || GetTxtNode()->GetpSwpHints() ||
        0 != GetTxtNode()->GetNumRule() ||
        GetTxtNode()->HasHiddenCharAttribute( true ) ||
         IsInFtn() || ( HasPara() && GetPara()->IsPrepMustFit() ) )
        return false;
    const SwAttrSet& aSet = GetTxtNode()->GetSwAttrSet();
    const SvxAdjust nAdjust = aSet.GetAdjust().GetAdjust();
    if( !bCollapse && ( ( ( ! IsRightToLeft() && ( SVX_ADJUST_LEFT != nAdjust ) ) ||
          (   IsRightToLeft() && ( SVX_ADJUST_RIGHT != nAdjust ) ) ) ||
          aSet.GetRegister().GetValue() ) )
        return false;
    const SvxLineSpacingItem &rSpacing = aSet.GetLineSpacing();
    if( !bCollapse && ( SVX_LINE_SPACE_MIN == rSpacing.GetLineSpaceRule() ||
        SVX_LINE_SPACE_FIX == rSpacing.GetLineSpaceRule() ||
        aSet.GetLRSpace().IsAutoFirst() ) )
        return false;

    SwTxtFly aTxtFly( this );
    SwRect aRect;
    bool bFirstFlyCheck = 0 != Prt().Height();
    if ( !bCollapse && bFirstFlyCheck &&
            aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
        return false;

    SwTwips nHeight = EmptyHeight();

    if ( GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() &&
            IsInDocBody() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(FindPageFrm()));
        if ( pGrid )
            nHeight = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
    }

    SWRECTFN( this )
    const SwTwips nChg = nHeight - (Prt().*fnRect->fnGetHeight)();

    if( !nChg )
        SetUndersized( false );
    AdjustFrm( nChg );

    if( HasBlinkPor() )
    {
        ClearPara();
        ResetBlinkPor();
    }
    SetCacheIdx( MSHRT_MAX );
    if( !IsEmpty() )
    {
        SetEmpty( true );
        SetCompletePaint();
    }
    if( !bCollapse && !bFirstFlyCheck &&
            aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
        return false;

    // #i35635# - call method <HideAndShowObjects()>
    // to assure that objects anchored at the empty paragraph are
    // correctly visible resp. invisible.
    HideAndShowObjects();
    return true;
}

bool SwTxtFrm::FillRegister( SwTwips& rRegStart, KSHORT& rRegDiff )
{
    const SwFrm *pFrm = this;
    rRegDiff = 0;
    while( !( ( FRM_BODY | FRM_FLY )
           & pFrm->GetType() ) && pFrm->GetUpper() )
        pFrm = pFrm->GetUpper();
    if( ( FRM_BODY| FRM_FLY ) & pFrm->GetType() )
    {
        SWRECTFN( pFrm )
        rRegStart = (pFrm->*fnRect->fnGetPrtTop)();
        pFrm = pFrm->FindPageFrm();
        if( pFrm->IsPageFrm() )
        {
            SwPageDesc* pDesc = ((SwPageFrm*)pFrm)->FindPageDesc();
            if( pDesc )
            {
                rRegDiff = pDesc->GetRegHeight();
                if( !rRegDiff )
                {
                    const SwTxtFmtColl *pFmt = pDesc->GetRegisterFmtColl();
                    if( pFmt )
                    {
                        const SvxLineSpacingItem &rSpace = pFmt->GetLineSpacing();
                        if( SVX_LINE_SPACE_FIX == rSpace.GetLineSpaceRule() )
                        {
                            rRegDiff = rSpace.GetLineHeight();
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( ( 4 * rRegDiff ) / 5 );
                        }
                        else
                        {
                            SwViewShell *pSh = getRootFrm()->GetCurrShell();
                            SwFontAccess aFontAccess( pFmt, pSh );
                            SwFont aFnt( *aFontAccess.Get()->GetFont() );

                            OutputDevice *pOut = 0;
                            if( !pSh || !pSh->GetViewOptions()->getBrowseMode() ||
                                pSh->GetViewOptions()->IsPrtFormat() )
                                pOut = GetTxtNode()->getIDocumentDeviceAccess()->getReferenceDevice( true );

                            if( pSh && !pOut )
                                pOut = pSh->GetWin();

                            if( !pOut )
                                pOut = GetpApp()->GetDefaultDevice();

                            MapMode aOldMap( pOut->GetMapMode() );
                            pOut->SetMapMode( MapMode( MAP_TWIP ) );

                            aFnt.ChgFnt( pSh, *pOut );
                            rRegDiff = aFnt.GetHeight( pSh, *pOut );
                            KSHORT nNettoHeight = rRegDiff;

                            switch( rSpace.GetLineSpaceRule() )
                            {
                                case SVX_LINE_SPACE_AUTO:
                                break;
                                case SVX_LINE_SPACE_MIN:
                                {
                                    if( rRegDiff < KSHORT( rSpace.GetLineHeight() ) )
                                        rRegDiff = rSpace.GetLineHeight();
                                    break;
                                }
                                default:
                                    OSL_FAIL( ": unknown LineSpaceRule" );
                            }
                            switch( rSpace.GetInterLineSpaceRule() )
                            {
                                case SVX_INTER_LINE_SPACE_OFF:
                                break;
                                case SVX_INTER_LINE_SPACE_PROP:
                                {
                                    long nTmp = rSpace.GetPropLineSpace();
                                    if( nTmp < 50 )
                                        nTmp = nTmp ? 50 : 100;
                                    nTmp *= rRegDiff;
                                    nTmp /= 100;
                                    if( !nTmp )
                                        ++nTmp;
                                    rRegDiff = (KSHORT)nTmp;
                                    nNettoHeight = rRegDiff;
                                    break;
                                }
                                case SVX_INTER_LINE_SPACE_FIX:
                                {
                                    rRegDiff = rRegDiff + rSpace.GetInterLineSpace();
                                    nNettoHeight = rRegDiff;
                                    break;
                                }
                                default: OSL_FAIL( ": unknown InterLineSpaceRule" );
                            }
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( rRegDiff - nNettoHeight +
                                                 aFnt.GetAscent( pSh, *pOut ) );
                            pOut->SetMapMode( aOldMap );
                        }
                    }
                }
                const long nTmpDiff = pDesc->GetRegAscent() - rRegDiff;
                if ( bVert )
                    rRegStart -= nTmpDiff;
                else
                    rRegStart += nTmpDiff;
            }
        }
    }
    return ( 0 != rRegDiff );
}

void SwHiddenTextPortion::Paint( const SwTxtPaintInfo & rInf) const
{
    (void)rInf;
#ifdef DBG_UTIL
    OutputDevice* pOut = (OutputDevice*)rInf.GetOut();
    Color aCol( SwViewOption::GetFieldShadingsColor() );
    Color aOldColor( pOut->GetFillColor() );
    pOut->SetFillColor( aCol );
    Point aPos( rInf.GetPos() );
    aPos.Y() -= 150;
    aPos.X() -= 25;
    SwRect aRect( aPos, Size( 100, 200 ) );
    ((OutputDevice*)pOut)->DrawRect( aRect.SVRect() );
    pOut->SetFillColor( aOldColor );
#endif
}

bool SwHiddenTextPortion::Format( SwTxtFormatInfo &rInf )
{
    Width( 0 );
    rInf.GetTxtFrm()->HideFootnotes( rInf.GetIdx(), rInf.GetIdx() + GetLen() );

    return false;
};

void SwControlCharPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if ( Width() )  // is only set during prepaint mode
    {
        rInf.DrawViewOpt( *this, POR_CONTROLCHAR );

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
//                    rTxt = sal_Unicode(0x2514); break;
//                case CHAR_RLM :
//                    rTxt = sal_Unicode(0x2518); break;
            }

            if ( !mnHalfCharWidth )
                mnHalfCharWidth = rInf.GetTxtSize( aOutString ).Width() / 2;

            Point aOldPos = rInf.GetPos();
            Point aNewPos( aOldPos );
            aNewPos.X() = aNewPos.X() + ( Width() / 2 ) - mnHalfCharWidth;
            const_cast< SwTxtPaintInfo& >( rInf ).SetPos( aNewPos );

            rInf.DrawText( aOutString, *this );

            const_cast< SwTxtPaintInfo& >( rInf ).SetPos( aOldPos );
        }
    }
}

bool SwControlCharPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion* pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );

    return false;
}

KSHORT SwControlCharPortion::GetViewWidth( const SwTxtSizeInfo& rInf ) const
{
    if( !mnViewWidth )
        mnViewWidth = rInf.GetTxtSize(OUString(' ')).Width();

    return mnViewWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
