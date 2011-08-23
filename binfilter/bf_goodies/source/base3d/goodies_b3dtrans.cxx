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

#include "b3dtrans.hxx"

#include "b3dvolum.hxx"

namespace binfilter {

/*************************************************************************
|*
|* Transformationen fuer alle 3D Ausgaben
|*
\************************************************************************/

B3dTransformationSet::B3dTransformationSet()
{
    Reset();
}

/*************************************************************************
|*
|* Reset der Werte
|*
\************************************************************************/

void B3dTransformationSet::Reset()
{
    // Matritzen auf Einheitsmatritzen
    aObjectTrans.Identity();
    PostSetObjectTrans();

    aOrientation.Orientation();
    PostSetOrientation();

    aTexture.Identity();

    fLeftBound = fBottomBound = -1.0;
    fRightBound = fTopBound = 1.0;
    fNearBound = 0.001;
    fFarBound = 1.001;

    eRatio = Base3DRatioGrow;
    fRatio = 0.0;

    aViewportRectangle = Rectangle(-1, -1, 2, 2);
    aVisibleRectangle = aViewportRectangle;

    bPerspective = TRUE;

    bProjectionValid = FALSE;
    bObjectToDeviceValid = FALSE;
    bWorldToViewValid = FALSE;

    CalcViewport();
}

/*************************************************************************
|*
|* Objekttransformation
|*
\************************************************************************/

void B3dTransformationSet::SetObjectTrans(Matrix4D& rObj)
{
    aObjectTrans = rObj;

    bObjectToDeviceValid = FALSE;
    bInvTransObjectToEyeValid = FALSE;

    PostSetObjectTrans();
}

void B3dTransformationSet::PostSetObjectTrans()
{
    // Zuweisen und Inverse bestimmen
    aInvObjectTrans = aObjectTrans;
    aInvObjectTrans.Invert();
}

/*************************************************************************
|*
|* Orientierungstransformation
|*
\************************************************************************/

#if ! defined ICC && ! defined __GNUC__
void B3dTransformationSet::SetOrientation( Vector3D& aVRP, Vector3D& aVPN, Vector3D& aVUP)
#else
void B3dTransformationSet::SetOrientation( Vector3D aVRP, Vector3D aVPN, Vector3D aVUP)
#endif
{
    aOrientation.Identity();
    aOrientation.Orientation(Point4D(aVRP), aVPN, aVUP);

    bInvTransObjectToEyeValid = FALSE;
    bObjectToDeviceValid = FALSE;
    bWorldToViewValid = FALSE;

    PostSetOrientation();
}

void B3dTransformationSet::PostSetOrientation()
{
    // Zuweisen und Inverse bestimmen
    aInvOrientation = aOrientation;
    aInvOrientation.Invert();
}

/*************************************************************************
|*
|* Projektionstransformation
|*
\************************************************************************/

void B3dTransformationSet::SetProjection(Matrix4D& mProject)
{
    aProjection = mProject;
    PostSetProjection();
}

const Matrix4D& B3dTransformationSet::GetProjection()
{
    if(!bProjectionValid)
        CalcViewport();
    return aProjection;
}

const Matrix4D& B3dTransformationSet::GetInvProjection()
{
    if(!bProjectionValid)
        CalcViewport();
    return aInvProjection;
}

void B3dTransformationSet::PostSetProjection()
{
    // Zuweisen und Inverse bestimmen
    aInvProjection = GetProjection();
    aInvProjection.Invert();

    // Abhaengige Matritzen invalidieren
    bObjectToDeviceValid = FALSE;
    bWorldToViewValid = FALSE;
}

/*************************************************************************
|*
|* Viewport-Transformation
|*
\************************************************************************/

void B3dTransformationSet::CalcViewport()
{
    // Faktoren fuer die Projektion
    double fLeft = fLeftBound;
    double fRight = fRightBound;
    double fBottom = fBottomBound;
    double fTop = fTopBound;

    // Soll das Seitenverhaeltnis Beachtung finden?
    // Falls ja, Bereich der Projektion an Seitenverhaeltnis anpassen
    if(GetRatio() != 0.0)
    {
        // Berechne aktuelles Seitenverhaeltnis der Bounds
        double fBoundWidth = (double)(aViewportRectangle.GetWidth() + 1);
        double fBoundHeight = (double)(aViewportRectangle.GetHeight() + 1);
        double fActRatio = 1;
        double fFactor;

        if(fBoundWidth != 0.0)
            fActRatio = fBoundHeight / fBoundWidth;
        // FIXME   else in this case has a lot of problems,  should this return.

        switch(eRatio)
        {
            case Base3DRatioShrink :
            {
                // Kleineren Teil vergroessern
                if(fActRatio > fRatio)
                {
                    // X vergroessern
                    fFactor = 1.0 / fActRatio;
                    fRight	*= fFactor;
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
                if(fActRatio > fRatio)
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
                    fRight	*= fFactor;
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
    aSetBound = aViewportRectangle;

    // Mit den neuen Werten Projektion und ViewPort setzen
    Matrix4D aNewProjection;

    // #i36281#
    // OpenGL needs a little more rough additional size to not let
    // the front face vanish. Changed from SMALL_DVALUE to 0.000001,
    // which is 1/10000th, comared with 1/tenth of a million from SMALL_DVALUE.
    const double fDistPart((fFarBound - fNearBound) * 0.0001);

    // Near, Far etwas grosszuegiger setzen, um falsches,
    // zu kritisches clippen zu verhindern
    if(bPerspective)
        aNewProjection.Frustum(fLeft, fRight, fBottom, fTop,
        fNearBound - fDistPart, fFarBound + fDistPart);
    else
        aNewProjection.Ortho(fLeft, fRight, fBottom, fTop,
        fNearBound - fDistPart, fFarBound + fDistPart);

    // jetzt schon auf gueltig setzen um Endlosschleife zu vermeiden
    bProjectionValid = TRUE;

    // Neue Projektion setzen
    SetProjection(aNewProjection);

    // fill parameters for ViewportTransformation
    // Translation
    aTranslate[0] = (double)aSetBound.Left() + ((aSetBound.GetWidth() - 1L) / 2.0);
    aTranslate[1] = (double)aSetBound.Top() + ((aSetBound.GetHeight() - 1L) / 2.0);
    aTranslate[2] = ZBUFFER_DEPTH_RANGE / 2.0;

    // Skalierung
    aScale[0] = (aSetBound.GetWidth() - 1L) / 2.0;
    aScale[1] = (aSetBound.GetHeight() - 1L) / -2.0;
    aScale[2] = ZBUFFER_DEPTH_RANGE / 2.0;

    // Auf Veraenderung des ViewPorts reagieren
    PostSetViewport();
}

void B3dTransformationSet::SetRatio(double fNew)
{
    if(fRatio != fNew)
    {
        fRatio = fNew;
        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;
    }
}

void B3dTransformationSet::SetDeviceRectangle(double fL, double fR, double fB, double fT,
    BOOL bBroadCastChange)
{
    if(fL != fLeftBound || fR != fRightBound || fB != fBottomBound || fT != fTopBound)
    {
        fLeftBound = fL;
        fRightBound = fR;
        fBottomBound = fB;
        fTopBound = fT;

        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;

        // Aenderung bekanntmachen
        if(bBroadCastChange)
            DeviceRectangleChange();
    }
}

void B3dTransformationSet::SetDeviceVolume(const B3dVolume& rVol, BOOL bBroadCastChange)
{
    SetDeviceRectangle(rVol.MinVec().X(), rVol.MaxVec().X(),
        rVol.MinVec().Y(), rVol.MaxVec().Y(), bBroadCastChange);
    SetFrontClippingPlane(rVol.MinVec().Z());
    SetBackClippingPlane(rVol.MaxVec().Z());
}

void B3dTransformationSet::DeviceRectangleChange()
{
}

void B3dTransformationSet::SetFrontClippingPlane(double fF)
{
    if(fNearBound != fF)
    {
        fNearBound = fF;
        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;
    }
}

void B3dTransformationSet::SetBackClippingPlane(double fB)
{
    if(fFarBound != fB)
    {
        fFarBound = fB;
        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;
    }
}

void B3dTransformationSet::SetPerspective(BOOL bNew)
{
    if(bPerspective != bNew)
    {
        bPerspective = bNew;
        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;
    }
}

void B3dTransformationSet::SetViewportRectangle(Rectangle& rRect, Rectangle& rVisible)
{
    if(rRect != aViewportRectangle || rVisible != aVisibleRectangle)
    {
        aViewportRectangle = rRect;
        aVisibleRectangle = rVisible;

        bProjectionValid = FALSE;
        bObjectToDeviceValid = FALSE;
        bWorldToViewValid = FALSE;
    }
}

void B3dTransformationSet::PostSetViewport()
{
}

const Vector3D& B3dTransformationSet::GetScale()
{
    if(!bProjectionValid)
        CalcViewport();
    return aScale;
}

const Vector3D& B3dTransformationSet::GetTranslate()
{
    if(!bProjectionValid)
        CalcViewport();
    return aTranslate;
}

/*************************************************************************
|*
|* Hilfsmatrixberechnungsroutinen
|*
\************************************************************************/

void B3dTransformationSet::CalcMatFromWorldToView()
{
    aMatFromWorldToView = aOrientation;
    aMatFromWorldToView *= GetProjection();
    aMatFromWorldToView.Scale(GetScale());
    aMatFromWorldToView.Translate(GetTranslate());
    aInvMatFromWorldToView = aMatFromWorldToView;
    aInvMatFromWorldToView.Invert();

    // gueltig setzen
    bWorldToViewValid = TRUE;
}

const Matrix4D& B3dTransformationSet::GetMatFromWorldToView()
{
    if(!bWorldToViewValid)
        CalcMatFromWorldToView();
    return aMatFromWorldToView;
}

const Matrix4D& B3dTransformationSet::GetInvMatFromWorldToView()
{
    if(!bWorldToViewValid)
        CalcMatFromWorldToView();
    return aInvMatFromWorldToView;
}

/*************************************************************************
|*
|* Direkter Zugriff auf verschiedene Transformationen
|*
\************************************************************************/

const Vector3D B3dTransformationSet::WorldToEyeCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetOrientation();
    return aVec;
}

const Vector3D B3dTransformationSet::EyeToWorldCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvOrientation();
    return aVec;
}

const Vector3D B3dTransformationSet::ViewToEyeCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec -= GetTranslate();
    aVec = aVec / GetScale();
    aVec *= GetInvProjection();
    return aVec;
}

const Vector3D B3dTransformationSet::WorldToViewCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetMatFromWorldToView();
    return aVec;
}

const Vector3D B3dTransformationSet::ObjectToWorldCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetObjectTrans();
    return aVec;
}

