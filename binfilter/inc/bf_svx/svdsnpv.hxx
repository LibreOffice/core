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

#ifndef _SVDSNPV_HXX
#define _SVDSNPV_HXX

#include <bf_svx/svdpntv.hxx>

#include <bf_svx/svdhlpln.hxx>
namespace binfilter {

//************************************************************
//   Defines
//************************************************************

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

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@  @@  @@  @@@@  @@@@@   @@ @@ @@ @@@@@ @@   @@
// @@  @@ @@@ @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
// @@     @@@@@@ @@  @@ @@  @@  @@ @@ @@ @@    @@ @ @@
//  @@@@  @@@@@@ @@@@@@ @@@@@   @@@@@ @@ @@@@  @@@@@@@
//     @@ @@ @@@ @@  @@ @@       @@@  @@ @@    @@@@@@@
// @@  @@ @@  @@ @@  @@ @@       @@@  @@ @@    @@@ @@@
//  @@@@  @@  @@ @@  @@ @@        @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrSnapView: public SdrPaintView
{
protected:
    SdrPageView*				pDragHelpLinePV;  // Bei verschieben: PageView, sonst NULL

    Size						aMagnSiz;
    Size						aSnapSiz; // muss dann mal raus
#ifndef TF_FRACTIONGRID
    Fraction					aSnapWdtX;
    Fraction					aSnapWdtY;
#endif
    SdrHelpLine					aDragHelpLine;    // Die neue oder Kopie der zu verschiebenden Hilfslinie
    Point						aLastMausPosNoSnap;

    USHORT						nMagnSizPix;
    USHORT						nDragHelpLineNum; // Bei verschieben: Index, sonst undefiniert
    long						nSnapAngle;
    long						nEliminatePolyPointLimitAngle;

    SdrCrookMode				eCrookMode;

    unsigned					bSnapEnab : 1;
    unsigned					bGridSnap : 1;
    unsigned					bSnapTo1Pix : 1;             // Wenn GridSnap aus, auf ein Pixel fangen um Werte wie 10.01 zu vermeiden
    unsigned					bBordSnap : 1;
    unsigned					bHlplSnap : 1;
    unsigned					bOFrmSnap : 1;
    unsigned					bOPntSnap : 1;
    unsigned					bOConSnap : 1;
    unsigned					bMoveMFrmSnap : 1;
    unsigned					bMoveOFrmSnap : 1;
    unsigned					bMoveOPntSnap : 1;
    unsigned					bMoveOConSnap : 1;
    unsigned					bMoveSnapOnlyTopLeft : 1;    //  Speacial fuer den Dialogeditor
    unsigned					bOrtho : 1;
    unsigned					bBigOrtho : 1;
    unsigned					bAngleSnapEnab : 1;
    unsigned					bMoveOnlyDragging : 1;       // Objekte nur verschieben bei Resize/Rotate/...
    unsigned					bSlantButShear : 1;          // Slant anstelle von Shear anwenden
    unsigned					bCrookNoContortion : 1;      // Objekte bei Crook nicht verzerren
    unsigned					bSetPageOrg : 1;             // Interaktives setzen des Seitenursprungs
    unsigned					bDragHelpLine : 1;    // TRUE wenn Drag vorhandene oder neue Hilfslinie
    unsigned					bHlplFixed : 1;       // TRUE=Hilfslinien fixiert, also nicht verschiebbar
    unsigned					bEliminatePolyPoints : 1;

private:
    void ClearVars();

protected:
    // Alles togglen was als Xor im Win, nix merken! NULL=alle OutDev
    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead,SvStream& rIn);
public:
    SdrSnapView(SdrModel* pModel1, OutputDevice* pOut=NULL);

    virtual void ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const;
    virtual BOOL IsAction() const;
    virtual void BrkAction(); // f.abg.Klassen Actions z,B, Draggen abbrechen.

    // Alle Fangeinstellungen sind Persistent.
    /*alt*/void SetSnapGrid(const Size& rSiz) { aSnapSiz=rSiz; SetSnapGridWidth(Fraction(rSiz.Width(),1),Fraction(rSiz.Height(),1)); }
    /*alt*/const Size& GetSnapGrid() const { return aSnapSiz; }

    void SetSnapGridWidth(const Fraction& rX, const Fraction& rY) { aSnapWdtX=rX; aSnapWdtY=rY; SnapMove(); }
    const Fraction& GetSnapGridWidthX() const { return aSnapWdtX; }
    const Fraction& GetSnapGridWidthY() const { return aSnapWdtY; }

