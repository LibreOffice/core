/*************************************************************************
 *
 *  $RCSfile: inftxt.cxx,v $
 *
 *  $Revision: 1.90 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:55:35 $
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

#include <com/sun/star/uno/Sequence.h>

#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_SRIPTSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <svx/pgrditem.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOptions
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>  // DrawGraphic
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // SwDoc
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>   // SwFmtDrop
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  // SwRootFrm
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>   // SwTxtInfo
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>   // SwFont
#endif
#ifndef _TXTFLY_HXX
#include <txtfly.hxx>   // SwTxtPaintInfo
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>    // SwBlink
#endif
#ifndef _NOTEURL_HXX
#include <noteurl.hxx>  // SwNoteURL
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx> // SwDrawTextInfo
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>   // SwFtnPortion
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>       // SwHangingPortion
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _ITRATR_HXX
#include <itratr.hxx>
#endif
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
#define CHAR_UNDERSCORE ((sal_Unicode)0x005F)
#define CHAR_LEFT_ARROW ((sal_Unicode)0x25C0)
#define CHAR_RIGHT_ARROW ((sal_Unicode)0x25B6)
#define CHAR_TAB ((sal_Unicode)0x2192)
#define CHAR_TAB_RTL ((sal_Unicode)0x2190)
#define CHAR_LINEBREAK ((sal_Unicode)0x21B5)
#define CHAR_LINEBREAK_RTL ((sal_Unicode)0x21B3)

#ifdef BIDI
#define DRAW_SPECIAL_OPTIONS_CENTER 1
#define DRAW_SPECIAL_OPTIONS_ROTATE 2
#endif

// steht im number.cxx
extern const sal_Char __FAR_DATA sBulletFntName[];

// OD 24.01.2003 #106593# - no longer needed, included in <frmtool.hxx>
//extern void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh );

#ifndef PRODUCT
// Test2: WYSIWYG++
// Test4: WYSIWYG debug
static sal_Bool bDbgLow = sal_False;
#endif

#ifndef PRODUCT

sal_Bool SwTxtSizeInfo::IsOptCalm() const { return !GetOpt().IsTest3(); }

sal_Bool SwTxtSizeInfo::IsOptLow() const { return bDbgLow; }

sal_Bool SwTxtSizeInfo::IsOptDbg() const { return GetOpt().IsTest4(); }

sal_Bool SwTxtSizeInfo::IsOptTest1() const { return GetOpt().IsTest1(); }

sal_Bool SwTxtSizeInfo::IsOptTest2() const { return GetOpt().IsTest2(); }

sal_Bool SwTxtSizeInfo::IsOptTest3() const { return GetOpt().IsTest3(); }

sal_Bool SwTxtSizeInfo::IsOptTest4() const { return GetOpt().IsTest4(); }

sal_Bool SwTxtSizeInfo::IsOptTest5() const { return GetOpt().IsTest5(); }

sal_Bool SwTxtSizeInfo::IsOptTest6() const { return GetOpt().IsTest6(); }

sal_Bool SwTxtSizeInfo::IsOptTest7() const { return GetOpt().IsTest7(); }

sal_Bool SwTxtSizeInfo::IsOptTest8() const { return GetOpt().IsTest8(); }

#endif

/*************************************************************************
 *                      SwLineInfo::SwLineInfo()
 *************************************************************************/

void SwLineInfo::CtorInit( const SwAttrSet& rAttrSet )
{
    pRuler = &rAttrSet.GetTabStops();
    pSpace = &rAttrSet.GetLineSpacing();
    nVertAlign = rAttrSet.GetParaVertAlign().GetValue();
    nDefTabStop = MSHRT_MAX;
}

/*************************************************************************
 *                      SwTxtInfo::CtorInit()
 *************************************************************************/

void SwTxtInfo::CtorInit( SwTxtFrm *pFrm )
{
    pPara = pFrm->GetPara();
    nTxtStart = pFrm->GetOfst();
    if( !pPara )
    {
        ASSERT( pPara, "+SwTxtInfo::CTOR: missing paragraph information" );
        pFrm->Format();
        pPara = pFrm->GetPara();
    }
}

SwTxtInfo::SwTxtInfo( const SwTxtInfo &rInf )
    : pPara( ((SwTxtInfo&)rInf).GetParaPortion() ),
      nTxtStart( rInf.GetTxtStart() )
{ }


#ifndef PRODUCT
/*************************************************************************
 *                      ChkOutDev()
 *************************************************************************/

void ChkOutDev( const SwTxtSizeInfo &rInf )
{
    if ( !rInf.GetVsh() )
        return;

    const OutputDevice* pOut = rInf.GetOut();
    const OutputDevice* pWin = rInf.GetVsh()->GetWin();
    const OutputDevice* pRef = rInf.GetRefDev();
    ASSERT( pOut && pRef, "ChkOutDev: invalid output devices" )
}
#endif  // PRODUCT


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
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      bSnapToGrid( rNew.SnapToGrid() ),
      nDirection( rNew.GetDirection() )
{
#ifndef PRODUCT
    ChkOutDev( *this );
#endif
}

