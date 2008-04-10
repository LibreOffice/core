/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: matril3d.cxx,v $
 * $Revision: 1.6 $
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
#include <goodies/matril3d.hxx>
#include <tools/debug.hxx>

/*************************************************************************
|*
|* Konstruktor B3dMaterial
|*
\************************************************************************/

B3dMaterial::B3dMaterial()
:   aAmbient(COL_BLACK),            // kein lokales Umgebungslicht
    aDiffuse(0x00, 0xb8, 0xff),     // Blau7
    aSpecular(COL_WHITE),           // Weisser Glanzpunkt
    aEmission(COL_BLACK),           // Keine Selbstleuchtfarbe
    nExponent(15)                   // Glanzpunktbuendelung
{
}

/*************************************************************************
|*
|* Materialeigenschaft setzen
|*
\************************************************************************/

void B3dMaterial::SetMaterial(Color rNew, Base3DMaterialValue eVal)
{
    switch(eVal)
    {
        case Base3DMaterialAmbient:
            aAmbient = rNew;
            break;
        case Base3DMaterialDiffuse:
            aDiffuse = rNew;
            break;
        case Base3DMaterialSpecular:
            aSpecular = rNew;
            break;
        case Base3DMaterialEmission:
            aEmission = rNew;
            break;
    }
}

/*************************************************************************
|*
|* Materialeigenschaft abfragen
|*
\************************************************************************/

Color B3dMaterial::GetMaterial(Base3DMaterialValue eVal) const
{
    if(eVal == Base3DMaterialAmbient)
        return aAmbient;
    if(eVal == Base3DMaterialDiffuse)
        return aDiffuse;
    if(eVal == Base3DMaterialEmission)
        return aEmission;
    return aSpecular;
}

/*************************************************************************
|*
|* Materialeigenschaften setzen, exponent der specular-Eigenschaft
|*
\************************************************************************/

void B3dMaterial::SetShininess(UINT16 nNew)
{
    nExponent = nNew;
}

/*************************************************************************
|*
|* Materialeigenschaften abfragen, exponent der specular-Eigenschaft
|*
\************************************************************************/

UINT16 B3dMaterial::GetShininess() const
{
    return nExponent;
}

/*************************************************************************
|*
|* Vergleichsoperator
|*
\************************************************************************/

BOOL B3dMaterial::operator==(const B3dMaterial& rMat)
{
     if(aAmbient == rMat.aAmbient
         && aDiffuse == rMat.aDiffuse
         && aSpecular == rMat.aSpecular
         && aEmission == rMat.aEmission
         && nExponent == rMat.nExponent)
         return TRUE;
     return FALSE;
}

/*************************************************************************
|*
|* Bucket fuer geometrische Daten
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(B3dMaterial, Bucket)

