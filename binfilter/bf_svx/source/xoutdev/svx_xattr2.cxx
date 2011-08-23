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

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif

#include "dialogs.hrc"

#ifndef _XDEF_HXX
#include <bf_svx/xdef.hxx>
#endif

#include "xattr.hxx"

#include "rectenum.hxx"
#include "xflbckit.hxx"
#include "xflbmpit.hxx"
#include "xflbmsli.hxx"
#include "xflbmsxy.hxx"
#include "xflbmtit.hxx"
#include "xflboxy.hxx"
#include "xflbstit.hxx"
#include "xflbtoxy.hxx"
#include "xfltrit.hxx"
#include "xftshtit.hxx"
#include "xgrscit.hxx"
#include "xlinjoit.hxx"
#include "xlntrit.hxx"

namespace binfilter {

#define GLOBALOVERFLOW

/************************************************************************/

//------------------------------
// class XLineTransparenceItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*	  XLineTransparenceItem::XLineTransparenceItem(USHORT)
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ XLineTransparenceItem::XLineTransparenceItem(USHORT nLineTransparence) :
/*N*/ 	SfxUInt16Item(XATTR_LINETRANSPARENCE, nLineTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn) :
/*N*/ 	SfxUInt16Item(XATTR_LINETRANSPARENCE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineTransparenceItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineTransparenceItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineTransparenceItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineTransparenceItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


//-----------------------
// class XLineJointItem	-
//-----------------------

/*N*/ TYPEINIT1_AUTOFACTORY(XLineJointItem, SfxEnumItem);

// -----------------------------------------------------------------------------

/*N*/ XLineJointItem::XLineJointItem( XLineJoint eLineJoint ) :
/*N*/ 	SfxEnumItem(XATTR_LINEJOINT, eLineJoint)
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ XLineJointItem::XLineJointItem( SvStream& rIn ) :
/*N*/ 	SfxEnumItem( XATTR_LINEJOINT, rIn )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ USHORT XLineJointItem::GetVersion( USHORT nFileFormatVersion ) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SfxPoolItem* XLineJointItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	XLineJointItem* pRet = new XLineJointItem( rIn );
/*N*/ 
/*N*/ 	if(nVer < 1)
/*?*/ 		pRet->SetValue(XLINEJOINT_ROUND);
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SfxPoolItem* XLineJointItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineJointItem( *this );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ sal_Bool XLineJointItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	::com::sun::star::drawing::LineJoint eJoint = ::com::sun::star::drawing::LineJoint_NONE;
/*N*/ 
/*N*/ 	switch( GetValue() )
/*N*/ 	{
/*N*/ 	case XLINEJOINT_NONE:
/*N*/ 		break;
/*N*/ 	case XLINEJOINT_MIDDLE:
/*N*/ 		eJoint = ::com::sun::star::drawing::LineJoint_MIDDLE;
/*N*/ 		break;
/*N*/ 	case XLINEJOINT_BEVEL:
/*N*/ 		eJoint = ::com::sun::star::drawing::LineJoint_BEVEL;
/*N*/ 		break;
/*N*/ 	case XLINEJOINT_MITER:
/*N*/ 		eJoint = ::com::sun::star::drawing::LineJoint_MITER;
/*N*/ 		break;
/*N*/ 	case XLINEJOINT_ROUND:
/*N*/ 		eJoint = ::com::sun::star::drawing::LineJoint_ROUND;
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		DBG_ERROR( "Unknown LineJoint enum value!" );
/*N*/ 	}
/*N*/ 
/*N*/ 	rVal <<= eJoint;
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL XLineJointItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	XLineJoint eJoint = XLINEJOINT_NONE;
/*N*/ 	::com::sun::star::drawing::LineJoint eUnoJoint;
/*N*/ 
/*N*/ 	if(!(rVal >>= eUnoJoint))
/*N*/     {
/*?*/         // also try an int (for Basic)
/*?*/         sal_Int32 nLJ;
/*?*/         if(!(rVal >>= nLJ))
/*?*/             return sal_False;
/*?*/         eUnoJoint = (::com::sun::star::drawing::LineJoint)nLJ;
/*N*/     }

/*N*/ 	switch( eUnoJoint )
/*N*/ 	{
/*N*/ 	case ::com::sun::star::drawing::LineJoint_NONE:
/*N*/ 		break;
/*N*/ 	case ::com::sun::star::drawing::LineJoint_MIDDLE:
/*N*/ 		eJoint = XLINEJOINT_MIDDLE;
/*N*/ 		break;
/*N*/ 	case ::com::sun::star::drawing::LineJoint_BEVEL:
/*N*/ 		eJoint = XLINEJOINT_BEVEL;
/*N*/ 		break;
/*N*/ 	case ::com::sun::star::drawing::LineJoint_MITER:
/*N*/ 		eJoint = XLINEJOINT_MITER;
/*N*/ 		break;
/*N*/ 	case ::com::sun::star::drawing::LineJoint_ROUND:
/*N*/ 		eJoint = XLINEJOINT_ROUND;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetValue( eJoint );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ USHORT XLineJointItem::GetValueCount() const
/*N*/ {
/*N*/ 	// don't forget to update the api interface also
/*N*/ 	return 5;
/*N*/ }

//------------------------------
// class XFillTransparenceItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*	  XFillTransparenceItem::XFillTransparenceItem(USHORT)
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ XFillTransparenceItem::XFillTransparenceItem(USHORT nFillTransparence) :
/*N*/ 	SfxUInt16Item(XATTR_FILLTRANSPARENCE, nFillTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn) :
/*N*/ 	SfxUInt16Item(XATTR_FILLTRANSPARENCE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillTransparenceItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillTransparenceItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillTransparenceItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	07.11.95 KA
|*	  Letzte Aenderung	07.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillTransparenceItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


//------------------------------
// class XFormTextShadowTranspItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowTranspItem, SfxUInt16Item);

/*************************************************************************
|*
|*	  XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT)
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.95 KA
|*	  Letzte Aenderung	09.11.95 KA
|*
*************************************************************************/

/*N*/ XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT nShdwTransparence) :
/*N*/ 	SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, nShdwTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.95 KA
|*	  Letzte Aenderung	09.11.95 KA
|*
*************************************************************************/

/*N*/ XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn) :
/*N*/ 	SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowTranspItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.95 KA
|*	  Letzte Aenderung	09.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowTranspItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextShadowTranspItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.95 KA
|*	  Letzte Aenderung	09.11.95 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextShadowTranspItem(rIn);
/*N*/ }


//------------------------------
// class XFillGradientStepCountItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XGradientStepCountItem, SfxUInt16Item);

/*************************************************************************
|*
|*	  XGradientStepCountItem::XGradientStepCountItem( USHORT )
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ XGradientStepCountItem::XGradientStepCountItem( USHORT nStepCount ) :
/*N*/ 	SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, nStepCount )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn )
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn ) :
/*N*/ 	SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, rIn )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XGradientStepCountItem::Clone( SfxItemPool* pPool ) const
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XGradientStepCountItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XGradientStepCountItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XGradientStepCountItem( rIn );
/*N*/ }

