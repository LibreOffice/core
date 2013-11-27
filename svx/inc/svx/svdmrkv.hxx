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



#ifndef _SVDMRKV_HXX
#define _SVDMRKV_HXX

#include <svx/svdhdl.hxx>
#include <svx/svdsnpv.hxx>
#include <svx/svdtypes.hxx>
#include <svx/sdrselection.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Predefines

class ImplMarkingOverlay;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines

// folgendes ist noch nicht bzw. erst zum Teil implementiert:
#define SDRSEARCH_DEEP         0x0001 /* rekursiv in Gruppenobjekte hinein */
#define SDRSEARCH_ALSOONMASTER 0x0002 /* MasterPages werden auch durchsucht */
#define SDRSEARCH_WHOLEPAGE    0x0004 /* Nicht nur die ObjList der PageView */
#define SDRSEARCH_TESTMARKABLE 0x0008 /* nur markierbare Objekte/Punkte/Handles/... */
#define SDRSEARCH_TESTMACRO    0x0010 /* Nur Objekte mit Macro */
#define SDRSEARCH_TESTTEXTEDIT 0x0020 /* Nur TextEdit-faehige Objekte */
//#define SDRSEARCH_WITHTEXT     0x0040 /* Nur Objekte mit Text */
//#define SDRSEARCH_TESTTEXTAREA 0x0080 /* Den Textbereich von Objekten mit Text (TextEditHit) */
//#define SDRSEARCH_BACKWARD     0x0100 /* Rueckwaertssuche */
#define SDRSEARCH_MARKED       0x0400 /* Nur markierte Objekte/Punkte/... */
#define SDRSEARCH_PASS2BOUND   0x0800 /* Wenn nix gefunden, dann neuer 2. Versuch mit BoundRectHit */
#define SDRSEARCH_PASS3NEAREST 0x1000 /* Wenn nix gefunden, dann neuer 3. Versuch mit NearestBoundRectHit */

#define SDRSEARCH_PICKMARKABLE (SDRSEARCH_TESTMARKABLE)
#define SDRSEARCH_PICKTEXTEDIT (SDRSEARCH_DEEP|SDRSEARCH_TESTMARKABLE|SDRSEARCH_TESTTEXTEDIT)
#define SDRSEARCH_PICKMACRO    (SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE|SDRSEARCH_TESTMACRO)

// SDRSEARCHPASS_... ist Rueckgabeparameterwert bei PickObj().
#define SDRSEARCHPASS_DIRECT       0x0000 /* Objekt durch direkten Hit getroffen     */
#define SDRSEARCHPASS_INACTIVELIST 0x0001 /* Obj auf der Page, nicht jedoch in der AktGroup (bei WHOLEPAGE) */
#define SDRSEARCHPASS_MASTERPAGE   0x0002 /* Objekt auf der MasterPage gefunden      */

enum SdrHitKind // usage checked
{
    SDRHIT_NONE,            // Nicht getroffen, Schnaps gesoffen
    SDRHIT_OBJECT,          // Treffer versenkt
    SDRHIT_HANDLE,          // Markierungshandle
    SDRHIT_HELPLINE,        // Hilfslinie
    SDRHIT_GLUEPOINT,       // Klebepunkt
    SDRHIT_TEXTEDIT,        // Offene OutlinerView getroffen
    SDRHIT_TEXTEDITOBJ,     // Objekt fuer SdrBeginTextEdit (Textbereich)
    SDRHIT_URLFIELD,        // Feld im TextObj getroffen (wird gerade nicht editiert)
    SDRHIT_MACRO,           // Objekt fuer BegMacroObj
    SDRHIT_MARKEDOBJECT,    // Markiertes Objekt (z.B. zum Draggen)
    SDRHIT_UNMARKEDOBJECT,  // nichtmarkiertes Objekt (z.B. zum markieren)
    SDRHIT_CELL             // hit on a cell inside a table shape (outside of the cells text area)
};

enum SdrViewEditMode // usage checked
{
    SDREDITMODE_EDIT,           // Auch bekannt aus Pfeil- oder Zeigermodus
    SDREDITMODE_CREATE,         // Objekterzeugungswerkzeug
    SDREDITMODE_GLUEPOINTEDIT   // Klebepunkteditiermodus
};

// usage checked
#define IMPSDR_POINTSDESCRIPTION     0x0001
#define IMPSDR_GLUEPOINTSDESCRIPTION 0x0002
#define FRAME_HANDLES_LIMIT     (250)

