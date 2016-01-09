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

#ifndef INCLUDED_SVX_SVDOBJ_HXX
#define INCLUDED_SVX_SVDOBJ_HXX

#include <typeinfo>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/vclptr.hxx>
#include <svl/lstner.hxx>
#include <svl/poolitem.hxx>
#include <svx/svdtypes.hxx>
#include <svx/xenum.hxx>
#include <svx/svxdllapi.h>
#include <svx/shapeproperty.hxx>
#include <tools/link.hxx>
#include <tools/weakbase.hxx>
#include <tools/mapunit.hxx>
#include <tools/gen.hxx>

class SfxBroadcaster;
class Pointer;
class AutoTimer;
class OutlinerParaObject;
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
class SetOfByte;
class OutputDevice;
class Fraction;

namespace basegfx {

class B2DPolyPolygon;
class B2DHomMatrix;

}

namespace sdr
{
    namespace properties
    {
        class BaseProperties;
    }

    class ObjectUser;
}

namespace sdr
{
    namespace contact
    {
        class ViewContact;
    } // end of namespace contact
}

namespace svx
{
    class PropertyChangeNotifier;
}

enum SdrObjKind {
    OBJ_NONE       = 0,  /// abstract object (SdrObject)
    OBJ_GRUP       = 1,  /// object group
    OBJ_LINE       = 2,  /// line
    OBJ_RECT       = 3,  /// rectangle (round corners optional)
    OBJ_CIRC       = 4,  /// circle, ellipse
    OBJ_SECT       = 5,  /// circle section
    OBJ_CARC       = 6,  /// circle arc
    OBJ_CCUT       = 7,  /// circle cut
    OBJ_POLY       = 8,  /// polygon, PolyPolygon
    OBJ_PLIN       = 9,  /// PolyLine
    OBJ_PATHLINE   =10,  /// open Bezier-curve
    OBJ_PATHFILL   =11,  /// closed Bezier-curve
    OBJ_FREELINE   =12,  /// open free-hand line
    OBJ_FREEFILL   =13,  /// closed free-hand line
    OBJ_SPLNLINE   =14,  /// natural cubic Spline                  (ni)
    OBJ_SPLNFILL   =15,  /// periodic cubic Spline                 (ni)
    OBJ_TEXT       =16,  /// text object
    OBJ_TEXTEXT    =17,  /// text extension frame                  (ni)
    OBJ_wegFITTEXT,      /// FitToSize-Text (all lines equal)
    OBJ_wegFITALLTEXT,   /// FitToSize-Text (by line)              (ni)
    OBJ_TITLETEXT  =20,  /// TitleText, special text object for StarDraw
    OBJ_OUTLINETEXT=21,  /// OutlineText, special text object for StarDraw
    OBJ_GRAF       =22,  /// foreign graphic (StarView Graphic)
    OBJ_OLE2       =23,  /// OLE object
    OBJ_EDGE       =24,  /// connector object
    OBJ_CAPTION    =25,  /// caption object
    OBJ_PATHPOLY   =26,  /// Polygon/PolyPolygon represented by SdrPathObj
    OBJ_PATHPLIN   =27,  /// Polyline represented by SdrPathObj
    OBJ_PAGE       =28,  /// object that represents a SdrPage
    OBJ_MEASURE    =29,  /// measurement object
    OBJ_DUMMY      =30,  /// dummy object for saving holes (to recover surrogates later)
    OBJ_FRAME      =31,  /// continuously activated OLE (PlugIn-Frame or similar)
    OBJ_UNO        =32,  /// Universal Network Object packed into SvDraw object
    OBJ_CUSTOMSHAPE=33,  /// custom shape
    OBJ_MEDIA      =34,  /// media shape
    OBJ_TABLE      =35,  /// table
    OBJ_MAXI
};

