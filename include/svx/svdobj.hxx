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

#include <memory>
#include <typeinfo>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <svl/lstner.hxx>
#include <svl/poolitem.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdglue.hxx>
#include <svx/xdash.hxx>
#include <svx/xpoly.hxx>
#include <svx/xenum.hxx>
#include <svx/sdrobjectuser.hxx>
#include <svx/svxdllapi.h>
#include <svx/shapeproperty.hxx>
#include <tools/weakbase.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/timer.hxx>

#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// forward declarations

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
class PolyPolygon;
class SfxPoolItem;
class SdrVirtObj;
class SdrDragView;

namespace sdr
{
    namespace properties
    {
        class BaseProperties;
    } // end of namespace properties
} // end of namespace sdr

// #110094#
namespace sdr
{
    namespace contact
    {
        class ViewContact;
    } // end of namespace contact
} // end of namespace sdr

namespace svx
{
    class PropertyChangeNotifier;
}

// Defines

enum SdrObjKind {OBJ_NONE       = 0,  // abstract object (SdrObject)
                 OBJ_GRUP       = 1,  // object group
                 OBJ_LINE       = 2,  // line
                 OBJ_RECT       = 3,  // rectangle (round corners optional)
                 OBJ_CIRC       = 4,  // circle, ellipse
                 OBJ_SECT       = 5,  // circle section
                 OBJ_CARC       = 6,  // circle arc
                 OBJ_CCUT       = 7,  // circle cut
                 OBJ_POLY       = 8,  // polygon, PolyPolygon
                 OBJ_PLIN       = 9,  // PolyLine
                 OBJ_PATHLINE   =10,  // open Bezier-curve
                 OBJ_PATHFILL   =11,  // closed Bezier-curve
                 OBJ_FREELINE   =12,  // open free-hand line
                 OBJ_FREEFILL   =13,  // closed free-hand line
                 OBJ_SPLNLINE   =14,  // Natuerlicher kubischer Spline          (ni)
                 OBJ_SPLNFILL   =15,  // Periodischer kubischer Spline          (ni)
                 OBJ_TEXT       =16,  // text object
                 OBJ_TEXTEXT    =17,  // Texterweiterungsrahmen                 (ni)
                 OBJ_wegFITTEXT,      // FitToSize-Text (alle Zeilen gleich)
                 OBJ_wegFITALLTEXT,   // FitToSize-Text (Zeilenweise)           (ni)
                 OBJ_TITLETEXT  =20,  // Titeltext. Spezial-Textobjekt fuer StarDraw
                 OBJ_OUTLINETEXT=21,  // OutlineText. Spezial-Textobjekt fuer StarDraw
                 OBJ_GRAF       =22,  // Fremdgrafik - (StarView Graphic)
                 OBJ_OLE2       =23,  // OLE object
                 OBJ_EDGE       =24,  // Verbindungsobjekt fuer Konnektoren
                 OBJ_CAPTION    =25,  // Legendenobjekt
                 OBJ_PATHPOLY   =26,  // Polygon/PolyPolygon dargestellt durch SdrPathObj
                 OBJ_PATHPLIN   =27,  // Polyline dargestellt durch SdrPathObj
                 OBJ_PAGE       =28,  // object that represents a SdrPage
                 OBJ_MEASURE    =29,  // measurement object
                 OBJ_DUMMY      =30,  // dummy object for saving holes (to recover surrogates later)
                 OBJ_FRAME      =31,  // continiously activated aktives OLE (PlugIn-Frame or similar)
                 OBJ_UNO        =32,  // Universal Network Object packed into SvDraw object
                 OBJ_CUSTOMSHAPE=33,  // custom shape
                 OBJ_MEDIA      =34,  // media shape
                 OBJ_TABLE      =35,  // table
                 OBJ_MAXI};

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
    TYPEINFO();
    virtual ~SdrObjUserCall();
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect);
};

