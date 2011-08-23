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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_LSPCITEM_HXX //autogen
#include <bf_svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <bf_svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <bf_svx/pgrditem.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>	// ViewShell
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>  // SwPageFrm
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _ATRHNDL_HXX
#include <atrhndl.hxx>
#endif
namespace binfilter {

/*************************************************************************
 *						class SwBreakPortion
 *************************************************************************/
/*N*/ SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion )
/*N*/     : SwLinePortion( rPortion ), nRestWidth( 0 )
/*N*/ {
/*N*/ 	nLineLength = 1;
/*N*/ 	SetWhichPor( POR_BRK );
/*N*/ }


/*N*/ SwLinePortion *SwBreakPortion::Compress()
/*N*/ { return (GetPortion() && GetPortion()->InTxtGrp() ? 0 : this); }


/*************************************************************************
 *					SwBreakPortion::CalcViewWidth()
 *************************************************************************/


/*************************************************************************
 *				   virtual SwBreakPortion::Format()
 *************************************************************************/

/*N*/ sal_Bool SwBreakPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/     nRestWidth = (USHORT)(rInf.Width() - rInf.X());
/*N*/ 	register const SwLinePortion *pRoot = rInf.GetRoot();
/*N*/ 	Width( 0 );
/*N*/ 	Height( pRoot->Height() );
/*N*/ 	SetAscent( pRoot->GetAscent() );
/*N*/ 	if ( rInf.GetIdx()+1 == rInf.GetTxt().Len() )
/*N*/ 		rInf.SetNewLine( sal_True );
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *              virtual SwBreakPortion::HandlePortion()
 *************************************************************************/



/*N*/ SwKernPortion::SwKernPortion( SwLinePortion &rPortion, short nKrn,
/*N*/                               sal_Bool bBG, sal_Bool bGK ) :
/*N*/     nKern( nKrn ), bBackground( bBG ), bGridKern( bGK )
/*N*/ {
/*N*/ 	Height( rPortion.Height() );
/*N*/ 	SetAscent( rPortion.GetAscent() );
/*N*/ 	nLineLength = 0;
/*N*/ 	SetWhichPor( POR_KERN );
/*N*/ 	if( nKern > 0 )
/*N*/ 		Width( nKern );
/*N*/  	rPortion.Insert( this );
/*N*/ }


/*N*/ void SwKernPortion::Paint( const SwTxtPaintInfo &rInf ) const
/*N*/ {
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if( Width() )
/*N*/ }

/*N*/ void SwKernPortion::FormatEOL( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/     if ( bGridKern )
/*N*/         return;
/*N*/ 
/*N*/     if( rInf.GetLast() == this )
/*N*/ 		rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
/*N*/ 	if( nKern < 0 )
/*N*/ 		Width( -nKern );
/*N*/ 	else
/*N*/ 		Width( 0 );
/*N*/ 	rInf.GetLast()->FormatEOL( rInf );
/*N*/ }

/*N*/ SwArrowPortion::SwArrowPortion( const SwLinePortion &rPortion ) :
/*N*/ 	bLeft( sal_True )
/*N*/ {
/*N*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	Height( rPortion.Height() );
/*N*/ }

/*N*/ void SwArrowPortion::Paint( const SwTxtPaintInfo &rInf ) const
/*N*/ {
/*N*/ 		DBG_BF_ASSERT(0, "STRIP");  //STRIP001 	((SwArrowPortion*)this)->aPos = rInf.GetPos();
/*N*/ }

/*N*/ SwLinePortion *SwArrowPortion::Compress() { return this; }

