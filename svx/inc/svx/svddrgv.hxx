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



#ifndef _SVDDRGV_HXX
#define _SVDDRGV_HXX

#include "svx/svxdllapi.h"
#include <svx/svdxcgv.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrUndoGeoObj;
class ImpSdrDragViewExtraData;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrDragView: public SdrExchangeView
{
private:
protected:
    SdrHdl*                     mpDragHdl;
    SdrDragMethod*              mpCurrentSdrDragMethod;
    SdrUndoGeoObj*              mpInsPointUndo;
    basegfx::B2DRange           maDragLimit;
    XubString                   maInsPointUndoStr;
    SdrHdlKind                  meDragHdl;

    /// bitfield
    bool                        mbFrameDrag : 1;        // z.Zt. FrameDrag
    bool                        mbDragSpecial : 1;     // z.Zt. Special Obj-Dragging
    bool                        mbMarkedHitMovesAlways : 1; // Persistent
    bool                        mbDragLimit : 1;
    bool                        mbDragHdl : 1;        // true: RefPt wird verschoben
    bool                        mbDragStripes : 1;    // Persistent
    bool                        mbSolidDragging : 1;  // allow solid create/drag of objects
    bool                        mbResizeAtCenter : 1;
    bool                        mbCrookAtCenter : 1;
    bool                        mbDragWithCopy : 1;
    bool                        mbInsGluePoint : 1;
    bool                        mbInsObjPointMode : 1;
    bool                        mbInsGluePointMode : 1;
    bool                        mbNoDragXorPolys : 1;

    bool ImpBegInsObjPoint(bool bIdxZwang, sal_uInt32 nIdx, const basegfx::B2DPoint& rPnt, bool bNewObj);

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrDragView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrDragView();

public:
    virtual void SetMarkHandles();

    virtual bool IsAction() const;
    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual basegfx::B2DRange TakeActionRange() const;

    void ShowDragObj();
    void HideDragObj();

    // Spezialimplementation fuer den Writer:
    // TakeDragObjAnchorPos() liefert die Position an der ein Objekt
    // beim Draggen einer Einfachselektion ungefaehr landet wenn es
    // "losgelassen" wird (EndDrag).
    // In der Regel ist das die linke obere Ecke des zu erwartenden neuen
    // SnapRects. Ausnahme: CaptionObj. Dort ist es die Position des
    // "Schwanzendes".
    // Bei Returncode false konnte ich die Position nicht bestimmen
    // (z.B. Punktverschiebung, Mehrfachselektion, Schieben der
    // Spiegelschse, ...)
    bool TakeDragObjAnchorPos(basegfx::B2DPoint& rPos, bool bTopRight = false ) const;

    void SetInsertGluePoint(bool bOn) { if(mbInsGluePoint != bOn) mbInsGluePoint = bOn; }
    bool IsInsertGluePoint() const { return mbInsGluePoint; }

    // Wird pForcedMeth uebergeben, so wird pHdl, ... nicht ausgewerten, sondern diese
    // Drag-Methode verwendet. Die Instanz geht dabei ins Eigentum der View ueber und
    // wird zum Ende des Draggings destruiert.
    virtual bool BegDragObj(const basegfx::B2DPoint& rPnt, const SdrHdl* pHdl = 0, double fMinMovLogic = 3.0,
        SdrDragMethod* pForcedMeth = 0);
    void MovDragObj(const basegfx::B2DPoint& rPnt);
    bool EndDragObj(bool bCopy = false);
    void BrkDragObj();
    bool IsDragObj() const { return mpCurrentSdrDragMethod && !mbInsPolyPoint && !IsInsertGluePoint(); }
    SdrHdl* GetDragHdl() const { return mpDragHdl; }
    SdrDragMethod* GetDragMethod() const { return mpCurrentSdrDragMethod; }
    bool IsDraggingPoints() const { return HDL_POLY == meDragHdl; }
    bool IsDraggingGluePoints() const { return HDL_GLUE == meDragHdl; }

    SdrHdlKind GetDragHdlKind() const { return meDragHdl; }
    bool IsDragLimit() const { return mbDragLimit; }
    const basegfx::B2DRange& GetDragLimit() const { return maDragLimit; }

    // Wer das beim BegDrag oder mittendrin schon festlegen will.
    // (Wird bei jedem BegDrag auf false zurueckgesetzt, also nach
    // BegDrag setzen.)
    void SetDragWithCopy(bool bOn) { if(mbDragWithCopy != bOn) mbDragWithCopy = bOn; }
    bool IsDragWithCopy() const { return mbDragWithCopy; }

