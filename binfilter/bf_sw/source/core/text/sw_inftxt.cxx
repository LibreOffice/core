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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <com/sun/star/uno/Sequence.h>

#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <bf_svtools/linguprops.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <bf_svtools/ctloptions.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <bf_svx/hyznitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <bf_svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_SRIPTSPACEITEM_HXX
#include <bf_svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <bf_svx/pgrditem.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <bf_svx/forbiddenruleitem.hxx>
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
#include <viewsh.hxx>	// ViewShell
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>		// SwDoc
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>	// SwFmtDrop
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>	// SwTxtInfo
#endif
#ifndef _NOTEURL_HXX
#include <noteurl.hxx>	// SwNoteURL
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>	// SwFtnPortion
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _ITRATR_HXX
#include <itratr.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)
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

#ifdef DBG_UTIL
// Test2: WYSIWYG++
// Test4: WYSIWYG debug
static sal_Bool bDbgLow = sal_False;
#endif

#ifdef DBG_UTIL


#endif

/*************************************************************************
 *						SwLineInfo::SwLineInfo()
 *************************************************************************/

/*N*/ void SwLineInfo::CtorInit( const SwAttrSet& rAttrSet )
/*N*/ {
/*N*/ 	pRuler = &rAttrSet.GetTabStops();
/*N*/ 	pSpace = &rAttrSet.GetLineSpacing();
/*N*/     nVertAlign = rAttrSet.GetParaVertAlign().GetValue();
/*N*/     nDefTabStop = MSHRT_MAX;
/*N*/ }

/*************************************************************************
 *						SwTxtInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtInfo::CtorInit( SwTxtFrm *pFrm )
/*N*/ {
/*N*/ 	pPara = pFrm->GetPara();
/*N*/ 	nTxtStart = pFrm->GetOfst();
/*N*/ 	if( !pPara )
/*N*/ 	{
/*?*/ 		ASSERT( pPara, "+SwTxtInfo::CTOR: missing paragraph information" );
/*?*/ 		pFrm->Format();
/*?*/ 		pPara = pFrm->GetPara();
/*N*/ 	}
/*N*/ }

/*N*/ SwTxtInfo::SwTxtInfo( const SwTxtInfo &rInf )
/*N*/ 	: pPara( ((SwTxtInfo&)rInf).GetParaPortion() ),
/*N*/ 	  nTxtStart( rInf.GetTxtStart() )
/*N*/ { }


#ifdef DBG_UTIL
/*************************************************************************
 *						ChkOutDev()
 *************************************************************************/

/*N*/ void ChkOutDev( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/     if ( !rInf.GetVsh() )
/*N*/         return;
/*N*/ 
/*N*/     const OutputDevice *pOut = rInf.GetOut();
/*N*/     const OutputDevice *pWin = rInf.GetVsh()->GetWin();
/*N*/     const OutputDevice *pRef = rInf.GetRefDev();
/*N*/     ASSERT( pOut && pRef, "ChkOutDev: invalid output devices" )
/*N*/ }
/*N*/ #endif	// PRODUCT


/*N*/ inline xub_StrLen GetMinLen( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/ 	const xub_StrLen nInfLen = rInf.GetIdx() + rInf.GetLen();
/*N*/ 	return Min( rInf.GetTxt().Len(), nInfLen );
/*N*/ }


/*N*/ SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew )
/*N*/ 	: SwTxtInfo( rNew ),
/*N*/       pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
/*N*/       pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
/*N*/ 	  pOut(((SwTxtSizeInfo&)rNew).GetOut()),
/*N*/       pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
/*N*/ 	  pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
/*N*/       pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
/*N*/ 	  pFrm(rNew.pFrm),
/*N*/ 	  pOpt(&rNew.GetOpt()),
/*N*/ 	  pTxt(&rNew.GetTxt()),
/*N*/ 	  nIdx(rNew.GetIdx()),
/*N*/ 	  nLen(rNew.GetLen()),
/*N*/       nKanaIdx( rNew.GetKanaIdx() ),
/*N*/ 	  bOnWin( rNew.OnWin() ),
/*N*/ 	  bNotEOL( rNew.NotEOL() ),
/*N*/ 	  bURLNotify( rNew.URLNotify() ),
/*N*/ 	  bStopUnderFlow( rNew.StopUnderFlow() ),
/*N*/       bFtnInside( rNew.IsFtnInside() ),
/*N*/ 	  bMulti( rNew.IsMulti() ),
/*N*/ 	  bFirstMulti( rNew.IsFirstMulti() ),
/*N*/ 	  bRuby( rNew.IsRuby() ),
/*N*/ 	  bHanging( rNew.IsHanging() ),
/*N*/ 	  bScriptSpace( rNew.HasScriptSpace() ),
/*N*/ 	  bForbiddenChars( rNew.HasForbiddenChars() ),
/*N*/       bSnapToGrid( rNew.SnapToGrid() ),
/*N*/ 	  nDirection( rNew.GetDirection() )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ChkOutDev( *this );
/*N*/ #endif
/*N*/ }

