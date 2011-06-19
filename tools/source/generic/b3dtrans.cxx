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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <tools/b3dtrans.hxx>
#include <tools/debug.hxx>

/*************************************************************************
|*
|* Transformationen fuer alle 3D Ausgaben
|*
\************************************************************************/

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

void B3dTransformationSet::Ortho(basegfx::B3DHomMatrix& rTarget, double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
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

/*************************************************************************
|*
|* Reset der Werte
|*
\************************************************************************/

void B3dTransformationSet::Reset()
{
    // Matritzen auf Einheitsmatritzen
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

    mbPerspective = sal_True;

    mbProjectionValid = sal_False;
    mbObjectToDeviceValid = sal_False;
    mbWorldToViewValid = sal_False;

    CalcViewport();
}

/*************************************************************************
|*
|* Objekttransformation
|*
\************************************************************************/

void B3dTransformationSet::SetObjectTrans(const basegfx::B3DHomMatrix& rObj)
{
    maObjectTrans = rObj;

    mbObjectToDeviceValid = sal_False;
    mbInvTransObjectToEyeValid = sal_False;

    PostSetObjectTrans();
}

void B3dTransformationSet::PostSetObjectTrans()
{
    // Zuweisen und Inverse bestimmen
    maInvObjectTrans = maObjectTrans;
    maInvObjectTrans.invert();
}

/*************************************************************************
|*
|* Orientierungstransformation
|*
\************************************************************************/

void B3dTransformationSet::SetOrientation( basegfx::B3DPoint aVRP, basegfx::B3DVector aVPN, basegfx::B3DVector aVUP)
{
    maOrientation.identity();
    Orientation(maOrientation, aVRP, aVPN, aVUP);

    mbInvTransObjectToEyeValid = sal_False;
    mbObjectToDeviceValid = sal_False;
    mbWorldToViewValid = sal_False;

    PostSetOrientation();
}

void B3dTransformationSet::SetOrientation(basegfx::B3DHomMatrix& mOrient)
{
    maOrientation = mOrient;

    mbInvTransObjectToEyeValid = sal_False;
    mbObjectToDeviceValid = sal_False;
    mbWorldToViewValid = sal_False;

    PostSetOrientation();
}

void B3dTransformationSet::PostSetOrientation()
{
    // Zuweisen und Inverse bestimmen
    maInvOrientation = maOrientation;
    maInvOrientation.invert();
}

/*************************************************************************
|*
|* Projektionstransformation
|*
\************************************************************************/

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

const basegfx::B3DHomMatrix& B3dTransformationSet::GetInvProjection()
{
    if(!mbProjectionValid)
        CalcViewport();
    return maInvProjection;
}

void B3dTransformationSet::PostSetProjection()
{
    // Zuweisen und Inverse bestimmen
    maInvProjection = GetProjection();
    maInvProjection.invert();

    // Abhaengige Matritzen invalidieren
    mbObjectToDeviceValid = sal_False;
    mbWorldToViewValid = sal_False;
}

/*************************************************************************
|*
|* Texturtransformation
|*
\************************************************************************/

void B3dTransformationSet::SetTexture(const basegfx::B2DHomMatrix& rTxt)
{
    maTexture = rTxt;
    PostSetTexture();
}

void B3dTransformationSet::PostSetTexture()
{
}

/*************************************************************************
|*
|* Viewport-Transformation
|*
\************************************************************************/

void B3dTransformationSet::CalcViewport()
{
    // Faktoren fuer die Projektion
    double fLeft(mfLeftBound);
    double fRight(mfRightBound);
    double fBottom(mfBottomBound);
    double fTop(mfTopBound);

    // Soll das Seitenverhaeltnis Beachtung finden?
    // Falls ja, Bereich der Projektion an Seitenverhaeltnis anpassen
    if(GetRatio() != 0.0)
    {
        // Berechne aktuelles Seitenverhaeltnis der Bounds
        double fBoundWidth = (double)(maViewportRectangle.GetWidth() + 1);
        double fBoundHeight = (double)(maViewportRectangle.GetHeight() + 1);
        double fActRatio = 1;
        double fFactor;

        if(fBoundWidth != 0.0)
            fActRatio = fBoundHeight / fBoundWidth;
        // FIXME   else in this case has a lot of problems,  should this return.

        switch(meRatio)
        {
            case Base3DRatioShrink :
            {
                // Kleineren Teil vergroessern
                if(fActRatio > mfRatio)
                {
                    // X vergroessern
                    fFactor = 1.0 / fActRatio;
                    fRight  *= fFactor;
                    fLeft *= fFactor;
                }
                else
                {
                    // Y vergroessern
                    fFactor = fActRatio;
                    fTop *= fFactor;
                    fBottom *= fFactor;
                }
                break;
            }
            case Base3DRatioGrow :
            {
                // GroesserenTeil verkleinern
                if(fActRatio > mfRatio)
                {
                    // Y verkleinern
                    fFactor = fActRatio;
                    fTop *= fFactor;
                    fBottom *= fFactor;
                }
                else
                {
                    // X verkleinern
                    fFactor = 1.0 / fActRatio;
                    fRight  *= fFactor;
                    fLeft *= fFactor;
                }
                break;
            }
            case Base3DRatioMiddle :
            {
                // Mitteln
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

    // Ueberschneiden sich Darstellungsflaeche und Objektflaeche?
    maSetBound = maViewportRectangle;

    // Mit den neuen Werten Projektion und ViewPort setzen
    basegfx::B3DHomMatrix aNewProjection;

    // #i36281#
    // OpenGL needs a little more rough additional size to not let
    // the front face vanish. Changed from SMALL_DVALUE to 0.000001,
    // which is 1/10000th, comared with 1/tenth of a million from SMALL_DVALUE.
    const double fDistPart((mfFarBound - mfNearBound) * 0.0001);

    // Near, Far etwas grosszuegiger setzen, um falsches,
    // zu kritisches clippen zu verhindern
    if(mbPerspective)
    {
        Frustum(aNewProjection, fLeft, fRight, fBottom, fTop, mfNearBound - fDistPart, mfFarBound + fDistPart);
    }
    else
    {
        Ortho(aNewProjection, fLeft, fRight, fBottom, fTop, mfNearBound - fDistPart, mfFarBound + fDistPart);
    }

    // jetzt schon auf gueltig setzen um Endlosschleife zu vermeiden
    mbProjectionValid = sal_True;

    // Neue Projektion setzen
    SetProjection(aNewProjection);

    // fill parameters for ViewportTransformation
    // Translation
    maTranslate.setX((double)maSetBound.Left() + ((maSetBound.GetWidth() - 1L) / 2.0));
    maTranslate.setY((double)maSetBound.Top() + ((maSetBound.GetHeight() - 1L) / 2.0));
    maTranslate.setZ(ZBUFFER_DEPTH_RANGE / 2.0);

    // Skalierung
    maScale.setX((maSetBound.GetWidth() - 1L) / 2.0);
    maScale.setY((maSetBound.GetHeight() - 1L) / -2.0);
    maScale.setZ(ZBUFFER_DEPTH_RANGE / 2.0);

    // Auf Veraenderung des ViewPorts reagieren
    PostSetViewport();
}

void B3dTransformationSet::SetRatio(double fNew)
{
    if(mfRatio != fNew)
    {
        mfRatio = fNew;
        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::SetRatioMode(Base3DRatio eNew)
{
    if(meRatio != eNew)
    {
        meRatio = eNew;
        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::SetDeviceRectangle(double fL, double fR, double fB, double fT,
    sal_Bool bBroadCastChange)
{
    if(fL != mfLeftBound || fR != mfRightBound || fB != mfBottomBound || fT != mfTopBound)
    {
        mfLeftBound = fL;
        mfRightBound = fR;
        mfBottomBound = fB;
        mfTopBound = fT;

        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;

        // Aenderung bekanntmachen
        if(bBroadCastChange)
            DeviceRectangleChange();
    }
}

void B3dTransformationSet::SetDeviceVolume(const basegfx::B3DRange& rVol, sal_Bool bBroadCastChange)
{
    SetDeviceRectangle(rVol.getMinX(), rVol.getMaxX(), rVol.getMinY(), rVol.getMaxY(), bBroadCastChange);
    SetFrontClippingPlane(rVol.getMinZ());
    SetBackClippingPlane(rVol.getMaxZ());
}

void B3dTransformationSet::DeviceRectangleChange()
{
}

void B3dTransformationSet::GetDeviceRectangle(double &fL, double &fR, double& fB, double& fT)
{
    fL = mfLeftBound;
    fR = mfRightBound;
    fB = mfBottomBound;
    fT = mfTopBound;

    mbProjectionValid = sal_False;
    mbObjectToDeviceValid = sal_False;
    mbWorldToViewValid = sal_False;
}

basegfx::B3DRange B3dTransformationSet::GetDeviceVolume()
{
    basegfx::B3DRange aRet;

    aRet.expand(basegfx::B3DTuple(mfLeftBound, mfBottomBound, mfNearBound));
    aRet.expand(basegfx::B3DTuple(mfRightBound, mfTopBound, mfFarBound));

    return aRet;
}

void B3dTransformationSet::SetFrontClippingPlane(double fF)
{
    if(mfNearBound != fF)
    {
        mfNearBound = fF;
        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::SetBackClippingPlane(double fB)
{
    if(mfFarBound != fB)
    {
        mfFarBound = fB;
        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::SetPerspective(sal_Bool bNew)
{
    if(mbPerspective != bNew)
    {
        mbPerspective = bNew;
        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::SetViewportRectangle(Rectangle& rRect, Rectangle& rVisible)
{
    if(rRect != maViewportRectangle || rVisible != maVisibleRectangle)
    {
        maViewportRectangle = rRect;
        maVisibleRectangle = rVisible;

        mbProjectionValid = sal_False;
        mbObjectToDeviceValid = sal_False;
        mbWorldToViewValid = sal_False;
    }
}

void B3dTransformationSet::PostSetViewport()
{
}

const Rectangle& B3dTransformationSet::GetLogicalViewportBounds()
{
    if(!mbProjectionValid)
        CalcViewport();
    return maSetBound;
}

const basegfx::B3DVector& B3dTransformationSet::GetScale()
{
    if(!mbProjectionValid)
        CalcViewport();
    return maScale;
}

const basegfx::B3DVector& B3dTransformationSet::GetTranslate()
{
    if(!mbProjectionValid)
        CalcViewport();
    return maTranslate;
}

/*************************************************************************
|*
|* Hilfsmatrixberechnungsroutinen
|*
\************************************************************************/

void B3dTransformationSet::CalcMatObjectToDevice()
{
    // ObjectToDevice berechnen (Orientation * Projection * Object)
    maObjectToDevice = maObjectTrans;
    maObjectToDevice *= maOrientation;
    maObjectToDevice *= GetProjection();

    // auf gueltig setzen
    mbObjectToDeviceValid = sal_True;
}

const basegfx::B3DHomMatrix& B3dTransformationSet::GetObjectToDevice()
{
    if(!mbObjectToDeviceValid)
        CalcMatObjectToDevice();
    return maObjectToDevice;
}

void B3dTransformationSet::CalcMatInvTransObjectToEye()
{
    maInvTransObjectToEye = maObjectTrans;
    maInvTransObjectToEye *= maOrientation;
    maInvTransObjectToEye.invert();
    maInvTransObjectToEye.transpose();

    // eventuelle Translationen rausschmeissen, da diese
    // Matrix nur zur Transformation von Vektoren gedacht ist
    maInvTransObjectToEye.set(3, 0, 0.0);
    maInvTransObjectToEye.set(3, 1, 0.0);
    maInvTransObjectToEye.set(3, 2, 0.0);
    maInvTransObjectToEye.set(3, 3, 1.0);

    // auf gueltig setzen
    mbInvTransObjectToEyeValid = sal_True;
}

const basegfx::B3DHomMatrix& B3dTransformationSet::GetInvTransObjectToEye()
{
    if(!mbInvTransObjectToEyeValid)
        CalcMatInvTransObjectToEye();
    return maInvTransObjectToEye;
}

basegfx::B3DHomMatrix B3dTransformationSet::GetMatFromObjectToView()
{
    basegfx::B3DHomMatrix aFromObjectToView = GetObjectToDevice();

    const basegfx::B3DVector& rScale(GetScale());
    aFromObjectToView.scale(rScale.getX(), rScale.getY(), rScale.getZ());
    const basegfx::B3DVector& rTranslate(GetTranslate());
    aFromObjectToView.translate(rTranslate.getX(), rTranslate.getY(), rTranslate.getZ());

    return aFromObjectToView;
}

void B3dTransformationSet::CalcMatFromWorldToView()
{
    maMatFromWorldToView = maOrientation;
    maMatFromWorldToView *= GetProjection();
    const basegfx::B3DVector& rScale(GetScale());
    maMatFromWorldToView.scale(rScale.getX(), rScale.getY(), rScale.getZ());
    const basegfx::B3DVector& rTranslate(GetTranslate());
    maMatFromWorldToView.translate(rTranslate.getX(), rTranslate.getY(), rTranslate.getZ());
    maInvMatFromWorldToView = maMatFromWorldToView;
    maInvMatFromWorldToView.invert();

    // gueltig setzen
    mbWorldToViewValid = sal_True;
}

const basegfx::B3DHomMatrix& B3dTransformationSet::GetMatFromWorldToView()
{
    if(!mbWorldToViewValid)
        CalcMatFromWorldToView();
    return maMatFromWorldToView;
}

const basegfx::B3DHomMatrix& B3dTransformationSet::GetInvMatFromWorldToView()
{
    if(!mbWorldToViewValid)
        CalcMatFromWorldToView();
    return maInvMatFromWorldToView;
}

/*************************************************************************
|*
|* Direkter Zugriff auf verschiedene Transformationen
|*
\************************************************************************/

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

const basegfx::B3DPoint B3dTransformationSet::EyeToViewCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetProjection();
    aVec *= GetScale();
    aVec += GetTranslate();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ViewToEyeCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec -= GetTranslate();
    aVec = aVec / GetScale();
    aVec *= GetInvProjection();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::WorldToViewCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetMatFromWorldToView();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ViewToWorldCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvMatFromWorldToView();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::DeviceToViewCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetScale();
    aVec += GetTranslate();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ViewToDeviceCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec -= GetTranslate();
    aVec = aVec / GetScale();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ObjectToWorldCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetObjectTrans();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::WorldToObjectCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvObjectTrans();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ObjectToViewCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetObjectTrans();
    aVec *= GetMatFromWorldToView();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ViewToObjectCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvMatFromWorldToView();
    aVec *= GetInvObjectTrans();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::ObjectToEyeCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetObjectTrans();
    aVec *= GetOrientation();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::EyeToObjectCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvOrientation();
    aVec *= GetInvObjectTrans();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::DeviceToEyeCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvProjection();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::EyeToDeviceCoor(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetProjection();
    return aVec;
}

const basegfx::B3DPoint B3dTransformationSet::InvTransObjectToEye(const basegfx::B3DPoint& rVec)
{
    basegfx::B3DPoint aVec(rVec);
    aVec *= GetInvTransObjectToEye();
    return aVec;
}

const basegfx::B2DPoint B3dTransformationSet::TransTextureCoor(const basegfx::B2DPoint& rVec)
{
    basegfx::B2DPoint aVec(rVec);
    aVec *= GetTexture();
    return aVec;
}

/*************************************************************************
|*
|* Konstruktor B3dViewport
|*
\************************************************************************/

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

void B3dViewport::SetVRP(const basegfx::B3DPoint& rNewVRP)
{
    aVRP = rNewVRP;
    CalcOrientation();
}

void B3dViewport::SetVPN(const basegfx::B3DVector& rNewVPN)
{
    aVPN = rNewVPN;
    CalcOrientation();
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

/*************************************************************************
|*
|* Konstruktor B3dViewport
|*
\************************************************************************/

B3dCamera::B3dCamera(
    const basegfx::B3DPoint& rPos, const basegfx::B3DVector& rLkAt,
    double fFocLen, double fBnkAng, sal_Bool bUseFocLen)
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

void B3dCamera::SetPosition(const basegfx::B3DPoint& rNewPos)
{
    if(rNewPos != aPosition)
    {
        // Zuweisen
        aCorrectedPosition = aPosition = rNewPos;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetLookAt(const basegfx::B3DVector& rNewLookAt)
{
    if(rNewLookAt != aLookAt)
    {
        // Zuweisen
        aLookAt = rNewLookAt;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetPositionAndLookAt(const basegfx::B3DPoint& rNewPos, const basegfx::B3DVector& rNewLookAt)
{
    if(rNewPos != aPosition || rNewLookAt != aLookAt)
    {
        // Zuweisen
        aPosition = rNewPos;
        aLookAt = rNewLookAt;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetFocalLength(double fLen)
{
    if(fLen != fFocalLength)
    {
        // Zuweisen
        if(fLen < 5.0)
            fLen = 5.0;
        fFocalLength = fLen;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetBankAngle(double fAngle)
{
    if(fAngle != fBankAngle)
    {
        // Zuweisen
        fBankAngle = fAngle;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetUseFocalLength(sal_Bool bNew)
{
    if(bNew != (sal_Bool)bUseFocalLength)
    {
        // Zuweisen
        bUseFocalLength = bNew;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::DeviceRectangleChange()
{
    // call parent
    B3dViewport::DeviceRectangleChange();

    // Auf Aenderung reagieren
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

sal_Bool B3dCamera::CalcFocalLength()
{
    double fWidth = GetDeviceRectangleWidth();
    sal_Bool bRetval = sal_False;

    if(bUseFocalLength)
    {
        // Position aufgrund der FocalLength korrigieren
        aCorrectedPosition = basegfx::B3DPoint(0.0, 0.0, fFocalLength * fWidth / 35.0);
        aCorrectedPosition = EyeToWorldCoor(aCorrectedPosition);
        bRetval = sal_True;
    }
    else
    {
        // FocalLength anhand der Position anpassen
        basegfx::B3DPoint aOldPosition;
        aOldPosition = WorldToEyeCoor(aOldPosition);
        if(fWidth != 0.0)
            fFocalLength = aOldPosition.getZ() / fWidth * 35.0;
        if(fFocalLength < 5.0)
            fFocalLength = 5.0;
    }
    return bRetval;
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
