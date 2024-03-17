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

#ifndef INCLUDED_SVX_SVDEDTV_HXX
#define INCLUDED_SVX_SVDEDTV_HXX

#include <svx/svdmrkv.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdundo.hxx>
#include <o3tl/typed_flags_set.hxx>

class SfxUndoAction;
class SdrUndoAction;
class SdrUndoGroup;
class SfxStyleSheet;
class SdrLayer;
class SvdProgressInfo;

enum class SdrHorAlign  {
    NONE,
    Left,
    Right,
    Center
};

enum class SdrVertAlign {
    NONE,
    Top,
    Bottom,
    Center
};

enum class SdrMergeMode {
    Merge,
    Subtract,
    Intersect
};

// Options for InsertObject()
enum class SdrInsertFlags
{
    NONE        = 0x0000,
    DONTMARK    = 0x0001, /* object will not be marked (the actual marking remains) */
    ADDMARK     = 0x0002, /* object will be added an existing selection  */
    SETDEFATTR  = 0x0004, /* actual attributes (+StyleSheet) are assigned to the object */
    SETDEFLAYER = 0x0008, /* actual layer is assigned to the object */
};
namespace o3tl
{
    template<> struct typed_flags<SdrInsertFlags> : is_typed_flags<SdrInsertFlags, 0x0f> {};
}

class SVXCORE_DLLPUBLIC SdrEditView : public SdrMarkView
{
    friend class SdrPageView;
    friend class SdrDragDistort;
    friend class SdrDragCrook;

protected:

    // cache the transformation queries, etc. a little
    bool m_bPossibilitiesDirty : 1;
    bool m_bReadOnly : 1;
    bool m_bGroupPossible : 1;
    bool m_bUnGroupPossible : 1;
    bool m_bGrpEnterPossible : 1;
    bool m_bToTopPossible : 1;
    bool m_bToBtmPossible : 1;
    bool m_bReverseOrderPossible : 1;
    bool m_bImportMtfPossible : 1;
    bool m_bCombinePossible : 1;
    bool m_bDismantlePossible : 1;
    bool m_bCombineNoPolyPolyPossible : 1;
    bool m_bDismantleMakeLinesPossible : 1;
    bool m_bOrthoDesiredOnMarked : 1;
    bool m_bOneOrMoreMovable : 1;        // at least one object is moveable
    bool m_bMoreThanOneNoMovRot : 1;     // more than one object is not movable nor turnable (Crook)
    bool m_bContortionPossible : 1;      // all polygones (grouped if necessary)
    bool m_bMoveAllowed : 1;
    bool m_bResizeFreeAllowed : 1;
    bool m_bResizePropAllowed : 1;
    bool m_bRotateFreeAllowed : 1;
    bool m_bRotate90Allowed : 1;
    bool m_bMirrorFreeAllowed : 1;
    bool m_bMirror45Allowed : 1;
    bool m_bMirror90Allowed : 1;
    bool m_bShearAllowed : 1;
    bool m_bEdgeRadiusAllowed : 1;
    bool m_bTransparenceAllowed : 1;
    bool m_bCropAllowed : 1;
    bool m_bGradientAllowed : 1;
    bool m_bCanConvToPath : 1;
    bool m_bCanConvToPoly : 1;
    bool m_bCanConvToContour : 1;
    bool m_bMoveProtect : 1;
    bool m_bResizeProtect : 1;

private:
    SVX_DLLPRIVATE void ImpResetPossibilityFlags();

protected:
    SAL_DLLPRIVATE void ImpBroadcastEdgesOfMarkedNodes();

    // convert the objects marked in poly resp. bezier
    SAL_DLLPRIVATE void ImpConvertTo(bool bPath, bool bLineToArea);

