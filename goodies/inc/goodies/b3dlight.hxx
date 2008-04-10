/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dlight.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SVX_MATRIL3D_HXX
#include <goodies/matril3d.hxx>
#endif
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>

/*************************************************************************
|*
|* Unterstuetzte Lichtquellen
|*
\************************************************************************/

#define BASE3D_MAX_NUMBER_LIGHTS        (8)

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
    Color                   aAmbient;
    Color                   aDiffuse;
    Color                   aSpecular;
    basegfx::B3DPoint               aPosition;
    basegfx::B3DPoint               aPositionEye;
    basegfx::B3DVector          aSpotDirection;
    basegfx::B3DVector          aSpotDirectionEye;
    sal_uInt16                      nSpotExponent;
    double                  fSpotCutoff;
    double                  fCosSpotCutoff;
    double                  fConstantAttenuation;
    double                  fLinearAttenuation;
    double                  fQuadraticAttenuation;

    unsigned                bIsFirstLight           : 1;
    unsigned                bIsEnabled              : 1;
    unsigned                bIsDirectionalSource    : 1;
    unsigned                bIsSpot                 : 1;
    unsigned                bIsAmbient              : 1;
    unsigned                bIsDiffuse              : 1;
    unsigned                bIsSpecular             : 1;
    unsigned                bLinearOrQuadratic      : 1;

public:
    B3dLight();

    // Zugriffsfunktionen
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient);
    const Color GetIntensity(Base3DMaterialValue=Base3DMaterialAmbient);
    void SetPositionEye(const basegfx::B3DPoint& rNew)
        { aPositionEye=rNew; }
    const basegfx::B3DPoint& GetPositionEye()
        { return aPositionEye; }
    void SetPosition(const basegfx::B3DPoint& rNew)
        { aPosition=rNew; }
    const basegfx::B3DPoint& GetPosition()
        { return aPosition; }
    void SetSpotDirection(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetSpotDirection()
        { return aSpotDirection; }
    void SetSpotDirectionEye(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetSpotDirectionEye()
        { return aSpotDirectionEye; }
    void SetSpotExponent(sal_uInt16 nNew)
        { nSpotExponent=nNew; }
    sal_uInt16 GetSpotExponent()
        { return nSpotExponent; }
    void SetSpotCutoff(double fNew) ;
    double GetSpotCutoff()
        { return fSpotCutoff; }
    double GetCosSpotCutoff()
        { return fCosSpotCutoff; }
    void SetConstantAttenuation(double fNew)
        { fConstantAttenuation=fNew; }
    double GetConstantAttenuation()
        { return fConstantAttenuation; }
    void SetLinearAttenuation(double fNew);
    double GetLinearAttenuation()
        { return fLinearAttenuation; }
    void SetQuadraticAttenuation(double fNew);
    double GetQuadraticAttenuation()
        { return fQuadraticAttenuation; }

    void Enable(sal_Bool bNew=sal_True)
        { bIsEnabled=bNew; }
    sal_Bool IsEnabled()
        { return bIsEnabled; }
    void SetFirst(sal_Bool bNew=sal_True)
        { bIsFirstLight=bNew; }
    sal_Bool IsFirst()
        { return bIsFirstLight; }
    void SetDirectionalSource(sal_Bool bNew=sal_True)
        { bIsDirectionalSource=bNew; }
    sal_Bool IsDirectionalSource()
        { return bIsDirectionalSource; }
    sal_Bool IsSpot()
        { return bIsSpot; }
    sal_Bool IsAmbient()
        { return bIsAmbient; }
    sal_Bool IsDiffuse()
        { return bIsDiffuse; }
    sal_Bool IsSpecular()
        { return bIsSpecular; }
    sal_Bool IsLinearOrQuadratic()
        { return bLinearOrQuadratic; }

    void Init();
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
    B3dLight                aLight[BASE3D_MAX_NUMBER_LIGHTS];

    // lokale Parameter des LightModels
    Color                   aGlobalAmbientLight;

    // Hauptschalter fuer die Beleuchtung
    unsigned                bLightingEnabled            : 1;
    unsigned                bLocalViewer                : 1;
    unsigned                bModelTwoSide               : 1;

public:
    // Konstruktor
    B3dLightGroup();
    virtual ~B3dLightGroup();

    // lokale Parameter des LightModels
    virtual void SetGlobalAmbientLight(const Color rNew);
    const Color GetGlobalAmbientLight();
    virtual void SetLocalViewer(sal_Bool bNew=sal_True);
    sal_Bool GetLocalViewer();
    virtual void SetModelTwoSide(sal_Bool bNew=sal_False);
    sal_Bool GetModelTwoSide();

    // Hauptschalter fuer die Beleuchtung
    virtual void EnableLighting(sal_Bool bNew=sal_True);
    sal_Bool IsLightingEnabled();

    // Lichtquellen Interface
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    const Color GetIntensity(Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    void SetPosition(const basegfx::B3DPoint& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetDirection(const basegfx::B3DVector& rNew,
        Base3DLightNumber=Base3DLight0);
    const basegfx::B3DPoint& GetPosition(Base3DLightNumber=Base3DLight0);
    const basegfx::B3DVector& GetDirection(Base3DLightNumber=Base3DLight0);
    void SetSpotDirection(const basegfx::B3DVector& rNew,
        Base3DLightNumber=Base3DLight0);
    const basegfx::B3DVector& GetSpotDirection(Base3DLightNumber=Base3DLight0);
    void SetSpotExponent(sal_uInt16 nNew,
        Base3DLightNumber=Base3DLight0);
    sal_uInt16 GetSpotExponent(Base3DLightNumber=Base3DLight0);
    void SetSpotCutoff(double fNew,
        Base3DLightNumber=Base3DLight0);
    double GetSpotCutoff(Base3DLightNumber=Base3DLight0);
    void SetConstantAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    double GetConstantAttenuation(Base3DLightNumber=Base3DLight0);
    void SetLinearAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    double GetLinearAttenuation(Base3DLightNumber=Base3DLight0);
    void SetQuadraticAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    double GetQuadraticAttenuation(Base3DLightNumber=Base3DLight0);
    void Enable(sal_Bool bNew=sal_True,
        Base3DLightNumber=Base3DLight0);
    sal_Bool IsEnabled(Base3DLightNumber=Base3DLight0);
    sal_Bool IsDirectionalSource(Base3DLightNumber=Base3DLight0);

    // Direkter Zugriff auf B3dLight
    B3dLight& GetLightObject(Base3DLightNumber=Base3DLight0);
};

#endif          // _B3D_B3DLIGHT_HXX
