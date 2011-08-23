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

#include "b3dlight.hxx"

#include <tools/debug.hxx>

namespace binfilter {

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
                bIsAmbient = TRUE;
            else
                bIsAmbient = FALSE;
            break;
        }
        case Base3DMaterialDiffuse:
        {
            aDiffuse = rNew;
            if(rNew.GetRed() || rNew.GetGreen() || rNew.GetBlue())
                bIsDiffuse = TRUE;
            else
                bIsDiffuse = FALSE;
            break;
        }
        default:
        {
            aSpecular = rNew;
            if(rNew.GetRed() || rNew.GetGreen() || rNew.GetBlue())
                bIsSpecular = TRUE;
            else
                bIsSpecular = FALSE;
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
            break;
        }
        case Base3DMaterialDiffuse:
        {
            return aDiffuse;
            break;
        }
        default:
        {
            return aSpecular;
            break;
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
        aPosition = Vector3D(1.0, 1.0, 1.0);
        aPosition.Normalize();
        bIsDiffuse = TRUE;
        bIsSpecular = TRUE;
        bIsEnabled = TRUE;
    }
    else
    {
        aDiffuse = Color(0, 0, 0, 0);
        aSpecular = Color(0, 0, 0, 0);
        aPosition = Vector3D(0.0, 0.0, 1.0);
        bIsDiffuse = FALSE;
        bIsSpecular = FALSE;
        bIsEnabled = FALSE;
    }
    aPositionEye = Vector3D(0.0, 0.0, 0.0);
    aSpotDirection = Vector3D(0.0, 0.0, -1.0);
    aSpotDirectionEye = Vector3D(0.0, 0.0, 0.0);
    nSpotExponent = 0;
    fSpotCutoff = 180.0;
    fConstantAttenuation = 1.0;
    fLinearAttenuation = 0.0;
    fQuadraticAttenuation = 0.0;
    bLinearOrQuadratic = FALSE;
    bIsDirectionalSource = TRUE;
    bIsSpot = FALSE;
    bIsAmbient = FALSE;
}

void B3dLight::WriteData(SvStream& rOut) const
{
    rOut << aAmbient;
    rOut << aDiffuse;
    rOut << aSpecular;

    rOut << aPosition;
    rOut << aPositionEye;
    rOut << aSpotDirection;
    rOut << aSpotDirectionEye;
    rOut << nSpotExponent;

    rOut << fSpotCutoff;
    rOut << fCosSpotCutoff;
    rOut << fConstantAttenuation;
    rOut << fLinearAttenuation;
    rOut << fQuadraticAttenuation;

    rOut << (BOOL)bIsFirstLight;
    rOut << (BOOL)bIsEnabled;
    rOut << (BOOL)bIsDirectionalSource;
    rOut << (BOOL)bIsSpot;
    rOut << (BOOL)bIsAmbient;
    rOut << (BOOL)bIsDiffuse;
    rOut << (BOOL)bIsSpecular;
    rOut << (BOOL)bLinearOrQuadratic;
}

void B3dLight::ReadData(SvStream& rIn)
{
    BOOL bTmp;

    rIn >> aAmbient;
    rIn >> aDiffuse;
    rIn >> aSpecular;

    rIn >> aPosition;
    rIn >> aPositionEye;
    rIn >> aSpotDirection;
    rIn >> aSpotDirectionEye;
    rIn >> nSpotExponent;

    rIn >> fSpotCutoff;
    rIn >> fCosSpotCutoff;
    rIn >> fConstantAttenuation;
    rIn >> fLinearAttenuation;
    rIn >> fQuadraticAttenuation;

    rIn >> bTmp; bIsFirstLight = bTmp;
    rIn >> bTmp; bIsEnabled = bTmp;
    rIn >> bTmp; bIsDirectionalSource = bTmp;
    rIn >> bTmp; bIsSpot = bTmp;
    rIn >> bTmp; bIsAmbient = bTmp;
    rIn >> bTmp; bIsDiffuse = bTmp;
    rIn >> bTmp; bIsSpecular = bTmp;
    rIn >> bTmp; bLinearOrQuadratic = bTmp;
}

/*************************************************************************
|*
|* Gruppe von Lichtquellen, Konstruktor
|*
\************************************************************************/

B3dLightGroup::B3dLightGroup()
:	aGlobalAmbientLight(255, 102, 102, 102),
    bLightingEnabled(TRUE),
    bLocalViewer(TRUE),
    bModelTwoSide(FALSE)
{
    // Lichtquellen initialisieren
    for(UINT16 i=0; i < BASE3D_MAX_NUMBER_LIGHTS;i++)
    {
        aLight[i].SetFirst(i==0);
        aLight[i].Init();
    }
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

void B3dLightGroup::SetLocalViewer(BOOL bNew)
{
    if(bLocalViewer != bNew)
    {
        bLocalViewer = bNew;
    }
}

/*************************************************************************
|*
|* Modus Beleuchtungsmodell beidseitig anwenden setzen
|*
\************************************************************************/

void B3dLightGroup::SetModelTwoSide(BOOL bNew)
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

BOOL B3dLightGroup::GetModelTwoSide()
{
    return bModelTwoSide;
}

/*************************************************************************
|*
|* Beleuchtungsmodell aktivieren/deaktivieren
|*
\************************************************************************/

void B3dLightGroup::EnableLighting(BOOL bNew)
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

BOOL B3dLightGroup::IsLightingEnabled()
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

void B3dLightGroup::SetPosition(const Vector3D& rNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetPosition(rNew);
        aLight[eNum].SetDirectionalSource(FALSE);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Access to Light out of range");
#endif
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetDirection(const Vector3D& rNew, Base3DLightNumber eNum)
{
    if(eNum >= Base3DLight0 && eNum <= Base3DLight7)
    {
        aLight[eNum].SetPosition(rNew);
        aLight[eNum].SetDirectionalSource(TRUE);
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

const Vector3D& B3dLightGroup::GetDirection(Base3DLightNumber eNum)
{
    if(eNum < Base3DLight0 || eNum > Base3DLight7)
    {
        eNum = Base3DLight0;
#ifdef DBG_UTIL
        DBG_ERROR("Access to Light out of range");
#endif
    }
    return aLight[eNum].GetPosition();
}

/*************************************************************************
|*
|* Eine Lichtquelle aktivieren/deaktivieren
|*
\************************************************************************/

void B3dLightGroup::Enable(BOOL bNew, Base3DLightNumber eNum)
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

BOOL B3dLightGroup::IsEnabled(Base3DLightNumber eNum)
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

void B3dLightGroup::WriteData(SvStream& rOut) const
{
    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        B3dLight& rLight = ((B3dLightGroup*)(this))->GetLightObject((Base3DLightNumber)(Base3DLight0 + a));
        rLight.WriteData(rOut);
    }

    rOut << aGlobalAmbientLight;

    rOut << (BOOL)bLightingEnabled;
    rOut << (BOOL)bLocalViewer;
    rOut << (BOOL)bModelTwoSide;
}

void B3dLightGroup::ReadData(SvStream& rIn)
{
    BOOL bTmp;

    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        B3dLight& rLight = GetLightObject((Base3DLightNumber)(Base3DLight0 + a));
        rLight.ReadData(rIn);
    }

    rIn >> aGlobalAmbientLight;

    rIn >> bTmp; bLightingEnabled = bTmp;
    rIn >> bTmp; bLocalViewer = bTmp;
    rIn >> bTmp; bModelTwoSide = bTmp;
}

}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