class SVX_DLLPUBLIC SdrObjMacroHitRec
{
public:
    Point                       aPos;
    Point                       aDownPos;
    OutputDevice*               pOut;
    const SetOfByte*            pVisiLayer;
    const SdrPageView*          pPageView;
    sal_uInt16                  nTol;
    bool                        bDown;

    SdrObjMacroHitRec();
};

// User data of a drawing object, e.g. application specific data.
// Every drawing object can have arbitrarily many such records (SV list).
// Whoever wants to save data here, must inherit from this and set a corresponding link in the factory.
class SVX_DLLPUBLIC SdrObjUserData
{
protected:
    sal_uInt32                      nInventor;
    sal_uInt16                      nIdentifier;
    sal_uInt16                      nVersion;

private:
    SVX_DLLPRIVATE void operator=(const SdrObjUserData& rData);        // not implemented
    SVX_DLLPRIVATE bool operator==(const SdrObjUserData& rData) const; // not implemented
    SVX_DLLPRIVATE bool operator!=(const SdrObjUserData& rData) const; // not implemented

public:
    TYPEINFO();

    SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId, sal_uInt16 nVer);
    SdrObjUserData(const SdrObjUserData& rData);
    virtual ~SdrObjUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    sal_uInt32 GetInventor() const;
    sal_uInt16 GetId() const;

    virtual bool HasMacro (const SdrObject* pObj) const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec, SdrObject* pObj);
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
};

class SdrObjUserDataList
{
    typedef boost::ptr_vector<SdrObjUserData> ListType;
    ListType maList;

public:
    SdrObjUserDataList();
    ~SdrObjUserDataList();

    size_t GetUserDataCount() const;
    const SdrObjUserData* GetUserData(size_t nNum) const;
    SdrObjUserData* GetUserData(size_t nNum);
    void AppendUserData(SdrObjUserData* pData);
    void DeleteUserData(size_t nNum);
};

// all geometrical data of an arbitrary object for use in undo/redo
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

// Bitsack for DrawObjects
class SdrObjPlusData
{
    friend class                SdrObject;

    SfxBroadcaster*             pBroadcast;    // broadcaster, if this object is referenced (bVirtObj=true). Also for connectors etc.
    SdrObjUserDataList*         pUserDataList; // application specific data
    SdrGluePointList*           pGluePoints;   // glue points for glueing object connectors

    // #i68101#
    // object name, title and description
    OUString aObjName;
    OUString aObjTitle;
    OUString aObjDescription;

public:
    TYPEINFO();
    SdrObjPlusData();
    virtual ~SdrObjPlusData();
    virtual SdrObjPlusData* Clone(SdrObject* pObj1) const;

    void SetGluePoints(const SdrGluePointList& rPts);
};

// provides information about various ZObject properties
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
    bool bGradientAllowed : 1;         // if false, object dooes not have an interactive gradient control
    bool bShearAllowed : 1;            // if false, object cannot be sheared
    bool bEdgeRadiusAllowed : 1;
    bool bNoOrthoDesired : 1;          // is true for Rect; is false for BMP, MTF
    bool bNoContortion : 1;            // if false, Kein verzerren (bei Crook) moeglich (nur true bei PathObj und Gruppierten PathObjs)
    bool bCanConvToPath : 1;           // if false, no conversion into PathObj possible
    bool bCanConvToPoly : 1;           // if false, no conversion into PolyObj possible
    bool bCanConvToContour : 1;        // if false, no conversion down to whole contour possible
    bool bCanConvToPathLineToArea : 1; // if false, no conversion into PathObj with transformation from LineToArea possible
    bool bCanConvToPolyLineToArea : 1; // if false, no conversion into PolyObj with transformation from LineToArea possible

    SdrObjTransformInfoRec();
};

// Abstract DrawObject

