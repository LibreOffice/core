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

#include <bf_svtools/lstner.hxx>

#include <vcl/timer.hxx>

#include <bf_svx/svdsob.hxx>

#include <bf_svx/svdtypes.hxx> // fuer SdrLayerID

#include <bf_svx/svdglue.hxx> // Klebepunkte

#include <bf_svx/xdash.hxx>

#include <bf_svx/xpoly.hxx>

#include <bf_svx/poly3d.hxx>

#include <bf_svx/xenum.hxx>

#include <tools/color.hxx>

class Pointer;
class AutoTimer;
class PolyPolygon;
namespace binfilter {

class SfxBroadcaster;
class SfxItemSet;
class SfxSetItem;
class SfxStyleSheet;
class SfxUndoAction;
class SfxItemPool;
class SfxPoolItem;

//************************************************************
//   Vorausdeklarationen
//************************************************************

class ExtOutputDevice;
//class ImpSdrMtfAnimator;
class OutlinerParaObject;
class SdrOutliner;
class SdrDragStat;
class SdrHdl;
class SdrHdlList;
class SdrItemPool;
class SdrMiscSetItem;
class SdrModel;
class SdrObjIOHeader;
class SdrObjList;
class SdrObject;
class SdrOutlinerSetItem;
class SdrPage;
class SdrPageView;
class SdrShadowSetItem;
class SdrView;
class SdrVirtObj;
class XFillAttrSetItem;
class XLineAttrSetItem;
class XPolyPolygon;
class XPolygon;
class XTextAttrSetItem;
class SdrLineGeometry;

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
                 OBJ_MAXI};

// Paintmodes, die den SdrObject::Paint-Methoden mitgegeben werden.
#define SDRPAINTMODE_MASTERPAGE		0x0001 /* Obj gehoert zur eingeblendeten Masterpage */
#define SDRPAINTMODE_TEXTEDIT		0x0002 /* An diesem Objekt ist z.Zt. TextEdit aktiv */
#define SDRPAINTMODE_DRAFTTEXT		0x0004 /* Entwurfsmodus, Text weglassen bzw. nur andeuten (Ersatzdarstellung) */
#define SDRPAINTMODE_DRAFTGRAF		0x0008 /* Entwurfsmodus, Grafiken nur andeuten (Ersatzdarstellung) */
#define SDRPAINTMODE_GLUEPOINTS		0x0010 /* Klebepunkte anzeigen */
#define SDRPAINTMODE_ANILIKEPRN		0x0020 /* Animationen so malen, als ob gedruckt wird (z.B. fuer Laufschrift im SdrPageObj) */
#define SDRPAINTMODE_HIDEDRAFTGRAF  0x0040 /* Entwurfsmodus, angedeutete Grafiken nicht darstellen */
#define SDRPAINTMODE_DRAFTLINE		0x0080 /* draftmode for LINE */
#define SDRPAINTMODE_DRAFTFILL		0x0100 /* draftmode for FILL */
                                       /* ... to be continued */
enum SdrUserCallType {SDRUSERCALL_MOVEONLY,       	// Nur verschoben, Groesse unveraendert
                      SDRUSERCALL_RESIZE,         	// Groesse und evtl. auch Pos veraendert
                      SDRUSERCALL_CHGATTR,  		// Attribute veraendert. Moeglicherweise neue Groesse wg. Linienbreite
                      SDRUSERCALL_DELETE,   		// Obj gibt es gleich nicht mehr. Schon keine Attr mehr.
                      SDRUSERCALL_COPY,     		// Zuweisungsoperator gerufen. Kann alles geaendert sein
                      SDRUSERCALL_INSERTED, 		// In eine Objektliste (z.B. Page) eingefuegt
                      SDRUSERCALL_REMOVED,  		// Aus der Objektliste entfernt
                      SDRUSERCALL_CHILD_MOVEONLY, 	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_RESIZE,   	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_CHGATTR,  	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_DELETE,   	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_COPY,     	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_INSERTED, 	// Ein Child einer Gruppe hat sich veraendert
                      SDRUSERCALL_CHILD_REMOVED}; 	// Ein Child einer Gruppe hat sich veraendert

/* nur voruebergehend, weil sonst MUSS-Aenderung */
#define nLayerID nLayerId

//************************************************************
//   Hilfsklasse SdrObjUserCall
//************************************************************

class SdrObjUserCall
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
    Point						aPos;
    Point						aDownPos;
    OutputDevice*				pOut;
    const SetOfByte*			pVisiLayer;
    const SdrPageView*			pPageView;
    USHORT						nTol;
    FASTBOOL					bDown;

public:
    SdrObjMacroHitRec()
    :	pOut(NULL),
        pVisiLayer(NULL),
        pPageView(NULL),
        nTol(0),
        bDown(FALSE)
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

class SdrObjUserData
{
protected:
    UINT32						nInventor;
    UINT16						nIdentifier;
    UINT16						nVersion;

private:
    void operator=(const SdrObjUserData& rData);        // nicht implementiert
    FASTBOOL operator==(const SdrObjUserData& rData) const; // nicht implementiert
    FASTBOOL operator!=(const SdrObjUserData& rData) const; // nicht implementiert

public:
    TYPEINFO();

    SdrObjUserData(UINT32 nInv, UINT16 nId, UINT16 nVer)
    :	nInventor(nInv),
        nIdentifier(nId),
        nVersion(nVer)
    {}
    SdrObjUserData(const SdrObjUserData& rData)
    :	nInventor(rData.nInventor),
        nIdentifier(rData.nIdentifier),
        nVersion(rData.nVersion)
    {}
    virtual ~SdrObjUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    UINT32  GetInventor() const { return nInventor; }
    UINT16  GetId() const { return nIdentifier; }
    virtual void WriteData(SvStream& rOut);
    virtual void ReadData(SvStream& rIn);