enum SdrUserCallType {SDRUSERCALL_MOVEONLY,         // only moved, size unchanged
                      SDRUSERCALL_RESIZE,           // size and maybe position changed
                      SDRUSERCALL_CHGATTR,          // attribute changed. Eventually new size, because of line width
                      SDRUSERCALL_DELETE,           // object is going to be deleted soon, no attributes anymore
                      SDRUSERCALL_COPY,             // assign operator called, everything can be changed
                      SDRUSERCALL_INSERTED,         // inserted into an object list (e.g. Page)
                      SDRUSERCALL_REMOVED,          // removed from an object list
                      SDRUSERCALL_CHILD_MOVEONLY,   // a child within a group has changed
                      SDRUSERCALL_CHILD_RESIZE,     // a child within a group has changed
                      SDRUSERCALL_CHILD_CHGATTR,    // a child within a group has changed
                      SDRUSERCALL_CHILD_DELETE,     // a child within a group has changed
                      SDRUSERCALL_CHILD_COPY,       // a child within a group has changed
                      SDRUSERCALL_CHILD_INSERTED,   // a child within a group has changed
                      SDRUSERCALL_CHILD_REMOVED};   // a child within a group has changed

class SVX_DLLPUBLIC SdrObjUserCall
{
public:
    virtual ~SdrObjUserCall();
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect);
};

class SVX_DLLPUBLIC SdrObjMacroHitRec
{
public:
    Point                       aPos;
    Point                       aDownPos;
    VclPtr<OutputDevice>        pOut;
    const SetOfByte*            pVisiLayer;
    const SdrPageView*          pPageView;
    sal_uInt16                  nTol;
    bool                        bDown;

    SdrObjMacroHitRec();
};

/**
 * User data of a drawing object, e.g. application specific data.
 * Every drawing object can have an arbitrary amount of such records (SV list).
 * Whoever wants to save data here, must inherit from this and set a corresponding link in the factory.
 */
class SVX_DLLPUBLIC SdrObjUserData
{
protected:
    sal_uInt32                      nInventor;
    sal_uInt16                      nIdentifier;
    sal_uInt16                      nVersion;

private:
    void operator=(const SdrObjUserData& rData) = delete;
    bool operator==(const SdrObjUserData& rData) const = delete;
    bool operator!=(const SdrObjUserData& rData) const = delete;

public:
    SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId, sal_uInt16 nVer);
    SdrObjUserData(const SdrObjUserData& rData);
    virtual ~SdrObjUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    sal_uInt32 GetInventor() const { return nInventor;}
    sal_uInt16 GetId() const { return nIdentifier;}
};

/**
 * All geometrical data of an arbitrary object for use in undo/redo
 */
class SVX_DLLPUBLIC SdrObjGeoData
{
public:
    Rectangle                   aBoundRect;
    Point                       aAnchor;
    SdrGluePointList*           pGPL;
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
class SVX_DLLPUBLIC SdrObjTransformInfoRec
{
public:
    bool bSelectAllowed : 1;           // if false, object cannot be selected
    bool bMoveAllowed : 1;             // if false, object cannot be moved
    bool bResizeFreeAllowed : 1;       // if false, object cannot be resized freely
    bool bResizePropAllowed : 1;       // if false, object cannot even be resized proportionally
    bool bRotateFreeAllowed : 1;       // if false, object cannot be rotated freely
    bool bRotate90Allowed : 1;         // if false, object cannot even be rotated in 90 degree steps
    bool bMirrorFreeAllowed : 1;       // if false, object cannot be mirrored freely
    bool bMirror45Allowed : 1;         // if false, object cannot even be mirrored over axes in a 45 degree raster
    bool bMirror90Allowed : 1;         // if false, object cannot even be mirrored over axes in a 90 degree raster
    bool bTransparenceAllowed : 1;     // if false, object does not have an interactive transparence control
    bool bGradientAllowed : 1;         // if false, object does not have an interactive gradient control
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

/// Abstract DrawObject

class SvxShape;
class SVX_DLLPUBLIC SdrObject: public SfxListener, public tools::WeakBase< SdrObject >
{
private:
    struct Impl;
    Impl* mpImpl;

    SdrObject( const SdrObject& ) = delete;

public:
    void AddObjectUser(sdr::ObjectUser& rNewUser);
    void RemoveObjectUser(sdr::ObjectUser& rOldUser);

    // BaseProperties section
private:
    sdr::properties::BaseProperties*                                mpProperties;
protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
public:
    virtual sdr::properties::BaseProperties& GetProperties() const;

