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

#pragma once

#include <memory>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <svl/lstner.hxx>
#include <svl/poolitem.hxx>
#include <svl/typedwhich.hxx>
#include <tools/degree.hxx>
#include <svx/DiagramDataInterface.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/svxdllapi.h>
#include <svx/shapeproperty.hxx>
#include <tools/link.hxx>
#include <tools/weakbase.h>
#include <tools/gen.hxx>
#include <unotools/resmgr.hxx>

#include <unordered_set>

class SfxBroadcaster;
class AutoTimer;
class OutlinerParaObject;
class Outliner;
class SdrOutliner;
class SdrDragStat;
class SdrHdl;
class SdrHdlList;
class SdrItemPool;
class SdrModel;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPageView;
class SdrView;
class SfxItemSet;
class SfxGrabBagItem;
class SfxSetItem;
class SfxStyleSheet;
class SfxUndoAction;
class XFillAttrSetItem;
class XLineAttrSetItem;
class SfxItemPool;
namespace tools { class PolyPolygon; }
class SfxPoolItem;
class SdrVirtObj;
class SdrDragView;
class SdrObjUserDataList;
class SdrObjPlusData;
class SdrGluePoint;
class SdrGluePointList;
class SdrLayerIDSet;
class Fraction;
enum class PointerStyle;
class Graphic;
class SvxShape;

namespace basegfx
{
    class B2DPoint;
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace sdr { class ObjectUser; }
namespace sdr::properties { class BaseProperties; }
namespace sdr::contact { class ViewContact; }

namespace svx { class PropertyChangeNotifier; }
namespace com::sun::star::drawing { class XShape; }


struct SVXCORE_DLLPUBLIC SdrObjectFreeOp;

// helper for constructing std::unique_ptr for SdrObjects where a
// deleter is needed - here, SdrObject::Free needs to be used.
typedef std::unique_ptr< SdrObject, SdrObjectFreeOp > SdrObjectUniquePtr;

enum class SdrInventor : sal_uInt32 {
    Unknown          = 0,
    BasicDialog      = sal_uInt32( 'D' | ('L' << 8) | ('G' << 16) | ('1' << 24) ),
    Default          = sal_uInt32( 'S' | ('V' << 8) | ('D' << 16) | ('r' << 24) ),
    E3d              = sal_uInt32( 'E' | ('3' << 8) | ('D' << 16) | ('1' << 24) ),
    FmForm           = sal_uInt32( 'F' | ('M' << 8) | ('0' << 16) | ('1' << 24) ),
    IMap             = sal_uInt32( 'I' | ('M' << 8) | ('A' << 16) | ('P' << 24) ),
    ReportDesign     = sal_uInt32( 'R' | ('P' << 8) | ('T' << 16) | ('1' << 24) ),
    ScOrSwDraw       = sal_uInt32( 'S' | ('C' << 8) | ('3' << 16) | ('0' << 24) ), // Used in sc/ and sw/
    SgaImap          = sal_uInt32( 'S' | ('D' << 8) | ('U' << 16) | ('D' << 24) ),
    StarDrawUserData = sal_uInt32( 'S' | ('D' << 8) | ('U' << 16) | ('D' << 24) ),
    Swg              = sal_uInt32( 'S' | ('W' << 8) | ('G' << 16) ),
};

enum class SdrUserCallType {
    MoveOnly,         // only moved, size unchanged
    Resize,           // size and maybe position changed
    ChangeAttr,       // attribute changed. Eventually new size, because of line width
    Delete,           // object is going to be deleted soon, no attributes anymore
    Inserted,         // inserted into an object list (e.g. Page)
    Removed,          // removed from an object list
    ChildMoveOnly,    // a child within a group has changed
    ChildResize,      // a child within a group has changed
    ChildChangeAttr,  // a child within a group has changed
    ChildDelete,      // a child within a group has changed
    ChildInserted,    // a child within a group has changed
    ChildRemoved      // a child within a group has changed
};

class SVXCORE_DLLPUBLIC SdrObjUserCall
{
public:
    virtual ~SdrObjUserCall();
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const tools::Rectangle& rOldBoundRect);
};

class SVXCORE_DLLPUBLIC SdrObjMacroHitRec
{
public:
    Point                       aPos;
    const SdrLayerIDSet*        pVisiLayer;
    const SdrPageView*          pPageView;
    sal_uInt16                  nTol;

    SdrObjMacroHitRec();
};

/**
 * User data of a drawing object, e.g. application specific data.
 * Every drawing object can have an arbitrary amount of such records (SV list).
 * Whoever wants to save data here, must inherit from this and set a corresponding link in the factory.
 */
class SVXCORE_DLLPUBLIC SdrObjUserData
{
    SdrInventor                      nInventor;
    sal_uInt16                       nIdentifier;

    void operator=(const SdrObjUserData& rData) = delete;
    bool operator==(const SdrObjUserData& rData) const = delete;
    bool operator!=(const SdrObjUserData& rData) const = delete;

public:
    SdrObjUserData(SdrInventor nInv, sal_uInt16 nId);
    SdrObjUserData(const SdrObjUserData& rData);
    virtual ~SdrObjUserData();

