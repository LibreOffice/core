/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVDVMARK_HXX
#define _SVDVMARK_HXX

#include <vcl/outdev.hxx>

namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XPolygon;
class XPolyPolygon;
class SdrObject;
class SdrPageView;
class SdrPaintView;

//************************************************************
//   Defines
//************************************************************

enum SdrViewUserMarkerAlign
{
    SDRMARKER_ALIGNCENTER,
    SDRMARKER_ALIGNINNER,
    SDRMARKER_ALIGNOUTER
};

//************************************************************
//   SdrViewUserMarker
//************************************************************

class SdrViewUserMarker
{
    friend class				SdrPaintView;

    SdrPaintView*				pView;
    OutputDevice*				pForcedOutDev;
    Point*						pPoint;
    Rectangle*					pRect;
    Polygon*					pPoly;
    PolyPolygon*				pPolyPoly;
    XPolygon*					pXPoly;
    XPolyPolygon*				pXPolyPoly;

    SdrViewUserMarkerAlign		eAlign;
    USHORT						nPixelDistance;      // Abstand des Rahmen zum eigentlichen pRect
    USHORT						nLineWdt;            // Strichstaerke
    USHORT						nCrossSize;          // fuer CrossHair und Stripes. 0=FullWindow
    FASTBOOL					bLineWdtLog;
    FASTBOOL					bCrossSizeLog;
    FASTBOOL					bSolidArea;
    FASTBOOL					bDashed;
    FASTBOOL					bCrossHair;          // Bei pPoint: Fadenkreuz. Sonst Dot (Rect oder Circle)
    FASTBOOL					bStripes;            // Bei pRect
    FASTBOOL					bEllipse;            // Bei pRect oder pPoint
    FASTBOOL					bPolyLine;           // Bei pPoly oder pXPoly
    FASTBOOL					bAnimate;
    FASTBOOL					bVisible;

    USHORT						nAnimateDelay;
    USHORT						nAnimateSpeed;
    USHORT						nAnimateAnz;
    FASTBOOL					bAnimateBwd;
    FASTBOOL					bAnimateToggle;
    USHORT						nAnimateDelayCountDown;
    USHORT						nAnimateSpeedCountDown;
    USHORT						nAnimateNum;

    FASTBOOL					bHasPointer;
    FASTBOOL					bMouseMovable;

protected:
    void ImpDelGeometrics();

public:
    SdrViewUserMarker(SdrPaintView* pView);

    // Jede Ableitung, die ImpDraw ueberladen hat muss
    // im Dtor IsVisible() fragen und ggf. Hiden!
    virtual ~SdrViewUserMarker();

    // Falls der Marker nur in einem bestimmten Window der View
    // dargestellt werden soll. Default ist NULL= alle Windows der View.
    void SetOutputDevice(OutputDevice* pOut) { pForcedOutDev=pOut; }
    OutputDevice* GetOutputDevice() const { return pForcedOutDev; }

    // Der Marker kann durchaus auch in einen andere View gesteckt werden.
    // (Beim Destruieren der View wird die View am Marker automatisch auf NULL gesetzt.)

    void SetLineWidth(USHORT nWdt);

    void SetAnimateDelay(USHORT nTime) { nAnimateDelay=(nTime+25)/50; }
    void SetAnimateSpeed(USHORT nTime) { nAnimateSpeed=(nTime+25)/50; if (nAnimateSpeed>0) nAnimateSpeed--; }
    void SetAnimateCount(USHORT nAnz) { nAnimateAnz=nAnz; }
    void SetAnimateBackward(FASTBOOL bOn) { bAnimateBwd=bOn; }
    void SetAnimateToggle(FASTBOOL bOn) { bAnimateToggle=bOn; }

    const Point* GetPoint() const { return pPoint; }
    const Rectangle* GetRectangle() const { return pRect; }
    const Polygon* GetPolygon() const { return pPoly; }
    const PolyPolygon* GetPolyPolygon() const { return pPolyPoly; }
    const XPolygon* GetXPolygon() const { return pXPoly; }
    const XPolyPolygon* GetXPolyPolygon() const { return pXPolyPoly; }

    FASTBOOL IsDot() const { return pPoint!=NULL && !bCrossHair; }
    FASTBOOL IsCrossHair() const { return pPoint!=NULL && bCrossHair; }
    FASTBOOL IsRectangle() const { return pRect!=NULL && !bEllipse && !bStripes; }
    FASTBOOL IsStripes() const { return pRect!=NULL && bStripes; }
    FASTBOOL IsEllipse() const { return pRect!=NULL && bEllipse; }
    FASTBOOL IsPolyLine() const { return (pPoly!=NULL || pPolyPoly!=NULL || pXPoly!=NULL || pXPolyPoly!=NULL) && bPolyLine; }

    FASTBOOL IsSolidArea() const { return bSolidArea && pPoint==NULL && !bPolyLine; }
    FASTBOOL IsDashed() const { return bDashed; }
    USHORT GetPixelDistance() const { return nPixelDistance; }
    USHORT GetLineWidth() const { return nLineWdt; }
    FASTBOOL IsLineWidthIsLogic() const { return bLineWdtLog; }
    USHORT GetCrossHairSize() const { return nCrossSize; }
    FASTBOOL IsCrossHairSizeIsLogic() const { return bCrossSizeLog; }

    FASTBOOL IsAnimate() const { return bAnimate; }
    USHORT GetAnimateDelay() const { return nAnimateDelay*50; }
    USHORT GetAnimateSpeed() const { return (nAnimateSpeed+1)*50; }
    USHORT GetAnimateCount() const { return nAnimateAnz; }
    FASTBOOL IsAnimateBackward() const { return bAnimateBwd; }
    FASTBOOL IsAnimateToggle() const { return bAnimateToggle; }

    void Show();
    void Hide();
    FASTBOOL IsVisible() const { return bVisible; }

    FASTBOOL HasPointer() const { return bHasPointer; }
    FASTBOOL IsMouseMovable() const { return bMouseMovable; }
};

}//end of namespace binfilter
#endif //_SVDVMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
