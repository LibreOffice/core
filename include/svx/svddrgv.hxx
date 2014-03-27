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

#ifndef INCLUDED_SVX_SVDDRGV_HXX
#define INCLUDED_SVX_SVDDRGV_HXX

#include <svx/svxdllapi.h>
#include <svx/svdxcgv.hxx>

class SdrUndoGeoObj;

class SVX_DLLPUBLIC SdrDragView: public SdrExchangeView
{
    friend class                SdrPageView;
    friend class                SdrDragMethod;

protected:
    SdrHdl*                     pDragHdl;
    SdrDragMethod*              mpCurrentSdrDragMethod;
    SdrUndoGeoObj*              pInsPointUndo;
    Rectangle                   aDragLimit;
    OUString               aInsPointUndoStr;
    SdrMarkList                 aFollowingEdges; // If nodes are dragged, all edges should follow as Xor
    SdrHdlKind                  eDragHdl;

    sal_uIntPtr                     nDragXorPolyLimit;
    sal_uIntPtr                     nDragXorPointLimit;
    sal_uInt16                      nDetailedEdgeDraggingLimit;

    bool                        bFramDrag : 1;        // currently frame dragging
    bool                        bDragSpecial : 1;     // currently Special Obj-dragging
    bool                        bMarkedHitMovesAlways : 1; // Persistent
    bool                        bDragLimit : 1;      // Limit on SnapRect instead of BoundRect
    bool                        bDragHdl : 1;        // TRUE: RefPt is slid
    bool                        bDragStripes : 1;    // Persistent
    bool                        bMirrRefDragObj : 1; // Persistent - During the drag, show the mirror axis of the mirrored objects as Xor
    bool                        mbSolidDragging : 1;  // allow solid create/drag of objects
    bool                        bMouseHideWhileDraggingPoints : 1;
    bool                        bResizeAtCenter : 1;
    bool                        bCrookAtCenter : 1;
    bool                        bDragWithCopy : 1;
    bool                        bInsGluePoint : 1;
    bool                        bInsObjPointMode : 1;
    bool                        bInsGluePointMode : 1;
    bool                        bNoDragXorPolys : 1;
    bool                        bAutoVertexCon : 1;  // Automatische Konnektorgenerierung an den Scheitelpunkten
    bool                        bAutoCornerCon : 1;  // Automatische Konnektorgenerierung an den Eckpunkten
    bool                        bRubberEdgeDragging : 1;
    bool                        bDetailedEdgeDragging : 1;

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    virtual void SetMarkHandles() SAL_OVERRIDE;
    void ShowDragObj();
    void HideDragObj();
    bool ImpBegInsObjPoint(bool bIdxZwang, sal_uInt32 nIdx, const Point& rPnt, bool bNewObj, OutputDevice* pOut);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrDragView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrDragView();

public:
    virtual bool IsAction() const SAL_OVERRIDE;
    virtual void MovAction(const Point& rPnt) SAL_OVERRIDE;
    virtual void EndAction() SAL_OVERRIDE;
    virtual void BckAction() SAL_OVERRIDE;
    virtual void BrkAction() SAL_OVERRIDE;
    virtual void TakeActionRect(Rectangle& rRect) const SAL_OVERRIDE;

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
    bool TakeDragObjAnchorPos(Point& rPos, bool bTopRight = false ) const;

    // Wird pForcedMeth uebergeben, so wird pHdl, ... nicht ausgewerten, sondern diese
    // Drag-Methode verwendet. Die Instanz geht dabei ins Eigentum der View ueber und
    // wird zum Ende des Draggings destruiert.
    virtual bool BegDragObj(const Point& rPnt, OutputDevice* pOut=NULL, SdrHdl* pHdl=NULL, short nMinMov=-3, SdrDragMethod* pForcedMeth=NULL);
    void MovDragObj(const Point& rPnt);
    bool EndDragObj(bool bCopy=false);
    void BrkDragObj();
    bool IsDragObj() const { return mpCurrentSdrDragMethod && !bInsPolyPoint && !bInsGluePoint; }
    SdrHdl* GetDragHdl() const { return pDragHdl; }
    SdrDragMethod* GetDragMethod() const { return mpCurrentSdrDragMethod; }
    bool IsDraggingPoints() const { return eDragHdl==HDL_POLY; }
    bool IsDraggingGluePoints() const { return eDragHdl==HDL_GLUE; }

    // Wer das beim BegDrag oder mittendrin schon festlegen will.
    // (Wird bei jedem BegDrag auf sal_False zurueckgesetzt, also nach
    // BegDrag setzen.)
    void SetDragWithCopy(bool bOn) { bDragWithCopy = bOn; }
    bool IsDragWithCopy() const { return bDragWithCopy; }

    void SetInsertGluePoint(bool bOn) { bInsGluePoint = bOn; }
    bool IsInsertGluePoint() const { return bInsGluePoint; }

    // Interaktives einfuegen eines neuen Punktes. nIdx=0 => vor dem ersten Punkt.
    bool IsInsObjPointPossible() const;
    bool BegInsObjPoint(const Point& rPnt, bool bNewObj) { return ImpBegInsObjPoint(false, 0L, rPnt, bNewObj, 0L); }
    void MovInsObjPoint(const Point& rPnt) { MovDragObj(rPnt); }
    bool EndInsObjPoint(SdrCreateCmd eCmd);
    void BrkInsObjPoint() { BrkDragObj(); }
    bool IsInsObjPoint() const { return mpCurrentSdrDragMethod && bInsPolyPoint; }

