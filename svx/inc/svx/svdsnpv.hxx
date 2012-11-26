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



#ifndef _SVDSNPV_HXX
#define _SVDSNPV_HXX

#include <svx/svdpntv.hxx>
#include <svx/svdhlpln.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class ImplPageOriginOverlay;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines

#define SDRSNAP_NOTSNAPPED  0x0000
#define SDRSNAP_XSNAPPED    0x0001
#define SDRSNAP_YSNAPPED    0x0002

// SDRCROOK_STRETCH ist noch nicht implementiert!
enum SdrCrookMode
{
    SDRCROOK_ROTATE,
    SDRCROOK_SLANT,
    SDRCROOK_STRETCH
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrSnapView: public SdrPaintView
{
private:
protected:
    // #114409#-1 Migrate PageOrigin
    class ImplPageOriginOverlay*            mpPageOriginOverlay;

    // #114409#-2 Migrate HelpLine
    class ImplHelpLineOverlay*              mpHelpLineOverlay;

    // logic grid snap values (grid snap)
    double                      mfSnapWdtX;
    double                      mfSnapWdtY;

    // discrete magnetic snap (helplines, border, frame, point snaps). Logic
    // magnetic snap is calculated on demand using GetFirstOutputDevice()
    sal_uInt16                  mnDiscreteMagneticSnap;

    // angle snap
    sal_Int32                   mnSnapAngle;

    // the angle for interactive point-in-line reduction
    sal_Int32                   mnEliminatePolyPointLimitAngle;

    // crook mode selected at view; not used in the snap implementations
    SdrCrookMode                meCrookMode;

    bool                        mbSnapEnabled : 1;
    bool                        mbGridSnap : 1;
    bool                        mbBorderSnap : 1;
    bool                        mbHelplineSnap : 1;
    bool                        mbOFrameSnap : 1;
    bool                        mbOPointSnap : 1;
    bool                        mbOConnectorSnap : 1;
    bool                        mbMoveSnapOnlyTopLeft : 1;    // dialog editor & report designer
    bool                        mbOrthogonal : 1;
    bool                        mbBigOrthogonal : 1;
    bool                        mbAngleSnap : 1;
    bool                        mbMoveOnlyDragging : 1;       // Objekte nur verschieben bei Resize/Rotate/...
    bool                        mbSlantButShear : 1;          // Slant anstelle von Shear anwenden
    bool                        mbCrookNoContortion : 1;      // Objekte bei Crook nicht verzerren
    bool                        mbHelplinesFixed : 1;       // true=Hilfslinien fixiert, also nicht verschiebbar
    bool                        mbEliminatePolyPoints : 1;

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrSnapView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrSnapView();

public:
    virtual bool IsAction() const;
    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction(); // f.abg.Klassen Actions z,B, Draggen abbrechen.
    virtual basegfx::B2DRange TakeActionRange() const;

    void SetSnapGridWidth(double fX, double fY) { mfSnapWdtX = fX; mfSnapWdtY = fY; }
    double GetSnapGridWidthX() const { return mfSnapWdtX; }
    double GetSnapGridWidthY() const { return mfSnapWdtY; }

    void SetDiscreteMagneticSnap(sal_uInt16 nPix) { mnDiscreteMagneticSnap = nPix; }
    sal_uInt16 GetDiscreteMagneticSnap() const { return mnDiscreteMagneticSnap; }

    // Auf die View bezogene Koordinaten!
    // Rueckgabewerte sind SDRSNAP_NOTSNAPPED,SDRSNAP_XSNAPPED,
    // SDRSNAP_YSNAPPED oder SDRSNAP_XYSNAPPED
    basegfx::B2DPoint GetSnapPos(const basegfx::B2DPoint& rPnt, sal_uInt16* pSnapFlags = 0) const;

    // Alle Fangeinstellungen sind Persistent.
    bool IsSnapEnabled() const { return mbSnapEnabled; }
    bool IsGridSnap() const { return mbGridSnap; } // Fang auf Rastergitter
    bool IsBorderSnap() const { return mbBorderSnap; } // Fang auf Seitenraender
    bool IsHelplineSnap() const { return mbHelplineSnap; } // Fang auf Hilfslinien
    bool IsOFrameSnap() const { return mbOFrameSnap; } // Fang auf LogFram von umgebenden Zeichenobjekten
    bool IsOPointSnap() const { return mbOPointSnap; } // Fang auf ausgepraegte Punkte von umgebenden Zeichenobjekten
    bool IsOConnectorSnap() const { return mbOConnectorSnap; } // Fang auf Konnektoren der Zeichenobjekte
    void SetSnapEnabled(bool bOn) { if(mbSnapEnabled != bOn) mbSnapEnabled = bOn; }
    void SetGridSnap(bool bOn) { if(mbGridSnap != bOn) mbGridSnap = bOn; }
    void SetBorderSnap(bool bOn) { mbBorderSnap = bOn; }
    void SetHelplineSnap(bool bOn) { if(mbHelplineSnap != bOn) mbHelplineSnap = bOn; }
    void SetOFrameSnap(bool bOn) { if(mbOFrameSnap != bOn) mbOFrameSnap = bOn; }
    void SetOPointSnap(bool bOn) { if(mbOPointSnap != bOn) mbOPointSnap = bOn; }
    void SetOConnectorSnap(bool bOn) { if(mbOConnectorSnap != bOn) mbOConnectorSnap = bOn; }

    // Normalerweise werden beim Move-Dragging von Zeichenobjekten alle
    // 4 Ecken des Object-SnapRects gefangen. Folgende Einstellmoeglichkeit,
    // wenn man nur auf die linke obere Ecke fangen will (z.B. DialogEditor):
    // Persistent, Default=false.
    void SetMoveSnapOnlyTopLeft(bool bOn) { if(mbMoveSnapOnlyTopLeft != bOn) mbMoveSnapOnlyTopLeft = bOn; }
    bool IsMoveSnapOnlyTopLeft() const { return mbMoveSnapOnlyTopLeft; }

    // Hilfslinien fixiert (nicht verschiebbar)
    // Persistent, Default=false.
    bool AreHelplinesFixed() const { return mbHelplinesFixed; }
    void SetHelplinesFixed(bool bOn) { if(mbHelplinesFixed != bOn) mbHelplinesFixed = bOn; }

    // #114409#-1 Migrate PageOrigin
    bool BegSetPageOrg(const basegfx::B2DPoint& rPnt);
    void MovSetPageOrg(const basegfx::B2DPoint& rPnt);
    bool EndSetPageOrg();
    void BrkSetPageOrg();
    bool IsSetPageOrg() const { return (0L != mpPageOriginOverlay); }

    // HitTest. Bei true steht in rnHelpLineNum die Nummer der Hilfslinie
    bool PickHelpLine(const basegfx::B2DPoint& rPnt, double fTol, sal_uInt32& rnHelpLineNum) const;

    // Verschieben einer vorhandenen Hilfslinie. nHelpLineNum von PickHelpLine verwenden.
    bool BegDragHelpLine(sal_uInt16 nHelpLineNum);
    // Interaktives einfuegen einer neuen Hilfslinie
    bool BegDragHelpLine(const basegfx::B2DPoint& rPnt, SdrHelpLineKind eNewKind);
    Pointer GetDraggedHelpLinePointer() const;

    // Aendern des Hilfslinientyps waerend des draggens
    // void SetDraggedHelpLineKind(SdrHelpLineKind eNewKind);
    void MovDragHelpLine(const basegfx::B2DPoint& rPnt);
    bool EndDragHelpLine();
    void BrkDragHelpLine();
    bool IsDragHelpLine() const { return (0 != mpHelpLineOverlay); }

    // SnapAngle ist fuer Winkel im Kreis, RotateDragging, ...
    // Der Winkelfang wird unterdrueckt, wenn er mit
    // durch SetAngleSnapEnabled(false) ausgeschaltet ist.
    // Der Winkelfang ist unabhaengig vom Koordinatenfang
    // und somit von der Einstellung IsSnapEnabled()
    // Es sollten nur Werte angegeben werden fuer die gilt:
    //     36000 modulu nWink = 0
    // Implementiert fuer:
    // - Rotate (Dragging)
    // - Shear (Dragging)
    // - Kreisbogen/-sektor/-abschnitt Winkel (Create und Dragging)
    // Persistent.
    void SetAngleSnapEnabled(bool bOn) { if(mbAngleSnap != bOn) mbAngleSnap = bOn; }
    bool IsAngleSnapEnabled() const { return mbAngleSnap; }
    void SetSnapAngle(sal_Int32 nWink) { if(mnSnapAngle != nWink) mnSnapAngle = nWink; }
    sal_Int32 GetSnapAngle() const { return mnSnapAngle; }

    // Ortho hat je nach Kontext verschiedene Effekte:
    // - Create
    //   - Linien werden nur im 45deg Raster zugelassen
    //   - Statt Rechtecke werden Quadrate erzeugt
    //   - Statt Ellipsen werden Kreise erzeugt
    // - Dragging
    //   - allgemeines Dragging
    //     - Move nur Hor, Vert oder 45deg
    //     - Resize proportional
    //     - Mirror: nichts
    //     - Shear ohne Resize
    //     - Crook ohne Resize
    //   - verschieben der Handles
    //     - Spiegelachse nur 45deg Raster
    //   - Objekteigenes Dragging
    //     - Rechteck Eckenradius: nichts
    //     - Kreisobjekt Winkel: nichts
    //     - Linie behaelt beim Draggen ihren Winkel bei und wird nur    (ni)
    //       verlaengert bzw. verkuerzt.
    // Defaultmaessig ist Ortho ausgeschaltet. Persistent.
    void SetOrthogonal(bool bOn) { if(mbOrthogonal != bOn) mbOrthogonal = bOn; } // unvollstaendig
    bool IsOrthogonal() const { return mbOrthogonal; }

    // BigOrtho hat nur Relevanz wenn Ortho eingeschaltet ist.
    // Beispiel: Ein Rechteck wird mit eingeschaltetem Ortho (also ein Quadrat)
    //   erzeugt und die Maus wurde dabei vom Nullpunkt zu den Koordinaten
    //   (80,30) gedraggt. Dann stuenden nun 2 Alternativen zur Bestimmung der
    //   Kantenlaenge des Quadrats zur Wahl: 30 und 80.
    //   Die normale Ortho-Funktuionalitaet brachte hierbei ein Quadrat mit
    //   Kantenlaenge 30 (also immer die kleinere Groesse). Bei hinzugeschal-
    //   tetem BigOrtho bekaeme man dagegen ein Quadrat der Kantenlaenge 80.
    // Gleiches gilt auch fuer Resize.
    // Defaultmaessig ist BigOrtho eingeschaltet. Persistent.
    void SetBigOrthogonal(bool bOn) { if(mbBigOrthogonal != bOn) mbBigOrthogonal = bOn; }
    bool IsBigOrthogonal() const { return mbBigOrthogonal; }

    // bei MoveOnlyDragging=true wird bei Resize/Rotate/Shear/Mirror/Crook
    // nur das Zentrum der markierten Objekte transformiert. Groesse, Form
    // und Drehwinkel der Objekte bleiben erhalten, nur ihre Positionen
    // aendern sich. Persistent. Default=false. (ni)
    void SetMoveOnlyDragging(bool bOn) { if(mbMoveOnlyDragging != bOn) mbMoveOnlyDragging = bOn; }
    bool IsMoveOnlyDragging() const { return mbMoveOnlyDragging; }

    // Slant anstelle von Shear anwenden. Persistent. Default=false.
    void SetSlantButShear(bool bOn) { if(mbSlantButShear != bOn) mbSlantButShear = bOn; }
    bool IsSlantButShear() const { return mbSlantButShear; }

    // Objekte bei Crook nicht verzerren. Persistent. Default=false. (ni)
    void SetCrookNoContortion(bool bOn) { if(mbCrookNoContortion != bOn) mbCrookNoContortion = bOn; }
    bool IsCrookNoContortion() const { return mbCrookNoContortion; }

    // Crook-Modus. Persistent. Default=SDRCROOK_ROTATE. (ni)
    void SetCrookMode(SdrCrookMode eMode) { if(meCrookMode != eMode) meCrookMode = eMode; }
    SdrCrookMode GetCrookMode() const { return meCrookMode; }

    // Special fuer IBM: Beim Draggen eines Polygonpunkts wird dieser
    // geloescht, wenn seine beiden angrenzenden Linien eh' fast eine
    // durchgehende Linie sind.
    void SetEliminatePolyPoints(bool bOn) { if(mbEliminatePolyPoints != bOn) mbEliminatePolyPoints = bOn; }
    bool IsEliminatePolyPoints() const { return mbEliminatePolyPoints; }
    void SetEliminatePolyPointLimitAngle(sal_Int32 nAngle) { mnEliminatePolyPointLimitAngle=nAngle; }
    sal_Int32 GetEliminatePolyPointLimitAngle() const { return mnEliminatePolyPointLimitAngle; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Begriffsdefinition:
//   - Etwas fangen=Gefangen werden kann z.B. der Mauszeiger oder die z.Zt. im
//     Drag befindlichen markierten Objekte.
//   - Auf etwas fangen=Man kann z.B. auf das Grid oder auf Hilfslinien fangen.
//
// Grundsaetzlich wird nur gefangen auf sichtbare Elemente (-> Border,
// Hilfslinien, Konnektoren; Ausnahme: Grid). Ebenso koennen nur sichtbare
// Elemente gefangen werden (->Konnektoren).
//
// Auf's Grid wird immer erst dann gefangen, wenn nix Anderes in der Naehe
// (->Magnetic) ist.
//
// Der "Cursor" (also der Mauszeiger) beim Erzeugen von Objekten, beim Draggen
// von Polygonpunkten, ... wird immer auf allen eingeschalteten Fangalternativen
// gefangen (max 6).
//
// Beim Verschieben markierter Objekte ist das etwas anders. Statt des einen
// Mauscursors gibt es hier 4 Alternativen an den markierten Objekten, die
// gefangen werden koennen:
//   1. die logisch-umschliessenden Rahmen der einzelnen Objekte
//   2. der logisch-umschliessende Rahmen aller markierten Objekte
//   3. ausgezeichnete Punkte der markierten Objekte (Polygonpunkte, ...)
//   4. die Konnektoren der markierten Objekte
// Da 1. und 2. einander ausschliessen (2. ist eine Verfeinerung von 1.)
// bleiben 3 voneinander unabhaengige Alternativen. Bei 6. Moeglichkeiten auf
// die gefangen werden kann kaeme man auf max. 18 Kombinationsmoeglichkeiten!
// Deshalb werden folgende Vereinfachungen festgelegt:
//   1. Konnektoren fangen sich nur auf Konnektoren.
// Verbleiben also nun noch max. 2x5+1=11 Fangkombinationen beim MoveDrag:
//   1-3.  umschliessende(r) Rahmen auf Grid/Border/Hilfslinien
//   4.    umschliessende(r) Rahmen auf ausgezeichnete Objektpunkte
//   5.    umschliessende(r) Rahmen auf umschliessenden Rahmen
//   6-8.  ausgezeichnete Punkte auf Grid/Border/Hilfslinien
//   7.    ausgezeichnete Punkte auf ausgezeichnete Objektpunkte
//   8-10. ausgezeichnete Punkte auf umschliessenden Rahmen
//   11.   Konnektoren auf Konnektoren
// Beim MouseMove-Event im DragMove werden also diese bis zu max. 11 moeglichen
// Alternativen durchgetestet und die mit dem gerigsten Korrekturaufwand
// vollzogen.
//
// Beim Resize, ... wird immer nur der logisch-umschliessende Rahmen der
// markierten Objekte gefangen.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDSNPV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
