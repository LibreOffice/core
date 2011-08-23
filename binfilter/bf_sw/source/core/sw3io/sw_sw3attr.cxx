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

#include <stdlib.h>
#include <stdio.h>


#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTEIRO_HXX //autogen
#include <fmteiro.hxx>
#endif
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#include <hfspacingitem.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _SW3IMP_HXX
#include <sw3imp.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FMTFTNTX_HXX //autogen
#include <fmtftntx.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
namespace binfilter {

////////////////////////////// Frame-Attribute ////////////////////////////


/*N*/ USHORT SwFmtSurround::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtSurround: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? 0 :
/*N*/ 		   (SOFFICE_FILEFORMAT_40==nFFVer ? 4 :	5);
/*N*/ }


/*N*/ SfxPoolItem* SwFmtSurround::Create( SvStream& rStrm, USHORT nVrs ) const
/*N*/ {
/*N*/ 	BYTE nType, bGold = 0, bAnch=0, bCont=0, bOutside = 0;
/*N*/ 	rStrm >> nType;
/*N*/ 	if ( nVrs < 5 )
/*N*/ 		rStrm >> bGold;
/*N*/ 	if ( nVrs > 1 )
/*N*/ 		rStrm >> bAnch;
/*N*/ 	if ( nVrs > 2 )
/*N*/ 		rStrm >> bCont;
/*N*/ 	if ( nVrs > 3 )
/*N*/ 		rStrm >> bOutside;
/*N*/ 
/*N*/ 	SwFmtSurround *pRet = new SwFmtSurround( (SwSurround) nType );
/*N*/ 	if( bGold && SURROUND_NONE != (SwSurround)nType &&
/*N*/ 				 SURROUND_THROUGHT != (SwSurround)nType )
/*N*/ 		pRet->SetSurround( SURROUND_IDEAL );
/*N*/ 	pRet->SetAnchorOnly( BOOL(bAnch) );
/*N*/ 	pRet->SetContour( BOOL(bCont) );
/*N*/ 	pRet->SetOutside( BOOL(bOutside) );
/*N*/ 	return pRet;
/*N*/ }


/*N*/ SvStream& SwFmtSurround::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	if ( nIVer < 5 )
/*N*/ 	{
/*N*/ 		SwSurround eType = GetSurround();
/*N*/ 		BYTE bGold = 0;
/*N*/ 		if( SURROUND_IDEAL == eType )
/*N*/ 		{
/*N*/ 			eType = SURROUND_PARALLEL;
/*N*/ 			bGold = 1;
/*N*/ 		}
/*N*/ 		rStrm << (BYTE) eType
/*N*/ 			  << (BYTE) bGold;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm << (BYTE) GetSurround();
/*N*/ 	if( nIVer > 1 )
/*N*/ 		  rStrm << (BYTE) IsAnchorOnly();
/*N*/ 	if( nIVer > 2 )
/*N*/ 		  rStrm << (BYTE) IsContour();
/*N*/ 	if( nIVer > 3 )
/*N*/ 		  rStrm << (BYTE) IsOutside();
/*N*/ 	return rStrm;
/*N*/ }

/*N*/ USHORT SwFmtVertOrient::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtHoriOrient: Gibt es ein neues Fileformat?" );
/*N*/ 	return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			 SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_VERTORIENT_REL;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtVertOrient::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	long nPos;
/*N*/ 	BYTE nOrient, nRelation;
/*N*/ 	rStrm >> nPos >> nOrient >> nRelation;
/*N*/ 
/*N*/ 	// fix #48690#: In 4.0-Doks wurde bei VERT_NONE die Relation nicht
/*N*/ 	// beachtet, aber wie FRAME behandelt. Das Attribut enthielt aber PRTAREA
/*N*/ 	if( VERT_NONE == (SwVertOrient)nOrient && nIVer < IVER_VERTORIENT_REL )
/*N*/ 		nRelation = FRAME;
/*N*/ 
/*N*/ 	return new SwFmtVertOrient( (SwTwips) nPos, (SwVertOrient) nOrient,
/*N*/ 								(SwRelationOrient) nRelation );
/*N*/ }


