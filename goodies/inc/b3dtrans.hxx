/*************************************************************************
 *
 *  $RCSfile: b3dtrans.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:08 $
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
#define _B3D_B3DTRANS_HXX

#ifndef _B3D_HMATRIX_HXX
#include "hmatrix.hxx"
#endif

// Zu verwendender DephRange des Z-Buffers
#define ZBUFFER_DEPTH_RANGE         (256.0 * 256.0 * 256.0)

// Vorausdeklarationen
class Base3D;
class B3dVolume;

/*************************************************************************
|*
|* Unterstuetzte Methoden, um das Seitenverhaeltnis einzuhalten
|*
\************************************************************************/

enum Base3DRatio
{
    Base3DRatioGrow = 1,
    Base3DRatioShrink,
    Base3DRatioMiddle
};

/*************************************************************************
|*
|* Typ der Projektion
|*
\************************************************************************/

enum Base3DProjectionType
{
    Base3DProjectionTypeParallel = 1,
    Base3DProjectionTypePerspective
};

/*************************************************************************
|*
|* Transformationen fuer alle 3D Ausgaben
|*
\************************************************************************/

class B3dTransformationSet
{
private:
    // Object Matrix Object -> World
    Matrix4D                aObjectTrans;
    Matrix4D                aInvObjectTrans;

    // Orientation Matrix
    Matrix4D                aOrientation;
    Matrix4D                aInvOrientation;

    // Projection Matrix
    Matrix4D                aProjection;
    Matrix4D                aInvProjection;

    // Texture Matrices
    Matrix4D                aTexture;

    // Speziell zum Umwandeln von Punkten Objekt -> Device
    Matrix4D                aObjectToDevice;

    // Transponierte Inverse fuer Vectortransformationen
    Matrix4D                aInvTransObjectToEye;

    // Transformation World->View
    Matrix4D                aMatFromWorldToView;
    Matrix4D                aInvMatFromWorldToView;

    // Parameters for ViewportTransformation
    Vector3D                aScale;
    Vector3D                aTranslate;

    // ViewPlane DeviceRectangle (vom Benutzer gesetzt)
    double                  fLeftBound;
    double                  fRightBound;
    double                  fBottomBound;
    double                  fTopBound;

    // Near and far clipping planes
    double                  fNearBound;
    double                  fFarBound;

    // Seitenverhaeltnis der 3D Abbildung (Y / X)
    // default ist 1:1 -> 1.0
    // Deaktivieren mit 0.0 als Wert
    double                  fRatio;

    // Der gesetzte Ausgabebereich (in logischen Koordinaten)
    // und der dazugehoerige sichtbare Bereich
    Rectangle               aViewportRectangle;
    Rectangle               aVisibleRectangle;

    // Die tatsaechlich von CalcViewport gesetzten Abmessungen
    // des sichtbaren Bereichs (in logischen Koordinaten)
    Rectangle               aSetBound;

    // Methode zur Aufrechterhaltung des Seitenverhaeltnisses
    // default ist Base3DRatioGrow
    Base3DRatio             eRatio;

    // Flags
    unsigned                bPerspective                : 1;
    unsigned                bWorldToViewValid           : 1;
    unsigned                bInvTransObjectToEyeValid   : 1;
    unsigned                bObjectToDeviceValid        : 1;
    unsigned                bProjectionValid            : 1;

public:
    B3dTransformationSet();

    // Zurueck auf Standard
    void Reset();

    // ObjectTrans
    void SetObjectTrans(Matrix4D& rObj);
    const Matrix4D& GetObjectTrans() { return aObjectTrans; }
    const Matrix4D& GetInvObjectTrans() { return aInvObjectTrans; }

    // Orientation
#ifndef ICC
    void SetOrientation(Vector3D& aVRP = Vector3D(0.0,0.0,1.0),
        Vector3D& aVPN = Vector3D(0.0,0.0,1.0),
        Vector3D& aVUP = Vector3D(0.0,1.0,0.0));
#else
    void SetOrientation(Vector3D aVRP = Vector3D(0.0,0.0,1.0),
        Vector3D aVPN = Vector3D(0.0,0.0,1.0),
        Vector3D aVUP = Vector3D(0.0,1.0,0.0));
#endif
    void SetOrientation(Matrix4D& mOrient);
    const Matrix4D& GetOrientation() { return aOrientation; }
    const Matrix4D& GetInvOrientation() { return aInvOrientation; }

    // Projection
    void SetProjection(Matrix4D& mProject);
    const Matrix4D& GetProjection();
    const Matrix4D& GetInvProjection();

    // Texture
    void SetTexture(Matrix4D& rTxt);
    const Matrix4D& GetTexture() { return aTexture; }

    // Seitenverhaeltnis und Modus zu dessen Aufrechterhaltung
    double GetRatio() { return fRatio; }
    void SetRatio(double fNew=1.0);
    Base3DRatio GetRatioMode() { return eRatio; }
    void SetRatioMode(Base3DRatio eNew=Base3DRatioGrow);

    // Parameter der ViewportTransformation
    void SetDeviceRectangle(double fL=-1.0, double fR=1.0, double fB=-1.0, double fT=1.0,
        BOOL bBroadCastChange=TRUE);
    void SetDeviceVolume(const B3dVolume& rVol, BOOL bBroadCastChange=TRUE);
    void GetDeviceRectangle(double &fL, double &fR, double& fB, double& fT);
    B3dVolume GetDeviceVolume();
    double GetDeviceRectangleWidth() const { return fRightBound - fLeftBound; }
    double GetDeviceRectangleHeight() const { return fTopBound - fBottomBound; }
    void SetFrontClippingPlane(double fF=0.0);
    double GetFrontClippingPlane() { return fNearBound; }
    void SetBackClippingPlane(double fB=1.0);
    double GetBackClippingPlane() { return fFarBound; }
    void SetPerspective(BOOL bNew);
    BOOL GetPerspective() { return bPerspective; }
    void SetViewportRectangle(Rectangle& rRect, Rectangle& rVisible);
    void SetViewportRectangle(Rectangle& rRect) { SetViewportRectangle(rRect, rRect); }
    const Rectangle& GetViewportRectangle() { return aViewportRectangle; }
    void CalcViewport();

