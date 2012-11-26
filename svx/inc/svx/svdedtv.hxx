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



#ifndef _SVDEDTV_HXX
#define _SVDEDTV_HXX

#include <svx/svdmrkv.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrUndoAction;
class SdrUndoGroup;
class SfxStyleSheet;
class SdrLayer;
class SvdProgressInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines

enum SdrHorAlign
{
    SDRHALIGN_NONE,
    SDRHALIGN_LEFT,
    SDRHALIGN_RIGHT,
    SDRHALIGN_CENTER
};

enum SdrVertAlign
{
    SDRVALIGN_NONE,
    SDRVALIGN_TOP,
    SDRVALIGN_BOTTOM,
    SDRVALIGN_CENTER
};

enum SdrMergeMode {
    SDR_MERGE_MERGE,
    SDR_MERGE_SUBSTRACT,
    SDR_MERGE_INTERSECT
};

// Optionen fuer InsertObjectToSdrObjList()
#define SDRINSERT_DONTMARK    0x0001 /* Obj wird nicht markiert (aktuelle Markierung bleibt bestehen) */
#define SDRINSERT_ADDMARK     0x0002 /* Das Obj wird zu einer ggf. bereits bestehenden Selektion hinzumarkiert */
#define SDRINSERT_SETDEFATTR  0x0004 /* Die aktuellen Attribute (+StyleSheet) werden dem Obj zugewiesen */
#define SDRINSERT_SETDEFLAYER 0x0008 /* Der aktuelle Layer wird dem Obj zugewiesen */

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrEditView: public SdrMarkView
{
private:
    SVX_DLLPRIVATE void ImpResetPossibilityFlags();

protected:

    // Die Transformationsnachfragen, etc. etwas cachen
    bool                        mbPossibilitiesDirty : 1;

    bool                        mbSelectionIsReadOnly : 1;
    bool                        mbGroupPossible : 1;
    bool                        mbUnGroupPossible : 1;
    bool                        mbGrpEnterPossible : 1;
    bool                        mbDeletePossible : 1;
    bool                        mbToTopPossible : 1;
    bool                        mbToBtmPossible : 1;
    bool                        mbReverseOrderPossible : 1;
    bool                        mbImportMtfPossible : 1;
    bool                        mbCombinePossible : 1;
    bool                        mbDismantlePossible : 1;
    bool                        mbCombineNoPolyPolyPossible : 1;
    bool                        mbDismantleMakeLinesPossible : 1;
    bool                        mbOrthoDesiredOnMarked : 1;
    bool                        mbMoreThanOneNotMovable : 1;   // Es ist mehr als ein Objekt nicht verschiebbar
    bool                        mbOneOrMoreMovable : 1;        // Wenigstens 1 Obj verschiebbar
    bool                        mbMoreThanOneNoMovRot : 1;     // Es ist mehr als ein Objekt nicht verschieb- und drehbar (Crook)
    bool                        mbContortionPossible : 1;      // Alles Polygone (ggf. gruppiert)
    bool                        mbMoveAllowedOnSelection : 1;
    bool                        mbResizeFreeAllowed : 1;
    bool                        mbResizePropAllowed : 1;
    bool                        mbRotateFreeAllowed : 1;
    bool                        mbRotate90Allowed : 1;
    bool                        mbMirrorFreeAllowed : 1;
    bool                        mbMirror45Allowed : 1;
    bool                        mbMirror90Allowed : 1;
    bool                        mbShearAllowed : 1;
    bool                        mbEdgeRadiusAllowed : 1;
    bool                        mbTransparenceAllowed : 1;
    bool                        mbGradientAllowed : 1;
    bool                        mbCanConvToPath : 1;
    bool                        mbCanConvToPoly : 1;
    bool                        mbCanConvToContour : 1;
    bool                        mbCanConvToPathLineToArea : 1;
    bool                        mbCanConvToPolyLineToArea : 1;
    bool                        mbMoveProtect : 1;
    bool                        mbResizeProtect : 1;

    // Z-Order von virtuellen Objekten zusammenhalten (Writer)
    bool                        mbBundleVirtObj : 1;

    void ImpBroadcastEdgesOfMarkedNodes();

    // Konvertierung der markierten Objekte in Poly bzw. Bezier.
    void ImpConvertTo(bool bPath, bool bLineToArea);

    // Konvertiert ein Obj, wirft bei Erfolg das alte as seiner Liste und
    // fuegt das neue an dessen Position ein. Inkl Undo. Es wird weder ein
    // MarkEntry noch ein ModelChgBroadcast generiert.
    SdrObject* ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea);

    // Setzen der beiden Flags mbToTopPossible und mbToBtmPossible.
    // bToTopPossibleDirty und bToBtmPossibleDirty werden dabei gleichzeitig
    // zurueckgesetzt.
    void ImpCheckToTopBtmPossible();

    // fuer CombineMarkedObjects und DismantleMarkedObjects
    void ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const;

    // fuer CombineMarkedObjects
    bool ImpCanConvertForCombine1(const SdrObject* pObj) const;
    bool ImpCanConvertForCombine(const SdrObject* pObj) const;
    basegfx::B2DPolyPolygon ImpGetPolyPolygon1(const SdrObject* pObj, bool bCombine) const;
    basegfx::B2DPolyPolygon ImpGetPolyPolygon(const SdrObject* pObj, bool bCombine) const;
    basegfx::B2DPolygon ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon) const;

    // fuer DismantleMarkedObjects
    bool ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPpolygon, bool bMakeLines) const;
    bool ImpCanDismantle(const SdrObject* pObj, bool bMakeLines) const;
    void ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uInt32& rPos, bool bMakeLines);
    bool ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const;
    void ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID);

    // Entfernt alle Obj der MarkList aus ihren ObjLists inkl Undo.
    // Die Eintraege in rMark bleiben erhalten.
    void deleteSdrObjectsWithUndo(const SdrObjectVector& rSdrObjectVector);

    // Die Transformationsnachfragen etwas cachen
    //void ImpCheckMarkTransform() const; veraltet
    // Checken, was man so mit den markierten Objekten alles machen kann
    virtual void CheckPossibilities();
    void ForcePossibilities() const;

    virtual void ImpCheckPolyPossibilities();

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrEditView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrEditView();

