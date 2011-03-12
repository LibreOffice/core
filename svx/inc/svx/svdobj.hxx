/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVDOBJ_HXX
#define _SVDOBJ_HXX

#include <memory>
#include <cppuhelper/weakref.hxx>
#include <vcl/mapmod.hxx>
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

/* nur voruebergehend, weil sonst MUSS-Aenderung */
// #define nLayerID nLayerId

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

class SdrObjMacroHitRec
{
public:
    Point                       aPos;
    Point                       aDownPos;
    OutputDevice*               pOut;
    const SetOfByte*            pVisiLayer;
    const SdrPageView*          pPageView;
    sal_uInt16                      nTol;
    bool                        bDown;

public:
    SdrObjMacroHitRec()
    :   pOut(NULL),
        pVisiLayer(NULL),
        pPageView(NULL),
        nTol(0),
        bDown(sal_False)
    {}
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
    SVX_DLLPRIVATE void operator=(const SdrObjUserData& rData);        // nicht implementiert
    SVX_DLLPRIVATE sal_Bool operator==(const SdrObjUserData& rData) const; // nicht implementiert
    SVX_DLLPRIVATE sal_Bool operator!=(const SdrObjUserData& rData) const; // nicht implementiert

public:
    TYPEINFO();

    SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId, sal_uInt16 nVer)
    :   nInventor(nInv),
        nIdentifier(nId),
        nVersion(nVer)
    {}
    SdrObjUserData(const SdrObjUserData& rData)
    :   nInventor(rData.nInventor),
        nIdentifier(rData.nIdentifier),
        nVersion(rData.nVersion)
    {}
    virtual ~SdrObjUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    sal_uInt32  GetInventor() const { return nInventor; }
    sal_uInt16  GetId() const { return nIdentifier; }

    virtual bool HasMacro (const SdrObject* pObj) const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec, SdrObject* pObj);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
};

//************************************************************
//   Hilfsklasse SdrObjUserDataList
//************************************************************

class SdrObjUserDataList
{
    Container                   aList;

public:
    SdrObjUserDataList()
    :   aList(1024,4,4)
    {}
    ~SdrObjUserDataList() { Clear(); }

    void Clear();
    sal_uInt16 GetUserDataCount() const { return sal_uInt16(aList.Count()); }
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const { return (SdrObjUserData*)aList.GetObject(nNum); }
    void InsertUserData(SdrObjUserData* pData, sal_uInt16 nPos=0xFFFF) { aList.Insert(pData,nPos); }
    SdrObjUserData* RemoveUserData(sal_uInt16 nNum) { return (SdrObjUserData*)aList.Remove(nNum);}
    void DeleteUserData(sal_uInt16 nNum) { delete RemoveUserData(nNum); }
};

//************************************************************
//   Hilfsklasse SdrObjGeoData
//
// Alle geometrischen Daten eines beliebigen Objektes zur �bergabe an's Undo/Redo
//
//************************************************************

class SVX_DLLPUBLIC SdrObjGeoData
{
public:
    Rectangle                   aBoundRect;
    Point                       aAnchor;
    SdrGluePointList*           pGPL;
    sal_Bool                        bMovProt;
    sal_Bool                        bSizProt;
    sal_Bool                        bNoPrint;
    sal_Bool                        bClosedObj;
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

public:
    SfxBroadcaster*             pBroadcast;    // Broadcaster, falls dieses Obj referenziert wird (bVirtObj=sal_True). Auch fuer Konnektoren etc.
    SdrObjUserDataList*         pUserDataList; // applikationsspeziefische Daten
    SdrGluePointList*           pGluePoints;   // Klebepunkte zum Ankleben von Objektverbindern
    AutoTimer*                  pAutoTimer;

    // #i68101#
    // object name, title and description
    String                      aObjName;
    String                      aObjTitle;
    String                      aObjDescription;

    // Name to be used by applications
    XubString                   aHTMLName;

public:
    TYPEINFO();
    SdrObjPlusData();
    virtual ~SdrObjPlusData();
    virtual SdrObjPlusData* Clone(SdrObject* pObj1) const;
};

//************************************************************
//   Hilfsklasse SdrObjTransformInfoRec
//
// gibt Auskunft ueber verschiedene Eigenschaften eines ZObjects
//
//************************************************************