    virtual std::unique_ptr<SdrObjUserData> Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    SdrInventor GetInventor() const { return nInventor;}
    sal_uInt16 GetId() const { return nIdentifier;}
};

/**
 * All geometrical data of an arbitrary object for use in undo/redo
 */
class SVXCORE_DLLPUBLIC SdrObjGeoData
{
public:
    tools::Rectangle                   aBoundRect;
    Point                       aAnchor;
    std::unique_ptr<SdrGluePointList>
                                pGPL;
    bool                        bMovProt;
    bool                        bSizProt;
    bool                        bNoPrint;
    bool                        bClosedObj;
    bool                        mbVisible;
    SdrLayerID                  mnLayerID;

public:
    SdrObjGeoData();
    virtual ~SdrObjGeoData();
};

/**
 * Provides information about various ZObject properties
 */
class SVXCORE_DLLPUBLIC SdrObjTransformInfoRec
{
public:
    bool bMoveAllowed : 1;             // if false, object cannot be moved
    bool bResizeFreeAllowed : 1;       // if false, object cannot be resized freely
    bool bResizePropAllowed : 1;       // if false, object cannot even be resized proportionally
    bool bRotateFreeAllowed : 1;       // if false, object cannot be rotated freely
    bool bRotate90Allowed : 1;         // if false, object cannot even be rotated in 90 degree steps
    bool bMirrorFreeAllowed : 1;       // if false, object cannot be mirrored freely
    bool bMirror45Allowed : 1;         // if false, object cannot even be mirrored over axes in a 45 degree raster
    bool bMirror90Allowed : 1;         // if false, object cannot even be mirrored over axes in a 90 degree raster
    bool bTransparenceAllowed : 1;     // if false, object does not have an interactive transparence control
    bool bShearAllowed : 1;            // if false, object cannot be sheared
    bool bEdgeRadiusAllowed : 1;
    bool bNoOrthoDesired : 1;          // is true for Rect; is false for BMP, MTF
    bool bNoContortion : 1;            // if false, contortion not possible (for crook, only true for PathObj and grouped PathObjs)
    bool bCanConvToPath : 1;           // if false, no conversion into PathObj possible
    bool bCanConvToPoly : 1;           // if false, no conversion into PolyObj possible
    bool bCanConvToContour : 1;        // if false, no conversion down to whole contour possible
    bool bCanConvToPathLineToArea : 1; // if false, no conversion into PathObj with transformation from LineToArea possible
    bool bCanConvToPolyLineToArea : 1; // if false, no conversion into PolyObj with transformation from LineToArea possible

    SdrObjTransformInfoRec();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SdrObject
//      SdrAttrObj
//          E3dObject
//              E3dCompoundObject
//                  E3dCubeObj
//                  E3dExtrudeObj
//                  E3dLatheObj
//                  E3dPolygonObj
//                  E3dSphereObj
//              E3dScene
//          SdrTextObj
//              SdrObjCustomShape
//                  OCustomShape
//              SdrEdgeObj
//              SdrMeasureObj
//              SdrPathObj
//              SdrRectObj
//                  SdrCaptionObj
//                  SdrCircObj
//                  SdrGrafObj
//                  SdrMediaObj
//                  SdrOle2Obj
//                      OOle2Obj
//                  SdrUnoObj
//                      DlgEdObj
//                          DlgEdForm
//                      OUnoObject
//                      FmFormObj
//              SdrTableObj
//      SdrObjGroup
//      SdrPageObj
//      SdrVirtObj
//          SwDrawVirtObj
//          SwVirtFlyDrawObj
//      SwFlyDrawObj

/// Abstract DrawObject
class SVXCORE_DLLPUBLIC SdrObject : public SfxListener, public tools::WeakBase
{
private:
    friend class                SdrObjListIter;
    friend class                SdrObjList;
    friend class                SdrVirtObj;
    friend class                SdrRectObj;

    // OperationSmiley: Allow at each SdrObject to set a FillGeometryDefiningShape,
    // so that for SdrObjects where this is set, the definition of a defined FillStyle
    // will use this, but the local geometry will be filled. This allows to fill
    // multiple shapes with a unified fill, e.g think about CustomShapes.
    // Currently this is *only* used for CustomShapes, but may be developed to get a
    // common mechanism - usages for it are easy to be found. The current limitation
    // to CustomShapes allows to think about these SdrObjects to 'vanish' during the
    // lifetime of 'this' - the SdrObjects without SdrPage and SdrModel are used as helper
    // objects for SdrObjCustomShape and thus their lifetime is limited to the lifetime
    // of this local object. For unifying this mechanism, some weak reference of
    // SdrObjects would have to be thought about (not easy with the current implementation).
    // So - allow *only* EnhancedCustomShape2d (which creates the visualizations for
    // SdrObjCustomShape) to set this. Already allow unified read to use it - thus already
    // allowing to implement as standard case for all kinds of SdrObjects.
    friend class EnhancedCustomShape2d;
    const SdrObject*            mpFillGeometryDefiningShape;
    void setFillGeometryDefiningShape(const SdrObject* pNew) { mpFillGeometryDefiningShape = pNew; }
public:
    const SdrObject* getFillGeometryDefiningShape() const { return mpFillGeometryDefiningShape; }

private:
    // the SdrModel this objects was created with, unchanged during SdrObject lifetime
    SdrModel&                   mrSdrModelFromSdrObject;

public:
    // A SdrObject always needs a SdrModel for lifetime (Pool, ...)
    SdrObject(SdrModel& rSdrModel);
    // Copy constructor
    SdrObject(SdrModel& rSdrModel, SdrObject const & rSource);

