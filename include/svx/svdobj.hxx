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

#ifndef _SVDOBJ_HXX
#define _SVDOBJ_HXX

#include <memory>
#include <typeinfo>
#include <cppuhelper/weakref.hxx>
#include <vcl/mapmod.hxx>
#include <tools/string.hxx>
#include <tools/weakbase.hxx>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx> // fuer SdrLayerID
#include <svx/svdglue.hxx> // Klebepunkte
#include <svx/xdash.hxx>
#include <svx/xpoly.hxx>
#include <svx/xenum.hxx>
#include <vcl/bitmapex.hxx>
#include <svx/sdrobjectuser.hxx>
#include "svx/svxdllapi.h"
#include "svx/shapeproperty.hxx"
#include <svl/poolitem.hxx>

#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

//************************************************************
//   Vorausdeklarationen
//************************************************************

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

//************************************************************
//   Defines
//************************************************************

enum SdrObjKind {OBJ_NONE       = 0,  // Abstraktes Objekt (SdrObject)
                 OBJ_GRUP       = 1,  // Objektgruppe
                 OBJ_LINE       = 2,  // Strecke
                 OBJ_RECT       = 3,  // Rechteck ww. mit runden Ecken
                 OBJ_CIRC       = 4,  // Kreis, Ellipse
                 OBJ_SECT       = 5,  // Kreissektor
                 OBJ_CARC       = 6,  // Kreisbogen
                 OBJ_CCUT       = 7,  // Kreisabschnitt
                 OBJ_POLY       = 8,  // Polygon, PolyPolygon
                 OBJ_PLIN       = 9,  // PolyLine
                 OBJ_PATHLINE   =10,  // Offene Bezierkurve
                 OBJ_PATHFILL   =11,  // Geschlossene Bezierkurve
                 OBJ_FREELINE   =12,  // Offene Freihandlinie
                 OBJ_FREEFILL   =13,  // Geschlossene Freihandlinie
                 OBJ_SPLNLINE   =14,  // Natuerlicher kubischer Spline          (ni)
                 OBJ_SPLNFILL   =15,  // Periodischer kubischer Spline          (ni)
                 OBJ_TEXT       =16,  // Textobjekt
                 OBJ_TEXTEXT    =17,  // Texterweiterungsrahmen                 (ni)
                 OBJ_wegFITTEXT,      // FitToSize-Text (alle Zeilen gleich)
                 OBJ_wegFITALLTEXT,   // FitToSize-Text (Zeilenweise)           (ni)
                 OBJ_TITLETEXT  =20,  // Titeltext. Spezial-Textobjekt fuer StarDraw
                 OBJ_OUTLINETEXT=21,  // OutlineText. Spezial-Textobjekt fuer StarDraw
                 OBJ_GRAF       =22,  // Fremdgrafik - (StarView Graphic)
                 OBJ_OLE2       =23,  // OLE-Objekt
                 OBJ_EDGE       =24,  // Verbindungsobjekt fuer Konnektoren
                 OBJ_CAPTION    =25,  // Legendenobjekt
                 OBJ_PATHPOLY   =26,  // Polygon/PolyPolygon dargestellt durch SdrPathObj
                 OBJ_PATHPLIN   =27,  // Polyline dargestellt durch SdrPathObj
                 OBJ_PAGE       =28,  // Objekt, das eine SdrPage darstellt
                 OBJ_MEASURE    =29,  // Bemassungsobjekt
                 OBJ_DUMMY      =30,  // Dummyobjekt zum speichern von Luecken (zur anschliessenden Wiederherstellung der Surrogate)
                 OBJ_FRAME      =31,  // staendig aktives OLE (PlugIn-Frame oder sowas)
                 OBJ_UNO        =32,  // Universal Network Object im SvDraw-Obj eingepackt
                 OBJ_CUSTOMSHAPE=33,  // CustomShape
                 OBJ_MEDIA      =34,  // Media shape
                 OBJ_TABLE      =35,  // Table
                 OBJ_MAXI};

