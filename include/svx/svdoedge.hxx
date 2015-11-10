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

#ifndef INCLUDED_SVX_SVDOEDGE_HXX
#define INCLUDED_SVX_SVDOEDGE_HXX

#include <svx/svdotext.hxx>
#include <svx/svdglue.hxx>
#include <svx/svxdllapi.h>


//   Vorausdeklarationen


class SdrDragMethod;
class SdrPageView;

namespace sdr { namespace properties {
    class ConnectorProperties;
}}


//   Hilfsklasse SdrObjConnection


class SdrObjConnection
{
    friend class                SdrEdgeObj;
    friend class                ImpEdgeHdl;
    friend class                SdrCreateView;

protected:
    Point                       aObjOfs;       // Wird beim Draggen eines Knotens gesetzt
    SdrObject*                  pObj;          // Referenziertes Objekt
    long                        nXDist;        // Hor. Objektabstand wenn bXDistOvr=TRUE
    long                        nYDist;        // Vert. Objektabstand wenn bYDistOvr=TRUE
    sal_uInt16                      nConId;        // Konnektornummer

    // bitfield
    bool                        bBestConn : 1;   // sal_True= es wird der guenstigste Konnektor gesucht
    bool                        bBestVertex : 1; // sal_True= es wird der guenstigste Scheitelpunkt zum konnekten gesucht
    bool                        bXDistOvr : 1;   // sal_True= Hor. Objektabstand wurde gedragt (Overwrite)
    bool                        bYDistOvr : 1;   // sal_True= Vert. Objektabstand wurde gedragt (Overwrite)
    bool                        bAutoVertex : 1; // AutoConnector am Scheitelpunkt nCon
    bool                        bAutoCorner : 1; // AutoConnector am Eckpunkt nCon

public:
    SdrObjConnection() { ResetVars(); }
    SVX_DLLPUBLIC ~SdrObjConnection();

    void ResetVars();
    bool TakeGluePoint(SdrGluePoint& rGP, bool bSetAbsolutePos) const;

    inline void SetBestConnection( bool rB ) { bBestConn = rB; };
    inline void SetBestVertex( bool rB ) { bBestVertex = rB; };
    inline void SetAutoVertex( bool rB ) { bAutoVertex = rB; };
    inline void SetConnectorId( sal_uInt16 nId ) { nConId = nId; };

    inline bool IsBestConnection() const { return bBestConn; };
    inline bool IsAutoVertex() const { return bAutoVertex; };
    inline sal_uInt16 GetConnectorId() const { return nConId; };
    inline SdrObject* GetObject() const { return pObj; }
};


//   Hilfsklasse SdrEdgeInfoRec


enum SdrEdgeLineCode {OBJ1LINE2,OBJ1LINE3,OBJ2LINE2,OBJ2LINE3,MIDDLELINE};

class SdrEdgeInfoRec
{
public:
    // Die 5 Distanzen werden beim draggen bzw. per SetAttr gesetzt und von
    // ImpCalcEdgeTrack ausgewertet. Per Get/SetAttr/Get/SetStyleSh werden
    // jedoch nur 0-3 longs transportiert.
    Point                       aObj1Line2;
    Point                       aObj1Line3;
    Point                       aObj2Line2;
    Point                       aObj2Line3;
    Point                       aMiddleLine;

    // Nachfolgende Werte werden von ImpCalcEdgeTrack gesetzt
    long                        nAngle1;           // Austrittswinkel am Obj1
    long                        nAngle2;           // Austrittswinkel am Obj2
    sal_uInt16                      nObj1Lines;        // 1..3
    sal_uInt16                      nObj2Lines;        // 1..3
    sal_uInt16                      nMiddleLine;       // 0xFFFF=keine, sonst Punktnummer des Linienbeginns
    char                        cOrthoForm;        // Form des Ortho-Verbindes, z.B. 'Z','U',I','L','S',...

public:
    SdrEdgeInfoRec()
    :   nAngle1(0),
        nAngle2(0),
        nObj1Lines(0),
        nObj2Lines(0),
        nMiddleLine(0xFFFF),
        cOrthoForm(0)
    {}

    Point& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode);
    const Point& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode) const { return const_cast<SdrEdgeInfoRec*>(this)->ImpGetLineVersatzPoint(eLineCode); }
    sal_uInt16 ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    bool ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    void ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal);
    long ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
};


//   Hilfsklasse SdrEdgeObjGeoData


