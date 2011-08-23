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

#ifndef _SVDOEDGE_HXX
#define _SVDOEDGE_HXX

#include <bf_svx/svdotext.hxx>

#include <bf_svx/svdglue.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrObjSurrogate;
class XPolygon;
class SdrPageView;
class SdrEdgeSetItem;

//************************************************************
//   Hilfsklasse SdrObjConnection
//************************************************************

class SdrObjConnection
{
    friend class				SdrEdgeObj;
    friend class				ImpEdgeHdl;
    friend class				SdrCreateView;

protected:
    Point						aObjOfs;       // Wird beim Draggen eines Knotens gesetzt
    SdrObjSurrogate*			pSuro;   // Datenhaltung zwischen ReadData() und AfterRead()
    SdrObject*					pObj;          // Referenziertes Objekt
    long						nXDist;        // Hor. Objektabstand wenn bXDistOvr=TRUE
    long						nYDist;        // Vert. Objektabstand wenn bYDistOvr=TRUE
    USHORT						nConId;        // Konnektornummer
    BOOL						bBestConn : 1;   // TRUE= es wird der guenstigste Konnektor gesucht
    BOOL						bBestVertex : 1; // TRUE= es wird der guenstigste Scheitelpunkt zum konnekten gesucht
    BOOL						bXDistOvr : 1;   // TRUE= Hor. Objektabstand wurde gedragt (Overwrite)
    BOOL						bYDistOvr : 1;   // TRUE= Vert. Objektabstand wurde gedragt (Overwrite)
    BOOL						bAutoVertex : 1; // AutoConnector am Scheitelpunkt nCon
    BOOL						bAutoCorner : 1; // AutoConnector am Eckpunkt nCon

public:
    SdrObjConnection() { ResetVars(); }
    ~SdrObjConnection();

    void ResetVars();
    FASTBOOL TakeGluePoint(SdrGluePoint& rGP, FASTBOOL bSetAbsolutePos) const;
    void Write(SvStream& rOut, const SdrObject* pObj) const;
    void Read(SvStream& rIn, const SdrObject* pObj);
    void ReadTilV10(SvStream& rIn, const SdrObject* pObj);
    void AfterRead(const SdrObject* pObj);

    inline void SetBestConnection( BOOL rB ) { bBestConn = rB; };
    inline void SetBestVertex( BOOL rB ) { bBestVertex = rB; };
    inline void SetAutoVertex( BOOL rB ) { bAutoVertex = rB; };
    inline void SetConnectorId( USHORT nId ) { nConId = nId; };

    inline BOOL IsBestConnection() const { return bBestConn; };
    inline BOOL IsBestVertex() const { return bBestVertex; };
    inline BOOL IsAutoVertex() const { return bAutoVertex; };
    inline sal_uInt16 GetConnectorId() const { return nConId; };
    inline SdrObject* GetObject() const { return pObj; }
};

//************************************************************
//   Hilfsklasse SdrEdgeInfoRec
//************************************************************

enum SdrEdgeLineCode {OBJ1LINE2,OBJ1LINE3,OBJ2LINE2,OBJ2LINE3,MIDDLELINE};

class SdrEdgeInfoRec
{
public:
    // Die 5 Distanzen werden beim draggen bzw. per SetAttr gesetzt und von
    // ImpCalcEdgeTrack ausgewertet. Per Get/SetAttr/Get/SetStyleSh werden
    // jedoch nur 0-3 longs transportiert.
    Point						aObj1Line2;
    Point						aObj1Line3;
    Point						aObj2Line2;
    Point						aObj2Line3;
    Point						aMiddleLine;

    // Nachfolgende Werte werden von ImpCalcEdgeTrack gesetzt
    long						nAngle1;           // Austrittswinkel am Obj1
    long						nAngle2;           // Austrittswinkel am Obj2
    USHORT						nObj1Lines;        // 1..3
    USHORT						nObj2Lines;        // 1..3
    USHORT						nMiddleLine;       // 0xFFFF=keine, sonst Punktnummer des Linienbeginns
    char						cOrthoForm;        // Form des Ortho-Verbindes, z.B. 'Z','U',I','L','S',...

public:
    SdrEdgeInfoRec()
    :	nAngle1(0),
        nAngle2(0),
        nObj1Lines(0),
        nObj2Lines(0),
        nMiddleLine(0xFFFF),
        cOrthoForm(0)
    {}

    Point& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode);
    const Point& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode) const { return ((SdrEdgeInfoRec*)this)->ImpGetLineVersatzPoint(eLineCode); }
    USHORT ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    FASTBOOL ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    void ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal);
    long ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;

    friend SvStream& operator<<(SvStream& rOut, const SdrEdgeInfoRec& rEI);
    friend SvStream& operator>>(SvStream& rIn, SdrEdgeInfoRec& rEI);
};

//************************************************************
//   Hilfsklasse SdrEdgeObjGeoData
//************************************************************


//************************************************************
//   Hilfsklasse SdrEdgeObj
//************************************************************

