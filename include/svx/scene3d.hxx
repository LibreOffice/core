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

#ifndef INCLUDED_SVX_SCENE3D_HXX
#define INCLUDED_SVX_SCENE3D_HXX

#include <svx/camera3d.hxx>
#include <tools/b3dtrans.hxx>
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
|* GeoData relevant for undo actions
|*
\************************************************************************/

class E3DSceneGeoData : public E3DObjGeoData
{
public:
    Camera3D                    aCamera;

    E3DSceneGeoData() {}
};

class Imp3DDepthRemapper;

/*************************************************************************
|*
|* base class for 3D scenes
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dScene : public E3dObject
{
private:
    // to allow sdr::properties::E3dSceneProperties access to StructureChanged()
    friend class sdr::properties::E3dSceneProperties;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    // transformations
    B3dCamera                   aCameraSet;
    Camera3D                    aCamera;

    Imp3DDepthRemapper*         mp3DDepthRemapper;

    // Flag to determine if only selected objects should be drawn
    bool                        bDrawOnlySelected       : 1;

    virtual void NewObjectInserted(const E3dObject* p3DObj) SAL_OVERRIDE;
    virtual void StructureChanged() SAL_OVERRIDE;

    void RebuildLists();

    virtual void Notify(SfxBroadcaster &rBC, const SfxHint  &rHint) SAL_OVERRIDE;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    void ImpCleanup3DDepthMapper();

public:
    TYPEINFO_OVERRIDE();
    E3dScene();
    E3dScene(E3dDefaultAttributes& rDefault);
    virtual ~E3dScene();

    virtual void SetBoundRectDirty() SAL_OVERRIDE;

    // access to cleanup of depth mapper
    void Cleanup3DDepthMapper() { ImpCleanup3DDepthMapper(); }

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;

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
    bool GetTwoSidedLighting() const
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
    bool GetLightOnOff1() const
        { return ((const Svx3DLightOnOff1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue(); }
    bool GetLightOnOff2() const
        { return ((const Svx3DLightOnOff2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue(); }
    bool GetLightOnOff3() const
        { return ((const Svx3DLightOnOff3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue(); }
    bool GetLightOnOff4() const
        { return ((const Svx3DLightOnOff4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue(); }
    bool GetLightOnOff5() const
        { return ((const Svx3DLightOnOff5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue(); }
    bool GetLightOnOff6() const
        { return ((const Svx3DLightOnOff6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue(); }
    bool GetLightOnOff7() const
        { return ((const Svx3DLightOnOff7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue(); }
    bool GetLightOnOff8() const
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
    void SetDrawOnlySelected(bool bNew) { bDrawOnlySelected = bNew; }
    bool GetDrawOnlySelected() const { return bDrawOnlySelected; }
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;

    virtual void    NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void    NbcMove(const Size& rSize) SAL_OVERRIDE;
    virtual void    NbcResize(const Point& rRef, const Fraction& rXFact,
                                                 const Fraction& rYFact) SAL_OVERRIDE;
    virtual void    RecalcSnapRect() SAL_OVERRIDE;

    virtual E3dScene* GetScene() const SAL_OVERRIDE;
    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }
    void removeAllNonSelectedObjects();

    virtual E3dScene* Clone() const SAL_OVERRIDE;
    E3dScene& operator=(const E3dScene&);

    virtual SdrObjGeoData *NewGeoData() const SAL_OVERRIDE;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix) SAL_OVERRIDE;
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix) SAL_OVERRIDE;

    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
    void RotateScene(const Point& rRef, long nWink, double sn, double cs);

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    // get transformations
    B3dCamera& GetCameraSet() { return aCameraSet; }

    // break up
    virtual bool IsBreakObjPossible() SAL_OVERRIDE;

    // polygon which is built during creation
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    // create moves
    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool MovCreate(SdrDragStat& rStat) SAL_OVERRIDE; // true=Xor muss repainted werden
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;
    virtual bool BckCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual void BrkCreate(SdrDragStat& rStat) SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SCENE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
