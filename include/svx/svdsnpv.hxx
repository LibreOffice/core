/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SVDSNPV_HXX
#define INCLUDED_SVX_SVDSNPV_HXX

#include <svx/svdpntv.hxx>
#include <svx/svdhlpln.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>


/** return value for SnapPos() method */
enum class SdrSnap
{
    NOTSNAPPED = 0x00,
    XSNAPPED   = 0x01,
    YSNAPPED   = 0x02,
    XYSNAPPED  = XSNAPPED | YSNAPPED,
};
namespace o3tl
{
    template<> struct typed_flags<SdrSnap> : is_typed_flags<SdrSnap, 3> {};
}

// SDRCROOK_STRETCH is not implemented yet!
enum SdrCrookMode {
    SDRCROOK_ROTATE,
    SDRCROOK_SLANT,
    SDRCROOK_STRETCH
};


// #114409#-1 Migrate PageOrigin
class ImplPageOriginOverlay;

class SVX_DLLPUBLIC SdrSnapView: public SdrPaintView
{
protected:
    // #114409#-1 Migrate PageOrigin
    class ImplPageOriginOverlay*            mpPageOriginOverlay;

    // #114409#-2 Migrate HelpLine
    class ImplHelpLineOverlay*              mpHelpLineOverlay;

    Size                        aMagnSiz;
    Fraction                    aSnapWdtX;
    Fraction                    aSnapWdtY;

    sal_uInt16                      nMagnSizPix;
    long                        nSnapAngle;
    long                        nEliminatePolyPointLimitAngle;

    SdrCrookMode                eCrookMode;

    bool                        bSnapEnab : 1;
    bool                        bGridSnap : 1;
    bool                        bSnapTo1Pix : 1;             // If GridSnap off, snap to one Pixel to avoid values like 10.01
    bool                        bBordSnap : 1;
    bool                        bHlplSnap : 1;
    bool                        bOFrmSnap : 1;
    bool                        bOPntSnap : 1;
    bool                        bOConSnap : 1;
    bool                        bMoveMFrmSnap : 1;
    bool                        bMoveOFrmSnap : 1;
    bool                        bMoveOPntSnap : 1;
    bool                        bMoveOConSnap : 1;
    bool                        bMoveSnapOnlyTopLeft : 1;    //  Special for dialogeditor
    bool                        bOrtho : 1;
    bool                        bBigOrtho : 1;
    bool                        bAngleSnapEnab : 1;
    bool                        bMoveOnlyDragging : 1;       // only move objects while Resize/Rotate/...
    bool                        bSlantButShear : 1;          // use slant instead of shear
    bool                        bCrookNoContortion : 1;      // no contorsion while Crook
    bool                        bHlplFixed : 1;              // sal_True= fixed auxiliary lines, so it isn't movable
    bool                        bEliminatePolyPoints : 1;

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrSnapView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrSnapView();

public:
    virtual bool IsAction() const SAL_OVERRIDE;
    virtual void MovAction(const Point& rPnt) SAL_OVERRIDE;
    virtual void EndAction() SAL_OVERRIDE;
    virtual void BckAction() SAL_OVERRIDE;
    virtual void BrkAction() SAL_OVERRIDE; // break actions for derived classes e.g. interrupt dragging.
    virtual void TakeActionRect(Rectangle& rRect) const SAL_OVERRIDE;

    void SetSnapGridWidth(const Fraction& rX, const Fraction& rY) { aSnapWdtX=rX; aSnapWdtY=rY; }
    const Fraction& GetSnapGridWidthX() const { return aSnapWdtX; }
    const Fraction& GetSnapGridWidthY() const { return aSnapWdtY; }

    void SetSnapMagnetic(const Size& rSiz) { if (rSiz!=aMagnSiz) { aMagnSiz=rSiz; } }
    void SetSnapMagneticPixel(sal_uInt16 nPix) { nMagnSizPix=nPix; }
    sal_uInt16 GetSnapMagneticPixel() const { return nMagnSizPix; }

    // RecalcLogicSnapMagnetic has to be called for every change of OutputDevices and every change of the MapMode!
    void RecalcLogicSnapMagnetic(const OutputDevice& rOut) { SetSnapMagnetic(rOut.PixelToLogic(Size(nMagnSizPix,nMagnSizPix))); }
    void SetActualWin(const OutputDevice* pWin) { SdrPaintView::SetActualWin(pWin); if (pWin!=NULL) RecalcLogicSnapMagnetic(*pWin); }

    // Coordinates referred to the view!
    // Returnvalues are SdrSnap::NOTSNAPPED,SdrSnap::XSNAPPED,
    // SdrSnap::YSNAPPED or SdrSnap::XYSNAPPED
    SdrSnap SnapPos(Point& rPnt, const SdrPageView* pPV) const;
    Point GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const;
    void CheckSnap(const Point& rPt, const SdrPageView* pPV, long& nBestXSnap, long& nBestYSnap, bool& bXSnapped, bool& bYSnapped) const;

