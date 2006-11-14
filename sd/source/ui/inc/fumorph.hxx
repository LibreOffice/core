/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fumorph.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:33:57 $
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
namespace basegfx {
    class B2DPolyPolygon;
    class B2DPolygon;
    class B2DPoint;
}

namespace sd {

class FuMorph
    : public FuPoor
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

private:
    FuMorph (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    void ImpInsertPolygons(List& rPolyPolyList3D, BOOL bAttributeFade,
        const SdrObject* pObj1, const SdrObject* pObj2);
    ::basegfx::B2DPolyPolygon* ImpCreateMorphedPolygon(
        const ::basegfx::B2DPolyPolygon& rPolyPolyStart,
        const ::basegfx::B2DPolyPolygon& rPolyPolyEnd,
        double fMorphingFactor);
    sal_Bool ImpMorphPolygons(
        const ::basegfx::B2DPolyPolygon& rPolyPoly1, const ::basegfx::B2DPolyPolygon& rPolyPoly2,
        const sal_uInt16 nSteps, List& rPolyPolyList3D);
    void ImpAddPolys(::basegfx::B2DPolyPolygon& rSmaller, const ::basegfx::B2DPolyPolygon& rBigger);
    void ImpEqualizePolyPointCount(::basegfx::B2DPolygon& rSmall, const ::basegfx::B2DPolygon& rBig);
    sal_uInt32 ImpGetNearestIndex(const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DPoint& rPos);
};

} // end of namespace sd

#endif
