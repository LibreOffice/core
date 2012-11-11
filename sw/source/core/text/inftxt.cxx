/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/uno/Sequence.h>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <hintids.hxx>
#include <sfx2/printer.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/tstpitem.hxx>
#include <xmloff/odffields.hxx>

#include <SwSmartTagMgr.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/unolingu.hxx>
#include <breakit.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <txatbase.hxx>
#include <fmtinfmt.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <PostItMgr.hxx>
#include <swmodule.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <viewsh.hxx>   // ViewShell
#include <viewopt.hxx>  // SwViewOptions
#include <frmtool.hxx>  // DrawGraphic
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <paratr.hxx>   // SwFmtDrop
#include <rootfrm.hxx>  // SwRootFrm
#include <inftxt.hxx>   // SwTxtInfo
#include <blink.hxx>    // SwBlink
#include <noteurl.hxx>  // SwNoteURL
#include <porftn.hxx>   // SwFtnPortion
#include <porrst.hxx>   // SwHangingPortion
#include <itratr.hxx>
#include <accessibilityoptions.hxx>
#include <wrong.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <SwGrammarMarkUp.hxx>
#include <cstdio>
// #i12836# enhanced pdf export
#include <EnhancedPDFExportHelper.hxx>

#include <unomid.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


#define CHAR_UNDERSCORE ((sal_Unicode)0x005F)
#define CHAR_LEFT_ARROW ((sal_Unicode)0x25C0)
#define CHAR_RIGHT_ARROW ((sal_Unicode)0x25B6)
#define CHAR_TAB ((sal_Unicode)0x2192)
#define CHAR_TAB_RTL ((sal_Unicode)0x2190)
#define CHAR_LINEBREAK ((sal_Unicode)0x21B5)
#define CHAR_LINEBREAK_RTL ((sal_Unicode)0x21B3)

#define DRAW_SPECIAL_OPTIONS_CENTER 1
#define DRAW_SPECIAL_OPTIONS_ROTATE 2

// variable moved to class <numfunc:GetDefBulletConfig>
//extern const sal_Char sBulletFntName[];
namespace numfunc
{
    extern const String& GetDefBulletFontname();
    extern bool IsDefBulletFontUserDefined();
}

#ifdef DBG_UTIL
sal_Bool SwTxtSizeInfo::IsOptDbg() const { return GetOpt().IsTest4(); }
#endif

SwLineInfo::SwLineInfo()
    : pRuler( 0 ),
      pSpace( 0 ),
      nVertAlign( 0 ),
      nDefTabStop( 0 ),
      bListTabStopIncluded( false ),
      nListTabStopPosition( 0 )
{
}

SwLineInfo::~SwLineInfo()
{
    delete pRuler;
}
void SwLineInfo::CtorInitLineInfo( const SwAttrSet& rAttrSet,
                                   const SwTxtNode& rTxtNode )
{
    delete pRuler;
    pRuler = new SvxTabStopItem( rAttrSet.GetTabStops() );
    if ( rTxtNode.GetListTabStopPosition( nListTabStopPosition ) )
    {
        bListTabStopIncluded = true;

        // insert the list tab stop into SvxTabItem instance <pRuler>
        const SvxTabStop aListTabStop( nListTabStopPosition,
                                       SVX_TAB_ADJUST_LEFT );
        pRuler->Insert( aListTabStop );

        // remove default tab stops, which are before the inserted list tab stop
        for ( sal_uInt16 i = 0; i < pRuler->Count(); i++ )
        {
            if ( (*pRuler)[i].GetTabPos() < nListTabStopPosition &&
                 (*pRuler)[i].GetAdjustment() == SVX_TAB_ADJUST_DEFAULT )
            {
                pRuler->Remove(i);
                continue;
            }
        }
    }

    if ( !rTxtNode.getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) )
    {
        // remove default tab stop at position 0
        for ( sal_uInt16 i = 0; i < pRuler->Count(); i++ )
        {
            if ( (*pRuler)[i].GetTabPos() == 0 &&
                 (*pRuler)[i].GetAdjustment() == SVX_TAB_ADJUST_DEFAULT )
            {
                pRuler->Remove(i);
                break;
            }
        }
    }

    pSpace = &rAttrSet.GetLineSpacing();
    nVertAlign = rAttrSet.GetParaVertAlign().GetValue();
    nDefTabStop = MSHRT_MAX;
}

void SwTxtInfo::CtorInitTxtInfo( SwTxtFrm *pFrm )
{
    pPara = pFrm->GetPara();
    nTxtStart = pFrm->GetOfst();
    if( !pPara )
    {
        OSL_ENSURE( pPara, "+SwTxtInfo::CTOR: missing paragraph information" );
        pFrm->Format();
        pPara = pFrm->GetPara();
    }
}

SwTxtInfo::SwTxtInfo( const SwTxtInfo &rInf )
    : pPara( ((SwTxtInfo&)rInf).GetParaPortion() ),
      nTxtStart( rInf.GetTxtStart() )
{ }


#if OSL_DEBUG_LEVEL > 0

void ChkOutDev( const SwTxtSizeInfo &rInf )
{
    if ( !rInf.GetVsh() )
        return;

    const OutputDevice* pOut = rInf.GetOut();
    const OutputDevice* pRef = rInf.GetRefDev();
    OSL_ENSURE( pOut && pRef, "ChkOutDev: invalid output devices" );
}
#endif


inline xub_StrLen GetMinLen( const SwTxtSizeInfo &rInf )
{
    const xub_StrLen nInfLen = rInf.GetIdx() + rInf.GetLen();
    return Min( rInf.GetTxt().Len(), nInfLen );
}


SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew )
    : SwTxtInfo( rNew ),
      pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
      pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
      pOut(((SwTxtSizeInfo&)rNew).GetOut()),
      pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
      pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
      pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
      pFrm(rNew.pFrm),
      pOpt(&rNew.GetOpt()),
      pTxt(&rNew.GetTxt()),
      nIdx(rNew.GetIdx()),
      nLen(rNew.GetLen()),
      nKanaIdx( rNew.GetKanaIdx() ),
      bOnWin( rNew.OnWin() ),
      bNotEOL( rNew.NotEOL() ),
      bURLNotify( rNew.URLNotify() ),
      bStopUnderFlow( rNew.StopUnderFlow() ),
      bFtnInside( rNew.IsFtnInside() ),
      bOtherThanFtnInside( rNew.IsOtherThanFtnInside() ),
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      bSnapToGrid( rNew.SnapToGrid() ),
      nDirection( rNew.GetDirection() )
{
#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif
}

void SwTxtSizeInfo::CtorInitTxtSizeInfo( SwTxtFrm *pFrame, SwFont *pNewFnt,
                   const xub_StrLen nNewIdx, const xub_StrLen nNewLen )
{
    pKanaComp = NULL;
    nKanaIdx = 0;
    pFrm = pFrame;
    CtorInitTxtInfo( pFrm );
    const SwTxtNode *pNd = pFrm->GetTxtNode();
    pVsh = pFrm->getRootFrm()->GetCurrShell();

    // Get the output and reference device
    if ( pVsh )
    {
        pOut = pVsh->GetOut();
        pRef = &pVsh->GetRefDev();
        bOnWin = pVsh->GetWin() || OUTDEV_WINDOW == pOut->GetOutDevType();
    }
    else
    {
        // Access via StarONE. We do not need a Shell or an active one.
        if ( pNd->getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE) )
        {
            // We can only pick the AppWin here? (there's nothing better to pick?)
            pOut = GetpApp()->GetDefaultDevice();
        }
        else
            pOut = pNd->getIDocumentDeviceAccess()->getPrinter( false );

        pRef = pOut;
    }