const Vector3D B3dTransformationSet::ViewToObjectCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvMatFromWorldToView();
    aVec *= GetInvObjectTrans();
    return aVec;
}

const Vector3D B3dTransformationSet::ObjectToEyeCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetObjectTrans();
    aVec *= GetOrientation();
    return aVec;
}

/*************************************************************************
|*
|* Konstruktor B3dViewport
|*
\************************************************************************/

B3dViewport::B3dViewport()
:	B3dTransformationSet(),
    aVRP(0, 0, 0),
    aVPN(0, 0, 1),
    aVUV(0, 1, 0)
{
    CalcOrientation();
}

void B3dViewport::SetVUV(const Vector3D& rNewVUV)
{
    aVUV = rNewVUV;
    CalcOrientation();
}

void B3dViewport::SetViewportValues(
    const Vector3D& rNewVRP,
    const Vector3D& rNewVPN,
    const Vector3D& rNewVUV)
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

B3dCamera::B3dCamera(const Vector3D& rPos, const Vector3D& rLkAt,
    double fFocLen, double fBnkAng, BOOL bUseFocLen)
:	B3dViewport(),
    aPosition(rPos),
    aCorrectedPosition(rPos),
    aLookAt(rLkAt),
    fFocalLength(fFocLen),
    fBankAngle(fBnkAng),
    bUseFocalLength(bUseFocLen)
{
    CalcNewViewportValues();
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
    Vector3D aViewVector = aPosition - aLookAt;
    Vector3D aNewVPN = aViewVector;

    Vector3D aNewVUV(0.0, 1.0, 0.0);
    if(aNewVPN.GetLength() < aNewVPN.Y())
        aNewVUV.X() = 0.5;

    aNewVUV.Normalize();
    aNewVPN.Normalize();

    Vector3D aNewToTheRight = aNewVPN;
    aNewToTheRight |= aNewVUV;
    aNewToTheRight.Normalize();
    aNewVUV = aNewToTheRight | aNewVPN;
    aNewVUV.Normalize();

    SetViewportValues(aPosition, aNewVPN, aNewVUV);
    if(CalcFocalLength())
        SetViewportValues(aCorrectedPosition, aNewVPN, aNewVUV);

    if(fBankAngle != 0.0)
    {
        Matrix4D aRotMat;
        aRotMat.RotateZ(fBankAngle);
        Vector3D aUp(0.0, 1.0, 0.0);
        aUp *= aRotMat;
        aUp = EyeToWorldCoor(aUp);
        aUp.Normalize();
        SetVUV(aUp);
    }
}

BOOL B3dCamera::CalcFocalLength()
{
    double fWidth = GetDeviceRectangleWidth();
    BOOL bRetval = FALSE;

    if(bUseFocalLength)
    {
        // Position aufgrund der FocalLength korrigieren
        aCorrectedPosition = Vector3D(0.0, 0.0, fFocalLength * fWidth / 35.0);
        aCorrectedPosition = EyeToWorldCoor(aCorrectedPosition);
        bRetval = TRUE;
    }
    else
    {
        // FocalLength anhand der Position anpassen
        Vector3D aOldPosition;
        aOldPosition = WorldToEyeCoor(aOldPosition);
        if(fWidth != 0.0)
            fFocalLength = aOldPosition.Z() / fWidth * 35.0;
        if(fFocalLength < 5.0)
            fFocalLength = 5.0;
    }
    return bRetval;
}
}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
