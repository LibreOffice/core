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

#ifndef _VOLUME3D_HXX
#define _VOLUME3D_HXX

#include <bf_goodies/b3dvolum.hxx>
namespace binfilter {
class Matrix4D;

//************************************************************
//   Vorausdeklarationen
//************************************************************

class Polygon3D;
class XPolygon;
class XPolyPolygon;
class Vol3DPointIterator;

/*************************************************************************
|*
|* dreidimensionales Volumen, symmetrisch zu den Koordinatenachsen
|*
\************************************************************************/

class Volume3D : public B3dVolume
{
public:
    Volume3D(const Vector3D& rPos, const Vector3D& r3DSize, BOOL bPosIsCenter = TRUE);
    Volume3D();

    // Zuweisungsoperator, um B3dVolumes an Volume3D zuweisen zu koennen
    void operator=(const B3dVolume& rVol)
    {
        aMinVec = rVol.MinVec();
        aMaxVec = rVol.MaxVec();
    }
    Volume3D	GetTransformVolume(const Matrix4D& rTfMatrix) const;
};

/*************************************************************************
|*
|* Iterator, der die Eckpunkte eines Volumens berechnet; wenn eine Matrix
|* uebergeben wird, werden die Punkte damit transformiert
|*
\************************************************************************/

class Vol3DPointIterator
{
protected:
    const Volume3D&				rVolume;
    const Matrix4D*				pTransform;
    Vector3D					a3DExtent;
    UINT16						nIndex;

public:
    Vol3DPointIterator(const Volume3D& rVol, const Matrix4D* pTf = NULL);

    BOOL Next(Vector3D& rVec);
    void Reset() { nIndex = 0; }
};

}//end of namespace binfilter
#endif			// _VOLUME3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
