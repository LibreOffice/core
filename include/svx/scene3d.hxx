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
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

    // transformations
    B3dCamera                   aCameraSet;
    Camera3D                    aCamera;

    Imp3DDepthRemapper*         mp3DDepthRemapper;

    // Flag to determine if only selected objects should be drawn
    bool                        bDrawOnlySelected       : 1;

    virtual void NewObjectInserted(const E3dObject* p3DObj) override;
    virtual void StructureChanged() override;

    void RebuildLists();

    virtual void Notify(SfxBroadcaster &rBC, const SfxHint  &rHint) override;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    void ImpCleanup3DDepthMapper();

public:
    TYPEINFO_OVERRIDE();
    E3dScene();
    E3dScene(E3dDefaultAttributes& rDefault);
    virtual ~E3dScene();

    virtual void SetBoundRectDirty() override;

    // access to cleanup of depth mapper
    void Cleanup3DDepthMapper() { ImpCleanup3DDepthMapper(); }

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;

    // Perspective: enum ProjectionType { PR_PARALLEL, PR_PERSPECTIVE }
    ProjectionType GetPerspective() const
        { return (ProjectionType) static_cast<const Svx3DPerspectiveItem&>(GetObjectItemSet().Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue(); }

    // Distance:
    double GetDistance() const
        { return (double)static_cast<const SfxUInt32Item&>(GetObjectItemSet().Get(SDRATTR_3DSCENE_DISTANCE)).GetValue(); }

    // Focal length: before cm, now 1/10th mm (*100)
    double GetFocalLength() const
        { return static_cast<const SfxUInt32Item&>(GetObjectItemSet().Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue(); }

    // set flag to draw only selected
    void SetDrawOnlySelected(bool bNew) { bDrawOnlySelected = bNew; }
    bool GetDrawOnlySelected() const { return bDrawOnlySelected; }
    virtual sal_uInt16 GetObjIdentifier() const override;

    virtual void    NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void    NbcMove(const Size& rSize) override;
    virtual void    NbcResize(const Point& rRef, const Fraction& rXFact,
                                                 const Fraction& rYFact) override;
    virtual void    RecalcSnapRect() override;

    virtual E3dScene* GetScene() const override;
    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }
    void removeAllNonSelectedObjects();

    virtual E3dScene* Clone() const override;
    E3dScene& operator=(const E3dScene&);

    virtual SdrObjGeoData *NewGeoData() const override;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo) override;

    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix) override;
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix) override;

    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    void RotateScene(const Point& rRef, long nAngle, double sn, double cs);

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    // get transformations
    B3dCamera& GetCameraSet() { return aCameraSet; }
    const B3dCamera& GetCameraSet() const { return aCameraSet; }

    // break up
    virtual bool IsBreakObjPossible() override;

    // polygon which is built during creation
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;

    // create moves
    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override; // true=Xor muss repainted werden
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
};

#endif // INCLUDED_SVX_SCENE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