class SdrEdgeObjGeoData : public SdrTextObjGeoData
{
public:
    SdrObjConnection            aCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection            aCon2;  // Verbindungszustand des Linienendes
    XPolygon*                   pEdgeTrack;
    bool                        bEdgeTrackDirty;// sal_True=Verbindungsverlauf muss neu berechnet werden.
    bool                        bEdgeTrackUserDefined;
    SdrEdgeInfoRec              aEdgeInfo;

public:
    SdrEdgeObjGeoData();
    virtual ~SdrEdgeObjGeoData();
};


//   Hilfsklasse SdrEdgeObj


class SVX_DLLPUBLIC SdrEdgeObj : public SdrTextObj
{
private:
    // to allow sdr::properties::ConnectorProperties access to ImpSetAttrToEdgeInfo()
    friend class sdr::properties::ConnectorProperties;

    friend class                SdrCreateView;
    friend class                ImpEdgeHdl;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

    SdrObjConnection            aCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection            aCon2;  // Verbindungszustand des Linienendes

    XPolygon*                   pEdgeTrack;
    sal_uInt16                  nNotifyingCount; // Verrieglung
    SdrEdgeInfoRec              aEdgeInfo;

    // bitfield
    bool                        bEdgeTrackDirty : 1; // sal_True=Verbindungsverlauf muss neu berechnet werden.
    bool                        bEdgeTrackUserDefined : 1;

    // Bool to allow supporession of default connects at object
    // inside test (HitTest) and object center test (see ImpFindConnector())
    bool                        mbSuppressDefaultConnect : 1;

    // Flag value for avoiding death loops when calculating BoundRects
    // from circulary connected connectors. A coloring algorithm is used
    // here. When the GetCurrentBoundRect() calculation of a SdrEdgeObj
    // is running, the flag is set, else it is always sal_False.
    bool                        mbBoundRectCalculationRunning : 1;

    // #i123048# need to remember if layouting was suppressed before to get
    // to a correct state for first real layouting
    bool                        mbSuppressed : 1;

public:
    // Interface to default connect suppression
    void SetSuppressDefaultConnect(bool bNew) { mbSuppressDefaultConnect = bNew; }
    bool GetSuppressDefaultConnect() const { return mbSuppressDefaultConnect; }

    bool IsBoundRectCalculationRunning() const { return mbBoundRectCalculationRunning; }

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    static XPolygon ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rCenter);
    void ImpRecalcEdgeTrack();  // Neuberechnung des Verbindungsverlaufs
    XPolygon ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const;
    XPolygon ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
        const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
        sal_uIntPtr* pnQuality, SdrEdgeInfoRec* pInfo) const;
    static bool ImpFindConnector(const Point& rPt, const SdrPageView& rPV, SdrObjConnection& rCon, const SdrEdgeObj* pThis, OutputDevice* pOut=nullptr);
    static SdrEscapeDirection ImpCalcEscAngle(SdrObject* pObj, const Point& aPt2);
    void ImpSetTailPoint(bool bTail1, const Point& rPt);
    void ImpUndirtyEdgeTrack();  // eventuelle Neuberechnung des Verbindungsverlaufs
    void ImpDirtyEdgeTrack();   // invalidate connector path, so it will be recalculated next time
    void ImpSetAttrToEdgeInfo(); // Werte vom Pool nach aEdgeInfo kopieren
    void ImpSetEdgeInfoToAttr(); // Werte vom aEdgeInfo in den Pool kopieren

