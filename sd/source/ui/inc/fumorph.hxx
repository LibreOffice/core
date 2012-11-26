/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_FU_MORPH_HXX
#define SD_FU_MORPH_HXX

#include "fupoor.hxx"

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
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

private:
    FuMorph (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    void ImpInsertPolygons(List& rPolyPolyList3D, bool bAttributeFade,
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