enum SdrUserCallType {SDRUSERCALL_MOVEONLY,         // Nur verschoben, Groesse unveraendert
                      SDRUSERCALL_RESIZE,           // Groesse und evtl. auch Pos veraendert
                      SDRUSERCALL_CHGATTR,          // Attribute veraendert. Moeglicherweise neue Groesse wg. Linienbreite
                      SDRUSERCALL_DELETE,           // Obj gibt es gleich nicht mehr. Schon keine Attr mehr.
                      SDRUSERCALL_COPY,             // Zuweisungsoperator gerufen. Kann alles geaendert sein
                      SDRUSERCALL_INSERTED,         // In eine Objektliste (z.B. Page) eingefuegt
                      SDRUSERCALL_REMOVED,          // Aus der Objektliste entfernt
                      SDRUSERCALL_CHILD_MOVEONLY,   // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_RESIZE,     // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_CHGATTR,    // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_DELETE,     // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_COPY,       // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_INSERTED,   // Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_REMOVED};   // Ein Child einer Gruppe hat sich veraendert

//************************************************************
//   Hilfsklasse SdrObjUserCall
//************************************************************

class SVX_DLLPUBLIC SdrObjUserCall
{
public:
    TYPEINFO();
    virtual ~SdrObjUserCall();
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect);
};

//************************************************************
//   Hilfsklasse SdrObjMacroHitRec
//************************************************************

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

//************************************************************
//   Hilfsklasse SdrObjUserData
//
// Anwenderdaten an einem Zeichenobjekt, z.B. applikationsspezifische Daten.
// Jedes Zeichenobjekt kann beliebig viele dieser Records haben (SV-Liste).
// Wer hier Daten halten will, muss sich ableiten und auch an der Factory
// einen entsprechenden Link setzen.
//
//************************************************************

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

//************************************************************
//   Hilfsklasse SdrObjUserDataList
//************************************************************

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

//************************************************************
//   Hilfsklasse SdrObjGeoData
//
// Alle geometrischen Daten eines beliebigen Objektes zur ??bergabe an's Undo/Redo
//
//************************************************************

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

//************************************************************
//   Hilfsklasse SdrObjPlusData
//
// Bitsack fuer DrawObjekte
//
//************************************************************

class SdrObjPlusData
{
    friend class                SdrObject;

    SfxBroadcaster*             pBroadcast;    // Broadcaster, falls dieses Obj referenziert wird (bVirtObj=true). Auch fuer Konnektoren etc.
    SdrObjUserDataList*         pUserDataList; // applikationsspeziefische Daten
    SdrGluePointList*           pGluePoints;   // Klebepunkte zum Ankleben von Objektverbindern

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

//************************************************************
//   Hilfsklasse SdrObjTransformInfoRec
//
// gibt Auskunft ueber verschiedene Eigenschaften eines ZObjects
//
//************************************************************

class SVX_DLLPUBLIC SdrObjTransformInfoRec
{
public:
    bool bSelectAllowed : 1;     // false=Obj kann nicht selektiert werden
    bool bMoveAllowed : 1;       // false=Obj kann nicht verschoben werden
    bool bResizeFreeAllowed : 1; // false=Obj kann nicht frei resized werden
    bool bResizePropAllowed : 1; // false=Obj kann nichtmal proportional resized werden
    bool bRotateFreeAllowed : 1; // false=Obj kann nicht frei gedreht werden
    bool bRotate90Allowed : 1;   // false=Obj kann nichtmal im 90deg Raster gedreht werden
    bool bMirrorFreeAllowed : 1; // false=Obj kann nicht frei gespiegelt werden
    bool bMirror45Allowed : 1;   // false=Obj kann nichtmal ueber Achse im 45deg Raster gespiegelt werden
    bool bMirror90Allowed : 1;   // false=Obj kann ebenfalls nicht ueber Achse im 90deg Raster gespiegelt werden
    bool bTransparenceAllowed : 1; // false=Obj does not have an interactive transparence control
    bool bGradientAllowed : 1; // false=Obj dooes not have an interactive gradient control
    bool bShearAllowed : 1;      // false=Obj kann nicht verzerrt werden
    bool bEdgeRadiusAllowed : 1;
    bool bNoOrthoDesired : 1;    // true bei Rect; ... false bei BMP,MTF;
    bool bNoContortion : 1;      // false=Kein verzerren (bei Crook) moeglich (nur true bei PathObj und Gruppierten PathObjs)
    bool bCanConvToPath : 1;     // false=Keine Konvertierung in PathObj moeglich
    bool bCanConvToPoly : 1;     // false=Keine Konvertierung in PolyObj moeglich
    bool bCanConvToContour : 1;     // false=no conversion down to whole contour possible
    bool bCanConvToPathLineToArea : 1; // false=Keine Konvertierung in PathObj moeglich mit Wandlung von LineToArea
    bool bCanConvToPolyLineToArea : 1; // false=Keine Konvertierung in PolyObj moeglich mit Wandlung von LineToArea