#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif

    // Set default layout mode ( LTR or RTL ).
    if ( pFrm->IsRightToLeft() )
    {
        pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
        pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
        nDirection = DIR_RIGHT2LEFT;
    }
    else
    {
        pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
        pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
        nDirection = DIR_LEFT2RIGHT;
    }

    //
    // The Options
    //
    pOpt = pVsh ?
           pVsh->GetViewOptions() :
           SW_MOD()->GetViewOption( pNd->getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE) ); // Options from Module, due to StarONE

    // bURLNotify is set if MakeGraphic prepares it
    // TODO: Unwind
    bURLNotify = pNoteURL && !bOnWin;

    SetSnapToGrid( pNd->GetSwAttrSet().GetParaGrid().GetValue() &&
                   pFrm->IsInDocBody() );

    pFnt = pNewFnt;
    pUnderFnt = 0;
    pTxt = &pNd->GetTxt();

    nIdx = nNewIdx;
    nLen = nNewLen;
    bNotEOL = sal_False;
    bStopUnderFlow = bFtnInside = bOtherThanFtnInside = sal_False;
    bMulti = bFirstMulti = bRuby = bHanging = bScriptSpace =
        bForbiddenChars = sal_False;

    SetLen( GetMinLen( *this ) );
}

SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew, const XubString &rTxt,
                              const xub_StrLen nIndex, const xub_StrLen nLength )
    : SwTxtInfo( rNew ),
      pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
      pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
      pOut(((SwTxtSizeInfo&)rNew).GetOut()),
      pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
      pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
      pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
      pFrm( rNew.pFrm ),
      pOpt(&rNew.GetOpt()),
      pTxt(&rTxt),
      nIdx(nIndex),
      nLen(nLength),
      nKanaIdx( rNew.GetKanaIdx() ),
      bOnWin( rNew.OnWin() ),
      bNotEOL( rNew.NotEOL() ),
      bURLNotify( rNew.URLNotify() ),
      bStopUnderFlow( rNew.StopUnderFlow() ),
      bFtnInside( rNew.IsFtnInside() ),
      bOtherThanFtnInside( rNew.IsOtherThanFtnInside() ),
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      bSnapToGrid( rNew.SnapToGrid() ),
      nDirection( rNew.GetDirection() )
{
#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif
    SetLen( GetMinLen( *this ) );
}

void SwTxtSizeInfo::SelectFont()
{
     // The path needs to go via ChgPhysFnt or the FontMetricCache gets confused.
     // In this case pLastMet has it's old value.
     // Wrong: GetOut()->SetFont( GetFont()->GetFnt() );
    GetFont()->Invalidate();
    GetFont()->ChgPhysFnt( pVsh, *GetOut() );
}

void SwTxtSizeInfo::NoteAnimation() const
{
    if( OnWin() )
        SwRootFrm::FlushVout();

    OSL_ENSURE( pOut == pVsh->GetOut(),
            "SwTxtSizeInfo::NoteAnimation() changed pOut" );
}

SwPosSize SwTxtSizeInfo::GetTxtSize( OutputDevice* pOutDev,
                                     const SwScriptInfo* pSI,
                                     const XubString& rTxt,
                                     const xub_StrLen nIndex,
                                     const xub_StrLen nLength,
                                     const sal_uInt16 nComp ) const
{
    SwDrawTextInfo aDrawInf( pVsh, *pOutDev, pSI, rTxt, nIndex, nLength );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
    return aSize;
}

SwPosSize SwTxtSizeInfo::GetTxtSize() const
{
    const SwScriptInfo& rSI =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    // in some cases, compression is not allowed or suppressed for
    // performance reasons
    sal_uInt16 nComp =( SW_CJK == GetFont()->GetActual() &&
                    rSI.CountCompChg() &&
                    ! IsMulti() ) ?
                    GetKanaComp() :
                                0 ;

    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rSI, *pTxt, nIdx, nLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    return pFnt->_GetTxtSize( aDrawInf );
}

void SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIndex,
                                const xub_StrLen nLength, const sal_uInt16 nComp,
                                sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff ) const
{
    SwDrawTextInfo aDrawInf( pVsh, *pOut, pSI, *pTxt, nIndex, nLength );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
    nMaxSizeDiff = (sal_uInt16)aDrawInf.GetKanaDiff();
    nMinSize = aSize.Width();
}

xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
                                       const xub_StrLen nMaxLen,
                                       const sal_uInt16 nComp ) const
{
    const SwScriptInfo& rScriptInfo =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    OSL_ENSURE( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" );
    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
                             *pTxt, GetIdx(), nMaxLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( 0 );

    return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
}

xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
                                       const xub_StrLen nMaxLen,
                                       const sal_uInt16 nComp,
                                       xub_StrLen& rExtraCharPos ) const
{
    const SwScriptInfo& rScriptInfo =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    OSL_ENSURE( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" );
    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
                             *pTxt, GetIdx(), nMaxLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( &rExtraCharPos );

    return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
}

void SwTxtPaintInfo::CtorInitTxtPaintInfo( SwTxtFrm *pFrame, const SwRect &rPaint )
{
    CtorInitTxtSizeInfo( pFrame );
    aTxtFly.CtorInitTxtFly( pFrame ),
    aPaintRect = rPaint;
    nSpaceIdx = 0;
    pSpaceAdd = NULL;
    pWrongList = NULL;
    pGrammarCheckList = NULL;
    pSmartTags = NULL;  // SMARTTAGS

#if OSL_DEBUG_LEVEL > 1
    pBrushItem = ((SvxBrushItem*)-1);
#else
    pBrushItem = 0;
#endif
}

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf, const XubString &rTxt )
    : SwTxtSizeInfo( rInf, rTxt ),
      pWrongList( rInf.GetpWrongList() ),
      pGrammarCheckList( rInf.GetGrammarCheckList() ),
      pSmartTags( rInf.GetSmartTags() ),    // SMARTTAGS
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pBrushItem( rInf.GetBrushItem() ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() )
{ }

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf )
    : SwTxtSizeInfo( rInf ),
      pWrongList( rInf.GetpWrongList() ),
      pGrammarCheckList( rInf.GetGrammarCheckList() ),
      pSmartTags( rInf.GetSmartTags() ),    // SMARTTAGS
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pBrushItem( rInf.GetBrushItem() ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() )
{ }

extern Color aGlobalRetoucheColor;

/*************************************************************************
 * lcl_IsDarkBackground
 *
 * Returns if the current background color is dark.
 *************************************************************************/

static sal_Bool lcl_IsDarkBackground( const SwTxtPaintInfo& rInf )
{
    const Color* pCol = rInf.GetFont()->GetBackColor();
    if( ! pCol || COL_TRANSPARENT == pCol->GetColor() )
    {
        const SvxBrushItem* pItem;
        SwRect aOrigBackRect;

        // Consider, that [GetBackgroundBrush(...)] can set <pCol>
        // See implementation in /core/layout/paintfrm.cxx
        // There is a background color, if there is a background brush and
        // its color is *not* "no fill"/"auto fill".
        if( rInf.GetTxtFrm()->GetBackgroundBrush( pItem, pCol, aOrigBackRect, sal_False ) )
        {
            if ( !pCol )
                pCol = &pItem->GetColor();

            // Determined color <pCol> can be <COL_TRANSPARENT>. Thus, check it.
            if ( pCol->GetColor() == COL_TRANSPARENT)
                pCol = NULL;
        }
        else
            pCol = NULL;
    }


    if( !pCol )
        pCol = &aGlobalRetoucheColor;

    return pCol->IsDark();
}

