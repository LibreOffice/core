/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: volume3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:47:47 $
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

#ifndef _VOLUME3D_HXX
#define _VOLUME3D_HXX

#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