    // SdrModel/SdrPage access on SdrObject level
    SdrPage* getSdrPageFromSdrObject() const;
    SdrModel& getSdrModelFromSdrObject() const;

    // access to possible children (SdrObjGroup/E3dScene)
    virtual SdrObjList* getChildrenOfSdrObject() const;

    // access to parent
    SdrObjList* getParentSdrObjListFromSdrObject() const;
    SdrObject* getParentSdrObjectFromSdrObject() const;

private:
    SVX_DLLPRIVATE void setParentOfSdrObject(SdrObjList* pNew);

public:
    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    void AddObjectUser(sdr::ObjectUser& rNewUser);
    void RemoveObjectUser(sdr::ObjectUser& rOldUser);

    sdr::contact::ViewContact& GetViewContact() const;

    virtual sdr::properties::BaseProperties& GetProperties() const;

    // DrawContact support: Methods for handling Object changes
    void ActionChanged() const;

    static SdrItemPool& GetGlobalDrawObjectItemPool();
    void SetRelativeWidth( double nValue );
    void SetRelativeWidthRelation( sal_Int16 eValue );
    void SetRelativeHeight( double nValue );
    void SetRelativeHeightRelation( sal_Int16 eValue );
    const double* GetRelativeWidth() const;
    sal_Int16 GetRelativeWidthRelation() const;
    const double* GetRelativeHeight() const;
    sal_Int16 GetRelativeHeightRelation() const;

    void SetDiagramData(std::shared_ptr<DiagramDataInterface> pDiagramData);
    const std::shared_ptr<DiagramDataInterface> & GetDiagramData() const;

    /// @param bNotMyself = true: set only ObjList to dirty, don't mark this object as dirty.
    ///
    /// This is needed for instance for NbcMove, because usually one moves SnapRect and aOutRect
    /// at the same time to avoid recomputation.
    virtual void SetBoundAndSnapRectsDirty(bool bNotMyself = false, bool bRecursive = true);

    // frees the SdrObject pointed to by the argument
    // In case the object has an SvxShape, which has the ownership of the object, it
    // is actually *not* deleted.
    static  void    Free( SdrObject*& _rpObject );

    // this method is only for access from Property objects
    virtual void SetBoundRectDirty();

    SfxItemPool & GetObjectItemPool() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);

    void AddReference(SdrVirtObj& rVrtObj);
    void DelReference(SdrVirtObj& rVrtObj);
    virtual SdrInventor GetObjInventor() const;
    virtual SdrObjKind GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    // Layer interface
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
    virtual void SetLayer(SdrLayerID nLayer);
    // renaming GetSdrLayerIDSet -> getMergedHierarchySdrLayerIDSet to make clear what happens here. rSet needs to be empty.
    void getMergedHierarchySdrLayerIDSet(SdrLayerIDSet& rSet) const;

    void SendUserCall(SdrUserCallType eUserCall, const tools::Rectangle& rBoundRect) const;

    // #i68101#
    // An object may have a user-set Name (Get/SetName()), e.g SdrGrafObj, SdrObjGroup
    // or SdrOle2Obj.
    // It may also have a Title and a Description for accessibility purposes.
    void SetName(const OUString& rStr, const bool bSetChanged = true);
    OUString GetName() const;
    void MakeNameUnique();
    void MakeNameUnique(std::unordered_set<OUString>& rNameSet);
    void SetTitle(const OUString& rStr);
    OUString GetTitle() const;
    void SetDescription(const OUString& rStr);
    OUString GetDescription() const;

    // for group objects
    bool IsGroupObject() const;
    virtual SdrObjList* GetSubList() const;

    /// The order number (aka ZOrder, aka z-index) determines whether a
    /// SdrObject is located above or below another.  Objects are painted from
    /// lowest to highest order number.  If the order of SdrObjects in the
    /// SdrObjList is changed, the bObjOrdNumsDirty flag is set on the SdrPage
    /// and the next GetOrdNum() call recalculates the order number of all
    /// SdrObjects in the SdrObjList.
    sal_uInt32 GetOrdNum() const;

    // setting the order number should only happen from the model or from the page
    void SetOrdNum(sal_uInt32 nNum);

    // GrabBagItem for interim interop purposes
    void GetGrabBagItem(css::uno::Any& rVal) const;