    // converts an object, when positive it removes the old one from its List
    // and inserts the new one instead. including Undo.
    // Nor MarkEntry nor ModelChgBroadcast is created.
    SAL_DLLPRIVATE rtl::Reference<SdrObject> ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea);

    // set both flags: bToTopPossible and bToBtmPossible.
    // bToTopPossibleDirty and bToBtmPossibleDirty are reset at the same time
    SAL_DLLPRIVATE void ImpCheckToTopBtmPossible();

    // for CombineMarkedObjects and DismantleMarkedObjects
    SAL_DLLPRIVATE void ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const;

    // for CombineMarkedObjects
    SAL_DLLPRIVATE static bool ImpCanConvertForCombine1(const SdrObject* pObj);
    SAL_DLLPRIVATE static bool ImpCanConvertForCombine(const SdrObject* pObj);
    SAL_DLLPRIVATE static basegfx::B2DPolyPolygon ImpGetPolyPolygon1(const SdrObject* pObj);
    SAL_DLLPRIVATE static basegfx::B2DPolyPolygon ImpGetPolyPolygon(const SdrObject* pObj);
    SAL_DLLPRIVATE static basegfx::B2DPolygon ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);

    // for DismantleMarkedObjects
    SAL_DLLPRIVATE static bool ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPpolygon, bool bMakeLines);
    SAL_DLLPRIVATE static bool ImpCanDismantle(const SdrObject* pObj, bool bMakeLines);
    SAL_DLLPRIVATE void ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, size_t& rPos, SdrPageView* pPV, bool bMakeLines);
    SAL_DLLPRIVATE static void ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad, SdrCrookMode eMode,
        bool bVertical, bool bNoContortion, bool bRotate, const tools::Rectangle& rMarkRect);
    SAL_DLLPRIVATE static void ImpDistortObj(SdrObject* pO, const tools::Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion);
    SAL_DLLPRIVATE bool ImpDelLayerCheck(SdrObjList const * pOL, SdrLayerID nDelID) const;
    SAL_DLLPRIVATE void ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID);

    // Removes all objects of the MarkList from their ObjLists including Undo.
    // The entries in rMark remain.
    // @return a list of objects that must be deleted after the outermost EndUndo
    SAL_DLLPRIVATE std::vector<rtl::Reference<SdrObject>> DeleteMarkedList(SdrMarkList const& rMark); // DeleteMarked -> DeleteMarkedList

    // Check possibilities of all marked objects
    virtual void CheckPossibilities();
    void ForcePossibilities() const { if (m_bPossibilitiesDirty || mbSomeObjChgdFlag) const_cast<SdrEditView*>(this)->CheckPossibilities(); }

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SAL_DLLPRIVATE SdrEditView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    SAL_DLLPRIVATE virtual ~SdrEditView() override;

