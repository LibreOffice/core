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

#ifndef _B3D_B3DTRANS_HXX
#define _B3D_B3DTRANS_HXX

#define ZBUFFER_DEPTH_RANGE         ((double)(256L * 256L * 256L))

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/range/b3drange.hxx>
#include <tools/gen.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <tools/toolsdllapi.h>


// Supported methods for setting/keeping the aspect ratio

enum Base3DRatio
{
    Base3DRatioGrow = 1,
    Base3DRatioShrink,
    Base3DRatioMiddle
};

// Supported projection types

enum Base3DProjectionType
{
    Base3DProjectionTypeParallel = 1,
    Base3DProjectionTypePerspective
};

// Transformation sets for 3D output

class TOOLS_DLLPUBLIC B3dTransformationSet
{
private:
    // Object Matrix Object -> World
    basegfx::B3DHomMatrix           maObjectTrans;
    basegfx::B3DHomMatrix           maInvObjectTrans;

    // Orientation Matrix
    basegfx::B3DHomMatrix           maOrientation;
    basegfx::B3DHomMatrix           maInvOrientation;

    // Projection Matrix
    basegfx::B3DHomMatrix           maProjection;
    basegfx::B3DHomMatrix           maInvProjection;

    // Texture Matrices
    basegfx::B2DHomMatrix           maTexture;

    // Special transformation set for converting Object -> Device
    basegfx::B3DHomMatrix           maObjectToDevice;

    // Transposed and inversed matrix for vector transformations
    basegfx::B3DHomMatrix           maInvTransObjectToEye;

    // Transformation for World->View
    basegfx::B3DHomMatrix           maMatFromWorldToView;
    basegfx::B3DHomMatrix           maInvMatFromWorldToView;

    // Parameters for ViewportTransformation
    basegfx::B3DVector          maScale;
    basegfx::B3DVector          maTranslate;

    // ViewPlane DeviceRectangle (user-defined)
    double                          mfLeftBound;
    double                          mfRightBound;
    double                          mfBottomBound;
    double                          mfTopBound;

    // Near and far clipping planes
    double                          mfNearBound;
    double                          mfFarBound;

    // Aspect ratio of 3D transformation (Y / X)
    // default: 1:1 -> 1.0
    // Disable with value 0.0
    double                          mfRatio;

    // Viewport area in logical coordinates
    Rectangle                       maViewportRectangle;
    // Visible area within viewport
    Rectangle                       maVisibleRectangle;

    // Actual coordinates as set by CalcViewport
    // of visible viewport area (logical coordinates)
    Rectangle                       maSetBound;

    // Method of keeping defined aspect ratio
    // default: Base3DRatioGrow
    Base3DRatio                     meRatio;

    // Flags
    unsigned                        mbPerspective               : 1;
    unsigned                        mbWorldToViewValid          : 1;
    unsigned                        mbInvTransObjectToEyeValid  : 1;
    unsigned                        mbObjectToDeviceValid       : 1;
    unsigned                        mbProjectionValid           : 1;

public:
    B3dTransformationSet();
    virtual ~B3dTransformationSet();

    void Reset();

    // ObjectTrans
    const basegfx::B3DHomMatrix& GetObjectTrans() { return maObjectTrans; }
    const basegfx::B3DHomMatrix& GetInvObjectTrans() { return maInvObjectTrans; }

    // Orientation
    void SetOrientation(
        basegfx::B3DPoint aVRP = basegfx::B3DPoint(0.0,0.0,1.0),
        basegfx::B3DVector aVPN = basegfx::B3DVector(0.0,0.0,1.0),
        basegfx::B3DVector aVUP = basegfx::B3DVector(0.0,1.0,0.0));
    const basegfx::B3DHomMatrix& GetOrientation() { return maOrientation; }
    const basegfx::B3DHomMatrix& GetInvOrientation() { return maInvOrientation; }

    // Projection
    void SetProjection(const basegfx::B3DHomMatrix& mProject);
    const basegfx::B3DHomMatrix& GetProjection();

    // Texture
    const basegfx::B2DHomMatrix& GetTexture() { return maTexture; }

    // aspect ratio accessors and the defined method of keeping defined aspect ratio
    double GetRatio() { return mfRatio; }
    void SetRatio(double fNew=1.0);
    Base3DRatio GetRatioMode() { return meRatio; }