/*************************************************************************
 * SwTxtPaintInfo::_DrawText()
 *************************************************************************/

void SwTxtPaintInfo::_DrawText( const XubString &rText, const SwLinePortion &rPor,
                                const xub_StrLen nStart, const xub_StrLen nLength,
                                const sal_Bool bKern, const sal_Bool bWrong,
                                const sal_Bool bSmartTag,
                                const sal_Bool bGrammarCheck )  // SMARTTAGS
{
    if( !nLength )
        return;

    if( GetFont()->IsBlink() && OnWin() && rPor.Width() )
    {
        // check if accessibility options allow blinking portions:
        const ViewShell* pSh = GetTxtFrm()->getRootFrm()->GetCurrShell();
        if ( pSh && ! pSh->GetAccessibilityOptions()->IsStopAnimatedText() &&
             ! pSh->IsPreView() )
        {
            if( !pBlink )
                pBlink = new SwBlink();

            Point aPoint( aPos );

            if ( GetTxtFrm()->IsRightToLeft() )
                GetTxtFrm()->SwitchLTRtoRTL( aPoint );

            if ( TEXT_LAYOUT_BIDI_STRONG != GetOut()->GetLayoutMode() )
                aPoint.X() -= rPor.Width();

            if ( GetTxtFrm()->IsVertical() )
                GetTxtFrm()->SwitchHorizontalToVertical( aPoint );

            pBlink->Insert( aPoint, &rPor, GetTxtFrm(), pFnt->GetOrientation() );

            if( !pBlink->IsVisible() )
                return;
        }
        else
        {
            delete pBlink;
            pBlink = NULL;
        }
    }

    // The SwScriptInfo is useless if we are inside a field portion
    SwScriptInfo* pSI = 0;
    if ( ! rPor.InFldGrp() )
        pSI = &GetParaPortion()->GetScriptInfo();

    // in some cases, kana compression is not allowed or suppressed for
    // performance reasons
    sal_uInt16 nComp = 0;
    if ( ! IsMulti() )
        nComp = GetKanaComp();

    sal_Bool bCfgIsAutoGrammar = sal_False;
    SvtLinguConfig().GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bCfgIsAutoGrammar;
    const sal_Bool bBullet = OnWin() && GetOpt().IsBlank() && IsNoSymbol();
    const sal_Bool bTmpWrong = bWrong && OnWin() && GetOpt().IsOnlineSpell();
    const sal_Bool bTmpGrammarCheck = bGrammarCheck && OnWin() && bCfgIsAutoGrammar && GetOpt().IsOnlineSpell();
    const sal_Bool bTmpSmart = bSmartTag && OnWin() && !GetOpt().IsPagePreview() && SwSmartTagMgr::Get().IsSmartTagsEnabled(); // SMARTTAGS

    OSL_ENSURE( GetParaPortion(), "No paragraph!");
    SwDrawTextInfo aDrawInf( pFrm->getRootFrm()->GetCurrShell(), *pOut, pSI, rText, nStart, nLength,
                             rPor.Width(), bBullet );

    aDrawInf.SetLeft( GetPaintRect().Left() );
    aDrawInf.SetRight( GetPaintRect().Right() );
    aDrawInf.SetUnderFnt( pUnderFnt );

    const long nSpaceAdd = ( rPor.IsBlankPortion() || rPor.IsDropPortion() ||
                             rPor.InNumberGrp() ) ? 0 : GetSpaceAdd();
    if ( nSpaceAdd )
    {
        xub_StrLen nCharCnt;
        // #i41860# Thai justified alignment needs some
        // additional information:
        aDrawInf.SetNumberOfBlanks( rPor.InTxtGrp() ?
                                    static_cast<const SwTxtPortion&>(rPor).GetSpaceCnt( *this, nCharCnt ) :
                                    0 );
    }

    aDrawInf.SetSpace( nSpaceAdd );
    aDrawInf.SetKanaComp( nComp );

    // the font is used to identify the current script via nActual
    aDrawInf.SetFont( pFnt );
    // the frame is used to identify the orientation
    aDrawInf.SetFrm( GetTxtFrm() );
    // we have to know if the paragraph should snap to grid
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    // for underlining we must know when not to add extra space behind
    // a character in justified mode
    aDrawInf.SetSpaceStop( ! rPor.GetPortion() ||
                             rPor.GetPortion()->InFixMargGrp() ||
                             rPor.GetPortion()->IsHolePortion() );

    if( GetTxtFly()->IsOn() )
    {
        // aPos needs to be the TopLeft, because we cannot calculate the
        // ClipRects otherwise
        const Point aPoint( aPos.X(), aPos.Y() - rPor.GetAscent() );
        const Size aSize( rPor.Width(), rPor.Height() );
        aDrawInf.SetPos( aPoint );
        aDrawInf.SetSize( aSize );
        aDrawInf.SetAscent( rPor.GetAscent() );
        aDrawInf.SetKern( bKern ? rPor.Width() : 0 );
        aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
        aDrawInf.SetGrammarCheck( bTmpGrammarCheck ? pGrammarCheckList : NULL );
        aDrawInf.SetSmartTags( bTmpSmart ? pSmartTags : NULL );     // SMARTTAGS
        GetTxtFly()->DrawTextOpaque( aDrawInf );
    }
    else
    {
        aDrawInf.SetPos( aPos );
        if( bKern )
            pFnt->_DrawStretchText( aDrawInf );
        else
        {
            aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
            aDrawInf.SetGrammarCheck( bTmpGrammarCheck ? pGrammarCheckList : NULL );
            aDrawInf.SetSmartTags( bTmpSmart ? pSmartTags : NULL );  // SMARTTAGS
            pFnt->_DrawText( aDrawInf );
        }
    }
}

void SwTxtPaintInfo::CalcRect( const SwLinePortion& rPor,
                               SwRect* pRect, SwRect* pIntersect ) const
{
    Size aSize( rPor.Width(), rPor.Height() );
    if( rPor.IsHangingPortion() )
        aSize.Width() = ((SwHangingPortion&)rPor).GetInnerWidth();
    if( rPor.InSpaceGrp() && GetSpaceAdd() )
    {
        SwTwips nAdd = rPor.CalcSpacing( GetSpaceAdd(), *this );
        if( rPor.InFldGrp() && GetSpaceAdd() < 0 && nAdd )
            nAdd += GetSpaceAdd() / SPACING_PRECISION_FACTOR;
        aSize.Width() += nAdd;
    }

    Point aPoint;

    if( IsRotated() )
    {
        long nTmp = aSize.Width();
        aSize.Width() = aSize.Height();
        aSize.Height() = nTmp;
        if ( 1 == GetDirection() )
        {
            aPoint.A() = X() - rPor.GetAscent();
            aPoint.B() = Y() - aSize.Height();
        }
        else
        {
            aPoint.A() = X() - rPor.Height() + rPor.GetAscent();
            aPoint.B() = Y();
        }
    }
    else
    {
        aPoint.A() = X();
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if ( GetTxtFrm()->IsVertLR() )
            aPoint.B() = Y() - rPor.Height() + rPor.GetAscent();
        else
            aPoint.B() = Y() - rPor.GetAscent();
    }

    // Adjust x coordinate if we are inside a bidi portion
    const sal_Bool bFrmDir = GetTxtFrm()->IsRightToLeft();
    sal_Bool bCounterDir = ( ! bFrmDir && DIR_RIGHT2LEFT == GetDirection() ) ||
                       (   bFrmDir && DIR_LEFT2RIGHT == GetDirection() );

    if ( bCounterDir )
        aPoint.A() -= aSize.Width();

    SwRect aRect( aPoint, aSize );

    if ( GetTxtFrm()->IsRightToLeft() )
        GetTxtFrm()->SwitchLTRtoRTL( aRect );

    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchHorizontalToVertical( aRect );

    if ( pRect )
        *pRect = aRect;

    if( aRect.HasArea() && pIntersect )
    {
        ::SwAlignRect( aRect, (ViewShell*)GetVsh() );

        if ( GetOut()->IsClipRegion() )
        {
            SwRect aClip( GetOut()->GetClipRegion().GetBoundRect() );
            aRect.Intersection( aClip );
        }

        *pIntersect = aRect;
    }
}

