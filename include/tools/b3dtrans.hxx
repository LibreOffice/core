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

#ifndef INCLUDED_TOOLS_B3DTRANS_HXX
#define INCLUDED_TOOLS_B3DTRANS_HXX

#define ZBUFFER_DEPTH_RANGE         (double(256L * 256L * 256L))

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/range/b3drange.hxx>
#include <tools/gen.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <tools/toolsdllapi.h>

/// Transformation sets for 3D output
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC B3dTransformationSet
{
private:
    // Object Matrix Object -> World
    basegfx::B3DHomMatrix maObjectTrans;
    basegfx::B3DHomMatrix maInvObjectTrans;

    // Orientation Matrix
    basegfx::B3DHomMatrix maOrientation;
    basegfx::B3DHomMatrix maInvOrientation;

    // Projection Matrix
    basegfx::B3DHomMatrix maProjection;
    basegfx::B3DHomMatrix maInvProjection;

    // Texture Matrices
    basegfx::B2DHomMatrix maTexture;

    // Parameters for ViewportTransformation
    basegfx::B3DVector    maScale;
    basegfx::B3DVector    maTranslate;

    // ViewPlane DeviceRectangle (user-defined)
    double                mfLeftBound;
    double                mfRightBound;
    double                mfBottomBound;
    double                mfTopBound;

    // Near and far clipping planes
    double                mfNearBound;
    double                mfFarBound;

    // Aspect ratio of 3D transformation (Y / X)
    // default: 1:1 -> 1.0
    // Disable with value 0.0
    double                mfRatio;

    // Viewport area in logical coordinates
    tools::Rectangle             maViewportRectangle;
    // Visible area within viewport
    tools::Rectangle             maVisibleRectangle;

    // Actual coordinates as set by CalcViewport
    // of visible viewport area (logical coordinates)
    tools::Rectangle             maSetBound;

    // Flags
    bool mbPerspective              : 1;
    bool mbProjectionValid          : 1;

public:
    B3dTransformationSet();
    virtual ~B3dTransformationSet();

    B3dTransformationSet(B3dTransformationSet const &) = default;
    B3dTransformationSet(B3dTransformationSet &&) = default;
    B3dTransformationSet & operator =(B3dTransformationSet const &) = default;
    B3dTransformationSet & operator =(B3dTransformationSet &&) = default;

    void Reset();

    /** Set the orientation

        @param vVRP the View Reference Point (VRP)
        @param vVPN the View Plane Normal (VPN)
        @param vVUP the View Up Plane (VUP)
    */
    void SetOrientation(
        const basegfx::B3DPoint& rVRP = basegfx::B3DPoint(0.0,0.0,1.0),
        const basegfx::B3DVector& rVPN = basegfx::B3DVector(0.0,0.0,1.0),
        const basegfx::B3DVector& rVUP = basegfx::B3DVector(0.0,1.0,0.0));

    // Projection
    void SetProjection(const basegfx::B3DHomMatrix& mProject);
    const basegfx::B3DHomMatrix& GetProjection();

    // Texture

    // aspect ratio accessors and the defined method of keeping defined aspect ratio
    double GetRatio() const { return mfRatio; }
    void SetRatio(double fNew);

    // Parameters of ViewportTransformation
    void SetDeviceRectangle(double fL=-1.0, double fR=1.0,
                            double fB=-1.0, double fT=1.0);
    double GetDeviceRectangleWidth() const { return mfRightBound - mfLeftBound; }

    void SetPerspective(bool bNew);

    void SetViewportRectangle(tools::Rectangle const & rRect, tools::Rectangle const & rVisible);
    void SetViewportRectangle(tools::Rectangle const & rRect) { SetViewportRectangle(rRect, rRect); }

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
        const basegfx::B3DPoint& aVRP = basegfx::B3DPoint(0.0,0.0,1.0),
        basegfx::B3DVector aVPN = basegfx::B3DVector(0.0,0.0,1.0),
        basegfx::B3DVector aVUP = basegfx::B3DVector(0.0,1.0,0.0));

protected:
    void PostSetObjectTrans();
    void PostSetOrientation();
    void PostSetProjection();

    virtual void DeviceRectangleChange();
};

/** Viewport for B3D

    Uses a simplified model, in which a point is described using a View
    Reference Point (VRP).
*/
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC B3dViewport : public B3dTransformationSet
{
private:
    basegfx::B3DPoint           aVRP;   // View Reference Point
    basegfx::B3DVector          aVPN;   // View Plane Normal
    basegfx::B3DVector          aVUV;   // View Up Vector

public:
    B3dViewport();
    virtual ~B3dViewport() override;

    B3dViewport(B3dViewport const &) = default;
    B3dViewport(B3dViewport &&) = default;
    B3dViewport & operator =(B3dViewport const &) = default;
    B3dViewport & operator =(B3dViewport &&) = default;

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

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC B3dCamera final : public B3dViewport
{
public:
    B3dCamera(
        const basegfx::B3DPoint& rPos = basegfx::B3DPoint(0.0, 0.0, 1.0),
        const basegfx::B3DVector& rLkAt = basegfx::B3DVector(0.0, 0.0, 0.0),
        double fFocLen = 35.0, double fBnkAng = 0.0);
    virtual ~B3dCamera() override;

    B3dCamera(B3dCamera const &) = default;
    B3dCamera(B3dCamera &&) = default;
    B3dCamera & operator =(B3dCamera const &) = default;
    B3dCamera & operator =(B3dCamera &&) = default;

private:
    void CalcNewViewportValues();
    bool CalcFocalLength();

    virtual void DeviceRectangleChange() override;

    basegfx::B3DPoint       aPosition;
    basegfx::B3DPoint       aCorrectedPosition;
    basegfx::B3DVector  aLookAt;
    double                  fFocalLength;
    double                  fBankAngle;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
