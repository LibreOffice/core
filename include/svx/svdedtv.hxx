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
    NOBROADCAST = 0x0010, /* insert with NbcInsertObject() for SolidDragging */
};
namespace o3tl
{
    template<> struct typed_flags<SdrInsertFlags> : is_typed_flags<SdrInsertFlags, 0x1f> {};
}

class SVX_DLLPUBLIC SdrEditView : public SdrMarkView
{
    friend class                SdrPageView;
    friend class                SdrDragDistort;
    friend class                SdrDragCrook;

protected:

    // cache the transformation queries, etc. a little
    bool                        bPossibilitiesDirty : 1;
    bool                        bReadOnly : 1;
    bool                        bGroupPossible : 1;
    bool                        bUnGroupPossible : 1;
    bool                        bGrpEnterPossible : 1;
    bool                        bToTopPossible : 1;
    bool                        bToBtmPossible : 1;
    bool                        bReverseOrderPossible : 1;
    bool                        bImportMtfPossible : 1;
    bool                        bCombinePossible : 1;
    bool                        bDismantlePossible : 1;
    bool                        bCombineNoPolyPolyPossible : 1;
    bool                        bDismantleMakeLinesPossible : 1;
    bool                        bOrthoDesiredOnMarked : 1;
    bool                        bOneOrMoreMovable : 1;        // at least one object is moveable
    bool                        bMoreThanOneNoMovRot : 1;     // more then one object is not movable nor turnable (Crook)
    bool                        bContortionPossible : 1;      // all polygones (grouped if necessary)
    bool                        bMoveAllowed : 1;
    bool                        bResizeFreeAllowed : 1;
    bool                        bResizePropAllowed : 1;
    bool                        bRotateFreeAllowed : 1;
    bool                        bRotate90Allowed : 1;
    bool                        bMirrorFreeAllowed : 1;
    bool                        bMirror45Allowed : 1;
    bool                        bMirror90Allowed : 1;
    bool                        bShearAllowed : 1;
    bool                        bEdgeRadiusAllowed : 1;
    bool                        bTransparenceAllowed : 1;
    bool                        bCropAllowed : 1;
    bool                        bGradientAllowed : 1;
    bool                        bCanConvToPath : 1;
    bool                        bCanConvToPoly : 1;
    bool                        bCanConvToContour : 1;
    bool                        bMoveProtect : 1;
    bool                        bResizeProtect : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpResetPossibilityFlags();

protected:
    void ImpBroadcastEdgesOfMarkedNodes();

    // convert the objects marked in poly resp. bezier
    void ImpConvertTo(bool bPath, bool bLineToArea);

    // converts an object, when positive it removes the old one from its List
    // and inserts the new one instead. including Undo.
    // Nor MarkEntry nor ModelChgBroadcast is created.
    SdrObject* ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea);

    // set both flags: bToTopPossible and bToBtmPossible.
    // bToTopPossibleDirty and bToBtmPossibleDirty are reset at same time
    void ImpCheckToTopBtmPossible();

    // for CombineMarkedObjects and DismantleMarkedObjects
    void ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const;

    // for CombineMarkedObjects
    static bool ImpCanConvertForCombine1(const SdrObject* pObj);
    static bool ImpCanConvertForCombine(const SdrObject* pObj);
    static basegfx::B2DPolyPolygon ImpGetPolyPolygon1(const SdrObject* pObj);
    static basegfx::B2DPolyPolygon ImpGetPolyPolygon(const SdrObject* pObj);
    static basegfx::B2DPolygon ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);

    // for DismantleMarkedObjects
    static bool ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPpolygon, bool bMakeLines);
    static bool ImpCanDismantle(const SdrObject* pObj, bool bMakeLines);
    void ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, size_t& rPos, SdrPageView* pPV, bool bMakeLines);
    static void ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad, SdrCrookMode eMode,
        bool bVertical, bool bNoContortion, bool bRotate, const tools::Rectangle& rMarkRect);
    static void ImpDistortObj(SdrObject* pO, const tools::Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion);
    bool ImpDelLayerCheck(SdrObjList const * pOL, SdrLayerID nDelID) const;
    void ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID);

    // Removes all objects of the MarkList from their ObjLists including Undo.
    // The entries in rMark remain.
    // @return a list of objects that must be deleted after the outermost EndUndo
    std::vector<SdrObject *> DeleteMarkedList(SdrMarkList const& rMark); // DeleteMarked -> DeleteMarkedList

    // Check possibilities of all marked objects
    virtual void CheckPossibilities();
    void ForcePossibilities() const { if (bPossibilitiesDirty || mbSomeObjChgdFlag) const_cast<SdrEditView*>(this)->CheckPossibilities(); }

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrEditView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrEditView() override;

