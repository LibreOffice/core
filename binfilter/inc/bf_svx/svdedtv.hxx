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

#ifndef _SVDEDTV_HXX
#define _SVDEDTV_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <bf_svx/svdhdl.hxx>

#include <bf_svx/svdmrkv.hxx>

#include <bf_svx/xpoly.hxx>

#include <bf_svx/svdmodel.hxx>
namespace binfilter {

class SfxUndoAction;
class SfxStyleSheet;

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrUndoAction;
class SdrUndoGroup;
class SdrLayer;
class SvdProgressInfo;

//************************************************************
//   Defines
//************************************************************

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

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//  @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrEditView: public SdrMarkView
{
    friend class				SdrPageView;

protected:

    // Die Transformationsnachfragen, etc. etwas cachen
    unsigned					bPossibilitiesDirty : 1;
    unsigned					bReadOnly : 1;
    unsigned					bGroupPossible : 1;
    unsigned					bUnGroupPossible : 1;
    unsigned					bGrpEnterPossible : 1;
    unsigned					bDeletePossible : 1;
    unsigned					bToTopPossible : 1;
    unsigned					bToBtmPossible : 1;
    unsigned					bReverseOrderPossible : 1;
    unsigned					bImportMtfPossible : 1;
    unsigned					bCombinePossible : 1;
    unsigned					bDismantlePossible : 1;
    unsigned					bCombineNoPolyPolyPossible : 1;
    unsigned					bDismantleMakeLinesPossible : 1;
    unsigned					bOrthoDesiredOnMarked : 1;
    unsigned					bMoreThanOneNotMovable : 1;   // Es ist mehr als ein Objekt nicht verschiebbar
    unsigned					bOneOrMoreMovable : 1;        // Wenigstens 1 Obj verschiebbar
    unsigned					bMoreThanOneNoMovRot : 1;     // Es ist mehr als ein Objekt nicht verschieb- und drehbar (Crook)
    unsigned					bContortionPossible : 1;      // Alles Polygone (ggf. gruppiert)
    unsigned					bAllPolys : 1;                // Alles Polygone (nicht gruppiert)
    unsigned					bOneOrMorePolys : 1;          // Mindestens 1 Polygon (nicht gruppiert)
    unsigned					bMoveAllowed : 1;
    unsigned					bResizeFreeAllowed : 1;
    unsigned					bResizePropAllowed : 1;
    unsigned					bRotateFreeAllowed : 1;
    unsigned					bRotate90Allowed : 1;
    unsigned					bMirrorFreeAllowed : 1;
    unsigned					bMirror45Allowed : 1;
    unsigned					bMirror90Allowed : 1;
    unsigned					bShearAllowed : 1;
    unsigned					bEdgeRadiusAllowed : 1;
    unsigned					bTransparenceAllowed : 1;
    unsigned					bGradientAllowed : 1;
    unsigned					bCanConvToPath : 1;
    unsigned					bCanConvToPoly : 1;
    unsigned					bCanConvToContour : 1;
    unsigned					bCanConvToPathLineToArea : 1;
    unsigned					bCanConvToPolyLineToArea : 1;
    unsigned					bMoveProtect : 1;
    unsigned					bResizeProtect : 1;
    unsigned					bCombineError : 1;
    // Z-Order von virtuellen Objekten zusammenhalten (Writer)
    unsigned					bBundleVirtObj : 1;

private:
    void ImpClearVars();
    void ImpResetPossibilityFlags();

protected:

    // Konvertierung der markierten Objekte in Poly bzw. Bezier.

    // Konvertiert ein Obj, wirft bei Erfolg das alte as seiner Liste und
    // fuegt das neue an dessen Position ein. Inkl Undo. Es wird weder ein
    // MarkEntry noch ein ModelChgBroadcast generiert.

    // Setzen der beiden Flags bToTopPossible und bToBtmPossible.
    // bToTopPossibleDirty und bToBtmPossibleDirty werden dabei gleichzeitig
    // zurueckgesetzt.
    void ImpCheckToTopBtmPossible();

    // fuer den Writer werden virtuelle Objekte buendig zusammengehalten (Z-Order)

    // fuer CombineMarkedObjects und DismantleMarkedObjects

    // fuer CombineMarkedObjects

    // fuer DismantleMarkedObjects

    // Entfernt alle Obj der MarkList aus ihren ObjLists inkl Undo.
    // Die Eintraege in rMark bleiben erhalten. rMark ist nicht const,
    // da ein ForceSort() gerufen wird.

    // Die Transformationsnachfragen etwas cachen
    //void ImpCheckMarkTransform() const; veraltet
    // Checken, was man so mit den markierten Objekten alles machen kann
    virtual void CheckPossibilities();
    void ForcePossibilities() const { if (bPossibilitiesDirty || bSomeObjChgdFlag) ((SdrEditView*)this)->CheckPossibilities(); }

public:
    SdrEditView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrEditView();

    // Jeder Aufruf einer undofaehigen Methode an der View generiert eine
    // UndoAction. Moechte man mehrere
    // Methodenaufrufe zu einer UndoAction zusammenfassen, so kann man diese
    // mit BegUndo() / EndUndo() klammern (beliebig tief). Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. NotifyNewUndoAction() wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. NotifyNewUndoAction() wird nicht gerufen bei einer
    // leeren Klammerung.
#ifndef WIN
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
#else  // ifndef WIN
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
#endif

    // Verschieben eines Layer (Layerreihenfolge aendern)

    // Markierte Objekte die ausserhalb ihrer Page liegen
    // werden ggf. einer anderen Page zugewiesen
    // z.Zt. noch ohne Undo!!!

    BOOL IsReadOnly() const { ForcePossibilities(); return bReadOnly; }

    // Loeschen aller markierten Objekte
    BOOL IsDeleteMarkedObjPossible() const { ForcePossibilities(); return bDeletePossible; }

    // Logisch- umschliessendes Rect aller markierten Objekte setzen.
    // Das das wirklich geschieht ist nicht garantiert, denn eine
    // waagerechte Linie hat z.B. immer eine Hoehe von 0.

    // Markierte Objekte kopieren und anstelle der alten markieren
    BOOL IsMoveAllowed() const { ForcePossibilities(); return bMoveAllowed && !bMoveProtect; }


    // Geometrische Attribute (Position, Groesse, Drehwinkel)
    // Bei der Position wird ein evtl. gesetzter PageOrigin beruecksichtigt.

    // Returnt NULL wenn:
    // - Nix markiert,
    // - kein StyleSheet an den markierten Objekten gesetzt
    // - Bei Mehrfachselektion die markierten Objekte auf unterschiedliche
    //   StyleSheets verweisen.

    // z.Zt. noch ohne Undo :(

    /* new interface src537 */

    BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);

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