/*N*/ void SwTxtSizeInfo::CtorInit( SwTxtFrm *pFrame, SwFont *pNewFnt,
/*N*/ 				   const xub_StrLen nNewIdx, const xub_StrLen nNewLen )
/*N*/ {
/*N*/     pKanaComp = NULL;
/*N*/     nKanaIdx = 0;
/*N*/ 	pFrm = pFrame;
/*N*/ 	SwTxtInfo::CtorInit( pFrm );
/*N*/ 	const SwTxtNode *pNd = pFrm->GetTxtNode();
/*N*/ 	pVsh = pFrm->GetShell();
/*N*/ 
/*N*/     // Get the output and reference device
/*N*/     if ( pVsh )
/*N*/     {
/*N*/         pOut = pVsh->GetOut();
/*N*/         pRef = &pVsh->GetRefDev();
/*N*/         bOnWin = pVsh->GetWin() || OUTDEV_WINDOW == pOut->GetOutDevType();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         //Zugriff ueber StarONE, es muss keine Shell existieren oder aktiv sein.
/*N*/         if ( pNd->GetDoc()->IsBrowseMode() ) //?!?!?!?
/*N*/             //in Ermangelung eines Besseren kann hier ja wohl nur noch das
/*N*/             //AppWin genommen werden?
/*N*/             pOut = GetpApp()->GetDefaultDevice();
/*N*/         else
/*N*/             pOut = pNd->GetDoc()->GetPrt(); //Muss es geben (oder sal_True uebergeben?)
/*N*/         pRef = pOut;
/*N*/     }
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/     ChkOutDev( *this );
/*N*/ #endif
/*N*/ 
/*N*/     // Set default layout mode ( LTR or RTL ).
/*N*/     if ( pFrm->IsRightToLeft() )
/*N*/     {
/*N*/         pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
/*N*/         pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
/*N*/         nDirection = DIR_RIGHT2LEFT;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
/*N*/         pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
/*N*/         nDirection = DIR_LEFT2RIGHT;
/*N*/     }
/*N*/ 
/*N*/     LanguageType eLang;
/*N*/     const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
/*N*/     if ( SvtCTLOptions::NUMERALS_HINDI == rCTLOptions.GetCTLTextNumerals() )
/*N*/         eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
/*N*/     else if ( SvtCTLOptions::NUMERALS_ARABIC == rCTLOptions.GetCTLTextNumerals() )
/*N*/         eLang = LANGUAGE_ENGLISH;
/*N*/     else
/*N*/         eLang = (LanguageType)::binfilter::GetAppLanguage();
/*N*/ 
/*N*/     pOut->SetDigitLanguage( eLang );
/*N*/     pRef->SetDigitLanguage( eLang );
/*N*/ 
/*N*/     //
/*N*/     // The Options
/*N*/     //
/*N*/     pOpt = pVsh ?
/*N*/            pVsh->GetViewOptions() :
/*N*/            SW_MOD()->GetViewOption(pNd->GetDoc()->IsHTMLMode()); //Options vom Module wg. StarONE
/*N*/ 
/*N*/     // bURLNotify wird gesetzt, wenn MakeGraphic dies vorbereitet
/*N*/     // TODO: Aufdröseln
/*N*/     bURLNotify = pNoteURL && !bOnWin;
/*N*/ //    bURLNotify = pNoteURL && !bOnWin
/*N*/ //        && (pOut && OUTDEV_PRINTER != pOut->GetOutDevType());
/*N*/ 
/*N*/     SetSnapToGrid( pNd->GetSwAttrSet().GetParaGrid().GetValue() &&
/*N*/                    pFrm->IsInDocBody() );
/*N*/ 
/*N*/     pFnt = pNewFnt;
/*N*/     pUnderFnt = 0;
/*N*/     pTxt = &pNd->GetTxt();
/*N*/ 
/*N*/ 	nIdx = nNewIdx;
/*N*/ 	nLen = nNewLen;
/*N*/ 	bNotEOL = sal_False;
/*N*/     bStopUnderFlow = bFtnInside = sal_False;
/*N*/ 	bMulti = bFirstMulti = bRuby = bHanging = bScriptSpace =
/*N*/ 		bForbiddenChars = sal_False;
/*N*/ #ifndef BIDI
/*N*/     nDirection = DIR_LEFT2RIGHT;
/*N*/ #endif
/*N*/ 
/*N*/ 	SetLen( GetMinLen( *this ) );
/*N*/ }

