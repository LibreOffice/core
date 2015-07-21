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

#include <com/sun/star/uno/Sequence.h>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <hintids.hxx>
#include <svl/ctloptions.hxx>
#include <sfx2/printer.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/shaditem.hxx>

#include <SwSmartTagMgr.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/unolingu.hxx>
#include <breakit.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <txatbase.hxx>
#include <fmtinfmt.hxx>
#include <swmodule.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <paratr.hxx>
#include <rootfrm.hxx>
#include <inftxt.hxx>
#include <blink.hxx>
#include <noteurl.hxx>
#include <porftn.hxx>
#include <porrst.hxx>
#include <itratr.hxx>
#include <accessibilityoptions.hxx>
#include <wrong.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <SwGrammarMarkUp.hxx>
#include "numrule.hxx"
#include <EnhancedPDFExportHelper.hxx>
#include <docufld.hxx>

//UUUU
#include <frmfmt.hxx>

#include <unomid.h>

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
                                   const SwTextNode& rTextNode )
{
    delete pRuler;
    pRuler = new SvxTabStopItem( rAttrSet.GetTabStops() );
    if ( rTextNode.GetListTabStopPosition( nListTabStopPosition ) )
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

    if ( !rTextNode.getIDocumentSettingAccess()->get(DocumentSettingId::TABS_RELATIVE_TO_INDENT) )
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
    nDefTabStop = USHRT_MAX;
}

void SwTextInfo::CtorInitTextInfo( SwTextFrm *pFrm )
{
    m_pPara = pFrm->GetPara();
    m_nTextStart = pFrm->GetOfst();
    if (!m_pPara)
    {
        SAL_WARN("sw.core", "+SwTextInfo::CTOR: missing paragraph information");
        pFrm->Format(pFrm->getRootFrm()->GetCurrShell()->GetOut());
        m_pPara = pFrm->GetPara();
    }
}

SwTextInfo::SwTextInfo( const SwTextInfo &rInf )
    : m_pPara( const_cast<SwTextInfo&>(rInf).GetParaPortion() )
    , m_nTextStart( rInf.GetTextStart() )
{ }

#if OSL_DEBUG_LEVEL > 0

void ChkOutDev( const SwTextSizeInfo &rInf )
{
    if ( !rInf.GetVsh() )
        return;

    const OutputDevice* pOut = rInf.GetOut();
    const OutputDevice* pRef = rInf.GetRefDev();
    OSL_ENSURE( pOut && pRef, "ChkOutDev: invalid output devices" );
}
#endif

inline sal_Int32 GetMinLen( const SwTextSizeInfo &rInf )
{
    const sal_Int32 nTextLen = rInf.GetText().getLength();
    if (rInf.GetLen() == COMPLETE_STRING)
        return nTextLen;
    const sal_Int32 nInfLen = rInf.GetIdx() + rInf.GetLen();
    return std::min(nTextLen, nInfLen);
}

SwTextSizeInfo::SwTextSizeInfo()
: m_pKanaComp(0)
, m_pVsh(0)
, m_pOut(0)
, m_pRef(0)
, m_pFnt(0)
, m_pUnderFnt(0)
, m_pFrm(0)
, m_pOpt(0)
, m_pText(0)
, m_nIdx(0)
, m_nLen(0)
, m_nKanaIdx(0)
, m_bOnWin    (false)
, m_bNotEOL   (false)
, m_bURLNotify(false)
, m_bStopUnderflow(false)
, m_bFootnoteInside(false)
, m_bOtherThanFootnoteInside(false)
, m_bMulti(false)
, m_bFirstMulti(false)
, m_bRuby(false)
, m_bHanging(false)
, m_bScriptSpace(false)
, m_bForbiddenChars(false)
, m_bSnapToGrid(false)
, m_nDirection(0)
{}

SwTextSizeInfo::SwTextSizeInfo( const SwTextSizeInfo &rNew )
    : SwTextInfo( rNew ),
      m_pKanaComp(rNew.GetpKanaComp()),
      m_pVsh(const_cast<SwTextSizeInfo&>(rNew).GetVsh()),
      m_pOut(const_cast<SwTextSizeInfo&>(rNew).GetOut()),
      m_pRef(const_cast<SwTextSizeInfo&>(rNew).GetRefDev()),
      m_pFnt(const_cast<SwTextSizeInfo&>(rNew).GetFont()),
      m_pUnderFnt(rNew.GetUnderFnt()),
      m_pFrm(rNew.m_pFrm),
      m_pOpt(&rNew.GetOpt()),
      m_pText(&rNew.GetText()),
      m_nIdx(rNew.GetIdx()),
      m_nLen(rNew.GetLen()),
      m_nKanaIdx( rNew.GetKanaIdx() ),
      m_bOnWin( rNew.OnWin() ),
      m_bNotEOL( rNew.NotEOL() ),
      m_bURLNotify( rNew.URLNotify() ),
      m_bStopUnderflow( rNew.StopUnderflow() ),
      m_bFootnoteInside( rNew.IsFootnoteInside() ),
      m_bOtherThanFootnoteInside( rNew.IsOtherThanFootnoteInside() ),
      m_bMulti( rNew.IsMulti() ),
      m_bFirstMulti( rNew.IsFirstMulti() ),
      m_bRuby( rNew.IsRuby() ),
      m_bHanging( rNew.IsHanging() ),
      m_bScriptSpace( rNew.HasScriptSpace() ),
      m_bForbiddenChars( rNew.HasForbiddenChars() ),
      m_bSnapToGrid( rNew.SnapToGrid() ),
      m_nDirection( rNew.GetDirection() )
{
#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif
}