public:
    // each call of an undo-capable method from its view, generates an undo action.
    // If one wishes to group method calls into one, these calls should be put
    // between BegUndo() and EndUndo() calls (unlimited).
    // The comment used for the UndoAction is the first BegUndo(String).
    // In this case NotifyNewUndoAction is called at the last EndUndo().
    // NotifyNewUndoAction() is not called for an empty group.
    void BegUndo()                         { mpModel->BegUndo();         } // open undo-grouping
    void BegUndo(const OUString& rComment) { mpModel->BegUndo(rComment); } // open undo-grouping
    void BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc=SdrRepeatFunc::NONE) { mpModel->BegUndo(rComment,rObjDescr,eFunc); } // open undo-grouping
    void EndUndo();                                                   // close undo-grouping  (incl. BroadcastEdges)
    void AddUndo(SdrUndoAction* pUndo)   { mpModel->AddUndo(pUndo);    } // add action
    // only after first BegUndo or before last EndUndo:
    void SetUndoComment(const OUString& rComment, const OUString& rObjDescr) { mpModel->SetUndoComment(rComment,rObjDescr); }
    bool IsUndoEnabled() const;

    std::vector< SdrUndoAction* > CreateConnectorUndo( SdrObject& rO );
    void AddUndoActions( std::vector< SdrUndoAction* >& );

    // Layermanagement with Undo.
    void InsertNewLayer(const OUString& rName, sal_uInt16 nPos);
    // Delete a layer including all objects contained
    void DeleteLayer(const OUString& rName);

    // Marked objects which are outside a page
    // are assigned to an other page; at the moment without undo!!!
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
    void ResizeMultMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, const bool bWdh, const bool bHgt);
    long GetMarkedObjRotate() const;
    void RotateMarkedObj(const Point& rRef, long nAngle, bool bCopy=false);
    void MirrorMarkedObj(const Point& rRef1, const Point& rRef2, bool bCopy=false);
    void MirrorMarkedObjHorizontal();
    void MirrorMarkedObjVertical();
    long GetMarkedObjShear() const;
    void ShearMarkedObj(const Point& rRef, long nAngle, bool bVShear=false, bool bCopy=false);
    void CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical, bool bNoContortion, bool bCopy=false);
    void DistortMarkedObj(const tools::Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion, bool bCopy=false);

    // copy marked objects and mark them instead of the old ones
    void CopyMarkedObj();
    void SetAllMarkedRect(const tools::Rectangle& rRect) { SetMarkedObjRect(rRect); }
    void MoveAllMarked(const Size& rSiz, bool bCopy=false) { MoveMarkedObj(rSiz,bCopy); }
    void ResizeAllMarked(const Point& rRef, const Fraction& xFact, const Fraction& yFact) { ResizeMarkedObj(rRef,xFact,yFact); }
    void RotateAllMarked(const Point& rRef, long nAngle) { RotateMarkedObj(rRef,nAngle); }
    void MirrorAllMarkedHorizontal() { MirrorMarkedObjHorizontal(); }
    void MirrorAllMarkedVertical() { MirrorMarkedObjVertical(); }
    void CopyMarked() { CopyMarkedObj(); }
    bool IsMoveAllowed() const { ForcePossibilities(); return bMoveAllowed && !bMoveProtect; }
    bool IsResizeAllowed(bool bProp=false) const;
    bool IsRotateAllowed(bool b90Deg=false) const;
    bool IsMirrorAllowed(bool b45Deg=false, bool b90Deg=false) const;
    bool IsTransparenceAllowed() const;
    bool IsGradientAllowed() const;
    bool IsShearAllowed() const;
    bool IsEdgeRadiusAllowed() const;
    bool IsCrookAllowed(bool bNoContortion=false) const;
    bool IsCropAllowed() const;
    bool IsDistortAllowed(bool bNoContortion=false) const;

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
    void DistributeMarkedObjects();

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

    void SetNotPersistAttrToMarked(const SfxItemSet& rAttr);
    void MergeNotPersistAttrFromMarked(SfxItemSet& rAttr) const;
    void MergeAttrFromMarked(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
    SfxItemSet GetAttrFromMarked(bool bOnlyHardAttr) const;
    void SetAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll);

    // geometrical attribute (position, size, rotation angle)
    // A PageOrigin set at a position is taken into account.
    SfxItemSet GetGeoAttrFromMarked() const;
    void SetGeoAttrToMarked(const SfxItemSet& rAttr);

    // Returns NULL if:
    // - nothing is marked,
    // - no stylesheet is set at the marked object
    // - point the marked objects to different StyleSheets for multiple selections
    SfxStyleSheet* GetStyleSheetFromMarked() const;

    // at the moment without undo :(
    void SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

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

    bool IsGroupPossible() const { ForcePossibilities(); return bGroupPossible; }
    bool IsUnGroupPossible() const { ForcePossibilities(); return bUnGroupPossible; }
    bool IsGroupEnterPossible() const { ForcePossibilities(); return bGrpEnterPossible; }

    // Convert marked objects to polygones/Beziercurves. The bool-functions
    // return sal_True, if at least one marked object could be converted.
    // Also member objects of group objects are converted.
    // For a better description see: SdrObj.HXX
    bool IsConvertToPathObjPossible() const { ForcePossibilities(); return bCanConvToPath; }
    bool IsConvertToPolyObjPossible() const { ForcePossibilities(); return bCanConvToPoly; }
    bool IsConvertToContourPossible() const { ForcePossibilities(); return bCanConvToContour; }
    void ConvertMarkedToPathObj(bool bLineToArea);
    void ConvertMarkedToPolyObj();

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
    bool IsToTopPossible() const { ForcePossibilities(); return bToTopPossible; }
    bool IsToBtmPossible() const { ForcePossibilities(); return bToBtmPossible; }
    bool IsReverseOrderPossible() const { ForcePossibilities(); return bReverseOrderPossible; }

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
    bool IsImportMtfPossible() const { ForcePossibilities(); return bImportMtfPossible; }

    // override SdrMarkView, for internal use
    virtual void MarkListHasChanged() override;
    virtual void ModelHasChanged() override;
};

#endif // INCLUDED_SVX_SVDEDTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