    // All attitudes to snap are persistent.
    bool IsSnapEnabled() const { return bSnapEnab; }
    bool IsGridSnap() const { return bGridSnap; } // Snap to grid
    bool IsBordSnap() const { return bBordSnap; } // Snap to border
    bool IsHlplSnap() const { return bHlplSnap; } // Snap to auxiliary line
    bool IsOFrmSnap() const { return bOFrmSnap; } // Snap to LogFram from surrounding drawing objects
    bool IsOPntSnap() const { return bOPntSnap; } // Snap to distinct points from surrounding drawing objects
    bool IsOConSnap() const { return bOConSnap; } // Snap to connectors of the drawing objects
    void SetSnapEnabled(bool bOn) { bSnapEnab=bOn; }
    void SetGridSnap(bool bOn) { bGridSnap=bOn; }
    void SetBordSnap(bool bOn) { bBordSnap=bOn; }
    void SetHlplSnap(bool bOn) { bHlplSnap=bOn; }
    void SetOFrmSnap(bool bOn) { bOFrmSnap=bOn; }
    void SetOPntSnap(bool bOn) { bOPntSnap=bOn; }
    void SetOConSnap(bool bOn) { bOConSnap=bOn; }

    // Usually every 4 corners of Object-SnapRects are snapped for Move-Dragging.
    // The following attitudes e.g. if you only want to snap the left corner on the top (e.g. DialogEditor)
    // persistent, Default=FALSE.
    void SetMoveSnapOnlyTopLeft(bool bOn) { bMoveSnapOnlyTopLeft=bOn; }
    bool IsMoveSnapOnlyTopLeft() const { return bMoveSnapOnlyTopLeft; }

    // #114409#-1 Migrate PageOrigin
    bool BegSetPageOrg(const Point& rPnt);
    void MovSetPageOrg(const Point& rPnt);
    bool EndSetPageOrg();
    void BrkSetPageOrg();
    bool IsSetPageOrg() const { return (0L != mpPageOriginOverlay); }

    // HitTest. If sal_True, in rnHelpLineNum is the number of the auxiliary line and in rpPv
    // the appendend PageView.
    bool PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, sal_uInt16& rnHelpLineNum, SdrPageView*& rpPV) const;

    // Move of an available auxiliary line. Use nHelpLineNum and pPV from PickHelpLine.
    bool BegDragHelpLine(sal_uInt16 nHelpLineNum, SdrPageView* pPV);
    // interactive insertionof a new auxiliary line
    bool BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind);
    Pointer GetDraggedHelpLinePointer() const;

    // change the type of auxiliary line while dragging
    // void SetDraggedHelpLineKind(SdrHelpLineKind eNewKind);
    void MovDragHelpLine(const Point& rPnt);
    bool EndDragHelpLine();
    void BrkDragHelpLine();
    bool IsDragHelpLine() const { return (0L != mpHelpLineOverlay); }

    // SnapAngle is for angles in circle, RotateDragging, ...
    // The snapping of an angle is beared down, if it is switched off
    // with SetAngleSnapEnabled(sal_False)
    // The snapping angles is independent of snapping coordinates
    // and so independent of the attitude IsSnapEnabled()
    // Only values should be specified for them is applied:
    //     36000 modulo nAngle = 0
    // Implemented for:
    // - Rotate (Dragging)
    // - Shear (Dragging)
    // - circular arc/-sector/-section angle (Create and Dragging)
    // persistent.
    void SetAngleSnapEnabled(bool bOn) { bAngleSnapEnab=bOn; }
    bool IsAngleSnapEnabled() const { return bAngleSnapEnab; }
    void SetSnapAngle(long nAngle) { nSnapAngle=nAngle; }
    long GetSnapAngle() const { return nSnapAngle; }

    // different effects from Ortho (depending on the context):
    // - Create
    //   - only lines in 45deg grid
    //   - instead of rectangles squares are created
    //   - instead of ellipse circles are created
    // - Dragging
    //   - general Dragging
    //     - Move only horizontal, vertical or 45deg
    //     - Resize proportional
    //     - Mirror: nothing
    //     - Shear without Resize
    //     - Crook without Resize
    //   - move handles
    //     - mirror axis only 45deg grid
    //   - object-specific Dragging
    //     - rectangle corner radius: nothing
    //     - circle object angle: nothing
    //     - line keeps while Dragging the angle and is only stretched/ contracted
    // Default value for Ortho is off. persistent.
    void SetOrtho(bool bOn) { bOrtho=bOn; } // unvollstaendig
    bool IsOrtho() const { return bOrtho; }

    // BigOrtho is only relevant if Ortho is switched on.
    // Example: rectangle is created and ortho is switched on (--> square)
    //   and the Mouse was dragged from zero to the coordinates
    //   (80,30). Now there are 2 alternatives to determine the edge length
    //   of the square: 30 and 80.
    //   The standard Ortho-Function took 30 (every time the smaller length)
    //   If BigOrtho is switched on, you get a square with edge length of 80.
    // The same also applies to Resize.
    // Default value for BigOrtho is on. persistent.
    void SetBigOrtho(bool bOn) { bBigOrtho=bOn; }
    bool IsBigOrtho() const { return bBigOrtho; }

    // If MoveOnlyDragging=sal_True only the center of the marked objects is
    // transformed when Resize/Rotate/Shear/Mirror/Crook is executed.
    // Size, form and rotation angle of the objects are conserved only their positions
    // are changed. persistent. Default=FALSE. (ni)
    void SetMoveOnlyDragging(bool bOn) { bMoveOnlyDragging=bOn; }
    bool IsMoveOnlyDragging() const { return bMoveOnlyDragging; }

    // Use Slant instead of Shear. persistent. Default=FALSE.
    void SetSlantButShear(bool bOn) { bSlantButShear=bOn; }
    bool IsSlantButShear() const { return bSlantButShear; }

    // Don't contort objecte while Crook. persistent. Default=FALSE. (ni)
    void SetCrookNoContortion(bool bOn) { bCrookNoContortion=bOn; }
    bool IsCrookNoContortion() const { return bCrookNoContortion; }

    // Crook-Mode. persistent. Default=SDRCROOK_ROTATE. (ni)
    void SetCrookMode(SdrCrookMode eMode) { eCrookMode=eMode; }
    SdrCrookMode GetCrookMode() const { return eCrookMode; }

    // Special for IBM: While Dragging of a traverse station, it is deleted
    // if its adjacent lines are almost a solid line.
    void SetEliminatePolyPoints(bool bOn) { bEliminatePolyPoints=bOn; }
    bool IsEliminatePolyPoints() const { return bEliminatePolyPoints; }
    void SetEliminatePolyPointLimitAngle(long nAngle) { nEliminatePolyPointLimitAngle=nAngle; }
    long GetEliminatePolyPointLimitAngle() const { return nEliminatePolyPointLimitAngle; }
};