    // z.B. fuer die Wiederherstellung von Surrogaten.
    // Siehe auch SdrObject::AfterRead().
    virtual void AfterRead();

    virtual FASTBOOL HasMacro (const SdrObject* pObj) const;
};

//************************************************************
//   Hilfsklasse SdrObjUserDataList
//************************************************************

class SdrObjUserDataList
{
    Container					aList;

public:
    SdrObjUserDataList()
    :	aList(1024,4,4)
    {}
    ~SdrObjUserDataList() { Clear(); }

    void Clear();
    USHORT GetUserDataCount() const { return USHORT(aList.Count()); }
    SdrObjUserData* GetUserData(USHORT nNum) const { return (SdrObjUserData*)aList.GetObject(nNum); }
    void InsertUserData(SdrObjUserData* pData, USHORT nPos=0xFFFF) { aList.Insert(pData,nPos); }
    SdrObjUserData* RemoveUserData(USHORT nNum) { return (SdrObjUserData*)aList.Remove(nNum);}
    void DeleteUserData(USHORT nNum) { delete RemoveUserData(nNum); }
};

//************************************************************
//   Hilfsklasse SdrObjGeoData
//
// Alle geometrischen Daten eines beliebigen Objektes zur Übergabe an's Undo/Redo
//
//************************************************************

class SdrObjGeoData
{
public:
    Rectangle					aBoundRect;
    Point						aAnchor;
    SdrGluePointList*			pGPL;
    BOOL						bMovProt;
    BOOL						bSizProt;
    BOOL						bNoPrint;
    BOOL						bClosedObj;
    UINT16						nLayerId;

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
    friend class				SdrObject;

public:
    SfxBroadcaster*				pBroadcast;    // Broadcaster, falls dieses Obj referenziert wird (bVirtObj=TRUE). Auch fuer Konnektoren etc.
    SdrObjUserDataList*			pUserDataList; // applikationsspeziefische Daten
    SdrGluePointList*			pGluePoints;   // Klebepunkte zum Ankleben von Objektverbindern
//	ImpSdrMtfAnimator*			pAnimator;     // Fuer Laufschrift und blinkenden Text
    AutoTimer*					pAutoTimer;
    XubString					aObjName;      // Jedes Objekt soll nun einen Namen haben

    // Name to be used by applications
    XubString					aHTMLName;

public:
    TYPEINFO();
    SdrObjPlusData();
    virtual ~SdrObjPlusData();
    // Keine Streamoperatoren. Jede Obj streamt seinen Teil in Obj::WriteData().
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
    unsigned					bSelectAllowed : 1;     // FALSE=Obj kann nicht selektiert werden
    unsigned					bMoveAllowed : 1;       // FALSE=Obj kann nicht verschoben werden
    unsigned					bResizeFreeAllowed : 1; // FALSE=Obj kann nicht frei resized werden
    unsigned					bResizePropAllowed : 1; // FALSE=Obj kann nichtmal proportional resized werden
    unsigned					bRotateFreeAllowed : 1; // FALSE=Obj kann nicht frei gedreht werden
    unsigned					bRotate90Allowed : 1;   // FALSE=Obj kann nichtmal im 90deg Raster gedreht werden
    unsigned					bMirrorFreeAllowed : 1; // FALSE=Obj kann nicht frei gespiegelt werden
    unsigned					bMirror45Allowed : 1;   // FALSE=Obj kann nichtmal ueber Achse im 45deg Raster gespiegelt werden
    unsigned					bMirror90Allowed : 1;   // FALSE=Obj kann ebenfalls nicht ueber Achse im 90deg Raster gespiegelt werden
    unsigned					bTransparenceAllowed : 1; // FALSE=Obj does not have an interactive transparence control
    unsigned					bGradientAllowed : 1; // FALSE=Obj dooes not have an interactive gradient control
    unsigned					bShearAllowed : 1;      // FALSE=Obj kann nicht verzerrt werden
    unsigned					bEdgeRadiusAllowed : 1;
    unsigned					bNoOrthoDesired : 1;    // TRUE bei Rect; ... FALSE bei BMP,MTF;
    unsigned					bNoContortion : 1;      // FALSE=Kein verzerren (bei Crook) moeglich (nur TRUE bei PathObj und Gruppierten PathObjs)
    unsigned					bCanConvToPath : 1;     // FALSE=Keine Konvertierung in PathObj moeglich
    unsigned					bCanConvToPoly : 1;     // FALSE=Keine Konvertierung in PolyObj moeglich
    unsigned					bCanConvToContour : 1;     // FALSE=no conversion down to whole contour possible
    unsigned					bCanConvToPathLineToArea : 1; // FALSE=Keine Konvertierung in PathObj moeglich mit Wandlung von LineToArea
    unsigned					bCanConvToPolyLineToArea : 1; // FALSE=Keine Konvertierung in PolyObj moeglich mit Wandlung von LineToArea

public:
    SdrObjTransformInfoRec()
    :	bSelectAllowed(TRUE),
        bMoveAllowed(TRUE),
        bResizeFreeAllowed(TRUE),
        bResizePropAllowed(TRUE),
        bRotateFreeAllowed(TRUE),
        bRotate90Allowed(TRUE),
        bMirrorFreeAllowed(TRUE),
        bMirror45Allowed(TRUE),
        bMirror90Allowed(TRUE),
        bTransparenceAllowed(TRUE),
        bGradientAllowed(TRUE),
        bShearAllowed(TRUE),
        bEdgeRadiusAllowed(TRUE),
        bNoOrthoDesired(TRUE),
        bNoContortion(TRUE),
        bCanConvToPath(TRUE),
        bCanConvToPoly(TRUE),
        bCanConvToContour(FALSE),
        bCanConvToPathLineToArea(TRUE),
        bCanConvToPolyLineToArea(TRUE)
    {}
};