void SwTextSizeInfo::CtorInitTextSizeInfo( OutputDevice* pRenderContext, SwTextFrm *pFrame, SwFont *pNewFnt,
                   const sal_Int32 nNewIdx, const sal_Int32 nNewLen )
{
    m_pKanaComp = NULL;
    m_nKanaIdx = 0;
    m_pFrm = pFrame;
    CtorInitTextInfo( m_pFrm );
    const SwTextNode *pNd = m_pFrm->GetTextNode();
    m_pVsh = m_pFrm->getRootFrm()->GetCurrShell();

    // Get the output and reference device
    if ( m_pVsh )
    {
        m_pOut = pRenderContext;
        m_pRef = &m_pVsh->GetRefDev();
        m_bOnWin = m_pVsh->GetWin() || OUTDEV_WINDOW == m_pOut->GetOutDevType() || m_pVsh->isOutputToWindow();
    }
    else
    {
        // Access via StarONE. We do not need a Shell or an active one.
        if ( pNd->getIDocumentSettingAccess()->get(DocumentSettingId::HTML_MODE) )
        {
            // We can only pick the AppWin here? (there's nothing better to pick?)
            m_pOut = Application::GetDefaultDevice();
        }
        else
            m_pOut = pNd->getIDocumentDeviceAccess().getPrinter( false );

        m_pRef = m_pOut;
    }

#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif

    // Set default layout mode ( LTR or RTL ).
    if ( m_pFrm->IsRightToLeft() )
    {
        m_pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
        m_pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
        m_nDirection = DIR_RIGHT2LEFT;
    }
    else
    {
        m_pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
        m_pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
        m_nDirection = DIR_LEFT2RIGHT;
    }

    // The Options

    m_pOpt = m_pVsh ?
           m_pVsh->GetViewOptions() :
           SW_MOD()->GetViewOption( pNd->getIDocumentSettingAccess()->get(DocumentSettingId::HTML_MODE) ); // Options from Module, due to StarONE

    // bURLNotify is set if MakeGraphic prepares it
    // TODO: Unwind
    m_bURLNotify = pNoteURL && !m_bOnWin;

    SetSnapToGrid( pNd->GetSwAttrSet().GetParaGrid().GetValue() &&
                   m_pFrm->IsInDocBody() );

    m_pFnt = pNewFnt;
    m_pUnderFnt = 0;
    m_pText = &pNd->GetText();

    m_nIdx = nNewIdx;
    m_nLen = nNewLen;
    m_bNotEOL = false;
    m_bStopUnderflow = m_bFootnoteInside = m_bOtherThanFootnoteInside = false;
    m_bMulti = m_bFirstMulti = m_bRuby = m_bHanging = m_bScriptSpace =
        m_bForbiddenChars = false;

    SetLen( GetMinLen( *this ) );
}

SwTextSizeInfo::SwTextSizeInfo( const SwTextSizeInfo &rNew, const OUString* pText,
                              const sal_Int32 nIndex, const sal_Int32 nLength )
    : SwTextInfo( rNew ),
      m_pKanaComp(rNew.GetpKanaComp()),
      m_pVsh(const_cast<SwTextSizeInfo&>(rNew).GetVsh()),
      m_pOut(const_cast<SwTextSizeInfo&>(rNew).GetOut()),
      m_pRef(const_cast<SwTextSizeInfo&>(rNew).GetRefDev()),
      m_pFnt(const_cast<SwTextSizeInfo&>(rNew).GetFont()),
      m_pUnderFnt(rNew.GetUnderFnt()),
      m_pFrm( rNew.m_pFrm ),
      m_pOpt(&rNew.GetOpt()),
      m_pText(pText),
      m_nIdx(nIndex),
      m_nLen(nLength),
      m_nKanaIdx( rNew.GetKanaIdx() ),
      m_bOnWin( rNew.OnWin() ),
      m_bNotEOL( rNew.NotEOL() ),
      m_bURLNotify( rNew.URLNotify() ),
      m_bStopUnderflow( rNew.StopUnderflow() ),
      m_bFootnoteInside( rNew.IsFootnoteInside() ),
      m_bOtherThanFootnoteInside( rNew.IsOtherThanFootnoteInside() ),
      m_bMulti( rNew.IsMulti() ),
      m_bFirstMulti( rNew.IsFirstMulti() ),
      m_bRuby( rNew.IsRuby() ),
      m_bHanging( rNew.IsHanging() ),
      m_bScriptSpace( rNew.HasScriptSpace() ),
      m_bForbiddenChars( rNew.HasForbiddenChars() ),
      m_bSnapToGrid( rNew.SnapToGrid() ),
      m_nDirection( rNew.GetDirection() )
{
#if OSL_DEBUG_LEVEL > 0
    ChkOutDev( *this );
#endif
    SetLen( GetMinLen( *this ) );
}

SwTextSizeInfo::SwTextSizeInfo( SwTextFrm *pTextFrm, SwFont *pTextFnt,
               const sal_Int32 nIndex,
               const sal_Int32 nLength )
    : m_bOnWin(false)
{
    CtorInitTextSizeInfo( pTextFrm->getRootFrm()->GetCurrShell()->GetOut(), pTextFrm, pTextFnt, nIndex, nLength );
}

void SwTextSizeInfo::SelectFont()
{
     // The path needs to go via ChgPhysFnt or the FontMetricCache gets confused.
     // In this case pLastMet has it's old value.
     // Wrong: GetOut()->SetFont( GetFont()->GetFnt() );
    GetFont()->Invalidate();
    GetFont()->ChgPhysFnt( m_pVsh, *GetOut() );
}

void SwTextSizeInfo::NoteAnimation() const
{
    if( OnWin() )
        SwRootFrm::FlushVout();

    OSL_ENSURE( m_pOut == m_pVsh->GetOut(),
            "SwTextSizeInfo::NoteAnimation() changed m_pOut" );
}