public:
    void ImpCrookObj(SdrObject& rO, const basegfx::B2DPoint& rRefPoint, const basegfx::B2DPoint& rRad, SdrCrookMode eMode,
        bool bVertical, bool bNoContortion, bool bRotate, const basegfx::B2DRange& rMarkRange);
    void ImpDistortObj(SdrObject& rO, const basegfx::B2DRange& rRefRange, const basegfx::B2DPolygon& rDistortedRect, bool bNoContortion);

    // Jeder Aufruf einer undofaehigen Methode an der View generiert eine
    // UndoAction. Moechte man mehrere
    // Methodenaufrufe zu einer UndoAction zusammenfassen, so kann man diese
    // mit BegUndo() / EndUndo() klammern (beliebig tief). Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. NotifyNewUndoAction() wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. NotifyNewUndoAction() wird nicht gerufen bei einer
    // leeren Klammerung.
    void BegUndo()                       { getSdrModelFromSdrView().BegUndo();         } // Undo-Klammerung auf
    void BegUndo(const String& rComment) { getSdrModelFromSdrView().BegUndo(rComment); } // Undo-Klammerung auf
    void BegUndo(const String& rComment, const String& rObjDescr, SdrRepeatFunc eFunc=SDRREPFUNC_OBJ_NONE) { getSdrModelFromSdrView().BegUndo(rComment,rObjDescr,eFunc); } // Undo-Klammerung auf
    void BegUndo(SdrUndoGroup* pUndoGrp) { getSdrModelFromSdrView().BegUndo(pUndoGrp); } // Undo-Klammerung auf
    void EndUndo();                                                   // Undo-Klammerung zu (inkl BroadcastEdges)
    void AddUndo(SdrUndoAction* pUndo)   { getSdrModelFromSdrView().AddUndo(pUndo);    } // Action hinzufuegen
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
    void SetUndoComment(const String& rComment) { getSdrModelFromSdrView().SetUndoComment(rComment); }
    void SetUndoComment(const String& rComment, const String& rObjDescr) { getSdrModelFromSdrView().SetUndoComment(rComment,rObjDescr); }
    bool IsUndoEnabled() const { return getSdrModelFromSdrView().IsUndoEnabled(); }

    std::vector< SdrUndoAction* > CreateConnectorUndo( SdrObject& rO );
    void AddUndoActions( std::vector< SdrUndoAction* >& );

    // Layerverwaltung. Mit Undo.
    SdrLayer* InsertNewLayer(const String& rName, sal_uInt16 nPos=0xFFFF);
    // Loeschen eines Layer inkl. aller darauf befindlichen Objekte
    void      DeleteLayer(const String& rName);

    // Markierte Objekte die ausserhalb ihrer Page liegen
    // werden ggf. einer anderen Page zugewiesen
    // z.Zt. noch ohne Undo!!!