//************************************************************
//   Hilfsklasse SdrObjTransformInfoRec
//************************************************************

class SdrPaintInfoRec
{
public:
    SetOfByte					aPaintLayer;
    Rectangle					aDirtyRect;
    Rectangle					aCheckRect;
    const SdrPageView*			pPV;  // Die PageView wird u.a. vom TextObj benoetigt fuer Paint wenn TextEdit
    const Link*					pPaintProc;
    const SdrObjList*			pAktList; // falls GroupEntered oder GluePoints nur auf flacher Ebene (ni)
    USHORT						nBrkEvent;
    USHORT						nPaintMode;
    UINT32						nOriginalDrawMode;

    BOOL						bPrinter;
    BOOL						bNotActive;
    BOOL						bOriginalDrawModeSet;

public:
    SdrPaintInfoRec()
    :	aPaintLayer(TRUE),
        pPV(NULL),
        pPaintProc(NULL),
        pAktList(NULL),
        nBrkEvent(0),
        nPaintMode(0),
        nOriginalDrawMode(0L),
        bPrinter(FALSE),
        bNotActive(TRUE),
        bOriginalDrawModeSet(FALSE)
    {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions

class SdrBroadcastItemChange
{
    sal_uInt32					mnCount;
    void*						mpData;

public:
    SdrBroadcastItemChange(const SdrObject& rObj);
    ~SdrBroadcastItemChange();

    sal_uInt32 GetRectangleCount() const { return mnCount; }
    const Rectangle& GetRectangle(sal_uInt32 nIndex) const;
};

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

class SdrObject: public SfxListener
{
    friend class				SdrObjListIter;
    friend class				SdrVirtObj;
    friend class				SdrRectObj;

    static SfxItemSet*			mpEmptyItemSet;

protected:
    Rectangle					aOutRect;     // umschliessendes Rechteck fuer Paint (inkl. LineWdt, ...)
    Point						aAnchor;      // Ankerposition (Writer)
    SdrObjList*					pObjList;     // Liste, in dem das Obj eingefuegt ist.
    SdrPage*					pPage;
    SdrModel*					pModel;
    SdrObjUserCall*				pUserCall;
    SdrObjPlusData*				pPlusData;    // Broadcaster, UserData, Konnektoren, ... (Das ist der Bitsack)

    UINT32						nOrdNum;      // Rangnummer des Obj in der Liste
    UINT16						nLayerId;

    // Objekt zeigt nur auf ein Anderes
    BOOL						bVirtObj : 1;
    BOOL						bBoundRectDirty : 1;
    BOOL						bSnapRectDirty : 1;
    BOOL						bNetLock : 1;   // ni
    BOOL						bInserted : 1;  // nur wenn TRUE gibt's RepaintBroadcast & SetModify
    BOOL						bGrouped : 1;   // Member eines GroupObjektes?

    // Die folgende Flags werden gestreamt
    BOOL						bMovProt : 1; // Position geschuetzt
    BOOL						bSizProt : 1; // Groesse geschuetzt
    BOOL						bNoPrint : 1; // Nicht drucken

    // Wenn bEmptyPresObj TRUE ist, handelt es sich um ein
    // Praesentationsobjekt, dem noch kein Inhalt zugewiesen
    // wurde. Default ist das Flag auf FALSE. Die Verwaltung
    // uebernimmt die Applikation. Im Zuweisungsoperator sowie
    // beim Clone wird das Flag nicht mitkopiert!
    // Das Flag ist persistent.
    BOOL						bEmptyPresObj : 1;     // Leeres Praesentationsobjekt (Draw)
    BOOL						bNotPersistent : 1;    // TRUE= Obj wird nicht gestreamt (Writer)
    BOOL						bNeedColorRestore : 1; // TRUE= Obj benoetigt ColorRestore vor Paint-Aufruf (Writer)

    // TRUE=Objekt ist als Objekt der MasterPage nicht sichtbar
    BOOL						bNotVisibleAsMaster : 1;

    // TRUE=Es handelt sich hierbei um ein geschlossenes Objekt, also nicht Linie oder Kreisbogen ...
    BOOL						bClosedObj : 1;
    BOOL						bWriterFlyFrame : 1;   // Special fuer den Writer
    BOOL						bIsEdge : 1;
    BOOL						bIs3DObj : 1;
    BOOL						bMarkProt : 1;  // Markieren verboten. Persistent
    BOOL						bIsUnoObj : 1;
    BOOL						bNotMasterCachable : 1;

    // global static ItemPool for not-yet-insetred items
private:
    static SdrItemPool*			mpGlobalItemPool;
public:
    static SdrItemPool* GetGlobalDrawObjectItemPool();
private:

protected:
    void ImpDeleteUserData();
    SdrObjUserData* ImpGetMacroUserData() const;

    // Fuer GetDragComment

    void ImpForcePlusData() { if (pPlusData==NULL) pPlusData=NewPlusData(); }
//	ImpSdrMtfAnimator* ImpGetMtfAnimator() const { return pPlusData!=NULL ? pPlusData->pAnimator : NULL; }


    // bNotMyself=TRUE bedeutet: Nur die ObjList auf Dirty setzen, nicht mich.
    // Wird z.B. benoetigt fuer NbcMove, denn da movt man SnapRect und aOutRect
    // i.d.R. gleich mit um die Neuberechnung zu sparen.
    virtual void SetRectsDirty(FASTBOOL bNotMyself=FALSE);

    // ueberladen, wenn man sich von SdrObjPlusData abgeleitet hat:
    virtual SdrObjPlusData* NewPlusData() const;

    // this is a weak reference to a possible living api wrapper for this shape
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoShape;

protected:
    // Diese 3 Methoden muss ein abgeleitetes Objekt ueberladen, wenn es eigene
    // geometrische Daten besitzt, die fuer den Undo-Fall gesichert werden
    // sollen. NewGeoData() erzeugt lediglich eine leere Instanz auf eine von
    // SdrObjGeoData abgeleitete Klasse.
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    TYPEINFO();
    SdrObject();
    virtual ~SdrObject();

    virtual void SetObjList(SdrObjList* pNewObjList);
    SdrObjList* GetObjList() const { return pObjList; }

    virtual void SetPage(SdrPage* pNewPage);
    SdrPage* GetPage() const { return pPage; }

    virtual void SetModel(SdrModel* pNewModel);
    SdrModel* GetModel() const { return pModel; }
    SdrItemPool* GetItemPool() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);
    const SfxBroadcaster* GetBroadcaster() const { return pPlusData!=NULL ? pPlusData->pBroadcast : NULL; }

    virtual void AddReference(SdrVirtObj& rVrtObj);
    virtual void DelReference(SdrVirtObj& rVrtObj);
    virtual UINT32 GetObjInventor() const;
    virtual UINT16 GetObjIdentifier() const;
    virtual SdrLayerID GetLayer() const;

    // Layer muss vorher leer sein!
    virtual void   GetLayer(SetOfByte& rSet) const;
    virtual void   NbcSetLayer(SdrLayerID nLayer);
    virtual void   SetLayer(SdrLayerID nLayer);
    void SetUserCall(SdrObjUserCall* pUser) { pUserCall=pUser; }
    SdrObjUserCall* GetUserCall() const { return pUserCall; }
    void SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect);