class SdrObjTransformInfoRec
{
public:
    unsigned                    bSelectAllowed : 1;     // sal_False=Obj kann nicht selektiert werden
    unsigned                    bMoveAllowed : 1;       // sal_False=Obj kann nicht verschoben werden
    unsigned                    bResizeFreeAllowed : 1; // sal_False=Obj kann nicht frei resized werden
    unsigned                    bResizePropAllowed : 1; // sal_False=Obj kann nichtmal proportional resized werden
    unsigned                    bRotateFreeAllowed : 1; // sal_False=Obj kann nicht frei gedreht werden
    unsigned                    bRotate90Allowed : 1;   // sal_False=Obj kann nichtmal im 90deg Raster gedreht werden
    unsigned                    bMirrorFreeAllowed : 1; // sal_False=Obj kann nicht frei gespiegelt werden
    unsigned                    bMirror45Allowed : 1;   // sal_False=Obj kann nichtmal ueber Achse im 45deg Raster gespiegelt werden
    unsigned                    bMirror90Allowed : 1;   // sal_False=Obj kann ebenfalls nicht ueber Achse im 90deg Raster gespiegelt werden
    unsigned                    bTransparenceAllowed : 1; // sal_False=Obj does not have an interactive transparence control
    unsigned                    bGradientAllowed : 1; // sal_False=Obj dooes not have an interactive gradient control
    unsigned                    bShearAllowed : 1;      // sal_False=Obj kann nicht verzerrt werden
    unsigned                    bEdgeRadiusAllowed : 1;
    unsigned                    bNoOrthoDesired : 1;    // sal_True bei Rect; ... sal_False bei BMP,MTF;
    unsigned                    bNoContortion : 1;      // sal_False=Kein verzerren (bei Crook) moeglich (nur sal_True bei PathObj und Gruppierten PathObjs)
    unsigned                    bCanConvToPath : 1;     // sal_False=Keine Konvertierung in PathObj moeglich
    unsigned                    bCanConvToPoly : 1;     // sal_False=Keine Konvertierung in PolyObj moeglich
    unsigned                    bCanConvToContour : 1;     // sal_False=no conversion down to whole contour possible
    unsigned                    bCanConvToPathLineToArea : 1; // sal_False=Keine Konvertierung in PathObj moeglich mit Wandlung von LineToArea
    unsigned                    bCanConvToPolyLineToArea : 1; // sal_False=Keine Konvertierung in PolyObj moeglich mit Wandlung von LineToArea

public:
    SdrObjTransformInfoRec()
    :   bSelectAllowed(sal_True),
        bMoveAllowed(sal_True),
        bResizeFreeAllowed(sal_True),
        bResizePropAllowed(sal_True),
        bRotateFreeAllowed(sal_True),
        bRotate90Allowed(sal_True),
        bMirrorFreeAllowed(sal_True),
        bMirror45Allowed(sal_True),
        bMirror90Allowed(sal_True),
        bTransparenceAllowed(sal_True),
        bGradientAllowed(sal_True),
        bShearAllowed(sal_True),
        bEdgeRadiusAllowed(sal_True),
        bNoOrthoDesired(sal_True),
        bNoContortion(sal_True),
        bCanConvToPath(sal_True),
        bCanConvToPoly(sal_True),
        bCanConvToContour(sal_False),
        bCanConvToPathLineToArea(sal_True),
        bCanConvToPolyLineToArea(sal_True)
    {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// To make things more safe, allow users of the object to register at it. The users need to be derived
// from SdrObjectUser to get a call. The users do not need to call RemoveObjectUser() at the object
// when they get called from PageInDestruction().
class SdrObject;

class SdrObjectUser
{
public:
    virtual void ObjectInDestruction(const SdrObject& rObject) = 0;
};

// typedef for GetParentContacts()
typedef ::std::vector< SdrObjectUser* > ObjectUserVector;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@ @@@@@  @@@@  @@@@@@
//  @@  @@ @@  @@     @@ @@    @@  @@   @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@@@@      @@ @@@@  @@       @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@  @@ @@  @@ @@    @@  @@   @@
//   @@@@  @@@@@   @@@@  @@@@@  @@@@    @@
//
// Abstraktes DrawObject
//
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
    unsigned                    bVirtObj : 1;
    unsigned                    bSnapRectDirty : 1;
    unsigned                    bNetLock : 1;   // ni
    unsigned                    bInserted : 1;  // nur wenn sal_True gibt's RepaintBroadcast & SetModify
    unsigned                    bGrouped : 1;   // Member eines GroupObjektes?

    // Die folgende Flags werden gestreamt
    unsigned                    bMovProt : 1; // If true, the position is protected
    unsigned                    bSizProt : 1; // If true, the size is protected
    unsigned                    bNoPrint : 1; // If true, the object is not printed.
    unsigned                    mbVisible : 1; // If false, the object is not visible on screen (but maybe on printer, depending on bNoprint
    // Wenn bEmptyPresObj sal_True ist, handelt es sich um ein
    // Praesentationsobjekt, dem noch kein Inhalt zugewiesen
    // wurde. Default ist das Flag auf FALSE. Die Verwaltung
    // uebernimmt die Applikation. Im Zuweisungsoperator sowie
    // beim Clone wird das Flag nicht mitkopiert!
    // Das Flag ist persistent.
    unsigned                    bEmptyPresObj : 1;     // Leeres Praesentationsobjekt (Draw)

    // sal_True=Objekt ist als Objekt der MasterPage nicht sichtbar
    unsigned                    bNotVisibleAsMaster : 1;

    // sal_True=Es handelt sich hierbei um ein geschlossenes Objekt, also nicht Linie oder Kreisbogen ...
    unsigned                    bClosedObj : 1;

    unsigned                    bIsEdge : 1;
    unsigned                    bIs3DObj : 1;
    unsigned                    bMarkProt : 1;  // Markieren verboten. Persistent
    unsigned                    bIsUnoObj : 1;
    unsigned                    bNotMasterCachable : 1;

    // #i25616#
    unsigned                    mbLineIsOutsideGeometry : 1;

    // #i25616#
    unsigned                    mbSupportTextIndentingOnLineWidthChange : 1;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // in this case the following member is initialized as nonempty rectangle
    Rectangle                   maBLIPSizeRectangle;

    // global static ItemPool for not-yet-insetred items
private:
    static SdrItemPool*         mpGlobalItemPool;
public:
    static SdrItemPool& GetGlobalDrawObjectItemPool();
private:
    SVX_DLLPRIVATE static void FreeGlobalDrawObjectItemPool();

protected:
    void ImpDeleteUserData();
    SdrObjUserData* ImpGetMacroUserData() const;
    Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;

    // Fuer GetDragComment
    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, String& rStr, sal_uInt16 nVal=0) const;