class SvxShape;
class SVX_DLLPUBLIC SdrObject: public SfxListener, public tools::WeakBase< SdrObject >
{
private:
    // ObjectUser section
    sdr::ObjectUserVector                                           maObjectUsers;

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
    SdrObjPlusData*             pPlusData;    // Broadcaster, UserData, connektors, ... (this is the Bitsack)

    sal_uInt32                  nOrdNum;      // order number of the object in the list

    SfxGrabBagItem*             pGrabBagItem; // holds the GrabBagItem property


    // Position in the navigation order.  SAL_MAX_UINT32 when not used.
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
    bool                        bEmptyPresObj : 1;     // empty presentation object (Draw)

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
    boost::optional<double>                                         mnRelativeWidth;
    sal_Int16                                                       meRelativeWidthRelation;
    boost::optional<double>                                         mnRelativeHeight;
    sal_Int16                                                       meRelativeHeightRelation;
public:
    static SdrItemPool& GetGlobalDrawObjectItemPool();
    void SetRelativeWidth( double nValue ) { mnRelativeWidth.reset( nValue ); }
    void SetRelativeWidthRelation( sal_Int16 eValue ) { meRelativeWidthRelation = eValue; }
    void SetRelativeHeight( double nValue ) { mnRelativeHeight.reset( nValue ); }
    void SetRelativeHeightRelation( sal_Int16 eValue ) { meRelativeHeightRelation = eValue; }
    boost::optional<double> GetRelativeWidth( ) const { return mnRelativeWidth; }
    sal_Int16 GetRelativeWidthRelation() const { return meRelativeWidthRelation; }
    boost::optional<double> GetRelativeHeight( ) const { return mnRelativeHeight; }
    sal_Int16 GetRelativeHeightRelation() const { return meRelativeHeightRelation; }
    // evil calc grid/shape drawlayer syncing
    Point GetGridOffset() const { return aGridOffset; }
    void SetGridOffset( const Point& rGridOffset ){ aGridOffset = rGridOffset; }
protected:
    void ImpDeleteUserData();
    SdrObjUserData* ImpGetMacroUserData() const;
    Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;

    // for GetDragComment
    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal=0) const;

    void ImpForcePlusData();

    OUString GetWinkStr(long nWink, bool bNoDegChar = false) const;
    OUString GetMetrStr(long nVal, MapUnit eWantMap=MAP_MM, bool bNoUnitChars = false) const;

    // bNotMyself=true means: set only ObjList to dirty, don't mark this object as dirty.
    // This is needed for instance for NbcMove, because usually one moves SnapRect and aOutRect
    // at the same time to avoid recomputation.
public:
    virtual void SetRectsDirty(sal_Bool bNotMyself = false);
protected:

    // override if the class inherits from SdrObjPlusData:
    virtual SdrObjPlusData* NewPlusData() const;

protected:
    // Diese 3 Methoden muss ein abgeleitetes Objekt ueberladen, wenn es eigene
    // geometrische Daten besitzt, die fuer den Undo-Fall gesichert werden
    // sollen. NewGeoData() erzeugt lediglich eine leere Instanz auf eine von
    // SdrObjGeoData abgeleitete Klasse.
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

protected:
    virtual ~SdrObject();

