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

#include <svx/svddrgv.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XLineAttrSetItem;
class XFillAttrSetItem;
class SdrEdgeObj;
class SdrObjConnection;

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
class ImplConnectMarkerOverlay;
class ImpSdrCreateViewExtraData;

class SVX_DLLPUBLIC SdrCreateView: public SdrDragView
{
    friend class                SdrPageView;

protected:
    SdrObject*                  pAktCreate;   // Aktuell in Erzeugung befindliches Objekt
    SdrPageView*                pCreatePV;    // Hier wurde die Erzeugung gestartet
    ImplConnectMarkerOverlay*   mpCoMaOverlay;

    // for migrating stuff from XOR, use ImpSdrCreateViewExtraData ATM to not need to
    // compile the apps all the time
    ImpSdrCreateViewExtraData*  mpCreateViewExtraData;

    Pointer                     aAktCreatePointer;

    INT32                       nAutoCloseDistPix;
    INT32                       nFreeHandMinDistPix;
    UINT32                      nAktInvent;     // Aktuell eingestelltes
    UINT16                      nAktIdent;      // Obj fuer Neuerzeugung

    unsigned                    bAutoTextEdit : 1; // Textedit nach dem erzeugen eines Textrahmens starten
    unsigned                    b1stPointAsCenter : 1;
    unsigned                    bUseIncompatiblePathCreateInterface : 1;
    unsigned                    bAutoClosePolys : 1;

    void ImpClearConnectMarker();

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpMakeCreateAttr();
    SVX_DLLPRIVATE void ImpDelCreateAttr();

protected:
    sal_Bool ImpBegCreateObj(sal_uInt32 nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
        sal_Int16 nMinMov, SdrPageView* pPV, const Rectangle& rLogRect, SdrObject* pPreparedFactoryObject);

    void ShowCreateObj(/*OutputDevice* pOut, BOOL bFull*/);
    void HideCreateObj(/*OutputDevice* pOut, BOOL bFull*/);
    BOOL CheckEdgeMode();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrCreateView();

public:
    virtual BOOL IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    virtual BOOL MouseMove(const MouseEvent& rMEvt, Window* pWin);

    void SetActiveLayer(const String& rName) { aAktLayer=rName; }
    const String& GetActiveLayer() const { return aAktLayer; }
    void SetMeasureLayer(const String& rName) { aMeasureLayer=rName; }
    const String& GetMeasureLayer() const { return aMeasureLayer; }

    // Ist der MeasureLayer nicht gesetzt (Leerstring), so
    // wird der ActiveLayer auch fuer Bemassung verwendet.
    void SetEditMode(SdrViewEditMode eMode) { SdrDragView::SetEditMode(eMode); CheckEdgeMode(); }
    void SetEditMode(BOOL bOn=TRUE) { SdrDragView::SetEditMode(bOn); CheckEdgeMode(); }
    void SetCreateMode(BOOL bOn=TRUE) { SdrDragView::SetCreateMode(bOn); CheckEdgeMode(); }
    void SetGluePointEditMode(BOOL bOn=TRUE) { SdrDragView::SetGluePointEditMode(bOn); CheckEdgeMode(); }

    // Feststellen, ob Textwerkzeug aktiviert
    BOOL IsTextTool() const;

    // Feststellen, ob Objektverbinderwerkzeug aktiviert
    BOOL IsEdgeTool() const;

    // Feststellen, ob Bemassungswerkzeug aktiviert
    BOOL IsMeasureTool() const;

    void SetCurrentObj(UINT16 nIdent, UINT32 nInvent=SdrInventor);
    void TakeCurrentObj(UINT16& nIdent, UINT32& nInvent) const  { nInvent=nAktInvent; nIdent=nAktIdent; }
    UINT32 GetCurrentObjInventor() const { return nAktInvent; }
    UINT16 GetCurrentObjIdentifier() const { return nAktIdent; }

    // Starten des normalen Create
    BOOL BegCreateObj(const Point& rPnt, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);
    sal_Bool BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const Point& rPnt);
    BOOL EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // z.B. wieder 1 Polygonpunkt zurueck.
    void BrkCreateObj();
    BOOL IsCreateObj() const { return pAktCreate!=NULL; }
    SdrObject* GetCreateObj() const { return pAktCreate; }

    // BegCreateCaptionObj() erzeugt ein SdrCaptionObj (Legendenobjekt).
    // rObjSiz ist die anfaengliche Groesse des Legenden-Textrahmens.
    // gedraggd wird lediglich die Laenge des Zipfel.
    BOOL BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);

    // Wenn TextEditAfterCreate auf TRUE steht (das ist der Default),
    // dann wird nach dem erzeugen eines Textrahmenobjekts (OBJ_TEXT,
    // OBJ_TEXTEXT, OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatisch ein TextEdit (SdrObjEditView::SdrBeginTextEdit) gestartet.
    BOOL IsTextEditAfterCreate() const { return bAutoTextEdit; }
    void SetTextEditAfterCreate(BOOL bOn) { bAutoTextEdit = bOn; }

    // Erzeugen eines Kreises/Rechtecks/Textrahmens wobei der 1. Punkt
    // nicht die linke obere Ecke, sondern das Zentrum des Objekts vorgibt.
    // Persistentes Flag. Default=FALSE.
    BOOL IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(BOOL bOn) { b1stPointAsCenter = bOn; }

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
    void SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& rPV);
    void HideConnectMarker();

    // Attribute des ggf. gerade in der Erzeugung befindlichen Objekts
    /* new interface src537 */
    BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;

    BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(BOOL& rOk) const;
    BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
};

#endif //_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