    // #110094# DrawContact section
private:
    sdr::contact::ViewContact*                                      mpViewContact;
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
public:
    sdr::contact::ViewContact& GetViewContact() const;

    // DrawContact support: Methods for handling Object changes
    void ActionChanged() const;

private:
    friend class                SdrObjListIter;
    friend class                SdrVirtObj;
    friend class                SdrRectObj;

protected:
    Rectangle                   aOutRect;     // surrounding rectangle for Paint (incl. LineWdt, ...)
    Point                       aAnchor;      // anchor position (Writer)
    Point                       aGridOffset;  // hack (Calc)
    SdrObjList*                 pObjList;     // list that includes this object
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrObjUserCall*             pUserCall;
    SdrObjPlusData*             pPlusData;    // Broadcaster, UserData, connectors, ... (this is the Bitsack)

    sal_uInt32                  nOrdNum;      // order number of the object in the list

    SfxGrabBagItem*             pGrabBagItem; // holds the GrabBagItem property


    // Position in the navigation order. SAL_MAX_UINT32 when not used.
    sal_uInt32                  mnNavigationPosition;
    SdrLayerID                  mnLayerID;

    // object is only pointing to another one
    bool                        bVirtObj : 1;
    bool                        bSnapRectDirty : 1;
    bool                        bNetLock : 1;   // ni
    bool                        bInserted : 1;  // only if set to true, there are RepaintBroadcast & SetModify
    bool                        bGrouped : 1;   // member of a group object?

    // the following flags will be streamed
    bool                        bMovProt : 1;   // if true, the position is protected
    bool                        bSizProt : 1;   // if true, the size is protected
    bool                        bNoPrint : 1;   // if true, the object is not printed.
    bool                        mbVisible : 1;  // if false, the object is not visible on screen (but maybe on printer, depending on bNoprint

    // If bEmptyPresObj is true, it is a presentation object that has no content yet.
    // The flag's default value is false.
    // The management is done by the application.
    // Neither assign operator nor cloning copies the flag!
    // The flag is persistent.
    bool                        bEmptyPresObj : 1; // empty presentation object (Draw)

    // if true, object is invisible as object of the MasterPage
    bool                        bNotVisibleAsMaster : 1;

    // if true, the object is closed, i.e. no line, arc...
    bool                        bClosedObj : 1;

    bool                        bIsEdge : 1;
    bool                        bIs3DObj : 1;
    bool                        bMarkProt : 1;  // marking forbidden, persistent
    bool                        bIsUnoObj : 1;
    bool                        bNotMasterCachable : 1;

    // #i25616#
    bool                        mbLineIsOutsideGeometry : 1;

    // #i25616#
    bool                        mbSupportTextIndentingOnLineWidthChange : 1;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // in this case the following member is initialized as nonempty rectangle
    Rectangle                   maBLIPSizeRectangle;

    // global static ItemPool for not-yet-inserted items
private:
    static SdrItemPool*         mpGlobalItemPool;

public:
    static SdrItemPool& GetGlobalDrawObjectItemPool();
    void SetRelativeWidth( double nValue );
    void SetRelativeWidthRelation( sal_Int16 eValue );
    void SetRelativeHeight( double nValue );
    void SetRelativeHeightRelation( sal_Int16 eValue );
    const double* GetRelativeWidth() const;
    sal_Int16 GetRelativeWidthRelation() const;
    const double* GetRelativeHeight() const;
    sal_Int16 GetRelativeHeightRelation() const;
    // evil calc grid/shape drawlayer syncing
    Point GetGridOffset() const { return aGridOffset; }
    void SetGridOffset( const Point& rGridOffset ){ aGridOffset = rGridOffset; }
protected:
    Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;

    // for GetDragComment
    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal=0) const;

    void ImpForcePlusData();

    OUString GetAngleStr(long nAngle, bool bNoDegChar = false) const;
    OUString GetMetrStr(long nVal, MapUnit eWantMap=MAP_MM, bool bNoUnitChars = false) const;

    /// @param bNotMyself = true: set only ObjList to dirty, don't mark this object as dirty.
    ///
    /// This is needed for instance for NbcMove, because usually one moves SnapRect and aOutRect
    /// at the same time to avoid recomputation.
public:
    virtual void SetRectsDirty(bool bNotMyself = false);
protected:

