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

/**
 * Definition:
 *  - snap something means e.g. the mouse pointer or some marked objects in drag can be snapped
 *  - snap on something means you can e.g. snap on the grid or on auxiliary lines
 *
 *
 * Basically, we only catch visible elements (borders, helper lines, connectors),
 * with the exception of the grid. Likewise, we can only catch visible elements (i.e. connectors).
 * We only catch on the grid, if nothing else is close by (magnetic behaviour).
 *
 * The cursor which is used during creation of objects, while dragging polygon points
 * etc. (i.e. the mouse cursor) is always caught on all enabled catch alternatives (max 6).
 *
 * This is different, when moving selected objects. Instead of one mouse cursor, there are
 * four alternatives, which can be caught at the selected objects:
 *   1. the logically enclosing frame of every single object
 *   2. the logically enclosing frame of all selected objects
 *   3. highlighted points of all selected objects (polygon points, ...)
 *   4. the connectors of the selected objects
 *
 * As the first and second case exclude each other (the second is a refinement of the first one)
 * three alternatives remain, which are independent from each other. For 6 possible values for the
 * caught ones, we would end up with max. 18 different possible combinations!
 *
 * Therefore, we introduce the following simplifications:
 *   1. Connectors only catch on connectors
 *
 * Only max. 2*5+1 = 11 combinations remain for MoveDrag:
 *   1-3.  enclosing frame on grid/border/helper lines
 *   4.    enclosing frame on highlighted object points
 *   5.    enclosing frame on enclosing frame
 *   6-8.  highlighted points on grid/border/helper lines
 *   7.    highlighted points on highlighted object points
 *   8-10. highlighted points on enclosing frame
 *   11.   connectors on connectors
 *
 * We test for all of these possible 11 combinations in the MouseMove event in the DragMove
 * and those with the least need of correction are carried out.
 *
 * In the Resize() etc. methods we only catch the logically enclosing frame of the highlighted objects.
 */

/** Return value for SnapPos() method */
enum class SdrSnap
{
    NOTSNAPPED = 0x00,
    XSNAPPED   = 0x01,
    YSNAPPED   = 0x02
};
namespace o3tl
{
    template<> struct typed_flags<SdrSnap> : is_typed_flags<SdrSnap, 3> {};
}

// TODO SdrCrookMode::Stretch is not implemented yet!
enum class SdrCrookMode {
    Rotate,
    Slant,
    Stretch
};


// #114409#-1 Migrate PageOrigin
class ImplPageOriginOverlay;

class SVX_DLLPUBLIC SdrSnapView : public SdrPaintView
{
protected:
    // #114409#-1 Migrate PageOrigin
    class ImplPageOriginOverlay*            mpPageOriginOverlay;

    // #114409#-2 Migrate HelpLine
    class ImplHelpLineOverlay*              mpHelpLineOverlay;

    Size                        aMagnSiz;
    Fraction                    aSnapWdtX;
    Fraction                    aSnapWdtY;

    sal_uInt16                  nMagnSizPix;
    long                        nSnapAngle;
    long                        nEliminatePolyPointLimitAngle;

    SdrCrookMode                eCrookMode;

    bool                        bSnapEnab : 1;
    bool                        bGridSnap : 1;
    bool                        bBordSnap : 1;
    bool                        bHlplSnap : 1;
    bool                        bOFrmSnap : 1;
    bool                        bOPntSnap : 1;
    bool                        bOConSnap : 1;
    bool                        bMoveSnapOnlyTopLeft : 1;    //  Special for dialogeditor
    bool                        bOrtho : 1;
    bool                        bBigOrtho : 1;
    bool                        bAngleSnapEnab : 1;
    bool                        bMoveOnlyDragging : 1;       // only move objects while Resize/Rotate/...
    bool                        bSlantButShear : 1;          // use slant instead of shear
    bool                        bCrookNoContortion : 1;      // no contorsion while Crook
    bool                        bEliminatePolyPoints : 1;

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrSnapView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrSnapView() override;

public:
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BckAction() override;
    virtual void BrkAction() override; // break actions for derived classes e.g. interrupt dragging.
    virtual void TakeActionRect(tools::Rectangle& rRect) const override;

