/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fumorph.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:35:55 $
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

#ifndef SD_FU_MORPH_HXX
#define SD_FU_MORPH_HXX

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

#include <math.h>

/*************************************************************************
|*
\************************************************************************/
class List;
class PolyPolygon3D;
class Polygon3D;
class Vector3D;

namespace sd {

class FuMorph
    : public FuPoor
{
public:
    TYPEINFO();

    FuMorph (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuMorph (void) {}

private:
    void ImpInsertPolygons(List& rPolyPolyList3D, BOOL bAttributeFade,
        const SdrObject* pObj1, const SdrObject* pObj2);
    PolyPolygon3D* ImpCreateMorphedPolygon(
        const PolyPolygon3D& rPolyPolyStart,
        const PolyPolygon3D& rPolyPolyEnd,
        const double fMorphingFactor);
    BOOL ImpMorphPolygons(
        const PolyPolygon3D& rPolyPoly1, const PolyPolygon3D& rPolyPoly2,
        const USHORT nSteps, List& rPolyPolyList3D);
    void ImpAddPolys(PolyPolygon3D& rSmaller, const PolyPolygon3D& rBigger);
    void ImpEqualizePolyPointCount(Polygon3D& rSmall, const Polygon3D& rBig);
    sal_uInt16 ImpGetNearestIndex(const Polygon3D& rPoly, const Vector3D& rPos);
};

} // end of namespace sd

#endif
