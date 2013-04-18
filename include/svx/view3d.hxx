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

#ifndef _E3D_VIEW3D_HXX
#define _E3D_VIEW3D_HXX

#include <svx/svdview.hxx>
#include <svx/def3d.hxx>
#include <svx/deflt3d.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class E3dObject;
class E3dScene;
class Impl3DMirrorConstructOverlay;

/*************************************************************************
|*
|* Ableitung von SdrView zur Bearbeitung von 3D-Objekten
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dView : public SdrView
{
protected:
    E3dDefaultAttributes        a3DDefaultAttr;
    MouseEvent                  aMouseEvent;                    // Die Parameter der letzten Events (Mouse, Keyboard)
    Color                       aDefaultLightColor;             // dito mit den Farben
    Color                       aDefaultAmbientColor;

    double                      fDefaultScaleX;                 // Verzerrungen
    double                      fDefaultScaleY;
    double                      fDefaultScaleZ;
    double                      fDefaultRotateX;                // und Drehungen
    double                      fDefaultRotateY;
    double                      fDefaultRotateZ;
    double                      fDefaultExtrusionDeepth;        // Extrusionstiefe
    double                      fDefaultLightIntensity;         // Intensitaeten der beiden (notwendigen) Licht-
    double                      fDefaultAmbientIntensity;       // quellen
    long                        nHDefaultSegments;              // wieviele HSegmente braucht mein Lathe-Ojekt
    long                        nVDefaultSegments;              // wieviele VSegmente braucht mein Lathe-Ojekt

    E3dDragConstraint           eDragConstraint;

    // Migrate selections
    Impl3DMirrorConstructOverlay*                   mpMirrorOverlay;

    sal_Bool                        bDoubleSided;

    void InitView();

    void ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat);
    void ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat);
    void ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj);
    void ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj);

    void InitScene(E3dScene* pScene, double fW, double fH, double fCamZ);
    void ImpIsConvertTo3DPossible(SdrObject* pObj, bool& rAny3D, bool& rGroupSelected) const;
    void BreakSingle3DObj(E3dObject* pObj);

public:
    TYPEINFO();
    E3dView(SdrModel* pModel, OutputDevice* pOut = 0L);
    virtual ~E3dView();

    // Alle markierten Objekte auf dem angegebenen OutputDevice ausgeben.
    virtual void DrawMarkedObj(OutputDevice& rOut) const;

    // Zugriff auf die Default-Attribute
    E3dDefaultAttributes& Get3DDefaultAttributes() { return a3DDefaultAttr; }
    virtual sal_Bool BegDragObj(const Point& rPnt, OutputDevice* pOut = NULL, SdrHdl* pHdl = NULL, short nMinMov = -3, SdrDragMethod* pForcedMeth = NULL);
    virtual void CheckPossibilities();

    // Event setzen/rausruecken
    void SetMouseEvent(const MouseEvent& rNew) { aMouseEvent = rNew; }
    const MouseEvent& GetMouseEvent() { return aMouseEvent; }

    // Model holen ueberladen, da bei einzelnen 3D Objekten noch eine Szene
    // untergeschoben werden muss
    virtual SdrModel* GetMarkedObjModel() const;

    // Bei Paste muss - falls in eine Scene eingefuegt wird - die
    // Objekte der Szene eingefuegt werden, die Szene selbst aber nicht
    using SdrView::Paste;
    virtual sal_Bool Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);

    // #83403# Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
    bool ImpCloneAll3DObjectsToDestScene(E3dScene* pSrcScene, E3dScene* pDstScene, Point aOffset);

    bool IsConvertTo3DObjPossible() const;
    void ConvertMarkedObjTo3D(bool bExtrude=true, basegfx::B2DPoint aPnt1 = basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint aPnt2 = basegfx::B2DPoint(0.0, 1.0));

    // Nachtraeglichhe Korrekturmoeglichkeit um alle Extrudes in einer
    // bestimmten Tiefensortierung anzulegen
    void DoDepthArrange(E3dScene* pScene, double fDepth);
    void ConvertMarkedToPolyObj(sal_Bool bLineToArea);
    E3dScene* SetCurrent3DObj(E3dObject* p3DObj);
    void Start3DCreation();

    // migration of overlay
    bool Is3DRotationCreationActive() const { return (0L != mpMirrorOverlay); }

    virtual void MovAction(const Point& rPnt);
    void End3DCreation(bool bUseDefaultValuesForMirrorAxes=false);
    void ResetCreationActive();

    double GetDefaultCamPosZ();

    double &DefaultScaleX ()
    {
        return fDefaultScaleX;
    }

    double DefaultScaleX () const
    {
        return fDefaultScaleX;
    }

    double &DefaultScaleY ()
    {
        return fDefaultScaleY;
    }

    double DefaultScaleY () const
    {
        return fDefaultScaleY;
    }

    double &DefaultScaleZ ()
    {
        return fDefaultScaleZ;
    }

    double DefaultScaleZ () const
    {
        return fDefaultScaleZ;
    }

    double &DefaultRotateX ()
    {
        return fDefaultRotateX;
    }

    double DefaultRotateX () const
    {
        return fDefaultRotateX;
    }

    double &DefaultRotateY ()
    {
        return fDefaultRotateY;
    }

    double DefaultRotateY () const
    {
        return fDefaultRotateY;
    }

    double &DefaultRotateZ ()
    {
        return fDefaultRotateZ;
    }

    double DefaultRotateZ () const
    {
        return fDefaultRotateZ;
    }

    double &DefaultExtrusionDeepth ()
    {
        return fDefaultExtrusionDeepth;
    }

    double DefaultExtrusionDeepth () const
    {
        return fDefaultExtrusionDeepth;
    }

    double GetDefaultCamFocal();

    double &DefaultLightIntensity ()
    {
        return fDefaultLightIntensity;
    }

    double DefaultLightIntensity () const
    {
        return fDefaultLightIntensity;
    }

    double &DefaultAmbientIntensity ()
    {
        return fDefaultAmbientIntensity;
    }

    double DefaultAmbientIntensity () const
    {
        return fDefaultAmbientIntensity;
    }

    const Color &DefaultLightColor () const
    {
        return aDefaultLightColor;
    }

    Color DefaultLightColor ()
    {
        return aDefaultLightColor;
    }

    const Color &DefaultAmbientColor () const
    {
        return aDefaultAmbientColor;
    }

    Color DefaultAmbientColor ()
    {
        return aDefaultAmbientColor;
    }

    long GetHDefaultSegments() const { return nHDefaultSegments; }
    void SetHDefaultSegments(long nSegs) { nHDefaultSegments = nSegs; }

    long GetVDefaultSegments() const { return nVDefaultSegments; }
    void SetVDefaultSegments(long nSegs) { nVDefaultSegments = nSegs; }

    bool IsBreak3DObjPossible() const;
    void Break3DObj();

    sal_Bool DoubleSided () const
    {
        return bDoubleSided;
    }

    sal_Bool &DoubleSided ()
    {
        return bDoubleSided;
    }

    SfxItemSet Get3DAttributes(E3dScene* pInScene = NULL, sal_Bool bOnly3DAttr=sal_False) const;
    void Set3DAttributes(const SfxItemSet& rAttr, E3dScene* pInScene = NULL, sal_Bool bOnly3DAttr=sal_False);
};

#endif          // _E3D_VIEW3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
