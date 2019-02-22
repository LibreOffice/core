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

#ifndef INCLUDED_SVX_OBJ3D_HXX
#define INCLUDED_SVX_OBJ3D_HXX

#include <svx/svdoattr.hxx>
#include <svx/svdobj.hxx>
#include <svx/def3d.hxx>
#include <svx/svdpage.hxx>
#include <svx/deflt3d.hxx>
#include <vcl/bitmap.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/xflclit.hxx>
#include <svl/itemset.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <svx/svxdllapi.h>

// Forward declarations
class SfxPoolItem;
class Viewport3D;
class E3dScene;

namespace basegfx { class B3DPolyPolygon; }
namespace sdr { namespace properties {
    class BaseProperties;
    class E3dProperties;
    class E3dCompoundProperties;
    class E3dExtrudeProperties;
    class E3dLatheProperties;
    class E3dSphereProperties;
}}

/*************************************************************************
|*
|* GeoData relevant for undo actions
|*
\************************************************************************/

class E3DObjGeoData : public SdrObjGeoData
{
public:
    basegfx::B3DRange           maLocalBoundVol;    // surrounding volume of the object
    basegfx::B3DHomMatrix       maTransformation;   // local transformations

    E3DObjGeoData() {}
};

/*************************************************************************
|*
|* Base class for 3D objects
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dObject : public SdrAttrObj
{
private:
    // Allow everything for E3dObjList and E3dDragMethod
    friend class E3dDragMethod;

 protected:
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    basegfx::B3DRange           maLocalBoundVol;    // surrounding volume of the object (from the geometry generation)
    basegfx::B3DHomMatrix       maTransformation;   // local transformation
    basegfx::B3DHomMatrix       maFullTransform;    // global transformation (including. parents)

    // Flags
    bool            mbTfHasChanged          : 1;
    bool            mbIsSelected            : 1;

protected:
    virtual basegfx::B3DRange RecalcBoundVolume() const;

    // E3dObject is only a helper class (for E3DScene and E3DCompoundObject)
    // and no instances should be created from anyone, so i move the constructors
    // to protected area
    E3dObject(SdrModel& rSdrModel);

    // protected destructor
    virtual ~E3dObject() override;

public:
    virtual void StructureChanged();
    virtual void SetTransformChanged();
    virtual void RecalcSnapRect() override;

    virtual SdrInventor GetObjInventor() const override;
    virtual sal_uInt16  GetObjIdentifier() const override;
    virtual void        TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual void        NbcMove(const Size& rSize) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;

    E3dScene* getParentE3dSceneFromE3dObject() const;
    virtual E3dScene* getRootE3dSceneFromE3dObject() const;

    const basegfx::B3DRange& GetBoundVolume() const;
    void InvalidateBoundVolume();

    // calculate complete transformation including all parents
    const basegfx::B3DHomMatrix& GetFullTransform() const;

    // get and (re)set transformation matrix
    const basegfx::B3DHomMatrix& GetTransform() const { return maTransformation;}
    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix);
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);

    // 2D rotations, are implemented as a rotation around the Z axis
    // which is vertical to the screen, plus a shift of the scene.
    // This means that also the scene (E3dScene) must define this
    // routine as virtual in its class.
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;

    // get wireframe polygon for local object. No transform is applied.
    basegfx::B3DPolyPolygon CreateWireframe() const;

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
    virtual E3dObject* CloneSdrObject(SdrModel& rTargetModel) const override;
    E3dObject& operator=( const E3dObject& rObj );

    virtual SdrObjGeoData *NewGeoData() const override;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo) override;

    // get/set the selection
    bool GetSelected() const { return mbIsSelected; }
    virtual void SetSelected(bool bNew);

    // break up
    virtual bool IsBreakObjPossible();
    virtual std::unique_ptr<SdrAttrObj,SdrObjectFreeOp> GetBreakObj();
};

/*************************************************************************
|*
|* Class for all compound objects (Cube, Lathe, Scene, Extrude)
|* This class saves some ISA queries and accelerates the behaviour
|* significantly, because all the attributes etc. are kept through this.
|* The polygons may only keep attributes if they are directly
|* subordinated to the scene.
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dCompoundObject : public E3dObject
{
private:
    // to allow sdr::properties::E3dCompoundProperties access to SetGeometryValid()
    friend class sdr::properties::E3dCompoundProperties;
    friend class sdr::properties::E3dExtrudeProperties;
    friend class sdr::properties::E3dLatheProperties;
    friend class sdr::properties::E3dSphereProperties;

protected:
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

protected:
    // convert given basegfx::B3DPolyPolygon to screen coor
    basegfx::B2DPolyPolygon TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate);

    // protected destructor
    virtual ~E3dCompoundObject() override;

public:
    E3dCompoundObject(SdrModel& rSdrModel);

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual void    AddToHdlList(SdrHdlList& rHdlList) const override;

    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void RecalcSnapRect() override;

    virtual E3dCompoundObject* CloneSdrObject(SdrModel& rTargetModel) const override;

    // implemented mainly for the purposes of Clone()
    E3dCompoundObject& operator=(const E3dCompoundObject& rObj);
};

#endif // INCLUDED_SVX_OBJ3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
