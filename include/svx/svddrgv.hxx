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

#ifndef INCLUDED_SVX_SVDDRGV_HXX
#define INCLUDED_SVX_SVDDRGV_HXX

#include <svx/svxdllapi.h>
#include <svx/svdxcgv.hxx>
#include <memory>

class SdrUndoGeoObj;

class SVXCORE_DLLPUBLIC SdrDragView : public SdrExchangeView
{
    friend class                SdrPageView;
    friend class                SdrDragMethod;

    // See GetDragXorPolyLimit/GetDragXorPointLimit
    enum : size_t {
        eDragXorPolyLimit = 100,
        eDragXorPointLimit = 500
    };

protected:
    SdrHdl*                     mpDragHdl;
    std::unique_ptr<SdrDragMethod> mpCurrentSdrDragMethod;
    SdrUndoGeoObj*              mpInsPointUndo;
    tools::Rectangle            maDragLimit;
    OUString                    maInsPointUndoStr;
    SdrHdlKind                  meDragHdl;

    bool                        mbFramDrag : 1;        // currently frame dragging
    bool                        mbMarkedHitMovesAlways : 1; // Persistent
    bool                        mbDragLimit : 1;      // Limit on SnapRect instead of BoundRect
    bool                        mbDragHdl : 1;        // TRUE: RefPt is slid
    bool                        mbDragStripes : 1;    // Persistent
    bool                        mbSolidDragging : 1;  // allow solid create/drag of objects
    bool                        mbResizeAtCenter : 1;
    bool                        mbCrookAtCenter : 1;
    bool                        mbDragWithCopy : 1;
    bool                        mbInsGluePoint : 1;
    bool                        mbInsObjPointMode : 1;
    bool                        mbInsGluePointMode : 1;
    bool                        mbNoDragXorPolys : 1;

protected:
    virtual void SetMarkHandles(SfxViewShell* pOtherShell) override;
    void ShowDragObj();
    void HideDragObj();
    bool ImpBegInsObjPoint(bool bIdxZwang, const Point& rPnt, bool bNewObj, OutputDevice* pOut);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrDragView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrDragView() override;

public:
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BckAction() override;
    virtual void BrkAction() override;
    virtual void TakeActionRect(tools::Rectangle& rRect) const override;

    // special implementation for Writer:
    // TakeDragObjAnchorPos() returns the position at which an object
    // approximately ends up during dragging when it is "released"
    // (EndDrag).
    // As a general rule, this is the left upper corner of the expected
    // new SnapRect. Exception: CaptionObj. There, it is the position
    // of the "tail end".
    // In case of return value 'false', the position could not be
    // determined (e.g. point shift, multiple selection, shift of the
    // mirror axis,...)
    bool TakeDragObjAnchorPos(Point& rPos, bool bTopRight ) const;

    // If pForcedMeth is passed, then pHdl, ... is not evaluated, but this Drag
    // method is used. In this, the ownership of the instance passes
    // to the View and is destroyed at the end of the dragging.
    virtual bool BegDragObj(const Point& rPnt, OutputDevice* pOut, SdrHdl* pHdl, short nMinMov=-3, SdrDragMethod* pForcedMeth=nullptr);
    void MovDragObj(const Point& rPnt);
    bool EndDragObj(bool bCopy=false);
    void BrkDragObj();
    bool IsDragObj() const { return mpCurrentSdrDragMethod && !mbInsPolyPoint && !mbInsGluePoint; }
    SdrHdl* GetDragHdl() const { return mpDragHdl; }
    SdrDragMethod* GetDragMethod() const { return mpCurrentSdrDragMethod.get(); }
    bool IsDraggingPoints() const { return meDragHdl==SdrHdlKind::Poly; }
    bool IsDraggingGluePoints() const { return meDragHdl==SdrHdlKind::Glue; }

    // If you want to define that already during BegDrag
    // or in the middle.
    // (Is reset to 'false' on each BegDrag, so set it after BegDrag.)
    void SetDragWithCopy(bool bOn) { mbDragWithCopy = bOn; }
    bool IsDragWithCopy() const { return mbDragWithCopy; }

    void SetInsertGluePoint(bool bOn) { mbInsGluePoint = bOn; }
    bool IsInsertGluePoint() const { return mbInsGluePoint; }

    // Interactive insertion of a new point. nIdx=0 => in front of the first point
    bool IsInsObjPointPossible() const;
    bool BegInsObjPoint(const Point& rPnt, bool bNewObj) { return ImpBegInsObjPoint(false, rPnt, bNewObj, nullptr); }
    void MovInsObjPoint(const Point& rPnt) { MovDragObj(rPnt); }
    bool EndInsObjPoint(SdrCreateCmd eCmd);
    bool IsInsObjPoint() const { return mpCurrentSdrDragMethod && mbInsPolyPoint; }