/*N*/ SwTwips SwTxtFrm::EmptyHeight() const
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::EmptyHeight with swapped frame" );
/*N*/ 
/*N*/     SwFont *pFnt;
/*N*/ 	const SwTxtNode& rTxtNode = *GetTxtNode();
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	if ( rTxtNode.HasSwAttrSet() )
/*N*/ 	{
/*N*/ 		const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
/*N*/         pFnt = new SwFont( pAttrSet, GetTxtNode()->GetDoc() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh);
/*N*/         pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
/*N*/ 		pFnt->ChkMagic( pSh, pFnt->GetActual() );
/*N*/ 	}
/*N*/ 
/*N*/     if ( IsVertical() )
/*?*/         pFnt->SetVertical( 2700 );
/*N*/ 
/*N*/ 	OutputDevice *pOut = pSh ? pSh->GetOut() : 0;
/*N*/ 	if ( !pOut || !rTxtNode.GetDoc()->IsBrowseMode() ||
/*N*/ 		 ( pSh->GetViewOptions()->IsPrtFormat() ) )
/*N*/ 	{
/*N*/         pOut = &rTxtNode.GetDoc()->GetRefDev();
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwDoc* pDoc = rTxtNode.GetDoc();
  /*N*/ 	if( ::binfilter::IsShowChanges( pDoc->GetRedlineMode() ) )
/*N*/ 	{
/*N*/ 		MSHORT nRedlPos = pDoc->GetRedlinePos( rTxtNode );
/*N*/         if( MSHRT_MAX != nRedlPos )
/*N*/         {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             SwAttrHandler aAttrHandler;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	SwTwips nRet;
/*N*/ 	if( !pOut )
/*?*/         nRet = IsVertical() ?
/*?*/                Prt().SSize().Width() + 1 :
/*?*/                Prt().SSize().Height() + 1;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pFnt->SetFntChg( sal_True );
/*N*/ 		pFnt->ChgPhysFnt( pSh, pOut );
/*N*/ 		nRet = pFnt->GetHeight( pSh, pOut );
/*N*/ 	}
/*N*/ 	delete pFnt;
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::FormatEmpty()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::FormatEmpty()
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::FormatEmpty with swapped frame" );
/*N*/ 
/*N*/ 	if ( HasFollow() || GetTxtNode()->GetpSwpHints() ||
/*N*/ 		0 != GetTxtNode()->GetNumRule() ||
/*N*/ 		0 != GetTxtNode()->GetOutlineNum() ||
/*N*/ 		 IsInFtn() || ( HasPara() && GetPara()->IsPrepMustFit() ) )
/*N*/ 		return sal_False;
/*N*/ 	const SwAttrSet& aSet = GetTxtNode()->GetSwAttrSet();
/*N*/ #ifdef BIDI
/*N*/     const USHORT nAdjust = aSet.GetAdjust().GetAdjust();
/*N*/     if( ( ( ! IsRightToLeft() && ( SVX_ADJUST_LEFT != nAdjust ) ) ||
/*N*/           (   IsRightToLeft() && ( SVX_ADJUST_RIGHT != nAdjust ) ) ) ||
/*N*/           aSet.GetRegister().GetValue() )
/*N*/ #else
/*N*/ 	if( SVX_ADJUST_LEFT != aSet.GetAdjust().GetAdjust()
/*N*/ 		|| aSet.GetRegister().GetValue() )
/*N*/ #endif
/*N*/ 		return sal_False;
/*N*/ 	const SvxLineSpacingItem &rSpacing = aSet.GetLineSpacing();
/*N*/ 	if( SVX_LINE_SPACE_MIN == rSpacing.GetLineSpaceRule() ||
/*N*/ 		SVX_LINE_SPACE_FIX == rSpacing.GetLineSpaceRule() ||
/*N*/ 		aSet.GetLRSpace().IsAutoFirst() )
/*N*/ 		return sal_False;
/*N*/ 	else
/*N*/ 	{
/*N*/         SwTxtFly aTxtFly( this );
/*N*/ 		SwRect aRect;
/*N*/ 		sal_Bool bFirstFlyCheck = 0 != Prt().Height();
/*N*/ 		if ( bFirstFlyCheck &&
/*N*/ 			 aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
/*N*/ 			return sal_False;
/*N*/ 		else
/*N*/ 		{
/*N*/             SwTwips nHeight = EmptyHeight();
/*N*/ 
/*N*/             if ( GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() &&
/*N*/                  IsInDocBody() )
/*N*/             {
/*N*/                 GETGRID( FindPageFrm() )
/*N*/                 if ( pGrid )
/*?*/                     nHeight = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
/*N*/             }
/*N*/ 
/*N*/             SWRECTFN( this )
/*N*/             const SwTwips nChg = nHeight - (Prt().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/             if( !nChg )
/*N*/ 				SetUndersized( sal_False );
/*N*/ 			AdjustFrm( nChg );
/*N*/ 
/*N*/ 			if( HasBlinkPor() )
/*N*/ 			{
/*?*/ 				ClearPara();
/*?*/ 				ResetBlinkPor();
/*N*/ 			}
/*N*/ 			SetCacheIdx( MSHRT_MAX );
/*N*/ 			if( !IsEmpty() )
/*N*/ 			{
/*N*/ 				SetEmpty( sal_True );
/*N*/ 				SetCompletePaint();
/*N*/ 			}
/*N*/ 			if( !bFirstFlyCheck &&
/*N*/ 				 aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
/*N*/ 				 return sal_False;
/*N*/ 			return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ sal_Bool SwTxtFrm::FillRegister( SwTwips& rRegStart, KSHORT& rRegDiff )
/*N*/ {
/*N*/ 	const SwFrm *pFrm = this;
/*N*/ 	rRegDiff = 0;
/*N*/ 	while( !( ( FRM_BODY | FRM_FLY )
/*N*/ 		   & pFrm->GetType() ) && pFrm->GetUpper() )
/*N*/ 		pFrm = pFrm->GetUpper();
/*N*/ 	if( ( FRM_BODY| FRM_FLY ) & pFrm->GetType() )
/*N*/ 	{
/*N*/         SWRECTFN( pFrm )
/*N*/         rRegStart = (pFrm->*fnRect->fnGetPrtTop)();
/*N*/ 		pFrm = pFrm->FindPageFrm();
/*N*/ 		if( pFrm->IsPageFrm() )
/*N*/ 		{
/*N*/ 			SwPageDesc* pDesc = ((SwPageFrm*)pFrm)->FindPageDesc();
/*N*/ 			if( pDesc )
/*N*/ 			{
/*N*/ 				rRegDiff = pDesc->GetRegHeight();
/*N*/ 				if( !rRegDiff )
/*N*/ 				{
/*N*/ 					const SwTxtFmtColl *pFmt = pDesc->GetRegisterFmtColl();
/*N*/ 					if( pFmt )
/*N*/ 					{
/*N*/ 						const SvxLineSpacingItem &rSpace = pFmt->GetLineSpacing();
/*N*/ 						if( SVX_LINE_SPACE_FIX == rSpace.GetLineSpaceRule() )
/*N*/ 						{
/*?*/ 							rRegDiff = rSpace.GetLineHeight();
/*?*/ 							pDesc->SetRegHeight( rRegDiff );
/*?*/ 							pDesc->SetRegAscent( ( 4 * rRegDiff ) / 5 );
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							ViewShell *pSh = GetShell();
/*N*/ 							SwFontAccess aFontAccess( pFmt, pSh );
/*N*/ 							SwFont aFnt( *aFontAccess.Get()->GetFont() );
/*N*/ 							OutputDevice *pOut = 0;
/*N*/ 							if( !GetTxtNode()->GetDoc()->IsBrowseMode() ||
/*N*/ 								(pSh && pSh->GetViewOptions()->IsPrtFormat()) )
/*N*/                                 pOut = &GetTxtNode()->GetDoc()->GetRefDev();
/*N*/                             if( pSh && !pOut )
/*?*/ 								pOut = pSh->GetWin();
/*N*/ 							if( !pOut )
/*?*/ 								pOut = GetpApp()->GetDefaultDevice();
/*N*/ 							MapMode aOldMap( pOut->GetMapMode() );
/*N*/ 							pOut->SetMapMode( MapMode( MAP_TWIP ) );
/*N*/ 							aFnt.ChgFnt( pSh, pOut );
/*N*/ 							rRegDiff = aFnt.GetHeight( pSh, pOut );
/*N*/ 							KSHORT nNettoHeight = rRegDiff;
/*N*/ 							switch( rSpace.GetLineSpaceRule() )
/*N*/ 							{
/*N*/ 								case SVX_LINE_SPACE_AUTO:
/*N*/ 								break;
/*N*/ 								case SVX_LINE_SPACE_MIN:
/*N*/ 								{
/*N*/ 									if( rRegDiff < KSHORT( rSpace.GetLineHeight() ) )
/*N*/ 										rRegDiff = rSpace.GetLineHeight();
/*N*/ 									break;
/*N*/ 								}
/*N*/ 								default: ASSERT(
/*N*/ 								sal_False, ": unknown LineSpaceRule" );
/*N*/ 							}
/*N*/ 							switch( rSpace.GetInterLineSpaceRule() )
/*N*/ 							{
/*N*/ 								case SVX_INTER_LINE_SPACE_OFF:
/*N*/ 								break;
/*?*/ 								case SVX_INTER_LINE_SPACE_PROP:
/*?*/ 								{
/*?*/ 									long nTmp = rSpace.GetPropLineSpace();
/*?*/ 									if( nTmp < 50 )
/*?*/ 										nTmp = nTmp ? 50 : 100;
/*?*/ 									nTmp *= rRegDiff;
/*?*/ 									nTmp /= 100;
/*?*/ 									if( !nTmp )
/*?*/ 										++nTmp;
/*?*/ 									rRegDiff = (KSHORT)nTmp;
/*?*/ 									nNettoHeight = rRegDiff;
/*?*/ 									break;
/*?*/ 								}
/*?*/ 								case SVX_INTER_LINE_SPACE_FIX:
/*?*/ 								{
/*?*/ 									rRegDiff += rSpace.GetInterLineSpace();
/*?*/ 									nNettoHeight = rRegDiff;
/*?*/ 									break;
/*?*/ 								}
/*?*/ 								default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
/*N*/ 							}
/*N*/ 							pDesc->SetRegHeight( rRegDiff );
/*N*/ 							pDesc->SetRegAscent( rRegDiff - nNettoHeight +
/*N*/ 												 aFnt.GetAscent( pSh, pOut ) );
/*N*/ 							pOut->SetMapMode( aOldMap );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/                 const long nTmpDiff = pDesc->GetRegAscent() - rRegDiff;
/*N*/                 if ( bVert )
/*?*/                     rRegStart -= nTmpDiff;
/*N*/                 else
/*N*/                     rRegStart += nTmpDiff;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return ( 0 != rRegDiff );
/*N*/ }
}