    SdrObjTransformInfoRec();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Abstraktes DrawObject
////////////////////////////////////////////////////////////////////////////////////////////////////

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

    ///////////////////////////////////////////////////////////////////////////////
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
    Rectangle                   aOutRect;     // umschliessendes Rechteck fuer Paint (inkl. LineWdt, ...)
    Point                       aAnchor;      // Ankerposition (Writer)
    Point                       aGridOffset;  // hack (Calc)
    SdrObjList*                 pObjList;     // Liste, in dem das Obj eingefuegt ist.
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrObjUserCall*             pUserCall;
    SdrObjPlusData*             pPlusData;    // Broadcaster, UserData, Konnektoren, ... (Das ist der Bitsack)

    sal_uInt32                  nOrdNum;      // Rangnummer des Obj in der Liste

    /** Position in the navigation order.  SAL_MAX_UINT32 when not used.
    */
    sal_uInt32                  mnNavigationPosition;
    SdrLayerID                  mnLayerID;

    // Objekt zeigt nur auf ein Anderes
    bool                        bVirtObj : 1;
    bool                        bSnapRectDirty : 1;
    bool                        bNetLock : 1;   // ni
    bool                        bInserted : 1;  // nur wenn true gibt's RepaintBroadcast & SetModify
    bool                        bGrouped : 1;   // Member eines GroupObjektes?

    // Die folgende Flags werden gestreamt
    bool                        bMovProt : 1; // If true, the position is protected
    bool                        bSizProt : 1; // If true, the size is protected
    bool                        bNoPrint : 1; // If true, the object is not printed.
    bool                        mbVisible : 1; // If false, the object is not visible on screen (but maybe on printer, depending on bNoprint
    // Wenn bEmptyPresObj true ist, handelt es sich um ein
    // Praesentationsobjekt, dem noch kein Inhalt zugewiesen
    // wurde. Default ist das Flag auf FALSE. Die Verwaltung
    // uebernimmt die Applikation. Im Zuweisungsoperator sowie
    // beim Clone wird das Flag nicht mitkopiert!
    // Das Flag ist persistent.
    bool                        bEmptyPresObj : 1;     // Leeres Praesentationsobjekt (Draw)

    // true=Objekt ist als Objekt der MasterPage nicht sichtbar
    bool                        bNotVisibleAsMaster : 1;

    // true=Es handelt sich hierbei um ein geschlossenes Objekt, also nicht Linie oder Kreisbogen ...
    bool                        bClosedObj : 1;

    bool                        bIsEdge : 1;
    bool                        bIs3DObj : 1;
    bool                        bMarkProt : 1;  // Markieren verboten. Persistent
    bool                        bIsUnoObj : 1;
    bool                        bNotMasterCachable : 1;

    // #i25616#
    bool                        mbLineIsOutsideGeometry : 1;