    void SetSnapMagnetic(const Size& rSiz) { if (rSiz!=aMagnSiz) { aMagnSiz=rSiz; SnapMove(); } }
    const Size& GetSnapMagnetic() const { return aMagnSiz; }
    void SetSnapMagneticPixel(USHORT nPix) { nMagnSizPix=nPix; }
    USHORT GetSnapMagneticPixel() const { return nMagnSizPix; }

    // RecalcLogicSnapMagnetic muss bei jedem Wechsel des OutputDevices
    // sowie bei jedem Wechsel des MapModes gerufen werden!
    void RecalcLogicSnapMagnetic(const OutputDevice& rOut) { SetSnapMagnetic(rOut.PixelToLogic(Size(nMagnSizPix,nMagnSizPix))); }

    // Auf die View bezogene Koordinaten!
    // Rueckgabewerte sind SDRSNAP_NOTSNAPPED,SDRSNAP_XSNAPPED,
    // SDRSNAP_YSNAPPED oder SDRSNAP_XYSNAPPED
    void SnapMove();

    // Alle Fangeinstellungen sind Persistent.
    BOOL IsSnapEnabled() const { return bSnapEnab; }
    BOOL IsGridSnap() const { return bGridSnap; } // Fang auf Rastergitter
    BOOL IsBordSnap() const { return bBordSnap; } // Fang auf Seitenraender
    BOOL IsHlplSnap() const { return bHlplSnap; } // Fang auf Hilfslinien
    BOOL IsOFrmSnap() const { return bOFrmSnap; } // Fang auf LogFram von umgebenden Zeichenobjekten
    BOOL IsOPntSnap() const { return bOPntSnap; } // Fang auf ausgepraegte Punkte von umgebenden Zeichenobjekten
    BOOL IsOConSnap() const { return bOConSnap; } // Fang auf Konnektoren der Zeichenobjekte
    void SetSnapEnabled(BOOL bOn) { bSnapEnab=bOn; SnapMove(); }
    void SetGridSnap(BOOL bOn) { bGridSnap=bOn; SnapMove(); }
    void SetBordSnap(BOOL bOn) { bBordSnap=bOn; SnapMove(); }
    void SetHlplSnap(BOOL bOn) { bHlplSnap=bOn; SnapMove(); }
    void SetOFrmSnap(BOOL bOn) { bOFrmSnap=bOn; SnapMove(); }
    void SetOPntSnap(BOOL bOn) { bOPntSnap=bOn; SnapMove(); }
    void SetOConSnap(BOOL bOn) { bOConSnap=bOn; SnapMove(); }

    // Normalerweise werden beim Move-Dragging von Zeichenobjekten alle
    // 4 Ecken des Object-SnapRects gefangen. Folgende Einstellmoeglichkeit,
    // wenn man nur auf die linke obere Ecke fangen will (z.B. DialogEditor):
    // Persistent, Default=FALSE.
    void SetMoveSnapOnlyTopLeft(BOOL bOn) { bMoveSnapOnlyTopLeft=bOn; SnapMove(); }
    BOOL IsMoveSnapOnlyTopLeft() const { return bMoveSnapOnlyTopLeft; }

    // Hilfslinien fixiert (nicht verschiebbar)
    // Persistent, Default=FALSE.
    BOOL IsHlplFixed() const { return bHlplFixed; }
    void SetHlplFixed(BOOL bOn) { bHlplFixed=bOn; }

    BOOL IsMoveMFrmSnap() const { return bMoveMFrmSnap; } // Fang des LogFram aller markierten Objekte
    BOOL IsMoveOFrmSnap() const { return bMoveOFrmSnap; } // Fang aller LogFram der markierten Objekte
    BOOL IsMoveOPntSnap() const { return bMoveOPntSnap; } // Fang ausgepraegter Punkte der markierten Objekte
    BOOL IsMoveOConSnap() const { return bMoveOConSnap; } // Fang der Konnektoren der markierten Objekte

    void SetMoveMFrmSnap(BOOL bOn) { bMoveMFrmSnap=bOn; SnapMove(); }
    void SetMoveOFrmSnap(BOOL bOn) { bMoveOFrmSnap=bOn; SnapMove(); }
    void SetMoveOPntSnap(BOOL bOn) { bMoveOPntSnap=bOn; SnapMove(); }
    void SetMoveOConSnap(BOOL bOn) { bMoveOConSnap=bOn; SnapMove(); }

