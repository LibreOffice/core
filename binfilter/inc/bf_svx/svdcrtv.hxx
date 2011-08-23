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

#ifndef _SVDCRTV_HXX
#define _SVDCRTV_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <bf_svx/svdhdl.hxx>

#include <bf_svx/svddrgv.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XLineAttrSetItem;
class XFillAttrSetItem;
class SdrEdgeObj;
class SdrObjConnection;
class ImpSdrConnectMarker;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@  @@@@  @@@@@@ @@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@   @@
//  @@     @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@ @ @@
//  @@     @@@@@  @@@@  @@@@@@   @@   @@@@   @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@ @@@
//   @@@@  @@  @@ @@@@@ @@  @@   @@   @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrCreateView: public SdrDragView
{
    friend class				SdrPageView;
    friend class				ImpSdrCreateLibObjMove;
    friend class				ImpSdrCreateLibObjResize;

protected:
    SdrObject*					pAktCreate;   // Aktuell in Erzeugung befindliches Objekt
    SdrPageView*				pCreatePV;    // Hier wurde die Erzeugung gestartet
    SdrObject*					pCurrentLibObj;
    SdrDragMethod*				pLibObjDragMeth;
    ImpSdrConnectMarker*		pConnectMarker;

    Pointer						aAktCreatePointer;

    INT32						nAutoCloseDistPix;
    INT32						nFreeHandMinDistPix;
    UINT32						nAktInvent;     // Aktuell eingestelltes
    UINT16						nAktIdent;      // Obj fuer Neuerzeugung

    unsigned					bAutoTextEdit : 1; // Textedit nach dem erzeugen eines Textrahmens starten
    unsigned					b1stPointAsCenter : 1;
    unsigned					bUseIncompatiblePathCreateInterface : 1;
    unsigned					bAutoClosePolys : 1;
    unsigned					bCurrentLibObjMoveNoResize : 1;
    unsigned					bCurrentLibObjSetDefAttr : 1;
    unsigned					bCurrentLibObjSetDefLayer : 1;

private:
    void ImpClearVars();
    void ImpMakeCreateAttr();

protected:

    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);
    BOOL CheckEdgeMode();