    BOOL IsGroupPossible() const { ForcePossibilities(); return bGroupPossible; }
    BOOL IsUnGroupPossible() const { ForcePossibilities(); return bUnGroupPossible; }
    BOOL IsGroupEnterPossible() const { ForcePossibilities(); return bGrpEnterPossible; }

    // Markierte Objekte in Polygone/Bezierkurven verwandeln. Die BOOL-
    // Funktionen returnen TRUE, wenn wenigstens eins der markierten
    // Objekte gewandelt werden kann. Memberobjekte von Gruppenobjekten
    // werden ebenfalls gewandelt. Naehere Beschreibung siehe SdrObj.HXX.
    BOOL IsConvertToPathObjPossible(BOOL bLineToArea) const { ForcePossibilities(); return bLineToArea ? bCanConvToPathLineToArea : bCanConvToPath; }
    BOOL IsConvertToPolyObjPossible(BOOL bLineToArea) const { ForcePossibilities(); return bLineToArea ? bCanConvToPolyLineToArea : bCanConvToPoly; }
    BOOL IsConvertToContourPossible() const { ForcePossibilities(); return bCanConvToContour; }

    // Alle markierten Objekte untereinander ausrichten. Normalerweise werden
    // das SnapRect der Obj verwendet. Ist bBoundRects=TRUE, werden stattdessen
    // die BoundRects ausgerichtet.

    // Markierte Objekte etwas nach "oben" holen

    // Markierte Objekte etwas nach "unten" holen

    // Markierte Objekte ganz nach "oben" stellen

    // Markierte Objekte ganz nach "unten" stellen

    // Markierte direkt vor das uebergebene Objekt stellen
    // NULL -> wie PutMarkedToTop();

    // Markierte direkt hinter das uebergebene Objekt stellen
    // NULL -> wie PutMarkedToBtm();

    // Z-Order der markierten Objekte vertauschen

    // Feststellen, ob nach vorn/hinten stellen moeglich ist
    // GetMaxToTop/BtmObj() wird von diesen Methoden nur begrenzt
    // beruecksichtigt, d.h. es kann vorkommen dass IsToTopPossible()
    // TRUE liefert, MovMarkedToTop() jedoch nichts aendert (z.B. bei
    // Mehrfachselektion), weil eine von der abgeleiteten View ueber
    // GetMaxToTopObj() auferlegte Restriktion dies verhindert.
    BOOL IsToTopPossible() const { ForcePossibilities(); return bToTopPossible; }
    BOOL IsToBtmPossible() const { ForcePossibilities(); return bToBtmPossible; }
    BOOL IsReverseOrderPossible() const { ForcePossibilities(); return bReverseOrderPossible; }

    // Ueber diese Methoden stellt die View fest, wie weit ein Objekt
    // nach vorn bzw. nach hinten gestellt werden darf (Z-Order). Das
    // zurueckgegebene Objekt wird dann nicht "ueberholt". Bei Rueckgabe
    // von NULL (Defaultverhalten) bestehen keine Restriktionen.

    // Folgende Methode wird gerufen, wenn z.B. durch ToTop, ToBtm, ... die
    // Reihenfolgen der Objekte geaendert wurde. Der Aufruf erfolgt dann nach
    // jedem SdrObjList::SetObjectOrdNum(nOldPos,nNewPos);

    // Falls ein oder mehrere Objekte des Types SdrGrafObj oder SdrOle2Obj
    // markiert sind und diese in der Lage sind ein StarView-Metafile zu
    // liefern, konvertiert diese Methode das Metafile in Drawingobjekte.
    // Die SdrGrafObjs/SdrOle2Objs werden dann durch die neue Objekte ersetzt.
    BOOL IsImportMtfPossible() const { ForcePossibilities(); return bImportMtfPossible; }

    // Wird der Modus VirtualObjectBundling eingeschaltet, werden beim
    // ToTop/ToBtm virtuelle Objekte die dasselbe Objekt referenzieren
    // in ihrer Z-Order buendig zusammengehalten (Writer).
    // Defaulteinstellung ist FALSE=ausgeschaltet.
    void SetVirtualObjectBundling(BOOL bOn) { bBundleVirtObj=bOn; }
    BOOL IsVirtualObjectBundling() const { return bBundleVirtObj; }

    // von der SdrMarkView ueberladen fuer den internen gebrauch
    virtual void MarkListHasChanged();
    virtual void ModelHasChanged();
};

}//end of namespace binfilter
#endif //_SVDEDTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
