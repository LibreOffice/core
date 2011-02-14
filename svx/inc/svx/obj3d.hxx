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
class E3dPolyScene;

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
|* Liste fuer 3D-Objekte
|*
\************************************************************************/

class E3dObjList : public SdrObjList
{
public:
    TYPEINFO();
    E3dObjList(SdrModel* pNewModel = 0, SdrPage* pNewPage = 0, E3dObjList* pNewUpList = 0);
    SVX_DLLPUBLIC E3dObjList(const E3dObjList& rSrcList);
    SVX_DLLPUBLIC virtual ~E3dObjList();

    virtual void NbcInsertObject(SdrObject* pObj, sal_uIntPtr nPos=CONTAINER_APPEND, const SdrInsertReason* pReason=NULL);
    virtual void InsertObject(SdrObject* pObj, sal_uIntPtr nPos=CONTAINER_APPEND, const SdrInsertReason* pReason=NULL);
    virtual SdrObject* NbcRemoveObject(sal_uIntPtr nObjNum);
    virtual SdrObject* RemoveObject(sal_uIntPtr nObjNum);
};

/*************************************************************************
|*
|* Basisklasse fuer 3D-Objekte
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dObject : public SdrAttrObj
{
private:
    // to allow sdr::properties::E3dProperties access to StructureChanged()
    friend class sdr::properties::E3dProperties;

    // Aus der E3dObjList und E3dDragMethod alles erlauben
    friend class E3dObjList;
    friend class E3dDragMethod;

 protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    E3dObjList                  maSubList;          // Subliste (Childobjekte)

    basegfx::B3DRange           maLocalBoundVol;    // umschliessendes Volumen des Objekts (aus geometrieerzeugung)
    basegfx::B3DHomMatrix       maTransformation;   // lokale Transformation
    basegfx::B3DHomMatrix       maFullTransform;    // globale Transformation (inkl. Parents)

    // Flags
    unsigned        mbTfHasChanged          : 1;
    unsigned        mbIsSelected            : 1;

 public:
    void SetBoundVolInvalid();

 protected:
    void SetTransformChanged();
    virtual void NewObjectInserted(const E3dObject* p3DObj);
    virtual void StructureChanged();
    basegfx::B3DRange RecalcBoundVolume() const;

protected:
    // E3dObject is only a helper class (for E3DScene and E3DCompoundObject)
    // and no instances should be created from anyone, so i move the constructors
    // to protected area
    E3dObject();
    E3dObject(sal_Bool bIsFromChart);

public:
    TYPEINFO();
    virtual void RecalcSnapRect();
    virtual void SetRectsDirty(sal_Bool bNotMyself = sal_False);

    virtual ~E3dObject();

    virtual sal_uInt32  GetObjInventor() const;
    virtual sal_uInt16  GetObjIdentifier() const;

    virtual void    TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    virtual void        NbcSetLayer(SdrLayerID nLayer);

    virtual void        SetObjList(SdrObjList* pNewObjList);
    virtual void        SetPage(SdrPage* pNewPage);
    virtual void        SetModel(SdrModel* pNewModel);
    virtual void        NbcMove(const Size& rSize);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual SdrObjList* GetSubList() const;

    // 3D-Objekt in die Gruppe einfuegen; Eigentumsuebergang!
    virtual void Insert3DObj(E3dObject* p3DObj);
    void Remove3DObj(E3dObject* p3DObj);

    E3dObject* GetParentObj() const;
    virtual E3dScene* GetScene() const;

    const basegfx::B3DRange& GetBoundVolume() const;
    void InvalidateBoundVolume();

    // komplette Transformation inklusive aller Parents berechnen
    const basegfx::B3DHomMatrix& GetFullTransform() const;

    // Transformationsmatrix abfragen bzw. (zurueck)setzen
    const basegfx::B3DHomMatrix& GetTransform() const;
    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix);
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);

    // [FG] 2D-Rotationen, werden hier als Rotationen um die Z-Achse, die in den Bildschirm zeigt,
    //      implementiert plus eine Verschiebung der Scene. Dies bedeutet auch die Scene (E3dScene)
    //      muss diese Routine in der Klasse als virtual definieren.
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);

    // get wireframe polygon for local object. No transform is applied.
    basegfx::B3DPolyPolygon CreateWireframe() const;

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    sal_uInt16 GetLogicalGroup() { return 0; }
    virtual void operator=(const SdrObject&);

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    // Selektion Setzen/Lesen
    bool GetSelected() const { return mbIsSelected; }
    void SetSelected(bool bNew);

    // Aufbrechen
    virtual sal_Bool IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

/*************************************************************************
|*
|* Klasse fuer alle zusammengesetzen Objekte (Cube, Lathe, Scene, Extrude)
|* Diese Klasse erspart ein paar ISA-Abfragen und sie beschleunigt das
|* Verhalten ungemein, da alle Attribute usw. fuer die untergeordneten
|* Polygone hierueber gehalten werden. Die Polygone duerfen nur Attribute
|* halten, wenn sie direkt einer Szene untergeordnet sind.
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
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // Material des Objektes
    Color                   aMaterialAmbientColor;

    // Attribute zur Geometrieerzeugung
    unsigned                bCreateNormals              : 1;
    unsigned                bCreateTexture              : 1;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    // convert given basegfx::B3DPolyPolygon to screen coor
    basegfx::B2DPolyPolygon TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate);

public :
    TYPEINFO();

    E3dCompoundObject();
    E3dCompoundObject(E3dDefaultAttributes& rDefault);
    virtual ~E3dCompoundObject();

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual void    AddToHdlList(SdrHdlList& rHdlList) const;

    // DoubleSided: sal_True/FALSE
    sal_Bool GetDoubleSided() const
        { return ((const Svx3DDoubleSidedItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_DOUBLE_SIDED)).GetValue(); }

    // NormalsKind: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetNormalsKind() const
        { return ((const Svx3DNormalsKindItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_KIND)).GetValue(); }

    // NormalsInvert: sal_True/FALSE
    sal_Bool GetNormalsInvert() const
        { return ((const Svx3DNormalsInvertItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_INVERT)).GetValue(); }

    // TextureProjX: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetTextureProjectionX() const
        { return ((const Svx3DTextureProjectionXItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X)).GetValue(); }

    // TextureProjY: 0 == sal_False/sal_False, 1 == sal_True/sal_False, else == sal_True/TRUE
    sal_uInt16 GetTextureProjectionY() const
        { return ((const Svx3DTextureProjectionYItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y)).GetValue(); }

    // Shadow3D: sal_True/FALSE
    sal_Bool GetShadow3D() const
        { return ((const Svx3DShadow3DItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SHADOW_3D)).GetValue(); }

    // MaterialColor: Color
    Color GetMaterialColor() const
        { return ((const Svx3DMaterialColorItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_COLOR)).GetValue(); }

    // MaterialEmission: Color
    Color GetMaterialEmission() const
        { return ((const Svx3DMaterialEmissionItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_EMISSION)).GetValue(); }

    // MaterialSpecular: Color
    Color GetMaterialSpecular() const
        { return ((const Svx3DMaterialSpecularItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_SPECULAR)).GetValue(); }

    // MaterialSpecularIntensity:
    sal_uInt16 GetMaterialSpecularIntensity() const
        { return ((const Svx3DMaterialSpecularIntensityItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY)).GetValue(); }

    // TextureFilter: sal_True/FALSE
    sal_Bool GetTextureFilter() const
        { return ((const Svx3DTextureFilterItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_FILTER)).GetValue(); }

    // #i28528#
    // Added extra Item (Bool) for chart2 to be able to show reduced line geometry
    sal_Bool GetReducedLineGeometry() const
        { return ((const Svx3DReducedLineGeometryItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void RecalcSnapRect();

    // Parameter Geometrieerzeugung setzen/lesen
    sal_Bool GetCreateNormals() const { return bCreateNormals; }
    void SetCreateNormals(sal_Bool bNew);

    sal_Bool GetCreateTexture() const { return bCreateTexture; }
    void SetCreateTexture(sal_Bool bNew);

    // Copy-Operator
    virtual void operator=(const SdrObject&);

    // Material des Objektes
    const Color& GetMaterialAmbientColor() const { return aMaterialAmbientColor; }
    void SetMaterialAmbientColor(const Color& rColor);

    // #110988#
    sal_Bool IsAOrdNumRemapCandidate(E3dScene*& prScene) const;
};

#endif          // _E3D_OBJ3D_HXX