    // Interaktives einfuegen eines neuen Punktes. nIdx=0 => vor dem ersten Punkt.
    bool IsInsObjPointPossible() const;
    bool IsInsPointPossible() const { return IsInsObjPointPossible(); }
    bool BegInsObjPoint(const basegfx::B2DPoint& rPnt, bool bNewObj) { return ImpBegInsObjPoint(false, 0, rPnt, bNewObj); }
    void MovInsObjPoint(const basegfx::B2DPoint& rPnt) { MovDragObj(rPnt); }
    bool EndInsObjPoint(SdrCreateCmd eCmd);
    void BrkInsObjPoint() { BrkDragObj(); }
    bool IsInsObjPoint() const { return mpCurrentSdrDragMethod && mbInsPolyPoint; }

    // Fuer die App zum Verwalten des Status. GetPreferedPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsObjPointMode(bool bOn) { if(mbInsObjPointMode != bOn) mbInsObjPointMode = bOn; }
    bool IsInsObjPointMode() const { return mbInsObjPointMode; }

    bool IsInsGluePointPossible() const;
    bool BegInsGluePoint(const basegfx::B2DPoint& rPnt);
    void MovInsGluePoint(const basegfx::B2DPoint& rPnt) { MovDragObj(rPnt); }
    bool EndInsGluePoint() { return EndDragObj(); }
    void BrkInsGluePoint() { BrkDragObj(); }

    // Fuer die App zum Verwalten des Status. GetPreferedPointer() wird
    // spaeter vielleicht einen passenden Pointer dafuer liefern
    void SetInsGluePointMode(bool bOn) { if(mbInsGluePointMode != bOn) mbInsGluePointMode = bOn; }
    bool IsInsGluePointMode() const { return mbInsGluePointMode; }

    // Begrenzungslinien ueber's gesamte Win waehrend des Draggens
    void SetDragStripes(bool bOn);
    bool IsDragStripes() const { return mbDragStripes; }

    // Beim Draggen werden i.d.R. die Konturen der markierten Objekte
    // als Xor-Polygone dargestellt. Wird dieses Flag hier gesetzt,
    // wird (z.B. bei Mehrfachselektion) nur ein Xor-Frame gezeichnet.
    // Bei objektspeziefischem Dragging (Polygonpunkte,Eckenradius,...
    // hat diese Einstellung keine Auswirkung.
    // Auch waerend des Draggens umschaltbar.
    // Default=Off
    void SetNoDragXorPolys(bool bOn);
    bool IsNoDragXorPolys() const { return mbNoDragXorPolys; }

    void SetSolidDragging(bool bOn) { if(mbSolidDragging != bOn) mbSolidDragging = bOn; }
    bool IsSolidDragging() const;

    // Ist ein spezieller Dragmode eingeschaltet, wie Rotate, Mirror oder Crook,
    // dann leitet ein Hit auf das markierte Objekt genau dieses Dragging ein.
    // Setzt man MarkedHitMovesAlways auf true, so leitet ein Hit auf das
    // markierte Objekt immer ein Moven ein, unabhaengig vom gesetzten DragMode.
    // Dieses Flag ist persistent und sollte von der App fuer den Anwender
    // konfigurierbar sein!
    void SetMarkedHitMovesAlways(bool bOn) { if(mbMarkedHitMovesAlways != bOn) mbMarkedHitMovesAlways = bOn; }
    bool IsMarkedHitMovesAlways() const { return mbMarkedHitMovesAlways; }

    bool IsOrthoDesired() const;

    // Beim Resize die Mitte als Referenz
    // Default=false.
    bool IsResizeAtCenter() const { return mbResizeAtCenter; }
    void SetResizeAtCenter(bool bOn) { if(mbResizeAtCenter != bOn) mbResizeAtCenter = bOn; }

    // Symmetrisches Crook
    // Default=false.
    bool IsCrookAtCenter() const { return mbCrookAtCenter; }
    void SetCrookAtCenter(bool bOn) { if(mbCrookAtCenter != bOn) mbCrookAtCenter = bOn; }

    // Das DragLimit ist bezogen auf die Page des Objekts.
    // (Oder auf die View??? Muss ich mal^^^^recherchieren. Joe.)
    // false=Kein Limit.
    // Das Rueckgabe-Rect muss absolute Koordinaten enthalten. Der Maximale
    // Dragbereich wird von der View dann so gewaehlt, dass das SnapRect des
    // Objekts bis Maximal auf die Kante des LimitRects gemoved bzw. gesized
    // wird. Bei Objekten wie Bezierkurven, gedrehten Rechtecken ist zu beachten
    // das durch anschliessende Neuberechnung des SnapRects (bei Resize)
    // Rundungsfehler auftreten koennen, wodurch das LimitRect minnimal
    // ueberschritten werden koennte...
    // Implementiert fuer Move und Resize.
    virtual bool TakeDragLimit(SdrDragMode eMode, basegfx::B2DRange& rRange) const;
};

#endif //_SVDDRGV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
