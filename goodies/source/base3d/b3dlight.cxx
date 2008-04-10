/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dlight.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_goodies.hxx"
#include <goodies/b3dlight.hxx>
#include <tools/debug.hxx>

/*************************************************************************
|*
|* Konstruktor B3dLight
|*
\************************************************************************/

B3dLight::B3dLight()
{
}

/*************************************************************************
|*
|* Intensitaet einer bestimmten Lichtkomponente setzen
|*
\************************************************************************/

void B3dLight::SetIntensity(const Color rNew, Base3DMaterialValue eVal)
{
    switch(eVal)
    {
        case Base3DMaterialAmbient:
        {
            aAmbient = rNew;
            if(rNew.GetRed() || rNew.GetGreen() || rNew.GetBlue())
                bIsAmbient = sal_True;
            else
                bIsAmbient = sal_False;
            break;
        }
        case Base3DMaterialDiffuse:
        {
            aDiffuse = rNew;
            if(rNew.GetRed() || rNew.GetGreen() || rNew.GetBlue())
                bIsDiffuse = sal_True;
            else
                bIsDiffuse = sal_False;
            break;
        }
        default:
        {
            aSpecular = rNew;
            if(rNew.GetRed() || rNew.GetGreen() || rNew.GetBlue())
                bIsSpecular = sal_True;
            else
                bIsSpecular = sal_False;
            break;
        }
    }
}

/*************************************************************************
|*
|* Intensitaet einer bestimmten Lichtkomponente lesen
|*
\************************************************************************/

const Color B3dLight::GetIntensity(Base3DMaterialValue eVal)
{
    switch(eVal)
    {
        case Base3DMaterialAmbient:
        {
            return aAmbient;
        }
        case Base3DMaterialDiffuse:
        {
            return aDiffuse;
        }
        default:
        {
            return aSpecular;
        }
    }
}

/*************************************************************************
|*
|* Lichtquelle initialisieren (selbe defaults wie OpenGL)
|*
\************************************************************************/

void B3dLight::Init()
{
    aAmbient = Color(255, 0, 0, 0);
    if(IsFirst())
    {
        aDiffuse = Color(255, 204, 204, 204);
        aSpecular = Color(255, 255, 255, 255);
        basegfx::B3DVector aTiltVector(1.0, 1.0, 1.0);
        aTiltVector.normalize();
        aPosition = aTiltVector;
        bIsDiffuse = sal_True;
        bIsSpecular = sal_True;
        bIsEnabled = sal_True;
    }
    else
    {
        aDiffuse = Color(0, 0, 0, 0);
        aSpecular = Color(0, 0, 0, 0);
        aPosition = basegfx::B3DPoint(0.0, 0.0, 1.0);
        bIsDiffuse = sal_False;
        bIsSpecular = sal_False;
        bIsEnabled = sal_False;
    }
    aPositionEye = basegfx::B3DPoint(0.0, 0.0, 0.0);
    aSpotDirection = basegfx::B3DVector(0.0, 0.0, -1.0);
    aSpotDirectionEye = basegfx::B3DVector(0.0, 0.0, 0.0);
    nSpotExponent = 0;
    fSpotCutoff = 180.0;
    fConstantAttenuation = 1.0;
    fLinearAttenuation = 0.0;
    fQuadraticAttenuation = 0.0;
    bLinearOrQuadratic = sal_False;
    bIsDirectionalSource = sal_True;
    bIsSpot = sal_False;
    bIsAmbient = sal_False;
}

/*************************************************************************
|*
|* Richtung der Lichtquelle setzen
|*
\************************************************************************/

void B3dLight::SetSpotDirection(const basegfx::B3DVector& rNew)
{
    aSpotDirection=rNew;
    aSpotDirection.normalize();
}

/*************************************************************************
|*
|* Richtung der Lichtquelle in Augkoordinaten setzen
|*
\************************************************************************/

void B3dLight::SetSpotDirectionEye(const basegfx::B3DVector& rNew)
{
    aSpotDirectionEye=rNew;
    aSpotDirectionEye.normalize();
}