    // Ein solcher Referenzpunkt ist z.B. der Punkt eines Symbols, der
    // der beim Einfuegen des Symbols auf das Raster gefangen werden soll
    // oder der Fixpunkt eines Bildes innerhalb eines Animationsobjektes.

    // Ein Objekt kann auch einen individuellen (vom Anwender zu vergebenden)
    // Namen tragen (z.B. Symbol oder Gruppenobjekt, OLE2, ... )
    virtual void SetName(const String& rStr);
    virtual String GetName() const;

    // support for HTMLName

    // Fuer Gruppenobjekte
    FASTBOOL IsGroupObject() const { return GetSubList()!=NULL; }
    virtual SdrObjList* GetSubList() const;
    SdrObject* GetUpGroup() const;

    // Ueber die Objekt-Ordnungsnummer kann man feststellen, ob ein Objekt vor
    // oder hinter einem anderen liegt. Objekte mit kleinen Ordnungsnummern werden
    // zuerst gezeichnet, Objekte mit grossen Ordnungsnummern zuletzt.
    // Wird die Reihenfolge der Objekte in der Liste veraendert, so wird ein
    // Dirty-Flag gesetzt (an der Page). Beim naechsten SdrObject::GetOrdNum()
    // werden die Ordnungsnummer aller Objekte der Liste neu bestimmt.
    UINT32 GetOrdNum() const;

    // Diese Methode sollte nur verwendet werden, wenn man ganz  genau weiss,
    // was man macht:
    UINT32 GetOrdNumDirect() const { return nOrdNum; }

    // Das Setzen der Ordnungsnummer sollte nur vom Model bzw. von der Page
    // geschehen.
    void SetOrdNum(UINT32 nNum) { nOrdNum=nNum; }

    const AutoTimer* GetAutoTimer() const { return pPlusData!=NULL ? pPlusData->pAutoTimer : NULL; }
    AutoTimer* GetAutoTimer() { return pPlusData!=NULL ? pPlusData->pAutoTimer : NULL; }

    // Das BoundRect wird benoetigt fuer:
    //    Redraw: Nur noetig, wenn Obj im sichtbaren Bereich
    //    Redraw: Bei geaenderten ObjAttributen wird dieser Bereich invalidiert
    //    Selektion (HitTest), 1. Stufe
    //    Obj nach vorn/hinten: Nur Objs im gleichen Bereich werden beruecksichtigt.
    virtual const Rectangle& GetBoundRect() const;
    virtual void RecalcBoundRect();
    void SendRepaintBroadcast(BOOL bNoPaintNeeded = FALSE) const;
    void SendRepaintBroadcast(const Rectangle& rRect) const;

    // Modified-Flag am Model setzen
    virtual void SetChanged();

    // Liefert Paint ein FALSE, so wurde das Paint durch einen Event abgebrochen.
    // nBrkEvent bestimmt, wodurch das Paint unterbrochen werden darf.
    // => pApp->AnyInput(nBrkEvent);
    // Der Parameter nPaintMode wurde Writer-speziefisch eingebaut. Beim InitRedraw
    // an der View kann ein USHORT mitgegeben werden, der dann bis hier zum Objekt
    // durchgereicht wird. rDirtyRect beschreibt den Bereich, der am OutputDevice
    // Invalidiert wurde. rDirtyRect kann groesser sein als das Objekt selbst.
    // Wird ein leeres Rectangle uebergeben, so soll stattdessen ein unendlich
    // grosses Rechteck gelten.
    virtual FASTBOOL Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const;