/*N*/ SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew, const XubString &rTxt,
/*N*/ 							  const xub_StrLen nIdx, const xub_StrLen nLen )
/*N*/ 	: SwTxtInfo( rNew ),
/*N*/       pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
/*N*/ 	  pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
/*N*/ 	  pOut(((SwTxtSizeInfo&)rNew).GetOut()),
/*N*/       pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
/*N*/ 	  pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
/*N*/       pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
/*N*/ 	  pFrm( rNew.pFrm ),
/*N*/ 	  pOpt(&rNew.GetOpt()),
/*N*/ 	  pTxt(&rTxt),
/*N*/ 	  nIdx(nIdx),
/*N*/ 	  nLen(nLen),
/*N*/       nKanaIdx( rNew.GetKanaIdx() ),
/*N*/ 	  bOnWin( rNew.OnWin() ),
/*N*/ 	  bNotEOL( rNew.NotEOL() ),
/*N*/ 	  bURLNotify( rNew.URLNotify() ),
/*N*/ 	  bStopUnderFlow( rNew.StopUnderFlow() ),
/*N*/       bFtnInside( rNew.IsFtnInside() ),
/*N*/ 	  bMulti( rNew.IsMulti() ),
/*N*/ 	  bFirstMulti( rNew.IsFirstMulti() ),
/*N*/ 	  bRuby( rNew.IsRuby() ),
/*N*/ 	  bHanging( rNew.IsHanging() ),
/*N*/ 	  bScriptSpace( rNew.HasScriptSpace() ),
/*N*/ 	  bForbiddenChars( rNew.HasForbiddenChars() ),
/*N*/       bSnapToGrid( rNew.SnapToGrid() ),
/*N*/ 	  nDirection( rNew.GetDirection() )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ChkOutDev( *this );
/*N*/ #endif
/*N*/ 	SetLen( GetMinLen( *this ) );
/*N*/ }

/*************************************************************************
 *						SwTxtSizeInfo::SelectFont()
 *************************************************************************/

