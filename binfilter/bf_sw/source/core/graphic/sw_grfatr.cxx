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

#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif

#ifndef _BF_GOODIES_GRAPHICOBJECT_HXX
#include <bf_goodies/graphicobject.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif
#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;

/*N*/ TYPEINIT1_AUTOFACTORY(SwCropGrf, SfxPoolItem)
/*N*/ TYPEINIT1_AUTOFACTORY(SwGammaGrf, SfxPoolItem)

/******************************************************************************
 *	Implementierung		class SwMirrorGrf
 ******************************************************************************/

/*N*/ SfxPoolItem* SwMirrorGrf::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwMirrorGrf( *this );
/*N*/ }

sal_uInt16 SwMirrorGrf::GetValueCount() const
{
   return RES_GRFMIRROR_END - RES_GRFMIRROR_BEGIN;
}

/*N*/ int SwMirrorGrf::operator==( const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return SfxEnumItem::operator==(rItem) &&
/*N*/ 			((SwMirrorGrf&)rItem).IsGrfToggle() == IsGrfToggle();
/*N*/ }

/*N*/ BOOL lcl_IsHoriOnEvenPages(int nEnum, BOOL bToggle)
/*N*/ {
/*N*/ 	BOOL bEnum = nEnum == RES_MIRROR_GRF_VERT ||
/*N*/ 				   nEnum == RES_MIRROR_GRF_BOTH;
/*N*/ 			return bEnum != bToggle;
/*N*/ }
/*N*/ BOOL lcl_IsHoriOnOddPages(int nEnum)
/*N*/ {
/*N*/ 	BOOL bEnum = nEnum == RES_MIRROR_GRF_VERT ||
/*N*/ 				   nEnum == RES_MIRROR_GRF_BOTH;
/*N*/ 			return bEnum;
/*N*/ }
/*N*/ BOOL SwMirrorGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_True,
/*N*/ 		 bVal;
/*N*/ 	// Vertikal und Horizontal sind mal getauscht worden!
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_MIRROR_HORZ_EVEN_PAGES:
/*N*/ 			bVal = lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle());
/*N*/ 		break;
/*N*/ 		case MID_MIRROR_HORZ_ODD_PAGES:
/*N*/ 			bVal = lcl_IsHoriOnOddPages(GetValue());
/*N*/ 		break;
/*N*/ 		case MID_MIRROR_VERT:
/*N*/ 			bVal = GetValue() == RES_MIRROR_GRF_HOR ||
/*N*/ 				   GetValue() == RES_MIRROR_GRF_BOTH;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			ASSERT( !this, "unknown MemberId" );
/*N*/ 			bRet = sal_False;
/*N*/ 	}
/*N*/ 	rVal.setValue( &bVal, ::getBooleanCppuType() );
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL SwMirrorGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	sal_Bool bVal = *(sal_Bool*)rVal.getValue();
/*N*/ 	// Vertikal und Horizontal sind mal getauscht worden!
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_MIRROR_HORZ_EVEN_PAGES:
/*N*/ 		case MID_MIRROR_HORZ_ODD_PAGES:
/*N*/ 		{
/*N*/ 			BOOL bIsVert = GetValue() == RES_MIRROR_GRF_HOR ||
/*N*/ 				   				GetValue() == RES_MIRROR_GRF_BOTH;
/*N*/ 			BOOL bOnOddPages = nMemberId == MID_MIRROR_HORZ_EVEN_PAGES ?
/*N*/ 									lcl_IsHoriOnOddPages(GetValue()) : bVal;
/*N*/ 			BOOL bOnEvenPages = nMemberId == MID_MIRROR_HORZ_ODD_PAGES ?
/*N*/ 									   lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle()) : bVal;
/*N*/ 			GRFMIRROR nEnum = bOnOddPages ?
/*N*/ 					bIsVert ? RES_MIRROR_GRF_BOTH : RES_MIRROR_GRF_VERT :
/*N*/ 						bIsVert ? RES_MIRROR_GRF_HOR : RES_DONT_MIRROR_GRF;
/*N*/ 			BOOL bToggle = bOnOddPages != bOnEvenPages;
/*N*/ 			SetValue(nEnum);
/*N*/ 			SetGrfToggle( bToggle );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_MIRROR_VERT:
/*N*/ 			if ( bVal )
/*N*/ 			{
/*?*/ 				if ( GetValue() == RES_MIRROR_GRF_VERT )
/*?*/ 					SetValue( RES_MIRROR_GRF_BOTH );
/*?*/ 				else if ( GetValue() != RES_MIRROR_GRF_BOTH )
/*?*/ 					SetValue( RES_MIRROR_GRF_HOR );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if ( GetValue() == RES_MIRROR_GRF_BOTH )
/*?*/ 					SetValue( RES_MIRROR_GRF_VERT );
/*N*/ 				else if ( GetValue() == RES_MIRROR_GRF_HOR )
/*?*/ 					SetValue( RES_DONT_MIRROR_GRF );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			ASSERT( !this, "unknown MemberId" );
/*N*/ 			bRet = sal_False;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/******************************************************************************
 *	Implementierung		class SwCropGrf
 ******************************************************************************/

/*N*/ SwCropGrf::SwCropGrf()
/*N*/ 	: SvxGrfCrop( RES_GRFATR_CROPGRF )
/*N*/ {}

SwCropGrf::SwCropGrf(sal_Int32 nL, sal_Int32 nR, sal_Int32 nT, sal_Int32 nB )
        : SvxGrfCrop( nL, nR, nT, nB, RES_GRFATR_CROPGRF )
{}

/*N*/ SfxPoolItem* SwCropGrf::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwCropGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwRotationGrf::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SwRotationGrf( GetValue(), aUnrotatedSize );
/*N*/ }


