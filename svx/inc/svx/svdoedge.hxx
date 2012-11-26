/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDOEDGE_HXX
#define _SVDOEDGE_HXX

#include <svx/svdotext.hxx>
#include <svx/svdglue.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/polygon/b2dpolygon.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrDragMethod;
class SdrView;
namespace sdr { namespace properties { class ConnectorProperties; }}

//************************************************************
//   Hilfsklasse SdrObjConnection
//************************************************************

class SdrObjConnection
{
private:
    friend class                SdrEdgeObj;
    friend class                ImpEdgeHdl;
    friend class                SdrCreateView;

protected:
    basegfx::B2DPoint           maObjOfs;       // Wird beim Draggen eines Knotens gesetzt
    SdrObject*                  mpConnectedSdrObject;          // Referenziertes Objekt
    sal_uInt16                  mnConnectorId;        // Konnektornummer

    // bitfield
    bool                        mbBestConnection : 1;   // true= es wird der guenstigste Konnektor gesucht
    bool                        mbBestVertex : 1; // true= es wird der guenstigste Scheitelpunkt zum konnekten gesucht
    bool                        mbAutoVertex : 1; // AutoConnector am Scheitelpunkt nCon

public:
    SVX_DLLPUBLIC ~SdrObjConnection();
    SdrObjConnection() { ResetVars(); }

    void ResetVars();
    bool TakeGluePoint(SdrGluePoint& rGP, bool bSetAbsolutePos) const;

    inline void SetBestConnection( bool rB ) { mbBestConnection = rB; };
    inline void SetBestVertex( bool rB ) { mbBestVertex = rB; };
    inline void SetAutoVertex( bool rB ) { mbAutoVertex = rB; };
    inline void SetConnectorId( sal_uInt16 nId ) { mnConnectorId = nId; };

    inline bool IsBestConnection() const { return mbBestConnection; };
    inline bool IsBestVertex() const { return mbBestVertex; };
    inline bool IsAutoVertex() const { return mbAutoVertex; };
    inline sal_uInt16 GetConnectorId() const { return mnConnectorId; };
    inline SdrObject* GetObject() const { return mpConnectedSdrObject; }
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
    basegfx::B2DPoint           aObj1Line2;
    basegfx::B2DPoint           aObj1Line3;
    basegfx::B2DPoint           aObj2Line2;
    basegfx::B2DPoint           aObj2Line3;
    basegfx::B2DPoint           aMiddleLine;

    // Nachfolgende Werte werden von ImpCalcEdgeTrack gesetzt
    long                        nAngle1;           // Austrittswinkel am Obj1
    long                        nAngle2;           // Austrittswinkel am Obj2
    sal_uInt16                  nObj1Lines;        // 1..3
    sal_uInt16                  nObj2Lines;        // 1..3
    sal_uInt16                  nMiddleLine;       // 0xFFFF=keine, sonst Punktnummer des Linienbeginns
    char                        cOrthoForm;        // Form des Ortho-Verbindes, z.B. 'Z','U',I','L','S',...

    SdrEdgeInfoRec()
    :   nAngle1(0),
        nAngle2(0),
        nObj1Lines(0),
        nObj2Lines(0),
        nMiddleLine(0xFFFF),
        cOrthoForm(0)
    {}

    basegfx::B2DPoint& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode);
    const basegfx::B2DPoint& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode) const { return const_cast< SdrEdgeInfoRec* >(this)->ImpGetLineVersatzPoint(eLineCode); }
    sal_uInt16 ImpGetPolyIdx(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
    bool ImpIsHorzLine(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
    void ImpSetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount, long nVal);
    long ImpGetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
};

//************************************************************
//   Hilfsklasse SdrEdgeObjGeoData
//************************************************************

class SdrEdgeObjGeoData : public SdrObjGeoData
{
public:
    SdrObjConnection            maCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection            maCon2;  // Verbindungszustand des Linienendes
    basegfx::B2DPolygon         maEdgeTrack;
    bool                        mbEdgeTrackDirty;// true=Verbindungsverlauf muss neu berechnet werden.
    bool                        mbEdgeTrackUserDefined;
    SdrEdgeInfoRec              maEdgeInfo;

    SdrEdgeObjGeoData();
    virtual ~SdrEdgeObjGeoData();
};

//************************************************************
//   Hilfsklasse SdrEdgeObj
//************************************************************

class SVX_DLLPUBLIC SdrEdgeObj : public SdrTextObj
{
private:
    // to allow sdr::properties::ConnectorProperties access to ImpSetAttrToEdgeInfo()
    friend class sdr::properties::ConnectorProperties;

    friend class                SdrCreateView;
    friend class                ImpEdgeHdl;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    SdrObjConnection            maCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection            maCon2;  // Verbindungszustand des Linienendes

    basegfx::B2DPolygon         maEdgeTrack;
    SdrEdgeInfoRec              maEdgeInfo;

