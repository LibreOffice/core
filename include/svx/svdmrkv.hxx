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

#ifndef _SVDMRKV_HXX
#define _SVDMRKV_HXX

#include <svx/svdmark.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdsnpv.hxx>
#include <svx/svdtypes.hxx>
#include "svx/svxdllapi.h"


// The following is not yet implemented, or just particially:
#define SDRSEARCH_DEEP         0x0001 /* recursive into group objects */
#define SDRSEARCH_ALSOONMASTER 0x0002 /* MasterPages are also scanned */
#define SDRSEARCH_WHOLEPAGE    0x0004 /* Not just the ObjList of PageView */
#define SDRSEARCH_TESTMARKABLE 0x0008 /* just markable Objekte/Punkte/Handles/... */
#define SDRSEARCH_TESTMACRO    0x0010 /* Just objects with macro */
#define SDRSEARCH_TESTTEXTEDIT 0x0020 /* Just TextEdit-enabed objects */
#define SDRSEARCH_WITHTEXT     0x0040 /* Just objects with text */
#define SDRSEARCH_TESTTEXTAREA 0x0080 /* The textarea of objects with text  (TextEditHit) */
#define SDRSEARCH_BACKWARD     0x0100 /* Backwards search */
#define SDRSEARCH_NEXT         0x0200 /* Search starts behind the transfered object/point/... */
#define SDRSEARCH_MARKED       0x0400 /* Just marked objects/points/... */
#define SDRSEARCH_PASS2BOUND   0x0800 /* In case of empty search results, then 2nd. try with BoundRectHit */
#define SDRSEARCH_PASS3NEAREST 0x1000 /* In case of empty search results, then new 3rd. Try with NearestBoundRectHit */
#define SDRSEARCH_BEFOREMARK   0x2000 /* if one marked one found, ignore all behind that */

#define SDRSEARCH_PICKMARKABLE (SDRSEARCH_TESTMARKABLE)
#define SDRSEARCH_PICKTEXTEDIT (SDRSEARCH_DEEP|SDRSEARCH_TESTMARKABLE|SDRSEARCH_TESTTEXTEDIT)
#define SDRSEARCH_PICKMACRO    (SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE|SDRSEARCH_TESTMACRO)

// SDRSEARCHPASS_... is return parameter value at PickObj().
#define SDRSEARCHPASS_DIRECT       0x0000 /* Object is hit by 'direct hit' */
#define SDRSEARCHPASS_INACTIVELIST 0x0001 /* Obj is on the page, but not in the AktGroup (at WHOLEPAGE) */
#define SDRSEARCHPASS_MASTERPAGE   0x0002 /* Object was found on the MasterPage */
#define SDRSEARCHPASS_BOUND        0x0010 /* Object found, butjust after Bound-Check gefunden   */
#define SDRSEARCHPASS_NEAREST      0x0020 /* Object found, but just after Nearest-Check */

enum SdrHitKind {SDRHIT_NONE,      // No hit
                 SDRHIT_OBJECT,    // Hit
                 SDRHIT_BOUNDRECT, // Hit at BoundRect
                 SDRHIT_BOUNDTL,   // Hit at BoundRect TopLeft
                 SDRHIT_BOUNDTC,   // Hit at BoundRect TopCenter
                 SDRHIT_BOUNDTR,   // Hit at BoundRect TopRight
                 SDRHIT_BOUNDCL,   // Hit at BoundRect CenterLeft
                 SDRHIT_BOUNDCR,   // Hit at BoundRect CenterRight
                 SDRHIT_BOUNDBL,   // Hit at BoundRect BottomLeft
                 SDRHIT_BOUNDBC,   // Hit at BoundRect BottomCenter
                 SDRHIT_BOUNDBR,/*,*/ // Hit at BoundRect BottomRight
                 /*SDRHIT_REFPOINT*/ // Reference point (Rotation axis, axis of reflextion) hit
                 SDRHIT_HANDLE,          // Marking handle
                 SDRHIT_HELPLINE,        // Reference line
                 SDRHIT_GLUEPOINT,       // Glue point
                 SDRHIT_TEXTEDIT,        // Open OutlinerView was hit
                 SDRHIT_TEXTEDITOBJ,     // Object for SdrBeginTextEdit (Textbereich)
                 SDRHIT_URLFIELD,        // Field im TextObj was hit (while it is currently not edited)
                 SDRHIT_MACRO,           // Object for BegMacroObj
                 SDRHIT_MARKEDOBJECT,    // Marked object (e.g. for dragging)
                 SDRHIT_UNMARKEDOBJECT, // non-marked Object (e.g. for marking)
                 SDRHIT_CELL};          // hit on a cell inside a table shape (outside of the cells text area)