/*N*/ SvStream& SwFmtVertOrient::Store( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 
/*N*/ 	SwTwips nPos = GetPos();
/*N*/ 
/*N*/ 	if( pIo && pIo->IsSw31Export() && pIo->pExportInfo &&
/*N*/ 		pIo->pExportInfo->bFlyFrmFmt )
/*N*/ 	{
/*N*/ 		const SfxItemSet *pItemSet = pIo->pExportInfo->pItemSet;
/*N*/ 		if( pItemSet )
/*N*/ 		{
/*N*/ 			const SvxULSpaceItem& rULSpace =
/*N*/ 				(const SvxULSpaceItem&)pItemSet->Get( RES_UL_SPACE );
/*N*/ 
/*N*/ 			nPos = GetPosConvertedToSw31( &rULSpace );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rStrm << (long) nPos
/*N*/ 		  << (BYTE) GetVertOrient()
/*N*/ 		  << (BYTE) GetRelationOrient();
/*N*/ 	return rStrm;
/*N*/ }

/*N*/ USHORT SwFmtHoriOrient::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtHoriOrient: Gibt es ein neues Fileformat?" );
/*N*/ 	return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			 SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_HORIORIENT_REL;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtHoriOrient::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	long nPos;
/*N*/ 	BYTE nOrient, nRelation, bToggle = 0, bGrf = 0;
/*N*/ 	rStrm >> nPos >> nOrient >> nRelation;
/*N*/ 
/*N*/ 	if( nIVer >= IVER_HORIORIENT_TOGGLE )
/*N*/ 		rStrm >> bToggle;
/*N*/ 
/*N*/ 	// fix #48690#: In 4.0-Doks wurde bei HORI_NONE die Relation nicht
/*N*/ 	// beachtet, aber wie FRAME behandelt. Das Attribut enthielt aber PRTAREA
/*N*/ 	if( HORI_NONE == (SwHoriOrient)nOrient && nIVer < IVER_HORIORIENT_REL )
/*N*/ 		nRelation = FRAME;
/*N*/ 
/*N*/ 	return new SwFmtHoriOrient
/*N*/ 		( (SwTwips) nPos, (SwHoriOrient) nOrient, (SwRelationOrient) nRelation,
/*N*/ 		  BOOL( bToggle ) );
/*N*/ }


/*N*/ SvStream& SwFmtHoriOrient::Store( SvStream& rStrm, USHORT nVersion ) const
/*N*/ {
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 
/*N*/ 	SwTwips nPos = GetPos();
/*N*/ 
/*N*/ 	if( pIo && pIo->IsSw31Export() && pIo->pExportInfo &&
/*N*/ 		pIo->pExportInfo->bFlyFrmFmt )
/*N*/ 	{
/*N*/ 		const SfxItemSet *pItemSet = pIo->pExportInfo->pItemSet;
/*N*/ 		if( pItemSet )
/*N*/ 		{
/*N*/ 			const SvxLRSpaceItem& rLRSpace =
/*N*/ 				(const SvxLRSpaceItem&)pItemSet->Get( RES_LR_SPACE );
/*N*/ 
/*N*/ 			nPos = GetPosConvertedToSw31( &rLRSpace );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nVersion >= IVER_HORIORIENT_TOGGLE )
/*N*/ 	{
/*N*/ 		rStrm	<< (long) nPos
/*N*/ 				<< (BYTE) GetHoriOrient()
/*N*/ 				<< (BYTE) GetRelationOrient()
/*N*/ 				<< (BYTE) IsPosToggle();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwHoriOrient eHori = GetHoriOrient();
/*N*/ 		SwRelationOrient eRel = GetRelationOrient();
/*N*/ 		if( eRel > PRTAREA )
/*N*/ 		{
/*?*/           if( !(pIo && pIo->pExportInfo && pIo->pExportInfo->pFlyFrm &&
/*?*/               pIo->pExportInfo->pFlyFrm->ConvertHoriTo40( eHori, eRel, nPos ) ) )
/*?*/           {
/*?*/               switch ( eRel )
/*?*/               {
/*?*/                   case REL_PG_LEFT: eRel = FRAME; eHori = HORI_LEFT; break;
/*?*/                   case REL_PG_RIGHT: eRel = FRAME; break;
/*?*/                   case REL_FRM_LEFT: eRel = PRTAREA; break;
/*?*/                   case REL_FRM_RIGHT: eRel = PRTAREA; break;
/*?*/                   case REL_PG_PRTAREA: eRel = PRTAREA; break;
/*?*/                   default: eRel = FRAME; break;
/*?*/               }
/*?*/           }
/*N*/ 		}
/*N*/ 		rStrm << (long) nPos
/*N*/ 			  << (BYTE) eHori;
/*N*/ 		rStrm << (BYTE) eRel;
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwFmtFrmSize::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtFrmSize: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? 0 : 2;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtFrmSize::Create( SvStream& rStrm, USHORT nVersion ) const
/*N*/ {
/*N*/ 	BYTE nSizeType, nWidthPercent = 0, nHeightPercent = 0;
/*N*/ 	INT32 nWidth, nHeight;
/*N*/ 	rStrm >> nSizeType >> nWidth >> nHeight;
/*N*/ 
/*N*/ 	if ( nVersion > 1 )
/*N*/ 		rStrm >> nWidthPercent >> nHeightPercent;
/*N*/ 
/*N*/ 	SwFmtFrmSize *pRet = new SwFmtFrmSize( (SwFrmSize) nSizeType, nWidth, nHeight );
/*N*/ 	pRet->SetWidthPercent ( nWidthPercent );
/*N*/ 	pRet->SetHeightPercent( nHeightPercent );
/*N*/ 	return pRet;
/*N*/ }


/*N*/ SvStream& SwFmtFrmSize::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 
/*N*/ 	Size aSz( GetSize() );
/*N*/ 	if( pIo && pIo->IsSw31Export() && pIo->pExportInfo &&
/*N*/ 		pIo->pExportInfo->bFlyFrmFmt )
/*N*/ 	{
/*N*/ 		const SfxItemSet *pItemSet = pIo->pExportInfo->pItemSet;
/*N*/ 		if( pItemSet )
/*N*/ 		{
/*N*/ 			const SvxLRSpaceItem& rLRSpace =
/*N*/ 				(const SvxLRSpaceItem&)pItemSet->Get( RES_LR_SPACE );
/*N*/ 			const SvxULSpaceItem& rULSpace =
/*N*/ 				(const SvxULSpaceItem&)pItemSet->Get( RES_UL_SPACE );
/*N*/ 
/*N*/ 			aSz = GetSizeConvertedToSw31( &rLRSpace, &rULSpace );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rStrm << (BYTE)  GetSizeType()
/*N*/ 		  << (INT32) aSz.Width()
/*N*/ 		  << (INT32) aSz.Height();
/*N*/ 
/*N*/ 	if( nIVer > 1 )
/*N*/ 	{
/*N*/ 		rStrm << (BYTE)  GetWidthPercent()
/*N*/ 			  << (BYTE)  GetHeightPercent();
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtFillOrder::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	BYTE nFillOrder;
/*N*/ 	rStrm >> nFillOrder;
/*N*/ 	return new SwFmtFillOrder( (SwFillOrder) nFillOrder );
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ SvStream& SwFmtFillOrder::Store( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE) GetFillOrder();
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtCol::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	// Die Longs muessen runtergerechnet werden
/*N*/ 	BYTE   nLineAdj, bOrtho, nLineHeight, nPenStyle;
/*N*/ 	INT16  nGutterWidth, nPenWidth;
/*N*/ 	UINT16 nWishWidth, nPenRed, nPenGreen, nPenBlue;
/*N*/ 	rStrm >> nLineAdj
/*N*/ 		  >> bOrtho
/*N*/ 		  >> nLineHeight
/*N*/ 		  >> nGutterWidth
/*N*/ 		  >> nWishWidth
/*N*/ 		  >> nPenStyle
/*N*/ 		  >> nPenWidth
/*N*/ 		  >> nPenRed
/*N*/ 		  >> nPenGreen
/*N*/ 		  >> nPenBlue;
/*N*/ 	SwFmtCol* p = new SwFmtCol;
/*N*/ 	Color aPenColor( nPenRed >> 8, nPenGreen >> 8, nPenBlue >> 8 );
/*N*/ //	Pen aPen( aPenColor, nPenWidth, (PenStyle) nPenStyle );
/*N*/ 	INT16 nCol;
/*N*/ 	rStrm >> nCol;
/*N*/ 	ASSERT( nWishWidth, "Damaged Doc: No WishWidth" );
/*N*/ 	if( !nWishWidth )
/*N*/ 	{
/*?*/ 		nWishWidth = USHRT_MAX;
/*?*/ 		if( nCol )
/*?*/           p->Init( nCol, nGutterWidth, nWishWidth );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for( short i = 0; i < nCol; i++ )
/*N*/ 		{
/*N*/ 			UINT16 nWidth, nLeft, nUpper, nRight, nLower;
/*N*/ 			rStrm >> nWidth >> nLeft >> nUpper >> nRight >> nLower;
/*N*/ 			SwColumn* pCol = new SwColumn;
/*N*/ 			pCol->SetWishWidth( nWidth );
/*N*/ 			pCol->SetLeft( nLeft );
/*N*/ 			pCol->SetUpper( nUpper );
/*N*/ 			pCol->SetRight( nRight );
/*N*/ 			pCol->SetLower( nLower );
/*N*/ 			p->GetColumns().Insert( pCol, i );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	p->SetLineWidth( nPenWidth );
/*N*/ 	p->SetLineColor( aPenColor );
/*N*/ 	p->SetWishWidth( nWishWidth );
/*N*/ 	p->SetLineHeight( nLineHeight );
/*N*/ 	p->SetLineAdj( (SwColLineAdj) nLineAdj );
/*N*/ 	// temporaerer Bug Fix
/*N*/ 	if( nCol )
/*N*/ 		// Wert direkt mit dem Silberhammer einschlagen.
/*N*/ 		p->_SetOrtho( (BOOL) bOrtho );
/*N*/ 	return p;
/*N*/ }


/*N*/ SvStream& SwFmtCol::Store( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 
/*N*/ 	rStrm << (BYTE)  GetLineAdj()
/*N*/ 		  << (BYTE)  IsOrtho()
/*N*/ 		  << (BYTE)  GetLineHeight()
/*N*/ 		  << (INT16) GetGutterWidth()
/*N*/ 		  << (UINT16)GetWishWidth()
/*N*/ 		  << (BYTE)  0 // rPen.GetStyle() - not available anymore
/*N*/ 		  << (INT16) GetLineWidth()
/*N*/ 		  << (UINT16)(GetLineColor().GetRed() << 8 )
/*N*/ 		  << (UINT16)(GetLineColor().GetGreen() << 8 )
/*N*/ 		  << (UINT16)(GetLineColor().GetBlue() << 8 )
/*N*/ 			;
/*N*/ 	INT16 nCol = GetNumCols();
/*N*/ 	rStrm << (INT16) nCol;
/*N*/ 	for( short i = 0; i < nCol; i++ )
/*N*/ 	{
/*N*/ 		const SwColumn* pCol = GetColumns()[ i ];
/*N*/ 		rStrm << (UINT16)pCol->GetWishWidth()
/*N*/ 			  << (INT16) pCol->GetLeft()
/*N*/ 			  << (INT16) pCol->GetUpper()
/*N*/ 			  << (INT16) pCol->GetRight()
/*N*/ 			  << (INT16) pCol->GetLower();
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ SfxPoolItem * SwFmtURL::Create(SvStream &rStrm, USHORT nIVer) const
/*N*/ {
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 	ASSERT( pIo, "Reader/Writer not found" );
/*N*/ 
/*N*/ 	String sURL, sTargetFrameName, sName;
/*N*/ 	BOOL bServerMap = FALSE;
/*N*/ 
/*N*/ 	SwFmtURL *pNew = new SwFmtURL;
/*N*/ 
/*N*/ 	if( pIo )
/*N*/ 		pNew->SetMap( pIo->InImageMap( sURL, sTargetFrameName, bServerMap ) );
/*N*/ 	if( nIVer>=1 )
/*N*/ 		rStrm.ReadByteString( sName, rStrm.GetStreamCharSet() );
/*N*/ 
/*N*/ 	// fix #30592#: Boese Falle: Beim setzen einer URL wird eine Image-Map
/*N*/ 	// wieder geloescht.
/*N*/ 	if( sURL.Len() )
/*N*/ 		pNew->SetURL( sURL, bServerMap );
/*N*/ 	pNew->SetTargetFrameName( sTargetFrameName );
/*N*/ 	pNew->SetName( sName );
/*N*/ 	return pNew;
/*N*/ }


/*N*/ SvStream & SwFmtURL::Store( SvStream &rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	ASSERT( nIVer != USHRT_MAX,
/*N*/ 			"SwFmtURL: Wer faengt da Version USHRT_MAX nicht ab?" );
/*N*/ 
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 	ASSERT( pIo, "Reader/Writer not found" );
/*N*/ 
/*N*/ 	if( pIo )
/*N*/ 		pIo->OutImageMap( sURL, sTargetFrameName, pMap, bIsServerMap );
/*N*/ 	if( nIVer >= 1 )
/*N*/ 		rStrm.WriteByteString( GetName(), rStrm.GetStreamCharSet() );
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwFmtURL::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtURL: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 1;
/*N*/ }


SfxPoolItem* SwFmtEditInReadonly::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtEditInReadonly( RES_EDIT_IN_READONLY, BOOL(n) );
}


SvStream& SwFmtEditInReadonly::Store(SvStream &rStrm, USHORT ) const
{
    rStrm << BYTE(GetValue());
    return rStrm;
}


/*N*/ USHORT SwFmtEditInReadonly::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtEditInReadonly: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }


SfxPoolItem* SwFmtLayoutSplit::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtLayoutSplit( BOOL(n) );
}


SvStream& SwFmtLayoutSplit::Store(SvStream &rStrm, USHORT ) const
{
    rStrm << BYTE(GetValue());
    return rStrm;
}


/*N*/ USHORT SwFmtLayoutSplit::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtLayoutSplit: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 		   SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 0;
/*N*/ }

SvStream& SwFmtFtnEndAtTxtEnd::Store( SvStream & rStrm, USHORT nIVer ) const
{
// alt: nur ein BOOL, jetzt 2 enums, 2 Strings, 1 uint16
    if( 0 == nIVer )        // old - only a boolitem
    {
        rStrm << (BYTE)( FTNEND_ATPGORDOCEND == GetValue() ? 0 : 1 );
    }
    else
    {
        rStrm << (BYTE)GetValue()
              << (UINT16)nOffset
              << (UINT16)aFmt.GetNumberingType();
        rStrm.WriteByteString( sPrefix, rStrm.GetStreamCharSet() );
        rStrm.WriteByteString( sSuffix, rStrm.GetStreamCharSet() );
    }

    return rStrm;
}

SfxPoolItem* SwFmtFtnEndAtTxtEnd::Create( SvStream &rStrm, USHORT nVer ) const
{
    SwFmtFtnEndAtTxtEnd* pNew = (SwFmtFtnEndAtTxtEnd*)Clone();

    BYTE nVal;
    rStrm >> nVal;
    pNew->SetValue( nVal );

    if( 0 < nVer )
    {
        UINT16 nOffset, nFmtType;
        String sPostfix, sSuffix;
        rStrm >> nOffset
              >> nFmtType;
        rStrm.ReadByteString( sPostfix, rStrm.GetStreamCharSet() );
        rStrm.ReadByteString( sSuffix, rStrm.GetStreamCharSet() );

        pNew->SetNumType( (SvxExtNumType)nFmtType );
        pNew->SetOffset( nOffset );
        pNew->SetPrefix( sPostfix );
        pNew->SetSuffix( sSuffix );
    }
    return pNew;
}

/*N*/ USHORT SwFmtFtnEndAtTxtEnd::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtFtnEndAtTxtEnd: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 		   SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 1;
/*N*/ }

SfxPoolItem* SwFmtNoBalancedColumns::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtNoBalancedColumns( BOOL(n) );
}


SvStream& SwFmtNoBalancedColumns::Store(SvStream &rStrm, USHORT ) const
{
    rStrm << BYTE(GetValue());
    return rStrm;
}


/*N*/ USHORT SwFmtNoBalancedColumns::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtNoBalancedColumns: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 		   SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 0;
/*N*/ }


/*M*/ SfxPoolItem* SwHeaderAndFooterEatSpacingItem::Create(SvStream &rStrm, USHORT) const
/*M*/ {
/*M*/     ASSERT( sal_False, "SwHeaderAndFooterEatSpacingItem::Create called for old fileformat" )
/*M*/ 	BYTE n;
/*M*/ 	rStrm >> n;
/*M*/     return new SwHeaderAndFooterEatSpacingItem( RES_HEADER_FOOTER_EAT_SPACING, BOOL(n) );
/*M*/ }


/*M*/ SvStream& SwHeaderAndFooterEatSpacingItem::Store(SvStream &rStrm, USHORT ) const
/*M*/ {
/*M*/     ASSERT( sal_False, "SwHeaderAndFooterEatSpacingItem::Store called for old fileformat" )
/*M*/ 	rStrm << BYTE(GetValue());
/*M*/ 	return rStrm;
/*M*/ }


/*M*/ USHORT SwHeaderAndFooterEatSpacingItem::GetVersion( USHORT nFFVer ) const
/*M*/ {
/*M*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*M*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*M*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*M*/             "SwHeaderAndFooterEatSpacingItem: Gibt es ein neues Fileformat?" );
/*M*/     return USHRT_MAX;
/*M*/ }


/////////////////////////////// Grafik-Attribute /////////////////////////


/*N*/ SfxPoolItem* SwMirrorGrf::Create( SvStream& rStrm, USHORT nIVer) const
/*N*/ {
/*N*/ 	BYTE nState;
/*N*/ 	BYTE nToggle = FALSE;
/*N*/ 
/*N*/ 	rStrm >> nState;
/*N*/ 
/*N*/ 	if (nIVer > 0)
/*N*/ 		rStrm >> nToggle;
/*N*/ 
/*N*/ 	SwMirrorGrf* pRet = new SwMirrorGrf( nState );
/*N*/ 	pRet->SetGrfToggle((BOOL)nToggle);
/*N*/ 	return pRet;
/*N*/ }


/*N*/ SvStream& SwMirrorGrf::Store( SvStream& rStrm, USHORT nIVer) const
/*N*/ {
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 
/*N*/ 	if (nIVer > 0)
/*N*/ 		rStrm << (BYTE)bGrfToggle;
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwMirrorGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtSurround: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_40 < nFFVer ? 1 : 0;
/*N*/ }


/*N*/ USHORT	SwCropGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwCropGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return GRFCROP_VERSION_SWDEFAULT;
/*N*/ }

/*N*/ USHORT	SwRotationGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwRotationGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwRotationGrf::Create( SvStream & rStrm, USHORT nVer ) const
{
    SwRotationGrf* pRet = (SwRotationGrf*)SfxUInt16Item::Create( rStrm, nVer );
    Size aSz;
    rStrm >> aSz;
    pRet->SetUnrotatedSize( aSz );
    return pRet;
}

SvStream& SwRotationGrf::Store( SvStream & rStrm, USHORT nIVer) const
{
    SfxUInt16Item::Store( rStrm, nIVer );
    rStrm << GetUnrotatedSize();
    return rStrm;
}

/*N*/ USHORT SwLuminanceGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwLuminanceGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT SwContrastGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwContrastGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }
/*N*/ USHORT SwChannelGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwChannelGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT SwGammaGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwChannelGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwGammaGrf::Create(SvStream & rStrm, USHORT ) const
{
    SwGammaGrf* pRet = (SwGammaGrf*)Clone();
    double aVal;
    rStrm >> aVal;
    pRet->SetValue( aVal );
    return pRet;
}