public:
    TYPEINFO_OVERRIDE();

    SdrEdgeObj();
    virtual ~SdrEdgeObj();

    SdrObjConnection& GetConnection(bool bTail1) { return *(bTail1 ? &aCon1 : &aCon2); }
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual const Rectangle& GetCurrentBoundRect() const override;
    virtual const Rectangle& GetSnapRect() const override;
    virtual bool IsNode() const override;
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const override;
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const override;
    virtual const SdrGluePointList* GetGluePointList() const override;
    virtual SdrGluePointList* ForceGluePointList() override;

    // bTail1=TRUE: Linienanfang, sonst LinienEnde
    // pObj=NULL: Disconnect
    void SetEdgeTrackDirty() { bEdgeTrackDirty=true; }
    void ConnectToNode(bool bTail1, SdrObject* pObj) override;
    void DisconnectFromNode(bool bTail1) override;
    SdrObject* GetConnectedNode(bool bTail1) const override;
    const SdrObjConnection& GetConnection(bool bTail1) const { return *(bTail1 ? &aCon1 : &aCon2); }
    bool CheckNodeConnection(bool bTail1) const;

    virtual void RecalcSnapRect() override;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const override;
    virtual SdrEdgeObj* Clone() const override;
    SdrEdgeObj& operator=(const SdrEdgeObj& rObj);
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    void    SetEdgeTrackPath( const basegfx::B2DPolyPolygon& rPoly );
    basegfx::B2DPolyPolygon GetEdgeTrackPath() const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void NbcMove(const Size& aSize) override;
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact) override;

    // #i54102# added rotate, mirror and shear support
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    // #102344# Added missing implementation
    virtual void NbcSetAnchorPos(const Point& rPnt) override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual Pointer GetCreatePointer() const override;
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;
    virtual bool IsPolyObj() const override;
    virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 i) const override;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

    /** updates edges that are connected to the edges of this object
        as if the connected objects send a repaint broadcast
        #103122#
    */
    void Reformat();

    // helper methods for the StarOffice api
    Point GetTailPoint( bool bTail ) const;
    void SetTailPoint( bool bTail, const Point& rPt );
    void setGluePointIndex( bool bTail, sal_Int32 nId = -1 );
    sal_Int32 getGluePointIndex( bool bTail );

    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    // for geometry access
    ::basegfx::B2DPolygon getEdgeTrack() const;

    // helper method for SdrDragMethod::AddConnectorOverlays. Adds a overlay polygon for
    // this connector to rResult.
    basegfx::B2DPolygon ImplAddConnectorOverlay(SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const;
};