    /** Line geometry creation and output (used during Paint())

        @attention Not intended for use outside SVX. Therefore,
        SdrLineGeometry is opaque here.

        This method sets up some attributes and then delegates to
        CreateLinePoly().

        @param rXOut
        Output device that specifies required resolution 

        @param rSet
        Item set attributing the line style

        @param bIsLineDraft
        Set to TRUE, if fast draft mode is requested (prevents thick lines)

        @return the generated line geometry. Ownership of the pointer
        transfers to the caller.
     */
    ::std::auto_ptr< SdrLineGeometry > ImpPrepareLineGeometry(ExtOutputDevice& rXOut, const SfxItemSet& rSet, BOOL bIsLineDraft = FALSE) const;
    void ImpDrawLineGeometry(   ExtOutputDevice& 	rXOut, 
                                Color&              rColor, 
                                sal_uInt16        	nTransparence, 
                                SdrLineGeometry&    rLineGeometry,
                                sal_Int32          	nDX=0, 
                                sal_Int32          	nDY=0			) const;
    void ImpDrawColorLineGeometry(ExtOutputDevice& rXOut, const SfxItemSet& rSet, SdrLineGeometry& rLineGeometry) const;
    /** Line geometry creation and output (used during Paint())

        @attention Not intended for use outside SVX. Therefore,
        SdrLineGeometry is opaque here.

        @param rOut
        Output device that specifies required resolution 

        @param bForceOnePixel
        Force generated line geometry to be a hair line of one pixel width (in device resolution)

        @param bForceTwoPixel
        Force generated line geometry to be a hair line of two pixel
        width (in device resolution). This is achieved by outputting a
        one pixel hair line four times.

        @param bIsLineDraft
        Set to TRUE, if fast draft mode is requested (prevents thick lines)

        @return the generated line geometry. Ownership of the pointer
        transfers to the caller.
     */
    virtual ::std::auto_ptr< SdrLineGeometry > CreateLinePoly( OutputDevice& 	rOut, 
                                                               BOOL 			bForceOnePixel, 
                                                               BOOL 			bForceTwoPixel, 
                                                               BOOL 			bIsLineDraft	) const;

    // HitTest, 2. Stufe. nTol ist die zulaessige Toleranz in logischen Einheiten.
    // rVisiLayer ist hauptsaechlich fuer Gruppenobjekte gedacht, die ja Objekte
    // mit unterschiedlichen Layerzuordnungen beinhalten koennen.
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    SdrObject* CheckHit(const Point& rPnt, USHORT nTol) const { return CheckHit(rPnt,nTol,NULL); }
    FASTBOOL IsHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const { return CheckHit(rPnt,nTol,pVisiLayer)!=NULL; }
    FASTBOOL IsHit(const Point& rPnt, USHORT nTol) const { return CheckHit(rPnt,nTol,NULL)!=NULL; }

    // Clone() soll eine komplette Kopie des Objektes erzeugen.
    virtual SdrObject* Clone() const;
    virtual SdrObject* Clone(SdrPage* pPage, SdrModel* pModel) const;
    virtual void operator=(const SdrObject& rObj);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".

    // Das Xor-Polygon wird von der View zu Draggen des Objektes benoetigt.
    // Alle XPolygone innerhalb des XPolyPolygon werden als PolyLine interpretiert.
    // Moechte man ein XPolygon, so muss man es explizit schliessen.
    virtual void TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const;

    // Die Kontur fuer TextToContour
    virtual void TakeContour(XPolyPolygon& rPoly) const;
    virtual void TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const;

    // Ueber GetHdlCount gibt ein Objekt die Anzahl seiner Handles preis.
    // Im Normalfall werden dies 8 sein, bei einer Strecke 2. Bei Polygonobjekten
    // (Polygon,Spline,Bezier) kann die Handleanzahl wesentlich groesser werden.
    // Polygonobjekten wird ausserdem die Moeglichkeit eingeraeumt einen Punkt
    // eines selektierten Objekts zu selektieren. Das Handle dieses Punktes wird
    // dann durch einen Satz neuer Handles ausgetauscht (PlusHdl). Bei einem
    // Polygon wird das wohl ein einfacher Selektionshandle sein, bei einer
    // Bezierkurve dagegen koennen das schon bis zu 3 Handles werden (inkl Gewichte).
    // GetHdl() und GetPlusHdl() muessen Handleinstanzen mit new erzeugen!
    // Ein Objekt, das bei HasSpacialDrag() TRUE liefert muss diese Methoden
    // zur Verfuegung stellen (inkl. FillHdlList).

    // Die Standardtransformationen (Move,Resize,Rotate,Mirror,Shear) werden von der
    // View uebernommen (TakeXorPoly(),...).
    // Objektspeziefisches draggen wie z.B. Eckenradius bei Rechtecken,
    // Stuetzstellen bei Splines, Gewichte bei Bezierkurven und Pointer von
    // Labelobjekten muss vom Objekt selbst gehandled werden. Um das Model
    // Statusfrei zu halten werden die Statusdaten an der View gehalten und dem
    // Objekt dann uebergeben. EndDrag liefrt im Normalfall TRUE fuer Erfolg.
    // FALSE kann zurueckgegeben werden, wenn das Dragging das Objekt nicht
    // veraendert hat, wobei dir evtl. Tatsache das die Maus nicht bewegt wurde
    // bereits von der View abgefangen wird.