////////////////////////////////////////////////////////////////////////////////////////////////////
// new helpers for selection roundabouts

SVX_DLLPUBLIC const String getSelectionDescription(const SdrObjectVector& rSdrObjectVector);
void sortSdrObjectSelection(SdrObjectVector& rSdrObjectVector);
::std::set< SdrEdgeObj*, sdr::selection::SelectionComparator > getAllConnectedEdges(const SdrObjectVector& rSdrObjectVector);
::std::vector< SdrEdgeObj* > getAllSdrEdgeObjConnectedToSdrObjectVector(const SdrObjectVector& rSdrObjects, bool bContained);

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrMarkView : public SdrSnapView, public Timer
{
private:
    ImplMarkingOverlay*                                 mpMarkObjOverlay;
    ImplMarkingOverlay*                                 mpMarkPointsOverlay;
    ImplMarkingOverlay*                                 mpMarkGluePointsOverlay;

    SVX_DLLPRIVATE void impCreatePointRanges() const;

protected:
    basegfx::B2DPoint           maRef1;            // Persistent - Drehmittelpunkt/Spiegelachse
    basegfx::B2DPoint           maRef2;            // Persistent
    basegfx::B2DPoint           maLastCrookCenter; // Persistent

private:
    // make private; access should be limited since it is dependent of selection and thus from
    // the selection to be up-to-date
    SdrHdlList                  maViewSdrHandleList;

    // new selection abstraction. Private to guarantee isolation
    sdr::selection::Selection   maSelection;

    // stuff to save focus during SdrHdl recreation
    sal_uInt32                  mnSavePolyNum;
    sal_uInt32                  mnSavePointNum;
    SdrHdlKind                  meSaveKind;
    const SdrObject*            mpSaveObj;

protected:
    basegfx::B2DRange           maMarkedPointRange;
    basegfx::B2DRange           maMarkedGluePointRange;

    sal_uInt32                  mnInsPointNum;      // Nummer des InsPoint

    SdrDragMode                 meViewDragMode;        // Persistent
    SdrViewEditMode             meViewEditMode;      // Persistent

    bool                        mbDesignMode : 1;      // DesignMode fuer SdrUnoObj
    bool                        mbForceFrameHandles : 1; // Persistent - FrameDrag auch bei Einzelobjekten
    bool                        mbPlusHdlAlways : 1;   // Persistent
    bool                        mbInsPolyPoint : 1;     // z.Zt InsPolyPointDragging
    bool                        mbSaveOldFocus : 1;

    virtual void AddDragModeHdl(SdrHdlList& rTarget, SdrDragMode eMode);
    virtual bool MouseMove(const MouseEvent& rMEvt, Window* pWin);

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl(SdrHdlList& rTarget);
    void ForceRefToMarked();

    virtual SdrObject* CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, sal_uInt32 nOptions, const SetOfByte* pMVisLay) const;
    virtual SdrObject* CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObjList* pOL, sal_uInt32 nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const;
    bool ImpIsFrameHandles() const;

    // select or deselect points associated with given SdrHdl vector
    bool ImpMarkPoints(const SdrHdlContainerType& rHdls, bool bUnmark);

    void SetMoveOutside(bool bOn);
    bool IsMoveOutside() const;

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrMarkView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrMarkView();

    // helper stuff for the complete RecreateAllMarkHandles implementation
    void OnDemandCreate(SdrHdlList& rTarget);
    void SaveMarkHandleFocus(const SdrHdlList& rTarget);
    virtual void CreateMarkHandles(SdrHdlList& rTarget);
    void RestoreMarkHandleFocus(SdrHdlList& rTarget);
    void AdaptFocusHandleOnMove(const basegfx::B2DVector& rDistance);

