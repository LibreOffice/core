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

#ifndef _XOUTX_HXX
#define _XOUTX_HXX

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#include <bf_svx/xhatch.hxx>
#include <bf_svx/xgrad.hxx>
#ifndef _XPOLY_HXX //autogen
#include <bf_svx/xpoly.hxx>
#endif
#ifndef _SVX_RECTENUM_HXX //autogen
#include <bf_svx/rectenum.hxx>
#endif
#ifndef _XENUM_HXX
#include <bf_svx/xenum.hxx>
#endif
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
namespace binfilter {
class SfxItemSet;
class BfGraphicObject;

class FormattedText;
class XPolygon;
class XPolyPolygon;
class XLineAttrSetItem;
class XFillAttrSetItem;
class XTextAttrSetItem;
class XLineAttrSetItem;
class XFillAttrSetItem;
class XOutdevItemPool;
class XLineParam;
struct ImpData;

// #101498# predeclaration for DrawPortionInfo
class DrawPortionInfo;

#if _SOLAR__PRIVATE

// -----------
// - ImpData -
// -----------

struct ImpData
{
    XGradient*	    pTransGradient;
    BOOL		    bSolidHatch;
};

#define iRotTransGradient()	( ( (ImpData*)pImpData )->pTransGradient )
#define iSolidHatch()		( ( (ImpData*)pImpData )->bSolidHatch )

#endif // _PRIVATE

/*************************************************************************
|*
|* Extended OutputDevice
|*
\************************************************************************/

#if defined(MACOSX) && (__GNUC__ < 3)
static Brush aBrush;
static Pen aPen;
#endif

class XOutputDevice
{
private:

    ImpData*			pImpData;

    long*				pLinePattern;
    USHORT				nLinePatternCnt;

    XLineStyle			eLineStyle;
    Color				aLineColor;
    USHORT				nLineTransparence;
    long				nLineWidth;
    XPolygon			aLineStartPoly, aLineEndPoly;
    long				nLineStartSqLen, nLineEndSqLen;
    BOOL				bLineStart, bLineEnd;
    BOOL				bHair;

    XFillStyle			eFillStyle;
    Color				aFillColor;
    USHORT				nFillTransparence;

    Bitmap				maFillBitmap;
    BfGraphicObject*      mpFillGraphicObject;
    void*               mpDummy1;
    void*               mpDummy2;
    void*               mpDummy3;
    void*               mpDummy4;
    void*               mpDummy5;
    long                mnDummy;
    long                mnDummy2;
    long                mnDummy3;
    long                mnDummy4;
    long                mnDummy5;
    MapMode				maLastMapMode;
    Size				maFillBitmapSize;
    Size				maBmpSize;
    Size				maBmpOutputSize;
    long                mnDummy6;
    long                mnDummy7;
    Point				maStartPoint;
    USHORT				mnBmpPerCentX;
    USHORT				mnBmpPerCentY;
    USHORT				mnBmpOffX;
    USHORT				mnBmpOffY;
    USHORT				mnBmpOffPosX;
    USHORT				mnBmpOffPosY;
    OutDevType			meLastOutDevType;
    RECT_POINT			meBmpRectPoint;
    BOOL				mbBmpTile;
    BOOL				mbBmpStretch;
    BOOL				mbBmpLogSize;
    BOOL				mbDummy;
    BOOL				mbRecalc;

    XGradient			aGradient;
    XHatch				aHatch;
    BOOL				bDither;

    BOOL				bIgnoreLineAttr;
    BOOL				bIgnoreLineStyle;
    BOOL				bIgnoreFillAttr;

    XFormTextStyle      eFormTextStyle;
    XFormTextAdjust     eFormTextAdjust;
    long                nFormTextDistance;
    long                nFormTextStart;
    BOOL                bFormTextMirror;
    BOOL                bFormTextOutline;
    XFormTextShadow     eFormTextShadow;
    Color               aFormTextShdwColor;
    USHORT              nFormTextShdwTransp;
    long                nFormTextShdwXVal;
    long                nFormTextShdwYVal;
    XFormTextStdForm    eFormTextStdForm;
    BOOL                bFormTextHideForm;

