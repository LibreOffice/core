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

#ifndef INCLUDED_SVX_SVDSNPV_HXX
#define INCLUDED_SVX_SVDSNPV_HXX

#include <svx/svdpntv.hxx>
#include <svx/svdhlpln.hxx>
#include <svx/svxdllapi.h>


#define SDRSNAP_NOTSNAPPED  0x0000
#define SDRSNAP_XSNAPPED    0x0001
#define SDRSNAP_YSNAPPED    0x0002
#define SDRSNAP_XYSNAPPED   0x0003

// SDRCROOK_STRETCH ist noch nicht implementiert!
enum SdrCrookMode {
    SDRCROOK_ROTATE,
    SDRCROOK_SLANT,
    SDRCROOK_STRETCH
};


// #114409#-1 Migrate PageOrigin
class ImplPageOriginOverlay;

class SVX_DLLPUBLIC SdrSnapView: public SdrPaintView
{
protected:
    // #114409#-1 Migrate PageOrigin
    class ImplPageOriginOverlay*            mpPageOriginOverlay;

    // #114409#-2 Migrate HelpLine
    class ImplHelpLineOverlay*              mpHelpLineOverlay;

    Size                        aMagnSiz;
    Fraction                    aSnapWdtX;
    Fraction                    aSnapWdtY;

    sal_uInt16                      nMagnSizPix;
    long                        nSnapAngle;
    long                        nEliminatePolyPointLimitAngle;

    SdrCrookMode                eCrookMode;

    bool                        bSnapEnab : 1;
    bool                        bGridSnap : 1;
    bool                        bSnapTo1Pix : 1;             // Wenn GridSnap aus, auf ein Pixel fangen um Werte wie 10.01 zu vermeiden
    bool                        bBordSnap : 1;
    bool                        bHlplSnap : 1;
    bool                        bOFrmSnap : 1;
    bool                        bOPntSnap : 1;
    bool                        bOConSnap : 1;
    bool                        bMoveMFrmSnap : 1;
    bool                        bMoveOFrmSnap : 1;
    bool                        bMoveOPntSnap : 1;
    bool                        bMoveOConSnap : 1;
    bool                        bMoveSnapOnlyTopLeft : 1;    //  Speacial fuer den Dialogeditor
    bool                        bOrtho : 1;
    bool                        bBigOrtho : 1;
    bool                        bAngleSnapEnab : 1;
    bool                        bMoveOnlyDragging : 1;       // Objekte nur verschieben bei Resize/Rotate/...
    bool                        bSlantButShear : 1;          // Slant anstelle von Shear anwenden
    bool                        bCrookNoContortion : 1;      // Objekte bei Crook nicht verzerren
    bool                        bHlplFixed : 1;       // sal_True=Hilfslinien fixiert, also nicht verschiebbar
    bool                        bEliminatePolyPoints : 1;

private:
    SVX_DLLPRIVATE void ClearVars();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrSnapView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrSnapView();

public:
    virtual bool IsAction() const SAL_OVERRIDE;
    virtual void MovAction(const Point& rPnt) SAL_OVERRIDE;
    virtual void EndAction() SAL_OVERRIDE;
    virtual void BckAction() SAL_OVERRIDE;
    virtual void BrkAction() SAL_OVERRIDE; // f.abg.Klassen Actions z,B, Draggen abbrechen.
    virtual void TakeActionRect(Rectangle& rRect) const SAL_OVERRIDE;

    void SetSnapGridWidth(const Fraction& rX, const Fraction& rY) { aSnapWdtX=rX; aSnapWdtY=rY; }
    const Fraction& GetSnapGridWidthX() const { return aSnapWdtX; }
    const Fraction& GetSnapGridWidthY() const { return aSnapWdtY; }

    void SetSnapMagnetic(const Size& rSiz) { if (rSiz!=aMagnSiz) { aMagnSiz=rSiz; } }
    const Size& GetSnapMagnetic() const { return aMagnSiz; }
    void SetSnapMagneticPixel(sal_uInt16 nPix) { nMagnSizPix=nPix; }
    sal_uInt16 GetSnapMagneticPixel() const { return nMagnSizPix; }

