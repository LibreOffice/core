/*************************************************************************
 *
 *  $RCSfile: b3dlight.cxx,v $
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

#ifndef _B3D_B3DLIGHT_HXX
#include "b3dlight.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

/*************************************************************************
|*
|* Richtung der Lichtquelle setzen
|*
\************************************************************************/

void B3dLight::SetSpotDirection(const Vector3D& rNew)
{
    aSpotDirection=rNew;
    aSpotDirection.Normalize();
}

/*************************************************************************
|*
|* Richtung der Lichtquelle in Augkoordinaten setzen
|*
\************************************************************************/

void B3dLight::SetSpotDirectionEye(const Vector3D& rNew)
{
    aSpotDirectionEye=rNew;
    aSpotDirectionEye.Normalize();
}

/*************************************************************************
|*
|* Kegel der Lichtquelle setzen
|*
\************************************************************************/

void B3dLight::SetSpotCutoff(double fNew)
{
    fSpotCutoff = fNew;
    bIsSpot = (fNew == 180.0) ? FALSE : TRUE;
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
        (fNew + fQuadraticAttenuation == 0.0) ? FALSE : TRUE;
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
        (fNew + fLinearAttenuation == 0.0) ? FALSE : TRUE;
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
:   aGlobalAmbientLight(255, 102, 102, 102),
    bLocalViewer(TRUE),
    bModelTwoSide(FALSE),
    bLightingEnabled(TRUE)
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
|* Modus globaler Viewer bei Berechnung specular reflection lesen
|*
\************************************************************************/

BOOL B3dLightGroup::GetLocalViewer()
{
    return bLocalViewer;
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
|* Die Position einer Lichtquelle lesen
|*
\************************************************************************/

const Vector3D& B3dLightGroup::GetPosition(Base3DLightNumber eNum)
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
#ifdef DBG_UTIL
    if(!IsDirectionalSource())
        DBG_ERROR("Zugriff auf die Richtung einer ungerichteten Lichtquelle!");
#endif
    return aLight[eNum].GetPosition();
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void B3dLightGroup::SetSpotDirection(const Vector3D& rNew, Base3DLightNumber eNum)
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

const Vector3D& B3dLightGroup::GetSpotDirection(Base3DLightNumber eNum)
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

void B3dLightGroup::SetSpotExponent(UINT16 nNew, Base3DLightNumber eNum)
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

UINT16 B3dLightGroup::GetSpotExponent(Base3DLightNumber eNum)
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
|* Abfrage, ob eine Lichtquelle als directional source eingerichtet ist
|*
\************************************************************************/

BOOL B3dLightGroup::IsDirectionalSource(Base3DLightNumber eNum)
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