    // Zwischenspeicher fuer FormText-Auto-Fonthoehe bei mehreren
    // zusammengehoerigen Aufrufen
    long				nFtAutoHeightSave;
    // umschliessendes Rechteck der Formtext-Effekte
    Rectangle			aFormTextBoundRect;

    void				ImpDrawFillPolyPolygon( const PolyPolygon& rPoly, BOOL bRect, BOOL bPrinter );
    void				ImpDrawLinePolygon(const Polygon& rPoly, BOOL bClosePoly);

protected:

    OutputDevice*		pOut;
    Point				aOfs;

    void				DrawFillPolyPolygon( const PolyPolygon& rPoly, BOOL bRect = FALSE );
    void				DrawLinePolygon( const Polygon& rPoly, BOOL bClosePoly );

    // #101498# changed interface due to bidi requirements
    sal_Int32 ImpDrawFormText(DrawPortionInfo* pInfo, const Polygon& rPoly, sal_Int32 nAbsStart, 
        sal_Bool bToLastPoint, sal_Bool bDraw, sal_Bool bIsShadow);
    void ImpDrawFormTextShadow(DrawPortionInfo* pInfo, const Polygon& rPoly, sal_Int32 nAbsStart, 
        sal_Bool bToLastPoint, sal_Bool bDraw);
    //long				ImpDrawFormText(const String& rText, const Polygon& rPoly,
    //									Font aFont, long nAbsStart, BOOL bIsShadow,
    //									BOOL bToLastPoint, BOOL bDraw,
    //									const long* pDXArray = NULL);
    //void				DrawFormTextShadow(const String& rText, const Polygon& rPoly,
    //									   const Font& rFont, long nAbsStart,
    //									   BOOL bToLastPoint, BOOL bDraw,
    //									   const long* pDXArray = NULL);

/*N*/ 	void				ImpDrawTiledBitmap( OutputDevice* pOut, const Rectangle& rRect,
/*N*/ 											const Point& rStartPoint, const Rectangle& rClipRect,
/*N*/                                             BOOL bPrinter );

 public:
                        XOutputDevice(OutputDevice* pOut);
                        ~XOutputDevice();

    OutputDevice*		GetOutDev() const { return pOut; }
    void				SetOutDev(OutputDevice* pNewOut) { pOut=pNewOut; }

    RasterOp			GetRasterOp() const { return pOut->GetRasterOp(); }
    void				SetRasterOp(RasterOp eRop) { pOut->SetRasterOp(eRop); }

