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

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _XOUTX_HXX //autogen
#include <bf_svx/xoutx.hxx>
#endif

#include "xchar.hxx"
#include "types.hxx"

#include "smdll.hxx"
#include "starmath.hrc"
namespace binfilter {









////////////////////////////////////////







////////////////////////////////////////


/*?*/ class SmPolygonLoader : public Resource
/*?*/ {
/*?*/ public:
/*?*/ 	SmPolygonLoader(const SmResId &rId, SmPolygon &rSmPolygon);
/*?*/ };


/*N*/ SmPolygonLoader::SmPolygonLoader(const SmResId& rId, SmPolygon &rSmPolygon)
/*N*/ :	Resource (rId)
/*N*/ {
/*N*/ 	USHORT	nBytesLeft;		// upper size limit to a single resource is 64 kB!
/*N*/
/*N*/ 	// set cursor to begin of resource
/*N*/ 	//GetClassRes();	// it's done automatically!
/*N*/
/*N*/ 	// get number of bytes from actual position to end of resource
/*N*/ 	nBytesLeft = GetRemainSizeRes();
/*N*/
/*N*/ 	char *pStr = (char *) GetClassRes();
/*N*/ 	SvMemoryStream	aStrm(pStr, nBytesLeft, STREAM_READ);
/*N*/
/*N*/ 	aStrm >> rSmPolygon.cChar
/*N*/ 		  >> rSmPolygon.aFontSize
/*N*/ 		  >> rSmPolygon.aOrigPos
/*N*/ 		  >> rSmPolygon.aOrigSize
/*N*/ 		  >> rSmPolygon.aPoly;
/*N*/
/*N*/ 	//! Warning: don't know why, but it has to be done!
/*N*/ 	IncrementRes(nBytesLeft);
/*N*/ }


////////////////////////////////////////
// SmPolygon
//

/*N*/ SmPolygon::SmPolygon()
/*N*/ {
/*N*/ 	cChar = sal_Char('\x00'),
/*N*/ 	fScaleX = fScaleY =
/*N*/ 	fDelayedFactorX = fDelayedFactorY = 1.0;
/*N*/ 	bDelayedScale = bDelayedBoundRect = FALSE;
/*N*/ }


/*N*/ SmPolygon::SmPolygon(sal_Unicode cCharP)
/*N*/ {
/*N*/ 	cChar = cCharP;
/*N*/ 	fScaleX = fScaleY =
/*N*/ 	fDelayedFactorX = fDelayedFactorY = 1.0;
/*N*/ 	bDelayedScale = bDelayedBoundRect = FALSE;
/*N*/
/*N*/ 	if (cChar == sal_Char('\0'))
/*N*/ 		return;
/*N*/
/*N*/ 	// get appropriate resource id
/*N*/ 	int  nResId = 0;
/*N*/     switch (cChar)
/*N*/ 	{
/*N*/ 		case MS_LINE : 			nResId = RID_XPP_LINE; 			break;
/*N*/ 		case MS_DLINE : 		nResId = RID_XPP_DLINE; 		break;
/*N*/ 		case MS_SQRT : 			nResId = RID_XPP_SQRT; 			break;
/*N*/ 		case MS_SQRT2 : 		nResId = RID_XPP_SQRT2; 		break;
/*N*/ 		case MS_HAT : 			nResId = RID_XPP_HAT; 			break;
/*N*/ 		case MS_TILDE : 		nResId = RID_XPP_TILDE; 		break;
/*N*/ 		case MS_BAR : 			nResId = RID_XPP_BAR; 			break;
/*N*/ 		case MS_VEC : 			nResId = RID_XPP_VEC; 			break;
/*N*/ 		case MS_LBRACE : 		nResId = RID_XPP_LBRACE; 		break;
/*N*/ 		case MS_RBRACE : 		nResId = RID_XPP_RBRACE; 		break;
/*N*/ 		case MS_LPARENT : 		nResId = RID_XPP_LPARENT; 		break;
/*N*/ 		case MS_RPARENT : 		nResId = RID_XPP_RPARENT; 		break;
/*N*/ 		case MS_LANGLE : 		nResId = RID_XPP_LANGLE; 		break;
/*N*/ 		case MS_RANGLE : 		nResId = RID_XPP_RANGLE; 		break;
/*N*/ 		case MS_LBRACKET : 		nResId = RID_XPP_LBRACKET; 		break;
/*N*/ 		case MS_RBRACKET : 		nResId = RID_XPP_RBRACKET; 		break;
/*N*/ 		case MS_LDBRACKET : 	nResId = RID_XPP_LDBRACKET; 	break;
/*N*/ 		case MS_RDBRACKET : 	nResId = RID_XPP_RDBRACKET; 	break;
/*N*/ 		case MS_LCEIL : 		nResId = RID_XPP_LCEIL; 		break;
/*N*/ 		case MS_RCEIL : 		nResId = RID_XPP_RCEIL; 		break;
/*N*/ 		case MS_LFLOOR : 		nResId = RID_XPP_LFLOOR; 		break;
/*N*/ 		case MS_RFLOOR : 		nResId = RID_XPP_RFLOOR; 		break;
/*N*/ 		case MS_OVERBRACE : 	nResId = RID_XPP_OVERBRACE;		break;
/*N*/ 		case MS_UNDERBRACE :	nResId = RID_XPP_UNDERBRACE;	break;
/*N*/
/*N*/ 		default :
/*N*/ 			DBG_ASSERT(0, "Sm: char hat kein Polygon");
/*N*/ 	}
/*N*/
/*N*/ 	if (nResId)
/*N*/ 	{
/*N*/ 		// SmPolygon (XPolyPolygon, ...) aus der Resource laden
/*N*/ 		SmResId aSmResId(nResId);
/*N*/ 		SmPolygonLoader(aSmResId, *this);
/*N*/
/*N*/ 		// die verbleibenden member Variablen setzen
/*N*/ 		aBoundRect = aPoly.GetBoundRect();
/*N*/ 		aPos	   = GetOrigPos();
/*N*/
/*N*/       // jetzt nach (0, 0) verschieben verbessert die Chancen, dass in Scale()
/*N*/       // (welches ia oefter aufgerufen wird) nicht das MoveTo ausgefuehrt
/*N*/       // werden muss
/*N*/ 		MoveTo(Point());
/*N*/ 	}
/*N*/ }


/*N*/ void SmPolygon::Scale()
/*N*/ {
/*N*/ 	DBG_ASSERT(bDelayedScale, "Sm: es gibt nichts zu skalieren");
/*N*/
/*N*/ 	Point aOrigin,
/*N*/ 		  aDelta;
/*N*/
/*N*/ 	if (aPos != aOrigin)
/*N*/ 	{
/*?*/ 		aDelta = aOrigin - aPos;
/*?*/ 		aPoly.Move(aDelta.X(), aDelta.Y());
/*N*/ 	}
/*N*/
/*N*/ 	aPoly.Scale(fDelayedFactorX, fDelayedFactorY);
/*N*/ 	fScaleX *= fDelayedFactorX;
/*N*/ 	fScaleY *= fDelayedFactorY;
/*N*/
/*N*/ 	bDelayedScale = FALSE;
/*N*/ 	fDelayedFactorX = fDelayedFactorY = 1.0;
/*N*/
/*N*/ 	// Anm.: aBoundRect stimmt hier immer noch nicht!
/*N*/   // Das passiert erst wenn es benoetigt wird.
/*N*/
/*N*/ 	// ggf Ausgangsposition wiederherstellen
/*N*/ 	if (aPos != aOrigin)
/*?*/ 		aPoly.Move(-aDelta.X(), -aDelta.Y());
/*?*/
/*N*/ }


/*N*/ void SmPolygon::ScaleBy(double fFactorX, double fFactorY)
/*N*/ {
/*N*/ 	if (fFactorX != 1.0  ||  fFactorY != 1.0)
/*N*/ 	{
/*N*/ 		fDelayedFactorX *= fFactorX;
/*N*/ 		fDelayedFactorY *= fFactorY;
/*N*/
/*N*/ 		bDelayedScale = TRUE;
/*N*/ 		bDelayedBoundRect = TRUE;
/*N*/ 	}
/*N*/ }


/*N*/ void SmPolygon::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
/*N*/ {
/*N*/ 	DBG_ASSERT(aOrigSize.Width() != 0, "Sm: Polygon hat keine Breite");
/*N*/ 	if (aOrigSize.Width() != 0)
/*N*/ 	{
/*N*/ 		double  fFactor = 1.0 / GetScaleX() * nWidth / aOrigSize.Width();
/*N*/ 		ScaleBy(fFactor, 1.0);
/*N*/ 	}
/*N*/ }


/*N*/ void SmPolygon::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
/*N*/ {
/*N*/   DBG_ASSERT(aOrigSize.Height() != 0, "Sm: Polygon hat keine Hoehe");
/*N*/ 	if (aOrigSize.Height() != 0)
/*N*/ 	{
/*N*/ 		double  fFactor = 1.0 / GetScaleY() * nHeight / aOrigSize.Height();
/*N*/ 		ScaleBy(1.0, fFactor);
/*N*/ 	}
/*N*/ }


/*N*/ void SmPolygon::Move(const Point &rPoint)
/*N*/ {
/*N*/ 	long  nX = rPoint.X(),
/*N*/ 		  nY = rPoint.Y();
/*N*/
/*N*/ 	aPoly     .Move(nX, nY);
/*N*/ 	aBoundRect.Move(nX, nY);
/*N*/ 	aPos      .Move(nX, nY);
/*N*/ }




/*N*/ const Rectangle & SmPolygon::GetBoundRect(const OutputDevice &rDev) const
/*N*/ {
/*N*/ 	SmPolygon *pNCthis = ((SmPolygon *) this);
/*N*/
/*N*/ 	if (bDelayedScale)
/*N*/ 		pNCthis->Scale();
/*N*/ 	if (bDelayedBoundRect)
/*N*/ 	{
/*N*/ 		pNCthis->aBoundRect = aPoly.GetBoundRect((OutputDevice *) &rDev);
/*N*/ 		pNCthis->bDelayedBoundRect = FALSE;
/*N*/ 	}
/*N*/ 	return aBoundRect;
/*N*/ }




}