    // RecalcLogicSnapMagnetic muss bei jedem Wechsel des OutputDevices
    // sowie bei jedem Wechsel des MapModes gerufen werden!
    void RecalcLogicSnapMagnetic(const OutputDevice& rOut) { SetSnapMagnetic(rOut.PixelToLogic(Size(nMagnSizPix,nMagnSizPix))); }
    void SetActualWin(const OutputDevice* pWin) { SdrPaintView::SetActualWin(pWin); if (pWin!=NULL) RecalcLogicSnapMagnetic(*pWin); }

    // Auf die View bezogene Koordinaten!
    // Rueckgabewerte sind SDRSNAP_NOTSNAPPED,SDRSNAP_XSNAPPED,
    // SDRSNAP_YSNAPPED oder SDRSNAP_XYSNAPPED
    sal_uInt16 SnapPos(Point& rPnt, const SdrPageView* pPV) const;
    Point GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const;
    void CheckSnap(const Point& rPt, const SdrPageView* pPV, long& nBestXSnap, long& nBestYSnap, bool& bXSnapped, bool& bYSnapped) const;

    // Alle Fangeinstellungen sind Persistent.
    bool IsSnapEnabled() const { return bSnapEnab; }
    bool IsGridSnap() const { return bGridSnap; } // Fang auf Rastergitter
    bool IsBordSnap() const { return bBordSnap; } // Fang auf Seitenraender
    bool IsHlplSnap() const { return bHlplSnap; } // Fang auf Hilfslinien
    bool IsOFrmSnap() const { return bOFrmSnap; } // Fang auf LogFram von umgebenden Zeichenobjekten
    bool IsOPntSnap() const { return bOPntSnap; } // Fang auf ausgepraegte Punkte von umgebenden Zeichenobjekten
    bool IsOConSnap() const { return bOConSnap; } // Fang auf Konnektoren der Zeichenobjekte
    void SetSnapEnabled(bool bOn) { bSnapEnab=bOn; }
    void SetGridSnap(bool bOn) { bGridSnap=bOn; }
    void SetBordSnap(bool bOn) { bBordSnap=bOn; }
    void SetHlplSnap(bool bOn) { bHlplSnap=bOn; }
    void SetOFrmSnap(bool bOn) { bOFrmSnap=bOn; }
    void SetOPntSnap(bool bOn) { bOPntSnap=bOn; }
    void SetOConSnap(bool bOn) { bOConSnap=bOn; }

    // Normalerweise werden beim Move-Dragging von Zeichenobjekten alle
    // 4 Ecken des Object-SnapRects gefangen. Folgende Einstellmoeglichkeit,
    // wenn man nur auf die linke obere Ecke fangen will (z.B. DialogEditor):
    // Persistent, Default=FALSE.
    void SetMoveSnapOnlyTopLeft(bool bOn) { bMoveSnapOnlyTopLeft=bOn; }
    bool IsMoveSnapOnlyTopLeft() const { return bMoveSnapOnlyTopLeft; }

    // Hilfslinien fixiert (nicht verschiebbar)
    // Persistent, Default=FALSE.
    bool IsHlplFixed() const { return bHlplFixed; }
    void SetHlplFixed(bool bOn) { bHlplFixed=bOn; }

    bool IsMoveMFrmSnap() const { return bMoveMFrmSnap; } // Fang des LogFram aller markierten Objekte
    bool IsMoveOFrmSnap() const { return bMoveOFrmSnap; } // Fang aller LogFram der markierten Objekte
    bool IsMoveOPntSnap() const { return bMoveOPntSnap; } // Fang ausgepraegter Punkte der markierten Objekte
    bool IsMoveOConSnap() const { return bMoveOConSnap; } // Fang der Konnektoren der markierten Objekte

    void SetMoveMFrmSnap(bool bOn) { bMoveMFrmSnap=bOn; }
    void SetMoveOFrmSnap(bool bOn) { bMoveOFrmSnap=bOn; }
    void SetMoveOPntSnap(bool bOn) { bMoveOPntSnap=bOn; }
    void SetMoveOConSnap(bool bOn) { bMoveOConSnap=bOn; }

    // #114409#-1 Migrate PageOrigin
    bool BegSetPageOrg(const Point& rPnt);
    void MovSetPageOrg(const Point& rPnt);
    bool EndSetPageOrg();
    void BrkSetPageOrg();
    bool IsSetPageOrg() const { return (0L != mpPageOriginOverlay); }

