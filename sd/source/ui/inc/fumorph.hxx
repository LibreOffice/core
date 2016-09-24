/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUMORPH_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUMORPH_HXX

#include "fupoor.hxx"

#include <math.h>
#include <vector>

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

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq ) override;

private:
    typedef ::std::vector< ::basegfx::B2DPolyPolygon* > B2DPolyPolygonList_impl;

    FuMorph (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    void ImpInsertPolygons(
        B2DPolyPolygonList_impl& rPolyPolyList3D,
        bool bAttributeFade,
        const SdrObject* pObj1,
        const SdrObject* pObj2
    );

    static ::basegfx::B2DPolyPolygon* ImpCreateMorphedPolygon(
        const ::basegfx::B2DPolyPolygon& rPolyPolyStart,
        const ::basegfx::B2DPolyPolygon& rPolyPolyEnd,
        double fMorphingFactor
    );

    static bool ImpMorphPolygons(
        const ::basegfx::B2DPolyPolygon& rPolyPoly1,
        const ::basegfx::B2DPolyPolygon& rPolyPoly2,
        const sal_uInt16 nSteps,
        B2DPolyPolygonList_impl& rPolyPolyList3D
    );

    static void ImpAddPolys(
        ::basegfx::B2DPolyPolygon& rSmaller,
        const ::basegfx::B2DPolyPolygon& rBigger
    );

    static void ImpEqualizePolyPointCount(
        ::basegfx::B2DPolygon& rSmall,
        const ::basegfx::B2DPolygon& rBig
    );

    static sal_uInt32 ImpGetNearestIndex(
        const ::basegfx::B2DPolygon& rPoly,
        const ::basegfx::B2DPoint& rPos
    );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
