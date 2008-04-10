/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: volume3d.hxx,v $
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

#ifndef _VOLUME3D_HXX
#define _VOLUME3D_HXX

#include <basegfx/range/b3drange.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************
namespace basegfx
{
    class B3DPolygon;
    class B3DPoint;
    class B3DHomMatrix;
}

/*************************************************************************
|*
|* dreidimensionales Volumen, symmetrisch zu den Koordinatenachsen
|*
\************************************************************************/

class SVX_DLLPUBLIC Volume3D : public basegfx::B3DRange
{
public:
    Volume3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& r3DSize, bool bPosIsCenter = true);
    explicit Volume3D(const basegfx::B3DRange& rVol);
    Volume3D();

    Volume3D GetTransformVolume(const basegfx::B3DHomMatrix& rTfMatrix) const;
    void CreateWireframe(basegfx::B3DPolygon& rPoly3D, const basegfx::B3DHomMatrix* pTf = 0L) const;
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
    const basegfx::B3DRange&            rVolume;
    const basegfx::B3DHomMatrix*        pTransform;
    basegfx::B3DVector              a3DExtent;
    sal_uInt16                          nIndex;

public:
    Vol3DPointIterator(const basegfx::B3DRange& rVol, const basegfx::B3DHomMatrix* pTf = NULL);

    bool Next(basegfx::B3DPoint& rVec);
    void Reset() { nIndex = 0; }
};


#endif          // _VOLUME3D_HXX
