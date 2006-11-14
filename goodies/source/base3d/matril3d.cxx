/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: matril3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 16:09:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"

#ifndef _B3D_MATRIL3D_HXX
#include "matril3d.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