    void SetSnapGridWidth(const Fraction& rX, const Fraction& rY) { aSnapWdtX=rX; aSnapWdtY=rY; }
    const Fraction& GetSnapGridWidthX() const { return aSnapWdtX; }
    const Fraction& GetSnapGridWidthY() const { return aSnapWdtY; }

    void SetSnapMagnetic(const Size& rSiz) { if (rSiz!=aMagnSiz) { aMagnSiz=rSiz; } }
    void SetSnapMagneticPixel(sal_uInt16 nPix) { nMagnSizPix=nPix; }
    sal_uInt16 GetSnapMagneticPixel() const { return nMagnSizPix; }

    // RecalcLogicSnapMagnetic has to be called for every change of OutputDevices and every change of the MapMode!
    void RecalcLogicSnapMagnetic(const OutputDevice& rOut) { SetSnapMagnetic(rOut.PixelToLogic(Size(nMagnSizPix,nMagnSizPix))); }
    void SetActualWin(const OutputDevice* pWin) { SdrPaintView::SetActualWin(pWin); if (pWin!=nullptr) RecalcLogicSnapMagnetic(*pWin); }

    // Coordinates referred to the view!
    // Returnvalues are SdrSnap::NOTSNAPPED,SdrSnap::XSNAPPED,
    // SdrSnap::YSNAPPED or SdrSnap::XYSNAPPED
    SdrSnap SnapPos(Point& rPnt, const SdrPageView* pPV) const;
    Point GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const;
    void CheckSnap(const Point& rPt, long& nBestXSnap, long& nBestYSnap, bool& bXSnapped, bool& bYSnapped) const;

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
    void BegSetPageOrg(const Point& rPnt);
    void MovSetPageOrg(const Point& rPnt);
    void EndSetPageOrg();
    void BrkSetPageOrg();
    bool IsSetPageOrg() const { return (nullptr != mpPageOriginOverlay); }

    // HitTest. If sal_True, in rnHelpLineNum is the number of the auxiliary line and in rpPv
    // the appended PageView.
    bool PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, sal_uInt16& rnHelpLineNum, SdrPageView*& rpPV) const;

    // Move of an available auxiliary line. Use nHelpLineNum and pPV from PickHelpLine.
    bool BegDragHelpLine(sal_uInt16 nHelpLineNum, SdrPageView* pPV);
    // interactive insertion of a new auxiliary line
    void BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind);
    PointerStyle GetDraggedHelpLinePointer() const;

    // change the type of auxiliary line while dragging
    // void SetDraggedHelpLineKind(SdrHelpLineKind eNewKind);
    void MovDragHelpLine(const Point& rPnt);
    bool EndDragHelpLine();
    void BrkDragHelpLine();
    bool IsDragHelpLine() const { return (nullptr != mpHelpLineOverlay); }

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
    void SetOrtho(bool bOn) { bOrtho=bOn; } // incomplete
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

    // Crook-Mode. persistent. Default=SdrCrookMode::Rotate. (ni)
    void SetCrookMode(SdrCrookMode eMode) { eCrookMode=eMode; }
    SdrCrookMode GetCrookMode() const { return eCrookMode; }

    // Special for IBM: While Dragging of a traverse station, it is deleted
    // if its adjacent lines are almost a solid line.
    void SetEliminatePolyPoints(bool bOn) { bEliminatePolyPoints=bOn; }
    bool IsEliminatePolyPoints() const { return bEliminatePolyPoints; }
    void SetEliminatePolyPointLimitAngle(long nAngle) { nEliminatePolyPointLimitAngle=nAngle; }
    long GetEliminatePolyPointLimitAngle() const { return nEliminatePolyPointLimitAngle; }
};

#endif // INCLUDED_SVX_SVDSNPV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
