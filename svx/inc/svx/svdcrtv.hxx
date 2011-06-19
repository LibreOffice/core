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

    sal_Int32                       nAutoCloseDistPix;
    sal_Int32                       nFreeHandMinDistPix;
    sal_uInt32                      nAktInvent;     // Aktuell eingestelltes
    sal_uInt16                      nAktIdent;      // Obj fuer Neuerzeugung

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

    void ShowCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/);
    void HideCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/);
    sal_Bool CheckEdgeMode();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrCreateView();

public:
    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);

    void SetActiveLayer(const String& rName) { aAktLayer=rName; }
    const String& GetActiveLayer() const { return aAktLayer; }
    void SetMeasureLayer(const String& rName) { aMeasureLayer=rName; }
    const String& GetMeasureLayer() const { return aMeasureLayer; }

    // Ist der MeasureLayer nicht gesetzt (Leerstring), so
    // wird der ActiveLayer auch fuer Bemassung verwendet.
    void SetEditMode(SdrViewEditMode eMode) { SdrDragView::SetEditMode(eMode); CheckEdgeMode(); }
    void SetEditMode(sal_Bool bOn=sal_True) { SdrDragView::SetEditMode(bOn); CheckEdgeMode(); }
    void SetCreateMode(sal_Bool bOn=sal_True) { SdrDragView::SetCreateMode(bOn); CheckEdgeMode(); }
    void SetGluePointEditMode(sal_Bool bOn=sal_True) { SdrDragView::SetGluePointEditMode(bOn); CheckEdgeMode(); }

    // Feststellen, ob Textwerkzeug aktiviert
    sal_Bool IsTextTool() const;

    // Feststellen, ob Objektverbinderwerkzeug aktiviert
    sal_Bool IsEdgeTool() const;

    // Feststellen, ob Bemassungswerkzeug aktiviert
    sal_Bool IsMeasureTool() const;

    void SetCurrentObj(sal_uInt16 nIdent, sal_uInt32 nInvent=SdrInventor);
    void TakeCurrentObj(sal_uInt16& nIdent, sal_uInt32& nInvent) const  { nInvent=nAktInvent; nIdent=nAktIdent; }
    sal_uInt32 GetCurrentObjInventor() const { return nAktInvent; }
    sal_uInt16 GetCurrentObjIdentifier() const { return nAktIdent; }

    // Starten des normalen Create
    sal_Bool BegCreateObj(const Point& rPnt, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);
    sal_Bool BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const Point& rPnt);
    sal_Bool EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // z.B. wieder 1 Polygonpunkt zurueck.
    void BrkCreateObj();
    sal_Bool IsCreateObj() const { return pAktCreate!=NULL; }
    SdrObject* GetCreateObj() const { return pAktCreate; }

    // BegCreateCaptionObj() erzeugt ein SdrCaptionObj (Legendenobjekt).
    // rObjSiz ist die anfaengliche Groesse des Legenden-Textrahmens.
    // gedraggd wird lediglich die Laenge des Zipfel.
    sal_Bool BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);

    // Wenn TextEditAfterCreate auf sal_True steht (das ist der Default),
    // dann wird nach dem erzeugen eines Textrahmenobjekts (OBJ_TEXT,
    // OBJ_TEXTEXT, OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatisch ein TextEdit (SdrObjEditView::SdrBeginTextEdit) gestartet.
    sal_Bool IsTextEditAfterCreate() const { return bAutoTextEdit; }
    void SetTextEditAfterCreate(sal_Bool bOn) { bAutoTextEdit = bOn; }

    // Erzeugen eines Kreises/Rechtecks/Textrahmens wobei der 1. Punkt
    // nicht die linke obere Ecke, sondern das Zentrum des Objekts vorgibt.
    // Persistentes Flag. Default=FALSE.
    sal_Bool IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(sal_Bool bOn) { b1stPointAsCenter = bOn; }

    // Fuer Polylines (OBJ_PLIN) und Freihandlinien (OBJ_FREELINE). Ist dieses
    // Flag sal_True, werden diese beiden Objekttypen implizit geschlossen und in
    // Polygon (OBJ_POLY) bzw. Freihandflaeche (OBJ_FREEFILL) gewandelt falls
    // zum Ende des Create die Distanz zwischen Startpunkt und Endpunkt des
    // Objekts <=n Pixel ist, wobei SetAutoCloseDistPix vorgegeben wird.
    // Default=TRUE.
    sal_Bool IsAutoClosePolys() const { return bAutoClosePolys; }
    void SetAutoClosePolys(sal_Bool bOn) { bAutoClosePolys=bOn; }

    // Default=5 Pixel.
    sal_uInt16 GetAutoCloseDistPix() const { return sal_uInt16(nAutoCloseDistPix); }
    void SetAutoCloseDistPix(sal_uInt16 nVal) { nAutoCloseDistPix=nVal; }

    // Vorgabe des minimalen Pixelabstands zwischen 2 Bezierpunkten bei der
    // Erzeugung einer Freihandlinie.
    // Default=10 Pixel.
    sal_uInt16 GetFreeHandMinDistPix() const { return sal_uInt16(nFreeHandMinDistPix); }
    void SetFreeHandMinDistPix(sal_uInt16 nVal) { nFreeHandMinDistPix=nVal; }

    // Wer das (zur restlichen Create-Funktionalitaet von SvDraw) inkompatible
    // Create-Interface am PathObj beibehalten moechte muss das nachfolgende
    // Flag setzen. Dieses wirkt sich aus bei den Objekttypen:
    //     OBJ_POLY, OBJ_PLIN, OBJ_PATHLINE, OBJ_PATHFILL
    // Dieses Flag hat nur voruebergehenden Character. Die betroffenen
    // Applikationen sollten alsbald umgestellt werden.
    // Default=sal_False;
    sal_Bool IsUseIncompatiblePathCreateInterface() const { return bUseIncompatiblePathCreateInterface; }
    void SetUseIncompatiblePathCreateInterface(sal_Bool bOn) { bUseIncompatiblePathCreateInterface = bOn; }
    void SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& rPV);
    void HideConnectMarker();

    // Attribute des ggf. gerade in der Erzeugung befindlichen Objekts
    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(sal_Bool& rOk) const;
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);
};

#endif //_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