/*************************************************************************
 * lcl_DrawSpecial
 *
 * Draws a special portion, e.g., line break portion, tab portion.
 * rPor     - The portion
 * rRect    - The rectangle surrounding the character
 * pCol     - Specify a color for the character
 * bCenter  - Draw the character centered, otherwise left aligned
 * bRotate  - Rotate the character if character rotation is set
 *************************************************************************/

static void lcl_DrawSpecial( const SwTxtPaintInfo& rInf, const SwLinePortion& rPor,
                      SwRect& rRect, const Color* pCol, sal_Unicode cChar,
                      sal_uInt8 nOptions )
{
    sal_Bool bCenter = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_CENTER );
    sal_Bool bRotate = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_ROTATE );

    // rRect is given in absolute coordinates
    if ( rInf.GetTxtFrm()->IsRightToLeft() )
        rInf.GetTxtFrm()->SwitchRTLtoLTR( rRect );
    if ( rInf.GetTxtFrm()->IsVertical() )
        rInf.GetTxtFrm()->SwitchVerticalToHorizontal( rRect );

    const SwFont* pOldFnt = rInf.GetFont();

    // Font is generated only once:
    static SwFont* pFnt = 0;
    if ( ! pFnt )
    {
        pFnt = new SwFont( *pOldFnt );
        pFnt->SetFamily( FAMILY_DONTKNOW, pFnt->GetActual() );
        pFnt->SetName( numfunc::GetDefBulletFontname(), pFnt->GetActual() );
        pFnt->SetStyleName( aEmptyStr, pFnt->GetActual() );
        pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, pFnt->GetActual() );
    }

    // Some of the current values are set at the font:
    if ( ! bRotate )
        pFnt->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );
    else
        pFnt->SetVertical( pOldFnt->GetOrientation() );

    if ( pCol )
        pFnt->SetColor( *pCol );
    else
        pFnt->SetColor( pOldFnt->GetColor() );

    Size aFontSize( 0, SPECIAL_FONT_HEIGHT );
    pFnt->SetSize( aFontSize, pFnt->GetActual() );

    ((SwTxtPaintInfo&)rInf).SetFont( pFnt );

    // The maximum width depends on the current orientation
    const sal_uInt16 nDir = pFnt->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
    SwTwips nMaxWidth = 0;
    switch ( nDir )
    {
    case 0 :
        nMaxWidth = rRect.Width();
        break;
    case 900 :
    case 2700 :
        nMaxWidth = rRect.Height();
        break;
    default:
        OSL_FAIL( "Unknown direction set at font" );
        break;
    }

    // check if char fits into rectangle
    const OUString aTmp( cChar );
    aFontSize = rInf.GetTxtSize( aTmp ).SvLSize();
    while ( aFontSize.Width() > nMaxWidth )
    {
        SwTwips nFactor = ( 100 * aFontSize.Width() ) / nMaxWidth;
        const SwTwips nOldWidth = aFontSize.Width();

        // new height for font
        const sal_uInt8 nAct = pFnt->GetActual();
        aFontSize.Height() = ( 100 * pFnt->GetSize( nAct ).Height() ) / nFactor;
        aFontSize.Width() = ( 100 * pFnt->GetSize( nAct).Width() ) / nFactor;

        if ( !aFontSize.Width() && !aFontSize.Height() )
            break;

        pFnt->SetSize( aFontSize, nAct );

        aFontSize = rInf.GetTxtSize( aTmp ).SvLSize();

        if ( aFontSize.Width() >= nOldWidth )
            break;
    }

    const Point aOldPos( rInf.GetPos() );

    // adjust values so that tab is vertically and horizontally centered
    SwTwips nX = rRect.Left();
    SwTwips nY = rRect.Top();
    switch ( nDir )
    {
    case 0 :
        if ( bCenter )
            nX += ( rRect.Width() - aFontSize.Width() ) / 2;
        nY += ( rRect.Height() - aFontSize.Height() ) / 2 + rInf.GetAscent();
        break;
    case 900 :
        if ( bCenter )
            nX += ( rRect.Width() - aFontSize.Height() ) / 2 + rInf.GetAscent();
        nY += ( rRect.Height() + aFontSize.Width() ) / 2;
        break;
    case 2700 :
        if ( bCenter )
            nX += ( rRect.Width() + aFontSize.Height() ) / 2 - rInf.GetAscent();
        nY += ( rRect.Height() - aFontSize.Width() ) / 2;
        break;
    }

    Point aTmpPos( nX, nY );
    ((SwTxtPaintInfo&)rInf).SetPos( aTmpPos );
    sal_uInt16 nOldWidth = rPor.Width();
    ((SwLinePortion&)rPor).Width( (sal_uInt16)aFontSize.Width() );
    rInf.DrawText( aTmp, rPor );
    ((SwLinePortion&)rPor).Width( nOldWidth );
    ((SwTxtPaintInfo&)rInf).SetFont( (SwFont*)pOldFnt );
    ((SwTxtPaintInfo&)rInf).SetPos( aOldPos );
}

void SwTxtPaintInfo::DrawRect( const SwRect &rRect, sal_Bool bNoGraphic,
                               sal_Bool bRetouche ) const
{
    if ( OnWin() || !bRetouche )
    {
        if( aTxtFly.IsOn() )
            ((SwTxtPaintInfo*)this)->GetTxtFly()->
                DrawFlyRect( pOut, rRect, *this, bNoGraphic );
        else if ( bNoGraphic )
            pOut->DrawRect( rRect.SVRect() );
        else
        {
            OSL_ENSURE( ((SvxBrushItem*)-1) != pBrushItem, "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( pBrushItem, pOut, aItemRect, rRect );
        }
    }
}

void SwTxtPaintInfo::DrawTab( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        SwRect aRect;
        CalcRect( rPor, &aRect );

        if ( ! aRect.HasArea() )
            return;

        const sal_Unicode cChar = GetTxtFrm()->IsRightToLeft() ?
                                  CHAR_TAB_RTL : CHAR_TAB;
        const sal_uInt8 nOptions = DRAW_SPECIAL_OPTIONS_CENTER |
                              DRAW_SPECIAL_OPTIONS_ROTATE;
        lcl_DrawSpecial( *this, rPor, aRect, 0, cChar, nOptions );
    }
}