public:
    TYPEINFO();
    SdrObject();

    // frees the SdrObject pointed to by the argument
    // In case the object has an SvxShape, which has the ownership of the object, it
    // is actually *not* deleted.
    static  void    Free( SdrObject*& _rpObject );

    // this method is only for access from Property objects
    virtual void SetBoundRectDirty();

    virtual void SetObjList(SdrObjList* pNewObjList);
    SdrObjList* GetObjList() const;

    virtual void SetPage(SdrPage* pNewPage);
    SdrPage* GetPage() const;

    virtual void SetModel(SdrModel* pNewModel);
    SdrModel* GetModel() const;
    SdrItemPool* GetObjectItemPool() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);
    const SfxBroadcaster* GetBroadcaster() const;

    virtual void AddReference(SdrVirtObj& rVrtObj);
    virtual void DelReference(SdrVirtObj& rVrtObj);
    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    // Layer interface
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
    virtual void SetLayer(SdrLayerID nLayer);
    // renaming GetLayerSet -> getMergedHierarchyLayerSet to make clear what happens here. rSet needs to be empty.
    virtual void getMergedHierarchyLayerSet(SetOfByte& rSet) const;

    // UserCall interface
    void SetUserCall(SdrObjUserCall* pUser);
    SdrObjUserCall* GetUserCall() const;
    void SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect) const;

    // Such a reference point is for instance:
    // - a symbol's point that should snap to the raster when inserting the symbol
    // - or an image's fix point within an animation object
    virtual bool HasRefPoint() const;
    virtual Point GetRefPoint() const;
    virtual void SetRefPoint(const Point& rPnt);

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

    // Ueber die Objekt-Ordnungsnummer kann man feststellen, ob ein Objekt vor
    // oder hinter einem anderen liegt. Objekte mit kleinen Ordnungsnummern werden
    // zuerst gezeichnet, Objekte mit grossen Ordnungsnummern zuletzt.
    // Wird die Reihenfolge der Objekte in der Liste veraendert, so wird ein
    // Dirty-Flag gesetzt (an der Page). Beim naechsten SdrObject::GetOrdNum()
    // werden die Ordnungsnummer aller Objekte der Liste neu bestimmt.
    sal_uInt32 GetOrdNum() const;

    // Warning: this method should only be used if you really knows what you're doing
    sal_uInt32 GetOrdNumDirect() const;

    // setting the order number should only happen from the model or from the page
    void SetOrdNum(sal_uInt32 nNum);

    // GrabBagItem for interim interop purposes
    void GetGrabBagItem(com::sun::star::uno::Any& rVal) const;

    void SetGrabBagItem(const com::sun::star::uno::Any& rVal);

    // Return the position in the navigation order for the called object.
    // Note that this method may update the navigation position of the
    // called and of other SdrObjects.  Therefore this method can not be
    // const.
    // @return
    //     If no navigation position has been explicitly defined then the
    //     result of GetOrdNum() is returned.
    sal_uInt32 GetNavigationPosition (void);

    // Set the position in the navigation position to the given value.
    // This method is typically used only by the model after a change to
    // the navigation order.
    // This method does not change the navigation position of other
    // objects.
    // Use SdrObjList::SetObjectNavigationPosition() instead.
    void SetNavigationPosition (const sal_uInt32 nPosition);

    // #111111#
    // To make clearer that this method may trigger RecalcBoundRect and thus may be
    // expensive and sometimes problematic (inside a bigger object change You will get
    // non-useful BoundRects sometimes) i rename that method from GetBoundRect() to
    // GetCurrentBoundRect().
    virtual const Rectangle& GetCurrentBoundRect() const;

    // #111111#
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

    // Das Xor-Polygon wird von der View zu Draggen des Objektes benoetigt.
    // Alle Polygone innerhalb des PolyPolygon werden als PolyLine interpretiert.
    // Moechte man ein geschlossenes Polygon, so muss man es explizit schliessen.
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // Die Kontur fuer TextToContour
    virtual basegfx::B2DPolyPolygon TakeContour() const;

    // Ueber GetHdlCount gibt ein Objekt die Anzahl seiner Handles preis.
    // Im Normalfall werden dies 8 sein, bei einer Strecke 2. Bei Polygonobjekten
    // (Polygon,Spline,Bezier) kann die Handleanzahl wesentlich groesser werden.
    // Polygonobjekten wird ausserdem die Moeglichkeit eingeraeumt einen Punkt
    // eines selektierten Objekts zu selektieren. Das Handle dieses Punktes wird
    // dann durch einen Satz neuer Handles ausgetauscht (PlusHdl). Bei einem
    // Polygon wird das wohl ein einfacher Selektionshandle sein, bei einer
    // Bezierkurve dagegen koennen das schon bis zu 3 Handles werden (inkl Gewichte).
    // GetHdl() und GetPlusHdl() muessen Handleinstanzen mit new erzeugen!
    // Ein Objekt, das bei HasSpacialDrag() true liefert muss diese Methoden
    // zur Verfuegung stellen (inkl. FillHdlList).
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // Die Standardtransformationen (Move,Resize,Rotate,Mirror,Shear) werden von der
    // View uebernommen (TakeXorPoly(),...).
    // Objektspeziefisches draggen wie z.B. Eckenradius bei Rechtecken,
    // Stuetzstellen bei Splines, Gewichte bei Bezierkurven und Pointer von
    // Labelobjekten muss vom Objekt selbst gehandled werden. Um das Model
    // Statusfrei zu halten werden die Statusdaten an der View gehalten und dem
    // Objekt dann uebergeben. EndDrag liefrt im Normalfall true fuer Erfolg.
    // false kann zurueckgegeben werden, wenn das Dragging das Objekt nicht
    // veraendert hat, wobei dir evtl. Tatsache das die Maus nicht bewegt wurde
    // bereits von der View abgefangen wird.
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

    // Jedes Objekt muss in der Lage sein sich selbst interaktiv zu erzeugen.
    // Beim MausDown wird zunaechst ein neues Objekt erzeugt und dann seine
    // BegCreate()-Methode gerufen. Bei jedem MausMode wird dann MovCreate
    // gerufen. BrkCreate() bedeutet, dass der User die interaktive Objekt-
    // erzeugung abgebrochen hat. EndCreate() wird gerufen beim MouseUp-Event.
    // Liefert EndCreate() ein true, so ist die Objekterzeugung abgeschlossen;
    // das Objekt wird in die entsprechende Liste eingefuegt. Andernfalls
    // (EndCreate()==false) gehe ich davon aus, dass weitere Punkte zur
    // Objekterzeugung notwendig sind (Polygon,...). Der Parameter eCmd
    // enthaelt die Anzahl der Mausklicks (so die App diese durchreicht).
    // BckCreate() -> Letztes EndCreate() rueckgaengig machen (z.B. letzten
    // Polygonpunkt wieder loeschen).
    // ReturnCode: true=Weiter gehts, false=Create dadurch abgebrochen.
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat); // if true, Xor needs to be repainted
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);

    // damit holt man sich den Pointer, der das Createn dieses Objekts symbolisiert
    virtual Pointer GetCreatePointer() const;

    // Polygon das waehrend des Erzeugens aufgezogen wird
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    // Die Methoden Move, Resize, Rotate, Mirror, Shear, SetSnapRect und
    // SetLogicRect rufen jeweils die entsprechenden Nbc-Methoden, versenden einen
    // Repaint-Broadcast und setzen den Modified-Status am Model. Abgeleitete
    // Objekte sollten i.d.R. nur die Nbc-Methoden ueberladen.
    // Nbc bedeutet: 'NoBroadcast'.
    virtual void NbcMove  (const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear (const Point& rRef, long nWink, double tn, bool bVShear);

    virtual void Move  (const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear (const Point& rRef, long nWink, double tn, bool bVShear);

    // Die relative Position eines Zeichenobjektes ist die Entfernung der
    // linken oberen Eche des logisch umschliessenden Rechtecks (SnapRect)
    // zum Anker.
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual Point GetRelativePos() const;
    void ImpSetAnchorPos(const Point& rPnt);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void SetAnchorPos(const Point& rPnt);
    virtual const Point& GetAnchorPos() const;

    // Snap wird nicht auf dem BoundRect ausgefuehrt, sondern nach Moeglichkeit auf
    // logischen Koordinaten (also ohne Beruecksichtigung von Strichstaerke, ... ).
    // SetSnapRect() versucht das Objekt so hinzusizen, dass es in das uebergebene
    // Rect passt (ohne Strichstaerke, ...)
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

    // Zum Fangen von/auf ausgezeichneten Punkten eines Obj (Polygonpunkte,
    // Kreismittelpunkt, ...)
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    // For objects, whose points can be moved individually.
    // (e.g. polygons, polylines, lines)
    // The points of those objects are selected (if necessary multiselection),
    // deleted, inserted, or as a multiselection moved or rotated...
    // Only such objects can have PlusHandles (e.g. the weights of an Bezier curve).
    virtual sal_Bool IsPolyObj() const;
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
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);
    SfxStyleSheet* GetStyleSheet() const;

    virtual bool HasTextEdit() const;

    // returns true if TextEditMode started
    virtual sal_Bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);

    // keep text in outliner's format
    // SetOutlinerParaObject: transfer ownership of *pTextObject!
    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
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
    // (see also documentation in SvdoEdge.hxx, SdrEdgeObj,
    // as well as SvdGlue.hxx and SvdGlEV.hxx)
    // There are nodes and edges. In theory an edge can also be a node, but this isn't implemented yet.
    // A node has a number of glue points, onto which edges can glued to
    // An edge can be either
    // - without any connection to any node,
    // - or connected on one end to a node, while the other end is not connected,
    // - or connected on both ends with exactly one node each.
    // The edge is listener for its upto two nodes.
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
    void NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs);
    void NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2);
    void NbcShearGluePoints (const Point& rRef, long nWink, double tn, bool bVShear);

    // is object an edge?
    virtual bool IsEdge() const;

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
    // - generate PolyPolygon with two polygons with four supporting points each.
    // In each case the return value is a SdrObject*, because it is also
    // allowed to return group objects (e.g. for SdrTextObj).
    // In the case of the conversion from TextObj to PathObj,
    // both modi (bLineToArea=true/false) would be identical.
    // The methods' default implementations report "I'm unable to do this" (false/null).
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;
    SdrObject* ConvertToPolyObj(bool bBezier, bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;
    SdrObject* ImpConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;

    // if true, reference onto an object
    bool IsVirtualObj() const;

    // is true, if object can probably be filled
    // is false, if object has probably line ends
    // is invalid, if this is a group object
    bool IsClosedObj() const;

    bool IsEdgeObj() const;
    bool Is3DObj() const;
    bool IsUnoObj() const;
    void SetMarkProtect(bool bProt);
    bool IsMarkProtect() const;
    void SetInserted(bool bIns);
    bool IsInserted() const;
    void SetMoveProtect(bool bProt);
    bool IsMoveProtect() const;
    void SetResizeProtect(bool bProt);
    bool IsResizeProtect() const;
    void SetPrintable(bool bPrn);
    bool IsPrintable() const;
    void SetVisible(bool bVisible);
    bool IsVisible() const;
    void SetEmptyPresObj(bool bEpt);
    bool IsEmptyPresObj() const;
    void SetNotVisibleAsMaster(bool bFlg);
    bool IsNotVisibleAsMaster() const;

    // #i25616#
    bool LineIsOutsideGeometry() const;

    // #i25616#
    bool DoesSupportTextIndentingOnLineWidthChange() const;

    // application specific data
    sal_uInt16 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const;

    void AppendUserData(SdrObjUserData* pData);

    // removes the record from the list and performs delete (FreeMem+Dtor).
    void DeleteUserData(sal_uInt16 nNum);

    // switch ItemPool for this object
    void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = 0L);

    // access to the UNO representation of the shape
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape();
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > getWeakUnoShape() const { return maWeakUnoShape; }

    static SdrObject* getSdrObjectFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInt );

    // sets a new UNO representation of the shape
    //  This is only a public interface function. The actual work is
    //  done by impl_setUnoShape().
    // Calling this function is only allowed for the UNO representation
    // itself!
    void setUnoShape(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface>& _rxUnoShape);

    // retrieves the instance responsible for notifying changes in the properties of the shape associated with
    // the SdrObject
    //
    // @precond
    //     There already exists an SvxShape instance associated with the SdrObject
    // @throws ::com::sun::star::uno::RuntimeException
    //     if there does nt yet exists an SvxShape instance associated with the SdrObject.
    ::svx::PropertyChangeNotifier&
        getShapePropertyChangeNotifier();

    // notifies a change in the given property, to all applicable listeners registered at the associated SvxShape
    //
    // This method is equivalent to calling getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty ),
    // exception that it is allowed to be called when there does not yet exist an associated SvxShape - in which
    // case the method will silently return without doing anything.
    void    notifyShapePropertyChange( const ::svx::ShapeProperty _eProperty ) const;

    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;

    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);

    // #116168#
    // give info if object is in destruction
    bool IsInDestruction() const;

    // return if fill is != XFILL_NONE
    bool HasFillStyle() const;
    bool HasLineStyle() const;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // the following methods are used to control it;
    // usually this data makes no sense after the import is finished, since the object
    // might be resized
    Rectangle GetBLIPSizeRectangle() const;
    void SetBLIPSizeRectangle( const Rectangle& aRect );

    /// @see mbDoNotInsertIntoPageAutomatically
    void SetDoNotInsertIntoPageAutomatically(bool bSet);
    /// @see mbDoNotInsertIntoPageAutomatically
    bool IsDoNotInsertIntoPageAutomatically() const;

    // #i121917#
    virtual bool HasText() const;

    virtual OString stringify() const;