enum SdrViewEditMode {SDREDITMODE_EDIT,           // Also known as arrow or pointer mode
                      SDREDITMODE_CREATE,         // Tool for object creation
                      SDREDITMODE_GLUEPOINTEDIT}; // Glue point editing mode

#define IMPSDR_MARKOBJDESCRIPTION    0x0000
#define IMPSDR_POINTSDESCRIPTION     0x0001
#define IMPSDR_GLUEPOINTSDESCRIPTION 0x0002


class ImplMarkingOverlay;

class SVX_DLLPUBLIC SdrMarkView : public SdrSnapView
{
    friend class                SdrPageView;

    // #114409#-3 Migrate selections
    ImplMarkingOverlay*                                 mpMarkObjOverlay;
    ImplMarkingOverlay*                                 mpMarkPointsOverlay;
    ImplMarkingOverlay*                                 mpMarkGluePointsOverlay;

protected:
    SdrObject*                  pMarkedObj;       // If not just one object ( i.e. More than one object ) is marked.
    SdrPageView*                pMarkedPV;        // If all marked obects are situated on the same PageView.

    Point                       aRef1;            // Persistent - Rotation center / axis of reflection
    Point                       aRef2;            // Persistent
    Point                       aLastCrookCenter; // Persistent
    SdrHdlList                  aHdl;
    sdr::ViewSelection*         mpSdrViewSelection;

    Rectangle                   aMarkedObjRect;
    Rectangle                   aMarkedObjRectNoOffset;
    Rectangle                   aMarkedPointsRect;
    Rectangle                   aMarkedGluePointsRect;

    sal_uInt16                      nFrameHandlesLimit;
    sal_uIntPtr                 mnInsPointNum;      // Number of the InsPoint
    sal_uIntPtr                     nMarkableObjCount;

    SdrDragMode                 eDragMode;        // Persistent
    SdrViewEditMode             eEditMode;      // Persistent
    SdrViewEditMode             eEditMode0;     // Persistent

    //HMHunsigned                   bHdlShown : 1;
    unsigned                    bRefHdlShownOnly : 1; // Axis of reflextion during dragging (ni)
    unsigned                    bDesignMode : 1;      // DesignMode for SdrUnoObj
    unsigned                    bForceFrameHandles : 1; // Persistent - FrameDrag auch bei Einzelobjekten
    unsigned                    bPlusHdlAlways : 1;   // Persistent
    unsigned                    bMarkHdlWhenTextEdit : 1; // Persistent, default=FALSE
    unsigned                    bInsPolyPoint : 1;     // at this time InsPolyPointDragging
    unsigned                    bMarkedObjRectDirty : 1;
    unsigned                    bMrkPntDirty : 1;
    unsigned                    bMarkedPointsRectsDirty : 1;
    unsigned                    bMarkableObjCountDirty : 1;

    // flag to completely disable handles at the view
    unsigned                    mbMarkHandlesHidden : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpSetPointsRects() const;
    void UndirtyMrkPnt() const;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
    virtual void ModelHasChanged(); // Is called by the PaintView
    virtual void SetMarkHandles();                                           // aHdl - fill (List of handles)
    virtual void SetMarkRects();                                             // Rects at the PageViews
    virtual void CheckMarked();                                              // Scan MarkList after Del and Lock Layer ...
    virtual void AddDragModeHdl(SdrDragMode eMode);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    void ForceRefToMarked();
    void ForceUndirtyMrkPnt() const                                       { if (bMrkPntDirty) UndirtyMrkPnt(); }

