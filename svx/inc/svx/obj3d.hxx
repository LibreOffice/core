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



#ifndef _E3D_OBJ3D_HXX
#define _E3D_OBJ3D_HXX

#include <svx/svdoattr.hxx>
#include <svx/svdobj.hxx>
#include <svx/volume3d.hxx>
#include <svx/def3d.hxx>
#include <svx/svdpage.hxx>
#include <svx/deflt3d.hxx>
#include <vcl/bitmap.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/xflclit.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2enums.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem;
class Viewport3D;
class E3dScene;
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
|* GeoData relevant fuer Undo-Actions
|*
\************************************************************************/

class E3DObjGeoData : public SdrObjGeoData
{
public:
    basegfx::B3DRange           maLocalBoundVol;    // umschliessendes Volumen des Objekts
    basegfx::B3DHomMatrix       maTransformation;   // lokale Transformation

    E3DObjGeoData() {}
};

/*************************************************************************
|*
|* Basisklasse fuer 3D-Objekte
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dObject : public SdrAttrObj
{
private:
protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    basegfx::B3DRange           maLocalBoundVol;    // umschliessendes Volumen des Objekts (aus geometrieerzeugung)
    basegfx::B3DHomMatrix       maTransformation;   // lokale Transformation
    basegfx::B3DHomMatrix       maFullTransform;    // globale Transformation (inkl. Parents)

    // Flags
    bool                        mbTfHasChanged : 1;
    bool                        mbIsSelected : 1;

    virtual void NewObjectInserted(const E3dObject& r3DObj);
    virtual basegfx::B3DRange RecalcBoundVolume() const;

    // E3dObject is only a helper class (for E3DScene and E3DCompoundObject)
    // and no instances should be created from anyone, so i move the constructors
    // to protected area
    E3dObject(SdrModel& rSdrModel);
    virtual ~E3dObject();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    virtual bool IsClosedObj() const;
    virtual bool IsE3dObject() const;
    virtual void SetBoundVolInvalid();
    virtual void SetTransformChanged();
    virtual void StructureChanged();

    virtual sal_uInt32  GetObjInventor() const;
    virtual sal_uInt16  GetObjIdentifier() const;

    virtual void    TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    E3dObject* GetParentObj() const;
    virtual E3dScene* GetScene() const;

    const basegfx::B3DRange& GetBoundVolume() const;
    virtual void InvalidateBoundVolume();

    // komplette Transformation inklusive aller Parents berechnen
    const basegfx::B3DHomMatrix& GetFullTransform() const;

    // Transformationsmatrix abfragen bzw. (zurueck)setzen
    const basegfx::B3DHomMatrix& GetB3DTransform() const;
    virtual void SetB3DTransform(const basegfx::B3DHomMatrix& rMatrix);
    // virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);

    // get wireframe polygon for local object. No transform is applied.
    basegfx::B3DPolyPolygon CreateWireframe() const;

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    sal_uInt16 GetLogicalGroup() { return 0; }

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    // Selektion Setzen/Lesen
    bool GetSelected() const { return mbIsSelected; }
    virtual void SetSelected(bool bNew);

    // Aufbrechen
    virtual sal_Bool IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

/*************************************************************************
|*
|* Klasse fuer alle zusammengesetzen Objekte (Cube, Lathe, Scene, Extrude)
|* Diese Klasse erspart ein paar RTTI-Abfragen und sie beschleunigt das
|* Verhalten ungemein, da alle Attribute usw. fuer die untergeordneten
|* Polygone hierueber gehalten werden. Die Polygone duerfen nur Attribute
|* halten, wenn sie direkt einer Szene untergeordnet sind.
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dCompoundObject : public E3dObject
{
private:
protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // Material des Objektes
    Color                   aMaterialAmbientColor;

    // Attribute zur Geometrieerzeugung
    bool                    bCreateNormals : 1;
    bool                    bCreateTexture : 1;

protected:
    void SetDefaultAttributes(const E3dDefaultAttributes& rDefault);

    // convert given basegfx::B3DPolyPolygon to screen coor
    basegfx::B2DPolyPolygon TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate);

    // protected constructor due to being only a helper class
    E3dCompoundObject(SdrModel& rSdrModel, const E3dDefaultAttributes& rDefault);
    virtual ~E3dCompoundObject();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public :
    // overloaded to reset maSdrObjectTransformation to identity to flag
    // that the 2D transformation of this 3D object has changed and needs recalculation
    virtual void InvalidateBoundVolume();

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual void    AddToHdlList(SdrHdlList& rHdlList) const;

    // get/setSdrObjectTransformation
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    // DoubleSided: TRUE/FALSE
    bool GetDoubleSided() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_DOUBLE_SIDED)).GetValue(); }

    // NormalsKind: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetNormalsKind() const
        { return ((const Svx3DNormalsKindItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_KIND)).GetValue(); }

    // NormalsInvert: TRUE/FALSE
    bool GetNormalsInvert() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_INVERT)).GetValue(); }

    // TextureProjX: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetTextureProjectionX() const
        { return ((const Svx3DTextureProjectionXItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X)).GetValue(); }

    // TextureProjY: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetTextureProjectionY() const
        { return ((const Svx3DTextureProjectionYItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y)).GetValue(); }

    // Shadow3D: TRUE/FALSE
    bool GetShadow3D() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SHADOW_3D)).GetValue(); }

    // MaterialColor: Color
    Color GetMaterialColor() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_COLOR)).GetValue(); }

    // MaterialEmission: Color
    Color GetMaterialEmission() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_EMISSION)).GetValue(); }

    // MaterialSpecular: Color
    Color GetMaterialSpecular() const
        { return ((const SvxColorItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_SPECULAR)).GetValue(); }

    // MaterialSpecularIntensity:
    sal_uInt16 GetMaterialSpecularIntensity() const
        { return ((const SfxUInt16Item&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY)).GetValue(); }

    // TextureFilter: TRUE/FALSE
    bool GetTextureFilter() const
        { return ((const SfxBoolItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_FILTER)).GetValue(); }

    // #i28528#
    // Added extra Item (Bool) for chart2 to be able to show reduced line geometry
    sal_Bool GetReducedLineGeometry() const
        { return ((const Svx3DReducedLineGeometryItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;

    // Parameter Geometrieerzeugung setzen/lesen
    bool GetCreateNormals() const { return bCreateNormals; }
    void SetCreateNormals(bool bNew);

    bool GetCreateTexture() const { return bCreateTexture; }
    void SetCreateTexture(bool bNew);

    // Material des Objektes
    const Color& GetMaterialAmbientColor() const { return aMaterialAmbientColor; }
    void SetMaterialAmbientColor(const Color& rColor);

    // #110988#
    sal_Bool IsAOrdNumRemapCandidate(E3dScene*& prScene) const;
};

#endif          // _E3D_OBJ3D_HXX
