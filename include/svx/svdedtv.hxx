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

#ifndef INCLUDED_SVX_SVDEDTV_HXX
#define INCLUDED_SVX_SVDEDTV_HXX

#include <svx/svdmrkv.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svxdllapi.h>

class SfxUndoAction;
class SdrUndoAction;
class SdrUndoGroup;
class SfxStyleSheet;
class SdrLayer;
class SvdProgressInfo;

enum SdrHorAlign  {
    SDRHALIGN_NONE,
    SDRHALIGN_LEFT,
    SDRHALIGN_RIGHT,
    SDRHALIGN_CENTER
};

enum SdrVertAlign {
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

// Optionen fuer InsertObject()
#define SDRINSERT_DONTMARK    0x0001 /* Obj wird nicht markiert (aktuelle Markierung bleibt bestehen) */
#define SDRINSERT_ADDMARK     0x0002 /* Das Obj wird zu einer ggf. bereits bestehenden Selektion hinzumarkiert */
#define SDRINSERT_SETDEFATTR  0x0004 /* Die aktuellen Attribute (+StyleSheet) werden dem Obj zugewiesen */
#define SDRINSERT_SETDEFLAYER 0x0008 /* Der aktuelle Layer wird dem Obj zugewiesen */
#define SDRINSERT_NOBROADCAST 0x0010 /* Einfuegen mit NbcInsertObject() fuer SolidDragging */

class SVX_DLLPUBLIC SdrEditView: public SdrMarkView
{
    friend class                SdrPageView;
    friend class                SdrDragDistort;
    friend class                SdrDragCrook;

protected:

    // Die Transformationsnachfragen, etc. etwas cachen
    bool                        bPossibilitiesDirty : 1;
    bool                        bReadOnly : 1;
    bool                        bGroupPossible : 1;
    bool                        bUnGroupPossible : 1;
    bool                        bGrpEnterPossible : 1;
    bool                        bDeletePossible : 1;
    bool                        bToTopPossible : 1;
    bool                        bToBtmPossible : 1;
    bool                        bReverseOrderPossible : 1;
    bool                        bImportMtfPossible : 1;
    bool                        bCombinePossible : 1;
    bool                        bDismantlePossible : 1;
    bool                        bCombineNoPolyPolyPossible : 1;
    bool                        bDismantleMakeLinesPossible : 1;
    bool                        bOrthoDesiredOnMarked : 1;
    bool                        bMoreThanOneNotMovable : 1;   // Es ist mehr als ein Objekt nicht verschiebbar
    bool                        bOneOrMoreMovable : 1;        // Wenigstens 1 Obj verschiebbar
    bool                        bMoreThanOneNoMovRot : 1;     // Es ist mehr als ein Objekt nicht verschieb- und drehbar (Crook)
    bool                        bContortionPossible : 1;      // Alles Polygone (ggf. gruppiert)
    bool                        bAllPolys : 1;                // Alles Polygone (nicht gruppiert)
    bool                        bOneOrMorePolys : 1;          // Mindestens 1 Polygon (nicht gruppiert)
    bool                        bMoveAllowed : 1;
    bool                        bResizeFreeAllowed : 1;
    bool                        bResizePropAllowed : 1;
    bool                        bRotateFreeAllowed : 1;
    bool                        bRotate90Allowed : 1;
    bool                        bMirrorFreeAllowed : 1;
    bool                        bMirror45Allowed : 1;
    bool                        bMirror90Allowed : 1;
    bool                        bShearAllowed : 1;
    bool                        bEdgeRadiusAllowed : 1;
    bool                        bTransparenceAllowed : 1;
    bool                        bGradientAllowed : 1;
    bool                        bCanConvToPath : 1;
    bool                        bCanConvToPoly : 1;
    bool                        bCanConvToContour : 1;
    bool                        bCanConvToPathLineToArea : 1;
    bool                        bCanConvToPolyLineToArea : 1;
    bool                        bMoveProtect : 1;
    bool                        bResizeProtect : 1;
    // Z-Order von virtuellen Objekten zusammenhalten (Writer)
    bool                        bBundleVirtObj : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpResetPossibilityFlags();

protected:
    void ImpBroadcastEdgesOfMarkedNodes();

    // Konvertierung der markierten Objekte in Poly bzw. Bezier.
    void ImpConvertTo(bool bPath, bool bLineToArea);