    // Parameters of ViewportTransformation
    void SetDeviceRectangle(double fL=-1.0, double fR=1.0, double fB=-1.0, double fT=1.0, sal_Bool bBroadCastChange=sal_True);
    double GetDeviceRectangleWidth() const { return mfRightBound - mfLeftBound; }
    double GetDeviceRectangleHeight() const { return mfTopBound - mfBottomBound; }
    double GetFrontClippingPlane() { return mfNearBound; }
    double GetBackClippingPlane() { return mfFarBound; }
    void SetPerspective(sal_Bool bNew);
    sal_Bool GetPerspective() { return mbPerspective; }
    void SetViewportRectangle(Rectangle& rRect, Rectangle& rVisible);
    void SetViewportRectangle(Rectangle& rRect) { SetViewportRectangle(rRect, rRect); }
    const Rectangle& GetViewportRectangle() { return maViewportRectangle; }
    void CalcViewport();

    // Direct accessors for miscellaneous transformations
    const basegfx::B3DPoint WorldToEyeCoor(const basegfx::B3DPoint& rVec);
    const basegfx::B3DPoint EyeToWorldCoor(const basegfx::B3DPoint& rVec);

    static void Frustum(
        basegfx::B3DHomMatrix& rTarget,
        double fLeft = -1.0, double fRight = 1.0,
        double fBottom = -1.0, double fTop = 1.0,
        double fNear = 0.001, double fFar = 1.0);
    static void Ortho(
        basegfx::B3DHomMatrix& rTarget,
        double fLeft = -1.0, double fRight = 1.0,
        double fBottom = -1.0, double fTop = 1.0,
        double fNear = 0.0, double fFar = 1.0);
    static void Orientation(
        basegfx::B3DHomMatrix& rTarget,
        basegfx::B3DPoint aVRP = basegfx::B3DPoint(0.0,0.0,1.0),
        basegfx::B3DVector aVPN = basegfx::B3DVector(0.0,0.0,1.0),
        basegfx::B3DVector aVUP = basegfx::B3DVector(0.0,1.0,0.0));

protected:
    void PostSetObjectTrans();
    void PostSetOrientation();
    void PostSetProjection();
    void PostSetViewport();

    virtual void DeviceRectangleChange();
};

/*************************************************************************
|* Viewport for B3D
|*
|* Uses a simplified model, in which a point
|* is described using a View Reference Point (VRP)
\************************************************************************/

class TOOLS_DLLPUBLIC B3dViewport : public B3dTransformationSet
{
private:
    basegfx::B3DPoint               aVRP;           // View Reference Point
    basegfx::B3DVector          aVPN;           // View Plane Normal
    basegfx::B3DVector          aVUV;           // View Up Vector

public:
    B3dViewport();
    virtual ~B3dViewport();

    void SetVUV(const basegfx::B3DVector& rNewVUV);
    void SetViewportValues(
        const basegfx::B3DPoint& rNewVRP,
        const basegfx::B3DVector& rNewVPN,
        const basegfx::B3DVector& rNewVUV);

    const basegfx::B3DPoint&    GetVRP() const  { return aVRP; }
    const basegfx::B3DVector&   GetVPN() const  { return aVPN; }
    const basegfx::B3DVector&   GetVUV() const  { return aVUV; }

protected:
    void CalcOrientation();
};

// B3D camera

class TOOLS_DLLPUBLIC B3dCamera : public B3dViewport
{
private:
    basegfx::B3DPoint       aPosition;
    basegfx::B3DPoint       aCorrectedPosition;
    basegfx::B3DVector  aLookAt;
    double                  fFocalLength;
    double                  fBankAngle;

    unsigned                bUseFocalLength         : 1;

public:
    B3dCamera(
        const basegfx::B3DPoint& rPos = basegfx::B3DPoint(0.0, 0.0, 1.0),
        const basegfx::B3DVector& rLkAt = basegfx::B3DVector(0.0, 0.0, 0.0),
        double fFocLen = 35.0, double fBnkAng = 0.0,
        sal_Bool bUseFocLen = sal_False);
    virtual ~B3dCamera();

    const basegfx::B3DPoint& GetPosition() const { return aPosition; }
    const basegfx::B3DVector& GetLookAt() const { return aLookAt; }

    // Focal length in mm
    double GetFocalLength() const { return fFocalLength; }

    double GetBankAngle() const { return fBankAngle; }

    sal_Bool GetUseFocalLength() const { return (sal_Bool)bUseFocalLength; }

protected:
    void CalcNewViewportValues();
    sal_Bool CalcFocalLength();

    virtual void DeviceRectangleChange();
};


#endif          // _B3D_B3DTRANS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