public:
    SdrCreateView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrCreateView();

    virtual void ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const;
    virtual BOOL IsAction() const;
    virtual void BrkAction();


    void SetActiveLayer(const String& rName) { aAktLayer=rName; }
    const String& GetActiveLayer() const { return aAktLayer; }
    void SetMeasureLayer(const String& rName) { aMeasureLayer=rName; }
    const String& GetMeasureLayer() const { return aMeasureLayer; }

    // Feststellen, ob Textwerkzeug aktiviert
    BOOL IsTextTool() const;

    // Feststellen, ob Objektverbinderwerkzeug aktiviert
    BOOL IsEdgeTool() const;

    // Feststellen, ob Bemassungswerkzeug aktiviert

    void SetCurrentObj(UINT16 nIdent, UINT32 nInvent=SdrInventor);
    void TakeCurrentObj(UINT16& nIdent, UINT32& nInvent) const  { nInvent=nAktInvent; nIdent=nAktIdent; }
    UINT32 GetCurrentObjInventor() const { return nAktInvent; }
    UINT16 GetCurrentObjIdentifier() const { return nAktIdent; }

    // Hier kann man ein vorgefertigtes Objekt als Create-Default setzen.
    // Die Methoden arbeiten in Wechselwirkung mit Set/TakeCurrentObj().
    // TakeCurrentObj liefert anschliessend OBJ_NONE solange bis wieder mit
    // obiger Methode ein Objekt fuer normales Create gesetzt wird. Das zuvor
    // gesetzte vorgefertigte Objekt wird dann per delete abgeraeumt.
    // Das uebergebene Obj beim Aufruf dieser Methode der View uebereignet!
    // Bei jedem BegCreate wird eine Kopie angefertigt, welche dann eingefuegt
    // wird. Wenn bMoveNoResize=FALSE, wird das Objekt dann via Resize
    // "aufgezogen". Andernfalls wird es nur gemoved.
    // Als Mauszeiger wird POINTER_CROSS verwendet.
    const SdrObject* GetCurrentLibObj() const { return pCurrentLibObj; }

    // Starten des normalen Create
    void BrkCreateObj();
    BOOL IsCreateObj() const { return pAktCreate!=NULL; }
    SdrObject* GetCreateObj() const { return pAktCreate; }

    // BegCreateCaptionObj() erzeugt ein SdrCaptionObj (Legendenobjekt).
    // rObjSiz ist die anfaengliche Groesse des Legenden-Textrahmens.
    // gedraggd wird lediglich die Laenge des Zipfel.

    // Einfuegen eines vorgefertigten Objekts. pObj wird dabei uebereignet.
    // Die Einstellung des CurrentObj bleibt hierbei unbeeinflusst!
    // Diese Funktion eignet sich dafuer, um via eigenem "Drag&Drop" von
    // einer Symbolbibliothek ein Symbol per Maus zu greifen und dieses auf
    // das Dokument zu ziehen.

    // Wenn TextEditAfterCreate auf TRUE steht (das ist der Default),
    // dann wird nach dem erzeugen eines Textrahmenobjekts (OBJ_TEXT,
    // OBJ_TEXTEXT, OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatisch ein TextEdit (SdrObjEditView::BegTextEdit) gestartet.
    BOOL IsTextEditAfterCreate() const { return bAutoTextEdit; }
    void SetTextEditAfterCreate(BOOL bOn) { bAutoTextEdit = bOn; }

    // Erzeugen eines Kreises/Rechtecks/Textrahmens wobei der 1. Punkt
    // nicht die linke obere Ecke, sondern das Zentrum des Objekts vorgibt.
    // Persistentes Flag. Default=FALSE.
    BOOL IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(BOOL bOn) { b1stPointAsCenter = bOn; SnapMove(); }

    // Fuer Polylines (OBJ_PLIN) und Freihandlinien (OBJ_FREELINE). Ist dieses
    // Flag TRUE, werden diese beiden Objekttypen implizit geschlossen und in
    // Polygon (OBJ_POLY) bzw. Freihandflaeche (OBJ_FREEFILL) gewandelt falls
    // zum Ende des Create die Distanz zwischen Startpunkt und Endpunkt des
    // Objekts <=n Pixel ist, wobei SetAutoCloseDistPix vorgegeben wird.
    // Default=TRUE.
    BOOL IsAutoClosePolys() const { return bAutoClosePolys; }
    void SetAutoClosePolys(BOOL bOn) { bAutoClosePolys=bOn; }

    // Default=5 Pixel.
    USHORT GetAutoCloseDistPix() const { return USHORT(nAutoCloseDistPix); }
    void SetAutoCloseDistPix(USHORT nVal) { nAutoCloseDistPix=nVal; }

    // Vorgabe des minimalen Pixelabstands zwischen 2 Bezierpunkten bei der
    // Erzeugung einer Freihandlinie.
    // Default=10 Pixel.
    USHORT GetFreeHandMinDistPix() const { return USHORT(nFreeHandMinDistPix); }
    void SetFreeHandMinDistPix(USHORT nVal) { nFreeHandMinDistPix=nVal; }

    // Wer das (zur restlichen Create-Funktionalitaet von SvDraw) inkompatible
    // Create-Interface am PathObj beibehalten moechte muss das nachfolgende
    // Flag setzen. Dieses wirkt sich aus bei den Objekttypen:
    //     OBJ_POLY, OBJ_PLIN, OBJ_PATHLINE, OBJ_PATHFILL
    // Dieses Flag hat nur voruebergehenden Character. Die betroffenen
    // Applikationen sollten alsbald umgestellt werden.
    // Default=FALSE;
    BOOL IsUseIncompatiblePathCreateInterface() const { return bUseIncompatiblePathCreateInterface; }
    void SetUseIncompatiblePathCreateInterface(BOOL bOn) { bUseIncompatiblePathCreateInterface = bOn; }
};

}//end of namespace binfilter
#endif //_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