public:
    // each call of an undo-capable method from its view, generates an undo action.
    // If one wishes to group method calls into one, these calls should be put
    // between BegUndo() and EndUndo() calls (unlimited).
    // The comment used for the UndoAction is the first BegUndo(String).
    // In this case NotifyNewUndoAction is called at the last EndUndo().
    // NotifyNewUndoAction() is not called for an empty group
    void BegUndo()
    {
        // open undo-grouping
        GetModel().BegUndo();
    }
    void BegUndo(const OUString& rComment)
    {
        // open undo-grouping
        GetModel().BegUndo(rComment);
    }
    void BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc=SdrRepeatFunc::NONE)
    {
        // open undo-grouping
        GetModel().BegUndo(rComment,rObjDescr,eFunc);
    }

    void EndUndo(); // close undo-grouping  (incl. BroadcastEdges)

    void AddUndo(std::unique_ptr<SdrUndoAction> pUndo)
    {
        // add action
        GetModel().AddUndo(std::move(pUndo));
    }

    // only after first BegUndo or before last EndUndo:
    void SetUndoComment(const OUString& rComment, const OUString& rObjDescr)
    {
        GetModel().SetUndoComment(rComment,rObjDescr);
    }

    bool IsUndoEnabled() const;

    /**
     * Checks if this or other views have an active text edit, if true, end them.
     */
    void EndTextEditAllViews() const;
    void EndTextEditCurrentView(bool bDontDeleteReally = false);

    SAL_DLLPRIVATE std::vector< std::unique_ptr<SdrUndoAction> > CreateConnectorUndo( const SdrObject& rO );
    SAL_DLLPRIVATE void AddUndoActions( std::vector< std::unique_ptr<SdrUndoAction> > );

    // Layermanagement with Undo.
    void InsertNewLayer(const OUString& rName, sal_uInt16 nPos);
    // Delete a layer including all objects contained
    void DeleteLayer(const OUString& rName);

    // Marked objects which are outside a page
    // are assigned to another page; at the moment without undo!!!
    void ForceMarkedObjToAnotherPage();
    void ForceMarkedToAnotherPage()   { ForceMarkedObjToAnotherPage(); }

    // delete all marked objects
    void DeleteMarkedObj();

    // Set a logical enclosing rectangle for all marked objects.
    // It is not guaranteed if this succeeds, as a horizontal
    // line has always a height of 0
    void SetMarkedObjRect(const tools::Rectangle& rRect);
    void MoveMarkedObj(const Size& rSiz, bool bCopy=false);
    void ResizeMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    SAL_DLLPRIVATE void ResizeMultMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, const bool bWdh, const bool bHgt);
    SAL_DLLPRIVATE Degree100 GetMarkedObjRotate() const;
    void RotateMarkedObj(const Point& rRef, Degree100 nAngle, bool bCopy=false);
    SAL_DLLPRIVATE void MirrorMarkedObj(const Point& rRef1, const Point& rRef2, bool bCopy=false);
    void MirrorMarkedObjHorizontal();
    void MirrorMarkedObjVertical();
    SAL_DLLPRIVATE Degree100 GetMarkedObjShear() const;
    SAL_DLLPRIVATE void ShearMarkedObj(const Point& rRef, Degree100 nAngle, bool bVShear=false, bool bCopy=false);
    SAL_DLLPRIVATE void CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical, bool bNoContortion, bool bCopy=false);
    SAL_DLLPRIVATE void DistortMarkedObj(const tools::Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion, bool bCopy=false);

    // copy marked objects and mark them instead of the old ones
    void CopyMarkedObj();
    void SetAllMarkedRect(const tools::Rectangle& rRect) { SetMarkedObjRect(rRect); }
    void MoveAllMarked(const Size& rSiz, bool bCopy=false) { MoveMarkedObj(rSiz,bCopy); }
    void ResizeAllMarked(const Point& rRef, const Fraction& xFact, const Fraction& yFact) { ResizeMarkedObj(rRef,xFact,yFact); }
    void RotateAllMarked(const Point& rRef, Degree100 nAngle) { RotateMarkedObj(rRef,nAngle); }
    void MirrorAllMarkedHorizontal() { MirrorMarkedObjHorizontal(); }
    void MirrorAllMarkedVertical() { MirrorMarkedObjVertical(); }
    void CopyMarked() { CopyMarkedObj(); }
    bool IsMoveAllowed() const { ForcePossibilities(); return m_bMoveAllowed && !m_bMoveProtect; }
    bool IsResizeAllowed(bool bProp=false) const;
    bool IsRotateAllowed(bool b90Deg=false) const;
    bool IsMirrorAllowed(bool b45Deg=false, bool b90Deg=false) const;
    bool IsTransparenceAllowed() const;
    bool IsGradientAllowed() const;
    bool IsShearAllowed() const;
    bool IsEdgeRadiusAllowed() const;
    bool IsCrookAllowed(bool bNoContortion=false) const;
    SAL_DLLPRIVATE bool IsCropAllowed() const;
    bool IsDistortAllowed(bool bNoContortion=false) const;

    // Consolidate the text from multiple, selected TextObjects,
    // attempting to identify paragraph fragments and join them together
    void CombineMarkedTextObjects();

    // Unite several objects to a polygon:
    // - rectangles/circles/text... are implicitly converted.
    // - polygones are closed automatically
    // - attributes and layer are taken from the first object marked
    //   (thus from lowest Z-order).
    // - group objects are included when all (!) member objects of
    //   the group can be changed. If a group includes for example
    //   a bitmap or an OLE-object, the complete group is not considered.
    // bNoPolyPoly=TRUE: all is grouped to one single polygon
    void CombineMarkedObjects(bool bNoPolyPoly = true);

    // for combining multiple polygons, with direct support of the modes
    // SID_POLY_MERGE, SID_POLY_SUBSTRACT, SID_POLY_INTERSECT
    void MergeMarkedObjects(SdrMergeMode eMode);

    // for distribution dialog function
    void DistributeMarkedObjects(sal_uInt16 SlotID);

    // for setting either the width or height of all selected
    // objects to the width/height of the last selected object
    // of the selection
    void EqualizeMarkedObjects(bool bWidth);

    // Decompose marked polypolygon objects into polygons.
    // Grouped objects are searched and decomposed, if all member objects are PathObjs.
    // bMakeLines=TRUE:  all polygones are decomposed into single lines resp. bezier segments
    void DismantleMarkedObjects(bool bMakeLines=false);
    bool IsCombinePossible(bool bNoPolyPoly=false) const;
    bool IsDismantlePossible(bool bMakeLines=false) const;

    // Inserts a new, completely constructed object. Subsequently the object belongs to
    // the model. After insertion the object is marked (if not prevented by nOptions).
    // Sometimes the object is not inserted, but deleted, this is the case when
    // the target layer is locked or not visible. In this case
    // the method returns FALSE.
    // Amongst others the method does not create an undo-action.
    bool InsertObjectAtView(SdrObject* pObj, SdrPageView& rPV, SdrInsertFlags nOptions=SdrInsertFlags::NONE);

    // Replace one drawing object by another.
    // *pNewObj belongs to me, *pOldObj is changed into Undo.
    // In any case an undo grouping is required and should be applied, e.g.:
    // aStr+=" replace";
    // BegUndo(aStr);
    // ReplaceObject(...);

    // EndUndo();
    void ReplaceObjectAtView(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, bool bMark=true);

    SAL_DLLPRIVATE void SetNotPersistAttrToMarked(const SfxItemSet& rAttr);
    SAL_DLLPRIVATE void MergeNotPersistAttrFromMarked(SfxItemSet& rAttr) const;
    void MergeAttrFromMarked(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
    SfxItemSet GetAttrFromMarked(bool bOnlyHardAttr) const;
    void SetAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll);

    // geometrical attribute (position, size, rotation angle)
    // A PageOrigin set at a position is taken into account.
    SfxItemSet GetGeoAttrFromMarked() const;
    // In LOK, interactive shape movement uses this function
    // in that case, margin is not taken into account
    // and the final position of the shape becomes incorrect
    // However, "Position and Size" dialog and other cases already add the margins.
    void SetGeoAttrToMarked(const SfxItemSet& rAttr, bool addPageMargin = false);

    // Returns NULL if:
    // - nothing is marked,
    // - no stylesheet is set at the marked object
    // - point the marked objects to different StyleSheets for multiple selections
    SfxStyleSheet* GetStyleSheetFromMarked() const;

    // at the moment without undo :(
    SAL_DLLPRIVATE void SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    /* new interface src537 */
    SAL_DLLPRIVATE void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;

    SAL_DLLPRIVATE void SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SAL_DLLPRIVATE SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    SAL_DLLPRIVATE void SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    // Group all marked objects to a single group.
    // Subsequently mark the new group . If the group spawns multiple
    // pages a group is created per page.
    // All groups created are subsequently marked.
    // The method creates SdrObjGroup-instances.
    void GroupMarked();

    // All marked object groups are dissolved (1 level).
    // Now all previously marked member objects are marked.
    // Previously marked objects, which are not group objects, remain marked.
    void UnGroupMarked();

    bool IsGroupPossible() const { ForcePossibilities(); return m_bGroupPossible; }
    bool IsUnGroupPossible() const { ForcePossibilities(); return m_bUnGroupPossible; }
    bool IsGroupEnterPossible() const { ForcePossibilities(); return m_bGrpEnterPossible; }

    // Convert marked objects to polygones/Beziercurves. The bool-functions
    // return sal_True, if at least one marked object could be converted.
    // Also member objects of group objects are converted.
    // For a better description see: SdrObj.HXX
    bool IsConvertToPathObjPossible() const { ForcePossibilities(); return m_bCanConvToPath; }
    bool IsConvertToPolyObjPossible() const { ForcePossibilities(); return m_bCanConvToPoly; }
    bool IsConvertToContourPossible() const { ForcePossibilities(); return m_bCanConvToContour; }
    void ConvertMarkedToPathObj(bool bLineToArea);
    SAL_DLLPRIVATE void ConvertMarkedToPolyObj();

    // Align all marked objects vertically. Normally the SnapRect of an object is used.
    void AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert);
    bool IsAlignPossible() const;

    // move marked objects "up"
    void MovMarkedToTop();

    // move marked objects "down"
    void MovMarkedToBtm();

    // move marked objects "at top"
    void PutMarkedToTop();

    // move marked objects "at bottom"
    void PutMarkedToBtm();

    // move marked immediately before the object passed
    // NULL -> as PutMarkedToTop();
    void PutMarkedInFrontOfObj(const SdrObject* pRefObj);

    // move marked immediately after object passed
    // NULL -> as PutMarkedToBtm();
    void PutMarkedBehindObj(const SdrObject* pRefObj);

    // swap Z-Order of marked objects
    void ReverseOrderOfMarked();

    // Check if forward, backward is possible.
    // GetMaxToBtmObj() is only partly taken into account by these methods.
    // Which means it can happen that IsToTopPossible() returns sal_True,
    // but MovMarkedToTop() changes nothing (e.g. for multiple selections),
    // as restriction derived via a view by GetMaxToTopObj() prevents this.
    bool IsToTopPossible() const { ForcePossibilities(); return m_bToTopPossible; }
    bool IsToBtmPossible() const { ForcePossibilities(); return m_bToBtmPossible; }
    bool IsReverseOrderPossible() const { ForcePossibilities(); return m_bReverseOrderPossible; }

    // Using this method the view determines how far an object
    // can be moved forward or backward (Z-order).
    // The object returned is not "obsolete". When NULL is
    // returned there is not such a restriction.
    virtual SdrObject* GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject* GetMaxToBtmObj(SdrObject* pObj) const;

    // Next method is called, if via ToTop, ToBtm, ... the
    // sequence of object has been changed. It is called after
    // each SdrObjList::SetObjectOrdNum(nOldPos,nNewPos);
    virtual void ObjOrderChanged(SdrObject* pObj, size_t nOldPos, size_t nNewPos);

    // If one or more objects of the type SdrGrafObj or SdrOle2Obj
    // are marked and these are capable to deliver a StarView-metafile,
    // this methods converts the metafile to a drawing object.
    // The SdrGrafObjs/SdrOle2Objs are replaced by the new objects.
    void DoImportMarkedMtf(SvdProgressInfo *pProgrInfo=nullptr);
    bool IsImportMtfPossible() const { ForcePossibilities(); return m_bImportMtfPossible; }

    // override SdrMarkView, for internal use
    SAL_DLLPRIVATE virtual void MarkListHasChanged() override;
    SAL_DLLPRIVATE virtual void ModelHasChanged() override;
};

#endif // INCLUDED_SVX_SVDEDTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
