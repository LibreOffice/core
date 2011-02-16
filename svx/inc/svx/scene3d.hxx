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

#ifndef _E3D_SCENE3D_HXX
#define _E3D_SCENE3D_HXX

#include <svx/camera3d.hxx>
#include <tools/b3dtrans.hxx>
#include <tools/time.hxx>
#include <svx/svxdllapi.h>
#include <svx/obj3d.hxx>

namespace sdr { namespace properties {
    class BaseProperties;
    class E3dSceneProperties;
}}

namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}

/*************************************************************************
|*
|* GeoData relevant fuer Undo-Actions
|*
\************************************************************************/

class E3DSceneGeoData : public E3DObjGeoData
{
public:
    Camera3D                    aCamera;

    E3DSceneGeoData() {}
};

// #110988#
class Imp3DDepthRemapper;

/*************************************************************************
|*
|* Basisklasse fuer 3D-Szenen
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dScene : public E3dObject
{
private:
    // to allow sdr::properties::E3dSceneProperties access to StructureChanged()
    friend class sdr::properties::E3dSceneProperties;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // Transformationen
    B3dCamera                   aCameraSet;
    Camera3D                    aCamera;

    // #110988#
    Imp3DDepthRemapper*         mp3DDepthRemapper;

    // Flag to determine if only selected objects should be drawn
    unsigned                    bDrawOnlySelected       : 1;

    virtual void NewObjectInserted(const E3dObject* p3DObj);
    virtual void StructureChanged();

    void RebuildLists();

    virtual void Notify(SfxBroadcaster &rBC, const SfxHint  &rHint);

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    // #110988#
    void ImpCleanup3DDepthMapper();

public:
    TYPEINFO();
    E3dScene();
    E3dScene(E3dDefaultAttributes& rDefault);
    virtual ~E3dScene();

    virtual void SetBoundRectDirty();

    // access to cleanup of depth mapper
    void Cleanup3DDepthMapper() { ImpCleanup3DDepthMapper(); }

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // #110988#
    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;

    // Perspective: enum ProjectionType { PR_PARALLEL, PR_PERSPECTIVE }
    ProjectionType GetPerspective() const
        { return (ProjectionType)((const Svx3DPerspectiveItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue(); }

    // Distance:
    double GetDistance() const
        { return (double)((const Svx3DDistanceItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_DISTANCE)).GetValue(); }

    // Focal length: before cm, now 1/10th mm (*100)
    double GetFocalLength() const
        { return ((const Svx3DFocalLengthItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue(); }

    // Two sided lighting:
    sal_Bool GetTwoSidedLighting() const
        { return ((const Svx3DTwoSidedLightingItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue(); }

    // Lightcolor:
    Color GetLightColor1() const
        { return ((const Svx3DLightcolor1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue(); }
    Color GetLightColor2() const
        { return ((const Svx3DLightcolor2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue(); }
    Color GetLightColor3() const
        { return ((const Svx3DLightcolor3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue(); }
    Color GetLightColor4() const
        { return ((const Svx3DLightcolor4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue(); }
    Color GetLightColor5() const
        { return ((const Svx3DLightcolor5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue(); }
    Color GetLightColor6() const
        { return ((const Svx3DLightcolor6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue(); }
    Color GetLightColor7() const
        { return ((const Svx3DLightcolor7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue(); }
    Color GetLightColor8() const
        { return ((const Svx3DLightcolor8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue(); }

    // Ambient color:
    Color GetGlobalAmbientColor() const
        { return ((const Svx3DAmbientcolorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue(); }

    // Light on/off:
    sal_Bool GetLightOnOff1() const
        { return ((const Svx3DLightOnOff1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue(); }
    sal_Bool GetLightOnOff2() const
        { return ((const Svx3DLightOnOff2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue(); }
    sal_Bool GetLightOnOff3() const
        { return ((const Svx3DLightOnOff3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue(); }
    sal_Bool GetLightOnOff4() const
        { return ((const Svx3DLightOnOff4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue(); }
    sal_Bool GetLightOnOff5() const
        { return ((const Svx3DLightOnOff5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue(); }
    sal_Bool GetLightOnOff6() const
        { return ((const Svx3DLightOnOff6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue(); }
    sal_Bool GetLightOnOff7() const
        { return ((const Svx3DLightOnOff7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue(); }
    sal_Bool GetLightOnOff8() const
        { return ((const Svx3DLightOnOff8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue(); }

    // Light direction:
    basegfx::B3DVector GetLightDirection1() const
        { return ((const Svx3DLightDirection1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue(); }
    basegfx::B3DVector GetLightDirection2() const
        { return ((const Svx3DLightDirection2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue(); }
    basegfx::B3DVector GetLightDirection3() const
        { return ((const Svx3DLightDirection3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue(); }
    basegfx::B3DVector GetLightDirection4() const
        { return ((const Svx3DLightDirection4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue(); }
    basegfx::B3DVector GetLightDirection5() const
        { return ((const Svx3DLightDirection5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue(); }
    basegfx::B3DVector GetLightDirection6() const
        { return ((const Svx3DLightDirection6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue(); }
    basegfx::B3DVector GetLightDirection7() const
        { return ((const Svx3DLightDirection7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue(); }
    basegfx::B3DVector GetLightDirection8() const
        { return ((const Svx3DLightDirection8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue(); }

    // ShadowSlant:
    sal_uInt16 GetShadowSlant() const
        { return ((const Svx3DShadowSlantItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue(); }

    // ShadeMode: 0 == FLAT, 1 == PHONG, 2 == SMOOTH, 3 == ForceDraft
    sal_uInt16 GetShadeMode() const
        { return ((const Svx3DShadeModeItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue(); }

    // set flag to draw only selected
    void SetDrawOnlySelected(sal_Bool bNew) { bDrawOnlySelected = bNew; }
    bool GetDrawOnlySelected() const { return bDrawOnlySelected; }
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual void    NbcSetSnapRect(const Rectangle& rRect);
    virtual void    NbcMove(const Size& rSize);
    virtual void    NbcResize(const Point& rRef, const Fraction& rXFact,
                                                 const Fraction& rYFact);
    virtual void    RecalcSnapRect();

    virtual E3dScene* GetScene() const;
    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }
    void removeAllNonSelectedObjects();

    virtual void operator=(const SdrObject&);

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix);
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);

    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    void RotateScene(const Point& rRef, long nWink, double sn, double cs);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // Transformationen rausgeben
    B3dCamera& GetCameraSet() { return aCameraSet; }

    // Aufbrechen
    virtual sal_Bool IsBreakObjPossible();

    basegfx::B3DVector GetShadowPlaneDirection() const;
    void SetShadowPlaneDirection(const basegfx::B3DVector& rVec);

    // Polygon das waehrend des Erzeugens aufgezogen wird
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    // create moves
    virtual FASTBOOL BegCreate(SdrDragStat& rStat);
    virtual FASTBOOL MovCreate(SdrDragStat& rStat); // sal_True=Xor muss repainted werden
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual FASTBOOL BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
};

#endif          // _E3D_SCENE3D_HXX