/*N*/ void SwTxtSizeInfo::SelectFont()
/*N*/ {
/*N*/ 	// 8731: Der Weg muss ueber ChgPhysFnt gehen, sonst geraet
/*N*/ 	// der FontMetricCache durcheinander. In diesem Fall steht pLastMet
/*N*/ 	// auf dem alten Wert.
/*N*/ 	// Falsch: GetOut()->SetFont( GetFont()->GetFnt() );
/*N*/ 	GetFont()->Invalidate();
/*N*/ 	GetFont()->ChgPhysFnt( pVsh, GetOut() );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::NoteAnimation()
 *************************************************************************/


/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwTxtSizeInfo::GetTxtSize( OutputDevice* pOutDev,
/*N*/                                      const SwScriptInfo* pSI,
/*N*/                                      const XubString& rTxt,
/*N*/ 									 const xub_StrLen nIdx,
/*N*/                                      const xub_StrLen nLen,
/*N*/                                      const USHORT nComp ) const
/*N*/ {
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOutDev, pSI, rTxt, nIdx, nLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
/*N*/     return aSize;
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwTxtSizeInfo::GetTxtSize() const
/*N*/ {
/*N*/     const SwScriptInfo& rSI =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/ 
/*N*/     // in some cases, compression is not allowed or surpressed for
/*N*/     // performance reasons
/*N*/     USHORT nComp =( SW_CJK == GetFont()->GetActual() &&
/*N*/                     rSI.CountCompChg() &&
/*N*/                     ! IsMulti() ) ?
/*N*/                     GetKanaComp() :
/*N*/                                 0 ;
/*N*/ 
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rSI, *pTxt, nIdx, nLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     return pFnt->_GetTxtSize( aDrawInf );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ void SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
/*N*/                                 const xub_StrLen nLen, const USHORT nComp,
/*N*/                                 USHORT& nMinSize, USHORT& nMaxSizeDiff ) const
/*N*/ {
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, pSI, *pTxt, nIdx, nLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
/*N*/     nMaxSizeDiff = (USHORT)aDrawInf.GetKanaDiff();
/*N*/     nMinSize = aSize.Width();
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/


/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

/*N*/ xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
/*N*/                                        const xub_StrLen nMaxLen,
/*N*/                                        const USHORT nComp ) const
/*N*/ {
/*N*/     const SwScriptInfo& rScriptInfo =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/ 
/*N*/     ASSERT( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" )
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
/*N*/                              *pTxt, GetIdx(), nMaxLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     aDrawInf.SetHyphPos( 0 );
/*N*/ 
/*N*/     return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

/*N*/ xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
/*N*/                                        const xub_StrLen nMaxLen,
/*N*/                                        const USHORT nComp,
/*N*/                                        xub_StrLen& rExtraCharPos ) const
/*N*/ {
/*N*/     const SwScriptInfo& rScriptInfo =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/ 
/*N*/     ASSERT( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" )
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
/*N*/                              *pTxt, GetIdx(), nMaxLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     aDrawInf.SetHyphPos( &rExtraCharPos );
/*N*/ 
/*N*/     return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
/*N*/ }

/*************************************************************************
 *					   SwTxtPaintInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtPaintInfo::CtorInit( SwTxtFrm *pFrame, const SwRect &rPaint )
/*N*/ {
/*N*/ 	SwTxtSizeInfo::CtorInit( pFrame );
/*N*/ 	aTxtFly.CtorInit( pFrame ),
/*N*/ 	aPaintRect = rPaint;
/*N*/ 	nSpaceIdx = 0;
/*N*/ 	pSpaceAdd = NULL;
/*N*/ 	pWrongList = NULL;
/*N*/ #ifndef DBG_UTIL
/*N*/ 	pBrushItem = 0;
/*N*/ #else
/*N*/ 	pBrushItem = ((SvxBrushItem*)-1);
/*N*/ #endif
/*N*/ }


/*N*/ SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf )
/*N*/ 	: SwTxtSizeInfo( rInf ),
/*N*/ 	  aTxtFly( *rInf.GetTxtFly() ),
/*N*/ 	  aPos( rInf.GetPos() ),
/*N*/ 	  aPaintRect( rInf.GetPaintRect() ),
/*N*/ 	  nSpaceIdx( rInf.GetSpaceIdx() ),
/*N*/ 	  pSpaceAdd( rInf.GetpSpaceAdd() ),
/*N*/ 	  pWrongList( rInf.GetpWrongList() ),
/*N*/       pBrushItem( rInf.GetBrushItem() )
/*N*/ { }

extern Color aGlobalRetoucheColor;

/*************************************************************************
 *                          lcl_IsDarkBackground
 *
 * Returns if the current background color is dark.
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::_DrawText()
 *************************************************************************/


/*************************************************************************
 *							lcl_CalcRect()
 *************************************************************************/


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


/*************************************************************************
 *					   SwTxtPaintInfo::DrawRect()
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::DrawTab()
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::DrawLineBreak()
 *************************************************************************/



/*************************************************************************
 *                     SwTxtPaintInfo::DrawRedArrow()
 *************************************************************************/



/*************************************************************************
 *					   SwTxtPaintInfo::DrawPostIts()
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::DrawBackGround()
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::DrawViewOpt()
 *************************************************************************/


/*************************************************************************
 *					   SwTxtPaintInfo::_NotifyURL()
 *************************************************************************/


/*************************************************************************
 *					lcl_InitHyphValues()
 *************************************************************************/

/*N*/ static void lcl_InitHyphValues( PropertyValues &rVals,
/*N*/ 			INT16 nMinLeading, INT16 nMinTrailing )
/*N*/ {
/*N*/ 	INT32 nLen = rVals.getLength();
/*N*/ 
/*N*/ 	if (0 == nLen)	// yet to be initialized?
/*N*/ 	{
/*N*/ 		rVals.realloc( 2 );
/*N*/ 		PropertyValue *pVal = rVals.getArray();
/*N*/ 
/*N*/ 		pVal[0].Name	= C2U( UPN_HYPH_MIN_LEADING );
/*N*/ 		pVal[0].Handle	= UPH_HYPH_MIN_LEADING;
/*N*/ 		pVal[0].Value	<<= nMinLeading;
/*N*/ 
/*N*/ 		pVal[1].Name	= C2U( UPN_HYPH_MIN_TRAILING );
/*N*/ 		pVal[1].Handle	= UPH_HYPH_MIN_TRAILING;
/*N*/ 		pVal[1].Value	<<= nMinTrailing;
/*N*/ 	}
/*N*/ 	else if (2 == nLen)	// already initialized once?
/*N*/ 	{
/*?*/ 		PropertyValue *pVal = rVals.getArray();
/*?*/ 		pVal[0].Value <<= nMinLeading;
/*?*/ 		pVal[1].Value <<= nMinTrailing;
/*?*/ 	}
/*?*/ 	else
/*?*/ 		DBG_ERROR( "unxpected size of sequence" );
/*N*/ }

/*************************************************************************
 *					SwTxtFormatInfo::GetHyphValues()
 *************************************************************************/
/*N*/ 
/*N*/ const PropertyValues & SwTxtFormatInfo::GetHyphValues() const
/*N*/ {
/*N*/ 	DBG_ASSERT( 2 == aHyphVals.getLength(),
/*N*/ 			"hyphenation values not yet initialized" );
/*N*/ 	return aHyphVals;
/*N*/ }

/*************************************************************************
 *					SwTxtFormatInfo::InitHyph()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::InitHyph( const sal_Bool bAutoHyph )
/*N*/ {
/*N*/ 	const SwAttrSet& rAttrSet = GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
/*N*/ 	SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
/*N*/ 	SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
/*N*/ 	SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
/*N*/ 	const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
/*N*/ 	MaxHyph() = rAttr.GetMaxHyphens();
/*N*/ 	sal_Bool bAuto = bAutoHyph || rAttr.IsHyphen();
/*N*/ 	if( bAuto || bInterHyph )
/*N*/ 	{
/*N*/ 		nHyphStart = nHyphWrdStart = STRING_LEN;
/*N*/ 		nHyphWrdLen = 0;
/*N*/ 
/*N*/ 		INT16 nMinLeading  = Max(rAttr.GetMinLead(), sal_uInt8(2));
/*N*/ 		INT16 nMinTrailing = rAttr.GetMinTrail();
/*N*/ 		lcl_InitHyphValues( aHyphVals, nMinLeading, nMinTrailing);
/*N*/ 	}
/*N*/ 	return bAuto;
/*N*/ }

/*************************************************************************
 *					SwTxtFormatInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtFormatInfo::CtorInit( SwTxtFrm *pNewFrm, const sal_Bool bNewInterHyph,
/*N*/ 								const sal_Bool bNewQuick, const sal_Bool bTst )
/*N*/ {
/*N*/ 	SwTxtPaintInfo::CtorInit( pNewFrm, SwRect() );
/*N*/ 
/*N*/ 	bQuick = bNewQuick;
/*N*/ 	bInterHyph = bNewInterHyph;
/*N*/ 
/*N*/ 	//! needs to be done in this order
/*N*/ 	nMinLeading		= 2;
/*N*/ 	nMinTrailing	= 2;
/*N*/ 	nMinWordLength	= 0;
/*N*/ 	bAutoHyph = InitHyph();
/*N*/ 
/*N*/ 	bIgnoreFly = sal_False;
/*N*/     bFakeLineStart = sal_False;
/*N*/ 	bShift = sal_False;
/*N*/ 	bDropInit = sal_False;
/*N*/ 	bTestFormat = bTst;
/*N*/ 	nLeft = 0;
/*N*/ 	nRight = 0;
/*N*/ 	nFirst = 0;
/*N*/ 	nRealWidth = 0;
/*N*/ 	nForcedLeftMargin = 0;
/*N*/ 	pRest = 0;
/*N*/ 	nLineHeight = 0;
/*N*/     nLineNettoHeight = 0;
/*N*/ 	SetLineStart(0);
/*N*/ 	Init();
/*N*/ }

/*************************************************************************
 *					SwTxtFormatInfo::IsHyphenate()
 *************************************************************************/
// Trennen oder nicht trennen, das ist hier die Frage:
// - in keinem Fall trennen, wenn der Hyphenator ERROR zurueckliefert,
//	 oder wenn als Sprache NOLANGUAGE eingestellt ist.
// - ansonsten immer trennen, wenn interaktive Trennung vorliegt
// - wenn keine interakt. Trennung, dann nur trennen, wenn im ParaFmt
//	 automatische Trennung eingestellt ist.

/*N*/ sal_Bool SwTxtFormatInfo::IsHyphenate() const
/*N*/ {
/*N*/ 	if( !bInterHyph && !bAutoHyph )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	LanguageType eTmp = GetFont()->GetLanguage();
/*N*/ 	if( LANGUAGE_DONTKNOW == eTmp || LANGUAGE_NONE == eTmp )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	uno::Reference< XHyphenator > xHyph = ::binfilter::GetHyphenator();
/*N*/ 	if (bInterHyph && xHyph.is())
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SvxSpellWrapper::CheckHyphLang( xHyph, eTmp );
/*N*/ 
/*N*/ 	if( !xHyph.is() || !xHyph->hasLocale( pBreakIt->GetLocale(eTmp) ) )
/*N*/ 		return sal_False;
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *					SwTxtFormatInfo::GetDropFmt()
 *************************************************************************/

// Dropcaps vom SwTxtFormatter::CTOR gerufen.
/*N*/ const SwFmtDrop *SwTxtFormatInfo::GetDropFmt() const
/*N*/ {
/*N*/ 	const SwFmtDrop *pDrop = &GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetDrop();
/*N*/ 	if( 1 >= pDrop->GetLines() ||
/*N*/ 		( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
/*N*/ 		pDrop = 0;
/*N*/ 	return pDrop;
/*N*/ }

/*************************************************************************
 *						SwTxtFormatInfo::Init()
 *************************************************************************/

/*N*/ void SwTxtFormatInfo::Init()
/*N*/ {
/*N*/ 	// Nicht initialisieren: pRest, nLeft, nRight, nFirst, nRealWidth
/*N*/ 	X(0);
/*N*/ 	bArrowDone = bFull = bFtnDone = bErgoDone = bNumDone = bNoEndHyph =
/*N*/ 		bNoMidHyph = bStop = bNewLine = bUnderFlow = sal_False;
/*N*/ 
/*N*/     // generally we do not allow number portions in follows, except...
/*N*/     if ( GetTxtFrm()->IsFollow() )
/*N*/     {
/*N*/         const SwTxtFrm* pMaster = GetTxtFrm()->FindMaster();
/*N*/         const SwLinePortion* pPara = pMaster->GetPara();
/*N*/ 
/*N*/         // there is a master for this follow and the master does not have
/*N*/         // any contents (especially it does not have a number portion)
/*N*/         bNumDone = ! pPara ||
/*N*/                    ! ((SwParaPortion*)pPara)->GetFirstPortion()->IsFlyPortion();
/*N*/     }
/*N*/ 
/*N*/ 	pRoot = 0;
/*N*/ 	pLast = 0;
/*N*/ 	pFly = 0;
/*N*/ 	pLastFld = 0;
/*N*/ 	pLastTab = 0;
/*N*/ 	pUnderFlow = 0;
/*N*/ 	cTabDecimal = 0;
/*N*/ 	nWidth = nRealWidth;
/*N*/ 	nForcedLeftMargin = 0;
/*N*/ 	nSoftHyphPos = 0;
/*N*/     nUnderScorePos = STRING_LEN;
/*N*/ 	cHookChar = 0;
/*N*/ 	SetIdx(0);
/*N*/ 	SetLen( GetTxt().Len() );
/*N*/ 	SetPaintOfst(0);
/*N*/ }

/*-----------------16.10.00 11:39-------------------
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 * --------------------------------------------------*/


/*************************************************************************
 *				   SwTxtFormatInfo::_CheckFtnPortion()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::_CheckFtnPortion( SwLineLayout* pCurr )
/*N*/ {
/*N*/ 	KSHORT nHeight = pCurr->GetRealHeight();
/*N*/ 	SwLinePortion *pPor = pCurr->GetPortion();
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 	while( pPor )
/*N*/ 	{
/*N*/ 		if( pPor->IsFtnPortion() && nHeight > ((SwFtnPortion*)pPor)->Orig() )
/*N*/ 		{
/*?*/ 			bRet = sal_True;
/*?*/ 			SetLineHeight( nHeight );
/*?*/             SetLineNettoHeight( pCurr->Height() );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }




/*************************************************************************
 *				   SwTxtFormatInfo::ScanPortionEnd()
 *************************************************************************/
/*N*/ xub_StrLen SwTxtFormatInfo::ScanPortionEnd( const xub_StrLen nStart,
/*N*/                                             const xub_StrLen nEnd )
/*N*/ {
/*N*/ 	cHookChar = 0;
/*N*/ 	const xub_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
/*N*/ 	xub_StrLen i = nStart;
/*N*/ 
/*N*/     // Removed for i7288. bSkip used to be passed from SwFldPortion::Format
/*N*/     // as IsFollow(). Therefore more than one special character was not
/*N*/     // handled correctly at the beginning of follow fields.
/*N*/ //    if ( bSkip && i < nEnd )
/*N*/ //       ++i;
/*N*/ 
/*N*/ 	for( ; i < nEnd; ++i )
/*N*/ 	{
/*N*/ 		const xub_Unicode cPos = GetChar( i );
/*N*/ 		switch( cPos )
/*N*/ 		{
/*N*/ 		case CH_TXTATR_BREAKWORD:
/*N*/ 		case CH_TXTATR_INWORD:
/*N*/ 			if( !HasHint( i ))
/*N*/ 				break;
/*N*/ 			// no break;
/*N*/ 
/*N*/ 		case CHAR_SOFTHYPHEN:
/*N*/ 		case CHAR_HARDHYPHEN:
/*N*/ 		case CHAR_HARDBLANK:
/*N*/ 		case CH_TAB:
/*N*/ 		case CH_BREAK:
/*N*/ 			cHookChar = cPos;
/*N*/ 			return i;
/*N*/ 
/*N*/         case CHAR_UNDERSCORE:
/*N*/             if ( STRING_LEN == nUnderScorePos )
/*N*/                 nUnderScorePos = i;
/*N*/             break;
/*N*/ 
/*N*/         default:
/*N*/ 			if( cTabDec == cPos )
/*N*/ 			{
/*N*/ 				ASSERT( cPos, "Unexspected end of string" );
/*N*/ 				if( cPos ) // robust
/*N*/ 				{
/*N*/ 					cHookChar = cPos;
/*N*/ 					return i;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return i;
/*N*/ }

/*N*/ BOOL SwTxtFormatInfo::LastKernPortion()
/*N*/ {
/*N*/ 	if( GetLast() )
/*N*/ 	{
/*N*/ 	 	if( GetLast()->IsKernPortion() )
/*N*/ 			return TRUE;
/*N*/ 		if( GetLast()->Width() || ( GetLast()->GetLen() &&
/*N*/ 			!GetLast()->IsHolePortion() ) )
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	SwLinePortion* pPor = GetRoot();
/*N*/ 	SwLinePortion *pKern = NULL;
/*N*/ 	while( pPor )
/*N*/ 	{
/*N*/ 		if( pPor->IsKernPortion() )
/*N*/ 			pKern = pPor;
/*N*/ 		else if( pPor->Width() || ( pPor->GetLen() && !pPor->IsHolePortion() ) )
/*N*/ 			pKern = NULL;
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 	if( pKern )
/*N*/ 	{
/*?*/ 		SetLast( pKern );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
 *						class SwTxtSlot
 *************************************************************************/

/*N*/ SwTxtSlot::SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor )
/*N*/ {
/*N*/ 	bOn = pPor->GetExpTxt( *pNew, aTxt );
/*N*/ 
/*N*/ 	// Der Text wird ausgetauscht...
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		pInf = (SwTxtSizeInfo*)pNew;
/*N*/ 		nIdx = pInf->GetIdx();
/*N*/ 		nLen = pInf->GetLen();
/*N*/ 		pInf->SetLen( pPor->GetLen() );
/*N*/ 		pOldTxt = &(pInf->GetTxt());
/*N*/ 		pInf->SetTxt( aTxt );
/*N*/ 		pInf->SetIdx( 0 );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						 SwTxtSlot::~SwTxtSlot()
 *************************************************************************/

/*N*/ SwTxtSlot::~SwTxtSlot()
/*N*/ {
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		pInf->SetTxt( *pOldTxt );
/*N*/ 		pInf->SetIdx( nIdx );
/*N*/ 		pInf->SetLen( nLen );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						class SwTxtSlotLen
 *************************************************************************/

/*N*/ SwTxtSlotLen::SwTxtSlotLen( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
/*N*/ 	const sal_Char *pCh )
/*N*/ {
/*N*/ 	if( pCh )
/*N*/ 	{
/*?*/ 		aTxt = XubString( pCh, RTL_TEXTENCODING_MS_1252 );
/*?*/ 		bOn = sal_True;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bOn = pPor->GetExpTxt( *pNew, aTxt );
/*N*/ 
/*N*/ 	// Der Text wird ausgetauscht...
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		pInf = (SwTxtSizeInfo*)pNew;
/*N*/ 		nIdx = pInf->GetIdx();
/*N*/ 		nLen = pInf->GetLen();
/*N*/ 		pOldTxt = &(pInf->GetTxt());
/*N*/ 		pInf->SetTxt( aTxt );
/*N*/ 		pInf->SetIdx( 0 );
/*N*/ 		pInf->SetLen( pInf->GetTxt().Len() );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						 SwTxtSlotLen::~SwTxtSlotLen()
 *************************************************************************/

/*N*/ SwTxtSlotLen::~SwTxtSlotLen()
/*N*/ {
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		pInf->SetTxt( *pOldTxt );
/*N*/ 		pInf->SetIdx( nIdx );
/*N*/ 		pInf->SetLen( nLen );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					   SwFontSave::SwFontSave()
 *************************************************************************/

/*N*/ SwFontSave::SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pNew,
/*N*/ 		SwAttrIter* pItr )
/*N*/ 		: pFnt( pNew ? ((SwTxtSizeInfo&)rInf).GetFont() : 0 )
/*N*/ {
/*N*/ 	if( pFnt )
/*N*/ 	{
/*N*/ 		pInf = &((SwTxtSizeInfo&)rInf);
/*N*/         // In these cases we temporarily switch to the new font:
/*N*/         // 1. the fonts have a different magic number
/*N*/         // 2. they have different script types
/*N*/         // 3. their background colors differ (this is not covered by 1.)
/*N*/ 		if( pFnt->DifferentMagic( pNew, pFnt->GetActual() ) ||
/*N*/             pNew->GetActual() != pFnt->GetActual() ||
/*N*/             ( ! pNew->GetBackColor() && pFnt->GetBackColor() ) ||
/*N*/             ( pNew->GetBackColor() && ! pFnt->GetBackColor() ) ||
/*N*/             ( pNew->GetBackColor() && pFnt->GetBackColor() &&
/*N*/               ( *pNew->GetBackColor() != *pFnt->GetBackColor() ) ) )
/*N*/ 		{
/*N*/ 			pNew->SetTransparent( sal_True );
/*N*/ 			pNew->SetAlign( ALIGN_BASELINE );
/*N*/ 			pInf->SetFont( pNew );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFnt = 0;
/*N*/ 		pNew->Invalidate();
/*N*/ 		pNew->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
/*N*/ 		if( pItr && pItr->GetFnt() == pFnt )
/*N*/ 		{
/*?*/ 			pIter = pItr;
/*?*/ 			pIter->SetFnt( pNew );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pIter = NULL;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					   SwFontSave::~SwFontSave()
 *************************************************************************/

/*N*/ SwFontSave::~SwFontSave()
/*N*/ {
/*N*/ 	if( pFnt )
/*N*/ 	{
/*N*/ 		// SwFont zurueckstellen
/*N*/ 		pFnt->Invalidate();
/*N*/ 		pInf->SetFont( pFnt );
/*N*/ 		if( pIter )
/*N*/ 		{
/*?*/ 			pIter->SetFnt( pFnt );
/*?*/ 			pIter->nPos = STRING_LEN;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					   SwDefFontSave::SwDefFontSave()
 *************************************************************************/


/*************************************************************************
 *					   SwDefFontSave::~SwDefFontSave()
 *************************************************************************/


/*************************************************************************
 *					SwTxtFormatInfo::ChgHyph()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::ChgHyph( const sal_Bool bNew )
/*N*/ {
/*N*/ 	const sal_Bool bOld = bAutoHyph;
/*N*/ 	if( bAutoHyph != bNew )
/*N*/ 	{
/*N*/ 		bAutoHyph = bNew;
/*N*/ 		InitHyph( bNew );
/*N*/ 		// 5744: Sprache am Hyphenator einstellen.
/*N*/ 		if( pFnt )
/*N*/ 			pFnt->ChgPhysFnt( pVsh, pOut );
/*N*/ 	}
/*N*/ 	return bOld;
/*N*/ }


}
