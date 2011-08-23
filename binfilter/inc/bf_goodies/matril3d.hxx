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

#ifndef _B3D_MATRIL3D_HXX
#define _B3D_MATRIL3D_HXX

#include "bucket.hxx"

#include <tools/color.hxx>

#include <tools/stream.hxx>

namespace binfilter {

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
    Color					aAmbient;
    Color					aDiffuse;
    Color					aSpecular;
    Color					aEmission;
    UINT16					nExponent;

public:
    B3dMaterial();

    // Laden/Speichern in StarView
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(SvStream& rIn);

    // Vergleichsoperator
    BOOL operator==(const B3dMaterial&);
    BOOL operator!=(const B3dMaterial& rMat) { return (!((*this) == rMat)); }
protected:
};

}//end of namespace binfilter

#endif          // _B3D_MATRIL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