    // For the app to manage the status. GetPreferredPointer() is
    // possibly going to deliver a matching pointer for it.
    void SetInsObjPointMode(bool bOn) { mbInsObjPointMode = bOn; }
    bool IsInsObjPointMode() const { return mbInsObjPointMode; }

    bool IsInsGluePointPossible() const;
    bool BegInsGluePoint(const Point& rPnt);
    bool IsInsGluePoint() const { return mpCurrentSdrDragMethod && mbInsGluePoint; }

    // For the app to manage the status. GetPreferredPointer() is
    // possibly going to deliver a matching pointer for it.
    void SetInsGluePointMode(bool bOn) { mbInsGluePointMode = bOn; }
    bool IsInsGluePointMode() const { return mbInsGluePointMode; }

    // border lines over the whole win persistent during the
    // whole dragging. Default=FALSE.
    void SetDragStripes(bool bOn);
    bool IsDragStripes() const { return mbDragStripes; }

    // As a general rule, the contours of the selected objects
    // are displayed as Xor-polygons. If this flag is set, only one
    // Xor-Frame is drawn (e.g. in case of multiple selection).
    // In case of object-specific dragging (polygon points, corner radius,...),
    // this setting has no influence.
    // Also changeable during the dragging.
    // Default=Off
    void SetNoDragXorPolys(bool bOn);
    bool IsNoDragXorPolys() const { return mbNoDragXorPolys; }

    // If the number of selected objects exceeds the value set here,
    // NoDragPolys is (temporarily) activated implicitly.
    // PolyPolygons etc. are regarded as multiple objects respectively.
    static size_t GetDragXorPolyLimit() { return eDragXorPolyLimit; }

    // Like DragXorPolyLimit, but in respect to the total number of
    // all polygons.
    // NoDragPolys is (temporarily) activated, if one of the limits
    // is exceeded.
    static size_t GetDragXorPointLimit() { return eDragXorPointLimit; }

    void SetSolidDragging(bool bOn);
    bool IsSolidDragging() const;

    // Connector handling is thus as follows (when using default settings):
    // - If at most 10 Connectors are affected, they are recalculated
    //   on each MouseMove.
    // - If 11 to 100 Connectors are affected, the connections
    //   are shown as straight lines while dragging.
    // - In case of more than 100 affected Connectors, nothing that refers
    //   to the Connectors is drawn while dragging.

    // If a special drag mode like Rotate, Mirror or Crook is enabled,
    // then a Hit on the selected object triggers exactly this dragging.
    // If MarkedHitMovesAlways is set to 'true', a Hit on the selected
    // object always triggers a Move, independent of the DragMode that is
    // set. This flag is persistent and should be configurable in the app
    // by the user!
    void SetMarkedHitMovesAlways(bool bOn) { mbMarkedHitMovesAlways = bOn; }
    bool IsMarkedHitMovesAlways() const { return mbMarkedHitMovesAlways; }

    bool IsOrthoDesired() const;

    // center as reference on Resize
    // Default=FALSE.
    bool IsResizeAtCenter() const { return mbResizeAtCenter; }
    void SetResizeAtCenter(bool bOn) { mbResizeAtCenter = bOn; }

    // symmetric Crook
    // Default=FALSE.
    bool IsCrookAtCenter() const { return mbCrookAtCenter; }
    void SetCrookAtCenter(bool bOn) { mbCrookAtCenter = bOn; }

    // Limitation of the working area. The limitation refers to the View,
    // not to the single PageViews. This limitation is only evaluated by
    // the View on interactions like Dragging and Create.
    // In case of actions controlled by the app through algorithms or
    // UI-controlled actions (SetGeoAttr, MoveMarkedObj, ...), the
    // app must honor this limit itself.
    // Furthermore, this limit is to be seen as a rough limit. In certain
    // cases (e.g. while rotating), objects cannot be dragged exactly
    // up to this limit, objects can overlap a bit because of rounding
    // errors,...
    // Default=EmptyRect=no limitation
    // only partially implemented
    void SetWorkArea(const tools::Rectangle& rRect) { maMaxWorkArea=rRect; }
    const tools::Rectangle& GetWorkArea() const { return maMaxWorkArea; }


    // The DragLimit refers to the Page of the object.
    // (TODO or to the View?? - must be researched...)
    // 'false' = no limit
    // The return Rect must contain absolute coordinates. The maximum
    // drag area is then selected by the View in a way that the object's
    // SnapRect is moved or resized at most up to the corner of the
    // LimitRect. For objects like Bezier curves, rotated rectangles,
    // it must be taken into account that because of subsequent
    // recalculation of the SnapRect (on Resize), rounding errors can
    // occur, because of which the LimitRect might be exceeded by a
    // very small extent...
    // Implemented for Move and Resize
    virtual bool TakeDragLimit(SdrDragMode eMode, tools::Rectangle& rRect) const;
};

#endif // INCLUDED_SVX_SVDDRGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