SvStream& SwGammaGrf::Store(SvStream & rStrm, USHORT ) const
{
    rStrm << GetValue();
    return rStrm;
}

/*N*/ USHORT SwInvertGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwInvertGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT	SwTransparencyGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwInvertGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT	SwDrawModeGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwDrawModeGrf: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }



//////////////////////////////// Text-Attribute ////////////////////////////


/*N*/ SfxPoolItem* SwFmtHardBlank::Create( SvStream& rStrm, USHORT nV ) const
/*N*/ {
/*N*/ 	sal_Char cChr = ' ';
/*N*/ 	if( nV )
/*N*/ 		rStrm >> cChr;
/*N*/ 	sal_Unicode c = ByteString::ConvertToUnicode( cChr, rStrm.GetStreamCharSet() );
/*N*/ 	return new SwFmtHardBlank( c, FALSE );
/*N*/ }


/*N*/ SvStream& SwFmtHardBlank::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	if( nIVer )
/*N*/ 	{
/*N*/ 		sal_Char c = ByteString::ConvertFromUnicode( GetChar(),
/*N*/ 												 	rStrm.GetStreamCharSet(),
/*N*/ 												 	FALSE );
/*N*/ 		if( !c )
/*N*/ 			c = ' ';	// TODO: unicode: is this sensible?
/*N*/ 		rStrm << c;
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwFmtHardBlank::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtHardBlank: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? 0 : 1;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtSoftHyph::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	return new SwFmtSoftHyph;
/*N*/ }