    // HitTest. Bei sal_True steht in rnHelpLineNum die Nummer der Hilfslinie und in rpPV
    // die zugehoerige PageView.
    bool PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, sal_uInt16& rnHelpLineNum, SdrPageView*& rpPV) const;

    // Verschieben einer vorhandenen Hilfslinie. nHelpLineNum und pPV von PickHelpLine verwenden.
    bool BegDragHelpLine(sal_uInt16 nHelpLineNum, SdrPageView* pPV);
    // Interaktives einfuegen einer neuen Hilfslinie
    bool BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind);
    Pointer GetDraggedHelpLinePointer() const;

    // Aendern des Hilfslinientyps waerend des draggens
    // void SetDraggedHelpLineKind(SdrHelpLineKind eNewKind);
    void MovDragHelpLine(const Point& rPnt);
    bool EndDragHelpLine();
    void BrkDragHelpLine();
    bool IsDragHelpLine() const { return (0L != mpHelpLineOverlay); }

    // SnapAngle ist fuer Winkel im Kreis, RotateDragging, ...
    // Der Winkelfang wird unterdrueckt, wenn er mit
    // durch SetAngleSnapEnabled(sal_False) ausgeschaltet ist.
    // Der Winkelfang ist unabhaengig vom Koordinatenfang
    // und somit von der Einstellung IsSnapEnabled()
    // Es sollten nur Werte angegeben werden fuer die gilt:
    //     36000 modulu nWink = 0
    // Implementiert fuer:
    // - Rotate (Dragging)
    // - Shear (Dragging)
    // - Kreisbogen/-sektor/-abschnitt Winkel (Create und Dragging)
    // Persistent.
    void SetAngleSnapEnabled(bool bOn) { bAngleSnapEnab=bOn; }
    bool IsAngleSnapEnabled() const { return bAngleSnapEnab; }
    void SetSnapAngle(long nWink) { nSnapAngle=nWink; }
    long GetSnapAngle() const { return nSnapAngle; }

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
    void SetOrtho(bool bOn) { bOrtho=bOn; } // unvollstaendig
    bool IsOrtho() const { return bOrtho; }

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
    void SetBigOrtho(bool bOn) { bBigOrtho=bOn; }
    bool IsBigOrtho() const { return bBigOrtho; }

    // bei MoveOnlyDragging=sal_True wird bei Resize/Rotate/Shear/Mirror/Crook
    // nur das Zentrum der markierten Objekte transformiert. Groesse, Form
    // und Drehwinkel der Objekte bleiben erhalten, nur ihre Positionen
    // aendern sich. Persistent. Default=FALSE. (ni)
    void SetMoveOnlyDragging(bool bOn) { bMoveOnlyDragging=bOn; }
    bool IsMoveOnlyDragging() const { return bMoveOnlyDragging; }

    // Slant anstelle von Shear anwenden. Persistent. Default=FALSE.
    void SetSlantButShear(bool bOn) { bSlantButShear=bOn; }
    bool IsSlantButShear() const { return bSlantButShear; }

    // Objekte bei Crook nicht verzerren. Persistent. Default=FALSE. (ni)
    void SetCrookNoContortion(bool bOn) { bCrookNoContortion=bOn; }
    bool IsCrookNoContortion() const { return bCrookNoContortion; }

    // Crook-Modus. Persistent. Default=SDRCROOK_ROTATE. (ni)
    void SetCrookMode(SdrCrookMode eMode) { eCrookMode=eMode; }
    SdrCrookMode GetCrookMode() const { return eCrookMode; }

    // Special fuer IBM: Beim Draggen eines Polygonpunkts wird dieser
    // geloescht, wenn seine beiden angrenzenden Linien eh' fast eine
    // durchgehende Linie sind.
    void SetEliminatePolyPoints(bool bOn) { bEliminatePolyPoints=bOn; }
    bool IsEliminatePolyPoints() const { return bEliminatePolyPoints; }
    void SetEliminatePolyPointLimitAngle(long nAngle) { nEliminatePolyPointLimitAngle=nAngle; }
    long GetEliminatePolyPointLimitAngle() const { return nEliminatePolyPointLimitAngle; }
};

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

#endif // INCLUDED_SVX_SVDSNPV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
