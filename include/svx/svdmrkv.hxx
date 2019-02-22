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

#ifndef INCLUDED_SVX_SVDMRKV_HXX
#define INCLUDED_SVX_SVDMRKV_HXX

#include <svx/svdmark.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdsnpv.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

class SfxViewShell;

// The following is not yet implemented, or just partially:
enum class SdrSearchOptions
{
    NONE         = 0x0000,
    DEEP         = 0x0001, /* recursive into group objects */
    ALSOONMASTER = 0x0002, /* MasterPages are also scanned */
    WHOLEPAGE    = 0x0004, /* Not just the ObjList of PageView */
    TESTMARKABLE = 0x0008, /* just markable objects/points/handles/... */
    TESTMACRO    = 0x0010, /* Just objects with macro */
    TESTTEXTEDIT = 0x0020, /* Just TextEdit-enabled objects */
    MARKED       = 0x0040, /* Just marked objects/points/... */
    PASS2BOUND   = 0x0080, /* In case of empty search results, then 2nd. try with BoundRectHit */
    BEFOREMARK   = 0x0100, /* if one marked one found, ignore all behind that */

    IMPISMASTER  = 0x0200, /* MasterPage is being searched right now */
    PICKMARKABLE = TESTMARKABLE,
    PICKTEXTEDIT = DEEP | TESTMARKABLE | TESTTEXTEDIT,
    PICKMACRO    = DEEP | ALSOONMASTER | WHOLEPAGE | TESTMACRO,
};
namespace o3tl
{
    template<> struct typed_flags<SdrSearchOptions> : is_typed_flags<SdrSearchOptions, 0x03ff> {};
}

enum class SdrHitKind
{
    NONE,            // No hit
    Object,          // Hit
    Handle,          // Marking handle
    HelpLine,        // Reference line
    Gluepoint,       // Glue point
    TextEdit,        // Open OutlinerView was hit
    TextEditObj,     // Object for SdrBeginTextEdit (Textbereich)
    UrlField,        // Field in TextObj was hit (while it is currently not edited)
    Macro,           // Object for BegMacroObj
    MarkedObject,    // Marked object (e.g. for dragging)
    UnmarkedObject,  // non-marked Object (e.g. for marking)
    Cell             // hit on a cell inside a table shape (outside of the cells text area)
};

enum class SdrViewEditMode {
    Edit,           // Also known as arrow or pointer mode
    Create,         // Tool for object creation
    GluePointEdit   // Glue point editing mode
};

/** options for ImpGetDescriptionString() */
enum class ImpGetDescriptionOptions
{
    NONE       = 0,
    POINTS     = 1,
    GLUEPOINTS = 2,
};

class ImplMarkingOverlay;

class SVX_DLLPUBLIC SdrMarkView : public SdrSnapView
{
    friend class                SdrPageView;

    // #114409#-3 Migrate selections
    ImplMarkingOverlay*                                 mpMarkObjOverlay;
    ImplMarkingOverlay*                                 mpMarkPointsOverlay;
    ImplMarkingOverlay*                                 mpMarkGluePointsOverlay;

protected:
    SdrObject*                  mpMarkedObj;       // If not just one object ( i.e. More than one object ) is marked.
    SdrPageView*                mpMarkedPV;        // If all marked obects are situated on the same PageView.

    Point                       maRef1;            // Persistent - Rotation center / axis of reflection
    Point                       maRef2;            // Persistent
    SdrHdlList                  maHdlList;
    sdr::ViewSelection          maSdrViewSelection;

    tools::Rectangle            maMarkedObjRect;
    tools::Rectangle            maMarkedPointsRect;
    tools::Rectangle            maMarkedGluePointsRect;

    sal_uInt16                  mnFrameHandlesLimit;

    SdrDragMode                 meDragMode;      // Persistent
    SdrViewEditMode             meEditMode;      // Persistent
    SdrViewEditMode             meEditMode0;     // Persistent

    bool                        mbDesignMode : 1;          // DesignMode for SdrUnoObj
    bool                        mbForceFrameHandles : 1;   // Persistent - FrameDrag also for single objects
    bool                        mbPlusHdlAlways : 1;       // Persistent
    bool                        mbInsPolyPoint : 1;        // at this time InsPolyPointDragging
    bool                        mbMarkedObjRectDirty : 1;
    bool                        mbMrkPntDirty : 1;
    bool                        mbMarkedPointsRectsDirty : 1;