    // Jedes Objekt muss in der Lage sein sich selbst interaktiv zu erzeugen.
    // Beim MausDown wird zunaechst ein neues Objekt erzeugt und dann seine
    // BegCreate()-Methode gerufen. Bei jedem MausMode wird dann MovCreate
    // gerufen. BrkCreate() bedeutet, dass der User die interaktive Objekt-
    // erzeugung abgebrochen hat. EndCreate() wird gerufen beim MouseUp-Event.
    // Liefert EndCreate() ein TRUE, so ist die Objekterzeugung abgeschlossen;
    // das Objekt wird in die entsprechende Liste eingefuegt. Andernfalls
    // (EndCreate()==FALSE) gehe ich davon aus, dass weitere Punkte zur
    // Objekterzeugung notwendig sind (Polygon,...). Der Parameter eCmd
    // enthaelt die Anzahl der Mausklicks (so die App diese durchreicht).
    // BckCreate() -> Letztes EndCreate() rueckgaengig machen (z.B. letzten
    // Polygonpunkt wieder loeschen).
    // RetrunCode: TRUE=Weiter gehts, FALSE=Create dadurch abgebrochen.

    // damit holt man sich den Pointer, der das Createn dieses Objekts symbolisiert
    virtual Pointer GetCreatePointer() const;

    // Polygon das waehrend des Erzeugens aufgezogen wird