    void ImpForcePlusData() { if (pPlusData==NULL) pPlusData=NewPlusData(); }

    String GetWinkStr(long nWink, bool bNoDegChar = false) const;
    String GetMetrStr(long nVal, MapUnit eWantMap=MAP_MM, bool bNoUnitChars = false) const;

    // bNotMyself=sal_True bedeutet: Nur die ObjList auf Dirty setzen, nicht mich.
    // Wird z.B. benoetigt fuer NbcMove, denn da movt man SnapRect und aOutRect
    // i.d.R. gleich mit um die Neuberechnung zu sparen.
public:
    virtual void SetRectsDirty(sal_Bool bNotMyself = sal_False);
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

    /** fres the SdrObject pointed to by the argument

        In case the object has an SvxShape, which has the ownership of the object, it
        is actually *not* deleted.
    */
    static  void    Free( SdrObject*& _rpObject );

    // This method is only for access from Property objects
    virtual void SetBoundRectDirty();

    virtual void SetObjList(SdrObjList* pNewObjList);
    SdrObjList* GetObjList() const { return pObjList; }

    virtual void SetPage(SdrPage* pNewPage);
    SdrPage* GetPage() const { return pPage; }

    virtual void SetModel(SdrModel* pNewModel);
    SdrModel* GetModel() const { return pModel; }
    SdrItemPool* GetObjectItemPool() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);
    const SfxBroadcaster* GetBroadcaster() const { return pPlusData!=NULL ? pPlusData->pBroadcast : NULL; }

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
    void SetUserCall(SdrObjUserCall* pUser) { pUserCall=pUser; }
    SdrObjUserCall* GetUserCall() const { return pUserCall; }
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
    void SetName(const String& rStr);
    String GetName() const;
    void SetTitle(const String& rStr);
    String GetTitle() const;
    void SetDescription(const String& rStr);
    String GetDescription() const;

    // support for HTMLName
    void SetHTMLName(const String& rStr);
    String GetHTMLName() const;

    // Fuer Gruppenobjekte
    sal_Bool IsGroupObject() const { return GetSubList()!=NULL; }
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
    sal_uInt32 GetOrdNumDirect() const { return nOrdNum; }

    // Das Setzen der Ordnungsnummer sollte nur vom Model bzw. von der Page
    // geschehen.
    void SetOrdNum(sal_uInt32 nNum) { nOrdNum=nNum; }

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

    const AutoTimer* GetAutoTimer() const { return pPlusData!=NULL ? pPlusData->pAutoTimer : NULL; }
    AutoTimer* GetAutoTimer() { return pPlusData!=NULL ? pPlusData->pAutoTimer : NULL; }
    AutoTimer* ForceAutoTimer();

    // #111111#
    // To make clearer that this method may trigger RecalcBoundRect and thus may be
    // expensive and somtimes problematic (inside a bigger object change You will get
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
    sal_Bool SingleObjectPainter(OutputDevice& rOut) const;
    sal_Bool LineGeometryUsageIsNecessary() const;

    // Clone() soll eine komplette Kopie des Objektes erzeugen.
    virtual SdrObject* Clone() const;
    virtual void operator=(const SdrObject& rObj);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

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
    // Ein Objekt, das bei HasSpacialDrag() sal_True liefert muss diese Methoden
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
    // Objekt dann uebergeben. EndDrag liefrt im Normalfall sal_True fuer Erfolg.
    // sal_False kann zurueckgegeben werden, wenn das Dragging das Objekt nicht
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
    // Liefert EndCreate() ein sal_True, so ist die Objekterzeugung abgeschlossen;
    // das Objekt wird in die entsprechende Liste eingefuegt. Andernfalls
    // (EndCreate()==sal_False) gehe ich davon aus, dass weitere Punkte zur
    // Objekterzeugung notwendig sind (Polygon,...). Der Parameter eCmd
    // enthaelt die Anzahl der Mausklicks (so die App diese durchreicht).
    // BckCreate() -> Letztes EndCreate() rueckgaengig machen (z.B. letzten
    // Polygonpunkt wieder loeschen).
    // RetrunCode: sal_True=Weiter gehts, sal_False=Create dadurch abgebrochen.
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
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear (const Point& rRef, long nWink, double tn, bool bVShear);

    // Die relative Position eines Zeichenobjektes ist die Entfernung der
    // linken oberen Eche des logisch umschliessenden Rechtecks (SnapRect)
    // zum Anker.
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual Point GetRelativePos() const;
    void ImpSetAnchorPos(const Point& rPnt) { aAnchor=rPnt; }
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
    void SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems = sal_False);
    const SfxPoolItem& GetMergedItem(const sal_uInt16 nWhich) const;

    // internal versions