    void BrkSetPageOrg();
    BOOL IsSetPageOrg() const { return bSetPageOrg; }

    // Interaktives einfuegen einer neuen Hilfslinie
    const SdrHelpLine& GetDraggedHelpLine() const { return aDragHelpLine; }
    SdrHelpLineKind GetDraggedHelpLineKind() const { return aDragHelpLine.GetKind(); }

    // Folgende 2 Methoden sind nur gueltig beim Verschieben einer
    // vorhandenen Hilfslinie oder evtl. nach EndDragHelpLine
    USHORT GetDraggedHelpLineIndex() const { return nDragHelpLineNum; }
    SdrPageView* GetDraggedHelpLinePageView() const { return pDragHelpLinePV; }

    // Aendern des Hilfslinientyps waerend des draggens
    void BrkDragHelpLine();
    BOOL IsDragHelpLine() const { return bDragHelpLine; }

    // SnapAngle ist fuer Winkel im Kreis, RotateDragging, ...
    // Der Winkelfang wird unterdrueckt, wenn er mit
    // durch SetAngleSnapEnabled(FALSE) ausgeschaltet ist.
    // Der Winkelfang ist unabhaengig vom Koordinatenfang
    // und somit von der Einstellung IsSnapEnabled()
    // Es sollten nur Werte angegeben werden fuer die gilt:
    //     36000 modulu nWink = 0
    // Implementiert fuer:
    // - Rotate (Dragging)
    // - Shear (Dragging)
    // - Kreisbogen/-sektor/-abschnitt Winkel (Create und Dragging)
    // Persistent.
    void SetAngleSnapEnabled(BOOL bOn) { bAngleSnapEnab=bOn; SnapMove(); }
    BOOL IsAngleSnapEnabled() const { return bAngleSnapEnab; }
    void SetSnapAngle(long nWink) { nSnapAngle=nWink;   SnapMove(); }
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
    void SetOrtho(BOOL bOn) { bOrtho=bOn; } // unvollstaendig
    BOOL IsOrtho() const { return bOrtho; }

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
    void SetBigOrtho(BOOL bOn) { bBigOrtho=bOn; SnapMove(); }
    BOOL IsBigOrtho() const { return bBigOrtho; }

    // bei MoveOnlyDragging=TRUE wird bei Resize/Rotate/Shear/Mirror/Crook
    // nur das Zentrum der markierten Objekte transformiert. Groesse, Form
    // und Drehwinkel der Objekte bleiben erhalten, nur ihre Positionen
    // aendern sich. Persistent. Default=FALSE. (ni)
    void SetMoveOnlyDragging(BOOL bOn) { bMoveOnlyDragging=bOn; }
    BOOL IsMoveOnlyDragging() const { return bMoveOnlyDragging; }

    // Slant anstelle von Shear anwenden. Persistent. Default=FALSE.
    void SetSlantButShear(BOOL bOn) { bSlantButShear=bOn; SnapMove(); }
    BOOL IsSlantButShear() const { return bSlantButShear; }

    // Objekte bei Crook nicht verzerren. Persistent. Default=FALSE. (ni)
    void SetCrookNoContortion(BOOL bOn) { bCrookNoContortion=bOn; SnapMove(); }
    BOOL IsCrookNoContortion() const { return bCrookNoContortion; }

    // Crook-Modus. Persistent. Default=SDRCROOK_ROTATE. (ni)
    void SetCrookMode(SdrCrookMode eMode) { eCrookMode=eMode; SnapMove(); }
    SdrCrookMode GetCrookMode() const { return eCrookMode; }

    // Special fuer IBM: Beim Draggen eines Polygonpunkts wird dieser
    // geloescht, wenn seine beiden angrenzenden Linien eh' fast eine
    // durchgehende Linie sind.
    void SetEliminatePolyPoints(BOOL bOn) { bEliminatePolyPoints=bOn; SnapMove(); }
    BOOL IsEliminatePolyPoints() const { return bEliminatePolyPoints; }
    void SetEliminatePolyPointLimitAngle(long nAngle) { nEliminatePolyPointLimitAngle=nAngle; SnapMove(); }
    long GetEliminatePolyPointLimitAngle() const { return nEliminatePolyPointLimitAngle; }
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

}//end of namespace binfilter
#endif //_SVDSNPV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