    // Fuer die App zum Verwalten des Status. GetPreferredPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsObjPointMode(bool bOn) { bInsObjPointMode = bOn; }
    bool IsInsObjPointMode() const { return bInsObjPointMode; }

    bool IsInsGluePointPossible() const;
    bool BegInsGluePoint(const Point& rPnt);
    void MovInsGluePoint(const Point& rPnt) { MovDragObj(rPnt); }
    bool EndInsGluePoint() { return EndDragObj(); }
    void BrkInsGluePoint() { BrkDragObj(); }
    bool IsInsGluePoint() const { return mpCurrentSdrDragMethod && bInsGluePoint; }

    // Fuer die App zum Verwalten des Status. GetPreferredPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsGluePointMode(bool bOn) { bInsGluePointMode = bOn; }
    bool IsInsGluePointMode() const { return bInsGluePointMode; }

    // Begrenzungslinien ueber's gesamte Win waehrend des Draggens
    // Persistent. Default=FALSE.
    void SetDragStripes(bool bOn);
    bool IsDragStripes() const { return bDragStripes; }

    // Handles waehrend des Draggens verstecken
    //HMHvoid SetDragHdlHide(bool bOn);
    //HMHBOOL IsDragHdlHide() const { return bNoDragHdl; }

    // Beim Draggen von Polygonpunkten und Klebepunkten
    // die Maus verstecken. Default=FALSE
    void SetMouseHideWhileDraggingPoints(bool bOn) { bMouseHideWhileDraggingPoints = bOn; }
    bool IsMouseHideWhileDraggingPoints() const { return bMouseHideWhileDraggingPoints; }

    // Beim Draggen werden i.d.R. die Konturen der markierten Objekte
    // als Xor-Polygone dargestellt. Wird dieses Flag hier gesetzt,
    // wird (z.B. bei Mehrfachselektion) nur ein Xor-Frame gezeichnet.
    // Bei objektspeziefischem Dragging (Polygonpunkte,Eckenradius,...
    // hat diese Einstellung keine Auswirkung.
    // Auch waerend des Draggens umschaltbar.
    // Default=Off
    void SetNoDragXorPolys(bool bOn);
    bool IsNoDragXorPolys() const { return bNoDragXorPolys; }

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
    void SetAutoVertexConnectors(bool bOn) { bAutoVertexCon = bOn; }
    bool IsAutoVertexConnectors() const { return bAutoVertexCon; }

    // Verbinder an Objektecken ankleben
    // Default=sal_False=Nein
    void SetAutoCornerConnectors(bool bOn) { bAutoCornerCon = bOn; }
    bool IsAutoCornerConnectors() const { return bAutoCornerCon; }

    // Dragging von verbundenen Objekten (Nodes):
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // DetailedEdgeDraggingLimit: Wie RubberEdgeDraggingLimit, jedoch bezieht
    // sich dieses Limit auf die detalierte Darstellung, d.h. nicht nur
    // Gummibaender sondern komplette Neuberechnunen sind beim Draggen sichtbar.
    // Diese detalierte Darstellung ist eh nur beim MoveDrag moeglich.
    // Defaultwert ist 10
    bool IsDetailedEdgeDragging() const { return bDetailedEdgeDragging; }

    sal_uInt16 GetDetailedEdgeDraggingLimit() const { return nDetailedEdgeDraggingLimit; }

    // EdgeDraggingLimit: Sind mehr als nEdgeObjAnz Kanten betroffen, werden
    // diese beim interaktiven Draggen nicht mit angezeigt.
    // Gemeint sind hier die "Gummibaender", die weniger Rechenzeit benoetigen
    // als die kompletten Neuberechnungen beim DetailedEdgeDragging.
    // Defaultwert ist 100
    bool IsRubberEdgeDragging() const { return bRubberEdgeDragging; }

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
    void SetMarkedHitMovesAlways(bool bOn) { bMarkedHitMovesAlways = bOn; }
    bool IsMarkedHitMovesAlways() const { return bMarkedHitMovesAlways; }

    // Beim Draggen der Spiegelachse das Spiegelbild der markierten Objekte
    // als Xor darstellen? Persistent. Noch nicht implementiert. Default TRUE.
    void SetMirrRefDragObj(bool bOn) { bMirrRefDragObj = bOn; }
    bool IsMirrRefDragObj() const { return bMirrRefDragObj; }

    bool IsOrthoDesired() const;

    // Beim Resize die Mitte als Referenz
    // Default=FALSE.
    bool IsResizeAtCenter() const { return bResizeAtCenter; }
    void SetResizeAtCenter(bool bOn) { bResizeAtCenter = bOn; }

    // Symmetrisches Crook
    // Default=FALSE.
    bool IsCrookAtCenter() const { return bCrookAtCenter; }
    void SetCrookAtCenter(bool bOn) { bCrookAtCenter = bOn; }

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
    virtual bool TakeDragLimit(SdrDragMode eMode, Rectangle& rRect) const;
};

#endif // INCLUDED_SVX_SVDDRGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
