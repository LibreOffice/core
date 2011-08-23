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

#ifndef _B3D_B3DLIGHT_HXX
#define _B3D_B3DLIGHT_HXX

#ifndef _SVX_VECTOR3D_HXX
#include "vector3d.hxx"
#endif

#ifndef _SVX_MATRIL3D_HXX
#include "matril3d.hxx"
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

namespace binfilter {

/*************************************************************************
|*
|* Unterstuetzte Lichtquellen
|*
\************************************************************************/

#define	BASE3D_MAX_NUMBER_LIGHTS		(8)

enum Base3DLightNumber
{
    Base3DLight0 = 0,
    Base3DLight1,
    Base3DLight2,
    Base3DLight3,
    Base3DLight4,
    Base3DLight5,
    Base3DLight6,
    Base3DLight7,
    Base3DLightNone
};

/*************************************************************************
|*
|* Basisklasse fuer Lichtquellen
|*
\************************************************************************/

class B3dLight
{
private:
    Color					aAmbient;
    Color					aDiffuse;
    Color					aSpecular;
    Vector3D				aPosition;
    Vector3D				aPositionEye;
    Vector3D				aSpotDirection;
    Vector3D				aSpotDirectionEye;
    UINT16					nSpotExponent;
    double					fSpotCutoff;
    double					fCosSpotCutoff;
    double					fConstantAttenuation;
    double					fLinearAttenuation;
    double					fQuadraticAttenuation;

    unsigned				bIsFirstLight			: 1;
    unsigned				bIsEnabled				: 1;
    unsigned				bIsDirectionalSource	: 1;
    unsigned				bIsSpot					: 1;
    unsigned				bIsAmbient				: 1;
    unsigned				bIsDiffuse				: 1;
    unsigned				bIsSpecular				: 1;
    unsigned				bLinearOrQuadratic		: 1;

public:
    B3dLight();

    // Zugriffsfunktionen
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient);
    const Color GetIntensity(Base3DMaterialValue=Base3DMaterialAmbient);
    void SetPositionEye(const Vector3D& rNew)
        { aPositionEye=rNew; }
    const Vector3D& GetPositionEye()
        { return aPositionEye; }
    void SetPosition(const Vector3D& rNew)
        { aPosition=rNew; }
    const Vector3D& GetPosition()
        { return aPosition; }
    const Vector3D& GetSpotDirection()
        { return aSpotDirection; }
    const Vector3D& GetSpotDirectionEye()
        { return aSpotDirectionEye; }
    void SetSpotExponent(UINT16 nNew)
        { nSpotExponent=nNew; }
    UINT16 GetSpotExponent()
        { return nSpotExponent; }
    double GetSpotCutoff()
        { return fSpotCutoff; }
    double GetCosSpotCutoff()
        { return fCosSpotCutoff; }
    void SetConstantAttenuation(double fNew)
        { fConstantAttenuation=fNew; }
    double GetConstantAttenuation()
        { return fConstantAttenuation; }
    double GetLinearAttenuation()
        { return fLinearAttenuation; }
    double GetQuadraticAttenuation()
        { return fQuadraticAttenuation; }

    void Enable(BOOL bNew=TRUE)
        { bIsEnabled=bNew; }
    BOOL IsEnabled()
        { return bIsEnabled; }
    void SetFirst(BOOL bNew=TRUE)
        { bIsFirstLight=bNew; }
    BOOL IsFirst()
        { return bIsFirstLight; }
    void SetDirectionalSource(BOOL bNew=TRUE)
        { bIsDirectionalSource=bNew; }
    BOOL IsDirectionalSource()
        { return bIsDirectionalSource; }
    BOOL IsSpot()
        { return bIsSpot; }
    BOOL IsAmbient()
        { return bIsAmbient; }
    BOOL IsDiffuse()
        { return bIsDiffuse; }
    BOOL IsSpecular()
        { return bIsSpecular; }
    BOOL IsLinearOrQuadratic()
        { return bLinearOrQuadratic; }

    void Init();

    // Laden/Speichern in StarView
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(SvStream& rIn);

protected:
};

/*************************************************************************
|*
|* Gruppe von Lichtquellen
|*
\************************************************************************/

class B3dLightGroup
{
private:
    // Lichtquellen
    B3dLight				aLight[BASE3D_MAX_NUMBER_LIGHTS];

    // lokale Parameter des LightModels
    Color					aGlobalAmbientLight;

    // Hauptschalter fuer die Beleuchtung
    unsigned				bLightingEnabled			: 1;
    unsigned				bLocalViewer				: 1;
    unsigned				bModelTwoSide				: 1;

public:
    // Konstruktor
    B3dLightGroup();

    // lokale Parameter des LightModels
    virtual void SetGlobalAmbientLight(const Color rNew);
    const Color GetGlobalAmbientLight();
    virtual void SetLocalViewer(BOOL bNew=TRUE);
    virtual void SetModelTwoSide(BOOL bNew=FALSE);
    BOOL GetModelTwoSide();

    // Hauptschalter fuer die Beleuchtung
    virtual void EnableLighting(BOOL bNew=TRUE);
    BOOL IsLightingEnabled();

    // Lichtquellen Interface
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    const Color GetIntensity(Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    void SetPosition(const Vector3D& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetDirection(const Vector3D& rNew,
        Base3DLightNumber=Base3DLight0);
    const Vector3D& GetDirection(Base3DLightNumber=Base3DLight0);
    void Enable(BOOL bNew=TRUE,
        Base3DLightNumber=Base3DLight0);
    BOOL IsEnabled(Base3DLightNumber=Base3DLight0);

    // Direkter Zugriff auf B3dLight
    B3dLight& GetLightObject(Base3DLightNumber=Base3DLight0);

    // Laden/Speichern in StarView
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(SvStream& rIn);

protected:
};

}//end of namespace binfilter

#endif          // _B3D_B3DLIGHT_HXX
