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



#ifndef _E3D_LATHE3D_HXX
#define _E3D_LATHE3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* 3D-Rotationsobjekt aus uebergebenem 2D-Polygon erzeugen
|*
|* Das aPolyPoly3D wird in nHSegments-Schritten um die Achse rotiert.
|* nVSegments gibt die Anzahl der Linien von aPolyPoly3D an und stellt damit
|* quasi eine vertikale Segmentierung dar.
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dLatheObj : public E3dCompoundObject
{
private:
    // Partcodes fuer Wireframe-Generierung: Standard oder Deckelflaeche
    enum { LATHE_PART_STD = 1, LATHE_PART_COVER = 2 };
    basegfx::B2DPolyPolygon maPolyPoly2D;

 protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    void SetDefaultAttributes(const E3dDefaultAttributes& rDefault);

    virtual ~E3dLatheObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

 public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    E3dLatheObj(
        SdrModel& rSdrModel,
        const E3dDefaultAttributes& rDefault,
        const basegfx::B2DPolyPolygon aPoly2D = basegfx::B2DPolyPolygon());
//  E3dLatheObj();

    // HorizontalSegments:
    sal_uInt32 GetHorizontalSegments() const
        { return ((const SfxUInt32Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_uInt32 GetVerticalSegments() const
        { return ((const SfxUInt32Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const
        { return ((const SfxUInt16Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetBackScale() const
        { return ((const SfxUInt16Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_BACKSCALE)).GetValue(); }

    // EndAngle: 0..10000
    sal_uInt32 GetEndAngle() const
        { return ((const SfxUInt32Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_END_ANGLE)).GetValue(); }

    // #107245# GetSmoothNormals() for bLatheSmoothed
    sal_Bool GetSmoothNormals() const
        { return ((const Svx3DSmoothNormalsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS)).GetValue(); }

    // #107245# GetSmoothLids() for bLatheSmoothFrontBack
    sal_Bool GetSmoothLids() const
        { return ((const Svx3DSmoothLidsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS)).GetValue(); }

    // #107245# GetCharacterMode() for bLatheCharacterMode
    sal_Bool GetCharacterMode() const
        { return ((const Svx3DCharacterModeItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE)).GetValue(); }

    // #107245# GetCloseFront() for bLatheCloseFront
    sal_Bool GetCloseFront() const
        { return ((const Svx3DCloseFrontItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT)).GetValue(); }

    // #107245# GetCloseBack() for bLatheCloseBack
    sal_Bool GetCloseBack() const
        { return ((const Svx3DCloseBackItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;
    void    ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs);

    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // Lokale Parameter setzen/lesen mit Geometrieneuerzeugung
    void SetPolyPoly2D(const basegfx::B2DPolyPolygon& rNew);
    const basegfx::B2DPolyPolygon& GetPolyPoly2D() { return maPolyPoly2D; }

    // Aufbrechen
    virtual sal_Bool IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

#endif          // _E3D_LATHE3D_HXX