protected:
    // Sets a new UNO shape
    //
    // The default implementation of this function sets the new UNO
    // shape. Derived classes should override the function to handle
    // any other actions that are needed when the shape is being
    // changed.
    //
    // The implementation _must_ call the same method of its parent
    // class (preferably as the first step)!
    virtual void    impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape );

    // helper function for reimplementing Clone().
    template< typename T > T* CloneHelper() const;

private:
    // only for internal use!
    SvxShape* getSvxShape();

    // do not use directly, always use getSvxShape() if you have to!
    SvxShape*   mpSvxShape;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >
                maWeakUnoShape;
    // HACK: Do not automatically insert newly created object into a page.
    // The user needs to do it manually later.
    bool mbDoNotInsertIntoPageAutomatically;
};

// Whoever creates his own objects must set a link in the SdrObjFactory class.
// The handler must have the following signature:
//    void Hdl(SdrObjFactory*)
// He must take a look at the referenced instance's nInventor and nIdentifier values,
// and must create a new drawing object instance accordingly.
// He must also make the pNewObj pointer reference to this instance.
class SVX_DLLPUBLIC SdrObjFactory
{
public:
    sal_uInt32                      nInventor;
    sal_uInt16                      nIdentifier;

    // for MakeNewObj():
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrObject*                  pNewObj;

    // for MakeNewObjUserData():
    SdrObject*                  pObj;
    SdrObjUserData*             pNewData;

private:
    SVX_DLLPRIVATE SdrObjFactory(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pNewPage, SdrModel* pNewModel);

public:
    static SdrObject* MakeNewObject(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pPage, SdrModel* pModel=NULL);
    static void InsertMakeObjectHdl(const Link& rLink);
    static void RemoveMakeObjectHdl(const Link& rLink);
    static void InsertMakeUserDataHdl(const Link& rLink);
    static void RemoveMakeUserDataHdl(const Link& rLink);
};

typedef tools::WeakReference< SdrObject > SdrObjectWeakRef;

template< typename T > T* SdrObject::CloneHelper() const
{
    OSL_ASSERT( typeid( T ) == typeid( *this ));
    T* pObj = dynamic_cast< T* >( SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL));
    if (pObj!=NULL)
        *pObj=*static_cast< const T* >( this );
    return pObj;
}

#endif // INCLUDED_SVX_SVDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
