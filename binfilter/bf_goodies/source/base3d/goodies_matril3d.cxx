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

#include "matril3d.hxx"

#include <tools/debug.hxx>

namespace binfilter {

/*************************************************************************
|*
|* Konstruktor B3dMaterial
|*
\************************************************************************/

B3dMaterial::B3dMaterial()
:	aAmbient(COL_BLACK),			// kein lokales Umgebungslicht
    aDiffuse(0x00, 0xb8, 0xff),		// Blau7
    aSpecular(COL_WHITE),			// Weisser Glanzpunkt
    aEmission(COL_BLACK),			// Keine Selbstleuchtfarbe
    nExponent(15)					// Glanzpunktbuendelung
{
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

}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