    // flag to completely disable handles at the view
    bool                        mbMarkHandlesHidden : 1;

    // Helper to get a possible GridOffset from SdrObject
    bool getPossibleGridOffsetForSdrObject(
        basegfx::B2DVector& rOffset,
        const SdrObject* pObj,
        const SdrPageView* pPV) const;

    // Helper to get a possible GridOffset from Position
    bool getPossibleGridOffsetForPosition(
        basegfx::B2DVector& rOffset,
        const basegfx::B2DPoint& rPoint,
        const SdrPageView* pPV) const;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpSetPointsRects() const;
    void UndirtyMrkPnt() const;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    virtual void ModelHasChanged() override; // Is called by the PaintView
    virtual void SetMarkHandles(SfxViewShell* pOtherShell); // maHdlList - fill (List of handles)
    void         SetMarkRects();                                             // Rects at the PageViews
    void         CheckMarked();                                              // Scan MarkList after Del and Lock Layer ...
    void         AddDragModeHdl(SdrDragMode eMode);
    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool RequestHelp(const HelpEvent& rHEvt) override;

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    void ForceRefToMarked();
    void ForceUndirtyMrkPnt() const                                       { if (mbMrkPntDirty) UndirtyMrkPnt(); }

    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay) const;
    SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList const * pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay, SdrObject*& rpRootObj) const;
    SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList const * pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay, SdrObject*& rpRootObj,const SdrMarkList * pMarkList) const;
    bool ImpIsFrameHandles() const;
    OUString ImpGetDescriptionString(const char* pStrCacheID, ImpGetDescriptionOptions nOpt=ImpGetDescriptionOptions::NONE) const;

    // Generates a string including degrees symbol, from an angel specification in 1/100deg
    bool ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark);
    virtual bool MarkPoints(const tools::Rectangle* pRect, bool bUnmark);
    bool MarkGluePoints(const tools::Rectangle* pRect, bool bUnmark);

    void SetMoveOutside(bool bOn);
    bool MarkableObjectsExceed( int n ) const;

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrMarkView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrMarkView() override;

public:
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BckAction() override;
    virtual void BrkAction() override;
    virtual void TakeActionRect(tools::Rectangle& rRect) const override;

    virtual void ClearPageView() override;
    virtual void HideSdrPage() override;
    bool IsObjMarkable(SdrObject const * pObj, SdrPageView const * pPV) const;

    // Returns sal_True if objects, points or glue points are selected by drawing a frame
    // (as long as the frame is drawn).
    bool IsMarking() const { return IsMarkObj() || IsMarkPoints() || IsMarkGluePoints(); }

    // Marking objects by drawing of a selection frame
    void BegMarkObj(const Point& rPnt, bool bUnmark = false);
    void MovMarkObj(const Point& rPnt);
    bool EndMarkObj();
    void BrkMarkObj();
    bool IsMarkObj() const { return (nullptr != mpMarkObjOverlay); }

    // DragModes: SDRDRAG_CREATE,SdrDragMode::Move,SdrDragMode::Resize,SdrDragMode::Rotate,SdrDragMode::Mirror,SdrDragMode::Shear,SdrDragMode::Crook
    // Move==Resize
    // The interface might maybe be changed in the future because of Ortho-Drag
    void SetDragMode(SdrDragMode eMode);
    SdrDragMode GetDragMode() const { return meDragMode; }
    void SetFrameHandles(bool bOn);
    bool IsFrameHandles() const { return mbForceFrameHandles; }

    // returns true if number of markable objects is greater than 1
    bool HasMultipleMarkableObjects() const { return MarkableObjectsExceed(1); };

    void SetEditMode(SdrViewEditMode eMode);
    SdrViewEditMode GetEditMode() const { return meEditMode; }

    void SetEditMode(bool bOn) { SetEditMode(bOn?SdrViewEditMode::Edit:SdrViewEditMode::Create); }
    bool IsEditMode() const { return meEditMode==SdrViewEditMode::Edit; }
    void SetCreateMode(bool bOn) { SetEditMode(bOn?SdrViewEditMode::Create:SdrViewEditMode::Edit); }
    bool IsCreateMode() const { return meEditMode==SdrViewEditMode::Create; }
    void SetGluePointEditMode(bool bOn) { SetEditMode(bOn?SdrViewEditMode::GluePointEdit:meEditMode0); }
    bool IsGluePointEditMode() const { return meEditMode==SdrViewEditMode::GluePointEdit; }

    void SetDesignMode(bool bOn = true);
    bool IsDesignMode() const { return mbDesignMode; }

    void SetFrameDragSingles(bool bOn=true) { SetFrameHandles(bOn); }
    bool IsFrameDragSingles() const { return IsFrameHandles(); }

    bool HasMarkableObj() const { return MarkableObjectsExceed(0); };