//  void ForceMarkedObjToAnotherPage();

    bool IsReadOnly() const { ForcePossibilities(); return mbSelectionIsReadOnly; }

    // Loeschen aller markierten Objekte
    void DeleteMarkedObj();
    bool IsDeleteMarkedObjPossible() const { ForcePossibilities(); return mbDeletePossible; }

    // Logisch- umschliessendes Rect aller markierten Objekte setzen.
    // Das das wirklich geschieht ist nicht garantiert, denn eine
    // waagerechte Linie hat z.B. immer eine Hoehe von 0.
    void SetMarkedObjSnapRange(const basegfx::B2DRange& rRange, bool bCopy = false);
    void MoveMarkedObj(const basegfx::B2DVector& rOffset, bool bCopy = false);
    void ResizeMarkedObj(const basegfx::B2DPoint& rRefPoint, const basegfx::B2DTuple& rScale, bool bCopy = false);
    double GetMarkedObjRotate() const;
    void RotateMarkedObj(const basegfx::B2DPoint& rRefPoint, double fAngle, bool bCopy = false);
    void MirrorMarkedObj(const basegfx::B2DPoint& rRefPoint1, const basegfx::B2DPoint& rRefPoint2, bool bCopy = false);
    void MirrorMarkedObjHorizontal(bool bCopy = false);
    void MirrorMarkedObjVertical(bool bCopy = false);
    double GetMarkedObjShearX() const;
    void ShearMarkedObj(const basegfx::B2DPoint& rRefPoint, double fAngle, bool bVShear = false, bool bCopy = false);
    void CrookMarkedObj(const basegfx::B2DPoint& rRefPoint, const basegfx::B2DPoint& rRad, SdrCrookMode eMode, bool bVertical = false, bool bNoContortion = false, bool bCopy = false);
    void DistortMarkedObj(const basegfx::B2DRange& rRefRange, const basegfx::B2DPolygon& rDistortedRect, bool bNoContortion = false, bool bCopy = false);

    // Markierte Objekte kopieren und anstelle der alten markieren
    void CopyMarkedObj();
    bool IsMoveAllowed() const { ForcePossibilities(); return mbMoveAllowedOnSelection && !mbMoveProtect; }
    bool IsResizeAllowed(bool bProp=false) const;
    bool IsRotateAllowed(bool b90Deg=false) const;
    bool IsMirrorAllowed(bool b45Deg=false, bool b90Deg=false) const;
    bool IsTransparenceAllowed() const;
    bool IsGradientAllowed() const;
    bool IsShearAllowed() const;
    bool IsEdgeRadiusAllowed() const;
    bool IsCrookAllowed(bool bNoContortion=false) const;
    bool IsDistortAllowed(bool bNoContortion=false) const;

    // Vereinigen mehrerer Objekte zu einem PolyPolygon:
    // - Rechtecke/Kreise/Text... werden implizit gewandelt.
    // - Polylines werden automatisch geschlossen.
    // - Die Attribute und der Layer werden vom Ersten der markierten Objekte
    //   uebernommen (also vom untersten der Z-Order).
    // - Gruppenobjekte werden miteinbezogen, wenn alle! Memberobjekte der
    //   Gruppe wandelbar sind. Beinhaltet eine Gruppe also beispielsweise
    //   eine Bitmap oder ein OLE-Objekt, wird die gesamte Gruppe nicht
    //   beruecksichtigt.
    // bNoPolyPoly=true: Alles wird zu einem einzigen Polygon zusammengefasst
    void CombineMarkedObjects(bool bNoPolyPoly = true);

    // for combining multiple polygons, with direct support of the modes
    // SID_POLY_MERGE, SID_POLY_SUBSTRACT, SID_POLY_INTERSECT
    void MergeMarkedObjects(SdrMergeMode eMode);

    // for distribution dialog function
    void DistributeMarkedObjects();

    // Markierte Polypolygonobjekte in Polygone zerlegen
    // Gruppenobjekte werden durchsucht und zerlegt, wenn es sich bei allen
    // Memberobjekten um PathObjs handelt.
    // bMakeLines=true: alle Polygone werden in einzelne Linien bzw.
    //                  Beziersegmente zerlegt
    void DismantleMarkedObjects(bool bMakeLines=false);
    bool IsCombinePossible(bool bNoPolyPoly=false) const;
    bool IsDismantlePossible(bool bMakeLines=false) const;

    // Ein neues bereits fertig konstruiertes Obj einfuegen. Das Obj gehoert
    // anschliessend dem Model. Nach dem Einfuegen wird das neue Objekt
    // markiert (wenn dies nicht via nOptions unterbunden wird).
    // U.U. wird das Obj jedoch nicht eingefuegt, sondern deleted, naemlich
    // wenn der Ziel-Layer gesperrt oder nicht sichtbar ist. In diesem Fall
    // returniert die Methode mit false.
    // Die Methode generiert u.a. auch eine Undo-Action.
    bool InsertObjectAtView(SdrObject& rObj, sal_uInt32 nOptions = 0);

    // Ein Zeichenobjekt durch ein neues ersetzen. *pNewObj gehoert
    // anschliessend mir, *pOldObj wandert ins Undo.
    // Sollte in jedem Fall mit einer Undo-Klammerung versehen werden, z.B.:
    // aStr+=" ersetzen";
    // BegUndo(aStr);
    // ReplaceObject(...);
    // ...
    // EndUndo();
    void ReplaceObjectAtView(SdrObject& rOldObj, SdrObject& rNewObj, bool bMark = true);

    void SetNotPersistAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll);
    void MergeNotPersistAttrFromMarked(SfxItemSet& rAttr) const;
    void MergeAttrFromMarked(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
    SfxItemSet GetAttrFromMarked(bool bOnlyHardAttr) const;
    void SetAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll);

    // Geometrische Attribute (Position, Groesse, Drehwinkel)
    // Bei der Position wird ein evtl. gesetzter PageOrigin beruecksichtigt.
    SfxItemSet GetGeoAttrFromMarked() const;
    void SetGeoAttrToMarked(const SfxItemSet& rAttr);

    // Returnt NULL wenn:
    // - Nix markiert,
    // - kein StyleSheet an den markierten Objekten gesetzt
    // - Bei Mehrfachselektion die markierten Objekte auf unterschiedliche
    //   StyleSheets verweisen.
    SfxStyleSheet* GetStyleSheetFromMarked() const;

    // z.Zt. noch ohne Undo :(
    void SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    // Alle markierten Objekte zu einer Gruppe zusammenfassen.
    // Anschliessend wird die neue Gruppe markiert. Bei einer
    // seitenuebergreifenden Markierung wird eine Gruppe je Seite erzeugt.
    // Alle erzeugten Gruppen sind anschliessend markiert.
    // Ueber pUserGrp kann ein eigenes Gruppenobjekt vorgegeben werden. Dieses
    // wird  jedoch nicht direkt verwendet, sondern via Clone kopiert.
    // Wird NULL uebergeben, macht sich die Methode SdrObjGroup-Instanzen.
    void GroupMarked(const SdrObject* pUserGrp=NULL);

    // Alle markierten Objektgruppen werden aufgeloesst (1 Level).
    // Anschliessend sind statt der Gruppenobjekte alle ehemaligen
    // Memberobjekte der aufgeloesten Gruppen markiert. Waren zum auch Objekte
    // markiert, die keine Gruppenobjekte sind, so bleiben diese weiterhin
    // zusaetzlich markiert.
    void UnGroupMarked();

    bool IsGroupPossible() const { ForcePossibilities(); return mbGroupPossible; }
    bool IsUnGroupPossible() const { ForcePossibilities(); return mbUnGroupPossible; }
    bool IsGroupEnterPossible() const { ForcePossibilities(); return mbGrpEnterPossible; }

    // Markierte Objekte in Polygone/Bezierkurven verwandeln. Die bool-
    // Funktionen returnen true, wenn wenigstens eins der markierten
    // Objekte gewandelt werden kann. Memberobjekte von Gruppenobjekten
    // werden ebenfalls gewandelt. Naehere Beschreibung siehe SdrObj.HXX.
    bool IsConvertToPathObjPossible(bool bLineToArea) const { ForcePossibilities(); return bool(bLineToArea ? mbCanConvToPathLineToArea : mbCanConvToPath); }
    bool IsConvertToPolyObjPossible(bool bLineToArea) const { ForcePossibilities(); return bool(bLineToArea ? mbCanConvToPolyLineToArea : mbCanConvToPoly); }
    bool IsConvertToContourPossible() const { ForcePossibilities(); return mbCanConvToContour; }
    void ConvertMarkedToPathObj(bool bLineToArea);
    void ConvertMarkedToPolyObj(bool bLineToArea);

    // Alle markierten Objekte untereinander ausrichten. Normalerweise werden
    // das SnapRect der Obj verwendet. Ist bBoundRects=true, werden stattdessen
    // die BoundRects ausgerichtet.
    void AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert, bool bBoundRects=false);
    bool IsAlignPossible() const;

    // Markierte Objekte etwas nach "oben" holen
    void MovMarkedToTop();

    // Markierte Objekte etwas nach "unten" holen
    void MovMarkedToBtm();

    // Markierte Objekte ganz nach "oben" stellen
    void PutMarkedToTop();

    // Markierte Objekte ganz nach "unten" stellen
    void PutMarkedToBtm();

    // Markierte direkt vor das uebergebene Objekt stellen
    // NULL -> wie PutMarkedToTop();
    void PutMarkedInFrontOfObj(const SdrObject* pRefObj);

    // Markierte direkt hinter das uebergebene Objekt stellen
    // NULL -> wie PutMarkedToBtm();
    void PutMarkedBehindObj(const SdrObject* pRefObj);

    // Z-Order der markierten Objekte vertauschen
    void ReverseOrderOfMarked();

    // Feststellen, ob nach vorn/hinten stellen moeglich ist
    // GetMaxToTop/BtmObj() wird von diesen Methoden nur begrenzt
    // beruecksichtigt, d.h. es kann vorkommen dass IsToTopPossible()
    // true liefert, MovMarkedToTop() jedoch nichts aendert (z.B. bei
    // Mehrfachselektion), weil eine von der abgeleiteten View ueber
    // GetMaxToTopObj() auferlegte Restriktion dies verhindert.
    bool IsToTopPossible() const { ForcePossibilities(); return mbToTopPossible; }
    bool IsToBtmPossible() const { ForcePossibilities(); return mbToBtmPossible; }
    bool IsReverseOrderPossible() const { ForcePossibilities(); return mbReverseOrderPossible; }

    // Ueber diese Methoden stellt die View fest, wie weit ein Objekt
    // nach vorn bzw. nach hinten gestellt werden darf (Z-Order). Das
    // zurueckgegebene Objekt wird dann nicht "ueberholt". Bei Rueckgabe
    // von NULL (Defaultverhalten) bestehen keine Restriktionen.
    virtual SdrObject* GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject* GetMaxToBtmObj(SdrObject* pObj) const;

    // Folgende Methode wird gerufen, wenn z.B. durch ToTop, ToBtm, ... die
    // Reihenfolgen der Objekte geaendert wurde. Der Aufruf erfolgt dann nach
    // jedem SdrObjList::SetNavigationPosition(nOldPos,nNewPos);
    virtual void ObjOrderChanged(SdrObject* pObj, sal_uInt32 nOldPos, sal_uInt32 nNewPos);

    // Falls ein oder mehrere Objekte des Types SdrGrafObj oder SdrOle2Obj
    // markiert sind und diese in der Lage sind ein StarView-Metafile zu
    // liefern, konvertiert diese Methode das Metafile in Drawingobjekte.
    // Die SdrGrafObjs/SdrOle2Objs werden dann durch die neue Objekte ersetzt.
    void DoImportMarkedMtf(SvdProgressInfo *pProgrInfo=NULL);
    bool IsImportMtfPossible() const { ForcePossibilities(); return mbImportMtfPossible; }

    // Wird der Modus VirtualObjectBundling eingeschaltet, werden beim
    // ToTop/ToBtm virtuelle Objekte die dasselbe Objekt referenzieren
    // in ihrer Z-Order buendig zusammengehalten (Writer).
    // Defaulteinstellung ist false=ausgeschaltet.
    void SetVirtualObjectBundling(bool bOn) { if(mbBundleVirtObj != bOn) mbBundleVirtObj = bOn; }
    bool IsVirtualObjectBundling() const { return mbBundleVirtObj; }

    // von der SdrMarkView ueberladen fuer den internen gebrauch
    virtual void handleSelectionChange();
};

#endif //_SVDEDTV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
