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

class SVX_DLLPUBLIC E3dScene
:   public E3dObject,
    public SdrObjList
{
private:
protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // Transformationen
    B3dCamera                   aCameraSet;
    Camera3D                    aCamera;

    // #110988#
    Imp3DDepthRemapper*         mp3DDepthRemapper;

    // Flag to determine if only selected objects should be drawn
    bool                        bDrawOnlySelected : 1;

    virtual void NewObjectInserted(const E3dObject& r3DObj);

    void RebuildLists();

    virtual void Notify(SfxBroadcaster &rBC, const SfxHint  &rHint);

    void SetDefaultAttributes(const E3dDefaultAttributes& rDefault);
    void ImpCleanup3DDepthMapper();
    virtual basegfx::B3DRange RecalcBoundVolume() const;

    virtual ~E3dScene();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    E3dScene(
        SdrModel& rSdrModel,
        const E3dDefaultAttributes& rDefault);

    // derived from SdrObjList
    virtual SdrPage* getSdrPageFromSdrObjList() const;
    virtual SdrObject* getSdrObjectFromSdrObjList() const;
    virtual SdrModel& getSdrModelFromSdrObjList() const;
    virtual void handleContentChange(const SfxHint& rHint);

    // derived from SdrObject
    virtual SdrObjList* getChildrenOfSdrObject() const;

    // react on model change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    virtual void StructureChanged();
    virtual void SetBoundVolInvalid();

    virtual void SetLayer(SdrLayerID nLayer);
    virtual void SetTransformChanged();

    // 3D-Objekt in die Gruppe einfuegen; Eigentumsuebergang!
    virtual void Insert3DObj(E3dObject& r3DObj);
    void Remove3DObj(E3dObject& r3DObj);

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // #110988#
    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;

    // Perspective: enum ProjectionType { PR_PARALLEL, PR_PERSPECTIVE }
    ProjectionType GetPerspective() const
        { return (ProjectionType)((const Svx3DPerspectiveItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue(); }

    // Distance:
    double GetDistance() const
        { return (double)((const SfxUInt32Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_DISTANCE)).GetValue(); }

    // Focal length: before cm, now 1/10th mm (*100)
    double GetFocalLength() const
        { return ((const SfxUInt32Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue(); }

    // Two sided lighting:
    bool GetTwoSidedLighting() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue(); }

    // Lightcolor:
    Color GetLightColor1() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue(); }
    Color GetLightColor2() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue(); }
    Color GetLightColor3() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue(); }
    Color GetLightColor4() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue(); }
    Color GetLightColor5() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue(); }
    Color GetLightColor6() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue(); }
    Color GetLightColor7() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue(); }
    Color GetLightColor8() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue(); }

    // Ambient color:
    Color GetGlobalAmbientColor() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue(); }

    // Light on/off:
    bool GetLightOnOff1() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue(); }
    bool GetLightOnOff2() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue(); }
    bool GetLightOnOff3() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue(); }
    bool GetLightOnOff4() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue(); }
    bool GetLightOnOff5() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue(); }
    bool GetLightOnOff6() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue(); }
    bool GetLightOnOff7() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue(); }
    bool GetLightOnOff8() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue(); }

    // Light direction:
    basegfx::B3DVector GetLightDirection1() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue(); }
    basegfx::B3DVector GetLightDirection2() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue(); }
    basegfx::B3DVector GetLightDirection3() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue(); }
    basegfx::B3DVector GetLightDirection4() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue(); }
    basegfx::B3DVector GetLightDirection5() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue(); }
    basegfx::B3DVector GetLightDirection6() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue(); }
    basegfx::B3DVector GetLightDirection7() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue(); }
    basegfx::B3DVector GetLightDirection8() const
        { return ((const SvxB3DVectorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue(); }

    // ShadowSlant:
    sal_uInt16 GetShadowSlant() const
        { return ((const SfxUInt16Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue(); }

    // ShadeMode: 0 == FLAT, 1 == PHONG, 2 == SMOOTH, 3 == ForceDraft
    sal_uInt16 GetShadeMode() const
        { return ((const Svx3DShadeModeItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue(); }

    // set flag to draw only selected
    void SetDrawOnlySelected(sal_Bool bNew) { bDrawOnlySelected = bNew; }
    bool GetDrawOnlySelected() const { return bDrawOnlySelected; }
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual E3dScene* GetScene() const;
    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }
    void removeAllNonSelectedObjects();

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    virtual void SetB3DTransform(const basegfx::B3DHomMatrix& rMatrix);
    // virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);

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
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);

    // Selektion Setzen/Lesen
    virtual void SetSelected(bool bNew);

    // get/setSdrObjectTransformation
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
};

#endif          // _E3D_SCENE3D_HXX
