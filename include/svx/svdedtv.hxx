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

#ifndef _SVDEDTV_HXX
#define _SVDEDTV_HXX

#include <svx/svdmrkv.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svxdllapi.h"

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
    unsigned                    bPossibilitiesDirty : 1;
    unsigned                    bReadOnly : 1;
    unsigned                    bGroupPossible : 1;
    unsigned                    bUnGroupPossible : 1;
    unsigned                    bGrpEnterPossible : 1;
    unsigned                    bDeletePossible : 1;
    unsigned                    bToTopPossible : 1;
    unsigned                    bToBtmPossible : 1;
    unsigned                    bReverseOrderPossible : 1;
    unsigned                    bImportMtfPossible : 1;
    unsigned                    bCombinePossible : 1;
    unsigned                    bDismantlePossible : 1;
    unsigned                    bCombineNoPolyPolyPossible : 1;
    unsigned                    bDismantleMakeLinesPossible : 1;
    unsigned                    bOrthoDesiredOnMarked : 1;
    unsigned                    bMoreThanOneNotMovable : 1;   // Es ist mehr als ein Objekt nicht verschiebbar
    unsigned                    bOneOrMoreMovable : 1;        // Wenigstens 1 Obj verschiebbar
    unsigned                    bMoreThanOneNoMovRot : 1;     // Es ist mehr als ein Objekt nicht verschieb- und drehbar (Crook)
    unsigned                    bContortionPossible : 1;      // Alles Polygone (ggf. gruppiert)
    unsigned                    bAllPolys : 1;                // Alles Polygone (nicht gruppiert)
    unsigned                    bOneOrMorePolys : 1;          // Mindestens 1 Polygon (nicht gruppiert)
    unsigned                    bMoveAllowed : 1;
    unsigned                    bResizeFreeAllowed : 1;
    unsigned                    bResizePropAllowed : 1;
    unsigned                    bRotateFreeAllowed : 1;
    unsigned                    bRotate90Allowed : 1;
    unsigned                    bMirrorFreeAllowed : 1;
    unsigned                    bMirror45Allowed : 1;
    unsigned                    bMirror90Allowed : 1;
    unsigned                    bShearAllowed : 1;
    unsigned                    bEdgeRadiusAllowed : 1;
    unsigned                    bTransparenceAllowed : 1;
    unsigned                    bGradientAllowed : 1;
    unsigned                    bCanConvToPath : 1;
    unsigned                    bCanConvToPoly : 1;
    unsigned                    bCanConvToContour : 1;
    unsigned                    bCanConvToPathLineToArea : 1;
    unsigned                    bCanConvToPolyLineToArea : 1;
    unsigned                    bMoveProtect : 1;
    unsigned                    bResizeProtect : 1;
    // Z-Order von virtuellen Objekten zusammenhalten (Writer)
    unsigned                    bBundleVirtObj : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpResetPossibilityFlags();