SwPosSize SwTextSizeInfo::GetTextSize( OutputDevice* pOutDev,
                                     const SwScriptInfo* pSI,
                                     const OUString& rText,
                                     const sal_Int32 nIndex,
                                     const sal_Int32 nLength,
                                     const sal_uInt16 nComp) const
{
    SwDrawTextInfo aDrawInf( m_pVsh, *pOutDev, pSI, rText, nIndex, nLength );
    aDrawInf.SetFrm( m_pFrm );
    aDrawInf.SetFont( m_pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    return m_pFnt->_GetTextSize( aDrawInf );
}

SwPosSize SwTextSizeInfo::GetTextSize() const
{
    const SwScriptInfo& rSI =
                     const_cast<SwParaPortion*>(GetParaPortion())->GetScriptInfo();

    // in some cases, compression is not allowed or suppressed for
    // performance reasons
    sal_uInt16 nComp =( SW_CJK == GetFont()->GetActual() &&
                    rSI.CountCompChg() &&
                    ! IsMulti() ) ?
                    GetKanaComp() :
                                0 ;

    SwDrawTextInfo aDrawInf( m_pVsh, *m_pOut, &rSI, *m_pText, m_nIdx, m_nLen );
    aDrawInf.SetFrm( m_pFrm );
    aDrawInf.SetFont( m_pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    return m_pFnt->_GetTextSize( aDrawInf );
}

void SwTextSizeInfo::GetTextSize( const SwScriptInfo* pSI, const sal_Int32 nIndex,
                                const sal_Int32 nLength, const sal_uInt16 nComp,
                                sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff,
                                vcl::TextLayoutCache const*const pCache) const
{
    SwDrawTextInfo aDrawInf( m_pVsh, *m_pOut, pSI, *m_pText, nIndex, nLength,
            0, false, pCache);
    aDrawInf.SetFrm( m_pFrm );
    aDrawInf.SetFont( m_pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    SwPosSize aSize = m_pFnt->_GetTextSize( aDrawInf );
    nMaxSizeDiff = (sal_uInt16)aDrawInf.GetKanaDiff();
    nMinSize = aSize.Width();
}

sal_Int32 SwTextSizeInfo::GetTextBreak( const long nLineWidth,
                                       const sal_Int32 nMaxLen,
                                       const sal_uInt16 nComp,
                                       vcl::TextLayoutCache const*const pCache) const
{
    const SwScriptInfo& rScriptInfo =
                     const_cast<SwParaPortion*>(GetParaPortion())->GetScriptInfo();

    OSL_ENSURE( m_pRef == m_pOut, "GetTextBreak is supposed to use the RefDev" );
    SwDrawTextInfo aDrawInf( m_pVsh, *m_pOut, &rScriptInfo,
                             *m_pText, GetIdx(), nMaxLen,  0, false, pCache );
    aDrawInf.SetFrm( m_pFrm );
    aDrawInf.SetFont( m_pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( 0 );

    return m_pFnt->GetTextBreak( aDrawInf, nLineWidth );
}

sal_Int32 SwTextSizeInfo::GetTextBreak( const long nLineWidth,
                                       const sal_Int32 nMaxLen,
                                       const sal_uInt16 nComp,
                                       sal_Int32& rExtraCharPos,
                                       vcl::TextLayoutCache const*const pCache) const
{
    const SwScriptInfo& rScriptInfo =
                     const_cast<SwParaPortion*>(GetParaPortion())->GetScriptInfo();

    OSL_ENSURE( m_pRef == m_pOut, "GetTextBreak is supposed to use the RefDev" );
    SwDrawTextInfo aDrawInf( m_pVsh, *m_pOut, &rScriptInfo,
                             *m_pText, GetIdx(), nMaxLen, 0, false, pCache );
    aDrawInf.SetFrm( m_pFrm );
    aDrawInf.SetFont( m_pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( &rExtraCharPos );

    return m_pFnt->GetTextBreak( aDrawInf, nLineWidth );
}

bool SwTextSizeInfo::_HasHint( const SwTextNode* pTextNode, sal_Int32 nPos )
{
    return pTextNode->GetTextAttrForCharAt(nPos);
}

void SwTextPaintInfo::CtorInitTextPaintInfo( OutputDevice* pRenderContext, SwTextFrm *pFrame, const SwRect &rPaint )
{
    CtorInitTextSizeInfo( pRenderContext, pFrame );
    aTextFly.CtorInitTextFly( pFrame ),
    aPaintRect = rPaint;
    nSpaceIdx = 0;
    pSpaceAdd = NULL;
    pWrongList = NULL;
    pGrammarCheckList = NULL;
    pSmartTags = NULL;

#if OSL_DEBUG_LEVEL > 1
    pBrushItem = reinterpret_cast<SvxBrushItem*>(-1);
#else
    pBrushItem = 0;
#endif
}

SwTextPaintInfo::SwTextPaintInfo( const SwTextPaintInfo &rInf, const OUString* pText )
    : SwTextSizeInfo( rInf, pText ),
      pWrongList( rInf.GetpWrongList() ),
      pGrammarCheckList( rInf.GetGrammarCheckList() ),
      pSmartTags( rInf.GetSmartTags() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pBrushItem( rInf.GetBrushItem() ),
      aTextFly( rInf.GetTextFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() )
{ }

SwTextPaintInfo::SwTextPaintInfo( const SwTextPaintInfo &rInf )
    : SwTextSizeInfo( rInf ),
      pWrongList( rInf.GetpWrongList() ),
      pGrammarCheckList( rInf.GetGrammarCheckList() ),
      pSmartTags( rInf.GetSmartTags() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pBrushItem( rInf.GetBrushItem() ),
      aTextFly( rInf.GetTextFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() )
{ }

SwTextPaintInfo::SwTextPaintInfo( SwTextFrm *pFrame, const SwRect &rPaint )
{
    CtorInitTextPaintInfo( pFrame->getRootFrm()->GetCurrShell()->GetOut(), pFrame, rPaint );
}

extern Color aGlobalRetoucheColor;

/// Returns if the current background color is dark.
static bool lcl_IsDarkBackground( const SwTextPaintInfo& rInf )
{
    const Color* pCol = rInf.GetFont()->GetBackColor();
    if( ! pCol || COL_TRANSPARENT == pCol->GetColor() )
    {
        const SvxBrushItem* pItem;
        SwRect aOrigBackRect;

        //UUUU
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

        // Consider, that [GetBackgroundBrush(...)] can set <pCol>
        // See implementation in /core/layout/paintfrm.cxx
        // There is a background color, if there is a background brush and
        // its color is *not* "no fill"/"auto fill".
        if( rInf.GetTextFrm()->GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, false ) )
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

void SwTextPaintInfo::_DrawText( const OUString &rText, const SwLinePortion &rPor,
                                const sal_Int32 nStart, const sal_Int32 nLength,
                                const bool bKern, const bool bWrong,
                                const bool bSmartTag,
                                const bool bGrammarCheck )
{
    if( !nLength )
        return;

    if( GetFont()->IsBlink() && OnWin() && rPor.Width() )
    {
        // check if accessibility options allow blinking portions:
        const SwViewShell* pSh = GetTextFrm()->getRootFrm()->GetCurrShell();
        if ( pSh && ! pSh->GetAccessibilityOptions()->IsStopAnimatedText() &&
             ! pSh->IsPreview() )
        {
            if( !pBlink )
                pBlink = new SwBlink();

            Point aPoint( aPos );

            if ( GetTextFrm()->IsRightToLeft() )
                GetTextFrm()->SwitchLTRtoRTL( aPoint );

            if ( TEXT_LAYOUT_BIDI_STRONG != GetOut()->GetLayoutMode() )
                aPoint.X() -= rPor.Width();

            if ( GetTextFrm()->IsVertical() )
                GetTextFrm()->SwitchHorizontalToVertical( aPoint );

            pBlink->Insert( aPoint, &rPor, GetTextFrm(), m_pFnt->GetOrientation() );

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
    if ( ! rPor.InFieldGrp() )
        pSI = &GetParaPortion()->GetScriptInfo();

    // in some cases, kana compression is not allowed or suppressed for
    // performance reasons
    sal_uInt16 nComp = 0;
    if ( ! IsMulti() )
        nComp = GetKanaComp();

    bool bCfgIsAutoGrammar = false;
    SvtLinguConfig().GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bCfgIsAutoGrammar;
    const bool bBullet = OnWin() && GetOpt().IsBlank() && IsNoSymbol();
    const bool bTmpWrong = bWrong && OnWin() && GetOpt().IsOnlineSpell();
    const bool bTmpGrammarCheck = bGrammarCheck && OnWin() && bCfgIsAutoGrammar && GetOpt().IsOnlineSpell();
    const bool bTmpSmart = bSmartTag && OnWin() && !GetOpt().IsPagePreview() && SwSmartTagMgr::Get().IsSmartTagsEnabled();

    OSL_ENSURE( GetParaPortion(), "No paragraph!");
    SwDrawTextInfo aDrawInf( m_pFrm->getRootFrm()->GetCurrShell(), *m_pOut, pSI, rText, nStart, nLength,
                             rPor.Width(), bBullet );

    aDrawInf.SetLeft( GetPaintRect().Left() );
    aDrawInf.SetRight( GetPaintRect().Right());

    aDrawInf.SetUnderFnt( m_pUnderFnt );

    const long nSpaceAdd = ( rPor.IsBlankPortion() || rPor.IsDropPortion() ||
                             rPor.InNumberGrp() ) ? 0 : GetSpaceAdd();
    if ( nSpaceAdd )
    {
        sal_Int32 nCharCnt = 0;
        // #i41860# Thai justified alignment needs some
        // additional information:
        aDrawInf.SetNumberOfBlanks( rPor.InTextGrp() ?
                                    static_cast<const SwTextPortion&>(rPor).GetSpaceCnt( *this, nCharCnt ) :
                                    0 );
    }

    aDrawInf.SetSpace( nSpaceAdd );
    aDrawInf.SetKanaComp( nComp );

    // the font is used to identify the current script via nActual
    aDrawInf.SetFont( m_pFnt );
    // the frame is used to identify the orientation
    aDrawInf.SetFrm( GetTextFrm() );
    // we have to know if the paragraph should snap to grid
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    // for underlining we must know when not to add extra space behind
    // a character in justified mode
    aDrawInf.SetSpaceStop( ! rPor.GetPortion() ||
                             rPor.GetPortion()->InFixMargGrp() ||
                             rPor.GetPortion()->IsHolePortion() );

    // Draw text next to the left border
    Point aFontPos(aPos);
    if( m_pFnt->GetLeftBorder() && !static_cast<const SwTextPortion&>(rPor).GetJoinBorderWithPrev() )
    {
        const sal_uInt16 nLeftBorderSpace = m_pFnt->GetLeftBorderSpace();
        if ( GetTextFrm()->IsRightToLeft() )
        {
            aFontPos.X() -= nLeftBorderSpace;
        }
        else
        {
            switch( m_pFnt->GetOrientation(GetTextFrm()->IsVertical()) )
            {
                case 0 :
                    aFontPos.X() += nLeftBorderSpace;
                    break;
                case 900 :
                    aFontPos.Y() -= nLeftBorderSpace;
                    break;
                case 1800 :
                    aFontPos.X() -= nLeftBorderSpace;
                    break;
                case 2700 :
                    aFontPos.Y() += nLeftBorderSpace;
                    break;
            }
        }
        if( aFontPos.X() < 0 )
            aFontPos.X() = 0;
        if( aFontPos.X() < 0 )
            aFontPos.X() = 0;
    }

    if( GetTextFly().IsOn() )
    {
        // aPos needs to be the TopLeft, because we cannot calculate the
        // ClipRects otherwise
        const Point aPoint( aFontPos.X(), aFontPos.Y() - rPor.GetAscent() );
        const Size aSize( rPor.Width(), rPor.Height() );
        aDrawInf.SetPos( aPoint );
        aDrawInf.SetSize( aSize );
        aDrawInf.SetAscent( rPor.GetAscent() );
        aDrawInf.SetKern( bKern ? rPor.Width() : 0 );
        aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
        aDrawInf.SetGrammarCheck( bTmpGrammarCheck ? pGrammarCheckList : NULL );
        aDrawInf.SetSmartTags( bTmpSmart ? pSmartTags : NULL );
        GetTextFly().DrawTextOpaque( aDrawInf );
    }
    else
    {
        aDrawInf.SetPos( aFontPos );
        if( bKern )
            m_pFnt->_DrawStretchText( aDrawInf );
        else
        {
            aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
            aDrawInf.SetGrammarCheck( bTmpGrammarCheck ? pGrammarCheckList : NULL );
            aDrawInf.SetSmartTags( bTmpSmart ? pSmartTags : NULL );
            m_pFnt->_DrawText( aDrawInf );
        }
    }
}

void SwTextPaintInfo::CalcRect( const SwLinePortion& rPor,
                               SwRect* pRect, SwRect* pIntersect,
                               const bool bInsideBox ) const
{
    Size aSize( rPor.Width(), rPor.Height() );
    if( rPor.IsHangingPortion() )
        aSize.Width() = static_cast<const SwHangingPortion&>(rPor).GetInnerWidth();
    if( rPor.InSpaceGrp() && GetSpaceAdd() )
    {
        SwTwips nAdd = rPor.CalcSpacing( GetSpaceAdd(), *this );
        if( rPor.InFieldGrp() && GetSpaceAdd() < 0 && nAdd )
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
        if ( GetTextFrm()->IsVertLR() )
            aPoint.B() = Y() - rPor.Height() + rPor.GetAscent();
        else
            aPoint.B() = Y() - rPor.GetAscent();
    }

    // Adjust x coordinate if we are inside a bidi portion
    const bool bFrmDir = GetTextFrm()->IsRightToLeft();
    const bool bCounterDir = ( !bFrmDir && DIR_RIGHT2LEFT == GetDirection() ) ||
                             (  bFrmDir && DIR_LEFT2RIGHT == GetDirection() );

    if ( bCounterDir )
        aPoint.A() -= aSize.Width();

    SwRect aRect( aPoint, aSize );

    if ( GetTextFrm()->IsRightToLeft() )
        GetTextFrm()->SwitchLTRtoRTL( aRect );

    if ( GetTextFrm()->IsVertical() )
        GetTextFrm()->SwitchHorizontalToVertical( aRect );

    if( bInsideBox && rPor.InTextGrp() )
    {
        const bool bJoinWithPrev =
            static_cast<const SwTextPortion&>(rPor).GetJoinBorderWithPrev();
        const bool bJoinWithNext =
            static_cast<const SwTextPortion&>(rPor).GetJoinBorderWithNext();
        const bool bIsVert = GetTextFrm()->IsVertical();
        aRect.Top(aRect.Top() + GetFont()->CalcShadowSpace(SvxShadowItemSide::TOP, bIsVert, bJoinWithPrev, bJoinWithNext ));
        aRect.Bottom(aRect.Bottom() - GetFont()->CalcShadowSpace(SvxShadowItemSide::BOTTOM, bIsVert, bJoinWithPrev, bJoinWithNext ));
        aRect.Left(aRect.Left() + GetFont()->CalcShadowSpace(SvxShadowItemSide::LEFT, bIsVert, bJoinWithPrev, bJoinWithNext ));
        aRect.Right(aRect.Right() - GetFont()->CalcShadowSpace(SvxShadowItemSide::RIGHT, bIsVert, bJoinWithPrev, bJoinWithNext ));
    }

    if ( pRect )
        *pRect = aRect;

    if( aRect.HasArea() && pIntersect )
    {
        ::SwAlignRect( aRect, GetVsh(), GetOut() );

        if ( GetOut()->IsClipRegion() )
        {
            SwRect aClip( GetOut()->GetClipRegion().GetBoundRect() );
            aRect.Intersection( aClip );
        }

        *pIntersect = aRect;
    }
}

/**
 * Draws a special portion
 * E.g.: line break portion, tab portion
 *
 * @param rPor The portion
 * @param rRect The rectangle surrounding the character
 * @param rCol Specify a color for the character
 * @param bCenter Draw the character centered, otherwise left aligned
 * @param bRotate Rotate the character if character rotation is set
 */
static void lcl_DrawSpecial( const SwTextPaintInfo& rInf, const SwLinePortion& rPor,
                      SwRect& rRect, const Color& rCol, sal_Unicode cChar,
                      sal_uInt8 nOptions )
{
    bool bCenter = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_CENTER );
    bool bRotate = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_ROTATE );

    // rRect is given in absolute coordinates
    if ( rInf.GetTextFrm()->IsRightToLeft() )
        rInf.GetTextFrm()->SwitchRTLtoLTR( rRect );
    if ( rInf.GetTextFrm()->IsVertical() )
        rInf.GetTextFrm()->SwitchVerticalToHorizontal( rRect );

    const SwFont* pOldFnt = rInf.GetFont();

    // Font is generated only once:
    static SwFont* m_pFnt = 0;
    if ( ! m_pFnt )
    {
        m_pFnt = new SwFont( *pOldFnt );
        m_pFnt->SetFamily( FAMILY_DONTKNOW, m_pFnt->GetActual() );
        m_pFnt->SetName( numfunc::GetDefBulletFontname(), m_pFnt->GetActual() );
        m_pFnt->SetStyleName( aEmptyOUStr, m_pFnt->GetActual() );
        m_pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, m_pFnt->GetActual() );
    }

    // Some of the current values are set at the font:
    if ( ! bRotate )
        m_pFnt->SetVertical( 0, rInf.GetTextFrm()->IsVertical() );
    else
        m_pFnt->SetVertical( pOldFnt->GetOrientation() );

    m_pFnt->SetColor(rCol);

    Size aFontSize( 0, SPECIAL_FONT_HEIGHT );
    m_pFnt->SetSize( aFontSize, m_pFnt->GetActual() );

    const_cast<SwTextPaintInfo&>(rInf).SetFont( m_pFnt );

    // The maximum width depends on the current orientation
    const sal_uInt16 nDir = m_pFnt->GetOrientation( rInf.GetTextFrm()->IsVertical() );
    SwTwips nMaxWidth;
    if (nDir == 900 || nDir == 2700)
        nMaxWidth = rRect.Height();
    else
    {
        assert(nDir == 0); //Unknown direction set at font
        nMaxWidth = rRect.Width();
    }

    // check if char fits into rectangle
    const OUString aTmp( cChar );
    aFontSize = rInf.GetTextSize( aTmp ).SvLSize();
    while ( aFontSize.Width() > nMaxWidth )
    {
        SwTwips nFactor = ( 100 * aFontSize.Width() ) / nMaxWidth;
        const SwTwips nOldWidth = aFontSize.Width();

        // new height for font
        const sal_uInt8 nAct = m_pFnt->GetActual();
        aFontSize.Height() = ( 100 * m_pFnt->GetSize( nAct ).Height() ) / nFactor;
        aFontSize.Width() = ( 100 * m_pFnt->GetSize( nAct).Width() ) / nFactor;

        if ( !aFontSize.Width() && !aFontSize.Height() )
            break;

        m_pFnt->SetSize( aFontSize, nAct );

        aFontSize = rInf.GetTextSize( aTmp ).SvLSize();

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
    const_cast<SwTextPaintInfo&>(rInf).SetPos( aTmpPos );
    sal_uInt16 nOldWidth = rPor.Width();
    const_cast<SwLinePortion&>(rPor).Width( (sal_uInt16)aFontSize.Width() );
    rInf.DrawText( aTmp, rPor );
    const_cast<SwLinePortion&>(rPor).Width( nOldWidth );
    const_cast<SwTextPaintInfo&>(rInf).SetFont( const_cast<SwFont*>(pOldFnt) );
    const_cast<SwTextPaintInfo&>(rInf).SetPos( aOldPos );
}

void SwTextPaintInfo::DrawRect( const SwRect &rRect, bool bNoGraphic,
                               bool bRetouche ) const
{
    if ( OnWin() || !bRetouche )
    {
        if( aTextFly.IsOn() )
            const_cast<SwTextPaintInfo*>(this)->GetTextFly().
                DrawFlyRect( m_pOut, rRect, *this, bNoGraphic );
        else if ( bNoGraphic )
            m_pOut->DrawRect( rRect.SVRect() );
        else
        {
            if(pBrushItem != reinterpret_cast<SvxBrushItem*>(-1))
            {
                ::DrawGraphic( pBrushItem, m_pOut, aItemRect, rRect );
            }
            else
            {
                OSL_ENSURE(false, "DrawRect: Uninitialized BrushItem!" );
            }
        }
    }
}

void SwTextPaintInfo::DrawTab( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        SwRect aRect;
        CalcRect( rPor, &aRect );

        if ( ! aRect.HasArea() )
            return;

        const sal_Unicode cChar = GetTextFrm()->IsRightToLeft() ? CHAR_TAB_RTL : CHAR_TAB;
        const sal_uInt8 nOptions = DRAW_SPECIAL_OPTIONS_CENTER | DRAW_SPECIAL_OPTIONS_ROTATE;

        lcl_DrawSpecial( *this, rPor, aRect, Color(NON_PRINTING_CHARACTER_COLOR), cChar, nOptions );
    }
}

void SwTextPaintInfo::DrawLineBreak( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        sal_uInt16 nOldWidth = rPor.Width();
        const_cast<SwLinePortion&>(rPor).Width( LINE_BREAK_WIDTH );

        SwRect aRect;
        CalcRect( rPor, &aRect );

        if( aRect.HasArea() )
        {
            const sal_Unicode cChar = GetTextFrm()->IsRightToLeft() ?
                                      CHAR_LINEBREAK_RTL : CHAR_LINEBREAK;
            const sal_uInt8 nOptions = 0;

            lcl_DrawSpecial( *this, rPor, aRect, Color(NON_PRINTING_CHARACTER_COLOR), cChar, nOptions );
        }

        const_cast<SwLinePortion&>(rPor).Width( nOldWidth );
    }
}

void SwTextPaintInfo::DrawRedArrow( const SwLinePortion &rPor ) const
{
    Size aSize( SPECIAL_FONT_HEIGHT, SPECIAL_FONT_HEIGHT );
    SwRect aRect( static_cast<const SwArrowPortion&>(rPor).GetPos(), aSize );
    sal_Unicode cChar;
    if( static_cast<const SwArrowPortion&>(rPor).IsLeft() )
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

    if ( GetTextFrm()->IsVertical() )
        GetTextFrm()->SwitchHorizontalToVertical( aRect );

    Color aCol( COL_LIGHTRED );

    if( aRect.HasArea() )
    {
        const sal_uInt8 nOptions = 0;
        lcl_DrawSpecial( *this, rPor, aRect, aCol, cChar, nOptions );
    }
}

void SwTextPaintInfo::DrawPostIts( const SwLinePortion&, bool bScript ) const
{
    if( OnWin() && m_pOpt->IsPostIts() )
    {
        Size aSize;
        Point aTmp;

        const sal_uInt16 nPostItsWidth = SwViewOption::GetPostItsWidth( GetOut() );
        const sal_uInt16 nFontHeight = m_pFnt->GetHeight( m_pVsh, *GetOut() );
        const sal_uInt16 nFontAscent = m_pFnt->GetAscent( m_pVsh, *GetOut() );

        switch ( m_pFnt->GetOrientation( GetTextFrm()->IsVertical() ) )
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

        if ( GetTextFrm()->IsRightToLeft() )
            GetTextFrm()->SwitchLTRtoRTL( aTmpRect );

        if ( GetTextFrm()->IsVertical() )
            GetTextFrm()->SwitchHorizontalToVertical( aTmpRect );

        const Rectangle aRect( aTmpRect.SVRect() );
        SwViewOption::PaintPostIts( const_cast<OutputDevice*>(GetOut()), aRect, bScript );
    }
}

void SwTextPaintInfo::DrawCheckBox(const SwFieldFormCheckboxPortion &rPor, bool bChecked) const
{
    SwRect aIntersect;
    CalcRect( rPor, &aIntersect, 0 );
    if ( aIntersect.HasArea() )
    {
        if (OnWin() && SwViewOption::IsFieldShadings() &&
                !GetOpt().IsPagePreview())
        {
            OutputDevice* pOut = const_cast<OutputDevice*>(GetOut());
            pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
            pOut->SetFillColor( SwViewOption::GetFieldShadingsColor() );
            pOut->SetLineColor();
            pOut->DrawRect( aIntersect.SVRect() );
            pOut->Pop();
        }
        const int delta=10;
        Rectangle r(aIntersect.Left()+delta, aIntersect.Top()+delta, aIntersect.Right()-delta, aIntersect.Bottom()-delta);
        m_pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
        m_pOut->SetLineColor( Color(0, 0, 0));
        m_pOut->SetFillColor();
        m_pOut->DrawRect( r );
        if (bChecked)
        {
            m_pOut->DrawLine(r.TopLeft(), r.BottomRight());
            m_pOut->DrawLine(r.TopRight(), r.BottomLeft());
        }
        m_pOut->Pop();
    }
}

void SwTextPaintInfo::DrawBackground( const SwLinePortion &rPor ) const
{
    OSL_ENSURE( OnWin(), "SwTextPaintInfo::DrawBackground: printer pollution ?" );

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect, true );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pOut = const_cast<OutputDevice*>(GetOut());
        pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );

        // For dark background we do not want to have a filled rectangle
        if ( GetVsh() && GetVsh()->GetWin() && lcl_IsDarkBackground( *this ) )
        {
            pOut->SetLineColor( SwViewOption::GetFontColor().GetColor() );
        }
        else
        {
            pOut->SetFillColor( SwViewOption::GetFieldShadingsColor() );
            pOut->SetLineColor();
        }

        DrawRect( aIntersect, true );
        pOut->Pop();
    }
}

void SwTextPaintInfo::DrawBackBrush( const SwLinePortion &rPor ) const
{
    {
        SwRect aIntersect;
        CalcRect( rPor, &aIntersect, 0, true );
        if(aIntersect.HasArea())
        {
            SwTextNode *pNd = m_pFrm->GetTextNode();
            const ::sw::mark::IMark* pFieldmark = NULL;
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
            bool bIsStartMark=(1==GetLen() && CH_TXT_ATR_FIELDSTART==GetText()[GetIdx()]);
            if(pFieldmark) {
                OSL_TRACE("Found Fieldmark");
                SAL_INFO("sw.core", pFieldmark->ToString() << "\n");
            }
            if(bIsStartMark) OSL_TRACE("Found StartMark");
            if (OnWin() && (pFieldmark!=NULL || bIsStartMark) &&
                    SwViewOption::IsFieldShadings() &&
                    !GetOpt().IsPagePreview())
            {
                OutputDevice* pOutDev = const_cast<OutputDevice*>(GetOut());
                pOutDev->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
                pOutDev->SetFillColor( SwViewOption::GetFieldShadingsColor() );
                pOutDev->SetLineColor( );
                pOutDev->DrawRect( aIntersect.SVRect() );
                pOutDev->Pop();
            }
        }
    }

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect, true );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pTmpOut = const_cast<OutputDevice*>(GetOut());

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pTmpOut );

        Color aFillColor;

        if( m_pFnt->GetHighlightColor() != COL_TRANSPARENT )
        {
            aFillColor = m_pFnt->GetHighlightColor();
        }
        else
        {
            if( !m_pFnt->GetBackColor() )
                return;
            aFillColor = *m_pFnt->GetBackColor();
        }

        pTmpOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );

        pTmpOut->SetFillColor(aFillColor);
        pTmpOut->SetLineColor();

        DrawRect( aIntersect, true, false );

        pTmpOut->Pop();
    }
}