    // Die Methoden Move, Resize, Rotate, Mirror, Shear, SetSnapRect und
    // SetLogicRect rufen jeweils die entsprechenden Nbc-Methoden, versenden einen
    // Repaint-Broadcast und setzen den Modified-Status am Model. Abgeleitete
    // Objekte sollten i.d.R. nur die Nbc-Methoden ueberladen.
    // Nbc bedeutet: 'NoBroadcast'.
    virtual void NbcMove  (const Size& rSiz);
/*N*/ 	virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
/*N*/ 	virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
/*N*/ 	virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
/*N*/ 	virtual void NbcShear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    virtual void Move  (const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Shear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

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

    // Drehwinkel und Shear
    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(FASTBOOL bVertical=FALSE) const;

    // Zum Fangen von/auf ausgezeichneten Punkten eines Obj (Polygonpunkte,
    // Kreismittelpunkt, ...)

    // Fuer Objekte, bei denen jeder einzelne Punkt verschoben werden kann,
    // z.B. Polygone, Polylines, Linien, ... . Bei diesen Objekten werden
    // Punkte selektiert (ggf. Mehrfachselektion), geloescht, eingefuegt,
    // als Mehrfachselektion verschoben und gedreht, ...
    // Nur solche Objekte koennen PlusHandles haben (z.B. die Gewichte an den
    // Bezierkurven.
    virtual FASTBOOL IsPolyObj() const;
    virtual USHORT GetPointCount() const;
    virtual const Point& GetPoint(USHORT i) const;
    virtual void SetPoint(const Point& rPnt, USHORT i);
    virtual void NbcSetPoint(const Point& rPnt, USHORT i);

    // Einfuegen eines neuen Polygonpunktes. Ret muss die Nummer des passenden
    // Hdl sein. 0xFFFF= einfuegen nicht moeglich.
    // Is bHideHim=TRUE, dann ist interaktives Ins angesagt. Der Punkt ist dann
    // geographisch so zu platzieren, dass er nicht auffaellt (z.B. identische
    // Pos wie ein Nachbarpunkt).
    // bNewObj=TRUE => Ein PolyPolygonObj soll nach Moeglichkeit ein neues
    // Polygon beginnen.
    // Ueber rInsNextAfter kann ein Flag zurueckgegeben werden, ob weitere
    // Punkte vor oder hinter dem gerade Eingefuegten eingefuegt werden sollten
    // (z.B. fuer Polylines Punkte anhaengen am Anfang/Ende).
    // => Beim einfuegen neuer Punkte ueber die View wird zunaechst die Methode
    // ohne Indexparameter gerufen. Diese stellt dann fest wo der Neue hinkommt
    // und ob Folgepunkte, vor oder hinter diesem eingefuegt werden sollen.
    // Ist das einfuegen diese Punktes an der View abgeschlossen wird dort
    // automatisch das einfuegen eines weiteren Punktes gestartet. Am Obj wird
    // dafuer die InsPoint-Methode mit Indexparameter gerufen (der Index
    // entspricht genau der Handlenummer, die im letzten Aufruf zurueckgegeben
    // wurde). Der Parameter bInsAfter enthaelt genau den Wert, den der 1.
    // Aufruf (InsPoint ohne Indexparameter) zurueckgab.

    // Neuen Punkt an (bInsAfter?nach:vor) einem bestimmten Index einfuegen.

    // Loeschen eines Punktes. Ret=FALSE -> Obj besser loeschen

    // Wenn Objektspaltung, dann Ret=NeuesObj, sonst Ret=NULL.
    // Bei Objektspaltung das Ursprungsobjekt die Punkte mit den niedrigen
    // Handlenummern behalten und den oberen Teil an das neue Objekt uebergeben.
    // Wenn sich beim Rip die Handlezuordnungen aendern muss das Objekt dies
    // ueber den Rueckgabeparameter rNewPt0Index bekanntgeben. Das passiert z.B.
    // wenn ein Polygon aufgetrennt wird und dadurch eine Polyline entsteht.
    // Das Zeichenobjekt muss in den Parameter rNewPt0Index (default=0) dann
    // die Handlenummer (=Index) schreiben, unter der der Punkt, der vorher mit
    // Index 0 ansprechbar war, nun erreichbar ist.
    // Das ist zwingend notwendig, wenn an der View Auftrennen bei
    // Mehrfachpunktselektion vollzogen wird.

    // Objekt schliessen (z.B. Polyline->Polygon)

    // Alle geometrischen Daten holen fuer's Undo/Redo
    virtual SdrObjGeoData* GetGeoData() const;

    // ItemSet access
    virtual const SfxItemSet& GetItemSet() const;
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);
    virtual void SetItem(const SfxPoolItem& rItem);
    virtual void ClearItem(const sal_uInt16 nWhich = 0);
    virtual void SetItemSet(const SfxItemSet& rSet);
    virtual void BroadcastItemChange(const SdrBroadcastItemChange& rChange);
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // syntactical sugar for ItemSet accesses
    void SetItemSetAndBroadcast(const SfxItemSet& rSet);
    const SfxPoolItem& GetItem(const sal_uInt16 nWhich) const;

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual BOOL AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) const;
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);
    virtual void PostItemChange(const sal_uInt16 nWhich);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    // NotPersistAttr fuer Layer, ObjName, geometrische Transformationen, ...
    void TakeNotPersistAttr(SfxItemSet& rAttr, FASTBOOL bMerge) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);

    // bDontRemoveHardAttr=FALSE: alle in der Vorlage gesetzten Attribute werden am
    // Zeichenobjekt auf Default gesetzt; TRUE: alle harten Attribute bleiben erhalten.
    virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
    virtual SfxStyleSheet* GetStyleSheet() const;

    // TextEdit
    virtual FASTBOOL HasTextEdit() const;
    virtual SdrObject* CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    SdrObject* CheckTextEditHit(const Point& rPnt, USHORT nTol) const { return CheckTextEditHit(rPnt,nTol,NULL); }
    FASTBOOL IsTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const { return CheckTextEditHit(rPnt,nTol,pVisiLayer)!=NULL; }
    FASTBOOL IsTextEditHit(const Point& rPnt, USHORT nTol) const { return CheckTextEditHit(rPnt,nTol,NULL)!=NULL; }

    // Return==TRUE: TextEditMode gestartet
    virtual FASTBOOL BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);

    // Text wird im Format des Outliners gehalten
    // SetOutlinerParaObject: Eigentumsuebereignung von *pTextObject!
    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual void NbcReformatText();
    virtual void ReformatText();

    // Animations-Iterationscounter Resetten. Neustart der Animation
    // beim naechsten Paint. pPageView=NULL: Alle PageViews.
    virtual void RestartAnimation(SdrPageView* pPageView) const;

    // Macrofaehigkeit, z.B. ein Rechteck als PushButton.
    virtual FASTBOOL HasMacro() const;

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
    virtual FASTBOOL IsNode() const;

    // Automatische Klebepunkte:
    // je 4 Scheitelpunkt- und Eckpositionen muss ein Knotenobjekt liefern
    // i.d.R. 0=oben, 1=rechts, 2=unten, 3=links
    virtual SdrGluePoint GetVertexGluePoint(USHORT nNum) const;

    // i.d.R. 0=linksoben, 1=rechtsoben, 2=rechtsunten, 3=linksunten

    // Liste aller Klebepunkte. Kann NULL sein.
    virtual SdrGluePointList* GetGluePointList();

    // Nach veraendern der GluePointList muss man am Obj SendRepaintBroadcast rufen!
    virtual SdrGluePointList* ForceGluePointList();

    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    void SetGlueReallyAbsolute(FASTBOOL bOn);
    void NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs);
    void NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2);
    void NbcShearGluePoints (const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    // Objekt ist eine Kante?

    // Xor-Darstellung der Kante beim Draggen einer der beiden Knoten

    // bTail1=TRUE: Linienanfang, sonst LinienEnde
    // pObj=NULL: Disconnect
    virtual void ConnectToNode(FASTBOOL bTail1, SdrObject* pObj);
    virtual void DisconnectFromNode(FASTBOOL bTail1);

    // Wenn ein Objekt in der Lage ist, sich in ein Polygon oder in eine
    // Bezierkurve (oder beides) zu verwandeln, dann sollten die folgenden
    // Methoden ueberladen werden.
    // Z.B. Ein RectObj mit Strichstaerke 10, SOLID_PEN in Polygon wandeln:
    // Im Modus bLineToArea=FALSE soll ein PolyObj mit 4 Stuetzstellen,
    // Strichstaerke 10 und SOLiD_PEN erzeugt werden.
    // Im Modus bLineToArea=TRUE dagegen soll das generierte Obj immer ein
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
    // dass beide Modi (bLineToArea=TRUE/FALSE) identisch sind.
    // Defaulted sind diese Methoden auf "Ich kann das nicht" (FALSE/NULL).
    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;
    SdrObject* ConvertToPolyObj(BOOL bBezier, BOOL bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion

    // Schreiben in einen Stream: Der Operator << schreibt zunaest den 28 Bytes
    //   langen SdrObjIOHeader in den Stream und ruft anschliessend die vMethode
    //   WriteData() auf. Zum Schluss wird noch das vorerst auf 0 initiallisierte
    //   Feld nByteAnz des SdrObjIOHeader gefuellt. Die geerbte Methode WriteData
    //   ist in ueberlagerten Methoden stets am Anfang zu rufen.
    virtual void WriteData(SvStream& rOut) const;

    // Lesen aus einem Stream: Zunaest wird der SdrIOHeader von einem Reader am
    //   SdrModel gelesen (Read und SeekBack). Der Reader versucht dann eine
    //   Instanz der entsprechenden Klasse entweder selbst zu erzeugen oder bei
    //   durch Aufruf eines Links erzeugen zu lassen. Anschliessend wird vom
    //   Reader der Operator >> aufgerufen der wiederum die vMethode ReadData()
    //   ruft. Der Parameter aHead in ReadData() erlaubt eine Versionskontrolle
    //   der zu lesenden Daten.
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    // AfterRead wird gerufen, nachdem das gesamte Model eingestreamt
    // worden ist. Damit ist ein PostProcessing moeglich um beispielsweise
    // Objektverknuepfungen ueber Surrogate herzustellen o.ae.
    // Anwendungsbeispiel hierfuer SdrEdgeObj (Objektverbinder)
    virtual void AfterRead();

    friend SvStream& operator<<(SvStream& rOut, const SdrObject& rObj);
    friend SvStream& operator>>(SvStream& rIn, SdrObject& rObj);

    // TRUE: Referenz auf ein Obj
    FASTBOOL IsVirtualObj() const { return bVirtObj; }

    // TRUE=Obj kann warsch. gefuellt werden; FALSE=Obj kann warsch. Linienenden haben.
    // ungueltig, wenn es sich um ein GroupObj handelt.
    FASTBOOL IsClosedObj() const { return bClosedObj; }
    FASTBOOL IsWriterFlyFrame() const { return bWriterFlyFrame; }
    FASTBOOL IsEdgeObj() const { return bIsEdge; }
    FASTBOOL Is3DObj() const { return bIs3DObj; }
    FASTBOOL IsUnoObj() const { return bIsUnoObj; }
    FASTBOOL IsMasterCachable() const { return !bNotMasterCachable; }
    FASTBOOL ShareLock() { FASTBOOL r=!bNetLock; bNetLock=TRUE; return r; }
    void ShareUnlock() { bNetLock=FALSE; }
    FASTBOOL IsShareLock() const { return bNetLock; }
    void SetMarkProtect(FASTBOOL bProt) { bMarkProt=bProt; }
    FASTBOOL IsMarkProtect() const { return bMarkProt; }
    void SetInserted(FASTBOOL bIns);
    FASTBOOL IsInserted() const { return bInserted; }
    void SetGrouped(FASTBOOL bGrp) { bGrouped=bGrp; }
    FASTBOOL IsGrouped() const { return bGrouped; }
    void SetMoveProtect(FASTBOOL bProt);
    FASTBOOL IsMoveProtect() const { return bMovProt; }
    void SetResizeProtect(FASTBOOL bProt);
    FASTBOOL IsResizeProtect() const { return bSizProt; }
    void SetPrintable(FASTBOOL bPrn);
    FASTBOOL IsPrintable() const { return !bNoPrint; }
    void SetEmptyPresObj(FASTBOOL bEpt) { bEmptyPresObj=bEpt; }
    FASTBOOL IsEmptyPresObj() const { return bEmptyPresObj; }
    FASTBOOL IsNotPersistent() const { return bNotPersistent; }
    FASTBOOL IsNeedColorRestore() const { return bNeedColorRestore; }
    void SetNotVisibleAsMaster(FASTBOOL bFlg) { bNotVisibleAsMaster=bFlg; }
    FASTBOOL IsNotVisibleAsMaster() const { return bNotVisibleAsMaster; }

    // applikationsspeziefische Daten
    USHORT GetUserDataCount() const;
    SdrObjUserData* GetUserData(USHORT nNum) const;

    // Insert uebernimmt den auf dem Heap angelegten Record in den Besitz
    // des Zeichenobjekts
    void InsertUserData(SdrObjUserData* pData, USHORT nPos=0xFFFF);

    // Delete entfernt den Record aus der Liste und ruft
    // ein delete (FreeMem+Dtor).

    // ItemPool fuer dieses Objekt wechseln
    virtual void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = NULL );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // access to the UNO representation of the shape
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape();
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > getWeakUnoShape() { return mxUnoShape; }

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
        GrantXShapeAccess aGrant
    )
    {
        mxUnoShape = _rxUnoShape;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual BOOL TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon);

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