/*N*/ SvStream& SwFmtSoftHyph::Store( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	return rStrm;
/*N*/ }

/////////////////////////////// Absatz-Attribute ///////////////////////////


/*N*/ USHORT SwFmtDrop::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtDrop: Gibt es ein neues Fileformat?" );
/*N*/ 	return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			 SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : DROP_WHOLEWORD;
/*N*/ 	return 0;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtDrop::Create( SvStream& rStrm, USHORT nVer ) const
/*N*/ {
/*N*/ 	UINT16 nLines, nChars, nDistance, nX, nY, nFmt;
/*N*/ 	BYTE bWhole = 0;
/*N*/ 	rStrm >> nFmt >> nLines >> nChars >> nDistance;
/*N*/ 	if( nVer >= DROP_WHOLEWORD )
/*N*/ 		rStrm >> bWhole;
/*N*/ 	else
/*N*/ 		rStrm >> nX >> nY;
/*N*/ 	SwFmtDrop* pAttr = new SwFmtDrop;
/*N*/ 	pAttr->GetLines() = (BYTE) nLines;
/*N*/ 	pAttr->GetChars() = (BYTE) nChars;
/*N*/ 	pAttr->GetDistance() = nDistance;
/*N*/ 	pAttr->GetWholeWord() = (BOOL) bWhole;
/*N*/ 	if( nFmt != IDX_NO_VALUE )
/*N*/ 	{
/*N*/ 		Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 		if( pIo )
/*N*/ 		{
/*N*/ 			if( pIo->aStringPool.Count() )
/*N*/ 			{
/*N*/ 				SwCharFmt* pSet = (SwCharFmt*) pIo->FindFmt( nFmt, SWG_CHARFMT );
/*N*/ 				if( pSet )
/*N*/ 					pAttr->SetCharFmt( pSet );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pAttr->nReadFmt = nFmt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pAttr;
/*N*/ }


/*N*/ SvStream& SwFmtDrop::Store( SvStream& rStrm, USHORT nVer ) const
/*N*/ {
/*N*/ 	USHORT nFmt = IDX_NO_VALUE;
/*N*/ 	USHORT nChars = GetWholeWord() ? 1 : GetChars();
/*N*/ 	const SwFmt* pFmt = GetCharFmt();
/*N*/ 	if( pFmt )
/*N*/ 	{
/*N*/ 		Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 		if( pIo )
/*N*/ 			nFmt = pIo->aStringPool.Find( pFmt->GetName(),
/*N*/ 										  pFmt->GetPoolFmtId() );
/*N*/ 	}
/*N*/ 	rStrm << (UINT16) nFmt
/*N*/ 		  << (UINT16) GetLines()
/*N*/ 		  << (UINT16) nChars
/*N*/ 		  << (UINT16) GetDistance();
/*N*/ 	if( nVer >= DROP_WHOLEWORD )
/*N*/ 		rStrm << (BYTE) GetWholeWord();
/*N*/ 	else
/*N*/ 		rStrm << (UINT16) 0 << (UINT16) 0;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SwRegisterItem::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	ASSERT( nIVer != USHRT_MAX,
/*N*/ 			"SwRegisterItem: Wer faengt da Version USHRT_MAX nicht ab?" );
/*N*/ 
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SwRegisterItem::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	BYTE bIsRegister;
/*N*/ 	rStrm >> bIsRegister;
/*N*/ 	return new SwRegisterItem( BOOL( bIsRegister != 0 ) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwRegisterItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwRegisterItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	// im 3.1 FF nicht speichern
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SwFmtLineNumber::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	ASSERT( nIVer != USHRT_MAX,
/*N*/ 			"SwFmtLineNumber: Wer faengt da Version USHRT_MAX nicht ab?" );
/*N*/ 
/*N*/ 	rStrm << static_cast<sal_uInt32>(nStartValue) << IsCount();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SwFmtLineNumber::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	SwFmtLineNumber *pTmp = new SwFmtLineNumber;
/*N*/ 	sal_uInt32 nTmp; BOOL bTmp;
/*N*/ 	rStrm >> nTmp; pTmp->SetStartValue( nTmp );
/*N*/ 	rStrm >> bTmp; pTmp->SetCountLines( bTmp );
/*N*/ 	return pTmp;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwFmtLineNumber::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwFmtLineNumber: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	// vor 5.0 nicht speichern
/*N*/ 	return SOFFICE_FILEFORMAT_40 <= nFFVer ? 0 : USHRT_MAX;
/*N*/ }



// -----------------------------------------------------------------------

/*N*/ SvStream& SwNumRuleItem::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	ASSERT( nIVer != USHRT_MAX,
/*N*/ 			"SwNumRuleItem: Wer faengt da Version USHRT_MAX nicht ab?" );
/*N*/ 
/*N*/ 	// Damit wir Pool-NumRules umbenennen koennen, muessen wir die
/*N*/ 	// PoolId der NumRule rausfinden und mit speichern.
/*N*/ 	UINT16 nPoolId = USHRT_MAX;
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 	if( pIo )
/*N*/ 	{
/*N*/ 		const SwNumRule *pNumRule = pIo->pDoc->FindNumRulePtr( GetValue() );
/*N*/ 		if( pNumRule )
/*N*/ 			nPoolId = pNumRule->GetPoolFmtId();
/*N*/ 	}
/*N*/ 
/*N*/ 	rStrm.WriteByteString( GetValue(), rStrm.GetStreamCharSet() ) << nPoolId;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SwNumRuleItem::Create( SvStream& rStrm,
/*N*/ 											 USHORT nIVer ) const
/*N*/ {
/*N*/ 	String sTmp;
/*N*/ 	UINT16 nPoolId;
/*N*/ 
/*N*/ 	rStrm.ReadByteString( sTmp, rStrm.GetStreamCharSet() );
/*N*/ 	if( nIVer>0 )
/*N*/ 	{
/*N*/ 		// Pool-NumRules muessen evtl. noch umbenannt werden.
/*N*/ 		rStrm >> nPoolId;
/*N*/ 
/*N*/ 		Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 		if( pIo && nPoolId >= RES_POOLNUMRULE_BEGIN &&
/*N*/ 			nPoolId < RES_POOLNUMRULE_END )
/*?*/ 			SwStyleNameMapper::FillUIName( nPoolId, sTmp );
/*N*/ 	}
/*N*/ 
/*N*/ 	return new SwNumRuleItem( sTmp );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwNumRuleItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwNumRuleItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	// im 3.1 und 4.0 FF nicht speichern
/*N*/ 	// Die FF-Version 0 enthielt keine PoolId.
/*N*/ 	return nFFVer <= SOFFICE_FILEFORMAT_40 ? USHRT_MAX : 1;
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ SfxPoolItem* SwTblBoxNumFormat::Create( SvStream& rStrm, USHORT nVer) const
/*N*/ {
/*N*/ 	BYTE nFlag;
/*N*/ 	UINT32 nTmp;
/*N*/ 
/*N*/ 	rStrm >> nTmp >> nFlag;
/*N*/ 
/*N*/ 	Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/ 	SvNumberFormatter* pN;
/*N*/ 	if( pIo && 0 != ( pN = pIo->pDoc->GetNumberFormatter( FALSE ) ) )
/*N*/ 		nTmp = pN->GetMergeFmtIndex( nTmp );
/*N*/ 
/*N*/ 	SwTblBoxNumFormat* pNew = new SwTblBoxNumFormat( nTmp, 0 != nFlag );
/*N*/ 	return pNew;
/*N*/ }


/*N*/ SvStream& SwTblBoxNumFormat::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	rStrm << GetValue() << (BYTE)bAuto;
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwTblBoxNumFormat::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwTblBoxNumFormat: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ SfxPoolItem* SwTblBoxValue::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	double dVal;
/*N*/ 	if( 0 == nIVer )
/*N*/ 	{
/*N*/ 		sal_Char* dummy;
/*N*/ 		ByteString sValue;
/*N*/ 		rStrm.ReadByteString( sValue );
/*N*/ 		dVal = strtod( sValue.GetBuffer(), &dummy );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm >> dVal;
/*N*/ 
/*N*/ 	return new SwTblBoxValue( dVal );
/*N*/ }


/*N*/ SvStream& SwTblBoxValue::Store( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	if( 0 == nIVer )
/*N*/ 	{
/*?*/         ByteString sValue(ByteString::CreateFromDouble(nValue));
/*?*/ 		rStrm.WriteByteString( sValue );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm << nValue;
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }


/*N*/ USHORT SwTblBoxValue::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SwTblBoxValue: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX :
/*N*/ 		   (SOFFICE_FILEFORMAT_40==nFFVer ? 0 : 1 );
/*N*/ }


}