void SwTxtPaintInfo::DrawLineBreak( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        KSHORT nOldWidth = rPor.Width();
        ((SwLinePortion&)rPor).Width( LINE_BREAK_WIDTH );

        SwRect aRect;
        CalcRect( rPor, &aRect );

        if( aRect.HasArea() )
        {
            const sal_Unicode cChar = GetTxtFrm()->IsRightToLeft() ?
                                      CHAR_LINEBREAK_RTL : CHAR_LINEBREAK;
            const sal_uInt8 nOptions = 0;
            lcl_DrawSpecial( *this, rPor, aRect, 0, cChar, nOptions );
        }

        ((SwLinePortion&)rPor).Width( nOldWidth );
    }
}

void SwTxtPaintInfo::DrawRedArrow( const SwLinePortion &rPor ) const
{
    Size aSize( SPECIAL_FONT_HEIGHT, SPECIAL_FONT_HEIGHT );
    SwRect aRect( ((SwArrowPortion&)rPor).GetPos(), aSize );
    sal_Unicode cChar;
    if( ((SwArrowPortion&)rPor).IsLeft() )
    {
        aRect.Pos().Y() += 20 - GetAscent();
        aRect.Pos().X() += 20;
        if( aSize.Height() > rPor.Height() )
            aRect.Height( rPor.Height() );
        cChar = CHAR_LEFT_ARROW;
    }
    else
    {
        if( aSize.Height() > rPor.Height() )
            aRect.Height( rPor.Height() );
        aRect.Pos().Y() -= aRect.Height() + 20;
        aRect.Pos().X() -= aRect.Width() + 20;
        cChar = CHAR_RIGHT_ARROW;
    }

    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchHorizontalToVertical( aRect );

    Color aCol( COL_LIGHTRED );

    if( aRect.HasArea() )
    {
        const sal_uInt8 nOptions = 0;
        lcl_DrawSpecial( *this, rPor, aRect, &aCol, cChar, nOptions );
    }
}

void SwTxtPaintInfo::DrawPostIts( const SwLinePortion&, sal_Bool bScript ) const
{
    if( OnWin() && pOpt->IsPostIts() )
    {
        Size aSize;
        Point aTmp;

        const sal_uInt16 nPostItsWidth = pOpt->GetPostItsWidth( GetOut() );
        const sal_uInt16 nFontHeight = pFnt->GetHeight( pVsh, *GetOut() );
        const sal_uInt16 nFontAscent = pFnt->GetAscent( pVsh, *GetOut() );

        switch ( pFnt->GetOrientation( GetTxtFrm()->IsVertical() ) )
        {
        case 0 :
            aSize.Width() = nPostItsWidth;
            aSize.Height() = nFontHeight;
            aTmp.X() = aPos.X();
            aTmp.Y() = aPos.Y() - nFontAscent;
            break;
        case 900 :
            aSize.Height() = nPostItsWidth;
            aSize.Width() = nFontHeight;
            aTmp.X() = aPos.X() - nFontAscent;
            aTmp.Y() = aPos.Y();
            break;
        case 2700 :
            aSize.Height() = nPostItsWidth;
            aSize.Width() = nFontHeight;
            aTmp.X() = aPos.X() - nFontHeight +
                                  nFontAscent;
            aTmp.Y() = aPos.Y();
            break;
        }

        SwRect aTmpRect( aTmp, aSize );

        if ( GetTxtFrm()->IsRightToLeft() )
            GetTxtFrm()->SwitchLTRtoRTL( aTmpRect );

        if ( GetTxtFrm()->IsVertical() )
            GetTxtFrm()->SwitchHorizontalToVertical( aTmpRect );

        const Rectangle aRect( aTmpRect.SVRect() );
        pOpt->PaintPostIts( (OutputDevice*)GetOut(), aRect, bScript );
    }
}

void SwTxtPaintInfo::DrawCheckBox( const SwFieldFormPortion &rPor, bool checked) const
{
    SwRect aIntersect;
    CalcRect( rPor, &aIntersect, 0 );
    if ( aIntersect.HasArea() )
    {
        if (OnWin() && SwViewOption::IsFieldShadings() &&
                !GetOpt().IsPagePreview())
        {
            OutputDevice* pOut_ = (OutputDevice*)GetOut();
            pOut_->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut_->SetFillColor( SwViewOption::GetFieldShadingsColor() );
            pOut_->SetLineColor();
            pOut_->DrawRect( aIntersect.SVRect() );
            pOut_->Pop();
        }
        const int delta=10;
        Rectangle r(aIntersect.Left()+delta, aIntersect.Top()+delta, aIntersect.Right()-delta, aIntersect.Bottom()-delta);
        pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        pOut->SetLineColor( Color(0, 0, 0));
        pOut->SetFillColor();
        pOut->DrawRect( r );
        if (checked) {
            pOut->DrawLine(r.TopLeft(), r.BottomRight());
            pOut->DrawLine(r.TopRight(), r.BottomLeft());
        }
        pOut->Pop();
    }
}

void SwTxtPaintInfo::DrawBackground( const SwLinePortion &rPor ) const
{
    OSL_ENSURE( OnWin(), "SwTxtPaintInfo::DrawBackground: printer pollution ?" );

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pTmpOut = (OutputDevice*)GetOut();
        pTmpOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );

        // For dark background we do not want to have a filled rectangle
        if ( GetVsh() && GetVsh()->GetWin() && lcl_IsDarkBackground( *this ) )
        {
            pTmpOut->SetLineColor( SwViewOption::GetFontColor().GetColor() );
        }
        else
        {
            pTmpOut->SetFillColor( SwViewOption::GetFieldShadingsColor() );
            pTmpOut->SetLineColor();
        }

        DrawRect( aIntersect, sal_True );
        pTmpOut->Pop();
    }
}

void SwTxtPaintInfo::_DrawBackBrush( const SwLinePortion &rPor ) const
{
    {
        SwRect aIntersect;
        CalcRect( rPor, &aIntersect, 0 );
        if(aIntersect.HasArea())
        {
            SwTxtNode *pNd = pFrm->GetTxtNode();
            const ::sw::mark::IFieldmark* pFieldmark = NULL;
            if(pNd)
            {
                const SwDoc *doc=pNd->GetDoc();
                if(doc)
                {
                    SwIndex aIndex(pNd, GetIdx());
                    SwPosition aPosition(*pNd, aIndex);
                    pFieldmark=doc->getIDocumentMarkAccess()->getFieldmarkFor(aPosition);
                }
            }
            bool bIsStartMark=(1==GetLen() && CH_TXT_ATR_FIELDSTART==GetTxt().GetChar(GetIdx()));
            if(pFieldmark) {
                OSL_TRACE("Found Fieldmark");
#if OSL_DEBUG_LEVEL > 1
                OUString str = pFieldmark->ToString( );
                fprintf( stderr, "%s\n", rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr( ) );
#endif
            }
            if(bIsStartMark) OSL_TRACE("Found StartMark");
            if (OnWin() && (pFieldmark!=NULL || bIsStartMark) &&
                    SwViewOption::IsFieldShadings() &&
                    !GetOpt().IsPagePreview())
            {
                OutputDevice* pOutDev = (OutputDevice*)GetOut();
                pOutDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
                bool bFilled = false;
                // If this is a comment range, need to look up the color of the comment author.
                if (pFieldmark->GetFieldname() == ODF_COMMENTRANGE)
                {
                    // Search for the position of the postit field
                    const sal_Unicode fld[] = { CH_TXTATR_INWORD, 0 };
                    xub_StrLen nEndIdx = GetTxt().SearchChar(fld, GetIdx());
                    if (nEndIdx != STRING_NOTFOUND)
                    {
                        SwTxtAttr* pTxtAttr = pNd->GetTxtAttrForCharAt(nEndIdx, RES_TXTATR_FIELD);
                        const SwFmtFld& rPostItField = pTxtAttr->GetFld();
                        // Look up the author name
                        const OUString& rAuthor = rPostItField.GetFld()->GetPar1();
                        sal_uInt16 nIndex = pNd->GetDoc()->InsertRedlineAuthor(rAuthor);
                        pOutDev->SetFillColor( SwPostItMgr::GetColorLight(nIndex) );
                        pOutDev->SetLineColor( SwPostItMgr::GetColorAnchor(nIndex) );
                        bFilled = true;
                    }
                }
                if (!bFilled)
                {
                    pOutDev->SetFillColor( SwViewOption::GetFieldShadingsColor() );
                    pOutDev->SetLineColor( );
                }
                pOutDev->DrawRect( aIntersect.SVRect() );
                pOutDev->Pop();
            }
        }
    }
    if( !pFnt->GetBackColor() ) return;

    OSL_ENSURE( pFnt->GetBackColor(), "DrawBackBrush: Lost Color" );

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pTmpOut = (OutputDevice*)GetOut();

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pTmpOut );

        pTmpOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );

        pTmpOut->SetFillColor( *pFnt->GetBackColor() );
        pTmpOut->SetLineColor();

        DrawRect( aIntersect, sal_True, sal_False );

        pTmpOut->Pop();
    }
}

