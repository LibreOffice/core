/*************************************************************************
 *
 *  $RCSfile: b3dtrans.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _B3D_VOLUM_HXX
#include "b3dvolum.hxx"
#endif

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

#ifndef ICC
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

void B3dTransformationSet::SetOrientation(Matrix4D& mOrient)
{
    aOrientation = mOrient;

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
|* Texturtransformation
|*
\************************************************************************/

void B3dTransformationSet::SetTexture(Matrix4D& rTxt)
{
    aTexture = rTxt;
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
        double fActRatio;
        double fFactor;

        if(fBoundWidth != 0.0)
            fActRatio = fBoundHeight / fBoundWidth;

        switch(eRatio)
        {
            case Base3DRatioShrink :
            {
                // Kleineren Teil vergroessern
                if(fActRatio > fRatio)
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
    aSetBound = aViewportRectangle;

    // Mit den neuen Werten Projektion und ViewPort setzen
    Matrix4D aNewProjection;
    double fDistPart = (fFarBound - fNearBound) * SMALL_DVALUE;

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

void B3dTransformationSet::SetRatioMode(Base3DRatio eNew)
{
    if(eRatio != eNew)
    {
        eRatio = eNew;
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

void B3dTransformationSet::GetDeviceRectangle(double &fL, double &fR, double& fB, double& fT)
{
    fL = fLeftBound;
    fR = fRightBound;
    fB = fBottomBound;
    fT = fTopBound;

    bProjectionValid = FALSE;
    bObjectToDeviceValid = FALSE;
    bWorldToViewValid = FALSE;
}

B3dVolume B3dTransformationSet::GetDeviceVolume()
{
    B3dVolume aRet;
    aRet.MinVec() = Vector3D(fLeftBound, fBottomBound, fNearBound);
    aRet.MaxVec() = Vector3D(fRightBound, fTopBound, fFarBound);
    return aRet;
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

const Rectangle& B3dTransformationSet::GetLogicalViewportBounds()
{
    if(!bProjectionValid)
        CalcViewport();
    return aSetBound;
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

void B3dTransformationSet::CalcMatObjectToDevice()
{
    // ObjectToDevice berechnen (Orientation * Projection * Object)
    aObjectToDevice = aObjectTrans;
    aObjectToDevice *= aOrientation;
    aObjectToDevice *= GetProjection();

    // auf gueltig setzen
    bObjectToDeviceValid = TRUE;
}

const Matrix4D& B3dTransformationSet::GetObjectToDevice()
{
    if(!bObjectToDeviceValid)
        CalcMatObjectToDevice();
    return aObjectToDevice;
}

void B3dTransformationSet::CalcMatInvTransObjectToEye()
{
    aInvTransObjectToEye = aObjectTrans;
    aInvTransObjectToEye *= aOrientation;
    aInvTransObjectToEye.Invert();
    aInvTransObjectToEye.Transpose();

    // eventuelle Translationen rausschmeissen, da diese
    // Matrix nur zur Transformation von Vektoren gedacht ist
    aInvTransObjectToEye[3] = Point4D(0.0, 0.0, 0.0, 1.0);

    // auf gueltig setzen
    bInvTransObjectToEyeValid = TRUE;
}

const Matrix4D& B3dTransformationSet::GetInvTransObjectToEye()
{
    if(!bInvTransObjectToEyeValid)
        CalcMatInvTransObjectToEye();
    return aInvTransObjectToEye;
}

Matrix4D B3dTransformationSet::GetMatFromObjectToView()
{
    Matrix4D aFromObjectToView = GetObjectToDevice();

    aFromObjectToView.Scale(GetScale());
    aFromObjectToView.Translate(GetTranslate());

    return aFromObjectToView;
}

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

const Vector3D B3dTransformationSet::EyeToViewCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetProjection();
    aVec *= GetScale();
    aVec += GetTranslate();
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

const Vector3D B3dTransformationSet::ViewToWorldCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvMatFromWorldToView();
    return aVec;
}

const Vector3D B3dTransformationSet::DeviceToViewCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetScale();
    aVec += GetTranslate();
    return aVec;
}

const Vector3D B3dTransformationSet::ViewToDeviceCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec -= GetTranslate();
    aVec = aVec / GetScale();
    return aVec;
}

const Vector3D B3dTransformationSet::ObjectToWorldCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetObjectTrans();
    return aVec;
}

const Vector3D B3dTransformationSet::WorldToObjectCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvObjectTrans();
    return aVec;
}

const Vector3D B3dTransformationSet::ObjectToViewCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetObjectTrans();
    aVec *= GetMatFromWorldToView();
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

const Vector3D B3dTransformationSet::EyeToObjectCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvOrientation();
    aVec *= GetInvObjectTrans();
    return aVec;
}

const Vector3D B3dTransformationSet::DeviceToEyeCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvProjection();
    return aVec;
}

const Vector3D B3dTransformationSet::EyeToDeviceCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetProjection();
    return aVec;
}

const Vector3D B3dTransformationSet::InvTransObjectToEye(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    aVec *= GetInvTransObjectToEye();
    return aVec;
}

const Vector3D B3dTransformationSet::TransTextureCoor(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
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

void B3dViewport::SetVRP(const Vector3D& rNewVRP)
{
    aVRP = rNewVRP;
    CalcOrientation();
}

void B3dViewport::SetVPN(const Vector3D& rNewVPN)
{
    aVPN = rNewVPN;
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

void B3dCamera::SetPosition(const Vector3D& rNewPos)
{
    if(rNewPos != aPosition)
    {
        // Zuweisen
        aCorrectedPosition = aPosition = rNewPos;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetLookAt(const Vector3D& rNewLookAt)
{
    if(rNewLookAt != aLookAt)
    {
        // Zuweisen
        aLookAt = rNewLookAt;

        // Neuberechnung
        CalcNewViewportValues();
    }
}

void B3dCamera::SetPositionAndLookAt(const Vector3D& rNewPos, const Vector3D& rNewLookAt)
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

void B3dCamera::SetUseFocalLength(BOOL bNew)
{
    if(bNew != (BOOL)bUseFocalLength)
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