/*************************************************************************
|*
|* Kegel der Lichtquelle setzen
|*
\************************************************************************/

void B3dLight::SetSpotCutoff(double fNew)
{
    fSpotCutoff = fNew;
    bIsSpot = (fNew == 180.0) ? sal_False : sal_True;
    fCosSpotCutoff = cos(fNew * F_PI180);
}

/*************************************************************************
|*
|* Lineare Attenuation setzen
|*
\************************************************************************/

void B3dLight::SetLinearAttenuation(double fNew)
{
    fLinearAttenuation = fNew;
    bLinearOrQuadratic =
        (fNew + fQuadraticAttenuation == 0.0) ? sal_False : sal_True;
}

/*************************************************************************
|*
|* Quadratische Attenuation setzen
|*
\************************************************************************/

void B3dLight::SetQuadraticAttenuation(double fNew)
{
    fQuadraticAttenuation = fNew;
    bLinearOrQuadratic =
        (fNew + fLinearAttenuation == 0.0) ? sal_False : sal_True;
}

/*************************************************************************
|*
|* Gruppe von Lichtquellen, Konstruktor
|*
\************************************************************************/

B3dLightGroup::B3dLightGroup()
:   aGlobalAmbientLight(255, 102, 102, 102),
    bLightingEnabled(sal_True),
    bLocalViewer(sal_True),
    bModelTwoSide(sal_False)
{
    // Lichtquellen initialisieren
    for(sal_uInt16 i=0; i < BASE3D_MAX_NUMBER_LIGHTS;i++)
    {
        aLight[i].SetFirst(i==0);
        aLight[i].Init();
    }
}

B3dLightGroup::~B3dLightGroup()
{
}

/*************************************************************************
|*
|* globales Umgebungslicht setzen
|*
\************************************************************************/

void B3dLightGroup::SetGlobalAmbientLight(const Color rNew)
{
    if(aGlobalAmbientLight != rNew)
    {
        aGlobalAmbientLight = rNew;
    }
}

/*************************************************************************
|*
|* globales Umgebungslicht lesen
|*
\************************************************************************/

const Color B3dLightGroup::GetGlobalAmbientLight()
{
    return aGlobalAmbientLight;
}

/*************************************************************************
|*
|* Modus globaler Viewer bei Berechnung specular reflection setzen
|*
\************************************************************************/

void B3dLightGroup::SetLocalViewer(sal_Bool bNew)
{
    if(bLocalViewer != bNew)
    {
        bLocalViewer = bNew;
    }
}

/*************************************************************************
|*
|* Modus globaler Viewer bei Berechnung specular reflection lesen
|*
\************************************************************************/

sal_Bool B3dLightGroup::GetLocalViewer()
{
    return bLocalViewer;
}

/*************************************************************************
|*
|* Modus Beleuchtungsmodell beidseitig anwenden setzen
|*
\************************************************************************/

void B3dLightGroup::SetModelTwoSide(sal_Bool bNew)
{
    if(bModelTwoSide != bNew)
    {
        bModelTwoSide = bNew;
    }
}

/*************************************************************************
|*
|* Modus Beleuchtungsmodell beidseitig anwenden lesen
|*
\************************************************************************/

sal_Bool B3dLightGroup::GetModelTwoSide()
{
    return bModelTwoSide;
}

/*************************************************************************
|*
|* Beleuchtungsmodell aktivieren/deaktivieren
|*
\************************************************************************/

void B3dLightGroup::EnableLighting(sal_Bool bNew)
{
    if(bLightingEnabled != bNew)
    {
        bLightingEnabled = bNew;
    }
}

/*************************************************************************
|*
|* Abfrage, ob das Beleuchtungsmodell aktiviert/deaktiviert ist
|*
\************************************************************************/

sal_Bool B3dLightGroup::IsLightingEnabled()
{
    return bLightingEnabled;
}

