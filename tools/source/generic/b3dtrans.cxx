/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/b3dtrans.hxx>




B3dTransformationSet::B3dTransformationSet()
{
    Reset();
}

B3dTransformationSet::~B3dTransformationSet()
{
}

void B3dTransformationSet::Orientation(basegfx::B3DHomMatrix& rTarget, basegfx::B3DPoint aVRP, basegfx::B3DVector aVPN, basegfx::B3DVector aVUP)
{
    rTarget.translate( -aVRP.getX(), -aVRP.getY(), -aVRP.getZ());
    aVUP.normalize();
    aVPN.normalize();
    basegfx::B3DVector aRx(aVUP);
    basegfx::B3DVector aRy(aVPN);
    aRx = aRx.getPerpendicular(aRy);
    aRx.normalize();
    aRy = aRy.getPerpendicular(aRx);
    aRy.normalize();
    basegfx::B3DHomMatrix aTemp;
    aTemp.set(0, 0, aRx.getX());
    aTemp.set(0, 1, aRx.getY());
    aTemp.set(0, 2, aRx.getZ());
    aTemp.set(1, 0, aRy.getX());
    aTemp.set(1, 1, aRy.getY());
    aTemp.set(1, 2, aRy.getZ());
    aTemp.set(2, 0, aVPN.getX());
    aTemp.set(2, 1, aVPN.getY());
    aTemp.set(2, 2, aVPN.getZ());
    rTarget *= aTemp;
}