    void SetGrabBagItem(const css::uno::Any& rVal);

    // Return the position in the navigation order for the called object.
    // Note that this method may update the navigation position of the
    // called and of other SdrObjects.  Therefore this method can not be
    // const.
    // @return
    //     If no navigation position has been explicitly defined then the
    //     result of GetOrdNum() is returned.
    sal_uInt32 GetNavigationPosition() const;

    // To make clearer that this method may trigger RecalcBoundRect and thus may be
    // expensive and sometimes problematic (inside a bigger object change You will get
    // non-useful BoundRects sometimes) i rename that method from GetBoundRect() to
    // GetCurrentBoundRect().
    virtual const tools::Rectangle& GetCurrentBoundRect() const;

    // To have a possibility to get the last calculated BoundRect e.g for producing
    // the first rectangle for repaints (old and new need to be used) without forcing
    // a RecalcBoundRect (which may be problematical and expensive sometimes) i add here
    // a new method for accessing the last BoundRect.
    virtual const tools::Rectangle& GetLastBoundRect() const;

    virtual void RecalcBoundRect();

    void BroadcastObjectChange() const;

    const SfxBroadcaster* GetBroadcaster() const;

    // set modified-flag in the model
    virtual void SetChanged();

    // Tooling for painting a single object to an OutputDevice. This will be needed as long
    // as not all painting is changed to use DrawContact objects.
    void SingleObjectPainter(OutputDevice& rOut) const;
    bool LineGeometryUsageIsNecessary() const;

    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation, that
    // means no change of the rotation point (only centered) and no shear allowed
    virtual bool HasLimitedRotation() const;

    // Returns a copy of the object. Every inherited class must reimplement this.
    virtual SdrObject* CloneSdrObject(SdrModel& rTargetModel) const;

    // Overwriting this object makes no sense, it is too complicated for that
    SdrObject& operator=(const SdrObject& rObj) = delete;
    SdrObject& operator=(SdrObject&& rObj) = delete;

    // TakeObjName...() is for the display in the UI, e.g. "3 frames selected"
    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

    /// The Xor-Polygon is required by the View to drag the object.
    /// All polygons within the PolyPolygon are interpreted as PolyLine.
    /// To get a closed Polygon, close it explicitly.
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    /// contour for TextToContour
    virtual basegfx::B2DPolyPolygon TakeContour() const;

    /// Via GetHdlCount the number of Handles can be retrieved.
    /// Normally 8, if it's a line 2.
    /// For Polygon objects (Polygon/Spline/Bezier) the number may be much
    /// larger.  Polygon objects are also allowed to select a point of a
    /// selected object.  The handle of that point will then be replaced by
    /// a new set of handles (PlusHdl).  For a Polygon this would be a simple
    /// selection Handle, for a Bezier curve this may be up to 3 Handles
    /// (including Weights).
    /// GetHdl() and GetPlusHdl() must create Handle instances with new!
    /// An object that returns true from HasSpacialDrag() must provide these
    /// methods (incl. FillHdlList()).
    virtual sal_uInt32 GetHdlCount() const;
    virtual void AddToPlusHdlList(SdrHdlList& rHdlList, SdrHdl& rHdl) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;
    virtual void addCropHandles(SdrHdlList& rTarget) const;

    /// The standard transformations (Move,Resize,Rotate,Mirror,Shear) are
    /// taken over by the View (TakeXorPoly(),...).
    /// Object type specific dragging like corner radius of Rectangle,
    /// control points of Splines, weights of Bezier curves, pointer of
    /// Label objects must be handled by the object itself.
    /// To keep the Model free of state, the state is kept in the View
    /// and then passed to the object. EndDrag usually returns true for success,
    /// false may be returned if the dragging did not modify the object,
    /// where the View already handles the case that the pointer was not
    /// moved at all.
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    // FullDrag support. This is for standard interactions and for SdrObjOwn
    // support. If supportsFullDrag() returns true, getFullDragClone has to
    // return a cloned SdrObject (who's ownership it loses) at which modifications
    // like Move(), Scale(), etc or applySpecialDrag() will be executed. That
    // object will be visualized on overlay for full drag, but should not be
    // part of the model, thus not changing anything since it's only a temporary
    // helper object for interaction
    virtual bool supportsFullDrag() const;
    virtual SdrObjectUniquePtr getFullDragClone() const;