    // bitfield
    bool                        mbEdgeTrackDirty : 1; // true=Verbindungsverlauf muss neu berechnet werden.
    bool                        mbEdgeTrackUserDefined : 1;

    // #109007#
    // Bool to allow supporession of default connects at object
    // inside test (HitTest) and object center test (see FindConnector())
    bool                        mbSuppressDefaultConnect : 1;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void ImpRecalcEdgeTrack();  // Neuberechnung des Verbindungsverlaufs
    basegfx::B2DPolygon ImpCalcEdgeTrack(
        SdrObjConnection& rCon1,
        SdrObjConnection& rCon2,
        SdrEdgeInfoRec* pInfo) const;
    static void FindConnector(
        const basegfx::B2DPoint& rPt,
        const SdrView& rSdrView,
        SdrObjConnection& rCon,
        const SdrEdgeObj* pThis,
        OutputDevice* pOut = 0);
    sal_uInt16 ImpCalcEscAngle(SdrObject* pObj, const basegfx::B2DPoint& aPt2) const;
    void ImpSetTailPoint(bool bTail1, const basegfx::B2DPoint& rPt);
    void ImpUndirtyEdgeTrack();  // eventuelle Neuberechnung des Verbindungsverlaufs
    void ImpDirtyEdgeTrack();   // invalidate connector path, so it will be recalculated next time
    void ImpSetAttrToEdgeInfo(); // Werte vom Pool nach aEdgeInfo kopieren
    void ImpSetEdgeInfoToAttr(); // Werte vom aEdgeInfo in den Pool kopieren

    virtual ~SdrEdgeObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // #109007#
    // Interface to default connect suppression
    void SetSuppressDefaultConnect(bool bNew) { mbSuppressDefaultConnect = bNew; }
    bool GetSuppressDefaultConnect() const { return mbSuppressDefaultConnect; }

    virtual bool IsSdrEdgeObj() const;
    virtual bool IsClosedObj() const;

    SdrEdgeObj(SdrModel& rSdrModel);

    SdrObjConnection& GetConnection(bool bTail1) { return *(bTail1 ? &maCon1 : &maCon2); }
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt32 nNum) const;
    virtual const SdrGluePointList* GetGluePointList() const;
    virtual SdrGluePointList* ForceGluePointList();

    // bTail1=true: Linienanfang, sonst LinienEnde
    // pObj=NULL: Disconnect
    void SetEdgeTrackDirty() { mbEdgeTrackDirty = true; }
    void ConnectToNode(bool bTail1, SdrObject* pObj);
    void DisconnectFromNode(bool bTail1);
    SdrObject* GetConnectedNode(bool bTail1) const;
    const SdrObjConnection& GetConnection(bool bTail1) const { return *(bTail1 ? &maCon1 : &maCon2); }
    bool CheckNodeConnection(bool bTail1) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    void SetEdgeTrackPath( const basegfx::B2DPolygon& rPoly );
    basegfx::B2DPolygon GetEdgeTrackPath() const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    sal_uInt32 impOldGetHdlCount() const;
    SdrHdl* impOldGetHdl(SdrHdlList& rHdlList, sal_uInt32 nHdlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);

    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;
    virtual bool IsPolygonObject() const;
    virtual sal_uInt32 GetObjectPointCount() const;
    virtual basegfx::B2DPoint GetObjectPoint(sal_uInt32 i) const;
    virtual void SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i);

    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

    /** updates edges that are connected to the edges of this object
        as if the connected objects send a repaint broadcast
        #103122#
    */
    void ReformatEdge();

    // helper methods for the StarOffice api
    basegfx::B2DPoint GetTailPoint( bool bTail ) const;
    void SetTailPoint( bool bTail, const basegfx::B2DPoint& rPt );
    void setGluePointIndex( bool bTail, sal_Int32 nId = -1 );
    sal_Int32 getGluePointIndex( bool bTail );

    // for geometry access
//  ::basegfx::B2DPolygon getEdgeTrack() const;

    // helper method for SdrDragMethod::AddConnectorOverlays. Adds a overlay polygon for
    // this connector to rResult.
    basegfx::B2DPolygon ImplAddConnectorOverlay(SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const;

    // get/setSdrObjectTransformation
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
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
//  bool   EdgeEscAsRay        Default false
//      true= die Verbindungslinie tritt aus dem Obj Strahlenfoermig aus.
//      Also Winkelvorgabe durch die Strecke ObjMitte/Konnektor.
//
//  bool   EdgeEscUseObjAngle  Default false
//      Objektdrehwinkelberuecksichtigung.
//      true= Bei der Bestimmung des Objektaustrittswinkels wird der
//      Drehwinkel des Objekts als Offset beruecksichtigt.
//
//  sal_uInt32  EdgeFlowDefDist     Default 0, min 0, max ?
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

#endif //_SVDOEDGE_HXX

