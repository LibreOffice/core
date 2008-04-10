/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: matril3d.hxx,v $
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

#ifndef _B3D_MATRIL3D_HXX
#define _B3D_MATRIL3D_HXX

#include <goodies/bucket.hxx>
#include <tools/color.hxx>
#include <tools/stream.hxx>

/*************************************************************************
|*
|* Moegliche MaterialModes fuer Polygone
|*
\************************************************************************/

enum Base3DMaterialMode
{
    Base3DMaterialFront = 0,
    Base3DMaterialBack,
    Base3DMaterialFrontAndBack
};

/*************************************************************************
|*
|* Moegliche MaterialValues fuer Polygone
|*
\************************************************************************/

enum Base3DMaterialValue
{
    Base3DMaterialAmbient = 0,
    Base3DMaterialDiffuse,
    Base3DMaterialSpecular,
    Base3DMaterialEmission
};

/*************************************************************************
|*
|* Basisklasse fuer Materialparameter
|*
\************************************************************************/

class B3dMaterial
{
private:
    Color                   aAmbient;
    Color                   aDiffuse;
    Color                   aSpecular;
    Color                   aEmission;
    UINT16                  nExponent;

public:
    B3dMaterial();

    // Zugriffsfunktionen
    void SetMaterial(Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient);
    Color GetMaterial(Base3DMaterialValue=Base3DMaterialAmbient) const;
    void SetShininess(UINT16 nNew);
    UINT16 GetShininess() const;

    // Vergleichsoperator
    BOOL operator==(const B3dMaterial&);
    BOOL operator!=(const B3dMaterial& rMat) { return (!((*this) == rMat)); }
protected:
};

/*************************************************************************
|*
|* Bucket fuer Materialeigenschaften
|*
\************************************************************************/

BASE3D_DECL_BUCKET(B3dMaterial, Bucket)

#endif          // _B3D_MATRIL3D_HXX