protected:
    void ImpBroadcastEdgesOfMarkedNodes();

    // Konvertierung der markierten Objekte in Poly bzw. Bezier.
    void ImpConvertTo(sal_Bool bPath, sal_Bool bLineToArea);

    // Konvertiert ein Obj, wirft bei Erfolg das alte as seiner Liste und
    // fuegt das neue an dessen Position ein. Inkl Undo. Es wird weder ein
    // MarkEntry noch ein ModelChgBroadcast generiert.
    SdrObject* ImpConvertOneObj(SdrObject* pObj, sal_Bool bPath, sal_Bool bLineToArea);

    // Setzen der beiden Flags bToTopPossible und bToBtmPossible.
    // bToTopPossibleDirty und bToBtmPossibleDirty werden dabei gleichzeitig
    // zurueckgesetzt.
    void ImpCheckToTopBtmPossible();

    // fuer CombineMarkedObjects und DismantleMarkedObjects
    void ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const;

    // fuer CombineMarkedObjects
    sal_Bool ImpCanConvertForCombine1(const SdrObject* pObj) const;
    sal_Bool ImpCanConvertForCombine(const SdrObject* pObj) const;
    basegfx::B2DPolyPolygon ImpGetPolyPolygon1(const SdrObject* pObj, sal_Bool bCombine) const;
    basegfx::B2DPolyPolygon ImpGetPolyPolygon(const SdrObject* pObj, sal_Bool bCombine) const;
    basegfx::B2DPolygon ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon) const;

    // fuer DismantleMarkedObjects
    sal_Bool ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPpolygon, sal_Bool bMakeLines) const;
    sal_Bool ImpCanDismantle(const SdrObject* pObj, sal_Bool bMakeLines) const;
    void ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uIntPtr& rPos, SdrPageView* pPV, sal_Bool bMakeLines);
    void ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad, SdrCrookMode eMode,
        sal_Bool bVertical, sal_Bool bNoContortion, sal_Bool bRotate, const Rectangle& rMarkRect);
    void ImpDistortObj(SdrObject* pO, const Rectangle& rRef, const XPolygon& rDistortedRect, sal_Bool bNoContortion);
    sal_Bool ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const;
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
    void BegUndo(const String& rComment) { pMod->BegUndo(rComment); } // Undo-Klammerung auf
    void BegUndo(const String& rComment, const String& rObjDescr, SdrRepeatFunc eFunc=SDRREPFUNC_OBJ_NONE) { pMod->BegUndo(rComment,rObjDescr,eFunc); } // Undo-Klammerung auf
    void EndUndo();                                                   // Undo-Klammerung zu (inkl BroadcastEdges)
    void AddUndo(SdrUndoAction* pUndo)   { pMod->AddUndo(pUndo);    } // Action hinzufuegen
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
    void SetUndoComment(const String& rComment) { pMod->SetUndoComment(rComment); }
    void SetUndoComment(const String& rComment, const String& rObjDescr) { pMod->SetUndoComment(rComment,rObjDescr); }
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

    sal_Bool IsReadOnly() const { ForcePossibilities(); return bReadOnly; }

    // Loeschen aller markierten Objekte
    void DeleteMarkedObj();
    sal_Bool IsDeleteMarkedObjPossible() const { ForcePossibilities(); return bDeletePossible; }

    // Logisch- umschliessendes Rect aller markierten Objekte setzen.
    // Das das wirklich geschieht ist nicht garantiert, denn eine
    // waagerechte Linie hat z.B. immer eine Hoehe von 0.
    void SetMarkedObjRect(const Rectangle& rRect, sal_Bool bCopy=sal_False);
    void MoveMarkedObj(const Size& rSiz, bool bCopy=false);
    void ResizeMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void ResizeMultMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, const bool bCopy, const bool bWdh, const bool bHgt);
    long GetMarkedObjRotate() const;
    void RotateMarkedObj(const Point& rRef, long nWink, bool bCopy=false);
    void MirrorMarkedObj(const Point& rRef1, const Point& rRef2, bool bCopy=false);
    void MirrorMarkedObjHorizontal(sal_Bool bCopy=sal_False);
    void MirrorMarkedObjVertical(sal_Bool bCopy=sal_False);
    long GetMarkedObjShear() const;
    void ShearMarkedObj(const Point& rRef, long nWink, bool bVShear=false, bool bCopy=false);
    void CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical=false, bool bNoContortion=false, bool bCopy=false);
    void DistortMarkedObj(const Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion=false, bool bCopy=false);

    // Markierte Objekte kopieren und anstelle der alten markieren
    void CopyMarkedObj();
    void SetAllMarkedRect(const Rectangle& rRect, sal_Bool bCopy=sal_False) { SetMarkedObjRect(rRect,bCopy); }
    void MoveAllMarked(const Size& rSiz, sal_Bool bCopy=sal_False) { MoveMarkedObj   (rSiz,bCopy); }
    void ResizeAllMarked(const Point& rRef, const Fraction& xFact, const Fraction& yFact, sal_Bool bCopy=sal_False) { ResizeMarkedObj (rRef,xFact,yFact,bCopy); }
    long GetAllMarkedRotate() const { return GetMarkedObjRotate(); }
    void RotateAllMarked(const Point& rRef, long nWink, sal_Bool bCopy=sal_False) { RotateMarkedObj(rRef,nWink,bCopy); }
    void MirrorAllMarked(const Point& rRef1, const Point& rRef2, sal_Bool bCopy=sal_False) { MirrorMarkedObj(rRef1,rRef2,bCopy); }
    void MirrorAllMarkedHorizontal(sal_Bool bCopy=sal_False) { MirrorMarkedObjHorizontal(bCopy); }
    void MirrorAllMarkedVertical(sal_Bool bCopy=sal_False) { MirrorMarkedObjVertical(bCopy); }
    long GetAllMarkedShear() const { return GetMarkedObjShear(); }
    void ShearAllMarked(const Point& rRef, long nWink, sal_Bool bVShear=sal_False, sal_Bool bCopy=sal_False) { ShearMarkedObj(rRef,nWink,bVShear,bCopy); }
    void CrookAllMarked(const Point& rRef, const Point& rRad, SdrCrookMode eMode, sal_Bool bVertical=sal_False, sal_Bool bNoContortion=sal_False, sal_Bool bCopy=sal_False) { CrookMarkedObj(rRef,rRad,eMode,bVertical,bNoContortion,bCopy); }
    void CopyMarked() { CopyMarkedObj(); }
    sal_Bool IsMoveAllowed() const { ForcePossibilities(); return bMoveAllowed && !bMoveProtect; }
    sal_Bool IsResizeAllowed(sal_Bool bProp=sal_False) const;
    sal_Bool IsRotateAllowed(sal_Bool b90Deg=sal_False) const;
    sal_Bool IsMirrorAllowed(sal_Bool b45Deg=sal_False, sal_Bool b90Deg=sal_False) const;
    sal_Bool IsTransparenceAllowed() const;
    sal_Bool IsGradientAllowed() const;
    sal_Bool IsShearAllowed() const;
    sal_Bool IsEdgeRadiusAllowed() const;
    sal_Bool IsCrookAllowed(sal_Bool bNoContortion=sal_False) const;
    sal_Bool IsDistortAllowed(sal_Bool bNoContortion=sal_False) const;

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
    void CombineMarkedObjects(sal_Bool bNoPolyPoly = sal_True);

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
    void DismantleMarkedObjects(sal_Bool bMakeLines=sal_False);
    sal_Bool IsCombinePossible(sal_Bool bNoPolyPoly=sal_False) const;
    sal_Bool IsDismantlePossible(sal_Bool bMakeLines=sal_False) const;

    // Ein neues bereits fertig konstruiertes Obj einfuegen. Das Obj gehoert
    // anschliessend dem Model. Nach dem Einfuegen wird das neue Objekt
    // markiert (wenn dies nicht via nOptions unterbunden wird).
    // U.U. wird das Obj jedoch nicht eingefuegt, sondern deleted, naemlich
    // wenn der Ziel-Layer gesperrt oder nicht sichtbar ist. In diesem Fall
    // returniert die Methode mit FALSE.
    // Die Methode generiert u.a. auch eine Undo-Action.
    sal_Bool InsertObjectAtView(SdrObject* pObj, SdrPageView& rPV, sal_uIntPtr nOptions=0);

    // Ein Zeichenobjekt durch ein neues ersetzen. *pNewObj gehoert
    // anschliessend mir, *pOldObj wandert ins Undo.
    // Sollte in jedem Fall mit einer Undo-Klammerung versehen werden, z.B.:
    // aStr+=" ersetzen";
    // BegUndo(aStr);
    // ReplaceObject(...);
    // ...
    // EndUndo();
    void ReplaceObjectAtView(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, sal_Bool bMark=sal_True);

    void SetNotPersistAttrToMarked(const SfxItemSet& rAttr, sal_Bool bReplaceAll);
    void MergeNotPersistAttrFromMarked(SfxItemSet& rAttr, sal_Bool bOnlyHardAttr) const;
    void MergeAttrFromMarked(SfxItemSet& rAttr, sal_Bool bOnlyHardAttr) const;
    SfxItemSet GetAttrFromMarked(sal_Bool bOnlyHardAttr) const;
    void SetAttrToMarked(const SfxItemSet& rAttr, sal_Bool bReplaceAll);

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
    void SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);

    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const;

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(sal_Bool& rOk) const;
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);

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

    sal_Bool IsGroupPossible() const { ForcePossibilities(); return bGroupPossible; }
    sal_Bool IsUnGroupPossible() const { ForcePossibilities(); return bUnGroupPossible; }
    sal_Bool IsGroupEnterPossible() const { ForcePossibilities(); return bGrpEnterPossible; }

    // Markierte Objekte in Polygone/Bezierkurven verwandeln. Die sal_Bool-
    // Funktionen returnen sal_True, wenn wenigstens eins der markierten
    // Objekte gewandelt werden kann. Memberobjekte von Gruppenobjekten
    // werden ebenfalls gewandelt. Naehere Beschreibung siehe SdrObj.HXX.
    sal_Bool IsConvertToPathObjPossible(sal_Bool bLineToArea) const { ForcePossibilities(); return sal_Bool(bLineToArea ? bCanConvToPathLineToArea : bCanConvToPath); }
    sal_Bool IsConvertToPolyObjPossible(sal_Bool bLineToArea) const { ForcePossibilities(); return sal_Bool(bLineToArea ? bCanConvToPolyLineToArea : bCanConvToPoly); }
    sal_Bool IsConvertToContourPossible() const { ForcePossibilities(); return bCanConvToContour; }
    void ConvertMarkedToPathObj(sal_Bool bLineToArea);
    void ConvertMarkedToPolyObj(sal_Bool bLineToArea);

    // Alle markierten Objekte untereinander ausrichten. Normalerweise werden
    // das SnapRect der Obj verwendet. Ist bBoundRects=sal_True, werden stattdessen
    // die BoundRects ausgerichtet.
    void AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert, sal_Bool bBoundRects=sal_False);
    sal_Bool IsAlignPossible() const;

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
    sal_Bool IsToTopPossible() const { ForcePossibilities(); return bToTopPossible; }
    sal_Bool IsToBtmPossible() const { ForcePossibilities(); return bToBtmPossible; }
    sal_Bool IsReverseOrderPossible() const { ForcePossibilities(); return bReverseOrderPossible; }

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
    sal_Bool IsImportMtfPossible() const { ForcePossibilities(); return bImportMtfPossible; }

    // Wird der Modus VirtualObjectBundling eingeschaltet, werden beim
    // ToTop/ToBtm virtuelle Objekte die dasselbe Objekt referenzieren
    // in ihrer Z-Order buendig zusammengehalten (Writer).
    // Defaulteinstellung ist sal_False=ausgeschaltet.
    void SetVirtualObjectBundling(sal_Bool bOn) { bBundleVirtObj=bOn; }
    sal_Bool IsVirtualObjectBundling() const { return bBundleVirtObj; }

    // von der SdrMarkView ueberladen fuer den internen gebrauch
    virtual void MarkListHasChanged();
    virtual void ModelHasChanged();
};

#endif //_SVDEDTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