// migrate selections

protected:
    // all available changing methods
    SdrMarkList& GetMarkedObjectListWriteAccess() { return maSdrViewSelection.GetMarkedObjectListWriteAccess(); }

public:
    // all available const methods for read access to selection
    const SdrMarkList& GetMarkedObjectList() const { return maSdrViewSelection.GetMarkedObjectList(); }
    // returns SAL_MAX_SIZE if not found
    size_t TryToFindMarkedObject(const SdrObject* pObj) const { return GetMarkedObjectList().FindObject(pObj); }
    SdrPageView* GetSdrPageViewOfMarkedByIndex(size_t nNum) const { return GetMarkedObjectList().GetMark(nNum)->GetPageView(); }
    SdrMark* GetSdrMarkByIndex(size_t nNum) const { return GetMarkedObjectList().GetMark(nNum); }
    SdrObject* GetMarkedObjectByIndex(size_t nNum) const { return GetMarkedObjectList().GetMark(nNum)->GetMarkedSdrObj(); }
    size_t GetMarkedObjectCount() const { return GetMarkedObjectList().GetMarkCount(); }
    void SortMarkedObjects() const { GetMarkedObjectList().ForceSort(); }
    bool AreObjectsMarked() const { return 0 != GetMarkedObjectList().GetMarkCount(); }
    OUString const & GetDescriptionOfMarkedObjects() const { return GetMarkedObjectList().GetMarkDescription(); }
    OUString const & GetDescriptionOfMarkedPoints() const { return GetMarkedObjectList().GetPointMarkDescription(); }
    OUString const & GetDescriptionOfMarkedGluePoints() const { return GetMarkedObjectList().GetGluePointMarkDescription(); }

    // Get a list of all those links which are connected to marked nodes,
    // but which are not marked themselves.
    const SdrMarkList& GetEdgesOfMarkedNodes() const { return maSdrViewSelection.GetEdgesOfMarkedNodes(); }
    const SdrMarkList& GetMarkedEdgesOfMarkedNodes() const { return maSdrViewSelection.GetMarkedEdgesOfMarkedNodes(); }
    const std::vector<SdrObject*>& GetTransitiveHullOfMarkedObjects() const { return maSdrViewSelection.GetAllMarkedObjects(); }


    // mechanism to complete disable handles at the view. Handles will be hidden and deleted
    // when set, no new ones created, no interaction allowed. Handles will be recreated and shown
    // when reset. Default is false.
    void hideMarkHandles();
    void showMarkHandles();
    bool areMarkHandlesHidden() const { return mbMarkHandlesHidden; }

    bool IsMarkedHit(const Point& rPnt, short nTol=-2) const { return IsMarkedObjHit(rPnt,nTol); }
    bool IsMarkedObjHit(const Point& rPnt, short nTol=-2) const;

    SdrHdl* PickHandle(const Point& rPnt) const;

    // Pick: Supported options for nOptions are:
    // SdrSearchOptions::DEEP SdrSearchOptions::ALSOONMASTER SdrSearchOptions::TESTMARKABLE SdrSearchOptions::TESTTEXTEDIT
    // SdrSearchOptions::MARKED
    // SdrSearchOptions::WHOLEPAGE
    SdrObject* PickObj(const Point& rPnt, short nTol, SdrPageView*& rpPV, SdrSearchOptions nOptions, SdrObject** ppRootObj, bool* pbHitPassDirect=nullptr) const;
    SdrObject* PickObj(const Point& rPnt, short nTol, SdrPageView*& rpPV, SdrSearchOptions nOptions=SdrSearchOptions::NONE) const;
    bool MarkObj(const Point& rPnt, short nTol=-2, bool bToggle=false, bool bDeep=false);

    // Pick: Supported options for nOptions are SdrSearchOptions::PASS2BOUND
    bool PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const;

    // Selects the most upper of the marked objects (O1) and scans from there
    // towards bottom direction, selecting the first non-marked object (O2).
    // In case of success the marking of O1 is deleted, a marking is created at
    // O2 and TRUE is returned. With the parameter bPrev=sal_True the scan
    // direction is turned to the other direction.
    bool MarkNextObj(bool bPrev=false);

    // Selects the most upper of the marked objects which is hit by rPnt/nTol
    // and scans from there to bottom direction, selecting the first non-marked
    // object (O2). In case of success the marking of O1 is deleted, a marking
    // is created at O2 and sal_True is returned. With the parameter
    // bPrev=sal_True the scan direction is turned to the other direction.
    bool MarkNextObj(const Point& rPnt, short nTol, bool bPrev);

    // Mark all objects within a rectangular area
    // Just objects are marked which are inclosed completely
    void MarkObj(const tools::Rectangle& rRect, bool bUnmark);
    void MarkObj(SdrObject* pObj, SdrPageView* pPV, bool bUnmark=false, bool bImpNoSetMarkHdl=false);
    void MarkAllObj(SdrPageView* pPV=nullptr); // pPage=NULL => all displayed pages
    void UnmarkAllObj(SdrPageView const * pPV=nullptr); // pPage=NULL => all displayed pages

    // This function is time-consuming intensive, as the MarkList has to be scanned.
    bool IsObjMarked(SdrObject const * pObj) const;
    void UnMarkAll(SdrPageView const * pPV=nullptr) { UnmarkAllObj(pPV); }

    // Request/set the size of the marking handles. Declaration in Pixel.
    // The value is meant to be the edge length ( link length ).
    // Pair values are round up to impair values: 3->3, 4->5, 5->5, 6->7, 7->7, ...
    // Default value is 7, minimum value is 3 Pixels.
    sal_uInt16 GetMarkHdlSizePixel() const;
    void SetMarkHdlSizePixel(sal_uInt16 nSiz);

    virtual bool HasMarkablePoints() const;
    virtual sal_Int32 GetMarkablePointCount() const;
    virtual bool HasMarkedPoints() const;

    // There might be points which can't be marked:
    virtual bool IsPointMarkable(const SdrHdl& rHdl) const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark=false);

    /** should only be used from outside svx for special ui elements */
    bool MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark);

    bool UnmarkPoint(SdrHdl& rHdl) { return MarkPoint(rHdl,true); }
    bool IsPointMarked(const SdrHdl& rHdl) const { ForceUndirtyMrkPnt(); return rHdl.IsSelected(); }
    bool MarkAllPoints() { return MarkPoints(nullptr,false); }
    bool UnmarkAllPoints() { return MarkPoints(nullptr,true); }

    // Selects the first marked point (P1) which is hit by rPnt
    // and from there it searches the first non-marked point(P2).
    // In case of success the marking of
    // P1 is deleted, a mark is set at P2.
    void MarkNextPoint();

    // Search for the number of the suitable handle. In case of empty search result,
    // SAL_MAX_SIZE is returned.
    size_t GetHdlNum(SdrHdl const * pHdl) const { return maHdlList.GetHdlNum(pHdl); }
    SdrHdl* GetHdl(size_t nHdlNum)  const { return maHdlList.GetHdl(nHdlNum); }
    const SdrHdlList& GetHdlList() const { return maHdlList; }

    // Draw a selection frame for marking of points.
    // This routine will just be started in case that HasMarkablePoints() returns sal_True.
    bool BegMarkPoints(const Point& rPnt, bool bUnmark = false);
    void MovMarkPoints(const Point& rPnt);
    bool EndMarkPoints();
    void BrkMarkPoints();
    bool IsMarkPoints() const { return (nullptr != mpMarkPointsOverlay); }

    // Select that additional handles are displayed permanently.
    void SetPlusHandlesAlwaysVisible(bool bOn);
    bool IsPlusHandlesAlwaysVisible() const { return mbPlusHdlAlways; }

    bool HasMarkableGluePoints() const;
    bool HasMarkedGluePoints() const;

    // A gluepoint is clearly identified by the SdrObject
    // (to which it belongs) as well as by a sal_uInt16 nId (as each SdrObject may consist of
    // several glue points. Here at the View there is an additional
    // SdrPageView, which should be defined correctly always.
    // Alternatively a gluepoint may be characterized by a SdrHdl.
    // In this case the SdrHdl instance consists of all required information.
    // And in this case, the glue point are always is marked by enforcement
    // (Handlers are just situated at marked gluepoints )
    // Attention: With each change of the glue point status the handle list is re-calculated.
    // All previously saved SdrHdl* became invalid by this, the same with the point IDs!
    bool PickGluePoint(const Point& rPnt, SdrObject*& rpObj, sal_uInt16& rnId, SdrPageView*& rpPV) const;
    bool MarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, bool bUnmark);
    void UnmarkGluePoint(const SdrObject* pObj, sal_uInt16 nId) { MarkGluePoint(pObj,nId,true); }
    bool IsGluePointMarked(const SdrObject* pObj, sal_uInt16 nId) const;

    // Get the Hdl (handle) of a marked GluePoint. Non-marked
    // GluePoints don`t have handles
    SdrHdl* GetGluePointHdl(const SdrObject* pObj, sal_uInt16 nId) const;

    // Mark all points within this rectangular (View coordinates)
    bool MarkAllGluePoints() { return MarkGluePoints(nullptr,false); }
    bool UnmarkAllGluePoints() { return MarkGluePoints(nullptr,true); }

    // Selects the first marked point (P1) which is hit by rPnt
    // and from there it searches the first non-marked point(P2).
    // In case of success the marking of
    // P1 is deleted, a mark is set at P2.
    void MarkNextGluePoint();

    // Draw a selection frame for glue point marking.
    // This routine will just be started in case that HasMarkablePoints() returns sal_True.
    // The GlueEditMode sal_True is disregarded.
    // bool BegMarkGluePoints(const Point& rPnt, OutputDevice* pOut);
    bool BegMarkGluePoints(const Point& rPnt, bool bUnmark = false);
    void MovMarkGluePoints(const Point& rPnt);
    void EndMarkGluePoints();
    void BrkMarkGluePoints();
    bool IsMarkGluePoints() const { return (nullptr != mpMarkGluePointsOverlay); }

    // bRestraintPaint=sal_False causes the handles not to be drawn immediately.
    // AdjustMarkHdl is just called in case of changes; usually this causes an Invalidate
    // At the end of a redraw the handles are drawn automatically.
    // The purpose is to avoid unnecessary flickering. -> This does not yet work, that's why sal_True!
    void AdjustMarkHdl(SfxViewShell* pOtherShell = nullptr); //HMHBOOL bRestraintPaint=sal_True);

    const tools::Rectangle& GetMarkedObjRect() const; // SnapRects of Objects, without line width
    tools::Rectangle GetMarkedObjBoundRect() const;   // incl. line width, overlapping rags, ...
    const tools::Rectangle& GetMarkedPointsRect() const;     // Enclosing rectangle of all marked points
    const tools::Rectangle& GetMarkedGluePointsRect() const; // Enclosing rectangle of all marked glue points
    const tools::Rectangle& GetAllMarkedRect() const { return GetMarkedObjRect(); }
    tools::Rectangle GetAllMarkedBoundRect() const { return GetMarkedObjBoundRect(); }

    // Will be always called, if the list of marked objects might be changed.
    // If you override this method, be sure that you call the
    // methods of the base class!
    virtual void MarkListHasChanged();

    // Entering (Editing) of a maybe marked object group. If there are several
    // object groups marked, the most upper group is selected. After that
    // all member objects of the group are directly accessible. All other
    // objects may not be processed in the meantime (until the next
    // LeaveGroup()). With markings which overlaps pages, every page is processed
    // separately. The method returns sal_True, if at least one group was entered.
    void EnterMarkedGroup();

    // Rotation center point and start point of the axis of reflextion, respectively
    const Point& GetRef1() const { return maRef1; }
    void SetRef1(const Point& rPt);

    // End point of the axis of reflextion
    const Point& GetRef2() const { return maRef2; }
    void SetRef2(const Point& rPt);
    /// Get access to the view shell owning this draw view, if any.
    virtual SfxViewShell* GetSfxViewShell() const;
};


// - Hit tolarances:
//   It has to be declared in logical coordinates. So please translate the
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


#endif // INCLUDED_SVX_SVDMRKV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