    // override if the class inherits from SdrObjPlusData:
    static SdrObjPlusData* NewPlusData();

protected:
    /// A derived class must override these 3 methods if it has own geometric
    /// data that must be saved for Undo.
    /// NewGeoData() creates an empty instance of a class derived from
    /// SdrObjGeoData.
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

protected:
    virtual ~SdrObject();

public:
    SdrObject();

    // frees the SdrObject pointed to by the argument
    // In case the object has an SvxShape, which has the ownership of the object, it
    // is actually *not* deleted.
    static  void    Free( SdrObject*& _rpObject );

    // this method is only for access from Property objects
    virtual void SetBoundRectDirty();

    virtual void SetObjList(SdrObjList* pNewObjList);
    SdrObjList* GetObjList() const { return pObjList;}

    virtual void SetPage(SdrPage* pNewPage);
    SdrPage* GetPage() const { return pPage;}

    virtual void SetModel(SdrModel* pNewModel);
    SdrModel* GetModel() const { return pModel;}
    SfxItemPool & GetObjectItemPool() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);
    const SfxBroadcaster* GetBroadcaster() const;

    void AddReference(SdrVirtObj& rVrtObj);
    void DelReference(SdrVirtObj& rVrtObj);
    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    // Layer interface
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
    virtual void SetLayer(SdrLayerID nLayer);
    // renaming GetLayerSet -> getMergedHierarchyLayerSet to make clear what happens here. rSet needs to be empty.
    void getMergedHierarchyLayerSet(SetOfByte& rSet) const;