public:
    // completely recreate all SdrHdl based controller objects for all views
    virtual void RecreateAllMarkHandles();

    // from time derivation; used for automatic on-demand SdrHdl creation after
    // RecreateAllMarkHandles calls; either triggered from timer or by hand from
    // GetHdlList() to ensure a new, correct SdrHdl list
    virtual void Timeout();

    void MarkGluePoints(const basegfx::B2DRange* pRange, bool bUnmark);
    virtual void MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark);
    void TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, String& rStr, sal_uInt16 nVal = 0, sal_uInt16 nOpt = 0) const;

    virtual bool IsAction() const;
    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual basegfx::B2DRange TakeActionRange() const;

    virtual void HideSdrPage();
    virtual bool IsObjMarkable(const SdrObject& rObj) const;

    // Liefert true wenn Objekte, Punkte oder Klebepunkte durch Rahmenaufziehen
    // selektiert werden (solange wie der Rahmen aufgezogen wird).
    bool IsMarking() const { return IsMarkObj() || IsMarkPoints() || IsMarkGluePoints(); }

    // Objekte durch Aufziehen eines Selektionsrahmens markieren
    bool BegMarkObj(const basegfx::B2DPoint& rPnt, bool bUnmark = false);
    void MovMarkObj(const basegfx::B2DPoint& rPnt);
    bool EndMarkObj();
    void BrkMarkObj();
    bool IsMarkObj() const { return (0 != mpMarkObjOverlay); }

    // DragModes: SDRDRAG_CREATE,SDRDRAG_MOVE,SDRDRAG_RESIZE,SDRDRAG_ROTATE,SDRDRAG_MIRROR,SDRDRAG_SHEAR,SDRDRAG_CROOK
    // Move==Resize
    // Das Interface wird hier evtl noch geaendert wg. Ortho-Drag
    void SetDragMode(SdrDragMode eMode);
    SdrDragMode GetDragMode() const { return meViewDragMode; }
    bool ChkDragMode(SdrDragMode eMode) const;
    void SetFrameHandles(bool bOn);
    bool IsFrameHandles() const { return mbForceFrameHandles; }

    virtual void SetViewEditMode(SdrViewEditMode eMode);
    SdrViewEditMode GetViewEditMode() const { return meViewEditMode; }
    bool IsEditMode() const { return SDREDITMODE_EDIT == GetViewEditMode(); }
    bool IsCreateMode() const { return SDREDITMODE_CREATE == GetViewEditMode(); }
    bool IsGluePointEditMode() const { return SDREDITMODE_GLUEPOINTEDIT == GetViewEditMode(); }

    void SetDesignMode(bool bOn = true);
    bool IsDesignMode() const { return mbDesignMode; }

    // Feststellen, ob und wo ein Objekt bzw. ob ein Referenzpunkt
    // (Rotationsmittelpunkt,Spiegelachse) getroffen wird (SW special).
    SdrHitKind PickSomething(const basegfx::B2DPoint& rPnt, double fTol = 2.0) const;
    bool HasMarkableObj() const;
    sal_uInt32 GetMarkableObjCount() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // new interface to selection, a direct wrapper to sdr::selection::Selection

    // selection const accesses
    SdrObjectVector getSelectedSdrObjectVectorFromSdrMarkView() const { return maSelection.getVector(); }
    SdrObject* getSelectedIfSingle() const { return maSelection.getSingle(); }
    bool areSdrObjectsSelected() const { return !maSelection.empty(); }
    sal_uInt32 getSelectedSdrObjectCount() const { return maSelection.size(); }
    bool isSdrObjectSelected(const SdrObject& rObject) const { return maSelection.isSdrObject(rObject); }
    sdr::selection::Indices getSelectedPointsForSelectedSdrObject(const SdrObject& rObject) const { return maSelection.getIndicesForSdrObject(rObject, true); }
    String getSelectedPointsDescription() const { return maSelection.getIndexDescription(true); }
    bool arePointsSelected() const { return maSelection.hasIndices(true); }
    sdr::selection::Indices getSelectedGluesForSelectedSdrObject(const SdrObject& rObject) const { return maSelection.getIndicesForSdrObject(rObject, false); }
    String getSelectedGluesDescription() const { return maSelection.getIndexDescription(false); }
    bool areGluesSelected() const { return maSelection.hasIndices(false); }

    // selection changers. These will trigger a delayedSelectionChanged()
    void removeSdrObjectFromSelection(const SdrObject& rObject) { maSelection.removeSdrObject(rObject); }
    void addSdrObjectToSelection(const SdrObject& rObject) { maSelection.addSdrObject(rObject); }
    void setSdrObjectSelection(const SdrObjectVector& rSdrObjectVector) { maSelection.setSdrObjects(rSdrObjectVector); }
    void clearSdrObjectSelection() { maSelection.clear(); }

    // selection changers for points
    void setSelectedPointsForSelectedSdrObject(const SdrObject& rObject, const sdr::selection::Indices& rNew) { maSelection.setIndicesForSdrObject(rObject, rNew, true); }
    void clearSelectedPointsForSelectedSdrObject(const SdrObject& rObject) { maSelection.clearIndicesForSdrObject(rObject, true); }

    // selection changers for glues
    void setSelectedGluesForSelectedSdrObject(const SdrObject& rObject, const sdr::selection::Indices& rNew) { maSelection.setIndicesForSdrObject(rObject, rNew, false); }
    void clearSelectedGluesForSelectedSdrObject(const SdrObject& rObject) { maSelection.clearIndicesForSdrObject(rObject, false); }

    // react on selection changes
    virtual void handleSelectionChange();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // mechanism to complete disable handles at the view. Handles will be hidden and deleted
    // when set, no new ones created, no interaction allowed. Handles will be recreated and shown
    // when reset. Default is false.
    bool IsMarkedObjHit(const basegfx::B2DPoint& rPnt, double fTol = 2.0) const;

    // Pick: Unterstuetzte Optionen fuer nOptions sind SEARCH_NEXT, SEARCH_BACKWARD (ni)
    SdrHdl* PickHandle(const basegfx::B2DPoint& rPnt) const;

    // Pick: Unterstuetzte Optionen fuer nOptions sind:
    // SDRSEARCH_DEEP SDRSEARCH_ALSOONMASTER SDRSEARCH_TESTMARKABLE SDRSEARCH_TESTTEXTEDIT
    // SDRSEARCH_WITHTEXT SDRSEARCH_TESTTEXTAREA SDRSEARCH_BACKWARD SDRSEARCH_MARKED
    // SDRSEARCH_WHOLEPAGE
    virtual bool PickObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject*& rpObj, sal_uInt32 nOptions, SdrObject** ppRootObj, sal_uInt32* pnMarkNum = 0, sal_uInt16* pnPassNum = 0) const;
    virtual bool PickObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject*& rpObj, sal_uInt32 nOptions = 0) const;
    bool MarkObj(const basegfx::B2DPoint& rPnt, double fTol = 2.0, bool bToggle = false, bool bDeep = false);

    // Pick: Unterstuetzte Optionen fuer nOptions sind SDRSEARCH_PASS2BOUND und SDRSEARCH_PASS3NEAREST
    bool PickMarkedObj(const basegfx::B2DPoint& rPnt, SdrObject*& rpObj, sal_uInt32* pnMarkNum = 0, sal_uInt32 nOptions = 0) const;

    // Sucht sich das Oberste der markierten Objekte (O1) und sucht von dort
    // aus in Richtung nach Unten dann das erste nichtmarkierte Objekt (O2).
    // Bei Erfolg wird die Markierung von O1 entfernt, an O2 gesetzt und true
    // returniert. Mit dem Parameter bPrev=true geht die Suche genau in die
    // andere Richtung.
    bool MarkNextObj(bool bPrev = false);

    // Sucht sich das Oberste der markierten Objekte (O1) das von rPnt/nTol
    // getroffen wird und sucht von dort aus in Richtung nach Unten dann das
    // erste nichtmarkierte Objekt (O2). Bei Erfolg wird die Markierung von
    // O1 entfernt, an O2 gesetzt und true returniert.
    // Mit dem Parameter bPrev=true geht die Suche genau in die andere Richtung.
    bool MarkNextObj(const basegfx::B2DPoint& rPnt, double fTol = 2.0, bool bPrev = false);

    // Alle Objekte innerhalb eines rechteckigen Bereichs markieren
    // Markiert werden nur Objekte, die vollstaendig eingeschlossen sind.
    bool MarkObj(const basegfx::B2DRange& rRange, bool bUnmark = false);
    void MarkObj(SdrObject& rObj, bool bUnmark = false);
    void MarkAllObj();
    void UnmarkAllObj();

    // Diese Funktion kostet etwas Zeit, da die MarkList durchsucht werden muss.
    bool IsObjMarked(const SdrObject& rObj) const;

    // Groesse der Markierungs-Handles abfragen/setzen. Angabe in Pixel.
    // Der Wert ist als Kantenlaenge zu betrachten. Gerade Werte werden
    // auf Ungerade aufgerundet: 3->3, 4->5, 5->5, 6->7, 7->7, ...
    // Defaultwert ist 7, Mindestwert 3 Pixel.
    sal_uInt16 GetMarkHdlSizePixel() const;
    void SetMarkHdlSizePixel(sal_uInt16 nSiz);

    // Die Groesse der Markierungs-Handles wird ueber die jeweilige Aufloesung
    // und die Groesse des Bereichs der markierten Objekte so angepasst, dass
    // sie sich bei einer Frame-Selektion moeglichst nicht ueberschneiden.
    // Dazu muessen die Handles ggf. verkleinert dargestellt werden. Mit der
    // MinMarkHdlSize kann man hierfuer eine Mindestgroesse angeben.
    // Defaultwert ist 3, Mindestwert 3 Pixel.
    bool IsSolidMarkHdl() const;
    void SetSolidMarkHdl(bool bOn);

    virtual bool HasMarkablePoints() const;
    virtual sal_uInt32 GetMarkablePointCount() const;
    virtual bool HasMarkedPoints() const;
    virtual sal_uInt32 GetMarkedPointCount() const;

    // Nicht alle Punkte lassen sich markieren:
    virtual bool IsPointMarkable(const SdrHdl& rHdl) const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark = false);

    /** should only be used from outside svx for special ui elements */
    bool MarkPointHelper(SdrHdl* pHdl, bool bUnmark);

    // alle Punkte innerhalb dieses Rechtecks markieren (Viewkoordinaten)
    bool IsPointMarked(const SdrHdl& rHdl) const { return &rHdl && rHdl.IsSelected(); }

    // Sucht sich den ersten markierten Punkt (P1) und sucht von dort
    // aus in den ersten nichtmarkierte Punkt (P2).
    // Bei Erfolg wird die Markierung von P1 entfernt, an P2 gesetzt und true
    // returniert. Mit dem Parameter bPrev=true geht die Suche genau in die
    // andere Richtung.
