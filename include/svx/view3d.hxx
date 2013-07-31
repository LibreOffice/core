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

/*
 * Forward declarations
 */

class E3dObject;
class E3dScene;
class Impl3DMirrorConstructOverlay;

/**
 * Derived class of SdrView to edit 3D objects.
 */

class SVX_DLLPUBLIC E3dView : public SdrView
{
protected:
    E3dDefaultAttributes        a3DDefaultAttr;
    MouseEvent                  aMouseEvent;                    // The parameters of the last Events (Mouse, Keyboard)
    Color                       aDefaultLightColor;             // The paramaters for the last colors
    Color                       aDefaultAmbientColor;

    double                      fDefaultScaleX;                 // Scaling
    double                      fDefaultScaleY;
    double                      fDefaultScaleZ;
    double                      fDefaultRotateX;                // and Rotation
    double                      fDefaultRotateY;
    double                      fDefaultRotateZ;
    double                      fDefaultExtrusionDeepth;        // Extrusion depth
    double                      fDefaultLightIntensity;         // Intensity of the two (necessary) light sources.
    double                      fDefaultAmbientIntensity;
    long                        nHDefaultSegments;              // Amount of HSegments required by the Lathe object
    long                        nVDefaultSegments;              // Amount of VSegments required by the Lathe object

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

    // Output all marked Objects on the given OutputDevice.
    virtual void DrawMarkedObj(OutputDevice& rOut) const;

    // Access to the default attributes.
    E3dDefaultAttributes& Get3DDefaultAttributes() { return a3DDefaultAttr; }
    virtual sal_Bool BegDragObj(const Point& rPnt, OutputDevice* pOut = NULL, SdrHdl* pHdl = NULL, short nMinMov = -3, SdrDragMethod* pForcedMeth = NULL);
    virtual void CheckPossibilities();

    // Get/Set Event
    void SetMouseEvent(const MouseEvent& rNew) { aMouseEvent = rNew; }
    const MouseEvent& GetMouseEvent() { return aMouseEvent; }

    // Override getting the model, as we need to supply a Scene together with individual 3D Objects.
    virtual SdrModel* GetMarkedObjModel() const;

    // On Paste: We need to insert the objects of the Scene, but not the Scene itself
    using SdrView::Paste;
    virtual sal_Bool Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);

    // #83403# Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
    bool ImpCloneAll3DObjectsToDestScene(E3dScene* pSrcScene, E3dScene* pDstScene, Point aOffset);

    bool IsConvertTo3DObjPossible() const;
    void ConvertMarkedObjTo3D(bool bExtrude=true, basegfx::B2DPoint aPnt1 = basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint aPnt2 = basegfx::B2DPoint(0.0, 1.0));

    // Means to create all Extrudes in a certain depth order.
    void DoDepthArrange(E3dScene* pScene, double fDepth);
    void ConvertMarkedToPolyObj(sal_Bool bLineToArea);
    E3dScene* SetCurrent3DObj(E3dObject* p3DObj);
    void Start3DCreation();

    // Migration of overlay
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

#endif // _E3D_VIEW3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