// definition:
//   - snap something= e.g. the mouse pointer or some marked objects in drag can be snapped
//   - snap on sth.= you can e.g. snap on the grid or on auxiliary lines.
//
//
// Grundsaetzlich wird nur gefangen auf sichtbare Elemente (-> Border,
// Hilfslinien, Konnektoren; Ausnahme: Grid). Ebenso koennen nur sichtbare
// Elemente gefangen werden (->Konnektoren).
//
// Auf's Grid wird immer erst dann gefangen, wenn nix Anderes in der Naehe
// (->Magnetic) ist.
//
// Der "Cursor" (also der Mauszeiger) beim Erzeugen von Objekten, beim Draggen
// von Polygonpunkten, ... wird immer auf allen eingeschalteten Fangalternativen
// gefangen (max 6).
//
// Beim Verschieben markierter Objekte ist das etwas anders. Statt des einen
// Mauscursors gibt es hier 4 Alternativen an den markierten Objekten, die
// gefangen werden koennen:
//   1. die logisch-umschliessenden Rahmen der einzelnen Objekte
//   2. der logisch-umschliessende Rahmen aller markierten Objekte
//   3. ausgezeichnete Punkte der markierten Objekte (Polygonpunkte, ...)
//   4. die Konnektoren der markierten Objekte
// Da 1. und 2. einander ausschliessen (2. ist eine Verfeinerung von 1.)
// bleiben 3 voneinander unabhaengige Alternativen. Bei 6. Moeglichkeiten auf
// die gefangen werden kann kaeme man auf max. 18 Kombinationsmoeglichkeiten!
// Deshalb werden folgende Vereinfachungen festgelegt:
//   1. Konnektoren fangen sich nur auf Konnektoren.
// Verbleiben also nun noch max. 2x5+1=11 Fangkombinationen beim MoveDrag:
//   1-3.  umschliessende(r) Rahmen auf Grid/Border/Hilfslinien
//   4.    umschliessende(r) Rahmen auf ausgezeichnete Objektpunkte
//   5.    umschliessende(r) Rahmen auf umschliessenden Rahmen
//   6-8.  ausgezeichnete Punkte auf Grid/Border/Hilfslinien
//   7.    ausgezeichnete Punkte auf ausgezeichnete Objektpunkte
//   8-10. ausgezeichnete Punkte auf umschliessenden Rahmen
//   11.   Konnektoren auf Konnektoren
// Beim MouseMove-Event im DragMove werden also diese bis zu max. 11 moeglichen
// Alternativen durchgetestet und die mit dem gerigsten Korrekturaufwand
// vollzogen.
//
// Beim Resize, ... wird immer nur der logisch-umschliessende Rahmen der
// markierten Objekte gefangen.

#endif // INCLUDED_SVX_SVDSNPV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