    // UserCall interface
    void SetUserCall(SdrObjUserCall* pUser);
    SdrObjUserCall* GetUserCall() const { return pUserCall;}
    void SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect) const;

    // #i68101#
    // An object may have a user-set Name (Get/SetName()), e.g SdrGrafObj, SdrObjGroup
    // or SdrOle2Obj.
    // It may also have a Title and a Description for accessibility purposes.
    void SetName(const OUString& rStr);
    OUString GetName() const;
    void SetTitle(const OUString& rStr);
    OUString GetTitle() const;
    void SetDescription(const OUString& rStr);
    OUString GetDescription() const;

    // for group objects
    bool IsGroupObject() const;
    virtual SdrObjList* GetSubList() const;
    SdrObject* GetUpGroup() const;

    /// The order number (aka ZOrder, aka z-index) determines whether a
    /// SdrObject is located above or below another.  Objects are painted from
    /// lowest to highest order number.  If the order of SdrObjects in the
    /// SdrObjList is changed, the bObjOrdNumsDirty flag is set on the SdrPage
    /// and the next GetOrdNum() call recalculates the order number of all
    /// SdrObjects in the SdrObjList.
    sal_uInt32 GetOrdNum() const;

    // Warning: this method should only be used if you really know what you're doing
    sal_uInt32 GetOrdNumDirect() const { return nOrdNum;}

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
    sal_uInt32 GetNavigationPosition();

    // Set the position in the navigation position to the given value.
    // This method is typically used only by the model after a change to
    // the navigation order.
    // This method does not change the navigation position of other
    // objects.
    // Use SdrObjList::SetObjectNavigationPosition() instead.
    void SetNavigationPosition (const sal_uInt32 nPosition);

    // To make clearer that this method may trigger RecalcBoundRect and thus may be
    // expensive and sometimes problematic (inside a bigger object change You will get
    // non-useful BoundRects sometimes) i rename that method from GetBoundRect() to
    // GetCurrentBoundRect().
    virtual const Rectangle& GetCurrentBoundRect() const;

    // To have a possibility to get the last calculated BoundRect e.g for producing
    // the first rectangle for repaints (old and new need to be used) without forcing
    // a RecalcBoundRect (which may be problematical and expensive sometimes) i add here
    // a new method for accessing the last BoundRect.
    virtual const Rectangle& GetLastBoundRect() const;

    virtual void RecalcBoundRect();

    void BroadcastObjectChange() const;

    // set modified-flag in the model
    virtual void SetChanged();

    // Tooling for painting a single object to a OutputDevice. This will be needed as long
    // as not all painting is changed to use DrawContact objects.
    bool SingleObjectPainter(OutputDevice& rOut) const;
    bool LineGeometryUsageIsNecessary() const;

    // Returns a copy of the object. Every inherited class must reimplement this (in class Foo
    // it should be sufficient to do "virtual Foo* Clone() const { return CloneHelper< Foo >(); }".
    // Note that this function uses operator= internally.
    virtual SdrObject* Clone() const;

    // implemented mainly for the purposes of Clone()
    SdrObject& operator=(const SdrObject& rObj);

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
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
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
    virtual SdrObject* getFullDragClone() const;

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
    virtual Pointer GetCreatePointer() const;

    /// Polygon dragged by the user when creating the object
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    /// The methods Move, Resize, Rotate, Mirror, Shear, SetSnapRect and
    /// SetLogicRect call the corresponding Nbc-methods, send a Repaint
    /// broadcast and set the Modified state on the Model.
    /// Derived classes should usually only override the Nbc methods.
    /// Nbc means "no broadcast".
    virtual void NbcMove  (const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcCrop  (const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear (const Point& rRef, long nAngle, double tn, bool bVShear);

    virtual void Move  (const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Crop  (const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Rotate(const Point& rRef, long nAngle, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear (const Point& rRef, long nAngle, double tn, bool bVShear);

    /// The relative position of a SdrObject is the distance of the upper
    /// left corner of the logic bounding rectangle (SnapRect) to the anchor.
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual Point GetRelativePos() const;
    void ImpSetAnchorPos(const Point& rPnt);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void SetAnchorPos(const Point& rPnt);
    virtual const Point& GetAnchorPos() const;

    /// Snap is not done on the BoundRect but if possible on logic coordinates
    /// (i.e. without considering stroke width, ...)
    /// SetSnapRect() tries to size the Object so that it fits into the
    /// passed Rect (without stroke width, ...)
    virtual void RecalcSnapRect();
    virtual const Rectangle& GetSnapRect() const;
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void NbcSetSnapRect(const Rectangle& rRect);

    // Logic Rect: for the Rect for instance without regard to rotation angle, shear, ...
    virtual const Rectangle& GetLogicRect() const;
    virtual void SetLogicRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    // the default is to set the logic rect to the given rectangle rMaxRect. If the shape
    // has an intrinsic aspect ratio it may set the logic rect so the aspect
    // ratio is kept but still inside the rectangle rMaxRect.
    // If bShrinkOnly is set to true, the size of the current logic rect will not
    // be changed if it is smaller than the given rectangle rMaxRect.
    virtual void AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false );

    // rotation and shear angle
    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical = false) const;

    /// snap to special points of an Object (polygon points, center of circle)
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    // For objects, whose points can be moved individually.
    // (e.g. polygons, polylines, lines)
    // The points of those objects are selected (if necessary multiselection),
    // deleted, inserted, or as a multiselection moved or rotated...
    // Only such objects can have PlusHandles (e.g. the weights of an Bezier curve).
    virtual bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 i) const;
    void SetPoint(const Point& rPnt, sal_uInt32 i);
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i);

    // get all geometrical data for undo/redo
    virtual SdrObjGeoData* GetGeoData() const;
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    // ItemSet access
    const SfxItemSet& GetMergedItemSet() const;
    void SetMergedItem(const SfxPoolItem& rItem);
    void ClearMergedItem(const sal_uInt16 nWhich = 0);
    void SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems = false);
    const SfxPoolItem& GetMergedItem(const sal_uInt16 nWhich) const;

    // internal versions
protected:
    const SfxItemSet& GetObjectItemSet() const;
    void SetObjectItem(const SfxPoolItem& rItem);
    void SetObjectItemSet(const SfxItemSet& rSet);
    const SfxPoolItem& GetObjectItem(const sal_uInt16 nWhich) const;

    // get SfxMapUnit the object is using
    SfxMapUnit GetObjectMapUnit() const;