void SwTxtSizeInfo::CtorInit( SwTxtFrm *pFrame, SwFont *pNewFnt,
                   const xub_StrLen nNewIdx, const xub_StrLen nNewLen )
{
    pKanaComp = NULL;
    nKanaIdx = 0;
    pFrm = pFrame;
    SwTxtInfo::CtorInit( pFrm );
    const SwTxtNode *pNd = pFrm->GetTxtNode();
    pVsh = pFrm->GetShell();

    // Get the output and reference device
    if ( pVsh )
    {
        pOut = pVsh->GetOut();
        pRef = &pVsh->GetRefDev();
        bOnWin = pVsh->GetWin() || OUTDEV_WINDOW == pOut->GetOutDevType();
    }
    else
    {
        //Zugriff ueber StarONE, es muss keine Shell existieren oder aktiv sein.
        ASSERT( pVsh, "SwTxtSizeInfo::CtorInit(), pVsh = 0" )
        if ( pNd->GetDoc()->IsBrowseMode() ) //?!?!?!?
            //in Ermangelung eines Besseren kann hier ja wohl nur noch das
            //AppWin genommen werden?
            pOut = GetpApp()->GetDefaultDevice();
        else
            pOut = pNd->GetDoc()->GetPrt(); //Muss es geben (oder sal_True uebergeben?)
        pRef = pOut;
    }

#ifndef PRODUCT
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

    LanguageType eLang;
    const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
    if ( SvtCTLOptions::NUMERALS_HINDI == rCTLOptions.GetCTLTextNumerals() )
        eLang = LANGUAGE_ARABIC;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == rCTLOptions.GetCTLTextNumerals() )
        eLang = LANGUAGE_ENGLISH;
    else
        eLang = (LanguageType)::GetAppLanguage();

    pOut->SetDigitLanguage( eLang );
    pRef->SetDigitLanguage( eLang );

    //
    // The Options
    //
    pOpt = pVsh ?
           pVsh->GetViewOptions() :
           SW_MOD()->GetViewOption(pNd->GetDoc()->IsHTMLMode()); //Options vom Module wg. StarONE

    // bURLNotify wird gesetzt, wenn MakeGraphic dies vorbereitet
    // TODO: Aufdröseln
    bURLNotify = pNoteURL && !bOnWin;
//    bURLNotify = pNoteURL && !bOnWin
//        && (pOut && OUTDEV_PRINTER != pOut->GetOutDevType());

    SetSnapToGrid( pNd->GetSwAttrSet().GetParaGrid().GetValue() &&
                   pFrm->IsInDocBody() );

    pFnt = pNewFnt;
    pUnderFnt = 0;
    pTxt = &pNd->GetTxt();

    nIdx = nNewIdx;
    nLen = nNewLen;
    bNotEOL = sal_False;
    bStopUnderFlow = bFtnInside = sal_False;
    bMulti = bFirstMulti = bRuby = bHanging = bScriptSpace =
        bForbiddenChars = sal_False;
#ifndef BIDI
    nDirection = DIR_LEFT2RIGHT;
#endif

    SetLen( GetMinLen( *this ) );
}

SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew, const XubString &rTxt,
                              const xub_StrLen nIdx, const xub_StrLen nLen )
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
      nIdx(nIdx),
      nLen(nLen),
      nKanaIdx( rNew.GetKanaIdx() ),
      bOnWin( rNew.OnWin() ),
      bNotEOL( rNew.NotEOL() ),
      bURLNotify( rNew.URLNotify() ),
      bStopUnderFlow( rNew.StopUnderFlow() ),
      bFtnInside( rNew.IsFtnInside() ),
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      bSnapToGrid( rNew.SnapToGrid() ),
      nDirection( rNew.GetDirection() )
{
#ifndef PRODUCT
    ChkOutDev( *this );
#endif
    SetLen( GetMinLen( *this ) );
}

/*************************************************************************
 *                      SwTxtSizeInfo::SelectFont()
 *************************************************************************/

void SwTxtSizeInfo::SelectFont()
{
    // 8731: Der Weg muss ueber ChgPhysFnt gehen, sonst geraet
    // der FontMetricCache durcheinander. In diesem Fall steht pLastMet
    // auf dem alten Wert.
    // Falsch: GetOut()->SetFont( GetFont()->GetFnt() );
    GetFont()->Invalidate();
    GetFont()->ChgPhysFnt( pVsh, *GetOut() );
}

/*************************************************************************
 *                      SwTxtSizeInfo::NoteAnimation()
 *************************************************************************/

void SwTxtSizeInfo::NoteAnimation() const
{
    if( OnWin() )
        SwRootFrm::FlushVout();

    ASSERT( pOut == pVsh->GetOut(),
            "SwTxtSizeInfo::NoteAnimation() changed pOut" )
}

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