//------------------------------------------------------------------------



//------------------------------
// class XFillBmpTileItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileItem::XFillBmpTileItem( BOOL bTile ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_TILE, bTile )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileItem::XFillBmpTileItem( SvStream& rIn ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_TILE, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



//------------------------------
// class XFillBmpTilePosItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosItem, SfxEnumItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpPosItem::XFillBmpPosItem( RECT_POINT eRP ) :
/*N*/ 			SfxEnumItem( XATTR_FILLBMP_POS, eRP )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpPosItem::XFillBmpPosItem( SvStream& rIn ) :
/*N*/ 			SfxEnumItem( XATTR_FILLBMP_POS, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

/*N*/ USHORT XFillBmpPosItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 9;
/*N*/ }


//------------------------------
// class XFillBmpTileSizeXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeXItem, SfxMetricItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeXItem::XFillBmpSizeXItem( long nSizeX ) :
/*N*/ 			SfxMetricItem( XATTR_FILLBMP_SIZEX, nSizeX )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeXItem::XFillBmpSizeXItem( SvStream& rIn ) :
/*N*/ 			SfxMetricItem( XATTR_FILLBMP_SIZEX, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeXItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeXItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeXItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeXItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



/*************************************************************************
|*
|*	  Beschreibung
|*	  Ersterstellung	05.11.96 KA
|*	  Letzte Aenderung	05.11.96 KA
|*
\*************************************************************************/



//------------------------------
// class XFillBmpTileSizeYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeYItem, SfxMetricItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeYItem::XFillBmpSizeYItem( long nSizeY ) :
/*N*/ 			SfxMetricItem( XATTR_FILLBMP_SIZEY, nSizeY )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeYItem::XFillBmpSizeYItem( SvStream& rIn ) :
/*N*/ 			SfxMetricItem( XATTR_FILLBMP_SIZEY, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeYItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeYItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeYItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeYItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



/*************************************************************************
|*
|*	  Beschreibung
|*	  Ersterstellung	05.11.96 KA
|*	  Letzte Aenderung	05.11.96 KA
|*
\*************************************************************************/



//------------------------------
// class XFillBmpTileLogItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeLogItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeLogItem::XFillBmpSizeLogItem( BOOL bLog ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_SIZELOG, bLog )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpSizeLogItem::XFillBmpSizeLogItem( SvStream& rIn ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_SIZELOG, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeLogItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeLogItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpSizeLogItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpSizeLogItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



//------------------------------
// class XFillBmpTileOffXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetXItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( USHORT nOffX ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, nOffX )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( SvStream& rIn ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileOffsetXItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileOffsetXItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileOffsetXItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileOffsetXItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



//------------------------------
// class XFillBmpTileOffYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetYItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( USHORT nOffY ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, nOffY )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( SvStream& rIn ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileOffsetYItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileOffsetYItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpTileOffsetYItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpTileOffsetYItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



//------------------------------
// class XFillBmpStretchItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpStretchItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpStretchItem::XFillBmpStretchItem( BOOL bStretch ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_STRETCH, bStretch )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ XFillBmpStretchItem::XFillBmpStretchItem( SvStream& rIn ) :
/*N*/ 			SfxBoolItem( XATTR_FILLBMP_STRETCH, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpStretchItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpStretchItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpStretchItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpStretchItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	28.02.96 KA
|*	  Letzte Aenderung	28.02.96 KA
|*
*************************************************************************/



//------------------------------
// class XFillBmpTileOffPosXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetXItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( USHORT nOffPosX ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, nOffPosX )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( SvStream& rIn ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosOffsetXItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosOffsetXItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosOffsetXItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosOffsetXItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/



//------------------------------
// class XFillBmpTileOffPosYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetYItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( USHORT nOffPosY ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, nOffPosY )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( SvStream& rIn ) :
/*N*/ 			SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, rIn )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosOffsetYItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosOffsetYItem( *this );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBmpPosOffsetYItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new XFillBmpPosOffsetYItem( rIn );
/*N*/ }


/*************************************************************************
|*
|*
|*
|*	  Beschreibung
|*	  Ersterstellung	KA 29.04.96
|*	  Letzte Aenderung	KA 29.04.96
|*
*************************************************************************/


//--------------------------
// class XFillBackgroundItem
//--------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillBackgroundItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XFillBackgroundItem::XFillBackgroundItem( BOOL )
|*
|*	  Beschreibung
|*	  Ersterstellung	19.11.96 KA
|*	  Letzte Aenderung
|*
*************************************************************************/

/*N*/ XFillBackgroundItem::XFillBackgroundItem( BOOL bFill ) :
/*N*/ 	SfxBoolItem( XATTR_FILLBACKGROUND, bFill )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn )
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn ) :
/*N*/ 	SfxBoolItem( XATTR_FILLBACKGROUND, rIn )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillBackgroundItem::Clone( SfxItemPool* pPool ) const
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBackgroundItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillBackgroundItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	23.01.96 KA
|*	  Letzte Aenderung	23.01.96 KA
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillBackgroundItem( rIn );
/*N*/ }

}