public:
    // syntactical sugar for ItemSet accesses
    void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems = false);

    // NotPersistAttr for Layer, ObjName, geometrical transformations etc.
    void TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);
    void NbcApplyNotPersistAttr(const SfxItemSet& rAttr);

    // if bDontRemoveHardAttr is false, set all attributes, which were set in the style sheet, to their default value
    // if true, all hard attributes keep their values
    void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    SfxStyleSheet* GetStyleSheet() const;

    virtual bool HasTextEdit() const;

    // returns true if TextEditMode started
    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);

    // keep text in outliner's format
    // SetOutlinerParaObject: transfer ownership of *pTextObject!
    void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual void NbcReformatText();
    virtual void ReformatText();

    void BurnInStyleSheetAttributes();

    // macro abilities, e.g. a rectangle as PushButton.
    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;
    bool IsMacroHit(const SdrObjMacroHitRec& rRec) const;

    // Connectors
    // (see also documentation in SvdoEdge.hxx, SdrEdgeObj, as well as SvdGlue.hxx and SvdGlEV.hxx)
    //
    // There are nodes and edges. In theory an edge can also be a node, but this isn't implemented yet.
    // A node has a number of glue points, onto which edges can glued to
    // An edge can be either
    // - without any connection to any node,
    // - or connected on one end to a node, while the other end is not connected,
    // - or connected on both ends with exactly one node each.
    // The edge is listener for its up to two nodes.
    // Whenever a node is moved or resized, all its connected edges follow.
    // This is also true for SetGluePoint()... on the node.
    // On the other hand, moving/resizing an edge breaks the connection.

    // is object a node?
    virtual bool IsNode() const;

    // automatic glue points:
    // a node object must provide four vertex and corner positions
    // usually 0: top, 1: right, 2: bottom, 3: left
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const;

    // usually:
    // 0: top-left, 1: top-right, 2: bottom-right, 3: bottom-left
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const;

    // list of all glue points, can be NULL
    virtual const SdrGluePointList* GetGluePointList() const;
    //virtual SdrGluePointList* GetGluePointList();

    // after changing the GluePointList, one has to call the object's SendRepaintBroadcast!
    virtual SdrGluePointList* ForceGluePointList();

    // to be set temporarily when transforming related object(?)
    void SetGlueReallyAbsolute(bool bOn);
    void NbcRotateGluePoints(const Point& rRef, long nAngle, double sn, double cs);
    void NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2);
    void NbcShearGluePoints (const Point& rRef, long nAngle, double tn, bool bVShear);

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
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const;
    SdrObject* ConvertToPolyObj(bool bBezier, bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;
    static SdrObject* ImpConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false);

    // if true, reference onto an object
    bool IsVirtualObj() const { return bVirtObj;}

    // is true, if object can probably be filled
    // is false, if object has probably line ends
    // is invalid, if this is a group object
    bool IsClosedObj() const { return bClosedObj;}

    bool IsEdgeObj() const { return bIsEdge;}
    bool Is3DObj() const { return bIs3DObj;}
    bool IsUnoObj() const { return bIsUnoObj;}
    void SetMarkProtect(bool bProt);
    bool IsMarkProtect() const { return bMarkProt;}
    void SetInserted(bool bIns);
    bool IsInserted() const { return bInserted;}
    void SetMoveProtect(bool bProt);
    bool IsMoveProtect() const { return bMovProt;}
    void SetResizeProtect(bool bProt);
    bool IsResizeProtect() const { return bSizProt;}
    void SetPrintable(bool bPrn);
    bool IsPrintable() const { return !bNoPrint;}
    void SetVisible(bool bVisible);
    bool IsVisible() const { return mbVisible;}
    void SetEmptyPresObj(bool bEpt);
    bool IsEmptyPresObj() const { return bEmptyPresObj;}
    void SetNotVisibleAsMaster(bool bFlg);
    bool IsNotVisibleAsMaster() const { return bNotVisibleAsMaster;}

    // #i25616#
    bool LineIsOutsideGeometry() const { return mbLineIsOutsideGeometry;}

    // #i25616#
    bool DoesSupportTextIndentingOnLineWidthChange() const { return mbSupportTextIndentingOnLineWidthChange;}

    // application specific data
    sal_uInt16 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const;

    void AppendUserData(SdrObjUserData* pData);

    // removes the record from the list and performs delete (FreeMem+Dtor).
    void DeleteUserData(sal_uInt16 nNum);

    // switch ItemPool for this object
    void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = nullptr);

    // access to the UNO representation of the shape
    virtual css::uno::Reference< css::uno::XInterface > getUnoShape();
    css::uno::WeakReference< css::uno::XInterface > getWeakUnoShape() const { return maWeakUnoShape; }

    static SdrObject* getSdrObjectFromXShape( const css::uno::Reference< css::uno::XInterface >& xInt );

    // sets a new UNO representation of the shape
    // This is only a public interface function. The actual work is
    // done by impl_setUnoShape().
    // Calling this function is only allowed for the UNO representation
    // itself!
    void setUnoShape(
            const css::uno::Reference<
                css::uno::XInterface>& _rxUnoShape);

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
    Rectangle GetBLIPSizeRectangle() const { return maBLIPSizeRectangle;}
    void SetBLIPSizeRectangle( const Rectangle& aRect );

    /// @see mbDoNotInsertIntoPageAutomatically
    void SetDoNotInsertIntoPageAutomatically(bool bSet);
    /// @see mbDoNotInsertIntoPageAutomatically
    bool IsDoNotInsertIntoPageAutomatically() const { return mbDoNotInsertIntoPageAutomatically;}

    // #i121917#
    virtual bool HasText() const;

    OString stringify() const;

    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const;