SwPosSize SwTxtSizeInfo::GetTxtSize( OutputDevice* pOutDev,
                                     const SwScriptInfo* pSI,
                                     const XubString& rTxt,
                                     const xub_StrLen nIdx,
                                     const xub_StrLen nLen,
                                     const USHORT nComp ) const
{
    SwDrawTextInfo aDrawInf( pVsh, *pOutDev, pSI, rTxt, nIdx, nLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
    return aSize;
}

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

SwPosSize SwTxtSizeInfo::GetTxtSize() const
{
    const SwScriptInfo& rSI =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    // in some cases, compression is not allowed or surpressed for
    // performance reasons
    USHORT nComp =( SW_CJK == GetFont()->GetActual() &&
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

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

void SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
                                const xub_StrLen nLen, const USHORT nComp,
                                USHORT& nMinSize, USHORT& nMaxSizeDiff ) const
{
    SwDrawTextInfo aDrawInf( pVsh, *pOut, pSI, *pTxt, nIdx, nLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
    nMaxSizeDiff = (USHORT)aDrawInf.GetKanaDiff();
    nMinSize = aSize.Width();
}

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
                                       const xub_StrLen nMaxLen ) const
{
    const SwScriptInfo& rSI =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    // in some cases, compression is not allowed or surpressed for
    // performance reasons
    USHORT nComp =( SW_CJK == GetFont()->GetActual() &&
                    rSI.CountCompChg() &&
                    ! IsMulti() ) ?
                    GetKanaComp() :
                                0 ;

    ASSERT( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" )
    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rSI, *pTxt, nIdx, nMaxLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( 0 );
    return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
}

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
                                       const xub_StrLen nMaxLen,
                                       const USHORT nComp ) const
{
    const SwScriptInfo& rScriptInfo =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    ASSERT( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" )
    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
                             *pTxt, GetIdx(), nMaxLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( 0 );

    return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
}

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
                                       const xub_StrLen nMaxLen,
                                       const USHORT nComp,
                                       xub_StrLen& rExtraCharPos ) const
{
    const SwScriptInfo& rScriptInfo =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    ASSERT( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" )
    SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
                             *pTxt, GetIdx(), nMaxLen );
    aDrawInf.SetFrm( pFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( SnapToGrid() );
    aDrawInf.SetKanaComp( nComp );
    aDrawInf.SetHyphPos( &rExtraCharPos );

    return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
}

/*************************************************************************
 *                     SwTxtPaintInfo::CtorInit()
 *************************************************************************/

void SwTxtPaintInfo::CtorInit( SwTxtFrm *pFrame, const SwRect &rPaint )
{
    SwTxtSizeInfo::CtorInit( pFrame );
    aTxtFly.CtorInit( pFrame ),
    aPaintRect = rPaint;
    nSpaceIdx = 0;
    pSpaceAdd = NULL;
    pWrongList = NULL;
#ifdef PRODUCT
    pBrushItem = 0;
#else
    pBrushItem = ((SvxBrushItem*)-1);
#endif
}

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf, const XubString &rTxt )
    : SwTxtSizeInfo( rInf, rTxt ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pWrongList( rInf.GetpWrongList() ),
      pBrushItem( rInf.GetBrushItem() )
{ }

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf )
    : SwTxtSizeInfo( rInf ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pWrongList( rInf.GetpWrongList() ),
      pBrushItem( rInf.GetBrushItem() )
{ }

extern Color aGlobalRetoucheColor;

/*************************************************************************
 *                          lcl_IsDarkBackground
 *
 * Returns if the current background color is dark.
 *************************************************************************/

sal_Bool lcl_IsDarkBackground( const SwTxtPaintInfo& rInf )
{
    const Color* pCol = rInf.GetFont()->GetBackColor();
    if( ! pCol || COL_TRANSPARENT == pCol->GetColor() )
    {
        const SvxBrushItem* pItem;
        SwRect aOrigBackRect;

        /// OD 21.08.2002
        ///     consider, that [GetBackgroundBrush(...)] can set <pCol>
        ///     - see implementation in /core/layout/paintfrm.cxx
        /// OD 21.08.2002 #99657#
        ///     There is a background color, if there is a background brush and
        ///     its color is *not* "no fill"/"auto fill".
        if( rInf.GetTxtFrm()->GetBackgroundBrush( pItem, pCol, aOrigBackRect, FALSE ) )
        {
            if ( !pCol )
                pCol = &pItem->GetColor();

            /// OD 30.08.2002 #99657#
            /// determined color <pCol> can be <COL_TRANSPARENT>. Thus, check it.
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
 *                     SwTxtPaintInfo::_DrawText()
 *************************************************************************/

void SwTxtPaintInfo::_DrawText( const XubString &rText, const SwLinePortion &rPor,
                            const xub_StrLen nStart, const xub_StrLen nLen,
                            const sal_Bool bKern, const sal_Bool bWrong )
{
    if( !nLen )
        return;
    if( GetFont()->IsBlink() && OnWin() && rPor.Width() )
    {
        // check if accessibility options allow blinking portions:
        const ViewShell* pSh = GetTxtFrm()->GetShell();
        if ( pSh && ! pSh->GetAccessibilityOptions()->IsStopAnimatedText() &&
             ! pSh->IsPreView() )
        {
            if( !pBlink )
                pBlink = new SwBlink();

            Point aPoint( aPos );

#ifdef BIDI
            if ( GetTxtFrm()->IsRightToLeft() )
                GetTxtFrm()->SwitchLTRtoRTL( aPoint );

            if ( TEXT_LAYOUT_BIDI_STRONG != GetOut()->GetLayoutMode() )
                aPoint.X() -= rPor.Width();
#endif

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

    short nSpaceAdd = ( rPor.IsBlankPortion() || rPor.IsDropPortion() ||
                        rPor.InNumberGrp() ) ? 0 : GetSpaceAdd();

#ifdef BIDI

    // The SwScriptInfo is useless if we are inside a field portion
    SwScriptInfo* pSI = 0;
    if ( ! rPor.InFldGrp() )
        pSI = &GetParaPortion()->GetScriptInfo();

    // in some cases, kana compression is not allowed or surpressed for
    // performance reasons
    USHORT nComp = 0;
    if ( ! IsMulti() )
        nComp = GetKanaComp();

    const sal_Bool bBullet = OnWin() && GetOpt().IsBlank() && IsNoSymbol();
    sal_Bool bTmpWrong = bWrong && OnWin() && GetOpt().IsOnlineSpell()
                             && !GetOpt().IsHideSpell();
    SwParaPortion* pPara = GetParaPortion();
    ASSERT( pPara, "No paragraph!");
    SwDrawTextInfo aDrawInf( pFrm->GetShell(), *pOut, pSI, rText, nStart, nLen,
                             rPor.Width(), bBullet );

#else

    const SwScriptInfo& rSI =
                     ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();

    // in some cases, compression is not allowed or surpressed for
    // performance reasons
    USHORT nComp =( SW_CJK == GetFont()->GetActual() &&
                    rSI.CountCompChg() &&
                    ! IsMulti() &&
                    ! rPor.InFldGrp() ) ?
                    GetKanaComp() :
                                0 ;

    const sal_Bool bBullet = OnWin() && GetOpt().IsBlank() && IsNoSymbol();
    sal_Bool bTmpWrong = bWrong && OnWin() && GetOpt().IsOnlineSpell()
                             && !GetOpt().IsHideSpell();
    SwParaPortion* pPara = GetParaPortion();
    ASSERT( pPara, "No paragraph!");
    SwDrawTextInfo aDrawInf( pFrm->GetShell(), *pOut, &pPara->GetScriptInfo(),
                             rText, nStart, nLen, rPor.Width(), bBullet );

#endif

    aDrawInf.SetLeft( GetPaintRect().Left() );
    aDrawInf.SetRight( GetPaintRect().Right() );
    aDrawInf.SetUnderFnt( pUnderFnt );
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
        // aPos muss als TopLeft vorliegen, weil die ClipRects sonst
        // nicht berechnet werden koennen.
        const Point aPoint( aPos.X(), aPos.Y() - rPor.GetAscent() );
        const Size aSize( rPor.Width(), rPor.Height() );
        aDrawInf.SetFont( pFnt );
        aDrawInf.SetPos( aPoint );
        aDrawInf.SetSize( aSize );
        aDrawInf.SetAscent( rPor.GetAscent() );
        aDrawInf.SetKern( bKern ? rPor.Width() : 0 );
        aDrawInf.SetSpace( nSpaceAdd );
        aDrawInf.SetKanaComp( nComp );
        aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
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
            aDrawInf.SetSpace( nSpaceAdd );
            aDrawInf.SetKanaComp( nComp );
            pFnt->_DrawText( aDrawInf );
        }
    }
}

/*************************************************************************
 *                          lcl_CalcRect()
 *************************************************************************/

void lcl_CalcRect( const SwTxtPaintInfo* pInf, const SwLinePortion& rPor,
                   SwRect* pRect, SwRect* pIntersect )
{
    Size aSize( rPor.Width(), rPor.Height() );
    if( rPor.IsHangingPortion() )
        aSize.Width() = ((SwHangingPortion&)rPor).GetInnerWidth();
    if( rPor.InSpaceGrp() && pInf->GetSpaceAdd() )
    {
        SwTwips nAdd = rPor.CalcSpacing( pInf->GetSpaceAdd(), *pInf );
        if( rPor.InFldGrp() && pInf->GetSpaceAdd() < 0 && nAdd )
            nAdd += pInf->GetSpaceAdd();
        aSize.Width() += nAdd;
    }

    Point aPoint;

    if( pInf->IsRotated() )
    {
        long nTmp = aSize.Width();
        aSize.Width() = aSize.Height();
        aSize.Height() = nTmp;
        if ( 1 == pInf->GetDirection() )
        {
            aPoint.A() = pInf->X() - rPor.GetAscent();
            aPoint.B() = pInf->Y() - aSize.Height();
        }
        else
        {
            aPoint.A() = pInf->X() - rPor.Height() + rPor.GetAscent();
            aPoint.B() = pInf->Y();
        }
    }
    else
    {
        aPoint.A() = pInf->X();
        aPoint.B() = pInf->Y() - rPor.GetAscent();
    }

#ifdef BIDI
    // Adjust x coordinate if we are inside a bidi portion
    const BOOL bFrmDir = pInf->GetTxtFrm()->IsRightToLeft();
    BOOL bCounterDir = ( ! bFrmDir && DIR_RIGHT2LEFT == pInf->GetDirection() ) ||
                       (   bFrmDir && DIR_LEFT2RIGHT == pInf->GetDirection() );

    if ( bCounterDir )
        aPoint.A() -= aSize.Width();
#endif

    SwRect aRect( aPoint, aSize );

#ifdef BIDI
    if ( pInf->GetTxtFrm()->IsRightToLeft() )
        pInf->GetTxtFrm()->SwitchLTRtoRTL( aRect );
#endif

    if ( pInf->GetTxtFrm()->IsVertical() )
        pInf->GetTxtFrm()->SwitchHorizontalToVertical( aRect );

    if ( pRect )
        *pRect = aRect;

    if( aRect.HasArea() && pIntersect )
    {
        ::SwAlignRect( aRect, (ViewShell*)pInf->GetVsh() );

        if ( pInf->GetOut()->IsClipRegion() )
        {
            SwRect aClip( pInf->GetOut()->GetClipRegion().GetBoundRect() );
            aRect.Intersection( aClip );
        }

        *pIntersect = aRect;
    }
}

/*************************************************************************
 *                          lcl_DrawSpecial
 *
 * Draws a special portion, e.g., line break portion, tab portion.
 * rPor - The portion
 * rRect - The rectangle surrounding the character
 * pCol     - Specify a color for the character
 * bCenter  - Draw the character centered, otherwise left aligned
 * bRotate  - Rotate the character if character rotation is set
 *************************************************************************/

#ifdef BIDI
void lcl_DrawSpecial( const SwTxtPaintInfo& rInf, const SwLinePortion& rPor,
                      SwRect& rRect, const Color* pCol, sal_Unicode cChar,
                      BYTE nOptions )
{
    sal_Bool bCenter = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_CENTER );
    sal_Bool bRotate = 0 != ( nOptions & DRAW_SPECIAL_OPTIONS_ROTATE );

    // rRect is given in absolute coordinates
    if ( rInf.GetTxtFrm()->IsRightToLeft() )
        rInf.GetTxtFrm()->SwitchRTLtoLTR( rRect );
#else
void lcl_DrawSpecial( const SwTxtPaintInfo& rInf, const SwLinePortion& rPor,
                      SwRect& rRect, const Color* pCol, sal_Unicode cChar,
                      sal_Bool bCenter, sal_Bool bRotate )
{
#endif
    if ( rInf.GetTxtFrm()->IsVertical() )
        rInf.GetTxtFrm()->SwitchVerticalToHorizontal( rRect );

    const SwFont* pOldFnt = rInf.GetFont();

    // Font is generated only once:
    static SwFont* pFnt = 0;
    if ( ! pFnt )
    {
        pFnt = new SwFont( *pOldFnt );
        pFnt->SetFamily( FAMILY_DONTKNOW, pFnt->GetActual() );
        pFnt->SetName( XubString::CreateFromAscii( sBulletFntName ), pFnt->GetActual() );
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
    const USHORT nDir = pFnt->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
    SwTwips nMaxWidth;
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
        ASSERT( sal_False, "Unknown direction set at font" )
        break;
    }

    // check if char fits into rectangle
    const XubString aTmp( cChar );
    aFontSize = rInf.GetTxtSize( aTmp ).SvLSize();
    while ( aFontSize.Width() > nMaxWidth )
    {
        SwTwips nFactor = ( 100 * aFontSize.Width() ) / nMaxWidth;
        const SwTwips nOldWidth = aFontSize.Width();

        // new height for font
        const BYTE nAct = pFnt->GetActual();
        aFontSize.Height() = ( 100 * pFnt->GetSize( nAct ).Height() ) / nFactor;
        aFontSize.Width() = ( 100 * pFnt->GetSize( nAct).Width() ) / nFactor;

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
#ifdef BIDI
    USHORT nOldWidth = rPor.Width();
    ((SwLinePortion&)rPor).Width( (USHORT)aFontSize.Width() );
    rInf.DrawText( aTmp, rPor );
    ((SwLinePortion&)rPor).Width( nOldWidth );
#else
    rInf.DrawText( aTmp, rPor );
#endif
    ((SwTxtPaintInfo&)rInf).SetFont( (SwFont*)pOldFnt );
    ((SwTxtPaintInfo&)rInf).SetPos( aOldPos );
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawRect()
 *************************************************************************/

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
            ASSERT( ((SvxBrushItem*)-1) != pBrushItem, "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( pBrushItem, pOut, aItemRect, rRect );
        }
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawTab()
 *************************************************************************/

void SwTxtPaintInfo::DrawTab( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        SwRect aRect;

        lcl_CalcRect( this, rPor, &aRect, 0 );

        if ( ! aRect.HasArea() )
            return;

#ifdef BIDI
        const sal_Unicode cChar = GetTxtFrm()->IsRightToLeft() ?
                                  CHAR_TAB_RTL : CHAR_TAB;
        const BYTE nOptions = DRAW_SPECIAL_OPTIONS_CENTER |
                              DRAW_SPECIAL_OPTIONS_ROTATE;
        lcl_DrawSpecial( *this, rPor, aRect, 0, cChar, nOptions );
#else
        lcl_DrawSpecial( *this, rPor, aRect, 0, CHAR_TAB, sal_True, sal_True );
#endif

    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawLineBreak()
 *************************************************************************/

void SwTxtPaintInfo::DrawLineBreak( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        KSHORT nOldWidth = rPor.Width();
        ((SwLinePortion&)rPor).Width( ((SwBreakPortion&)rPor).GetRestWidth() );

        SwRect aRect;

        lcl_CalcRect( this, rPor, &aRect, 0 );

        if( aRect.HasArea() )
        {
#ifdef BIDI
            const sal_Unicode cChar = GetTxtFrm()->IsRightToLeft() ?
                                      CHAR_LINEBREAK_RTL : CHAR_LINEBREAK;
            const BYTE nOptions = 0;
            lcl_DrawSpecial( *this, rPor, aRect, 0, cChar, nOptions );
#else
            lcl_DrawSpecial( *this, rPor, aRect, 0, CHAR_LINEBREAK,
                             sal_False, sal_False );
#endif
        }

        ((SwLinePortion&)rPor).Width( nOldWidth );
    }
}


/*************************************************************************
 *                     SwTxtPaintInfo::DrawRedArrow()
 *************************************************************************/

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
#ifdef BIDI
        const BYTE nOptions = 0;
        lcl_DrawSpecial( *this, rPor, aRect, &aCol, cChar, nOptions );
#else
        lcl_DrawSpecial( *this, rPor, aRect, &aCol, cChar, sal_False, sal_False );
#endif
    }
}


/*************************************************************************
 *                     SwTxtPaintInfo::DrawPostIts()
 *************************************************************************/

void SwTxtPaintInfo::DrawPostIts( const SwLinePortion &rPor, sal_Bool bScript ) const
{
    if( OnWin() && pOpt->IsPostIts() )
    {
        Size aSize;
        Point aTmp;

        const USHORT nPostItsWidth = pOpt->GetPostItsWidth( GetOut() );
        const USHORT nFontHeight = pFnt->GetHeight( pVsh, *GetOut() );
        const USHORT nFontAscent = pFnt->GetAscent( pVsh, *GetOut() );

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

#ifdef BIDI
        if ( GetTxtFrm()->IsRightToLeft() )
            GetTxtFrm()->SwitchLTRtoRTL( aTmpRect );
#endif

        if ( GetTxtFrm()->IsVertical() )
            GetTxtFrm()->SwitchHorizontalToVertical( aTmpRect );

        const Rectangle aRect( aTmpRect.SVRect() );
        pOpt->PaintPostIts( (OutputDevice*)GetOut(), aRect, bScript );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawBackGround()
 *************************************************************************/
void SwTxtPaintInfo::DrawBackground( const SwLinePortion &rPor ) const
{
    ASSERT( OnWin(), "SwTxtPaintInfo::DrawBackground: printer polution ?" );

    SwRect aIntersect;

    lcl_CalcRect( this, rPor, 0, &aIntersect );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pOut = (OutputDevice*)GetOut();
        sal_Bool bChgColor = sal_False;

        // For dark background we do not want to have a filled rectangle
        if ( GetVsh() && GetVsh()->GetWin() && lcl_IsDarkBackground( *this ) )
        {
            const StyleSettings& rS = GetVsh()->GetWin()->
                                      GetSettings().GetStyleSettings();

            Color aCol( SwViewOption::GetFontColor().GetColor() );
            Color aOldColor( pOut->GetLineColor() );

            if ( 0 != ( bChgColor = aOldColor != aCol ) )
                pOut->SetLineColor( aCol );

            DrawRect( aIntersect, sal_True );

            if ( bChgColor )
                pOut->SetLineColor( aOldColor );
        }
        else
        {
            Color aCol( SwViewOption::GetFieldShadingsColor() );
            Color aOldColor( pOut->GetFillColor() );

            if( 0 != ( bChgColor = aOldColor != aCol ) )
                pOut->SetFillColor( aCol );

            DrawRect( aIntersect, sal_True );

            if ( bChgColor )
                pOut->SetFillColor( aOldColor );
        }
    }
}

void SwTxtPaintInfo::_DrawBackBrush( const SwLinePortion &rPor ) const
{
    ASSERT( pFnt->GetBackColor(), "DrawBackBrush: Lost Color" );

    SwRect aIntersect;

    lcl_CalcRect( this, rPor, 0, &aIntersect );

    if ( aIntersect.HasArea() )
    {
        OutputDevice* pOut = (OutputDevice*)GetOut();
        const Color aOldColor( pOut->GetFillColor() );
        sal_Bool bChgColor;
        if( 0 != ( bChgColor = aOldColor != *pFnt->GetBackColor() ) )
            pOut->SetFillColor( *pFnt->GetBackColor() );
        DrawRect( aIntersect, sal_True, sal_False );
        if( bChgColor )
            pOut->SetFillColor( aOldColor );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawViewOpt()
 *************************************************************************/

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
            case POR_REF :
                if ( !GetOpt().IsPagePreview() &&
                            !GetOpt().IsReadonly() &&\
                            SwViewOption::IsFieldShadings() )
                    bDraw = sal_True;
            break;
            case POR_TAB:       if ( GetOpt().IsTab() )     bDraw = sal_True; break;
            case POR_SOFTHYPH:  if ( GetOpt().IsSoftHyph() )bDraw = sal_True; break;
            case POR_BLANK:     if ( GetOpt().IsHardBlank())bDraw = sal_True; break;
            default:
            {
                ASSERT( !this, "SwTxtPaintInfo::DrawViewOpt: don't know how to draw this" );
                break;
            }
        }
        if ( bDraw )
            DrawBackground( rPor );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::_NotifyURL()
 *************************************************************************/

void SwTxtPaintInfo::_NotifyURL( const SwLinePortion &rPor ) const
{
    ASSERT( pNoteURL, "NotifyURL: pNoteURL gone with the wind!" );

    SwRect aIntersect;

    lcl_CalcRect( this, rPor, 0, &aIntersect );

    if( aIntersect.HasArea() )
    {
        SwTxtNode *pNd = (SwTxtNode*)GetTxtFrm()->GetTxtNode();
        SwIndex aIndex( pNd, GetIdx() );
        SwTxtAttr *pAttr = pNd->GetTxtAttr( aIndex, RES_TXTATR_INETFMT );
        if( pAttr )
        {
            const SwFmtINetFmt& rFmt = pAttr->GetINetFmt();
            pNoteURL->InsertURLNote( rFmt.GetValue(), rFmt.GetTargetFrame(),
                aIntersect );
        }
    }
}

/*************************************************************************
 *                  lcl_InitHyphValues()
 *************************************************************************/

static void lcl_InitHyphValues( PropertyValues &rVals,
            INT16 nMinLeading, INT16 nMinTrailing )
{
    INT32 nLen = rVals.getLength();

    if (0 == nLen)  // yet to be initialized?
    {
        rVals.realloc( 2 );
        PropertyValue *pVal = rVals.getArray();

        pVal[0].Name    = C2U( UPN_HYPH_MIN_LEADING );
        pVal[0].Handle  = UPH_HYPH_MIN_LEADING;
        pVal[0].Value   <<= nMinLeading;

        pVal[1].Name    = C2U( UPN_HYPH_MIN_TRAILING );
        pVal[1].Handle  = UPH_HYPH_MIN_TRAILING;
        pVal[1].Value   <<= nMinTrailing;
    }
    else if (2 == nLen) // already initialized once?
    {
        PropertyValue *pVal = rVals.getArray();
        pVal[0].Value <<= nMinLeading;
        pVal[1].Value <<= nMinTrailing;
    }
    else
        DBG_ERROR( "unxpected size of sequence" );
}

/*************************************************************************
 *                  SwTxtFormatInfo::GetHyphValues()
 *************************************************************************/

const PropertyValues & SwTxtFormatInfo::GetHyphValues() const
{
    DBG_ASSERT( 2 == aHyphVals.getLength(),
            "hyphenation values not yet initialized" );
    return aHyphVals;
}

/*************************************************************************
 *                  SwTxtFormatInfo::InitHyph()
 *************************************************************************/

sal_Bool SwTxtFormatInfo::InitHyph( const sal_Bool bAutoHyph )
{
    const SwAttrSet& rAttrSet = GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
    SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
    SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
    SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
    const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
    MaxHyph() = rAttr.GetMaxHyphens();
    sal_Bool bAuto = bAutoHyph || rAttr.IsHyphen();
    if( bAuto || bInterHyph )
    {
        nHyphStart = nHyphWrdStart = STRING_LEN;
        nHyphWrdLen = 0;

        INT16 nMinLeading  = Max(rAttr.GetMinLead(), sal_uInt8(2));
        INT16 nMinTrailing = rAttr.GetMinTrail();
        lcl_InitHyphValues( aHyphVals, nMinLeading, nMinTrailing);
    }
    return bAuto;
}

/*************************************************************************
 *                  SwTxtFormatInfo::CtorInit()
 *************************************************************************/

void SwTxtFormatInfo::CtorInit( SwTxtFrm *pNewFrm, const sal_Bool bNewInterHyph,
                                const sal_Bool bNewQuick, const sal_Bool bTst )
{
    SwTxtPaintInfo::CtorInit( pNewFrm, SwRect() );

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
 *                  SwTxtFormatInfo::IsHyphenate()
 *************************************************************************/
// Trennen oder nicht trennen, das ist hier die Frage:
// - in keinem Fall trennen, wenn der Hyphenator ERROR zurueckliefert,
//   oder wenn als Sprache NOLANGUAGE eingestellt ist.
// - ansonsten immer trennen, wenn interaktive Trennung vorliegt
// - wenn keine interakt. Trennung, dann nur trennen, wenn im ParaFmt
//   automatische Trennung eingestellt ist.

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
 *                  SwTxtFormatInfo::GetDropFmt()
 *************************************************************************/

// Dropcaps vom SwTxtFormatter::CTOR gerufen.
const SwFmtDrop *SwTxtFormatInfo::GetDropFmt() const
{
    const SwFmtDrop *pDrop = &GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetDrop();
    if( 1 >= pDrop->GetLines() ||
        ( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
        pDrop = 0;
    return pDrop;
}

/*************************************************************************
 *                      SwTxtFormatInfo::Init()
 *************************************************************************/

void SwTxtFormatInfo::Init()
{
    // Nicht initialisieren: pRest, nLeft, nRight, nFirst, nRealWidth
    X(0);
    bArrowDone = bFull = bFtnDone = bErgoDone = bNumDone = bNoEndHyph =
        bNoMidHyph = bStop = bNewLine = bUnderFlow = sal_False;

    // generally we do not allow number portions in follows, except...
    if ( GetTxtFrm()->IsFollow() )
    {
        const SwTxtFrm* pMaster = GetTxtFrm()->FindMaster();
        const SwLinePortion* pPara = pMaster->GetPara();

        // there is a master for this follow and the master does not have
        // any contents (especially it does not have a number portion)
        bNumDone = ! pPara ||
                   ! ((SwParaPortion*)pPara)->GetFirstPortion()->IsFlyPortion();
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

/*-----------------16.10.00 11:39-------------------
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 * --------------------------------------------------*/

SwTxtFormatInfo::SwTxtFormatInfo( const SwTxtFormatInfo& rInf,
    SwLineLayout& rLay, SwTwips nActWidth ) : SwTxtPaintInfo( rInf )
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
    bFull = FALSE;
    bFtnDone = TRUE;
    bErgoDone = TRUE;
    bNumDone = TRUE;
    bStop = FALSE;
    bNewLine = TRUE;
    bShift  = FALSE;
    bUnderFlow = FALSE;
    bInterHyph = FALSE;
    bAutoHyph = FALSE;
    bDropInit = FALSE;
    bQuick  = rInf.bQuick;
    bNoEndHyph  = FALSE;
    bNoMidHyph  = FALSE;
    bIgnoreFly = FALSE;
    bFakeLineStart = FALSE;

    cTabDecimal = 0;
    cHookChar = 0;
    nMaxHyph = 0;
    bTestFormat = rInf.bTestFormat;
    SetMulti( sal_True );
    SetFirstMulti( rInf.IsFirstMulti() );
}

/*************************************************************************
 *                 SwTxtFormatInfo::_CheckFtnPortion()
 *************************************************************************/

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




/*************************************************************************
 *                 SwTxtFormatInfo::ScanPortionEnd()
 *************************************************************************/
xub_StrLen SwTxtFormatInfo::ScanPortionEnd( const xub_StrLen nStart,
                                            const xub_StrLen nEnd )
{
    cHookChar = 0;
    const xub_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
    xub_StrLen i = nStart;

    // Removed for i7288. bSkip used to be passed from SwFldPortion::Format
    // as IsFollow(). Therefore more than one special character was not
    // handled correctly at the beginning of follow fields.
//    if ( bSkip && i < nEnd )
//       ++i;

    for( ; i < nEnd; ++i )
    {
        const xub_Unicode cPos = GetChar( i );
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
            cHookChar = cPos;
            return i;

        case CHAR_UNDERSCORE:
            if ( STRING_LEN == nUnderScorePos )
                nUnderScorePos = i;
            break;

        default:
            if( cTabDec == cPos )
            {
                ASSERT( cPos, "Unexspected end of string" );
                if( cPos ) // robust
                {
                    cHookChar = cPos;
                    return i;
                }
            }
        }
    }
    return i;
}

BOOL SwTxtFormatInfo::LastKernPortion()
{
    if( GetLast() )
    {
         if( GetLast()->IsKernPortion() )
            return TRUE;
        if( GetLast()->Width() || ( GetLast()->GetLen() &&
            !GetLast()->IsHolePortion() ) )
            return FALSE;
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
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
 *                      class SwTxtSlot
 *************************************************************************/

SwTxtSlot::SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor )
{
    bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pInf->SetLen( pPor->GetLen() );
        pOldTxt = &(pInf->GetTxt());
        pInf->SetTxt( aTxt );
        pInf->SetIdx( 0 );
    }
}

/*************************************************************************
 *                       SwTxtSlot::~SwTxtSlot()
 *************************************************************************/

SwTxtSlot::~SwTxtSlot()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
    }
}

/*************************************************************************
 *                      class SwTxtSlotLen
 *************************************************************************/

SwTxtSlotLen::SwTxtSlotLen( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
    const sal_Char *pCh )
{
    if( pCh )
    {
        aTxt = XubString( pCh, RTL_TEXTENCODING_MS_1252 );
        bOn = sal_True;
    }
    else
        bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetTxt( aTxt );
        pInf->SetIdx( 0 );
        pInf->SetLen( pInf->GetTxt().Len() );
    }
}

/*************************************************************************
 *                       SwTxtSlotLen::~SwTxtSlotLen()
 *************************************************************************/

SwTxtSlotLen::~SwTxtSlotLen()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
    }
}

/*************************************************************************
 *                     SwFontSave::SwFontSave()
 *************************************************************************/

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

/*************************************************************************
 *                     SwFontSave::~SwFontSave()
 *************************************************************************/

SwFontSave::~SwFontSave()
{
    if( pFnt )
    {
        // SwFont zurueckstellen
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
        if( pIter )
        {
            pIter->SetFnt( pFnt );
            pIter->nPos = STRING_LEN;
        }
    }
}

/*************************************************************************
 *                     SwDefFontSave::SwDefFontSave()
 *************************************************************************/

SwDefFontSave::SwDefFontSave( const SwTxtSizeInfo &rInf )
        : pFnt( ((SwTxtSizeInfo&)rInf).GetFont()  )
{
    BOOL bAlter = pFnt->GetFixKerning() ||
         ( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet(pFnt->GetActual()) )
        ;

    const sal_Bool bFamily = bAlter && COMPARE_EQUAL !=
            pFnt->GetName( pFnt->GetActual() ).CompareToAscii( sBulletFntName );
    const sal_Bool bRotation = (sal_Bool)pFnt->GetOrientation() &&
                                ! rInf.GetTxtFrm()->IsVertical();

    if( bFamily || bRotation )
    {
        pNewFnt = new SwFont( *pFnt );

        if ( bFamily )
        {
            pNewFnt->SetFamily( FAMILY_DONTKNOW, pFnt->GetActual() );
            pNewFnt->SetName( XubString::CreateFromAscii( sBulletFntName ),
                              pFnt->GetActual() );
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

/*************************************************************************
 *                     SwDefFontSave::~SwDefFontSave()
 *************************************************************************/

SwDefFontSave::~SwDefFontSave()
{
    if( pFnt )
    {
        delete pNewFnt;
        // SwFont zurueckstellen
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
    }
}

/*************************************************************************
 *                  SwTxtFormatInfo::ChgHyph()
 *************************************************************************/

sal_Bool SwTxtFormatInfo::ChgHyph( const sal_Bool bNew )
{
    const sal_Bool bOld = bAutoHyph;
    if( bAutoHyph != bNew )
    {
        bAutoHyph = bNew;
        InitHyph( bNew );
        // 5744: Sprache am Hyphenator einstellen.
        if( pFnt )
            pFnt->ChgPhysFnt( pVsh, *pOut );
    }
    return bOld;
}


