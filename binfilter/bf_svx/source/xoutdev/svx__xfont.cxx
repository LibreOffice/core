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

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <math.h>

#ifndef _XDEF_HXX
#include <bf_svx/xdef.hxx>
#endif

#include "xoutx.hxx"

// #101498#
#ifndef _OUTLINER_HXX
#include "outliner.hxx"
#endif


#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif


#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

/*************************************************************************
|*
|*	  XOutGetCharOutline()
|*
|*	  Ein Zeichen eines Outlinefonts in ein Bezier-PolyPolygon umwandeln
|*	  Wenn keine Umwandlung moeglich ist, wird ein leeres PolyPolygon
|*	  zurueckgegeben
|*	  Ersterstellung	12.01.95 ESO
|*	  Letzte Aenderung	09.05.95 ESO
|*
*************************************************************************/

// #102382# Remove XOutGetCharOutline
// #100318# deprecated, use XOutGetTextOutline
//XPolyPolygon XOutGetCharOutline( USHORT nChar, OutputDevice& rOut, BOOL bOptimizeSize )
//{
//	PolyPolygon aPolyPoly;
//
//	//if( !rOut.GetGlyphOutline( (xub_Unicode) nChar, aPolyPoly, bOptimizeSize ) && rOut.GetOutDevType() == OUTDEV_PRINTER )
//	// #97492#
//	String aGlyphString((xub_Unicode) nChar);
//
//	if( !rOut.GetTextOutline(aPolyPoly, aGlyphString) && OUTDEV_PRINTER == rOut.GetOutDevType())
//	{
//		VirtualDevice aVDev;
//		aVDev.SetMapMode( rOut.GetMapMode() );
//		aVDev.SetFont( rOut.GetFont() );
//		aVDev.SetTextAlign( rOut.GetTextAlign() );
//		// #97492#
//		//aVDev.GetGlyphOutline( (xub_Unicode) nChar, aPolyPoly, FALSE );
//		aVDev.GetTextOutline(aPolyPoly, aGlyphString);
//	}
//
//	// #97492# since GetTextOutline(...) is base line oriented, the
//	// polygon needs to be moved one line height
//	aPolyPoly.Move(0, rOut.GetFontMetric().GetAscent());
//
//	return XPolyPolygon( aPolyPoly );
//}

// #100318# new for XOutGetCharOutline
// #102382# new interface for XOutGetTextOutline to support PolyPolyVector
/*N*/ sal_Bool XOutGetTextOutlines(PolyPolyVector& rPolyPolyVector, const String& rText, OutputDevice& rOut, 
/*N*/ 	xub_StrLen nBase, xub_StrLen nIndex, xub_StrLen nLen)
/*N*/ {
/*N*/ 	// #102382# use GetTextOutlines and PolyPolyVector now
/*N*/ 	if(!rOut.GetTextOutlines(rPolyPolyVector, rText, nBase, nIndex, nLen) && OUTDEV_PRINTER == rOut.GetOutDevType())
/*N*/ 	{
/*?*/ 		VirtualDevice aVDev;
/*?*/ 
/*?*/ 		aVDev.SetMapMode(rOut.GetMapMode());
/*?*/ 		aVDev.SetFont(rOut.GetFont());
/*?*/ 		aVDev.SetTextAlign(rOut.GetTextAlign());
/*?*/ 
/*?*/ 		// #102382# use GetTextOutlines and PolyPolyVector now
/*?*/ 		aVDev.GetTextOutlines(rPolyPolyVector, rText, nBase, nIndex, nLen);
/*N*/ 	}

    // since GetTextOutline(...) is base line oriented, the
    // polygon needs to be moved one line height
    // #102382# now a loop is necessary for moving all contained single PolyPolygons
/*N*/ 	for(sal_uInt32 a(0); a < rPolyPolyVector.size(); a++)
/*N*/ 	{
/*N*/ 		rPolyPolyVector[a].Move(0, rOut.GetFontMetric().GetAscent());
/*N*/ 	}
/*N*/ 
/*N*/ 	return (rPolyPolyVector.size() != 0);
/*N*/ }