void SwTextPaintInfo::DrawBorder( const SwLinePortion &rPor ) const
{
    SwRect aDrawArea;
    CalcRect( rPor, &aDrawArea );
    if ( aDrawArea.HasArea() )
    {
        PaintCharacterBorder(
            *m_pFnt, aDrawArea, GetTextFrm()->IsVertical(),
            rPor.GetJoinBorderWithPrev(), rPor.GetJoinBorderWithNext());
    }
}

void SwTextPaintInfo::DrawViewOpt( const SwLinePortion &rPor,
                                  const sal_uInt16 nWhich ) const
{
    if( OnWin() && !IsMulti() )
    {
        bool bDraw = false;
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
            if ( !GetOpt().IsPagePreview()
                 && !GetOpt().IsReadonly()
                 && SwViewOption::IsFieldShadings()
                 && ( POR_NUMBER != nWhich
                      || m_pFrm->GetTextNode()->HasMarkedLabel())) // #i27615#
            {
                bDraw = true;
            }
            break;
        case POR_INPUTFLD:
            // input field shading also in read-only mode
            if ( !GetOpt().IsPagePreview()
                 && SwViewOption::IsFieldShadings() )
            {
                bDraw = true;
            }
            break;
        case POR_TAB:       if ( GetOpt().IsTab() )     bDraw = true; break;
        case POR_SOFTHYPH:  if ( GetOpt().IsSoftHyph() )bDraw = true; break;
        case POR_BLANK:     if ( GetOpt().IsHardBlank())bDraw = true; break;
        default:
            {
                OSL_ENSURE( false, "SwTextPaintInfo::DrawViewOpt: don't know how to draw this" );
                break;
            }
        }
        if ( bDraw )
            DrawBackground( rPor );
    }
}