void B3dTransformationSet::Frustum(basegfx::B3DHomMatrix& rTarget, double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
{
    if(!(fNear > 0.0))
    {
        fNear = 0.001;
    }
    if(!(fFar > 0.0))
    {
        fFar = 1.0;
    }
    if(fNear == fFar)
    {
        fFar = fNear + 1.0;
    }
    if(fLeft == fRight)
    {
        fLeft -= 1.0;
        fRight += 1.0;
    }
    if(fTop == fBottom)
    {
        fBottom -= 1.0;
        fTop += 1.0;
    }
    basegfx::B3DHomMatrix aTemp;

    aTemp.set(0, 0, 2.0 * fNear / (fRight - fLeft));
    aTemp.set(1, 1, 2.0 * fNear / (fTop - fBottom));
    aTemp.set(0, 2, (fRight + fLeft) / (fRight - fLeft));
    aTemp.set(1, 2, (fTop + fBottom) / (fTop - fBottom));
    aTemp.set(2, 2, -1.0 * ((fFar + fNear) / (fFar - fNear)));
    aTemp.set(3, 2, -1.0);
    aTemp.set(2, 3, -1.0 * ((2.0 * fFar * fNear) / (fFar - fNear)));
    aTemp.set(3, 3, 0.0);

    rTarget *= aTemp;
}

void B3dTransformationSet::Ortho(basegfx::B3DHomMatrix& rTarget,
                                 double fLeft, double fRight, double fBottom, double fTop,
                                 double fNear, double fFar)
{
    if(fNear == fFar)
    {
        OSL_FAIL("Near and far clipping plane in Ortho definition are identical");
        fFar = fNear + 1.0;
    }
    if(fLeft == fRight)
    {
        OSL_FAIL("Left and right in Ortho definition are identical");
        fLeft -= 1.0;
        fRight += 1.0;
    }
    if(fTop == fBottom)
    {
        OSL_FAIL("Top and bottom in Ortho definition are identical");
        fBottom -= 1.0;
        fTop += 1.0;
    }
    basegfx::B3DHomMatrix aTemp;

    aTemp.set(0, 0, 2.0 / (fRight - fLeft));
    aTemp.set(1, 1, 2.0 / (fTop - fBottom));
    aTemp.set(2, 2, -1.0 * (2.0 / (fFar - fNear)));
    aTemp.set(0, 3, -1.0 * ((fRight + fLeft) / (fRight - fLeft)));
    aTemp.set(1, 3, -1.0 * ((fTop + fBottom) / (fTop - fBottom)));
    aTemp.set(2, 3, -1.0 * ((fFar + fNear) / (fFar - fNear)));

    rTarget *= aTemp;
}


void B3dTransformationSet::Reset()
{
    
    maObjectTrans.identity();
    PostSetObjectTrans();

    Orientation(maOrientation);
    PostSetOrientation();

    maTexture.identity();

    mfLeftBound = mfBottomBound = -1.0;
    mfRightBound = mfTopBound = 1.0;
    mfNearBound = 0.001;
    mfFarBound = 1.001;

    meRatio = Base3DRatioGrow;
    mfRatio = 0.0;

    maViewportRectangle = Rectangle(-1, -1, 2, 2);
    maVisibleRectangle = maViewportRectangle;

    mbPerspective = true;

    mbProjectionValid = false;
    mbObjectToDeviceValid = false;
    mbWorldToViewValid = false;

    CalcViewport();
}


void B3dTransformationSet::PostSetObjectTrans()
{
    
    maInvObjectTrans = maObjectTrans;
    maInvObjectTrans.invert();
}

void B3dTransformationSet::SetOrientation( basegfx::B3DPoint aVRP, basegfx::B3DVector aVPN, basegfx::B3DVector aVUP)
{
    maOrientation.identity();
    Orientation(maOrientation, aVRP, aVPN, aVUP);

    mbInvTransObjectToEyeValid = false;
    mbObjectToDeviceValid = false;
    mbWorldToViewValid = false;

    PostSetOrientation();
}

void B3dTransformationSet::PostSetOrientation()
{
    
    maInvOrientation = maOrientation;
    maInvOrientation.invert();
}


void B3dTransformationSet::SetProjection(const basegfx::B3DHomMatrix& mProject)
{
    maProjection = mProject;
    PostSetProjection();
}

const basegfx::B3DHomMatrix& B3dTransformationSet::GetProjection()
{
    if(!mbProjectionValid)
        CalcViewport();
    return maProjection;
}

void B3dTransformationSet::PostSetProjection()
{
    
    maInvProjection = GetProjection();
    maInvProjection.invert();

    
    mbObjectToDeviceValid = false;
    mbWorldToViewValid = false;
}


void B3dTransformationSet::CalcViewport()
{
    
    double fLeft(mfLeftBound);
    double fRight(mfRightBound);
    double fBottom(mfBottomBound);
    double fTop(mfTopBound);

    
    if(GetRatio() != 0.0)
    {
        
        double fBoundWidth = (double)(maViewportRectangle.GetWidth() + 1);
        double fBoundHeight = (double)(maViewportRectangle.GetHeight() + 1);
        double fActRatio = 1;
        double fFactor;

        if(fBoundWidth != 0.0)
            fActRatio = fBoundHeight / fBoundWidth;
        

        switch(meRatio)
        {
            case Base3DRatioShrink :
            {
                
                if(fActRatio > mfRatio)
                {
                    
                    fFactor = 1.0 / fActRatio;
                    fRight  *= fFactor;
                    fLeft *= fFactor;
                }
                else
                {
                    
                    fFactor = fActRatio;
                    fTop *= fFactor;
                    fBottom *= fFactor;
                }
                break;
            }
            case Base3DRatioGrow :
            {
                
                if(fActRatio > mfRatio)
                {
                    
                    fFactor = fActRatio;
                    fTop *= fFactor;
                    fBottom *= fFactor;
                }
                else
                {
                    
                    fFactor = 1.0 / fActRatio;
                    fRight  *= fFactor;
                    fLeft *= fFactor;
                }
                break;
            }
            case Base3DRatioMiddle :
            {
                
                fFactor = ((1.0 / fActRatio) + 1.0) / 2.0;
                fRight *= fFactor;
                fLeft *= fFactor;
                fFactor = (fActRatio + 1.0) / 2.0;
                fTop *= fFactor;
                fBottom *= fFactor;
                break;
            }
        }
    }

    
    maSetBound = maViewportRectangle;

    
    basegfx::B3DHomMatrix aNewProjection;

    
    
    
    
    const double fDistPart((mfFarBound - mfNearBound) * 0.0001);

    
    if(mbPerspective)
    {
        Frustum(aNewProjection, fLeft, fRight, fBottom, fTop, mfNearBound - fDistPart, mfFarBound + fDistPart);
    }
    else
    {
        Ortho(aNewProjection, fLeft, fRight, fBottom, fTop, mfNearBound - fDistPart, mfFarBound + fDistPart);
    }

    
    mbProjectionValid = true;

    
    SetProjection(aNewProjection);

    
    
    maTranslate.setX((double)maSetBound.Left() + ((maSetBound.GetWidth() - 1L) / 2.0));
    maTranslate.setY((double)maSetBound.Top() + ((maSetBound.GetHeight() - 1L) / 2.0));
    maTranslate.setZ(ZBUFFER_DEPTH_RANGE / 2.0);

    
    maScale.setX((maSetBound.GetWidth() - 1L) / 2.0);
    maScale.setY((maSetBound.GetHeight() - 1L) / -2.0);
    maScale.setZ(ZBUFFER_DEPTH_RANGE / 2.0);

    
    PostSetViewport();
}

void B3dTransformationSet::SetRatio(double fNew)
{
    if(mfRatio != fNew)
    {
        mfRatio = fNew;
        mbProjectionValid = false;
        mbObjectToDeviceValid = false;
        mbWorldToViewValid = false;
    }
}

void B3dTransformationSet::SetDeviceRectangle(double fL, double fR, double fB, double fT,
                                              bool bBroadCastChange)
{
    if(fL != mfLeftBound || fR != mfRightBound || fB != mfBottomBound || fT != mfTopBound)
    {
        mfLeftBound = fL;
        mfRightBound = fR;
        mfBottomBound = fB;
        mfTopBound = fT;

        mbProjectionValid = false;
        mbObjectToDeviceValid = false;
        mbWorldToViewValid = false;

        
        if(bBroadCastChange)
            DeviceRectangleChange();
    }
}

void B3dTransformationSet::DeviceRectangleChange()
{
}

void B3dTransformationSet::SetPerspective(bool bNew)
{
    if(mbPerspective != bNew)
    {
        mbPerspective = bNew;
        mbProjectionValid = false;
        mbObjectToDeviceValid = false;
        mbWorldToViewValid = false;
    }
}

void B3dTransformationSet::SetViewportRectangle(Rectangle& rRect, Rectangle& rVisible)
{
    if(rRect != maViewportRectangle || rVisible != maVisibleRectangle)
    {
        maViewportRectangle = rRect;
        maVisibleRectangle = rVisible;

        mbProjectionValid = false;
        mbObjectToDeviceValid = false;
        mbWorldToViewValid = false;
    }
}

void B3dTransformationSet::PostSetViewport()
{
}



const basegfx::B3DPoint B3dTransformationSet::WorldToEyeCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetOrientation();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::EyeToWorldCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvOrientation();
    return aVec;
}



