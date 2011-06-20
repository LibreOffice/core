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

#ifndef SD_FU_MORPH_HXX
#define SD_FU_MORPH_HXX

#include "fupoor.hxx"

#include <math.h>

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

    void ImpInsertPolygons(List& rPolyPolyList3D, sal_Bool bAttributeFade,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