    // Spezielle Matritzen anfordern
    Matrix4D GetMatFromObjectToView();

    // Transponierte Inverse fuer Vectortransformationen
    const Matrix4D& GetInvTransObjectToEye();

    // Speziell zum Umwandeln von Punkten Objekt -> Device
    const Matrix4D& GetObjectToDevice();

    // Speziell zum Umwandeln von Punkten World -> View
    const Matrix4D& GetMatFromWorldToView();
    const Matrix4D& GetInvMatFromWorldToView();

    // Bounds des Viewports lesen
    const Rectangle& GetLogicalViewportBounds();
    const Vector3D& GetScale();
    const Vector3D& GetTranslate();

    // Direkter Zugriff auf verschiedene Transformationen
    const Vector3D WorldToEyeCoor(const Vector3D& rVec);
    const Vector3D EyeToWorldCoor(const Vector3D& rVec);
    const Vector3D EyeToViewCoor(const Vector3D& rVec);
    const Vector3D ViewToEyeCoor(const Vector3D& rVec);
    const Vector3D WorldToViewCoor(const Vector3D& rVec);
    const Vector3D ViewToWorldCoor(const Vector3D& rVec);
    const Vector3D DeviceToViewCoor(const Vector3D& rVec);
    const Vector3D ViewToDeviceCoor(const Vector3D& rVec);
    const Vector3D ObjectToWorldCoor(const Vector3D& rVec);
    const Vector3D WorldToObjectCoor(const Vector3D& rVec);
    const Vector3D ObjectToViewCoor(const Vector3D& rVec);
    const Vector3D ViewToObjectCoor(const Vector3D& rVec);
    const Vector3D ObjectToEyeCoor(const Vector3D& rVec);
    const Vector3D EyeToObjectCoor(const Vector3D& rVec);
    const Vector3D DeviceToEyeCoor(const Vector3D& rVec);
    const Vector3D EyeToDeviceCoor(const Vector3D& rVec);

    const Vector3D InvTransObjectToEye(const Vector3D& rVec);
    const Vector3D TransTextureCoor(const Vector3D& rVec);

protected:
    void PostSetObjectTrans();
    void PostSetOrientation();
    void PostSetProjection();
    void PostSetTexture();
    void PostSetViewport();

    void CalcMatObjectToDevice();
    void CalcMatFromWorldToView();
    void CalcMatInvTransObjectToEye();

    virtual void DeviceRectangleChange();
};

/*************************************************************************
|*
|* Viewport fuer B3D
|*
|* Verwendet wird hier ein vereinfachtes System, bei dem der abzubildende
|* Punkt durch VRP repraesentiert wird
|*
\************************************************************************/

class B3dViewport : public B3dTransformationSet
{
private:
    Vector3D                aVRP;           // View Reference Point
    Vector3D                aVPN;           // View Plane Normal
    Vector3D                aVUV;           // View Up Vector

public:
    B3dViewport();

    void SetVRP(const Vector3D& rNewVRP);
    void SetVPN(const Vector3D& rNewVPN);
    void SetVUV(const Vector3D& rNewVUV);
    void SetViewportValues(
        const Vector3D& rNewVRP,
        const Vector3D& rNewVPN,
        const Vector3D& rNewVUV);

    const Vector3D& GetVRP() const  { return aVRP; }
    const Vector3D& GetVPN() const  { return aVPN; }
    const Vector3D& GetVUV() const  { return aVUV; }

protected:
    void CalcOrientation();
};

/*************************************************************************
|*
|* Kamera fuer B3D
|*
\************************************************************************/

class B3dCamera : public B3dViewport
{
private:
    Vector3D                aPosition;
    Vector3D                aCorrectedPosition;
    Vector3D                aLookAt;
    double                  fFocalLength;
    double                  fBankAngle;

    unsigned                bUseFocalLength         : 1;

public:
    B3dCamera(const Vector3D& rPos = Vector3D(0.0, 0.0, 1.0),
        const Vector3D& rLkAt = Vector3D(0.0, 0.0, 0.0),
        double fFocLen = 35.0, double fBnkAng = 0.0,
        BOOL bUseFocLen = FALSE);

    // Positionen
    void SetPosition(const Vector3D& rNewPos);
    const Vector3D& GetPosition() const { return aPosition; }
    void SetLookAt(const Vector3D& rNewLookAt);
    const Vector3D& GetLookAt() const { return aLookAt; }
    void SetPositionAndLookAt(const Vector3D& rNewPos, const Vector3D& rNewLookAt);

    // Brennweite in mm
    void SetFocalLength(double fLen);
    double GetFocalLength() const { return fFocalLength; }

    // Neigung links/rechts
    void SetBankAngle(double fAngle);
    double GetBankAngle() const { return fBankAngle; }

    // FocalLength Flag
    void SetUseFocalLength(BOOL bNew);
    BOOL GetUseFocalLength() const { return (BOOL)bUseFocalLength; }

protected:
    void CalcNewViewportValues();
    BOOL CalcFocalLength();

    virtual void DeviceRectangleChange();
};


#endif          // _B3D_B3DTRANS_HXX