/*N*/ int	SwRotationGrf::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	return SfxUInt16Item::operator==( rCmp ) &&
/*N*/ 		GetUnrotatedSize() == ((SwRotationGrf&)rCmp).GetUnrotatedSize();
/*N*/ }


/*N*/ BOOL SwRotationGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
/*N*/     // where we still want this to be a sal_Int16
/*N*/     rVal <<= (sal_Int16)GetValue();
/*N*/     return TRUE;
/*N*/ }

/*N*/ BOOL SwRotationGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
/*N*/     // where we still want this to be a sal_Int16
/*N*/     sal_Int16 nValue;
/*N*/ 	if (rVal >>= nValue)
/*N*/ 	{
/*N*/         // UINT16 argument needed
/*N*/         SetValue( (UINT16) nValue );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/
/*N*/     DBG_ERROR( "SwRotationGrf::PutValue - Wrong type!" );
/*N*/ 	return FALSE;
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwLuminanceGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwLuminanceGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwContrastGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwContrastGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwChannelRGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwChannelRGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwChannelGGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwChannelGGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwChannelBGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwChannelBGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwGammaGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwGammaGrf( *this );
/*N*/ }

/*N*/ int	SwGammaGrf::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	return SfxPoolItem::operator==( rCmp ) &&
/*N*/ 		nValue == ((SwGammaGrf&)rCmp).GetValue();
/*N*/ }

/*N*/ BOOL SwGammaGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= nValue;
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ BOOL SwGammaGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	return rVal >>= nValue;
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwInvertGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwInvertGrf( *this );
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwTransparencyGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwTransparencyGrf( *this );
/*N*/ }
// ------------------------------------------------------------------
/*N*/ BOOL SwTransparencyGrf::QueryValue( ::com::sun::star::uno::Any& rVal,
/*N*/ 										BYTE nMemberId  ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(ISA(SfxByteItem),"Put/QueryValue should be removed!");
/*N*/ 	sal_Int16 nRet = GetValue();
/*N*/     DBG_ASSERT( 0 <= nRet && nRet <= 100, "value out of range" );
/*N*/     rVal <<= nRet;
/*N*/ 	return TRUE;
/*N*/ }
// ------------------------------------------------------------------
/*N*/ BOOL SwTransparencyGrf::PutValue( const ::com::sun::star::uno::Any& rVal,
/*N*/ 										BYTE nMemberId  )
/*N*/ {
/*N*/ 	//temporary conversion until this is a SfxInt16Item!
/*N*/ 	DBG_ASSERT(ISA(SfxByteItem),"Put/QueryValue should be removed!");
/*N*/ 	sal_Int16 nVal;
/*N*/ 	if(!(rVal >>= nVal) || nVal < -100 || nVal > 100)
/*N*/ 		return FALSE;
/*N*/     if(nVal < 0)
/*N*/     {
/*N*/         // for compatibility with old documents
/*N*/         // OD 05.11.2002 #104308# - introduce rounding as for SO 6.0 PP2
/*N*/         // introduced by fix of #104293#.
/*N*/         nVal = ( ( nVal * 128 ) - (99/2) ) / 100;
/*N*/         nVal += 128;
/*N*/     }
/*N*/     DBG_ASSERT( 0 <= nVal && nVal <= 100, "value out of range" );
/*N*/ 	SetValue(nVal);
/*N*/ 	return TRUE;
/*N*/ }

// ------------------------------------------------------------------

/*N*/ SfxPoolItem* SwDrawModeGrf::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwDrawModeGrf( *this );
/*N*/ }


/*N*/ BOOL SwDrawModeGrf::QueryValue( ::com::sun::star::uno::Any& rVal,
/*N*/ 								BYTE nMemberId ) const
/*N*/ {
/*N*/ 	drawing::ColorMode eRet = (drawing::ColorMode)GetEnumValue();
/*N*/ 	rVal <<= eRet;
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwDrawModeGrf::PutValue( const ::com::sun::star::uno::Any& rVal,
/*N*/ 								BYTE nMemberId  )
/*N*/ {
/*N*/ 	sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
/*N*/ 	if(eVal >= 0 && eVal <= GRAPHICDRAWMODE_WATERMARK)
/*N*/ 	{
/*N*/ 		SetEnumValue((USHORT)eVal);
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }



}