B3dViewport::B3dViewport()
:   B3dTransformationSet(),
    aVRP(0, 0, 0),
    aVPN(0, 0, 1),
    aVUV(0, 1, 0)
{
    CalcOrientation();
}

B3dViewport::~B3dViewport()
{
}

void B3dViewport::SetVUV(const basegfx::B3DVector& rNewVUV)
{
    aVUV = rNewVUV;
    CalcOrientation();
}

void B3dViewport::SetViewportValues(
    const basegfx::B3DPoint& rNewVRP,
    const basegfx::B3DVector& rNewVPN,
    const basegfx::B3DVector& rNewVUV)
{
    aVRP = rNewVRP;
    aVPN = rNewVPN;
    aVUV = rNewVUV;
    CalcOrientation();
}

void B3dViewport::CalcOrientation()
{
    SetOrientation(aVRP, aVPN, aVUV);
}



B3dCamera::B3dCamera(
    const basegfx::B3DPoint& rPos, const basegfx::B3DVector& rLkAt,
    double fFocLen, double fBnkAng, bool bUseFocLen)
:   B3dViewport(),
    aPosition(rPos),
    aCorrectedPosition(rPos),
    aLookAt(rLkAt),
    fFocalLength(fFocLen),
    fBankAngle(fBnkAng),
    bUseFocalLength(bUseFocLen)
{
    CalcNewViewportValues();
}

B3dCamera::~B3dCamera()
{
}

void B3dCamera::DeviceRectangleChange()
{
    
    B3dViewport::DeviceRectangleChange();

    
    CalcNewViewportValues();
}

void B3dCamera::CalcNewViewportValues()
{
    basegfx::B3DVector aViewVector(aPosition - aLookAt);
    basegfx::B3DVector aNewVPN(aViewVector);

    basegfx::B3DVector aNewVUV(0.0, 1.0, 0.0);
    if(aNewVPN.getLength() < aNewVPN.getY())
        aNewVUV.setX(0.5);

    aNewVUV.normalize();
    aNewVPN.normalize();

    basegfx::B3DVector aNewToTheRight = aNewVPN;
    aNewToTheRight = aNewToTheRight.getPerpendicular(aNewVUV);
    aNewToTheRight.normalize();
    aNewVUV = aNewToTheRight.getPerpendicular(aNewVPN);
    aNewVUV.normalize();

    SetViewportValues(aPosition, aNewVPN, aNewVUV);
    if(CalcFocalLength())
        SetViewportValues(aCorrectedPosition, aNewVPN, aNewVUV);

    if(fBankAngle != 0.0)
    {
        basegfx::B3DHomMatrix aRotMat;
        aRotMat.rotate(0.0, 0.0, fBankAngle);
        basegfx::B3DVector aUp(0.0, 1.0, 0.0);
        aUp *= aRotMat;
        aUp = EyeToWorldCoor(aUp);
        aUp.normalize();
        SetVUV(aUp);
    }
}

bool B3dCamera::CalcFocalLength()
{
    double fWidth = GetDeviceRectangleWidth();
    bool bRetval = false;

    if(bUseFocalLength)
    {
        
        aCorrectedPosition = basegfx::B3DPoint(0.0, 0.0, fFocalLength * fWidth / 35.0);
        aCorrectedPosition = EyeToWorldCoor(aCorrectedPosition);
        bRetval = true;
    }
    else
    {
        
        basegfx::B3DPoint aOldPosition;
        aOldPosition = WorldToEyeCoor(aOldPosition);
        if(fWidth != 0.0)
            fFocalLength = aOldPosition.getZ() / fWidth * 35.0;
        if(fFocalLength < 5.0)
            fFocalLength = 5.0;
    }
    return bRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