void SwTextPaintInfo::_NotifyURL( const SwLinePortion &rPor ) const
{
    OSL_ENSURE( pNoteURL, "NotifyURL: pNoteURL gone with the wind!" );

    SwRect aIntersect;
    CalcRect( rPor, 0, &aIntersect );

    if( aIntersect.HasArea() )
    {
        SwTextNode *pNd = const_cast<SwTextNode*>(GetTextFrm()->GetTextNode());
        SwTextAttr *const pAttr =
            pNd->GetTextAttrAt(GetIdx(), RES_TXTATR_INETFMT);
        if( pAttr )
        {
            const SwFormatINetFormat& rFormat = pAttr->GetINetFormat();
            pNoteURL->InsertURLNote( rFormat.GetValue(), rFormat.GetTargetFrame(),
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

const PropertyValues & SwTextFormatInfo::GetHyphValues() const
{
    OSL_ENSURE( 2 == aHyphVals.getLength(),
            "hyphenation values not yet initialized" );
    return aHyphVals;
}

bool SwTextFormatInfo::InitHyph( const bool bAutoHyphen )
{
    const SwAttrSet& rAttrSet = GetTextFrm()->GetTextNode()->GetSwAttrSet();
    SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
    SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
    SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
    const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
    MaxHyph() = rAttr.GetMaxHyphens();
    const bool bAuto = bAutoHyphen || rAttr.IsHyphen();
    if( bAuto || bInterHyph )
    {
        const sal_Int16 nMinimalLeading  = std::max(rAttr.GetMinLead(), sal_uInt8(2));
        const sal_Int16 nMinimalTrailing = rAttr.GetMinTrail();
        lcl_InitHyphValues( aHyphVals, nMinimalLeading, nMinimalTrailing);
    }
    return bAuto;
}

void SwTextFormatInfo::CtorInitTextFormatInfo( OutputDevice* pRenderContext, SwTextFrm *pNewFrm, const bool bNewInterHyph,
                                const bool bNewQuick, const bool bTst )
{
    CtorInitTextPaintInfo( pRenderContext, pNewFrm, SwRect() );

    bQuick = bNewQuick;
    bInterHyph = bNewInterHyph;

    //! needs to be done in this order
    nMinLeading     = 2;
    nMinTrailing    = 2;
    nMinWordLength  = 0;
    bAutoHyph = InitHyph();

    bIgnoreFly = false;
    bFakeLineStart = false;
    bShift = false;
    bDropInit = false;
    bTestFormat = bTst;
    nLeft = 0;
    nRight = 0;
    nFirst = 0;
    nRealWidth = 0;
    nForcedLeftMargin = 0;
    pRest = 0;
    nLineHeight = 0;
    nLineNetHeight = 0;
    SetLineStart(0);

    SvtCTLOptions::TextNumerals const nTextNumerals(
            SW_MOD()->GetCTLOptions().GetCTLTextNumerals());
    // cannot cache for NUMERALS_CONTEXT because we need to know the string
    // for the whole paragraph now
    if (nTextNumerals != SvtCTLOptions::NUMERALS_CONTEXT)
    {
        // set digit mode to what will be used later to get same results
        SwDigitModeModifier const m(*m_pRef, LANGUAGE_NONE /*dummy*/);
        assert(m_pRef->GetDigitLanguage() != LANGUAGE_NONE);
        SetCachedVclData(m_pRef->CreateTextLayoutCache(*m_pText));
    }

    Init();
}

/**
 * If the Hyphenator returns ERROR or the language is set to NOLANGUAGE
 * we do not hyphenate.
 * Else, we always hyphenate if we do interactive hyphenation.
 * If we do not do interactive hyphenation, we only hyphenate if ParaFormat is
 * set to automatic hyphenation.
 */
bool SwTextFormatInfo::IsHyphenate() const
{
    if( !bInterHyph && !bAutoHyph )
        return false;

    LanguageType eTmp = GetFont()->GetLanguage();
    if( LANGUAGE_DONTKNOW == eTmp || LANGUAGE_NONE == eTmp )
        return false;

    uno::Reference< XHyphenator > xHyph = ::GetHyphenator();
    if (!xHyph.is())
        return false;

    if (bInterHyph)
        SvxSpellWrapper::CheckHyphLang( xHyph, eTmp );

    return xHyph->hasLocale( g_pBreakIt->GetLocale(eTmp) );
}

const SwFormatDrop *SwTextFormatInfo::GetDropFormat() const
{
    const SwFormatDrop *pDrop = &GetTextFrm()->GetTextNode()->GetSwAttrSet().GetDrop();
    if( 1 >= pDrop->GetLines() ||
        ( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
        pDrop = 0;
    return pDrop;
}

void SwTextFormatInfo::Init()
{
    // Not initialized: pRest, nLeft, nRight, nFirst, nRealWidth
    X(0);
    bArrowDone = bFull = bFootnoteDone = bErgoDone = bNumDone = bNoEndHyph =
        bNoMidHyph = bStop = bNewLine = bUnderflow = bTabOverflow = false;

    // generally we do not allow number portions in follows, except...
    if ( GetTextFrm()->IsFollow() )
    {
        const SwTextFrm* pMaster = GetTextFrm()->FindMaster();
        OSL_ENSURE(pMaster, "pTextFrm without Master");
        const SwLinePortion* pTmpPara = pMaster ? pMaster->GetPara() : NULL;

        // there is a master for this follow and the master does not have
        // any contents (especially it does not have a number portion)
        bNumDone = ! pTmpPara ||
                   ! static_cast<const SwParaPortion*>(pTmpPara)->GetFirstPortion()->IsFlyPortion();
    }

    pRoot = 0;
    pLast = 0;
    pFly = 0;
    pLastField = 0;
    pLastTab = 0;
    pUnderflow = 0;
    cTabDecimal = 0;
    nWidth = nRealWidth;
    nForcedLeftMargin = 0;
    nSoftHyphPos = 0;
    nUnderScorePos = COMPLETE_STRING;
    cHookChar = 0;
    SetIdx(0);
    SetLen( GetText().getLength() );
    SetPaintOfst(0);
}

SwTextFormatInfo::SwTextFormatInfo(OutputDevice* pRenderContext, SwTextFrm *pFrame, const bool bInterHyphL,
                                   const bool bQuickL, const bool bTst)
{
    CtorInitTextFormatInfo(pRenderContext, pFrame, bInterHyphL, bQuickL, bTst);
}

/**
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 */
SwTextFormatInfo::SwTextFormatInfo( const SwTextFormatInfo& rInf,
    SwLineLayout& rLay, SwTwips nActWidth ) : SwTextPaintInfo( rInf ),
    bTabOverflow( false )
{
    pRoot = &rLay;
    pLast = &rLay;
    pFly = NULL;
    pLastField = NULL;
    pUnderflow = NULL;
    pRest = NULL;
    pLastTab = NULL;

    nSoftHyphPos = 0;
    nUnderScorePos = COMPLETE_STRING;
    nLineStart = rInf.GetIdx();
    nLeft = rInf.nLeft;
    nRight = rInf.nRight;
    nFirst = rInf.nLeft;
    nRealWidth = sal_uInt16(nActWidth);
    nWidth = nRealWidth;
    nLineHeight = 0;
    nLineNetHeight = 0;
    nForcedLeftMargin = 0;

    nMinLeading = 0;
    nMinTrailing = 0;
    nMinWordLength = 0;
    bFull = false;
    bFootnoteDone = true;
    bErgoDone = true;
    bNumDone = true;
    bArrowDone = true;
    bStop = false;
    bNewLine = true;
    bShift = false;
    bUnderflow = false;
    bInterHyph = false;
    bAutoHyph = false;
    bDropInit = false;
    bQuick  = rInf.bQuick;
    bNoEndHyph = false;
    bNoMidHyph = false;
    bIgnoreFly = false;
    bFakeLineStart = false;

    cTabDecimal = 0;
    cHookChar = 0;
    nMaxHyph = 0;
    bTestFormat = rInf.bTestFormat;
    SetMulti( true );
    SetFirstMulti( rInf.IsFirstMulti() );
}

bool SwTextFormatInfo::_CheckFootnotePortion( SwLineLayout* pCurr )
{
    const sal_uInt16 nHeight = pCurr->GetRealHeight();
    for( SwLinePortion *pPor = pCurr->GetPortion(); pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->IsFootnotePortion() && nHeight > static_cast<SwFootnotePortion*>(pPor)->Orig() )
        {
            SetLineHeight( nHeight );
            SetLineNetHeight( pCurr->Height() );
            return true;
        }
    }
    return false;
}

sal_Int32 SwTextFormatInfo::ScanPortionEnd( const sal_Int32 nStart,
                                            const sal_Int32 nEnd )
{
    cHookChar = 0;
    sal_Int32 i = nStart;

    // Used for decimal tab handling:
    const sal_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
    const sal_Unicode cThousandSep  = ',' == cTabDec ? '.' : ',';

    // #i45951# German (Switzerland) uses ' as thousand separator
    const sal_Unicode cThousandSep2 = ',' == cTabDec ? '.' : '\'';

    bool bNumFound = false;
    const bool bTabCompat = GetTextFrm()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT);

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
            if ( COMPLETE_STRING == nUnderScorePos )
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

                // Compatibility: First non-digit character behind a
                // a digit character becomes the hook character
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
    if ( i == nEnd && i < GetText().getLength() && bNumFound )
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

bool SwTextFormatInfo::LastKernPortion()
{
    if( GetLast() )
    {
        if( GetLast()->IsKernPortion() )
            return true;
        if( GetLast()->Width() || ( GetLast()->GetLen() &&
            !GetLast()->IsHolePortion() ) )
            return false;
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
        return true;
    }
    return false;
}

SwTextSlot::SwTextSlot(
    const SwTextSizeInfo *pNew,
    const SwLinePortion *pPor,
    bool bTextLen,
    bool bExgLists,
    OUString const & rCh )
    : pOldText(0)
    , pOldSmartTagList(0)
    , pOldGrammarCheckList(0)
    , pTempList(0)
    , nIdx(0)
    , nLen(0)
    , pInf(NULL)
{
    if( rCh.isEmpty() )
    {
        bOn = pPor->GetExpText( *pNew, aText );
    }
    else
    {
        aText = rCh;
        bOn = true;
    }

    // The text is replaced ...
    if( bOn )
    {
        pInf = const_cast<SwTextSizeInfo*>(pNew);
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldText = &(pInf->GetText());
        m_pOldCachedVclData = pInf->GetCachedVclData();
        pInf->SetText( aText );
        pInf->SetIdx( 0 );
        pInf->SetLen( bTextLen ? pInf->GetText().getLength() : pPor->GetLen() );
        pInf->SetCachedVclData(nullptr);

        // ST2
        if ( bExgLists )
        {
            pOldSmartTagList = static_cast<SwTextPaintInfo*>(pInf)->GetSmartTags();
            if ( pOldSmartTagList )
            {
                const sal_uInt16 nPos = pOldSmartTagList->GetWrongPos(nIdx);
                const sal_Int32 nListPos = pOldSmartTagList->Pos(nPos);
                if( nListPos == nIdx )
                    static_cast<SwTextPaintInfo*>(pInf)->SetSmartTags( pOldSmartTagList->SubList( nPos ) );
                else if( !pTempList && nPos < pOldSmartTagList->Count() && nListPos < nIdx && !aText.isEmpty() )
                {
                    pTempList = new SwWrongList( WRONGLIST_SMARTTAG );
                    pTempList->Insert( OUString(), 0, 0, aText.getLength(), 0 );
                    static_cast<SwTextPaintInfo*>(pInf)->SetSmartTags( pTempList );
                }
                else
                    static_cast<SwTextPaintInfo*>(pInf)->SetSmartTags( 0);
            }
            pOldGrammarCheckList = static_cast<SwTextPaintInfo*>(pInf)->GetGrammarCheckList();
            if ( pOldGrammarCheckList )
            {
                const sal_uInt16 nPos = pOldGrammarCheckList->GetWrongPos(nIdx);
                const sal_Int32 nListPos = pOldGrammarCheckList->Pos(nPos);
                if( nListPos == nIdx )
                    static_cast<SwTextPaintInfo*>(pInf)->SetGrammarCheckList( pOldGrammarCheckList->SubList( nPos ) );
                else if( !pTempList && nPos < pOldGrammarCheckList->Count() && nListPos < nIdx && !aText.isEmpty() )
                {
                    pTempList = new SwWrongList( WRONGLIST_GRAMMAR );
                    pTempList->Insert( OUString(), 0, 0, aText.getLength(), 0 );
                    static_cast<SwTextPaintInfo*>(pInf)->SetGrammarCheckList( pTempList );
                }
                else
                    static_cast<SwTextPaintInfo*>(pInf)->SetGrammarCheckList( 0);
            }
        }
    }
}

SwTextSlot::~SwTextSlot()
{
    if( bOn )
    {
        pInf->SetCachedVclData(m_pOldCachedVclData);
        pInf->SetText( *pOldText );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );

        // ST2
        // Restore old smart tag list
        if ( pOldSmartTagList )
            static_cast<SwTextPaintInfo*>(pInf)->SetSmartTags( pOldSmartTagList );
        if ( pOldGrammarCheckList )
            static_cast<SwTextPaintInfo*>(pInf)->SetGrammarCheckList( pOldGrammarCheckList );
        delete pTempList;
    }
}

SwFontSave::SwFontSave(const SwTextSizeInfo &rInf, SwFont *pNew,
        SwAttrIter* pItr)
    : pInf(NULL)
    , pFnt(pNew ? const_cast<SwTextSizeInfo&>(rInf).GetFont() : NULL)
    , pIter(NULL)
{
    if( pFnt )
    {
        pInf = &const_cast<SwTextSizeInfo&>(rInf);
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
            pNew->SetTransparent( true );
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
            pIter->nPos = COMPLETE_STRING;
        }
    }
}

bool SwTextFormatInfo::ChgHyph( const bool bNew )
{
    const bool bOld = bAutoHyph;
    if( bAutoHyph != bNew )
    {
        bAutoHyph = bNew;
        InitHyph( bNew );
        // Set language in the Hyphenator
        if( m_pFnt )
            m_pFnt->ChgPhysFnt( m_pVsh, *m_pOut );
    }
    return bOld;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