/*************************************************************************
|*
|*	  XOutputDevice::ImpDrawFormText(String, Polygon, Font, nAbsStart)
|*
|*	  Einen String entlang eines Polygons ausgeben; nAbsStart
|*	  ueberschreibt den XFormTextStartItem-Wert und wird fuer die
|*	  Ausgabe mehrerer Strings entlang des gleichen Polygons benoetigt.
|*	  Rueckgabewert ist die Endposition des ausgegebenen Textes in
|*	  Bezug auf den Linienanfang.
|*
|*	  nAbsStart <  0: Die Gesamtlaenge aller Strings, notwendig fuer
|*					  alle Formatierungen ausser Linksbuendig
|*				<= 0: Item-Startwert verwenden
|*				>  0: Text an dieser absoluten Position ausgeben; ist
|*					  normalerweise der Rueckgabewert eines vorigen
|*					  DrawFormText-Aufrufs
|*
|*	  bToLastPoint: alle Linien einschliesslich der letzten Zeichnen,
|*					sonst die letzte Linie auslassen
|*
|*	  bDraw: wenn FALSE, wird nichts ausgegeben, sondern nur das BoundRect
|*			 berechnet
|*
|*	  pDXArray: wenn vorhanden, enthaelt dieses Array die horizontalen
|*				Positionen der einzelnen Zeichen, beginnend beim ersten
|*				und endend hinter dem letzten Zeichen; es muß also
|*				rText.Len()-1 long-Werte enthalten
|*
|*
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	11.10.95 ESO
|*
*************************************************************************/