    // Allgemeine Ausgabemethoden
    void				DrawLine( const Point& rStart, const Point& rEnd );
    void				DrawRect( const Rectangle& rRect, ULONG nXRound=0, ULONG nYRound=0 );
    void				DrawEllipse( const Rectangle& rRect );
    void				DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd );
    void				DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rEnd );
    void				DrawXPolyLine( const XPolygon& rXPoly );
    void				DrawXPolygon( const XPolygon& rXPoly );
    void				DrawXPolyPolygon( const XPolyPolygon& rXPoly );

    // Bitmap Ausgabemethoden
    void				DrawBitmap( const Point& rPos, const Size& rSize, const Bitmap& rBmp );
    void				DrawBitmap( const Point& rDestPos, const Size& rDestSize,
                                    const Point& rSrcPos,  const Size& rSrcSize );

    // Text Ausgabemethoden
    // Der formatierte Text kommt aus der EditEngine von MT
    void				DrawFormattedText( const Point& rPos, const FormattedText& rText );

    // #101498# changed interface due to bidi requirements
    sal_Int32 DrawFormText(DrawPortionInfo* pInfo, const Polygon& rPoly,
        sal_Int32 nAbsStart = 0L, sal_Bool bToLastPoint = sal_True, 
        sal_Bool bDraw = sal_True);

    // #101498# XPolygon version of DrawFormText not used, removed.
    // Difference was only to change XPolygon to Polygon using 
    // XOutCreatePolygon(...)
    //long				DrawFormText(const String& rText, const XPolygon& rXPoly,
    //								 Font aFont, long nAbsStart = 0,
    //								 BOOL bToLastPoint = TRUE, BOOL bDraw = TRUE,
    //								 const long* pDXArray = NULL);

    const Rectangle&	GetFormTextBoundRect() const { return aFormTextBoundRect; }

    // Nur Temporaer
    const Color&		GetLineColor() const { return pOut->GetLineColor(); }
    void				SetLineColor( const Color& rColor ) { pOut->SetLineColor( rColor ); }

    const Color&		GetFillColor() const { return pOut->GetFillColor(); }
    void				SetFillColor( const Color& rColor ) { pOut->SetFillColor( rColor ); }

    // Attributeinstellungen
    void				SetLineAttr(const SfxItemSet& rAttr);
    void				SetFillAttr(const SfxItemSet& rAttr);
    void				SetTextAttr(const SfxItemSet& rAttr);

    // Attributeinstellungen ignorieren
    void				SetIgnoreLineAttr(BOOL bIgnore) { bIgnoreLineAttr = bIgnore; }
    BOOL				GetIgnoreLineAttr() { return bIgnoreLineAttr; }
    void				SetIgnoreLineStyle(BOOL bIgnore) { bIgnoreLineStyle = bIgnore; }
    BOOL				GetIgnoreLineStyle() { return bIgnoreLineStyle; }
    void				SetIgnoreFillAttr(BOOL bIgnore) { bIgnoreFillAttr = bIgnore; }
    BOOL				GetIgnoreFillAttr() { return bIgnoreFillAttr; }

    // StarView- Linecolor/style und Fillcolor/style temporaer ueberschreiben, z.B. fuer
    // dragging in SvDraw
    void				OverrideLineColor(const Color& rColor );
    void				OverrideFillColor(const Color& rColor );

    // Zum Abschalten der Linienenden
    BOOL				IsLineStart() const { return bLineStart; }
    void				OverrideLineStart(BOOL bOn) { bLineStart=bOn; }
    BOOL				IsLineEnd() const { return bLineEnd; }
    void				OverrideLineEnd(BOOL bOn) { bLineEnd=bOn; }

    // Allgemeine Einstellungen
    const Point&		GetOffset() const { return aOfs; }
    void				SetOffset( const Point& rOfs );

    // Initialisierung der Linienenden statisch, damit auch ohne XOutDev
    // von SvDraw verwendbar
    static long			InitLineStartEnd(XPolygon& rXPoly, long nWidth, BOOL bCenter);
};

// Nur aus Kompatibilitaetsgruenden fuer SvDraw
class ExtOutputDevice : public XOutputDevice 
{
public:

        ExtOutputDevice(OutputDevice* pNewOut): XOutputDevice(pNewOut) {}
};

/*************************************************************************
|*
|* Funktionen zur Berechnung von Bezierkurven; muessen global sein,
|* da sie auch von XPolygon::GetBoundRect benoetigt werden
|*
\************************************************************************/

Rectangle		XOutCalcXPolyExtent(const XPolygon& rXPoly, OutputDevice* pOut);
long			XOutCalcBezierStepCount( const XPolygon& rXPoly, USHORT nIndex, OutputDevice* pOut, USHORT nRough = 0 );
void			XOutCalcBezier( const XPolygon& rXPoly, USHORT nBezIndex, Polygon& rPoly, USHORT nPolyIndex, long nSteps );
Polygon			XOutCreatePolygon( const XPolygon& rXPoly, OutputDevice* pOut, USHORT nRough = 0 );
Polygon			XOutCreatePolygonBezier( const XPolygon& rXPoly, OutputDevice* pOut );
// #102382# Remove XOutGetCharOutline
//XPolyPolygon	XOutGetCharOutline( USHORT nChar, OutputDevice& rOut, BOOL bOptimizeSize = TRUE );

}//end of namespace binfilter
#endif      // _XOUTX_HXX