    /// Every object must be able to create itself interactively.
    /// On MouseDown first an object is created, and its BegCreate() method
    /// is called.  On every MouseMove, MovCreate() is called.  BrkCreate()
    /// is called if the user cancels the interactive object creation.
    /// EndCreate() is called on the MouseUp event.  If EndCreate() returns
    /// true, the creation is finished; the object is inserted into the
    /// corresponding list.  Otherwise it is assumed that further points
    /// are necessary to create the object (Polygon, ...).  The parameter
    /// eCmd contains the number of mouse clicks (if the application
    /// provides it).
    /// BckCreate() will undo the previous EndCreate(), e.g. to delete the
    /// last point of the Polygon.  If BckCreate() returns false, creation
    /// is cancelled.
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat); // if true, Xor needs to be repainted
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);

    /// get the cursor/pointer that signals creating this object
    virtual PointerStyle GetCreatePointer() const;

    /// Polygon dragged by the user when creating the object
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    /// The methods Move, Resize, Rotate, Mirror, Shear, SetSnapRect and
    /// SetLogicRect call the corresponding Nbc-methods, send a Repaint
    /// broadcast and set the Modified state on the Model.
    /// Derived classes should usually only override the Nbc methods.
    /// Nbc means "no broadcast".
    virtual void NbcMove  (const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcCrop  (const basegfx::B2DPoint& rRef, double fxFact, double fyFact);
    virtual void NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs);
    // Utility for call sites that don't have sin and cos handy
    void NbcRotate(const Point& rRef, Degree100 nAngle);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear (const Point& rRef, Degree100 nAngle, double tn, bool bVShear);

    virtual void Move  (const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Crop  (const basegfx::B2DPoint& rRef, double fxFact, double fyFact);
    virtual void Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear (const Point& rRef, Degree100 nAngle, double tn, bool bVShear);

    /// The relative position of a SdrObject is the distance of the upper
    /// left corner of the logic bounding rectangle (SnapRect) to the anchor.
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual Point GetRelativePos() const;
    void ImpSetAnchorPos(const Point& rPnt);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void SetAnchorPos(const Point& rPnt);

    /// Snap is not done on the BoundRect but if possible on logic coordinates
    /// (i.e. without considering stroke width, ...)
    /// SetSnapRect() tries to size the Object so that it fits into the
    /// passed Rect (without stroke width, ...)
    virtual void RecalcSnapRect();
    virtual const tools::Rectangle& GetSnapRect() const;
    virtual void SetSnapRect(const tools::Rectangle& rRect);
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect);

    // Logic Rect: for the Rect for instance without regard to rotation angle, shear, ...
    virtual const tools::Rectangle& GetLogicRect() const;
    virtual void SetLogicRect(const tools::Rectangle& rRect);
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect);

    // the default is to set the logic rect to the given rectangle rMaxRect. If the shape
    // has an intrinsic aspect ratio it may set the logic rect so the aspect
    // ratio is kept but still inside the rectangle rMaxRect.
    // If bShrinkOnly is set to true, the size of the current logic rect will not
    // be changed if it is smaller than the given rectangle rMaxRect.
    virtual void AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false );

    // rotation and shear angle
    virtual Degree100 GetRotateAngle() const;
    virtual Degree100 GetShearAngle(bool bVertical = false) const;

    /// snap to special points of an Object (polygon points, center of circle)
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    // For objects, whose points can be moved individually.
    // (e.g. polygons, polylines, lines)
    // The points of those objects are selected (if necessary multiselection),
    // deleted, inserted, or as a multiselection moved or rotated...
    // Only such objects can have PlusHandles (e.g. the weights of a Bezier curve).
    virtual bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 i) const;
    void SetPoint(const Point& rPnt, sal_uInt32 i);
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i);

    // get all geometrical data for undo/redo
    virtual std::unique_ptr<SdrObjGeoData> GetGeoData() const;
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    // ItemSet access
    const SfxItemSet& GetMergedItemSet() const;
    void SetMergedItem(const SfxPoolItem& rItem);
    void ClearMergedItem(const sal_uInt16 nWhich = 0);
    void SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems = false);
    const SfxPoolItem& GetMergedItem(const sal_uInt16 nWhich) const;
    template<class T>
    const T&           GetMergedItem( TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetMergedItem(sal_uInt16(nWhich)));
    }

    // syntactical sugar for ItemSet accesses
    void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems = false);

    // NotPersistAttr for Layer, ObjName, geometrical transformations etc.
    void TakeNotPersistAttr(SfxItemSet& rAttr) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);
    void NbcApplyNotPersistAttr(const SfxItemSet& rAttr);

    // if bDontRemoveHardAttr is false, set all attributes, which were set in the style sheet, to their default value
    // if true, all hard attributes keep their values
    void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    SfxStyleSheet* GetStyleSheet() const;

    virtual bool HasTextEdit() const;

    // keep text in outliner's format
    // SetOutlinerParaObject: transfer ownership of *pTextObject!
    void SetOutlinerParaObject(std::optional<OutlinerParaObject> pTextObject);
    virtual void NbcSetOutlinerParaObject(std::optional<OutlinerParaObject> pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual void NbcReformatText();

    void BurnInStyleSheetAttributes();

    // macro abilities, e.g. a rectangle as PushButton.
    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual PointerStyle GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const tools::Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    bool IsMacroHit(const SdrObjMacroHitRec& rRec) const;

    // Connectors
    // (see also documentation in SvdoEdge.hxx, SdrEdgeObj, as well as SvdGlue.hxx and SvdGlEV.hxx)
    //
    // There are nodes and edges. In theory an edge can also be a node, but this isn't implemented yet.
    // A node has a number of gluepoints, onto which edges can glued to
    // An edge can be either
    // - without any connection to any node,
    // - or connected on one end to a node, while the other end is not connected,
    // - or connected on both ends with exactly one node each.
    // The edge is listener for its up to two nodes.
    // Whenever a node is moved or resized, all its connected edges follow.
    // This is also true for SetGluePoint()... on the node.
    // On the other hand, moving/resizing an edge breaks the connection.

    // automatic gluepoints:
    // a node object must provide four vertex and corner positions
    // usually 0: top, 1: right, 2: bottom, 3: left
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const;

    // usually:
    // 0: top-left, 1: top-right, 2: bottom-right, 3: bottom-left
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const;

    // list of all gluepoints, can be NULL
    virtual const SdrGluePointList* GetGluePointList() const;

    // after changing the GluePointList, one has to call the object's SendRepaintBroadcast!
    virtual SdrGluePointList* ForceGluePointList();

    // to be set temporarily when transforming related object(?)
    void SetGlueReallyAbsolute(bool bOn);
    void NbcRotateGluePoints(const Point& rRef, Degree100 nAngle, double sn, double cs);
    void NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2);
    void NbcShearGluePoints (const Point& rRef, double tn, bool bVShear);

    // if bTail1 is true, line start, otherwise line end
    // if pObj is null disconnect
    virtual void ConnectToNode(bool bTail1, SdrObject* pObj);
    virtual void DisconnectFromNode(bool bTail1);
    virtual SdrObject* GetConnectedNode(bool bTail1) const;

    // sets the writing mode of the object's context
    // Objects which itself do not support different writing modes will ignore this call.
    // Objects which support different writing modes, but have an own, explicit writing mode set,
    // will also ignore this call.
    // Objects which support different writing modes, and whose own mode is set to css.text.WritingMode2.CONTEXT,
    // will use the given writing mode to calculate their "effective writing mode".
    // The behaviour of this method is undefined if you pass css.text.WritingMode2.CONTEXT.
    // @param _nContextWritingMode
    //     the effective writing mode of the context of the object
    virtual void    SetContextWritingMode( const sal_Int16 _nContextWritingMode );

    // If an object is able to convert itself into a polygon or into a Bezier curve (or into both),
    // then the following methods should be overridden.
    // E.g., convert a RectObj with line width 10, SOLID_PEN into a polygon:
    // In the bLineToArea=false mode a PolyObj with four supporting points,
    // line width 10 and SOLID_PEN shall be created.
    // On the contrary in the bLineToArea=true mode the generated object should
    // still have a line attribute NULL_PEN, and the line (also line pattern)
    // itself should be emulated by the polygon area, which thereby can be
    // manipulated by the user afterwards.
    // The RectObj therefore can only convert correctly if it has an area attribute NULL_BRUSH.
    // In this case it would have to:
    // - set SOLID_BRUSH with the color of the given pen,
    // - set NULL_PEN, and
    // - generate tools::PolyPolygon with two polygons with four supporting points each.
    // In each case the return value is a SdrObject*, because it is also
    // allowed to return group objects (e.g. for SdrTextObj).
    // In the case of the conversion from TextObj to PathObj,
    // both modi (bLineToArea=true/false) would be identical.
    // The methods' default implementations report "I'm unable to do this" (false/null).
    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const;
    SdrObjectUniquePtr ConvertToPolyObj(bool bBezier, bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;
private:
    SdrObject* ImpConvertToContourObj(bool bForceLineDash);
public:

    // if true, reference onto an object
    bool IsVirtualObj() const { return m_bVirtObj;}

    // is true, if object can probably be filled
    // is false, if object has probably line ends
    // is invalid, if this is a group object
    bool IsClosedObj() const { return m_bClosedObj;}

    // tdf#118662 reorganize inserted state, no local bool needed anymore,
    // it depends on being a member of a SdrObjList
    void InsertedStateChange();
    bool IsInserted() const { return nullptr != getParentSdrObjListFromSdrObject(); }

    bool IsEdgeObj() const { return m_bIsEdge;}
    bool Is3DObj() const { return m_bIs3DObj;}
    bool IsUnoObj() const { return m_bIsUnoObj;}
    void SetMoveProtect(bool bProt);
    bool IsMoveProtect() const { return m_bMovProt;}
    void SetResizeProtect(bool bProt);
    bool IsResizeProtect() const { return m_bSizProt;}
    void SetPrintable(bool bPrn);
    bool IsPrintable() const { return !m_bNoPrint;}
    void SetVisible(bool bVisible);
    bool IsVisible() const { return mbVisible;}
    void SetMarkProtect(bool bProt);
    bool IsMarkProtect() const { return m_bMarkProt;}

    /// Whether the aspect ratio should be kept by default when resizing.
    virtual bool shouldKeepAspectRatio() const { return false; }

    // application specific data
    sal_uInt16 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const;

    void AppendUserData(std::unique_ptr<SdrObjUserData> pData);

    // removes the record from the list and performs delete (FreeMem+Dtor).
    void DeleteUserData(sal_uInt16 nNum);

    // access to the UNO representation of the shape
    virtual css::uno::Reference< css::drawing::XShape > getUnoShape();

    static SdrObject* getSdrObjectFromXShape( const css::uno::Reference< css::uno::XInterface >& xInt );

    // retrieves the instance responsible for notifying changes in the properties of the shape associated with
    // the SdrObject
    //
    // @precond
    //     There already exists an SvxShape instance associated with the SdrObject
    // @throws css::uno::RuntimeException
    //     if there does nt yet exists an SvxShape instance associated with the SdrObject.
    svx::PropertyChangeNotifier& getShapePropertyChangeNotifier();

    // notifies a change in the given property, to all applicable listeners registered at the associated SvxShape
    //
    // This method is equivalent to calling getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty ),
    // exception that it is allowed to be called when there does not yet exist an associated SvxShape - in which
    // case the method will silently return without doing anything.
    void notifyShapePropertyChange( const svx::ShapeProperty _eProperty ) const;

    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;

    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);

    // give info if object is in destruction
    bool IsInDestruction() const;

    // return if fill is != XFILL_NONE
    bool HasFillStyle() const;
    bool HasLineStyle() const;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // the following methods are used to control it;
    // usually this data makes no sense after the import is finished, since the object
    // might be resized
    const tools::Rectangle& GetBLIPSizeRectangle() const { return maBLIPSizeRectangle;}
    void SetBLIPSizeRectangle( const tools::Rectangle& aRect );

    // #i121917#
    virtual bool HasText() const;

    bool Equals(const SdrObject&) const;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

    /// Is this a textbox of a drawinglayer shape?
    virtual bool IsTextBox() const;

    void SetEmptyPresObj(bool bEpt);
    bool IsEmptyPresObj() const { return m_bEmptyPresObj;}
    void SetNotVisibleAsMaster(bool bFlg);
    bool IsNotVisibleAsMaster() const { return m_bNotVisibleAsMaster;}
    void SetUserCall(SdrObjUserCall* pUser);
    SdrObjUserCall* GetUserCall() const { return m_pUserCall;}
    /// @see mbDoNotInsertIntoPageAutomatically
    void SetDoNotInsertIntoPageAutomatically(bool bSet);
    /// @see mbDoNotInsertIntoPageAutomatically
    bool IsDoNotInsertIntoPageAutomatically() const { return mbDoNotInsertIntoPageAutomatically;}

    // Warning: this method should only be used if you really know what you're doing
    sal_uInt32 GetOrdNumDirect() const { return m_nOrdNum;}

    // #i25616#
    bool DoesSupportTextIndentingOnLineWidthChange() const { return mbSupportTextIndentingOnLineWidthChange;}

    const Point& GetAnchorPos() const;

    // #i25616#
    bool LineIsOutsideGeometry() const { return mbLineIsOutsideGeometry;}

    // Set the position in the navigation position to the given value.
    // This method is typically used only by the model after a change to
    // the navigation order.
    // This method does not change the navigation position of other
    // objects.
    // Use SdrObjList::SetObjectNavigationPosition() instead.
    void SetNavigationPosition (const sal_uInt32 nPosition);

    /// Sets a new UNO representation of the shape
    ///
    /// Calling this function is only allowed for the UNO representation
    /// itself!
    ///
    /// The default implementation of this function sets the new UNO
    /// shape. Derived classes should override the function to handle
    /// any other actions that are needed when the shape is being
    /// changed.
    ///
    /// The implementation _must_ call the same method of its parent
    /// class (preferably as the first step)!
    ///
    /// If you override this, you __have to__ override both of them.
    /// The second one is an optimised variant that speeds up creating
    /// new objects.
    ///
    virtual void setUnoShape( const css::uno::Reference< css::drawing::XShape >& _rxUnoShape );
    virtual void setUnoShape( SvxShape& rNewShape );

    const css::uno::WeakReference< css::drawing::XShape >& getWeakUnoShape() const { return maWeakUnoShape; }

    void setSuitableOutlinerBg(Outliner& rOutliner) const;
    // If fillstyle is drawing::FillStyle_BITMAP, returns the graphic.
    const Graphic* getFillGraphic() const;

    const OUString& getHyperlink() const { return msHyperlink; }
    void setHyperlink(const OUString& sHyperlink) { msHyperlink = sHyperlink; }