void SwTxtPaintInfo::DrawViewOpt( const SwLinePortion &rPor,
                                  const MSHORT nWhich ) const
{
    if( OnWin() && !IsMulti() )
    {
        sal_Bool bDraw = sal_False;
        switch( nWhich )
        {
            case POR_FTN:
            case POR_QUOVADIS:
            case POR_NUMBER:
            case POR_FLD:
            case POR_URL:
            case POR_HIDDEN:
            case POR_TOX:
            case POR_REF:
            case POR_META:
            case POR_CONTROLCHAR:
                if ( !GetOpt().IsPagePreview() &&
                     !GetOpt().IsReadonly() &&
                     SwViewOption::IsFieldShadings() &&
                     (POR_NUMBER != nWhich ||
                      pFrm->GetTxtNode()->HasMarkedLabel())) // #i27615#
                    bDraw = sal_True;
            break;
            case POR_TAB:       if ( GetOpt().IsTab() )     bDraw = sal_True; break;
            case POR_SOFTHYPH:  if ( GetOpt().IsSoftHyph() )bDraw = sal_True; break;
            case POR_BLANK:     if ( GetOpt().IsHardBlank())bDraw = sal_True; break;
            default:
            {
                OSL_ENSURE( !this, "SwTxtPaintInfo::DrawViewOpt: don't know how to draw this" );
                break;
            }
        }
        if ( bDraw )
            DrawBackground( rPor );
    }
}

void SwTxtPaintInfo::_NotifyURL( const SwLinePortion &rPor ) const
{
    OSL_ENSURE( pNoteURL, "NotifyURL: pNoteURL gone with the wind!" );

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect );

    if( aIntersect.HasArea() )
    {
        SwTxtNode *pNd = (SwTxtNode*)GetTxtFrm()->GetTxtNode();
        SwTxtAttr *const pAttr =
            pNd->GetTxtAttrAt(GetIdx(), RES_TXTATR_INETFMT);
        if( pAttr )
        {
            const SwFmtINetFmt& rFmt = pAttr->GetINetFmt();
            pNoteURL->InsertURLNote( rFmt.GetValue(), rFmt.GetTargetFrame(),
                aIntersect );
        }
    }
}

static void lcl_InitHyphValues( PropertyValues &rVals,
            sal_Int16 nMinLeading, sal_Int16 nMinTrailing )
{
    sal_Int32 nLen = rVals.getLength();

    if (0 == nLen)  // yet to be initialized?
    {
        rVals.realloc( 2 );
        PropertyValue *pVal = rVals.getArray();

        pVal[0].Name    = UPN_HYPH_MIN_LEADING;
        pVal[0].Handle  = UPH_HYPH_MIN_LEADING;
        pVal[0].Value   <<= nMinLeading;

        pVal[1].Name    = UPN_HYPH_MIN_TRAILING;
        pVal[1].Handle  = UPH_HYPH_MIN_TRAILING;
        pVal[1].Value   <<= nMinTrailing;
    }
    else if (2 == nLen) // already initialized once?
    {
        PropertyValue *pVal = rVals.getArray();
        pVal[0].Value <<= nMinLeading;
        pVal[1].Value <<= nMinTrailing;
    }
    else {
        OSL_FAIL( "unexpected size of sequence" );
    }
}

const PropertyValues & SwTxtFormatInfo::GetHyphValues() const
{
    OSL_ENSURE( 2 == aHyphVals.getLength(),
            "hyphenation values not yet initialized" );
    return aHyphVals;
}

sal_Bool SwTxtFormatInfo::InitHyph( const sal_Bool bAutoHyphen )
{
    const SwAttrSet& rAttrSet = GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
    SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
    SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
    SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
    const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
    MaxHyph() = rAttr.GetMaxHyphens();
    sal_Bool bAuto = bAutoHyphen || rAttr.IsHyphen();
    if( bAuto || bInterHyph )
    {
        nHyphStart = nHyphWrdStart = STRING_LEN;
        nHyphWrdLen = 0;

        const sal_Int16 nMinimalLeading  = Max(rAttr.GetMinLead(), sal_uInt8(2));
        const sal_Int16 nMinimalTrailing = rAttr.GetMinTrail();
        lcl_InitHyphValues( aHyphVals, nMinimalLeading, nMinimalTrailing);
    }
    return bAuto;
}

void SwTxtFormatInfo::CtorInitTxtFormatInfo( SwTxtFrm *pNewFrm, const sal_Bool bNewInterHyph,
                                const sal_Bool bNewQuick, const sal_Bool bTst )
{
    CtorInitTxtPaintInfo( pNewFrm, SwRect() );

    bQuick = bNewQuick;
    bInterHyph = bNewInterHyph;

    //! needs to be done in this order
    nMinLeading     = 2;
    nMinTrailing    = 2;
    nMinWordLength  = 0;
    bAutoHyph = InitHyph();

    bIgnoreFly = sal_False;
    bFakeLineStart = sal_False;
    bShift = sal_False;
    bDropInit = sal_False;
    bTestFormat = bTst;
    nLeft = 0;
    nRight = 0;
    nFirst = 0;
    nRealWidth = 0;
    nForcedLeftMargin = 0;
    pRest = 0;
    nLineHeight = 0;
    nLineNettoHeight = 0;
    SetLineStart(0);
    Init();
}

/*************************************************************************
 * SwTxtFormatInfo::IsHyphenate()
 * If the Hyphenator returns ERROR or the language is set to NOLANGUAGE
 * we do not hyphenate.
 * Else, we always hyphenate if we do interactive hyphenation.
 * If we do not do interactive hyphenation, we only hyphenate if ParaFmt is
 * set to automatic hyphenation.
 *************************************************************************/
sal_Bool SwTxtFormatInfo::IsHyphenate() const
{
    if( !bInterHyph && !bAutoHyph )
        return sal_False;

    LanguageType eTmp = GetFont()->GetLanguage();
    if( LANGUAGE_DONTKNOW == eTmp || LANGUAGE_NONE == eTmp )
        return sal_False;

    uno::Reference< XHyphenator > xHyph = ::GetHyphenator();
    if (bInterHyph && xHyph.is())
        SvxSpellWrapper::CheckHyphLang( xHyph, eTmp );

    if( !xHyph.is() || !xHyph->hasLocale( pBreakIt->GetLocale(eTmp) ) )
        return sal_False;
    return sal_True;
}

