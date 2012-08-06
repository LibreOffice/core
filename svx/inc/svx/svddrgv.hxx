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

#ifndef _SVDDRGV_HXX
#define _SVDDRGV_HXX

#include "svx/svxdllapi.h"
#include <svx/svdxcgv.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrUndoGeoObj;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@@@@   @@@@   @@@@   @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@  @@ @@      @@ @@ @@ @@    @@ @ @@
//  @@  @@ @@@@@  @@@@@@ @@ @@@  @@@@@ @@ @@@@  @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@@@@  @@  @@ @@  @@  @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrDragView: public SdrExchangeView
{
    friend class                SdrPageView;
    friend class                SdrDragMethod;

protected:
    SdrHdl*                     pDragHdl;
    SdrDragMethod*              mpCurrentSdrDragMethod;
    SdrUndoGeoObj*              pInsPointUndo;
    Rectangle                   aDragLimit;
    rtl::OUString               aInsPointUndoStr;
    SdrMarkList                 aFollowingEdges; // If nodes are dragged, all edges should follow as Xor
    SdrHdlKind                  eDragHdl;

    sal_uIntPtr                     nDragXorPolyLimit;
    sal_uIntPtr                     nDragXorPointLimit;
    sal_uInt16                      nDetailedEdgeDraggingLimit;

    unsigned                    bFramDrag : 1;        // currently frame dragging
    unsigned                    bDragSpecial : 1;     // currently Special Obj-dragging
    unsigned                    bMarkedHitMovesAlways : 1; // Persistent
    unsigned                    bDragLimit : 1;      // Limit on SnapRect instead of BoundRect
    unsigned                    bDragHdl : 1;        // TRUE: RefPt is slid
    unsigned                    bDragStripes : 1;    // Persistent
    unsigned                    bMirrRefDragObj : 1; // Persistent - During the drag, show the mirror axis of the mirrored objects as Xor
    unsigned                    mbSolidDragging : 1;  // allow solid create/drag of objects
    unsigned                    bMouseHideWhileDraggingPoints : 1;
    unsigned                    bResizeAtCenter : 1;
    unsigned                    bCrookAtCenter : 1;
    unsigned                    bDragWithCopy : 1;
    unsigned                    bInsGluePoint : 1;
    unsigned                    bInsObjPointMode : 1;
    unsigned                    bInsGluePointMode : 1;
    unsigned                    bNoDragXorPolys : 1;
    unsigned                    bAutoVertexCon : 1;  // Automatische Konnektorgenerierung an den Scheitelpunkten
    unsigned                    bAutoCornerCon : 1;  // Automatische Konnektorgenerierung an den Eckpunkten
    unsigned                    bRubberEdgeDragging : 1;
    unsigned                    bDetailedEdgeDragging : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    virtual void SetMarkHandles();
    void ShowDragObj();
    void HideDragObj();
    sal_Bool ImpBegInsObjPoint(sal_Bool bIdxZwang, sal_uInt32 nIdx, const Point& rPnt, sal_Bool bNewObj, OutputDevice* pOut);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrDragView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrDragView();

public:
    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    // Spezialimplementation fuer den Writer:
    // TakeDragObjAnchorPos() liefert die Position an der ein Objekt
    // beim Draggen einer Einfachselektion ungefaehr landet wenn es
    // "losgelassen" wird (EndDrag).
    // In der Regel ist das die linke obere Ecke des zu erwartenden neuen
    // SnapRects. Ausnahme: CaptionObj. Dort ist es die Position des
    // "Schwanzendes".
    // Bei Returncode sal_False konnte ich die Position nicht bestimmen
    // (z.B. Punktverschiebung, Mehrfachselektion, Schieben der
    // Spiegelschse, ...)
    sal_Bool TakeDragObjAnchorPos(Point& rPos, sal_Bool bTopRight = sal_False ) const;

    // Wird pForcedMeth uebergeben, so wird pHdl, ... nicht ausgewerten, sondern diese
    // Drag-Methode verwendet. Die Instanz geht dabei ins Eigentum der View ueber und
    // wird zum Ende des Draggings destruiert.
    virtual sal_Bool BegDragObj(const Point& rPnt, OutputDevice* pOut=NULL, SdrHdl* pHdl=NULL, short nMinMov=-3, SdrDragMethod* pForcedMeth=NULL);
    void MovDragObj(const Point& rPnt);
    sal_Bool EndDragObj(sal_Bool bCopy=sal_False);
    void BrkDragObj();
    sal_Bool IsDragObj() const { return mpCurrentSdrDragMethod && !bInsPolyPoint && !bInsGluePoint; }
    SdrHdl* GetDragHdl() const { return pDragHdl; }
    SdrDragMethod* GetDragMethod() const { return mpCurrentSdrDragMethod; }
    sal_Bool IsDraggingPoints() const { return eDragHdl==HDL_POLY; }
    sal_Bool IsDraggingGluePoints() const { return eDragHdl==HDL_GLUE; }

    // Wer das beim BegDrag oder mittendrin schon festlegen will.
    // (Wird bei jedem BegDrag auf sal_False zurueckgesetzt, also nach
    // BegDrag setzen.)
    void SetDragWithCopy(sal_Bool bOn) { bDragWithCopy = bOn; }
    sal_Bool IsDragWithCopy() const { return bDragWithCopy; }

    void SetInsertGluePoint(sal_Bool bOn) { bInsGluePoint = bOn; }
    sal_Bool IsInsertGluePoint() const { return bInsGluePoint; }

    // Interaktives einfuegen eines neuen Punktes. nIdx=0 => vor dem ersten Punkt.
    sal_Bool IsInsObjPointPossible() const;
    sal_Bool IsInsPointPossible() const { return IsInsObjPointPossible(); }
    sal_Bool BegInsObjPoint(const Point& rPnt, sal_Bool bNewObj) { return ImpBegInsObjPoint(sal_False, 0L, rPnt, bNewObj, 0L); }
    void MovInsObjPoint(const Point& rPnt) { MovDragObj(rPnt); }
    sal_Bool EndInsObjPoint(SdrCreateCmd eCmd);
    void BrkInsObjPoint() { BrkDragObj(); }
    sal_Bool IsInsObjPoint() const { return mpCurrentSdrDragMethod && bInsPolyPoint; }

    // Fuer die App zum Verwalten des Status. GetPreferedPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsObjPointMode(sal_Bool bOn) { bInsObjPointMode = bOn; }
    sal_Bool IsInsObjPointMode() const { return bInsObjPointMode; }

    sal_Bool IsInsGluePointPossible() const;
    sal_Bool BegInsGluePoint(const Point& rPnt);
    void MovInsGluePoint(const Point& rPnt) { MovDragObj(rPnt); }
    sal_Bool EndInsGluePoint() { return EndDragObj(); }
    void BrkInsGluePoint() { BrkDragObj(); }
    sal_Bool IsInsGluePoint() const { return mpCurrentSdrDragMethod && bInsGluePoint; }

    // Fuer die App zum Verwalten des Status. GetPreferedPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsGluePointMode(sal_Bool bOn) { bInsGluePointMode = bOn; }
    sal_Bool IsInsGluePointMode() const { return bInsGluePointMode; }

    // Begrenzungslinien ueber's gesamte Win waehrend des Draggens
    // Persistent. Default=FALSE.
    void SetDragStripes(sal_Bool bOn);
    sal_Bool IsDragStripes() const { return bDragStripes; }

    // Handles waehrend des Draggens verstecken
    //HMHvoid SetDragHdlHide(sal_Bool bOn);
    //HMHBOOL IsDragHdlHide() const { return bNoDragHdl; }

    // Beim Draggen von Polygonpunkten und Klebepunkten
    // die Maus verstecken. Default=FALSE
    void SetMouseHideWhileDraggingPoints(sal_Bool bOn) { bMouseHideWhileDraggingPoints = bOn; }
    sal_Bool IsMouseHideWhileDraggingPoints() const { return bMouseHideWhileDraggingPoints; }

    // Beim Draggen werden i.d.R. die Konturen der markierten Objekte
    // als Xor-Polygone dargestellt. Wird dieses Flag hier gesetzt,
    // wird (z.B. bei Mehrfachselektion) nur ein Xor-Frame gezeichnet.
    // Bei objektspeziefischem Dragging (Polygonpunkte,Eckenradius,...
    // hat diese Einstellung keine Auswirkung.
    // Auch waerend des Draggens umschaltbar.
    // Default=Off
    void SetNoDragXorPolys(sal_Bool bOn);
    sal_Bool IsNoDragXorPolys() const { return bNoDragXorPolys; }

    // Uebersteigt die Anzahl der markierten Objekte den hier eingestellten
    // Wert, wird implizit (temporaer) auf NoDragPolys geschaltet.
    // PolyPolygone etc werden entsprechend als mehrere Objekte gewertet.
    // Default=100
    void  SetDragXorPolyLimit(sal_uIntPtr nObjAnz) { nDragXorPolyLimit=nObjAnz; }
    sal_uIntPtr GetDragXorPolyLimit() const { return nDragXorPolyLimit; }

    // Wie DragXorPolyLimit, jedoch bezogen auf die Gesamtpunktanzahl
    // aller Polygone. Default=500.
    // Auf NoDragPolys wird (temporaer) geschaltet, wenn eins der Limits
    // ueberstiegen wird.
    void  SetDragXorPointLimit(sal_uIntPtr nPntAnz) { nDragXorPointLimit=nPntAnz; }
    sal_uIntPtr GetDragXorPointLimit() const { return nDragXorPointLimit; }

    void SetSolidDragging(bool bOn);
    bool IsSolidDragging() const;

    // Dragging/Creating von Verbindern:
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Verbinder an Objektscheitelpunkte ankleben
    // Default=sal_True=Ja
    void SetAutoVertexConnectors(sal_Bool bOn) { bAutoVertexCon = bOn; }
    sal_Bool IsAutoVertexConnectors() const { return bAutoVertexCon; }

    // Verbinder an Objektecken ankleben
    // Default=sal_False=Nein
    void SetAutoCornerConnectors(sal_Bool bOn) { bAutoCornerCon = bOn; }
    sal_Bool IsAutoCornerConnectors() const { return bAutoCornerCon; }

    // Dragging von verbundenen Objekten (Nodes):
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // DetailedEdgeDraggingLimit: Wie RubberEdgeDraggingLimit, jedoch bezieht
    // sich dieses Limit auf die detalierte Darstellung, d.h. nicht nur
    // Gummibaender sondern komplette Neuberechnunen sind beim Draggen sichtbar.
    // Diese detalierte Darstellung ist eh nur beim MoveDrag moeglich.
    // Defaultwert ist 10
    sal_Bool IsDetailedEdgeDragging() const { return bDetailedEdgeDragging; }

    sal_uInt16 GetDetailedEdgeDraggingLimit() const { return nDetailedEdgeDraggingLimit; }

    // EdgeDraggingLimit: Sind mehr als nEdgeObjAnz Kanten betroffen, werden
    // diese beim interaktiven Draggen nicht mit angezeigt.
    // Gemeint sind hier die "Gummibaender", die weniger Rechenzeit benoetigen
    // als die kompletten Neuberechnungen beim DetailedEdgeDragging.
    // Defaultwert ist 100
    sal_Bool IsRubberEdgeDragging() const { return bRubberEdgeDragging; }

    // Verbinderhandling also zu deutsch wie folgt (bei Defaulteinstellungen):
    // - Sind bis max 10 Verbinder betroffen werden diese bei jedem
    //   MouseMove neu berechnet
    // - Sind zwischen 11 und 100 Verbinder betroffen werden die
    //   Verbindungen beim Draggen als gerade Linien dargestellt.
    // - Bei mehr als 100 betroffenen Verbindern wird beim Draggen nichts
    //   mehr gezeichnet was auf Verbinder hinweist.

    // Ist ein spezieller Dragmode eingeschaltet, wie Rotate, Mirror oder Crook,
    // dann leitet ein Hit auf das markierte Objekt genau dieses Dragging ein.
    // Setzt man MarkedHitMovesAlways auf sal_True, so leitet ein Hit auf das
    // markierte Objekt immer ein Moven ein, unabhaengig vom gesetzten DragMode.
    // Dieses Flag ist persistent und sollte von der App fuer den Anwender
    // konfigurierbar sein!
    void SetMarkedHitMovesAlways(sal_Bool bOn) { bMarkedHitMovesAlways = bOn; }
    sal_Bool IsMarkedHitMovesAlways() const { return bMarkedHitMovesAlways; }

    // Beim Draggen der Spiegelachse das Spiegelbild der markierten Objekte
    // als Xor darstellen? Persistent. Noch nicht implementiert. Default TRUE.
    void SetMirrRefDragObj(sal_Bool bOn) { bMirrRefDragObj = bOn; }
    sal_Bool IsMirrRefDragObj() const { return bMirrRefDragObj; }

    sal_Bool IsOrthoDesired() const;

    // Beim Resize die Mitte als Referenz
    // Default=FALSE.
    sal_Bool IsResizeAtCenter() const { return bResizeAtCenter; }
    void SetResizeAtCenter(sal_Bool bOn) { bResizeAtCenter = bOn; }

    // Symmetrisches Crook
    // Default=FALSE.
    sal_Bool IsCrookAtCenter() const { return bCrookAtCenter; }
    void SetCrookAtCenter(sal_Bool bOn) { bCrookAtCenter = bOn; }

    // Begrenzung des Arbeitsbereichs. Die Begrenzung bezieht sich auf die
    // View, nicht auf die einzelnen PageViews. Von der View wird diese
    // Begrenzung nur bei Interaktionen wie Dragging und Create ausgewertet.
    // Bei von der App algorithmisch oder UI-gesteuerte Aktionen (SetGeoAttr,
    // MoveMarkedObj, ...) muss die App dieses Limit selbst beruecksichtigen.
    // Ferner ist dieses Limit als Grob-Limit zu sehen. U.U. koennen Objekte
    // (z.B. beim Drehen) nicht exakt bis an dieses Limit herangedraggt werden,
    // koennen Objekte durch Rundungsfehler doch etwas ueberstehen, ... .
    // Default=EmptyRect=keine Begrenzung.
    // erst z.T. impl.
    // (besser in die DragView?)
    void SetWorkArea(const Rectangle& rRect) { aMaxWorkArea=rRect; }
    const Rectangle& GetWorkArea() const { return aMaxWorkArea; }


    // Das DragLimit ist bezogen auf die Page des Objekts.
    // (Oder auf die View??? Muss ich mal^^^^recherchieren. Joe.)
    // sal_False=Kein Limit.
    // Das Rueckgabe-Rect muss absolute Koordinaten enthalten. Der Maximale
    // Dragbereich wird von der View dann so gewaehlt, dass das SnapRect des
    // Objekts bis Maximal auf die Kante des LimitRects gemoved bzw. gesized
    // wird. Bei Objekten wie Bezierkurven, gedrehten Rechtecken ist zu beachten
    // das durch anschliessende Neuberechnung des SnapRects (bei Resize)
    // Rundungsfehler auftreten koennen, wodurch das LimitRect minnimal
    // ueberschritten werden koennte...
    // Implementiert fuer Move und Resize.
    virtual sal_Bool TakeDragLimit(SdrDragMode eMode, Rectangle& rRect) const;
};

#endif //_SVDDRGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
