/*************************************************************************
 *
 *  $RCSfile: matril3d.cxx,v $
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

void B3dMaterial::WriteData(SvStream& rOut) const
{
    rOut << aAmbient;
    rOut << aDiffuse;
    rOut << aSpecular;
    rOut << aEmission;
    rOut << nExponent;
}

void B3dMaterial::ReadData(SvStream& rIn)
{
    rIn >> aAmbient;
    rIn >> aDiffuse;
    rIn >> aSpecular;
    rIn >> aEmission;
    rIn >> nExponent;
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