protected:
    mutable tools::Rectangle    m_aOutRect;     // surrounding rectangle for Paint (incl. LineWidth, ...)
    Point                       m_aAnchor;      // anchor position (Writer)
    SdrObjUserCall*             m_pUserCall;
    std::unique_ptr<SdrObjPlusData>
                                m_pPlusData;    // Broadcaster, UserData, connectors, ... (this is the Bitsack)
    // object is only pointing to another one
    bool                        m_bVirtObj : 1;
    bool                        m_bSnapRectDirty : 1;
    // the following flags will be streamed
    bool                        m_bMovProt : 1;   // if true, the position is protected
    bool                        m_bSizProt : 1;   // if true, the size is protected
    // If bEmptyPresObj is true, it is a presentation object that has no content yet.
    // The flag's default value is false.
    // The management is done by the application.
    // Neither assign operator nor cloning copies the flag!
    // The flag is persistent.
    bool                        m_bEmptyPresObj : 1; // empty presentation object (Draw)
    // if true, object is invisible as object of the MasterPage
    bool                        m_bNotVisibleAsMaster : 1;
    // if true, the object is closed, i.e. no line, arc...
    bool                        m_bClosedObj : 1;
    bool                        m_bIsEdge : 1;
    bool                        m_bIs3DObj : 1;
    bool                        m_bIsUnoObj : 1;
    // #i25616#
    bool                        mbLineIsOutsideGeometry : 1;
    // #i25616#
    bool                        mbSupportTextIndentingOnLineWidthChange : 1;

    virtual ~SdrObject() override;

    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties();

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact();

    tools::Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;

    // for GetDragComment
    OUString ImpGetDescriptionStr(TranslateId pStrCacheID) const;

    void ImpForcePlusData();

    OUString GetMetrStr(tools::Long nVal) const;

    /// A derived class must override these 3 methods if it has own geometric
    /// data that must be saved for Undo.
    /// NewGeoData() creates an empty instance of a class derived from
    /// SdrObjGeoData.
    virtual std::unique_ptr<SdrObjGeoData> NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestoreGeoData(const SdrObjGeoData& rGeo);

    // internal versions
    const SfxItemSet& GetObjectItemSet() const;
    void SetObjectItem(const SfxPoolItem& rItem);
    void SetObjectItemSet(const SfxItemSet& rSet);
    const SfxPoolItem& GetObjectItem(const sal_uInt16 nWhich) const;
    template<class T> const T& GetObjectItem( TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetObjectItem(sal_uInt16(nWhich)));
    }

    const SfxItemSet* getBackgroundFillSet() const;