class SdrEdgeObj : public SdrTextObj
{
    friend class				SdrCreateView;
    friend class				ImpEdgeHdl;

protected:
    SdrObjConnection			aCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection			aCon2;  // Verbindungszustand des Linienendes

    XPolygon*					pEdgeTrack;
    FASTBOOL					bEdgeTrackDirty; // TRUE=Verbindungsverlauf muss neu berechnet werden.
    FASTBOOL					nNotifyingCount; // Verrieglung
    SdrEdgeInfoRec				aEdgeInfo;

    // #109007#
    // Bool to allow supporession of default connects at object
    // inside test (HitTest) and object center test (see ImpFindConnector())
    FASTBOOL					mbSuppressDefaultConnect;

public:
    // #109007#
    // Interface to default connect suppression
    void SetSuppressDefaultConnect(sal_Bool bNew) { mbSuppressDefaultConnect = (FASTBOOL)bNew; }
    sal_Bool GetSuppressDefaultConnect() const { return (sal_Bool)mbSuppressDefaultConnect; }

protected:
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

    XPolygon ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rCenter) const;
    void ImpRecalcEdgeTrack();  // Neuberechnung des Verbindungsverlaufs
    XPolygon ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const;
    XPolygon ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
        const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
        ULONG* pnQuality, SdrEdgeInfoRec* pInfo) const;
    USHORT ImpCalcEscAngle(SdrObject* pObj, const Point& aPt2) const;
    void ImpSetTailPoint(FASTBOOL bTail1, const Point& rPt);
    void ImpSetAttrToEdgeInfo(); // Werte vom Pool nach aEdgeInfo kopieren
    void ImpSetEdgeInfoToAttr(); // Werte vom aEdgeInfo in den Pool kopieren

public:
    TYPEINFO();

    SdrEdgeObj();
    virtual ~SdrEdgeObj();

    SdrObjConnection& GetConnection(FASTBOOL bTail1) { return *(bTail1 ? &aCon1 : &aCon2); }
    virtual UINT16 GetObjIdentifier() const;
    virtual const Rectangle& GetBoundRect() const;
    virtual const Rectangle& GetSnapRect() const;
    virtual FASTBOOL IsNode() const;
    virtual SdrGluePoint GetVertexGluePoint(USHORT nNum) const;

    // ItemSet access
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);

    // bTail1=TRUE: Linienanfang, sonst LinienEnde
    // pObj=NULL: Disconnect
    void SetEdgeTrackDirty() { bEdgeTrackDirty=TRUE; }
    void ConnectToNode(FASTBOOL bTail1, SdrObject* pObj);
    void DisconnectFromNode(FASTBOOL bTail1);
    SdrObject* GetConnectedNode(FASTBOOL bTail1) const;
    const SdrObjConnection& GetConnection(FASTBOOL bTail1) const { return *(bTail1 ? &aCon1 : &aCon2); }

    virtual void RecalcBoundRect();
    virtual void RecalcSnapRect();
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;



    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcMove(const Size& aSize);
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact);

    // #102344# Added missing implementation
    virtual void NbcSetAnchorPos(const Point& rPnt);


    virtual void NbcSetPoint(const Point& rPnt, USHORT i);


    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);
    virtual void AfterRead();

    /** updates edges that are connected to the edges of this object
        as if the connected objects send a repaint broadcast
        #103122#
    */
    void Reformat();

    // helper methods for the StarOffice api
    Point GetTailPoint( BOOL bTail ) const;
    void SetTailPoint( BOOL bTail, const Point& rPt );
    void setGluePointIndex( sal_Bool bTail, sal_Int32 nId = -1 );
    sal_Int32 getGluePointIndex( sal_Bool bTail );
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Zur Bestimmung der Verlaufslinie werden folgende Item-Parameter des SdrItemPool verwendet:
//
//  USHORT EdgeFlowAngle       Default 9000 (=90.00 Deg), min 0, max 9000
//      Verlauffreiheitswinkel.
//      Der Winkel, in dem die Verbindungslinie verlaufen darf.
//
//  USHORT EdgeEscAngle        Default 9000 (=90.00 Deg), min 0, max 9000
//      Objektaustrittswinkel.
//      Der Winkel, in dem die Verbindungslinie aus dem Objekt austreten darf.
//
//  BOOL   EdgeEscAsRay        Default FALSE
//      TRUE= die Verbindungslinie tritt aus dem Obj Strahlenfoermig aus.
//      Also Winkelvorgabe durch die Strecke ObjMitte/Konnektor.
//
//  BOOL   EdgeEscUseObjAngle  Default FALSE
//      Objektdrehwinkelberuecksichtigung.
//      TRUE= Bei der Bestimmung des Objektaustrittswinkels wird der
//      Drehwinkel des Objekts als Offset beruecksichtigt.
//
//  ULONG  EdgeFlowDefDist     Default 0, min 0, max ?
//      Das ist der Default-Mindestabstand der bei der Berechnung der
//      Verbindungslinie zu den angedockten Objekten in logischen Einheiten.
//      Dieser Abstand wird innerhalb des Objektes "ueberschrieben", sobald
//      der User an den Linien draggd. Beim Andocken an ein neues Objekt wird
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
//
//////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOEDGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