// #101498# Helper method ImpGetTextLength to get text length taking into account
// hor/vertical and BIDI, especially right-to-left.
/*N*/ sal_Int32 ImpGetTextLength(OutputDevice* pOut, DrawPortionInfo* pInfo, sal_Int32* pDXArray, 
/*N*/ 	xub_StrLen nIndex, xub_StrLen nLen)
/*N*/ {
/*N*/ 	sal_Bool bRightToLeft = pInfo->IsRTL();
/*N*/ 	sal_Int32 nRetval(0L);
/*N*/ 
/*N*/ 	if(pInfo->rFont.IsVertical())
/*N*/ 	{
/*?*/ 		nRetval = pOut->GetTextHeight() * (sal_Int32)nLen;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(bRightToLeft)
/*N*/ 		{
/*?*/ 			nRetval = pOut->GetTextArray(pInfo->rText, pDXArray, 
/*?*/ 				pInfo->nTextStart + (pInfo->nTextLen - (nIndex + nLen)), 
/*?*/ 				nLen);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nRetval = pOut->GetTextArray(pInfo->rText, pDXArray, 
/*N*/ 				pInfo->nTextStart + nIndex, 
/*N*/ 				nLen);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nRetval;
/*N*/ }

// #101498# Helper method ImpDrawTextArray to draw text taking into account
// hor/vertical and BIDI, especially right-to-left.

// #101498# Helper method ImpXOutGetTextOutline to get text outline taking into account
// hor/vertical and BIDI, especially right-to-left.
// #102382# new interface for XOutGetTextOutline to support PolyPolyVector
/*N*/ sal_Bool ImpXOutGetTextOutlines(PolyPolyVector& rPolyPolyVector, OutputDevice* pOut, DrawPortionInfo* pInfo, 
/*N*/ 	xub_StrLen nIndex, xub_StrLen nLen)
/*N*/ {
/*N*/ 	sal_Bool bRightToLeft(pInfo->IsRTL());
/*N*/ 
/*N*/ 	if(bRightToLeft)
/*N*/ 	{
/*?*/ 		return XOutGetTextOutlines(rPolyPolyVector, pInfo->rText, *pOut, 
/*?*/ 			pInfo->nTextStart + (pInfo->nTextLen - (nIndex + nLen)), 
/*?*/ 			pInfo->nTextStart + (pInfo->nTextLen - (nIndex + nLen)), 
/*?*/ 			nLen);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		return XOutGetTextOutlines(rPolyPolyVector, pInfo->rText, *pOut, 
/*N*/ 			pInfo->nTextStart + nIndex, 
/*N*/ 			pInfo->nTextStart + nIndex, 
/*N*/ 			nLen);
/*N*/ 	}
/*N*/ }

// #101498# changed interface due to bidi requirements
/*N*/ sal_Int32 XOutputDevice::ImpDrawFormText(DrawPortionInfo* pInfo, const Polygon& rPoly, sal_Int32 nAbsStart, 
/*N*/ 	sal_Bool bToLastPoint, sal_Bool bDraw, sal_Bool bIsShadow)
/*N*/ {
/*N*/ 	Font aFont = pInfo->rFont;
/*N*/ 	const sal_Int32* pDXArray = pInfo->pDXArray;
/*N*/ 	sal_Bool bUseBreakIterator(sal_False);
/*N*/ 
/*N*/ 	Reference < ::com::sun::star::i18n::XBreakIterator > xBreak;
/*N*/ 	Reference < XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	Reference < XInterface > xInterface = xMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator"));
/*N*/ 	::com::sun::star::lang::Locale aFontLocale = SvxCreateLocale(pInfo->rFont.GetLanguage());
/*N*/ 	
/*N*/ 	if(xInterface.is())
/*N*/ 	{
/*N*/ 		Any x = xInterface->queryInterface(::getCppuType((const Reference< XBreakIterator >*)0));
/*N*/ 		x >>= xBreak;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(xBreak.is())
/*N*/ 	{
/*N*/ 		bUseBreakIterator = sal_True;
/*N*/ 	}
/*N*/ 
/*N*/ 	long	nXMin = LONG_MAX, nYMin = LONG_MAX;
/*N*/ 	long	nXMax = LONG_MIN, nYMax = LONG_MIN;
/*N*/ 	long	nMaxCharExtent;
/*N*/ 	long	nPolyLen = 0;
/*N*/ 	long	nTextLen = 0;
/*N*/ 	long	nTotal = 0;
/*N*/ 	long	nTextWidth;
/*N*/ 	long	nStart;
/*N*/ 	long	nShadowX;
/*N*/ 	USHORT	nPntCnt = rPoly.GetSize();
/*N*/ 	USHORT	nLastPnt;
/*N*/ 	USHORT	nPnt;
/*N*/ 	xub_StrLen nChar(0);
/*N*/ 	const xub_StrLen nCharCnt(pInfo->nTextLen);
/*N*/ 	short	nDirection;

/*N*/ 	if ( nPntCnt < 2 || !nCharCnt)
/*N*/ 		return 0;

/*N*/ 	BOOL bIsSlantShadow = ( bIsShadow && eFormTextShadow == XFTSHADOW_SLANT );
/*N*/ 	BOOL bDrawAsPoly = ( bFormTextOutline || bIsSlantShadow );

/*N*/ 	if ( nAbsStart > 0 )	nStart = nAbsStart;
/*N*/ 	else					nStart = nFormTextStart;

/*N*/ 	aFont.SetTransparent(TRUE);

    // Attribute sichern
/*N*/ 	XLineStyle eOldLineStyle = eLineStyle;
/*N*/ 	XFillStyle eOldFillStyle = eFillStyle;

/*N*/ 	Color aOldLineColor( pOut->GetLineColor() );

/*N*/ 	pOut->SetFillColor( aFont.GetColor() );

/*N*/ 	if ( !(bFormTextOutline && bHair) || bIsShadow )
/*N*/ 		pOut->SetLineColor();
/*N*/ 
/*N*/ 	if ( !bFormTextOutline || bIsShadow )
/*N*/ 		eLineStyle = XLINE_NONE;
/*N*/ 
/*N*/ 	eFillStyle = XFILL_SOLID;
/*N*/ 
/*N*/ 	if ( eFormTextAdjust == XFT_AUTOSIZE && nAbsStart > 0 )
/*?*/ 		aFont.SetSize(Size(0, nFtAutoHeightSave));
/*N*/ 
/*N*/ 	Font aOldFont = pOut->GetFont();
/*N*/ 	pOut->SetFont(aFont);
/*N*/ 	long nAscent = pOut->GetFontMetric().GetAscent();
/*N*/ 
/*N*/ 	// #78478# outline text in FontWork
/*N*/ 	if(bFormTextOutline)
/*N*/ 	{
/*?*/ 		eLineStyle = XLINE_SOLID;
/*?*/ 		pOut->SetLineColor(aLineColor);
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( eFormTextAdjust == XFT_AUTOSIZE )
/*N*/ 	{
/*N*/ 		// bei Autosize den Arrayinhalt immer ignorieren
/*N*/ 		pDXArray = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	nTextWidth = ImpGetTextLength(pOut, pInfo, const_cast<sal_Int32*>(pDXArray), 0, pInfo->nTextLen);
/*N*/ 
/*N*/ 	if ( eFormTextAdjust != XFT_LEFT && nAbsStart <= 0 )
/*N*/ 	{
/*N*/ 		// Gesamtlaenge des Polygons berechnen
/*N*/ 		USHORT nMax = nPntCnt;
/*N*/ 		nPnt = 1;
/*N*/ 
/*N*/ 		if ( !bToLastPoint )
/*N*/ 			nMax--;
/*N*/ 
/*N*/ 		for ( ; nPnt < nMax; nPnt++)
/*N*/ 		{
/*N*/ 			double	fDx = rPoly[nPnt].X() - rPoly[nPnt-1].X();
/*N*/ 			double	fDy = rPoly[nPnt].Y() - rPoly[nPnt-1].Y();
/*N*/ 			nPolyLen += (long) (sqrt(fDx * fDx + fDy * fDy) + 0.5);
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( nAbsStart == 0 )
/*N*/ 			nAbsStart = - nTextWidth;
/*N*/ 
/*N*/ 		if ( eFormTextAdjust == XFT_AUTOSIZE )
/*N*/ 		{
/*N*/ 			nFtAutoHeightSave = aFont.GetSize().Height();
/*N*/ 			nFtAutoHeightSave = - (nPolyLen * nFtAutoHeightSave / nAbsStart);
/*N*/ 			aFont.SetSize(Size(0, nFtAutoHeightSave));
/*N*/ 			pOut->SetFont(aFont);
/*N*/ 			nFtAutoHeightSave = aFont.GetSize().Height();
/*N*/ 			nTextWidth = pOut->GetTextWidth(pInfo->rText, pInfo->nTextStart, pInfo->nTextLen);
/*N*/ 			nAscent = pOut->GetFontMetric().GetAscent();
/*N*/ 			nAbsStart = - nPolyLen;
/*N*/ 		}
/*N*/ 		// Bei rechtsbuendiger Ausgabe auch Startposition beruecksichtigen
/*N*/ 		if ( eFormTextAdjust == XFT_RIGHT )
/*N*/ 			nAbsStart -= nStart;
/*N*/ 		nStart = nPolyLen + nAbsStart;
/*N*/ 
/*N*/ 		if ( eFormTextAdjust != XFT_RIGHT )
/*N*/ 			nStart /= 2;
/*N*/ 	}
/*N*/ 	if ( nStart < 0 )
/*N*/ 		nStart = 0;
/*N*/ 
/*N*/ 	if ( bIsShadow && eFormTextShadow != XFTSHADOW_SLANT )
/*N*/ 		nShadowX = nFormTextShdwXVal;
/*N*/ 	else
/*N*/ 		nShadowX = 0;
/*N*/ 
/*N*/ 	nTotal = nStart + nTextWidth;
/*N*/ 
/*N*/ 	if ( bIsShadow && eFormTextShadow == XFTSHADOW_SLANT &&
/*N*/ 		 nFormTextShdwYVal && nFormTextShdwYVal != 100 )
/*N*/ 		nAscent = nAscent * nFormTextShdwYVal / 100;
/*N*/ 
/*N*/ 	// Maximaler Zeichenbereich ca. 1,4 (sqrt(2)) * Ascent fuer BoundRect
/*N*/ 	nMaxCharExtent = nAscent * 7 / 5;
/*N*/ 
/*N*/ 	// Laufrichtung des Polygons ggf. spiegeln
/*N*/ 	if ( bFormTextMirror )
/*N*/ 	{
/*N*/ 		nDirection = -1;
/*N*/ 		nLastPnt = (USHORT) -1;
/*N*/ 		nPnt = nPntCnt - 2;
/*N*/ 		if ( !bToLastPoint )
/*N*/ 			nPnt--;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nDirection = 1;
/*N*/ 		nLastPnt = nPntCnt - 1;
/*N*/ 		nPnt = 1;
/*N*/ 		if ( bToLastPoint )
/*N*/ 			nLastPnt++;
/*N*/ 	}
/*N*/ 
/*N*/ 	while ( nChar < nCharCnt && nPnt != nLastPnt )
/*N*/ 	{
/*N*/ 		xub_StrLen nGlyphLen(1);
/*N*/ 
/*N*/ 		if(bUseBreakIterator)
/*N*/ 		{
/*N*/ 			sal_Int32 nDone(0L);
/*N*/ 			nGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->rText, nChar + pInfo->nTextStart, aFontLocale, 
/*N*/ 				CharacterIteratorMode::SKIPCELL, 1, nDone) - (nChar + pInfo->nTextStart);
/*N*/ 		}
/*N*/ 
/*N*/ 		Point	aPos = rPoly[nPnt];
/*N*/ 		double	fDx, fDy, fLen;
/*N*/ 		long	nLen, nChar1Len;
/*N*/ 
/*N*/ 		fDx = aPos.X() - rPoly[nPnt-nDirection].X();
/*N*/ 		fDy = aPos.Y() - rPoly[nPnt-nDirection].Y();
/*N*/ 		fLen = sqrt(fDx * fDx + fDy * fDy);
/*N*/ 		nLen = (long) (fLen + 0.5);
/*N*/ 
/*N*/ 		nChar1Len = ImpGetTextLength(pOut, pInfo, const_cast<sal_Int32*>(pDXArray), nChar, nGlyphLen);
/*N*/ 
/*N*/ 		double fXDist, fYDist;
/*N*/ 		double fCos = fDx;
/*N*/ 		double fSin = fDy;
/*N*/ 		double fPartLen = fLen;
/*N*/ 		double fLenSum = fLen - nTextLen;
/*N*/ 		USHORT nSumPnt = nPnt + nDirection;
/*N*/ 
/*N*/ 		// Laenge halbieren, um Zeichenmitte als Referenzpunkt zu verwenden
/*N*/ 		nChar1Len /= 2;
/*N*/ 
/*N*/ 		while ( nSumPnt != nLastPnt && fLenSum < nChar1Len )
/*N*/ 		{
/*N*/ 			fCos = rPoly[nSumPnt].X() - rPoly[nSumPnt-nDirection].X();
/*N*/ 			fSin = rPoly[nSumPnt].Y() - rPoly[nSumPnt-nDirection].Y();
/*N*/ 			fPartLen = sqrt(fSin * fSin + fCos * fCos);
/*N*/ 			fLenSum += fPartLen;
/*N*/ 			nSumPnt += nDirection;
/*N*/ 		}
/*N*/ 		fSin /= - fPartLen;
/*N*/ 		fCos /=   fPartLen;
/*N*/ 
/*N*/ 		fXDist = fSin * nFormTextDistance;
/*N*/ 		fYDist = fCos * nFormTextDistance;
/*N*/ 
/*N*/ 		if ( nStart > 0 )
/*N*/ 			nStart -= nLen;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nLen -= nTextLen;
/*N*/ 			nTextLen = - nLen;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( nLen > 0 && nStart <= 0 )
/*N*/ 		{
/*N*/ 			xub_StrLen nCnt(0);
/*N*/ 
/*N*/ 			if ( nStart < 0 )
/*N*/ 			{
/*N*/ 				nLen = - nStart;
/*N*/ 				nStart = 0;
/*N*/ 			}
/*N*/ 
/*N*/ 			do
/*N*/ 			{
/*N*/ 				xub_StrLen nNextGlyphLen(1);
/*N*/ 
/*N*/ 				if(bUseBreakIterator)
/*N*/ 				{
/*N*/ 					sal_Int32 nDone(0L);
/*N*/ 					nNextGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->rText, nChar + pInfo->nTextStart + nCnt, aFontLocale, 
/*N*/ 						CharacterIteratorMode::SKIPCELL, 1, nDone) - (nChar + pInfo->nTextStart + nCnt);
/*N*/ 				}
/*N*/ 
/*N*/ 				nTextLen = ImpGetTextLength(pOut, pInfo, const_cast<sal_Int32*>(pDXArray), nChar, nCnt + nNextGlyphLen);
/*N*/ 
/*N*/ 				nCnt += nNextGlyphLen;
/*N*/ 			}
/*N*/ 			while ( nChar + nCnt < nCharCnt && nTextLen < nLen );
/*N*/ 
/*N*/ 			aPos.X() -= (long) (fDx * (nLen - nShadowX) / fLen + fXDist);
/*N*/ 			aPos.Y() -= (long) (fDy * (nLen - nShadowX) / fLen + fYDist);
/*N*/ 
/*N*/ 			// BoundRect-Approximation
/*N*/ 			nXMin = Min(nXMin, aPos.X() - nMaxCharExtent);
/*N*/ 			nYMin = Min(nYMin, aPos.Y() - nMaxCharExtent);
/*N*/ 			nXMax = Max(nXMax, aPos.X() + nMaxCharExtent);
/*N*/ 			nYMax = Max(nYMax, aPos.Y() + nMaxCharExtent);
/*N*/ 
/*N*/ 			if ( eFormTextStyle == XFT_ROTATE )
/*N*/ 			{
/*N*/ 				if ( bDrawAsPoly )
/*N*/ 				{
/*N*/ 					// #101145# Do create outline polygons more efficient
/*N*/ 					// #102382# new interface supporting PolyPolyVector
/*N*/ 					PolyPolyVector aPolyPolyVector;
/*N*/ 					sal_Bool bOkay(ImpXOutGetTextOutlines(aPolyPolyVector, pOut, pInfo, nChar, nCnt));
/*N*/ 
/*N*/ 					// #102382# iterate over single PolyPolygons
/*N*/ 					for(sal_uInt32 a(0); a < aPolyPolyVector.size(); a++)
/*N*/ 					{
/*N*/ 						PolyPolygon aPolyPoly(aPolyPolyVector[a]);
/*N*/ 
/*N*/ 						if(aPolyPoly.Count() > 0 && aPolyPoly[0].GetSize() > 0)
/*N*/ 						{
/*N*/ 							XPolyPolygon aChar(aPolyPoly);
/*N*/ 
/*N*/ 							// #102382# new interface supporting PolyPolyVector
/*N*/ 							if(bIsSlantShadow)
/*N*/ 							{
/*N*/ 								if(nFormTextShdwYVal && 100 != nFormTextShdwYVal)
/*N*/ 								{
/*?*/ 									aChar.Scale(1.0, (double)nFormTextShdwYVal / 100.0);
/*N*/ 								}
/*N*/ 
/*N*/ 								aChar.SlantX(nAscent,
/*N*/ 									sin(- F_PI * nFormTextShdwXVal / 1800.0),
/*N*/ 									cos(- F_PI * nFormTextShdwXVal / 1800.0));
/*N*/ 							}
/*N*/ 
/*N*/ 							aChar.Translate(Point(aPos.X(), aPos.Y() - nAscent));
/*N*/ 							aChar.Rotate(aPos, fSin, fCos);
/*N*/ 							
/*N*/ 							if(bDraw)
/*N*/ 							{
/*?*/ 								DrawXPolyPolygon(aChar);
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*?*/ 					// #83801# vertical writing for contour
/*?*/ 					if(aFont.IsVertical())
/*?*/ 					{
/*?*/ 						short nAngle = (short) (acos(fCos) * 1800 / F_PI + 0.5);
/*?*/ 						if(fSin < 0)
/*?*/ 							nAngle = 3600 - nAngle;
/*?*/ 						nAngle += 2700;
/*?*/ 						aFont.SetOrientation(nAngle);
/*?*/ 						pOut->SetFont(aFont);
/*?*/ 						if(bDraw)
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						short nAngle = (short) (acos(fCos) * 1800 / F_PI + 0.5);
/*N*/ 						if(fSin < 0)
/*N*/ 							nAngle = 3600 - nAngle;
/*N*/ 						aFont.SetOrientation(nAngle);
/*N*/ 						pOut->SetFont(aFont);
/*N*/ 						if(bDraw)
/*N*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ImpDrawTextArray(pOut, aPos, pInfo, (long*)pDXArray, nChar, nCnt);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				bDrawAsPoly = ( bDrawAsPoly || eFormTextStyle != XFT_UPRIGHT);
/*?*/ 
/*?*/ 				for (xub_StrLen i = 0; i < nCnt;)
/*?*/ 				{
/*?*/ 					xub_StrLen nNextGlyphLen(1);
/*?*/ 					
/*?*/ 					if(bUseBreakIterator)
/*?*/ 					{
/*?*/ 						sal_Int32 nDone(0L);
/*?*/ 						nNextGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->rText, nChar + pInfo->nTextStart + i, aFontLocale, 
/*?*/ 							CharacterIteratorMode::SKIPCELL, 1, nDone) - (nChar + pInfo->nTextStart + i);
/*?*/ 					}
/*?*/ 					
/*?*/ 					XPolyPolygon aChar(0);
/*?*/ 					Point aPolyPos = aPos;
/*?*/ 
/*?*/ 					if ( i > 0 )
/*?*/ 					{
/*?*/ 						long nW;
/*?*/ 
/*?*/ 						nW = ImpGetTextLength(pOut, pInfo, const_cast<sal_Int32*>(pDXArray), nChar, i);
/*?*/ 
/*?*/ 						aPolyPos.X() += (long) (fDx * nW / fLen);
/*?*/ 						aPolyPos.Y() += (long) (fDy * nW / fLen);
/*?*/ 					}
/*?*/ 					if ( bDrawAsPoly )
/*?*/ 					{
/*?*/ 						// #100318# new for XOutGetCharOutline
/*?*/ 						// #102382# new interface supporting PolyPolyVector
/*?*/ 						PolyPolyVector aPolyPolyVector;
/*?*/ 
/*?*/ 						// #108756#
/*?*/ 						// Here the wrong index was called for the ImpXOutGetTextOutlines(...)
/*?*/ 						// call. nChar needs to be added here to index the different characters.
/*?*/ 						sal_Bool bOkay(ImpXOutGetTextOutlines(aPolyPolyVector, pOut, pInfo, nChar + i, nNextGlyphLen));
/*?*/ 
/*?*/ 						// #102382# iterate over single PolyPolygons
/*?*/ 						for(sal_uInt32 a(0); a < aPolyPolyVector.size(); a++)
/*?*/ 						{
/*?*/ 							PolyPolygon aPolyPoly(aPolyPolyVector[a]);
/*?*/ 
/*?*/ 							if(aPolyPoly.Count() > 0 && aPolyPoly[0].GetSize() > 0)
/*?*/ 							{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 
/*?*/ 					if ( eFormTextStyle == XFT_SLANTY )
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						long nW;
/*?*/ 
/*?*/ 						nW = ImpGetTextLength(pOut, pInfo, const_cast<sal_Int32*>(pDXArray), nChar + i, nNextGlyphLen);
/*?*/ 
/*?*/ 						aPolyPos.X() -= nW / 2;
/*?*/ 
/*?*/ 						if ( eFormTextStyle == XFT_SLANTX )
/*?*/ 						{
/*?*/ 							aPolyPos.X() += (long) (fDy * nAscent / fLen);
/*?*/ 							aPolyPos.Y() -= (long) (fDx * nAscent / fLen);
/*?*/ 							aChar.SlantX(0, fSin, fCos);
/*?*/ 						}
/*?*/ 						else if ( bDrawAsPoly )
/*?*/ 							aPolyPos.Y() -= nAscent;
/*?*/ 					}
/*?*/ 					aChar.Translate(aPolyPos);
/*?*/ 
/*?*/ 					if ( bDraw )
/*?*/ 					{
/*?*/ 						if ( bDrawAsPoly )
/*?*/ 							DrawXPolyPolygon(aChar);
/*?*/ 						else
/*?*/ 						{
/*?*/ 						DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	ImpDrawTextArray(pOut, aPolyPos, pInfo, (long*)pDXArray, nChar + i, nNextGlyphLen);
/*?*/ 						}
/*?*/ 					}
/*?*/ 
/*?*/ 					i += nNextGlyphLen;
/*?*/ 				}
/*N*/ 			}
/*N*/ 			nChar += nCnt;
/*N*/ 			nTextLen -= nLen;
/*N*/ 		}
/*N*/ 		nPnt += nDirection;
/*N*/ 	}
/*N*/ 	// auch letzten Punkt fuer BoundRect-Approximation beruecksichtigen
/*N*/ 	Point aPos = rPoly[nPnt - nDirection];
/*N*/ 	nXMin = Min(nXMin, aPos.X() - nMaxCharExtent);
/*N*/ 	nYMin = Min(nYMin, aPos.Y() - nMaxCharExtent);
/*N*/ 	nXMax = Max(nXMax, aPos.X() + nMaxCharExtent);
/*N*/ 	nYMax = Max(nYMax, aPos.Y() + nMaxCharExtent);

/*N*/ 	aFormTextBoundRect.Union(Rectangle(nXMin, nYMin, nXMax, nYMax));
/*N*/ 
/*N*/ 	pOut->SetFont(aOldFont);
/*N*/ 	pOut->SetLineColor( aOldLineColor );
/*N*/ 	eLineStyle = eOldLineStyle;
/*N*/ 	eFillStyle = eOldFillStyle;
/*N*/ 
/*N*/ 	return nTotal;
/*N*/ }

// #101498# changed interface due to bidi requirements
/*N*/ void XOutputDevice::ImpDrawFormTextShadow(DrawPortionInfo* pInfo, const Polygon& rPoly, sal_Int32 nAbsStart, sal_Bool bToLastPoint, sal_Bool bDraw)
/*N*/ {
/*N*/ 	if(XFTSHADOW_NONE != eFormTextShadow)
/*N*/ 	{
/*N*/ 		sal_Bool bOutline(bFormTextOutline);
/*N*/ 		Font aShadowFont(pInfo->rFont);
/*N*/ 
/*N*/ 		aShadowFont.SetColor(aFormTextShdwColor);
/*N*/ 
/*N*/ 		DrawPortionInfo aNewInfo(
/*N*/ 			pInfo->rStartPos,
/*N*/ 			pInfo->rText,
/*N*/ 			pInfo->nTextStart,
/*N*/ 			pInfo->nTextLen,
/*N*/ 			aShadowFont,
/*N*/ 			pInfo->nPara,
/*N*/ 			pInfo->nIndex,
/*N*/ 			pInfo->pDXArray,
/*N*/ 			pInfo->GetBiDiLevel());
/*N*/ 
/*N*/ 		if(XFTSHADOW_SLANT != eFormTextShadow)
/*N*/ 		{
/*N*/ 			nFormTextDistance += nFormTextShdwYVal;
/*N*/ 		}
/*N*/ 
/*N*/ 		ImpDrawFormText(&aNewInfo, rPoly, nAbsStart, bToLastPoint, bDraw, sal_True);
/*N*/ 
/*N*/ 		if(XFTSHADOW_SLANT != eFormTextShadow)
/*N*/ 		{
/*N*/ 			nFormTextDistance -= nFormTextShdwYVal;
/*N*/ 		}
/*N*/ 
/*N*/ 		bFormTextOutline = bOutline;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
*************************************************************************/

// #101498# changed interface due to bidi requirements
/*N*/ sal_Int32 XOutputDevice::DrawFormText(DrawPortionInfo* pInfo, const Polygon& rPoly,
/*N*/ 	sal_Int32 nAbsStart, sal_Bool bToLastPoint, sal_Bool bDraw)
/*N*/ {
/*N*/ 	if(nAbsStart <= 0L)
/*N*/ 	{
/*N*/ 		// empty BoundRect at first call
/*N*/ 		aFormTextBoundRect = Rectangle();
/*N*/ 	}
/*N*/ 
/*N*/ 	ImpDrawFormTextShadow(pInfo, rPoly, nAbsStart, bToLastPoint, bDraw);
/*N*/ 
/*N*/ 	return ImpDrawFormText(pInfo, rPoly, nAbsStart, bToLastPoint, bDraw, sal_False);
/*N*/ }

// eof
}