class SdrObjFactory
{
public:
    UINT32						nInventor;
    UINT16						nIdentifier;

    // fuer MakeNewObj():
    SdrPage*					pPage;
    SdrModel*					pModel;
    SdrObject*					pNewObj;

    // fuer MakeNewObjUserData():
    SdrObject*					pObj;
    SdrObjUserData*				pNewData;

private:
    SdrObjFactory(UINT32 nInvent, UINT16 nIdent, SdrPage* pNewPage, SdrModel* pNewModel) { nInventor=nInvent; nIdentifier=nIdent; pNewObj=NULL; pPage=pNewPage; pModel=pNewModel; pObj=NULL; pNewData=NULL; }
    SdrObjFactory(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj1) { nInventor=nInvent; nIdentifier=nIdent; pNewObj=NULL; pPage=NULL; pModel=NULL; pObj=pObj1; pNewData=NULL; }

public:
    static SdrObject* MakeNewObject(UINT32 nInvent, UINT16 nIdent, SdrPage* pPage, SdrModel* pModel=NULL);
    static void InsertMakeObjectHdl(const Link& rLink);
    static void RemoveMakeObjectHdl(const Link& rLink);
    static SdrObjUserData* MakeNewObjUserData(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj);
    static void InsertMakeUserDataHdl(const Link& rLink);
    static void RemoveMakeUserDataHdl(const Link& rLink);
};

}//end of namespace binfilter
#endif //_SVDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