    // Konvertiert ein Obj, wirft bei Erfolg das alte as seiner Liste und
    // fuegt das neue an dessen Position ein. Inkl Undo. Es wird weder ein
    // MarkEntry noch ein ModelChgBroadcast generiert.
    SdrObject* ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea);

    // Setzen der beiden Flags bToTopPossible und bToBtmPossible.
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
    void ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uIntPtr& rPos, SdrPageView* pPV, bool bMakeLines);
    void ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad, SdrCrookMode eMode,
        bool bVertical, bool bNoContortion, bool bRotate, const Rectangle& rMarkRect);
    void ImpDistortObj(SdrObject* pO, const Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion);
    bool ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const;
    void ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID);

    // Entfernt alle Obj der MarkList aus ihren ObjLists inkl Undo.
    // Die Eintraege in rMark bleiben erhalten.
    void DeleteMarkedList(const SdrMarkList& rMark); // DeleteMarked -> DeleteMarkedList

    // Die Transformationsnachfragen etwas cachen
    //void ImpCheckMarkTransform() const; veraltet
    // Checken, was man so mit den markierten Objekten alles machen kann
    virtual void CheckPossibilities();
    void ForcePossibilities() const { if (bPossibilitiesDirty || bSomeObjChgdFlag) ((SdrEditView*)this)->CheckPossibilities(); }

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrEditView();