/*************************************************************************
 * SwTxtFormatInfo::GetDropFmt()
 * Dropcaps called by the SwTxtFormatter::CTOR
*************************************************************************/

const SwFmtDrop *SwTxtFormatInfo::GetDropFmt() const
{
    const SwFmtDrop *pDrop = &GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetDrop();
    if( 1 >= pDrop->GetLines() ||
        ( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
        pDrop = 0;
    return pDrop;
}

void SwTxtFormatInfo::Init()
{
    // Not initialized: pRest, nLeft, nRight, nFirst, nRealWidth
    X(0);
    bArrowDone = bFull = bFtnDone = bErgoDone = bNumDone = bNoEndHyph =
        bNoMidHyph = bStop = bNewLine = bUnderFlow = bTabOverflow = sal_False;

    // generally we do not allow number portions in follows, except...
    if ( GetTxtFrm()->IsFollow() )
    {
        const SwTxtFrm* pMaster = GetTxtFrm()->FindMaster();
        OSL_ENSURE(pMaster, "pTxtFrm without Master");
        const SwLinePortion* pTmpPara = pMaster ? pMaster->GetPara() : NULL;

        // there is a master for this follow and the master does not have
        // any contents (especially it does not have a number portion)
        bNumDone = ! pTmpPara ||
                   ! ((SwParaPortion*)pTmpPara)->GetFirstPortion()->IsFlyPortion();
    }

    pRoot = 0;
    pLast = 0;
    pFly = 0;
    pLastFld = 0;
    pLastTab = 0;
    pUnderFlow = 0;
    cTabDecimal = 0;
    nWidth = nRealWidth;
    nForcedLeftMargin = 0;
    nSoftHyphPos = 0;
    nUnderScorePos = STRING_LEN;
    cHookChar = 0;
    SetIdx(0);
    SetLen( GetTxt().Len() );
    SetPaintOfst(0);
}

/*--------------------------------------------------
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 * --------------------------------------------------*/

SwTxtFormatInfo::SwTxtFormatInfo( const SwTxtFormatInfo& rInf,
    SwLineLayout& rLay, SwTwips nActWidth ) : SwTxtPaintInfo( rInf ),
    bTabOverflow( sal_False )
{
    pRoot = &rLay;
    pLast = &rLay;
    pFly = NULL;
    pLastFld = NULL;
    pUnderFlow = NULL;
    pRest = NULL;
    pLastTab = NULL;

    nSoftHyphPos = 0;
    nUnderScorePos = STRING_LEN;
    nHyphStart = 0;
    nHyphWrdStart = 0;
    nHyphWrdLen = 0;
    nLineStart = rInf.GetIdx();
    nLeft = rInf.nLeft;
    nRight = rInf.nRight;
    nFirst = rInf.nLeft;
    nRealWidth = KSHORT(nActWidth);
    nWidth = nRealWidth;
    nLineHeight = 0;
    nLineNettoHeight = 0;
    nForcedLeftMargin = 0;

    nMinLeading = 0;
    nMinTrailing = 0;
    nMinWordLength = 0;
    bFull = sal_False;
    bFtnDone = sal_True;
    bErgoDone = sal_True;
    bNumDone = sal_True;
    bArrowDone = sal_True;
    bStop = sal_False;
    bNewLine = sal_True;
    bShift  = sal_False;
    bUnderFlow = sal_False;
    bInterHyph = sal_False;
    bAutoHyph = sal_False;
    bDropInit = sal_False;
    bQuick  = rInf.bQuick;
    bNoEndHyph  = sal_False;
    bNoMidHyph  = sal_False;
    bIgnoreFly = sal_False;
    bFakeLineStart = sal_False;

    cTabDecimal = 0;
    cHookChar = 0;
    nMaxHyph = 0;
    bTestFormat = rInf.bTestFormat;
    SetMulti( sal_True );
    SetFirstMulti( rInf.IsFirstMulti() );
}
sal_Bool SwTxtFormatInfo::_CheckFtnPortion( SwLineLayout* pCurr )
{
    KSHORT nHeight = pCurr->GetRealHeight();
    SwLinePortion *pPor = pCurr->GetPortion();
    sal_Bool bRet = sal_False;
    while( pPor )
    {
        if( pPor->IsFtnPortion() && nHeight > ((SwFtnPortion*)pPor)->Orig() )
        {
            bRet = sal_True;
            SetLineHeight( nHeight );
            SetLineNettoHeight( pCurr->Height() );
            break;
        }
        pPor = pPor->GetPortion();
    }
    return bRet;
}

xub_StrLen SwTxtFormatInfo::ScanPortionEnd( const xub_StrLen nStart,
                                            const xub_StrLen nEnd )
{
    cHookChar = 0;
    xub_StrLen i = nStart;

    //
    // Used for decimal tab handling:
    //
    const sal_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
    const sal_Unicode cThousandSep  = ',' == cTabDec ? '.' : ',';
    // #i45951# German (Switzerland) uses ' as thousand separator
    const sal_Unicode cThousandSep2 = ',' == cTabDec ? '.' : '\'';

    bool bNumFound = false;
    const bool bTabCompat = GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);

    for( ; i < nEnd; ++i )
    {
        const sal_Unicode cPos = GetChar( i );
        switch( cPos )
        {
        case CH_TXTATR_BREAKWORD:
        case CH_TXTATR_INWORD:
            if( !HasHint( i ))
                break;
            // no break;

        case CHAR_SOFTHYPHEN:
        case CHAR_HARDHYPHEN:
        case CHAR_HARDBLANK:
        case CH_TAB:
        case CH_BREAK:
        case CHAR_ZWSP :
        case CHAR_ZWNBSP :
            cHookChar = cPos;
            return i;

        case CHAR_UNDERSCORE:
            if ( STRING_LEN == nUnderScorePos )
                nUnderScorePos = i;
            break;

        default:
            if ( cTabDec )
            {
                if( cTabDec == cPos )
                {
                    OSL_ENSURE( cPos, "Unexpected end of string" );
                    if( cPos ) // robust
                    {
                        cHookChar = cPos;
                        return i;
                    }
                }

                //
                // Compatibility: First non-digit character behind a
                // a digit character becomes the hook character
                //
                if ( bTabCompat )
                {
                    if ( ( 0x2F < cPos && cPos < 0x3A ) ||
                         ( bNumFound && ( cPos == cThousandSep || cPos == cThousandSep2 ) ) )
                    {
                        bNumFound = true;
                    }
                    else
                    {
                        if ( bNumFound )
                        {
                            cHookChar = cPos;
                            SetTabDecimal( cPos );
                            return i;
                        }
                    }
                }
            }
        }
    }

    // Check if character *behind* the portion has
    // to become the hook:
    if ( i == nEnd && i < GetTxt().Len() && bNumFound )
    {
        const sal_Unicode cPos = GetChar( i );
        if ( cPos != cTabDec && cPos != cThousandSep && cPos !=cThousandSep2 && ( 0x2F >= cPos || cPos >= 0x3A ) )
        {
            cHookChar = GetChar( i );
            SetTabDecimal( cHookChar );
        }
    }

    return i;
}

sal_Bool SwTxtFormatInfo::LastKernPortion()
{
    if( GetLast() )
    {
         if( GetLast()->IsKernPortion() )
            return sal_True;
        if( GetLast()->Width() || ( GetLast()->GetLen() &&
            !GetLast()->IsHolePortion() ) )
            return sal_False;
    }
    SwLinePortion* pPor = GetRoot();
    SwLinePortion *pKern = NULL;
    while( pPor )
    {
        if( pPor->IsKernPortion() )
            pKern = pPor;
        else if( pPor->Width() || ( pPor->GetLen() && !pPor->IsHolePortion() ) )
            pKern = NULL;
        pPor = pPor->GetPortion();
    }
    if( pKern )
    {
        SetLast( pKern );
        return sal_True;
    }
    return sal_False;
}

SwTxtSlot::SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
                      bool bTxtLen, bool bExgLists, const sal_Char *pCh )
    : pOldTxt( 0 ),
      pOldSmartTagList( 0 ),
      pOldGrammarCheckList( 0 ),
      pTempList( 0 )
{
    if( pCh )
    {
        aTxt = XubString( pCh, RTL_TEXTENCODING_MS_1252 );
        bOn = sal_True;
    }
    else
        bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Teh text is replaced ...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetTxt( aTxt );
        pInf->SetIdx( 0 );
        pInf->SetLen( bTxtLen ? pInf->GetTxt().Len() : pPor->GetLen() );

        // ST2
        if ( bExgLists )
        {
            pOldSmartTagList = static_cast<SwTxtPaintInfo*>(pInf)->GetSmartTags();
            if ( pOldSmartTagList )
            {
                const sal_uInt16 nPos = pOldSmartTagList->GetWrongPos(nIdx);
                const xub_StrLen nListPos = pOldSmartTagList->Pos(nPos);
                if( nListPos == nIdx )
                    ((SwTxtPaintInfo*)pInf)->SetSmartTags( pOldSmartTagList->SubList( nPos ) );
                else if( !pTempList && nPos < pOldSmartTagList->Count() && nListPos < nIdx && aTxt.Len() )
                {
                    pTempList = new SwWrongList( WRONGLIST_SMARTTAG );
                    pTempList->Insert( OUString(), 0, 0, aTxt.Len(), 0 );
                    ((SwTxtPaintInfo*)pInf)->SetSmartTags( pTempList );
                }
                else
                    ((SwTxtPaintInfo*)pInf)->SetSmartTags( 0);
            }
            pOldGrammarCheckList = static_cast<SwTxtPaintInfo*>(pInf)->GetGrammarCheckList();
            if ( pOldGrammarCheckList )
            {
                const sal_uInt16 nPos = pOldGrammarCheckList->GetWrongPos(nIdx);
                const xub_StrLen nListPos = pOldGrammarCheckList->Pos(nPos);
                if( nListPos == nIdx )
                    ((SwTxtPaintInfo*)pInf)->SetGrammarCheckList( pOldGrammarCheckList->SubList( nPos ) );
                else if( !pTempList && nPos < pOldGrammarCheckList->Count() && nListPos < nIdx && aTxt.Len() )
                {
                    pTempList = new SwWrongList( WRONGLIST_GRAMMAR );
                    pTempList->Insert( OUString(), 0, 0, aTxt.Len(), 0 );
                    ((SwTxtPaintInfo*)pInf)->SetGrammarCheckList( pTempList );
                }
                else
                    ((SwTxtPaintInfo*)pInf)->SetGrammarCheckList( 0);
            }
        }
    }
}