private:
    struct Impl;
    std::unique_ptr<Impl>             mpImpl;
    SdrObjList*                       mpParentOfSdrObject;     // list that includes this object
    sal_uInt32                        m_nOrdNum;      // order number of the object in the list
    std::unique_ptr<SfxGrabBagItem>   m_pGrabBagItem; // holds the GrabBagItem property
    // Position in the navigation order. SAL_MAX_UINT32 when not used.
    sal_uInt32                        mnNavigationPosition;
    SdrLayerID                        mnLayerID;
    bool                              m_bNoPrint : 1;   // if true, the object is not printed.
    bool                              mbVisible : 1;  // if false, the object is not visible on screen (but maybe on printer, depending on bNoprint
    bool                              m_bMarkProt : 1;  // marking forbidden, persistent
    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // in this case the following member is initialized as nonempty rectangle
    tools::Rectangle                         maBLIPSizeRectangle;
    std::unique_ptr<sdr::properties::BaseProperties>
                                      mpProperties;
    std::unique_ptr<sdr::contact::ViewContact>
                                      mpViewContact;

    // do not use directly, always use getSvxShape() if you have to!
    SvxShape*                   mpSvxShape;
    css::uno::WeakReference< css::drawing::XShape >
                                maWeakUnoShape;
    // HACK: Do not automatically insert newly created object into a page.
    // The user needs to do it manually later.
    bool                        mbDoNotInsertIntoPageAutomatically;
    // Hyperlink for the whole shape
    OUString msHyperlink;

    // only for internal use!
    SvxShape* getSvxShape();

    SdrObject( const SdrObject& ) = delete;
};