// Zur Bestimmung der Verlaufslinie werden folgende Item-Parameter des SdrItemPool verwendet:
//
//  sal_uInt16 EdgeFlowAngle       Default 9000 (=90.00 Deg), min 0, max 9000
//      Verlauffreiheitswinkel.
//      Der Winkel, in dem die Verbindungslinie verlaufen darf.
//
//  sal_uInt16 EdgeEscAngle        Default 9000 (=90.00 Deg), min 0, max 9000
//      Objektaustrittswinkel.
//      Der Winkel, in dem die Verbindungslinie aus dem Objekt austreten darf.
//
//  bool   EdgeEscAsRay        Default FALSE
//      sal_True= die Verbindungslinie tritt aus dem Obj Strahlenfoermig aus.
//      Also Winkelvorgabe durch die Strecke ObjMitte/Konnektor.
//
//  bool   EdgeEscUseObjAngle  Default FALSE
//      Objektdrehwinkelberuecksichtigung.
//      sal_True= Bei der Bestimmung des Objektaustrittswinkels wird der
//      Drehwinkel des Objekts als Offset beruecksichtigt.
//
//  sal_uIntPtr  EdgeFlowDefDist     Default 0, min 0, max ?
//      Das ist der Default-Mindestabstand der bei der Berechnung der
//      Verbindungslinie zu den angedockten Objekten in logischen Einheiten.
//      Dieser Abstand wird innerhalb des Objektes "ueberschrieben", sobald
//      der User an den Linien dragged. Beim Andocken an ein neues Objekt wird
//      dann jedoch wieder dieser Default verwendet.
//
//
// Allgemeines zu Konnektoren:
//
// Es gibt Knoten und Kantenobjekte. Zwei Knoten koennen durch eine Kante
// miteinander verbunden werden. Ist eine Kante nur an einem Ende an einen
// Knoten geklebt, ist das andere Ende auf einer absoluten Position im Doc
// fixiert. Ebenso ist es natuerlich auch moeglich, dass eine Kante an beiden
// Enden "frei", also nicht mit einem Knotenobjekt verbunden ist.
//
// Ein Kantenobjekt kann theoretisch auch gleichzeitig Knotenobjekt sein. In
// der ersten Version wird das jedoch noch nicht realisiert werden.
//
// Eine Verbindung zwischen Knoten und Kante kann hergestellt werden durch:
// - Interaktives erzeugen eines neuen Kantenobjekts an der SdrView wobei
//   Anfangs- bzw. Endpunkt der Kante auf ein Konnektor (Klebestelle) eines
//   bereits vorhandenen Knotenobjekts gelegt wird.
// - Interaktives draggen des Anfangs- bzw. Endpunkts eines bestehenden
//   Kantenobjekts an der SdrView auf ein Konnektor (Klebestelle) eines
//   bereits vorhandenen Knotenobjekts.
// - Undo/Redo
// Verschieben von Knotenobjekten stellt keine Verbindungen her. Ebenso auch
// nicht das direkte Verschieben von Kantenendpunkten am SdrModel...
// Verbindungen koennen auch hergestellt werden, wenn die Konnektoren an der
// View nicht sichtbar geschaltet sind.
//
// Eine vorhandene Verbindung zwischen Knoten und Kante bleibt erhalten bei:
// - Draggen (Move/Resize/Rotate/...) des Knotenobjekts
// - Verschieben einer Konnektorposition im Knotemobjekt
// - gleichzeitiges Draggen (Move/Resize/Rotate/...) von Knoten und Kante
//
// Eine Verbindung zwischen Knoten und Kante kann geloesst werden durch:
// - Loeschen eines der Objekte
// - Draggen des Kantenobjekts ohne gleichzeitiges Draggen des Knotens
// - Loeschen des Konnektors am Knotenobjekt
// - Undo/Redo/Repeat
// Beim Draggen muss die Aufforderung zum loesen der Verbindung von ausserhalb
// des Models befohlen werden (z.B. von der SdrView). SdrEdgeObj::Move() loesst
// die Verbindung nicht selbsttaetig.
//
// Jedes Knotenobjekt kann Konnektoren, sog. Klebestellen besitzen. Das sind die
// geometrischen Punkte, an denen das verbindende Kantenobjekt bei hergestellter
// Verbindung endet. Defaultmaessig hat jedes Objekt keine Konnektoren. Trotzdem
// kann man bei bestimmten View-Einstellungen eine Kante andocken, da dann z.B.
// an den 4 Scheitelpunkten des Knotenobjekts bei Bedarf automatisch Konnektoren
// generiert werden. Jedes Objekt liefert dafuer 2x4 sog. Default-Konnektorposi-
// tionen, 4 an den Scheitelpunkten und 4 an den Eckpositionen. Im Normalfall
// liegen diese an den 8 Handlepositionen; Ausnahmen bilden hier Ellipsen,
// Parallelogramme, ... . Darueberhinaus koennen auch an jedem Knotenobjekt
// anwenderspeziefische Konnektoren gesetzt werden.
//
// Dann gibt es noch die Moeglichkeit, ein Kante an einem Objekt mit dem
// Attribut "bUseBestConnector" anzudocken. Es wird dann aus dem Angebot der
// Konnektoren des Objekts oder/und der Scheitelpunkte, jeweils die fuer den
// Verlauf der Verbindungslinie guenstigste Konnektorposition verwendet. Der
// Anwender vergibt dieses Attribut, indem er den Knoten in seiner Mitte
// andockt (siehe z.B. Visio).
// 09-06-1996: bUseBestConnector verwendet nur Scheitelpunktklebepunkte.
//
// Und hier noch etwas Begriffsdefinition:
//   Verbinder : Eben das Verbinderobjekt (Kantenobjekt)
//   Knoten    : Ein beliebiges Objekt, an dem ein Verbinder drangeklebt
//               werden kann, z.B. ein Rechteck, ...
//   Klebepunkt: Der Punkt, an dem der Verbinder an das Knotenobjekt
//               geklebt wird. Hierbei gibt es:
//                 Scheitelpunktklebepunkte: Jedes Knotenobjekt hat diese
//                     Klebepunkte von Natur aus. Moeglicherweise gibt es
//                     im Draw bereits die Option "Automatisch ankleben an
//                     Objektscheitelpunkte" (default an)
//                 Eckpunktklebepunkte: Auch diese Klebepunkte sind den
//                     Objekten von mir bereits mitgegeben. Wie die oben
//                     erwaehnten gibt es fuer diese moeglicherweise
//                     bereits auch eine Option im Draw. (default aus)
//                 Scheitelpunktklebepunkte und Eckpunktklebepunkte sind
//                     im Gegensatz zu Visio nicht optisch sichtbar; sie
//                     sind eben einfach da (wenn Option eingeschaltet).
//                 Benutzerdefinierte Klebepunkte: Gibt es an jedem
//                     Knotenobjekt beliebig viele. Per Option koennen sie
//                     sichtbar geschaltet werden (beim editieren immer
//                     sichtbar). Zur Zeit sind die jedoch noch nicht ganz
//                     fertigimplementiert.
//                 Automatische Klebepunktwahl: Wird der Verbinder so an
//                     das Knotenobjekt gedockt, dass der schwarke Rahmen
//                     das gesamte Objekt umfasst, so versucht der
//                     Verbinder von den 4 Scheitelpunktklebepunkten (und
//                     zwar nur von denen) den guenstigsten herauszufinden.

#endif // INCLUDED_SVX_SVDOEDGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