/*************************************************************************
|*
|* Die Intensitaet eines bestimmten Aspekts einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetIntensity(const Color rNew,
    Base3DMaterialValue eMat, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetIntensity(rNew, eMat);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Intensitaet eines bestimmten Aspekts einer Lichtquelle lesen
|*
\************************************************************************/

const Color B3dLightGroup::GetIntensity(Base3DMaterialValue eMat,
    Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetIntensity(eMat);
}

/*************************************************************************
|*
|* Die Position einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetPosition(const basegfx::B3DPoint& rNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetPosition(rNew);
        aLight[eNum].SetDirectionalSource(sal_False);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Position einer Lichtquelle lesen
|*
\************************************************************************/

const basegfx::B3DPoint& B3dLightGroup::GetPosition(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
#ifdef DBG_UTIL
    if(IsDirectionalSource())
        DBG_ERROR("Zugriff auf die Position einer gerichteten Lichtquelle!");
#endif
    return aLight[eNum].GetPosition();
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetDirection(const basegfx::B3DVector& rNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetPosition(rNew);
        aLight[eNum].SetDirectionalSource(sal_True);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle lesen
|*
\************************************************************************/

const basegfx::B3DVector& B3dLightGroup::GetDirection(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
#ifdef DBG_UTIL
    if(!IsDirectionalSource())
        DBG_ERROR("Zugriff auf die Richtung einer ungerichteten Lichtquelle!");
#endif
    return (basegfx::B3DVector&)aLight[eNum].GetPosition();
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetSpotDirection(const basegfx::B3DVector& rNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetSpotDirection(rNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle lesen
|*
\************************************************************************/

const basegfx::B3DVector& B3dLightGroup::GetSpotDirection(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetSpotDirection();
}

/*************************************************************************
|*
|* Den SpotExponent einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetSpotExponent(sal_uInt16 nNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetSpotExponent(nNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Den SpotExponent einer Lichtquelle lesen
|*
\************************************************************************/

sal_uInt16 B3dLightGroup::GetSpotExponent(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetSpotExponent();
}

/*************************************************************************
|*
|* Die Einengung des Lichtkegels einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetSpotCutoff(double fNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetSpotCutoff(fNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Einengung des Lichtkegels einer Lichtquelle lesen
|*
\************************************************************************/

double B3dLightGroup::GetSpotCutoff(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetSpotCutoff();
}

/*************************************************************************
|*
|* Den konstanten AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetConstantAttenuation(double fNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetConstantAttenuation(fNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Den konstanten AttenuationFactor einer Lichtquelle lesen
|*
\************************************************************************/

double B3dLightGroup::GetConstantAttenuation(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetConstantAttenuation();
}

/*************************************************************************
|*
|* Den linearen AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetLinearAttenuation(double fNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetLinearAttenuation(fNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Den linearen AttenuationFactor einer Lichtquelle lesen
|*
\************************************************************************/

double B3dLightGroup::GetLinearAttenuation(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetLinearAttenuation();
}

/*************************************************************************
|*
|* Den quadratischen AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetQuadraticAttenuation(double fNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetQuadraticAttenuation(fNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Den quadratischen AttenuationFactor einer Lichtquelle lesen
|*
\************************************************************************/

double B3dLightGroup::GetQuadraticAttenuation(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetQuadraticAttenuation();
}

/*************************************************************************
|*
|* Eine Lichtquelle aktivieren/deaktivieren
|*
\************************************************************************/

void B3dLightGroup::Enable(sal_Bool bNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].Enable(bNew);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Abfrage, ob eine Lichtquelle aktiviert/deaktiviert ist
|*
\************************************************************************/

sal_Bool B3dLightGroup::IsEnabled(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].IsEnabled();
}

/*************************************************************************
|*
|* Abfrage, ob eine Lichtquelle als directional source eingerichtet ist
|*
\************************************************************************/

sal_Bool B3dLightGroup::IsDirectionalSource(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].IsDirectionalSource();
}

/*************************************************************************
|*
|* Direkter Zugriff auf B3dLight fuer abgeleitete Klassen
|*
\************************************************************************/

B3dLight& B3dLightGroup::GetLightObject(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum];
}

// eof