struct SVXCORE_DLLPUBLIC SdrObjectFreeOp
{
    void operator()(SdrObject* obj)
    {
        SdrObject::Free(obj);
    }
};

struct SdrObjCreatorParams
{
    SdrInventor nInventor;
    SdrObjKind nObjIdentifier;
    SdrModel&   rSdrModel;
};

/**
 * Whoever creates his own objects must set a link in the SdrObjFactory class.
 * The handler must have the following signature:
 *      void Hdl(SdrObjFactory*)
 * He must take a look at the referenced instance's nInventor and nIdentifier values,
 * and must create a new drawing object instance accordingly.
 * He must also make the pNewObj pointer reference to this instance.
 */
class SVXCORE_DLLPUBLIC SdrObjFactory
{
public:
    static SdrObject* MakeNewObject(
        SdrModel& rSdrModel,
        SdrInventor nInventor,
        SdrObjKind nObjIdentifier,
        const tools::Rectangle* pSnapRect = nullptr);

    static void InsertMakeObjectHdl(Link<SdrObjCreatorParams, SdrObject*> const & rLink);
    static void RemoveMakeObjectHdl(Link<SdrObjCreatorParams, SdrObject*> const & rLink);

private:
    static SVX_DLLPRIVATE SdrObject* CreateObjectFromFactory(
        SdrModel& rSdrModel,
        SdrInventor nInventor,
        SdrObjKind nIdentifier);

    SdrObjFactory() = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