SwTxtSlot::~SwTxtSlot()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );

        // ST2
        // Restore old smart tag list
        if ( pOldSmartTagList )
            ((SwTxtPaintInfo*)pInf)->SetSmartTags( pOldSmartTagList );
        if ( pOldGrammarCheckList )
            ((SwTxtPaintInfo*)pInf)->SetGrammarCheckList( pOldGrammarCheckList );
        delete pTempList;
    }
}

SwFontSave::SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pNew,
        SwAttrIter* pItr )
        : pFnt( pNew ? ((SwTxtSizeInfo&)rInf).GetFont() : 0 )
{
    if( pFnt )
    {
        pInf = &((SwTxtSizeInfo&)rInf);
        // In these cases we temporarily switch to the new font:
        // 1. the fonts have a different magic number
        // 2. they have different script types
        // 3. their background colors differ (this is not covered by 1.)
        if( pFnt->DifferentMagic( pNew, pFnt->GetActual() ) ||
            pNew->GetActual() != pFnt->GetActual() ||
            ( ! pNew->GetBackColor() && pFnt->GetBackColor() ) ||
            ( pNew->GetBackColor() && ! pFnt->GetBackColor() ) ||
            ( pNew->GetBackColor() && pFnt->GetBackColor() &&
              ( *pNew->GetBackColor() != *pFnt->GetBackColor() ) ) )
        {
            pNew->SetTransparent( sal_True );
            pNew->SetAlign( ALIGN_BASELINE );
            pInf->SetFont( pNew );
        }
        else
            pFnt = 0;
        pNew->Invalidate();
        pNew->ChgPhysFnt( pInf->GetVsh(), *pInf->GetOut() );
        if( pItr && pItr->GetFnt() == pFnt )
        {
            pIter = pItr;
            pIter->SetFnt( pNew );
        }
        else
            pIter = NULL;
    }
}

SwFontSave::~SwFontSave()
{
    if( pFnt )
    {
        // Reset SwFont
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
        if( pIter )
        {
            pIter->SetFnt( pFnt );
            pIter->nPos = STRING_LEN;
        }
    }
}

SwDefFontSave::SwDefFontSave( const SwTxtSizeInfo &rInf )
        : pFnt( ((SwTxtSizeInfo&)rInf).GetFont()  )
{
    const sal_Bool bTmpAlter = pFnt->GetFixKerning() ||
         ( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet(pFnt->GetActual()) )
        ;

    const sal_Bool bFamily = bTmpAlter &&
          pFnt->GetName( pFnt->GetActual() ) != numfunc::GetDefBulletFontname();
    const sal_Bool bRotation = (sal_Bool)pFnt->GetOrientation() &&
                                ! rInf.GetTxtFrm()->IsVertical();

    if( bFamily || bRotation )
    {
        pNewFnt = new SwFont( *pFnt );

        if ( bFamily )
        {
            pNewFnt->SetFamily( FAMILY_DONTKNOW, pFnt->GetActual() );
            pNewFnt->SetName( numfunc::GetDefBulletFontname(), pFnt->GetActual() );
            pNewFnt->SetStyleName( aEmptyStr, pFnt->GetActual() );
            pNewFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, pFnt->GetActual() );
            pNewFnt->SetFixKerning( 0 );
        }

        if ( bRotation )
            pNewFnt->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );

        pInf = &((SwTxtSizeInfo&)rInf);
        pNewFnt->Invalidate();
        pInf->SetFont( pNewFnt );
    }
    else
    {
        pFnt = 0;
        pNewFnt = 0;
    }
}

SwDefFontSave::~SwDefFontSave()
{
    if( pFnt )
    {
        delete pNewFnt;
        // Reset SwFont
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
    }
}

sal_Bool SwTxtFormatInfo::ChgHyph( const sal_Bool bNew )
{
    const sal_Bool bOld = bAutoHyph;
    if( bAutoHyph != bNew )
    {
        bAutoHyph = bNew;
        InitHyph( bNew );
        // Set language in the Hyphenator
        if( pFnt )
            pFnt->ChgPhysFnt( pVsh, *pOut );
    }
    return bOld;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