    // #i25616#
    bool                        mbSupportTextIndentingOnLineWidthChange : 1;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // in this case the following member is initialized as nonempty rectangle
    Rectangle                   maBLIPSizeRectangle;

    // global static ItemPool for not-yet-insetred items
private:
    static SdrItemPool*         mpGlobalItemPool;
    boost::optional<double>                                         mnRelativeWidth;
    boost::optional<double>                                         mnRelativeHeight;
public:
    static SdrItemPool& GetGlobalDrawObjectItemPool();
    void SetRelativeWidth( double nValue ) { mnRelativeWidth.reset( nValue ); }
    void SetRelativeHeight( double nValue ) { mnRelativeHeight.reset( nValue ); }
    boost::optional<double> GetRelativeWidth( ) const { return mnRelativeWidth; }
    boost::optional<double> GetRelativeHeight( ) const { return mnRelativeHeight; }
    // evil calc grid/shape drawlayer syncing
    Point GetGridOffset() const { return aGridOffset; }
    void SetGridOffset( const Point& rGridOffset ){ aGridOffset = rGridOffset; }
protected:
    void ImpDeleteUserData();
    SdrObjUserData* ImpGetMacroUserData() const;
    Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;

    // Fuer GetDragComment
    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal=0) const;

    void ImpForcePlusData();

    OUString GetWinkStr(long nWink, bool bNoDegChar = false) const;
    OUString GetMetrStr(long nVal, MapUnit eWantMap=MAP_MM, bool bNoUnitChars = false) const;

    // bNotMyself=true bedeutet: Nur die ObjList auf Dirty setzen, nicht mich.
    // Wird z.B. benoetigt fuer NbcMove, denn da movt man SnapRect und aOutRect
    // i.d.R. gleich mit um die Neuberechnung zu sparen.
public:
    virtual void SetRectsDirty(sal_Bool bNotMyself = false);
protected:

    // ueberladen, wenn man sich von SdrObjPlusData abgeleitet hat:
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

    /** frees the SdrObject pointed to by the argument

        In case the object has an SvxShape, which has the ownership of the object, it
        is actually *not* deleted.
    */
    static  void    Free( SdrObject*& _rpObject );

    // This method is only for access from Property objects
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

    // Ein solcher Referenzpunkt ist z.B. der Punkt eines Symbols, der
    // der beim Einfuegen des Symbols auf das Raster gefangen werden soll
    // oder der Fixpunkt eines Bildes innerhalb eines Animationsobjektes.
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

    // Fuer Gruppenobjekte
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

    // Diese Methode sollte nur verwendet werden, wenn man ganz  genau weiss,
    // was man macht:
    sal_uInt32 GetOrdNumDirect() const;

    // Das Setzen der Ordnungsnummer sollte nur vom Model bzw. von der Page
    // geschehen.
    void SetOrdNum(sal_uInt32 nNum);

    /** Return the position in the navigation order for the called object.
        Note that this method may update the navigation position of the
        called and of other SdrObjects.  Therefore this method can not be
        const.
        @return
            If no navigation position has been explicitly defined then the
            result of GetOrdNum() is returned.
    */
    sal_uInt32 GetNavigationPosition (void);

    /** Set the position in the navigation position to the given value.
        This method is typically used only by the model after a change to
        the navigation order.
        This method does not change the navigation position of other
        objects.
        Use SdrObjList::SetObjectNavigationPosition() instead.
    */
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

    // Modified-Flag am Model setzen
    virtual void SetChanged();

    // Tooling for painting a single object to a OutputDevice. This will be needed as long
    // as not all painting is changed to use DrawContact objects.
    bool SingleObjectPainter(OutputDevice& rOut) const;
    bool LineGeometryUsageIsNecessary() const;