//    bool MarkNextPoint(bool bPrev = false);

    // Sucht sich den ersten markierten Punkt (P1) das von rPnt
    // getroffen wird und sucht von dort aus den
    // ersten nichtmarkierten Punkt (P2). Bei Erfolg wird die Markierung von
    // P1 entfernt, an P2 gesetzt und true returniert.
    // Mit dem Parameter bPrev=true geht die Suche genau in die andere Richtung.
//    bool MarkNextPoint(const basegfx::B2DPoint& rPnt, bool bPrev = false);

    // Die Nummer des passenden Handles raussuchen. Nicht gefunden
    // liefert CONTAINER_ENTRY_NOTFOUND.
    sal_uInt32 GetHdlNum(SdrHdl* pHdl) const;
    SdrHdl* GetHdlByIndex(sal_uInt32 nHdlNum) const;
    const SdrHdlList& GetHdlList() const;

    // Selektionsrahmen fuer Punktmarkierung aufziehen.
    // Wird nur gestartet, wenn HasMarkablePoints() true liefert.
    // bool BegMarkPoints(const Point& rPnt, OutputDevice* pOut);
    bool BegMarkPoints(const basegfx::B2DPoint& rPnt, bool bUnmark = false);
    void MovMarkPoints(const basegfx::B2DPoint& rPnt);
    bool EndMarkPoints();
    void BrkMarkPoints();
    bool IsMarkPoints() const { return (0 != mpMarkPointsOverlay); }

    // Zusatzhandles dauerhaft sichtbar schalten
    void SetPlusHandlesAlwaysVisible(bool bOn);
    bool IsPlusHandlesAlwaysVisible() const { return mbPlusHdlAlways; }

    bool HasMarkableGluePoints() const;
    sal_uInt32 GetMarkableGluePointCount() const;
    sal_uInt32 GetMarkedGluePointCount() const;

    // Ein Klebepunkt wird eindeutig identifiziert durch das SdrObject
    // (dem er zugehoert) sowie einem sal_uInt16 nId (da jedes SdrObject je
    // mehrere Klebepunkte haben kann. Hier an der View kommt zudem noch
    // eine SdrPageView, die stets korrekt gesetzt sein sollte.
    // Alternativ kann ein Klebepunkt durch ein SdrHdl bezeichnet werden.
    // Die SdrHdl-Instanz beinhaltet dann aller erforderlichen Informationen.
    // Der Klebepunkt ist in diesem Fall dann zwangslaeufig markiert (nur auf
    // markierten Klebepunkten sitzen Anfasser).
    // Achtung: Bei jeder Aenderung des Klebepunktmarkierungsstatus wird die
    // Handleliste erneut berechnet. Alle vorher gemerkten SdrHdl* sind
    // damit ungueltig, ebenso die Punkt-Id's!
    // Pick: Unterstuetzte Optionen fuer nOptions sind SEARCH_NEXT, SEARCH_BACKWARD
    bool PickGluePoint(const basegfx::B2DPoint& rPnt, SdrObject*& rpObj, sal_uInt32& rnId) const;
    bool MarkGluePoint(const SdrObject* pObj, sal_uInt32 nId, bool bUnmark = false);
    bool IsGluePointMarked(const SdrObject& rObj, sal_uInt32 nId) const;
    bool UnmarkGluePoint(const SdrHdl& rHdl);

    // Hdl eines markierten GluePoints holen. Nicht markierte
    // GluePoints haben keine Handles
    SdrHdl* GetGluePointHdl(const SdrObject* pObj, sal_uInt32 nId) const;
    bool IsGluePoint(const SdrHdl& rHdl) const { return &rHdl && HDL_GLUE == rHdl.GetKind(); }

    // Sucht sich den ersten markierten Klebepunkt (P1) und sucht von dort
    // aus in den ersten nichtmarkierte Klebepunkt (P2).
    // Bei Erfolg wird die Markierung von P1 entfernt, an P2 gesetzt und true
    // returniert. Mit dem Parameter bPrev=true geht die Suche genau in die
    // andere Richtung.