protected:
    const SfxItemSet& GetObjectItemSet() const;
    void SetObjectItem(const SfxPoolItem& rItem);
    void ClearObjectItem(const sal_uInt16 nWhich = 0);
    void SetObjectItemSet(const SfxItemSet& rSet);
    const SfxPoolItem& GetObjectItem(const sal_uInt16 nWhich) const;

public:
    // syntactical sugar for ItemSet accesses
    void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems = sal_False);

    // NotPersistAttr fuer Layer, ObjName, geometrische Transformationen, ...
    void TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);
    void NbcApplyNotPersistAttr(const SfxItemSet& rAttr);

    // bDontRemoveHardAttr=FALSE: alle in der Vorlage gesetzten Attribute werden am
    // Zeichenobjekt auf Default gesetzt; TRUE: alle harten Attribute bleiben erhalten.
    void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);
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
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;
    sal_Bool IsMacroHit(const SdrObjMacroHitRec& rRec) const { return CheckMacroHit(rRec)!=NULL; }

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
    // Im Modus bLineToArea=sal_False soll ein PolyObj mit 4 Stuetzstellen,
    // Strichstaerke 10 und SOLiD_PEN erzeugt werden.
    // Im Modus bLineToArea=sal_True dagegen soll das generierte Obj immer ein
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
    // dass beide Modi (bLineToArea=sal_True/sal_False) identisch sind.
    // Defaulted sind diese Methoden auf "Ich kann das nicht" (FALSE/NULL).
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;
    SdrObject* ConvertToPolyObj(sal_Bool bBezier, sal_Bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, sal_Bool bForceLineDash = sal_False) const;
    SdrObject* ImpConvertToContourObj(SdrObject* pRet, sal_Bool bForceLineDash = sal_False) const;

    // TRUE: Referenz auf ein Obj
    sal_Bool IsVirtualObj() const { return bVirtObj; }

    // sal_True=Obj kann warsch. gefuellt werden; sal_False=Obj kann warsch. Linienenden haben.
    // ungueltig, wenn es sich um ein GroupObj handelt.
    sal_Bool IsClosedObj() const { return bClosedObj; }

    sal_Bool IsEdgeObj() const { return bIsEdge; }
    sal_Bool Is3DObj() const { return bIs3DObj; }
    sal_Bool IsUnoObj() const { return bIsUnoObj; }
    sal_Bool IsMasterCachable() const { return !bNotMasterCachable; }
    sal_Bool ShareLock() { sal_Bool r=!bNetLock; bNetLock=sal_True; return r; }
    void ShareUnlock() { bNetLock=sal_False; }
    sal_Bool IsShareLock() const { return bNetLock; }
    void SetMarkProtect(sal_Bool bProt) { bMarkProt=bProt; }
    sal_Bool IsMarkProtect() const { return bMarkProt; }
    void SetInserted(sal_Bool bIns);
    sal_Bool IsInserted() const { return bInserted; }
    void SetGrouped(sal_Bool bGrp) { bGrouped=bGrp; }
    sal_Bool IsGrouped() const { return bGrouped; }
    void SetMoveProtect(sal_Bool bProt);
    sal_Bool IsMoveProtect() const { return bMovProt; }
    void SetResizeProtect(sal_Bool bProt);
    sal_Bool IsResizeProtect() const { return bSizProt; }
    void SetPrintable(sal_Bool bPrn);
    sal_Bool IsPrintable() const { return !bNoPrint; }
    void SetVisible(sal_Bool bVisible);
    sal_Bool IsVisible() const { return mbVisible; }
    void SetEmptyPresObj(sal_Bool bEpt) { bEmptyPresObj=bEpt; }
    sal_Bool IsEmptyPresObj() const { return bEmptyPresObj; }
    void SetNotVisibleAsMaster(sal_Bool bFlg) { bNotVisibleAsMaster=bFlg; }
    sal_Bool IsNotVisibleAsMaster() const { return bNotVisibleAsMaster; }

    // #i25616#
    sal_Bool LineIsOutsideGeometry() const { return mbLineIsOutsideGeometry; }

    // #i25616#
    sal_Bool DoesSupportTextIndentingOnLineWidthChange() const { return mbSupportTextIndentingOnLineWidthChange; }

    // applikationsspeziefische Daten
    sal_uInt16 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt16 nNum) const;

    // Insert uebernimmt den auf dem Heap angelegten Record in den Besitz
    // des Zeichenobjekts
    void InsertUserData(SdrObjUserData* pData, sal_uInt16 nPos=0xFFFF);

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

    // helper struct for granting access exclusive to SvxShape
    struct GrantXShapeAccess
    {
        friend class SvxShape;
    private:
        GrantXShapeAccess() { }
    };

    // setting the UNO representation is allowed for the UNO representation itself only!
    void setUnoShape(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape,
            GrantXShapeAccess /*aGrant*/
        )
    {
        impl_setUnoShape( _rxUnoShape );
    }

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

    sal_Bool IsTransparent( sal_Bool bCheckForAlphaChannel = sal_False ) const;

    // #116168#
    // Give info if object is in destruction
    sal_Bool IsInDestruction() const;

    // return if fill is != XFILL_NONE
    bool HasFillStyle() const;
    bool HasLineStyle() const;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // the following methods are used to control it;
    // usually this data makes no sence after the import is finished, since the object
    // might be resized
    Rectangle GetBLIPSizeRectangle() const;
    void SetBLIPSizeRectangle( const Rectangle& aRect );

protected:
    void    impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape );

private:
    /** only for internal use!
    */
    SvxShape* getSvxShape() const;

    /** do not use directly, always use getSvxShape() if you have to! */
    SvxShape*   mpSvxShape;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >
                maWeakUnoShape;
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
    SVX_DLLPRIVATE SdrObjFactory(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrObject* pObj1);

public:
    static SdrObject* MakeNewObject(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pPage, SdrModel* pModel=NULL);
    static void InsertMakeObjectHdl(const Link& rLink);
    static void RemoveMakeObjectHdl(const Link& rLink);
    static SdrObjUserData* MakeNewObjUserData(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrObject* pObj);
    static void InsertMakeUserDataHdl(const Link& rLink);
    static void RemoveMakeUserDataHdl(const Link& rLink);
};

typedef tools::WeakReference< SdrObject > SdrObjectWeakRef;

#endif //_SVDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