    /**
      Returns a copy of the object. Every inherited class must reimplement this (in class Foo
      it should be sufficient to do "virtual Foo* Clone() const { return CloneHelper< Foo >(); }".
      Note that this function uses operator= internally.
    */
    virtual SdrObject* Clone() const;
    /**
      Implemented mainly for the purposes of Clone().
    */
    SdrObject& operator=(const SdrObject& rObj);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
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
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
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
    // RetrunCode: true=Weiter gehts, false=Create dadurch abgebrochen.
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat); // TRUE=Xor muss repainted werden
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

    // Logic Rect: Beim Rect z.B. ohne Beruecksichtigung des Drehwinkels, Shear, ...
    virtual const Rectangle& GetLogicRect() const;
    virtual void SetLogicRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    /** the defaul is to set the logic rect to the given rectangle rMaxRect. If the shape
        has an intrinsic aspect ratio it may set the logic rect so the aspect
        ratio is kept but still inside the rectangle rMaxRect.

        If bShrinkOnly is set to true, the size of the current logic rect will not
        be changed if it is smaller than the given rectangle rMaxRect. */
    virtual void AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false );

    // Drehwinkel und Shear
    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical = false) const;

    // Zum Fangen von/auf ausgezeichneten Punkten eines Obj (Polygonpunkte,
    // Kreismittelpunkt, ...)
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    // Fuer Objekte, bei denen jeder einzelne Punkt verschoben werden kann,
    // z.B. Polygone, Polylines, Linien, ... . Bei diesen Objekten werden
    // Punkte selektiert (ggf. Mehrfachselektion), geloescht, eingefuegt,
    // als Mehrfachselektion verschoben und gedreht, ...
    // Nur solche Objekte koennen PlusHandles haben (z.B. die Gewichte an den
    // Bezierkurven.
    virtual sal_Bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 i) const;
    void SetPoint(const Point& rPnt, sal_uInt32 i);
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i);

    // Alle geometrischen Daten holen fuer's Undo/Redo
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

    // NotPersistAttr fuer Layer, ObjName, geometrische Transformationen, ...
    void TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);
    void NbcApplyNotPersistAttr(const SfxItemSet& rAttr);

    // bDontRemoveHardAttr=FALSE: alle in der Vorlage gesetzten Attribute werden am
    // Zeichenobjekt auf Default gesetzt; TRUE: alle harten Attribute bleiben erhalten.
    void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);
    SfxStyleSheet* GetStyleSheet() const;

    // TextEdit
    virtual bool HasTextEdit() const;

    // Return==TRUE: TextEditMode gestartet
    virtual sal_Bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);

    // Text wird im Format des Outliners gehalten
    // SetOutlinerParaObject: Eigentumsuebereignung von *pTextObject!
    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual void NbcReformatText();
    virtual void ReformatText();

    void BurnInStyleSheetAttributes();

    // Macrofaehigkeit, z.B. ein Rechteck als PushButton.
    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;
    bool IsMacroHit(const SdrObjMacroHitRec& rRec) const;

    // Konnektoren. (siehe auch Dokumentation in SvdoEdge.HXX, SdrEdgeObj
    //               sowie SvdGlue.HXX und SvdGlEV.HXX)
    // Es gibt Knoten und Kanten. Eine Kante kann theoretisch auch Knoten
    // sein, das ist jedoch in dieser Version noch nicht implementiert.
    // Ein Knoten hat eine Anzahl von Klebepunkten, wo Kanten angeklebt
    // werden koennen.
    // Eine Kante kann
    // - ohne Verbindungen sein
    // - an einem Ende auf der Wiese stehen und am anderen Ende an
    //   genau einem Knoten gebunden sein
    // - an beiden Enden mit jeweils genau einem Knoten verbunden sein.
    // Die Kante ist Listener bei seinen bis zu 2 Knoten.
    // Wird der Knoten verschoben oder Resized, folgen alle angebundenen
    // Kanten. Ebenso bei SetGluePoint()... am Knoten.
    // Beim Verschieben/Resizen der Kante wird dagegen die Verbindung
    // geloesst.
    // Objekt ist ein Knoten?
    virtual bool IsNode() const;

    // Automatische Klebepunkte:
    // je 4 Scheitelpunkt- und Eckpositionen muss ein Knotenobjekt liefern
    // i.d.R. 0=oben, 1=rechts, 2=unten, 3=links
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const;

    // i.d.R. 0=linksoben, 1=rechtsoben, 2=rechtsunten, 3=linksunten
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const;

    // Liste aller Klebepunkte. Kann NULL sein.
    virtual const SdrGluePointList* GetGluePointList() const;
    //virtual SdrGluePointList* GetGluePointList();

    // Nach veraendern der GluePointList muss man am Obj SendRepaintBroadcast rufen!
    virtual SdrGluePointList* ForceGluePointList();

    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    void SetGlueReallyAbsolute(bool bOn);
    void NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs);
    void NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2);
    void NbcShearGluePoints (const Point& rRef, long nWink, double tn, bool bVShear);

    // Objekt ist eine Kante?
    virtual bool IsEdge() const;

    // bTail1=TRUE: Linienanfang, sonst LinienEnde
    // pObj=NULL: Disconnect
    virtual void ConnectToNode(bool bTail1, SdrObject* pObj);
    virtual void DisconnectFromNode(bool bTail1);
    virtual SdrObject* GetConnectedNode(bool bTail1) const;

    /** sets the writing mode of the object's context

        Objects which itself do not support different writing modes will ignore this call.

        Objects which support different writing modes, but have an own, explicit writing mode set,
        will also ignore this call.

        Objects which support different writing modes, and whose own mode is set to css.text.WritingMode2.CONTEXT,
        will use the given writing mode to calculate their "effective writing mode".

        The behaviour of this method is undefined if you pass css.text.WritingMode2.CONTEXT.

        @param _nContextWritingMode
            the effective writing mode of the context of the object
    */
    virtual void    SetContextWritingMode( const sal_Int16 _nContextWritingMode );

    // Wenn ein Objekt in der Lage ist, sich in ein Polygon oder in eine
    // Bezierkurve (oder beides) zu verwandeln, dann sollten die folgenden
    // Methoden ueberladen werden.
    // Z.B. Ein RectObj mit Strichstaerke 10, SOLID_PEN in Polygon wandeln:
    // Im Modus bLineToArea=false soll ein PolyObj mit 4 Stuetzstellen,
    // Strichstaerke 10 und SOLiD_PEN erzeugt werden.
    // Im Modus bLineToArea=true dagegen soll das generierte Obj immer ein
    // LinienAttribut NULL_PEN haben, und die Linie (auch Linienmuster) selbst
    // durch Polygonflaechen emuliert werden, die dadurch anschliessend vom
    // Anwender manipuliert werden koennen. Das RectObj kann sich somit also
    // nur korrekt konvertieren, wenn es als Flaechenattribut NULL_BRUSH
    // besitzt. Es muesste in diesem Fall dann:
    // - SOLID_BRUSH mit der Farbe des gesetzten Pen setzen
    // - NULL_PEN setzen
    // - PolyPolygon mit 2 Polygonen zu je 4 Stuetzstellen generieren.
    // Der Returnwert ist jeweils ein SdrObject*, da als Rueckgabe zur Not
    // auch Gruppenobjekte erlaubt sind (z.B. fuer SdrTextObj).
    // Bei der Konvertierung von TextObj nach PathObj wird es wohl so sein,
    // dass beide Modi (bLineToArea=true/false) identisch sind.
    // Defaulted sind diese Methoden auf "Ich kann das nicht" (FALSE/NULL).
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;
    SdrObject* ConvertToPolyObj(bool bBezier, bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;
    SdrObject* ImpConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;

    // TRUE: Referenz auf ein Obj
    bool IsVirtualObj() const;

    // true=Obj kann warsch. gefuellt werden; false=Obj kann warsch. Linienenden haben.
    // ungueltig, wenn es sich um ein GroupObj handelt.
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

    // applikationsspeziefische Daten
    sal_uInt16 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const;

    void AppendUserData(SdrObjUserData* pData);

    // Delete entfernt den Record aus der Liste und ruft
    // ein delete (FreeMem+Dtor).
    void DeleteUserData(sal_uInt16 nNum);

    // ItemPool fuer dieses Objekt wechseln
    void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = 0L);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // access to the UNO representation of the shape
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape();
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > getWeakUnoShape() const { return maWeakUnoShape; }

    static SdrObject* getSdrObjectFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInt );

    /** Sets a new UNO representation of the shape
      *
      * This is only a public interface function. The actual work is
      * done by impl_setUnoShape().
      *
      * Calling this function is only allowed for the UNO representation
      * itself!
      */
    void setUnoShape(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface>& _rxUnoShape);

    /** retrieves the instance responsible for notifying changes in the properties of the shape associated with
        the SdrObject

        @precond
            There already exists an SvxShape instance associated with the SdrObject
        @throws ::com::sun::star::uno::RuntimeException
            if there does nt yet exists an SvxShape instance associated with the SdrObject.
    */
    ::svx::PropertyChangeNotifier&
        getShapePropertyChangeNotifier();

    /** notifies a change in the given property, to all applicable listeners registered at the associated SvxShape

        This method is equivalent to calling getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty ),
        exception that it is allowed to be called when there does not yet exist an associated SvxShape - in which
        case the method will silently return without doing anything.
    */
    void    notifyShapePropertyChange( const ::svx::ShapeProperty _eProperty ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
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
    // Give info if object is in destruction
    bool IsInDestruction() const;

    // return if fill is != XFILL_NONE
    bool HasFillStyle() const;
    bool HasLineStyle() const;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // the following methods are used to control it;
    // usually this data makes no sence after the import is finished, since the object
    // might be resized
    Rectangle GetBLIPSizeRectangle() const;
    void SetBLIPSizeRectangle( const Rectangle& aRect );

    /// @see mbDoNotInsertIntoPageAutomatically
    void SetDoNotInsertIntoPageAutomatically(bool bSet);
    /// @see mbDoNotInsertIntoPageAutomatically
    bool IsDoNotInsertIntoPageAutomatically() const;

    // #i121917#
    virtual bool HasText() const;

protected:
    /** Sets a new UNO shape
      *
      * The default implementation of this function sets the new UNO
      * shape. Derived classes should override the function to handle
      * any other actions that are needed when the shape is being
      * changed.
      *
      * The implementation _must_ call the same method of its parent
      * class (preferably as the first step)!
      */
    virtual void    impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape );

    /**
     Helper function for reimplementing Clone().
    */
    template< typename T > T* CloneHelper() const;

private:
    /** only for internal use!
    */
    SvxShape* getSvxShape();

    /** do not use directly, always use getSvxShape() if you have to! */
    SvxShape*   mpSvxShape;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >
                maWeakUnoShape;
    /** HACK: Do not automatically insert newly created object into a page.
      *
      * The user needs to do it manually later.
      */
    bool mbDoNotInsertIntoPageAutomatically;
};

//************************************************************
//   SdrDummyObj
//
// Wer sich eigene Objekte schafft muss einen Link in der Klasse
// SdrObjFactory setzen. Der Handler hat folgendes aussehen:
//    void Hdl(SdrObjFactory*)
// Er muss sich aus der uebergebenen Instanz die Werte nInventor und
// nIdentifier ansehen und entsprechend mit new eine Zeichenobjektinstanz
// erzeugen. Einen Zeiger auf diese Instanz hat er in der Membervariablen
// pNewObj zu hinterlassen.
//
//************************************************************

class SVX_DLLPUBLIC SdrObjFactory
{
public:
    sal_uInt32                      nInventor;
    sal_uInt16                      nIdentifier;

    // fuer MakeNewObj():
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrObject*                  pNewObj;

    // fuer MakeNewObjUserData():
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

#endif //_SVDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