public:
    // Jeder Aufruf einer undofaehigen Methode an der View generiert eine
    // UndoAction. Moechte man mehrere
    // Methodenaufrufe zu einer UndoAction zusammenfassen, so kann man diese
    // mit BegUndo() / EndUndo() klammern (beliebig tief). Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. NotifyNewUndoAction() wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. NotifyNewUndoAction() wird nicht gerufen bei einer
    // leeren Klammerung.
    void BegUndo()                       { pMod->BegUndo();         } // Undo-Klammerung auf
    void BegUndo(const OUString& rComment) { pMod->BegUndo(rComment); } // Undo-Klammerung auf
    void BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc=SDRREPFUNC_OBJ_NONE) { pMod->BegUndo(rComment,rObjDescr,eFunc); } // Undo-Klammerung auf
    void EndUndo();                                                   // Undo-Klammerung zu (inkl BroadcastEdges)
    void AddUndo(SdrUndoAction* pUndo)   { pMod->AddUndo(pUndo);    } // Action hinzufuegen
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
    void SetUndoComment(const OUString& rComment) { pMod->SetUndoComment(rComment); }
    void SetUndoComment(const OUString& rComment, const OUString& rObjDescr) { pMod->SetUndoComment(rComment,rObjDescr); }
    bool IsUndoEnabled() const;

    std::vector< SdrUndoAction* > CreateConnectorUndo( SdrObject& rO );
    void AddUndoActions( std::vector< SdrUndoAction* >& );

    // Layerverwaltung. Mit Undo.
    SdrLayer* InsertNewLayer(const OUString& rName, sal_uInt16 nPos=0xFFFF);
    // Loeschen eines Layer inkl. aller darauf befindlichen Objekte
    void      DeleteLayer(const OUString& rName);
    // Verschieben eines Layer (Layerreihenfolge aendern)
    void      MoveLayer(const OUString& rName, sal_uInt16 nNewPos);

    // Markierte Objekte die ausserhalb ihrer Page liegen
    // werden ggf. einer anderen Page zugewiesen
    // z.Zt. noch ohne Undo!!!
    void ForceMarkedObjToAnotherPage();
    void ForceMarkedToAnotherPage()   { ForceMarkedObjToAnotherPage(); }

    bool IsReadOnly() const { ForcePossibilities(); return bReadOnly; }

    // Loeschen aller markierten Objekte
    void DeleteMarkedObj();
    bool IsDeleteMarkedObjPossible() const { ForcePossibilities(); return bDeletePossible; }

    // Logisch- umschliessendes Rect aller markierten Objekte setzen.
    // Das das wirklich geschieht ist nicht garantiert, denn eine
    // waagerechte Linie hat z.B. immer eine Hoehe von 0.
    void SetMarkedObjRect(const Rectangle& rRect, bool bCopy=false);
    void MoveMarkedObj(const Size& rSiz, bool bCopy=false);
    void ResizeMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void ResizeMultMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, const bool bCopy, const bool bWdh, const bool bHgt);
    long GetMarkedObjRotate() const;
    void RotateMarkedObj(const Point& rRef, long nWink, bool bCopy=false);
    void MirrorMarkedObj(const Point& rRef1, const Point& rRef2, bool bCopy=false);
    void MirrorMarkedObjHorizontal(bool bCopy=false);
    void MirrorMarkedObjVertical(bool bCopy=false);
    long GetMarkedObjShear() const;
    void ShearMarkedObj(const Point& rRef, long nWink, bool bVShear=false, bool bCopy=false);
    void CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical=false, bool bNoContortion=false, bool bCopy=false);
    void DistortMarkedObj(const Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion=false, bool bCopy=false);

    // Markierte Objekte kopieren und anstelle der alten markieren
    void CopyMarkedObj();
    void SetAllMarkedRect(const Rectangle& rRect, bool bCopy=false) { SetMarkedObjRect(rRect,bCopy); }
    void MoveAllMarked(const Size& rSiz, bool bCopy=false) { MoveMarkedObj   (rSiz,bCopy); }
    void ResizeAllMarked(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false) { ResizeMarkedObj (rRef,xFact,yFact,bCopy); }
    long GetAllMarkedRotate() const { return GetMarkedObjRotate(); }
    void RotateAllMarked(const Point& rRef, long nWink, bool bCopy=false) { RotateMarkedObj(rRef,nWink,bCopy); }
    void MirrorAllMarked(const Point& rRef1, const Point& rRef2, bool bCopy=false) { MirrorMarkedObj(rRef1,rRef2,bCopy); }
    void MirrorAllMarkedHorizontal(bool bCopy=false) { MirrorMarkedObjHorizontal(bCopy); }
    void MirrorAllMarkedVertical(bool bCopy=false) { MirrorMarkedObjVertical(bCopy); }
    long GetAllMarkedShear() const { return GetMarkedObjShear(); }
    void ShearAllMarked(const Point& rRef, long nWink, bool bVShear=false, bool bCopy=false) { ShearMarkedObj(rRef,nWink,bVShear,bCopy); }
    void CrookAllMarked(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical=false, bool bNoContortion=false, bool bCopy=false) { CrookMarkedObj(rRef,rRad,eMode,bVertical,bNoContortion,bCopy); }
    void CopyMarked() { CopyMarkedObj(); }
    bool IsMoveAllowed() const { ForcePossibilities(); return bMoveAllowed && !bMoveProtect; }
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
    // bNoPolyPoly=TRUE: Alles wird zu einem einzigen Polygon zusammengefasst
    void CombineMarkedObjects(bool bNoPolyPoly = true);

    // for combining multiple polygons, with direct support of the modes
    // SID_POLY_MERGE, SID_POLY_SUBSTRACT, SID_POLY_INTERSECT
    void MergeMarkedObjects(SdrMergeMode eMode);

    // for distribution dialog function
    void DistributeMarkedObjects();

    // Markierte Polypolygonobjekte in Polygone zerlegen
    // Gruppenobjekte werden durchsucht und zerlegt, wenn es sich bei allen
    // Memberobjekten um PathObjs handelt.
    // bMakeLines=TRUE: alle Polygone werden in einzelne Linien bzw.
    //                  Beziersegmente zerlegt
    void DismantleMarkedObjects(bool bMakeLines=false);
    bool IsCombinePossible(bool bNoPolyPoly=false) const;
    bool IsDismantlePossible(bool bMakeLines=false) const;

    // Ein neues bereits fertig konstruiertes Obj einfuegen. Das Obj gehoert
    // anschliessend dem Model. Nach dem Einfuegen wird das neue Objekt
    // markiert (wenn dies nicht via nOptions unterbunden wird).
    // U.U. wird das Obj jedoch nicht eingefuegt, sondern deleted, naemlich
    // wenn der Ziel-Layer gesperrt oder nicht sichtbar ist. In diesem Fall
    // returniert die Methode mit FALSE.
    // Die Methode generiert u.a. auch eine Undo-Action.
    bool InsertObjectAtView(SdrObject* pObj, SdrPageView& rPV, sal_uIntPtr nOptions=0);

    // Ein Zeichenobjekt durch ein neues ersetzen. *pNewObj gehoert
    // anschliessend mir, *pOldObj wandert ins Undo.
    // Sollte in jedem Fall mit einer Undo-Klammerung versehen werden, z.B.:
    // aStr+=" ersetzen";
    // BegUndo(aStr);
    // ReplaceObject(...);

    // EndUndo();
    void ReplaceObjectAtView(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, bool bMark=true);

    void SetNotPersistAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll);
    void MergeNotPersistAttrFromMarked(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
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

    bool IsGroupPossible() const { ForcePossibilities(); return bGroupPossible; }
    bool IsUnGroupPossible() const { ForcePossibilities(); return bUnGroupPossible; }
    bool IsGroupEnterPossible() const { ForcePossibilities(); return bGrpEnterPossible; }

    // Markierte Objekte in Polygone/Bezierkurven verwandeln. Die bool-
    // Funktionen returnen sal_True, wenn wenigstens eins der markierten
    // Objekte gewandelt werden kann. Memberobjekte von Gruppenobjekten
    // werden ebenfalls gewandelt. Naehere Beschreibung siehe SdrObj.HXX.
    bool IsConvertToPathObjPossible(bool bLineToArea) const { ForcePossibilities(); return bLineToArea ? bCanConvToPathLineToArea : bCanConvToPath; }
    bool IsConvertToPolyObjPossible(bool bLineToArea) const { ForcePossibilities(); return bLineToArea ? bCanConvToPolyLineToArea : bCanConvToPoly; }
    bool IsConvertToContourPossible() const { ForcePossibilities(); return bCanConvToContour; }
    void ConvertMarkedToPathObj(bool bLineToArea);
    void ConvertMarkedToPolyObj(bool bLineToArea);

    // Alle markierten Objekte untereinander ausrichten. Normalerweise werden
    // das SnapRect der Obj verwendet. Ist bBoundRects=sal_True, werden stattdessen
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
    // sal_True liefert, MovMarkedToTop() jedoch nichts aendert (z.B. bei
    // Mehrfachselektion), weil eine von der abgeleiteten View ueber
    // GetMaxToTopObj() auferlegte Restriktion dies verhindert.
    bool IsToTopPossible() const { ForcePossibilities(); return bToTopPossible; }
    bool IsToBtmPossible() const { ForcePossibilities(); return bToBtmPossible; }
    bool IsReverseOrderPossible() const { ForcePossibilities(); return bReverseOrderPossible; }

    // Ueber diese Methoden stellt die View fest, wie weit ein Objekt
    // nach vorn bzw. nach hinten gestellt werden darf (Z-Order). Das
    // zurueckgegebene Objekt wird dann nicht "ueberholt". Bei Rueckgabe
    // von NULL (Defaultverhalten) bestehen keine Restriktionen.
    virtual SdrObject* GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject* GetMaxToBtmObj(SdrObject* pObj) const;

    // Folgende Methode wird gerufen, wenn z.B. durch ToTop, ToBtm, ... die
    // Reihenfolgen der Objekte geaendert wurde. Der Aufruf erfolgt dann nach
    // jedem SdrObjList::SetObjectOrdNum(nOldPos,nNewPos);
    virtual void ObjOrderChanged(SdrObject* pObj, sal_uIntPtr nOldPos, sal_uIntPtr nNewPos);

    // Falls ein oder mehrere Objekte des Types SdrGrafObj oder SdrOle2Obj
    // markiert sind und diese in der Lage sind ein StarView-Metafile zu
    // liefern, konvertiert diese Methode das Metafile in Drawingobjekte.
    // Die SdrGrafObjs/SdrOle2Objs werden dann durch die neue Objekte ersetzt.
    void DoImportMarkedMtf(SvdProgressInfo *pProgrInfo=NULL);
    bool IsImportMtfPossible() const { ForcePossibilities(); return bImportMtfPossible; }

    // Wird der Modus VirtualObjectBundling eingeschaltet, werden beim
    // ToTop/ToBtm virtuelle Objekte die dasselbe Objekt referenzieren
    // in ihrer Z-Order buendig zusammengehalten (Writer).
    // Defaulteinstellung ist sal_False=ausgeschaltet.
    void SetVirtualObjectBundling(bool bOn) { bBundleVirtObj=bOn; }
    bool IsVirtualObjectBundling() const { return bBundleVirtObj; }

    // von der SdrMarkView ueberladen fuer den internen gebrauch
    virtual void MarkListHasChanged() SAL_OVERRIDE;
    virtual void ModelHasChanged() SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SVDEDTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