protected:
    /// Sets a new UNO shape
    ///
    /// The default implementation of this function sets the new UNO
    /// shape. Derived classes should override the function to handle
    /// any other actions that are needed when the shape is being
    /// changed.
    ///
    /// The implementation _must_ call the same method of its parent
    /// class (preferably as the first step)!
    virtual void impl_setUnoShape( const css::uno::Reference< css::uno::XInterface >& _rxUnoShape );

    // helper function for reimplementing Clone().
    template< typename T > T* CloneHelper() const;

private:
    // only for internal use!
    SvxShape* getSvxShape();

    // do not use directly, always use getSvxShape() if you have to!
    SvxShape*   mpSvxShape;
    css::uno::WeakReference< css::uno::XInterface >
                maWeakUnoShape;
    // HACK: Do not automatically insert newly created object into a page.
    // The user needs to do it manually later.
    bool mbDoNotInsertIntoPageAutomatically;
};

/**
 * Whoever creates his own objects must set a link in the SdrObjFactory class.
 * The handler must have the following signature:
 *      void Hdl(SdrObjFactory*)
 * He must take a look at the referenced instance's nInventor and nIdentifier values,
 * and must create a new drawing object instance accordingly.
 * He must also make the pNewObj pointer reference to this instance.
 */
class SVX_DLLPUBLIC SdrObjFactory
{
public:
    sal_uInt32                  nInventor;
    sal_uInt16                  nIdentifier;

    // for MakeNewObj():
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrObject*                  pNewObj;

    // for MakeNewObjUserData():
    SdrObject*                  pObj;
    SdrObjUserData*             pNewData;

private:

    static SVX_DLLPRIVATE SdrObject* CreateObjectFromFactory(
        sal_uInt32 nInventor, sal_uInt16 nIdentifier, SdrPage* pPage, SdrModel* pModel );

    SVX_DLLPRIVATE SdrObjFactory(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pNewPage, SdrModel* pNewModel);

public:
    static SdrObject* MakeNewObject(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pPage, SdrModel* pModel=nullptr);
    static SdrObject* MakeNewObject( sal_uInt32 nInventor, sal_uInt16 nIdentifier, const Rectangle& rSnapRect, SdrPage* pPage );
    static void InsertMakeObjectHdl(const Link<SdrObjFactory*,void>& rLink);
    static void RemoveMakeObjectHdl(const Link<SdrObjFactory*,void>& rLink);
    static void InsertMakeUserDataHdl(const Link<SdrObjFactory*,void>& rLink);
    static void RemoveMakeUserDataHdl(const Link<SdrObjFactory*,void>& rLink);
};

typedef tools::WeakReference< SdrObject > SdrObjectWeakRef;

template< typename T > T* SdrObject::CloneHelper() const
{
    OSL_ASSERT( typeid( T ) == typeid( *this ));
    T* pObj = dynamic_cast< T* >( SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),nullptr));
    if (pObj!=nullptr)
        *pObj=*static_cast< const T* >( this );
    return pObj;
}

#endif // INCLUDED_SVX_SVDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