    //HMHvoid ImpShowMarkHdl(bool bNoRefHdl);
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uIntPtr nOptions, const SetOfByte* pMVisLay) const;
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, sal_uIntPtr nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const;
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, sal_uIntPtr nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj,const SdrMarkList * pMarkList) const;
    sal_Bool ImpIsFrameHandles() const;
    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal=0, sal_uInt16 nOpt=0) const;

    // Generates a string including degrees symbol, from an angel specification in 1/100deg
    sal_Bool ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, sal_Bool bUnmark);
    virtual sal_Bool MarkPoints(const Rectangle* pRect, sal_Bool bUnmark);
    sal_Bool MarkGluePoints(const Rectangle* pRect, sal_Bool bUnmark);

    void SetMoveOutside(sal_Bool bOn);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrMarkView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrMarkView();

public:
    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    virtual void ClearPageView();
    virtual void HideSdrPage();
    virtual sal_Bool IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    // Returns sal_True if objects, points or glue points are selected by drawing a frame
    // (as long as the frame is drawn).
    sal_Bool IsMarking() const { return IsMarkObj() || IsMarkPoints() || IsMarkGluePoints(); }

    // Marking objects by drawing of a selection frame
    sal_Bool BegMarkObj(const Point& rPnt, sal_Bool bUnmark = sal_False);
    void MovMarkObj(const Point& rPnt);
    sal_Bool EndMarkObj();
    void BrkMarkObj();
    sal_Bool IsMarkObj() const { return (0L != mpMarkObjOverlay); }

    // DragModes: SDRDRAG_CREATE,SDRDRAG_MOVE,SDRDRAG_RESIZE,SDRDRAG_ROTATE,SDRDRAG_MIRROR,SDRDRAG_SHEAR,SDRDRAG_CROOK
    // Move==Resize
    // The interface might maybe be changed in the future because of Ortho-Drag
    void SetDragMode(SdrDragMode eMode);
    SdrDragMode GetDragMode() const { return eDragMode; }
    sal_Bool ChkDragMode(SdrDragMode eMode) const;
    void SetFrameHandles(sal_Bool bOn);
    sal_Bool IsFrameHandles() const { return bForceFrameHandles; }

    // Limit. Exceeding the limit causes an implicite switch to FrameHandles. default=50.
    void SetFrameHandlesLimit(sal_uInt16 nAnz) { nFrameHandlesLimit=nAnz; }
    sal_uInt16 GetFrameHandlesLimit() const { return nFrameHandlesLimit; }

    void SetEditMode(SdrViewEditMode eMode);
    SdrViewEditMode GetEditMode() const { return eEditMode; }

    void SetEditMode(sal_Bool bOn=sal_True) { SetEditMode(bOn?SDREDITMODE_EDIT:SDREDITMODE_CREATE); }
    sal_Bool IsEditMode() const { return eEditMode==SDREDITMODE_EDIT; }
    void SetCreateMode(sal_Bool bOn=sal_True) { SetEditMode(bOn?SDREDITMODE_CREATE:SDREDITMODE_EDIT); }
    sal_Bool IsCreateMode() const { return eEditMode==SDREDITMODE_CREATE; }
    void SetGluePointEditMode(sal_Bool bOn=sal_True) { SetEditMode(bOn?SDREDITMODE_GLUEPOINTEDIT:eEditMode0); }
    sal_Bool IsGluePointEditMode() const { return eEditMode==SDREDITMODE_GLUEPOINTEDIT; }

    void SetDesignMode(sal_Bool bOn=sal_True);
    sal_Bool IsDesignMode() const { return bDesignMode; }

    void SetFrameDragSingles(sal_Bool bOn=sal_True) { SetFrameHandles(bOn); }
    sal_Bool IsFrameDragSingles() const { return IsFrameHandles(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// migrate selections

protected:
    // all available changing methods
    SdrMarkList& GetMarkedObjectListWriteAccess() { return mpSdrViewSelection->GetMarkedObjectListWriteAccess(); }
    void SetEdgesOfMarkedNodesDirty() { mpSdrViewSelection->SetEdgesOfMarkedNodesDirty(); }

public:
    // all available const methods for read access to selection
    const SdrMarkList& GetMarkedObjectList() const { return mpSdrViewSelection->GetMarkedObjectList(); }
    sal_uIntPtr TryToFindMarkedObject(const SdrObject* pObj) const { return GetMarkedObjectList().FindObject(pObj); }
    SdrPageView* GetSdrPageViewOfMarkedByIndex(sal_uIntPtr nNum) const { return GetMarkedObjectList().GetMark(nNum)->GetPageView(); }
    SdrMark* GetSdrMarkByIndex(sal_uIntPtr nNum) const { return GetMarkedObjectList().GetMark(nNum); }
    SdrObject* GetMarkedObjectByIndex(sal_uIntPtr nNum) const { return (GetMarkedObjectList().GetMark(nNum))->GetMarkedSdrObj(); }
    sal_uIntPtr GetMarkedObjectCount() const { return GetMarkedObjectList().GetMarkCount(); }
    void SortMarkedObjects() const { GetMarkedObjectList().ForceSort(); }
    sal_Bool AreObjectsMarked() const { return (0L != GetMarkedObjectList().GetMarkCount()); }
    OUString GetDescriptionOfMarkedObjects() const { return GetMarkedObjectList().GetMarkDescription(); }
    OUString GetDescriptionOfMarkedPoints() const { return GetMarkedObjectList().GetPointMarkDescription(); }
    OUString GetDescriptionOfMarkedGluePoints() const { return GetMarkedObjectList().GetGluePointMarkDescription(); }
    sal_Bool GetBoundRectFromMarkedObjects(SdrPageView* pPageView, Rectangle& rRect) const { return GetMarkedObjectList().TakeBoundRect(pPageView, rRect); }
    sal_Bool GetSnapRectFromMarkedObjects(SdrPageView* pPageView, Rectangle& rRect) const { return GetMarkedObjectList().TakeSnapRect(pPageView, rRect); }

    // Get a list of all those links which are connected to marked nodes,
    // but which are not marked themselves.
    const SdrMarkList& GetEdgesOfMarkedNodes() const { return mpSdrViewSelection->GetEdgesOfMarkedNodes(); }
    const SdrMarkList& GetMarkedEdgesOfMarkedNodes() const { return mpSdrViewSelection->GetMarkedEdgesOfMarkedNodes(); }
    const std::vector<SdrObject*>& GetTransitiveHullOfMarkedObjects() const { return mpSdrViewSelection->GetAllMarkedObjects(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

    // mechanism to complete disable handles at the view. Handles will be hidden and deleted
    // when set, no new ones created, no interaction allowed. Handles will be recreated and shown
    // when reset. Default is false.
    void hideMarkHandles();
    void showMarkHandles();
    bool areMarkHandlesHidden() const { return mbMarkHandlesHidden; }

    sal_Bool IsMarkedHit(const Point& rPnt, short nTol=-2) const { return IsMarkedObjHit(rPnt,nTol); }
    sal_Bool IsMarkedObjHit(const Point& rPnt, short nTol=-2) const;

    // Pick: Supported options for nOptions are SEARCH_NEXT, SEARCH_BACKWARD (ni)
    SdrHdl* PickHandle(const Point& rPnt, sal_uIntPtr nOptions=0, SdrHdl* pHdl0=NULL) const;

    // Pick: Supported options for nOptions are:
    // SDRSEARCH_DEEP SDRSEARCH_ALSOONMASTER SDRSEARCH_TESTMARKABLE SDRSEARCH_TESTTEXTEDIT
    // SDRSEARCH_WITHTEXT SDRSEARCH_TESTTEXTAREA SDRSEARCH_BACKWARD SDRSEARCH_MARKED
    // SDRSEARCH_WHOLEPAGE
    virtual sal_Bool PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr nOptions, SdrObject** ppRootObj, sal_uIntPtr* pnMarkNum=NULL, sal_uInt16* pnPassNum=NULL) const;
    virtual sal_Bool PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr nOptions=0) const;
    // sal_Bool PickObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr nOptions=0) const { return PickObj(rPnt,nHitTolLog,rpObj,rpPV,nOptions); }
    sal_Bool MarkObj(const Point& rPnt, short nTol=-2, sal_Bool bToggle=sal_False, sal_Bool bDeep=sal_False);

    // Pick: Supported options for nOptions are SDRSEARCH_PASS2BOUND und SDRSEARCH_PASS3NEAREST
    sal_Bool PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr* pnMarkNum=NULL, sal_uIntPtr nOptions=0) const;

    // Selects the most upper of the marked objects (O1) and scans from there
    // towards bottom direction, selecting the first non-marked object (O2).
    // In case of success the marking of O1 is deleted, a marking is created at
    // O2 and TRUE is returned. With the parameter bPrev=sal_True the scan
    // direction is turned to the other direction.
    sal_Bool MarkNextObj(sal_Bool bPrev=sal_False);

    // Selects the most upper of the marked objects which is hit by rPnt/nTol
    // and scans from there to bottom direction, selecting the first non-marked
    // object (O2). In case of success the marking of O1 is deleted, a marking
    // is created at O2 and sal_True is returned. With the parameter
    // bPrev=sal_True the scan direction is turned to the other direction.
    sal_Bool MarkNextObj(const Point& rPnt, short nTol=-2, sal_Bool bPrev=sal_False);

    // Mark all objects within a rectangular area
    // Just objects are marked which are inclosed completely
    sal_Bool MarkObj(const Rectangle& rRect, sal_Bool bUnmark=sal_False);
    void MarkObj(SdrObject* pObj, SdrPageView* pPV, sal_Bool bUnmark=sal_False, sal_Bool bImpNoSetMarkHdl=sal_False);
    void MarkAllObj(SdrPageView* pPV=NULL); // pPage=NULL => all displayed pages
    void UnmarkAllObj(SdrPageView* pPV=NULL); // pPage=NULL => all displayed pages

    // This function is time-consuming intensive, as the MarkList has to be scanned.
    sal_Bool IsObjMarked(SdrObject* pObj) const;
    // void MarkAll(SdrPageView* pPV=NULL) { MarkAllObj(pPV); } -> replace with inline
    void UnMarkAll(SdrPageView* pPV=NULL) { UnmarkAllObj(pPV); }

    // Request/set the size of the marking handles. Declaration in Pixel.
    // The value is meant to be the edge length ( link length ).
    // Pair values are round up to impair values: 3->3, 4->5, 5->5, 6->7, 7->7, ...
    // Default value is 7, minimum value is 3 Pixels.
    sal_uInt16 GetMarkHdlSizePixel() const;
    void SetMarkHdlSizePixel(sal_uInt16 nSiz);

    virtual sal_Bool HasMarkablePoints() const;
    virtual sal_uIntPtr GetMarkablePointCount() const;
    virtual sal_Bool HasMarkedPoints() const;
    virtual sal_uIntPtr GetMarkedPointCount() const;

    // There might be points which can't be marked:
    virtual sal_Bool IsPointMarkable(const SdrHdl& rHdl) const;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False);

    /** should only be used from outside svx for special ui elements */
    sal_Bool MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, sal_Bool bUnmark);

    // Mark all points within this rectangular alle Punkte (View coordinates)
    sal_Bool MarkPoints(const Rectangle& rRect, sal_Bool bUnmark=sal_False) { return MarkPoints(&rRect,bUnmark); }
    sal_Bool UnmarkPoint(SdrHdl& rHdl) { return MarkPoint(rHdl,sal_True); }
    sal_Bool UnMarkPoint(SdrHdl& rHdl) { return MarkPoint(rHdl,sal_True); }
    sal_Bool IsPointMarked(const SdrHdl& rHdl) const { ForceUndirtyMrkPnt(); return &rHdl!=NULL && rHdl.IsSelected(); }
    sal_Bool MarkAllPoints() { return MarkPoints(NULL,sal_False); }
    sal_Bool UnmarkAllPoints() { return MarkPoints(NULL,sal_True); }
    sal_Bool UnMarkAllPoints() { return MarkPoints(NULL,sal_True); }

    // Selects the first marked point (P1) which is hit by rPnt
    // and from there it searches the first non-marked point(P2).
    // In case of success the marking of
    // P1 is deleted, a mark is set at P2 and sal_True is returned.
    // With the parameter bPrev=sal_True the scan direction is turned to the other direction.
    sal_Bool MarkNextPoint(const Point& rPnt, sal_Bool bPrev=sal_False);

    // Search for the number of the suitable handle. In case of empty search result,
    // CONTAINER_ENTRY_NOTFOUND is returned.
    sal_uIntPtr GetHdlNum(SdrHdl* pHdl) const { return aHdl.GetHdlNum(pHdl); }
    SdrHdl* GetHdl(sal_uIntPtr nHdlNum)  const { return aHdl.GetHdl(nHdlNum); }
    const SdrHdlList& GetHdlList() const { return aHdl; }

    // Draw a selection frame for marking of points.
    // This routine will just be started in case that HasMarkablePoints() returns sal_True.
    sal_Bool BegMarkPoints(const Point& rPnt, sal_Bool bUnmark = sal_False);
    void MovMarkPoints(const Point& rPnt);
    sal_Bool EndMarkPoints();
    void BrkMarkPoints();
    sal_Bool IsMarkPoints() const { return (0L != mpMarkPointsOverlay); }

    // Select that additional handles are displayed permanently.
    void SetPlusHandlesAlwaysVisible(sal_Bool bOn);
    sal_Bool IsPlusHandlesAlwaysVisible() const { return bPlusHdlAlways; }

    // Are Handles visible during TextEdit (in double size)?
    // Persistent, default=FALSE
    void SetMarkHdlWhenTextEdit(sal_Bool bOn) { bMarkHdlWhenTextEdit=bOn; }
    sal_Bool IsMarkHdlWhenTextEdit() const { return bMarkHdlWhenTextEdit; }

    sal_Bool HasMarkableGluePoints() const;
    sal_Bool HasMarkedGluePoints() const;

    // A gluepoint is clearly identified by the SdrObject
    // (to which it belongs) as well as by a sal_uInt16 nId (as each SdrObject may consist of
    // several glue points. Here at the View there is an additional
    // SdrPageView, which should be defined correctly always.
    // Alternatively a gluepoint may be characterized by a SdrHdl.
    // In this case the SdrHdl instance consists of all required informations.
    // And in this case, the glue point are always is marked by enforcment
    // (Handlers are just situated at marked gluepoints )
    // Attention: With each change of the glue point status the handle list is re-calculated.
    // All previously saved SdrHdl* became invalid by this, the same with the point IDs!
    // Pick: Supported options for nOptions are SEARCH_NEXT, SEARCH_BACKWARD
    sal_Bool PickGluePoint(const Point& rPnt, SdrObject*& rpObj, sal_uInt16& rnId, SdrPageView*& rpPV, sal_uIntPtr nOptions=0) const;
    sal_Bool MarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, const SdrPageView* pPV, sal_Bool bUnmark=sal_False);
    sal_Bool UnmarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, const SdrPageView* pPV) { return MarkGluePoint(pObj,nId,pPV,sal_True); }
    sal_Bool IsGluePointMarked(const SdrObject* pObj, sal_uInt16 nId) const;

    // Get the Hdl (handle) of a marked GluePoint. Non-marked
    // GluePoints don`t have handles
    SdrHdl* GetGluePointHdl(const SdrObject* pObj, sal_uInt16 nId) const;
    sal_Bool IsGluePoint(const SdrHdl& rHdl) const { return &rHdl!=NULL && rHdl.GetKind()==HDL_GLUE; }

    // Mark all points within this rectangular (View coordinates)
    sal_Bool MarkGluePoints(const Rectangle& rRect) { return MarkGluePoints(&rRect,sal_False); }
    sal_Bool UnmarkGluePoints(const Rectangle& rRect) { return MarkGluePoints(&rRect,sal_True); }
    sal_Bool MarkAllGluePoints() { return MarkGluePoints(NULL,sal_False); }
    sal_Bool UnmarkAllGluePoints() { return MarkGluePoints(NULL,sal_True); }

    // Selects the first marked point (P1) which is hit by rPnt
    // and from there it searches the first non-marked point(P2).
    // In case of success the marking of
    // P1 is deleted, a mark is set at P2 and sal_True is returned.
    // With the parameter bPrev=sal_True the scan direction is turned to the other direction.
    sal_Bool MarkNextGluePoint(const Point& rPnt, sal_Bool bPrev=sal_False);

    // Draw a selection frame for glue point marking.
    // This routine will just be started in case that HasMarkablePoints() returns sal_True.
    // The GlueEditMode sal_True is disregarded.
    // sal_Bool BegMarkGluePoints(const Point& rPnt, OutputDevice* pOut);
    sal_Bool BegMarkGluePoints(const Point& rPnt, sal_Bool bUnmark = sal_False);
    void MovMarkGluePoints(const Point& rPnt);
    sal_Bool EndMarkGluePoints();
    void BrkMarkGluePoints();
    sal_Bool IsMarkGluePoints() const { return (0L != mpMarkGluePointsOverlay); }

    // bRestraintPaint=sal_False causes the handles not to be drawn immediately.
    // AdjustMarkHdl is just called in case of changes; usually this causes an Invalidate
    // At the end of a redraw the handles are drawn automatically.
    // The purpose is to avoid unnecessary flickering. -> This does not yet work, that's why sal_True!
    void AdjustMarkHdl(); //HMHBOOL bRestraintPaint=sal_True);

    const Rectangle& GetMarkedObjRect() const; // SnapRects of Objects, without line width
    Rectangle GetMarkedObjBoundRect() const;   // incl. line width, overlapping rags, ...
    const Rectangle& GetMarkedPointsRect() const;     // Enclosing rectangle of all marked points
    const Rectangle& GetMarkedGluePointsRect() const; // Enclosing rectangle of all marked glue points
    const Rectangle& GetAllMarkedRect() const { return GetMarkedObjRect(); }
    Rectangle GetAllMarkedBoundRect() const { return GetMarkedObjBoundRect(); }
    const Rectangle& GetAllMarkedPointsRect() const  { return GetMarkedPointsRect(); }
    Point GetGridOffset() const;

    // Will be always called, if the list of marked objects might be changed.
    // If you overload this method (as programmer), be shure that you call the
    // methods of the base class!
    virtual void MarkListHasChanged();

    // Entering (Editing) of a maybe marked object group. If there are several
    // object groups marked, the most upper group is selected. After that
    // all member objects of the group are directly accessable. All other
    // objects may not be processed in the meantime (until the next
    // LeaveGroup()). With markings which overlaps pages, every page is processed
    // separately. The method returns sal_True, if at least one group was entered.
    sal_Bool EnterMarkedGroup();

    // Get the center point of the last Crook-Dragging. Den kann man
    // bei einem anschliessenden Rotate sinnvoll als Drehmittelpunkt setzen.
    const Point& GetLastCrookCenter() const { return aLastCrookCenter; }

    // Is set by DragView automatically when finishing a Crook-Drag.
    void SetLastCrookCenter(const Point& rPt) { aLastCrookCenter=rPt; }

    // Rotation center point and start point of the axis of reflextion, respecively
    const Point& GetRef1() const { return aRef1; }
    void SetRef1(const Point& rPt);

    // End point of the axis of reflextion
    const Point& GetRef2() const { return aRef1; }
    void SetRef2(const Point& rPt);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// - Hit tolarances:
//   It have to be declared in logical coordinates. So please translate the
//   wanted pixel value with PixelToLogic in Logical values.
//   Taking as example a logical value of 100:
//   - For a horizontal hairline (Object with height 0), the generated data is +/-100, i.e.
//     a vertical area of 200 logical units is sensitive.
//   - For a polygon, a rectangular of the size (200,200) is generated and a
//     touch test between Poly and this Rect is processed.
//   - Obects which respond SdrObject::HasEdit()==TRUE ( e.g. a text frame ),
//     are specially treated: An additional sensitive area with a width of
//     2*Tol (200 units for this example) is created around the object.
//     When an object is directly hit, the Edit method is called.
//     In opposite, a hit in the surrounding sensitive area enables Dragging.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDMRKV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