//    bool MarkNextGluePoint(bool bPrev = false);

    // Sucht sich den ersten markierten Klebepunkt (P1) das von rPnt
    // getroffen wird und sucht von dort aus den
    // ersten nichtmarkierten Klebepunkt (P2). Bei Erfolg wird die Markierung
    // von P1 entfernt, an P2 gesetzt und true returniert.
    // Mit dem Parameter bPrev=true geht die Suche genau in die andere Richtung.
//    bool MarkNextGluePoint(const basegfx::B2DPoint& rPnt, bool bPrev = false);

    // Selektionsrahmen fuer Klebepunktmarkierung aufziehen.
    // Wird nur gestartet, wenn HasMarkableGluePoints() true liefert.
    // Der GlueEditMode true wird nicht abgefragt.
    // bool BegMarkGluePoints(const Point& rPnt, OutputDevice* pOut);
    bool BegMarkGluePoints(const basegfx::B2DPoint& rPnt, bool bUnmark = false);
    void MovMarkGluePoints(const basegfx::B2DPoint& rPnt);
    bool EndMarkGluePoints();
    void BrkMarkGluePoints();
    bool IsMarkGluePoints() const { return (0L != mpMarkGluePointsOverlay); }

    const basegfx::B2DRange& getMarkedObjectSnapRange() const;
    const basegfx::B2DRange& getMarkedPointRange() const;
    const basegfx::B2DRange& getMarkedGluePointRange() const;

    // Betreten (Editieren) einer evtl markierten Objektgruppe. Sind mehrere
    // Objektgruppen markiert, so wird die Oberste genommen. Anschliessend
    // liegen alle Memberobjekte der Gruppe im direkten Zugriff. Alle anderen
    // Objekte koennen waerendessen nicht bearbeitet werden (bis zum naechsten
    // LeaveGroup()). Bei einer seitenuebergreifenden Markierung wird jede Page
    // separat abgearbeitet. Die Methode liefert true, wenn wenigstens eine
    // Gruppe betreten wurde.
    bool EnterMarkedGroup();

    // Den Mittelpunkt des letzten Crook-Dragging abholen. Den kann man
    // bei einem anschliessenden Rotate sinnvoll als Drehmittelpunkt setzen.
    const basegfx::B2DPoint& GetLastCrookCenter() const { return maLastCrookCenter; }

    // Wird automatisch von der DragView beim beenden eines Crook-Drag gesetzt.
    void SetLastCrookCenter(const basegfx::B2DPoint& rPt) { maLastCrookCenter = rPt; }

    // Rotationsmittelpunkt bzw. Startpunkt der Spiegelachse
    const basegfx::B2DPoint& GetRef1() const { return maRef1; }
    void SetRef1(const basegfx::B2DPoint& rPt);

    // Endpunkt der Spiegelachse
    const basegfx::B2DPoint& GetRef2() const { return maRef2; }
    void SetRef2(const basegfx::B2DPoint& rPt);
//IAccessibility2 Implementation 2009-----
    void UnmarkObj(SdrObject* pObj);
//-----IAccessibility2 Implementation 2009
};

#endif //_SVDMRKV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
