/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: obj3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:48:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _E3D_OBJ3D_HXX
#define _E3D_OBJ3D_HXX

#ifndef _SVDOATTR_HXX //autogen
#include <svx/svdoattr.hxx>
#endif

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _VOLUME3D_HXX
#include <svx/volume3d.hxx>
#endif

#ifndef _DEF3D_HXX
#include <svx/def3d.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _E3D_DEFLT3D_HXX
#include <svx/deflt3d.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _B3D_B3DGEOM_HXX
#include <goodies/b3dgeom.hxx>
#endif

#ifndef _B3D_MATRIL3D_HXX
#include <goodies/matril3d.hxx>
#endif

#ifndef _B3D_B3DTEX_HXX
#include <goodies/b3dtex.hxx>
#endif

#ifndef _B3D_B3DLIGHT_HXX
#include <goodies/b3dlight.hxx>
#endif

#ifndef _B3D_BASE3D_HXX
#include <goodies/base3d.hxx>
#endif

#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_VECTOR_B2ENUMS_HXX
#include <basegfx/vector/b2enums.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem;
class Viewport3D;
class E3dScene;
class E3dPolyScene;
class Base3D;
namespace basegfx { class B3DPolyPolygon; }

namespace sdr
{
    namespace properties
    {
        class BaseProperties;
        class E3dProperties;
        class E3dCompoundProperties;
        class E3dExtrudeProperties;
        class E3dLatheProperties;
        class E3dSphereProperties;
    } // end of namespace properties
} // end of namespace sdr

/*************************************************************************
|*
|* Defines fuer nDrawFlag in Paint3D
|*
\************************************************************************/

#define E3D_DRAWFLAG_FILLED                     0x0001
#define E3D_DRAWFLAG_OUTLINE                    0x0002
#define E3D_DRAWFLAG_TRANSPARENT                0x0004
#define E3D_DRAWFLAG_GHOSTED                    0x0008

/*************************************************************************
|*
|* GeoData relevant fuer Undo-Actions
|*
\************************************************************************/

class E3DObjGeoData : public SdrObjGeoData
{
public:
    Volume3D                    aLocalBoundVol;     // umschliessendes Volumen des Objekts
    basegfx::B3DHomMatrix       aTfMatrix;          // lokale Transformation

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
    E3dObjList(SdrModel* pNewModel, SdrPage* pNewPage, E3dObjList* pNewUpList=NULL);
    E3dObjList(const E3dObjList& rSrcList);
    virtual ~E3dObjList();

    virtual void NbcInsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND,
        const SdrInsertReason* pReason=NULL);
    virtual SdrObject* NbcRemoveObject(ULONG nObjNum);
    virtual SdrObject* RemoveObject(ULONG nObjNum);
};

/*************************************************************************
|*
|* Basisklasse fuer 3D-Objekte
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dObject : public SdrAttrObj
{
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::E3dProperties access to StructureChanged()
    friend class sdr::properties::E3dProperties;

    // Aus der E3dObjList und E3dDragMethod alles erlauben
    friend class E3dObjList;
    friend class E3dDragMethod;

 protected:
    E3dObjList*     pSub;               // Subliste (Childobjekte)

    Volume3D        aBoundVol;          // umschliessendes Volumen mit allen Childs
    Volume3D        aLocalBoundVol;     // umschliessendes Volumen des Objekts
    basegfx::B3DHomMatrix       aTfMatrix;          // lokale Transformation
    basegfx::B3DHomMatrix       aFullTfMatrix;      // globale Transformation (inkl. Parents)

    // Flags
    unsigned        bTfHasChanged           : 1;
    unsigned        bBoundVolValid          : 1;
    unsigned        bIsSelected             : 1;

 public:
    virtual void SetBoundVolInvalid();

 protected:
    virtual void SetTransformChanged();
    virtual void NewObjectInserted(const E3dObject* p3DObj);
    virtual void StructureChanged(const E3dObject* p3DObj);
    virtual void RecalcBoundVolume();

    basegfx::B2DPolyPolygon ImpCreateWireframePoly() const;

public:
    TYPEINFO();
    E3dObject();
    E3dObject(BOOL bIsFromChart);

    virtual void RecalcSnapRect();
    virtual void RecalcBoundRect();
    virtual void SetRectsDirty(sal_Bool bNotMyself = sal_False);

    virtual ~E3dObject();

    virtual UINT32  GetObjInventor() const;
    virtual UINT16  GetObjIdentifier() const;

    virtual void    TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    virtual void        NbcSetLayer(SdrLayerID nLayer);

    virtual void        SetObjList(SdrObjList* pNewObjList);
    virtual void        SetPage(SdrPage* pNewPage);
    virtual void        SetModel(SdrModel* pNewModel);
    virtual void        NbcMove(const Size& rSize);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);

    virtual SdrObjList* GetSubList() const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual void    AddToHdlList(SdrHdlList& rHdlList) const;
    virtual FASTBOOL HasSpecialDrag() const;

    // 3D-Zeichenmethode
    virtual void Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
        const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags=0);

    // Objekt als Kontur in das Polygon einfuegen
    virtual basegfx::B2DPolyPolygon ImpTakeContour3D() const;

    // Schatten fuer 3D-Objekte zeichnen
    virtual void DrawShadows(Base3D *pBase3D, XOutputDevice& rXOut,
        const Rectangle& rBound, const Volume3D& rVolume,
        const SdrPaintInfoRec& rInfoRec);

    // 3D-Objekt in die Gruppe einfuegen; Eigentumsuebergang!
    virtual void Insert3DObj(E3dObject* p3DObj);
    void Remove3DObj(E3dObject* p3DObj);

    E3dObject* GetParentObj() const;
    virtual E3dScene* GetScene() const;

    const Volume3D& GetLocalBoundVolume() { return aLocalBoundVol; }
    virtual const Volume3D& GetBoundVolume() const;
    basegfx::B3DPoint GetCenter();

    // komplette Transformation inklusive aller Parents berechnen
    const basegfx::B3DHomMatrix& GetFullTransform() const;

    // Transformationsmatrix abfragen bzw. (zurueck)setzen
    virtual const basegfx::B3DHomMatrix& GetTransform() const;
    virtual void NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix);
    virtual void NbcResetTransform();
    virtual void SetTransform(const basegfx::B3DHomMatrix& rMatrix);
    virtual void ResetTransform();

    // Translation
    virtual void NbcTranslate(const basegfx::B3DVector& rTrans);
    virtual void Translate(const basegfx::B3DVector& rTrans);
    // Skalierung
    virtual void NbcScaleX  (double fSx);
    virtual void NbcScaleY  (double fSy);
    virtual void NbcScaleZ  (double fSz);
    virtual void NbcScale   (double fSx, double fSy, double fSz);
    virtual void NbcScale   (double fS);

    virtual void ScaleX (double fSx);
    virtual void ScaleY (double fSy);
    virtual void ScaleZ (double fSz);
    virtual void Scale  (double fSx, double fSy, double fSz);
    virtual void Scale  (double fS);

    // Rotation mit Winkel in Radiant
    virtual void NbcRotateX(double fAng);
    virtual void NbcRotateY(double fAng);
    virtual void NbcRotateZ(double fAng);

    virtual void RotateX(double fAng);
    virtual void RotateY(double fAng);
    virtual void RotateZ(double fAng);

    // [FG] 2D-Rotationen, werden hier als Rotationen um die Z-Achse, die in den Bildschirm zeigt,
    //      implementiert plus eine Verschiebung der Scene. Dies bedeutet auch die Scene (E3dScene)
    //      muss diese Routine in der Klasse als virtual definieren.
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);

    // Transformation auf die Koordinaten (nicht auf die lokale Matrix)
    // eines Objekts und seiner Childs anwenden; Objekte, die eigene
    // Koordinaten speichern, muessen diese Methode implementieren
    // Wireframe-Darstellung des Objekts erzeugen und die Linien als
    // Punkt-Paare in rPoly3D ablegen
    void CreateWireframe(basegfx::B3DPolygon& rWirePoly, const basegfx::B3DHomMatrix* pTf = 0L) const;

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    USHORT GetLogicalGroup() { return 0; }
    virtual void operator=(const SdrObject&);

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    // Selektion Setzen/Lesen
    BOOL GetSelected() { return bIsSelected; }
    void SetSelected(BOOL bNew);

    // Aufbrechen
    virtual BOOL IsBreakObjPossible();
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
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::E3dCompoundProperties access to SetGeometryValid()
    friend class sdr::properties::E3dCompoundProperties;
    friend class sdr::properties::E3dExtrudeProperties;
    friend class sdr::properties::E3dLatheProperties;
    friend class sdr::properties::E3dSphereProperties;

    // for access from E3dCompoundProperties only
    void InvalidateGeometry() { bGeometryValid = sal_False; }

protected:
    // Die Darstellungsgeometrie dieses Objektes
    B3dGeometry             aDisplayGeometry;

    // Material des Objektes
    Color                   aMaterialAmbientColor;
    B3dMaterial             aBackMaterial;

    // Attribute zur Geometrieerzeugung
    unsigned                bCreateNormals              : 1;
    unsigned                bCreateTexture              : 1;

    // Wird zwischen Vorder- und Hintergrundmaterial unterschieden
    unsigned                bUseDifferentBackMaterial   : 1;

    // Geometrie gueltig?
    unsigned                bGeometryValid              : 1;

    // THB: Temporary fix for SJ's flipping problem
    // TODO: Clarify with AW
    unsigned                bFullTfIsPositive           : 1;

protected:
    // Hilfsfunktionen zur Geometrieerzeugung
    basegfx::B3DPolyPolygon ImpGrowPoly(
        const basegfx::B3DPolyPolygon& rPolyPolyGrow,
        const basegfx::B3DPolyPolygon& rPolyPolyNormals,
        double fFactor);
    basegfx::B2VectorOrientation ImpGetOrientationInPoint(
        const basegfx::B3DPolygon& rPolygon,
        sal_uInt32 nIndex);
    basegfx::B3DPolyPolygon ImpCorrectGrownPoly(
        const basegfx::B3DPolyPolygon& aToBeCorrected,
        const basegfx::B3DPolyPolygon& aOriginal);

    basegfx::B3DPolyPolygon ImpScalePoly(
        const basegfx::B3DPolyPolygon& rPolyPolyScale,
        double fFactor);

    void ImpCreateFront(
        const basegfx::B3DPolyPolygon& rPolyPoly3D,
        const basegfx::B3DPolyPolygon& rFrontNormals,
        BOOL bCreateNormals = TRUE,
        BOOL bCreateTexture = TRUE);
    void ImpCreateBack(
        const basegfx::B3DPolyPolygon& rPolyPoly3D,
        const basegfx::B3DPolyPolygon& rBackNormals,
        BOOL bCreateNormals = TRUE,
        BOOL bCreateTexture = TRUE);

    basegfx::B3DPolyPolygon ImpCreateByPattern(const basegfx::B3DPolyPolygon& rPattern);
    basegfx::B3DPolyPolygon ImpAddFrontNormals(
        const basegfx::B3DPolyPolygon& rNormalsFront,
        const basegfx::B3DPoint& rOffset);
    basegfx::B3DPolyPolygon ImpAddBackNormals(
        const basegfx::B3DPolyPolygon& rNormalsBack,
        const basegfx::B3DPoint& rOffset);

    basegfx::B3DPolyPolygon ImpAddInBetweenNormals(
        const basegfx::B3DPolyPolygon& rPolyPolyFront,
        const basegfx::B3DPolyPolygon& rPolyPolyBack,
        const basegfx::B3DPolyPolygon& rNormals,
        BOOL bSmoothed = TRUE);
    void ImpCreateInBetween(
        const basegfx::B3DPolyPolygon& rPolyPolyFront,
        const basegfx::B3DPolyPolygon& rPolyPolyBack,
        const basegfx::B3DPolyPolygon& rFrontNormals,
        const basegfx::B3DPolyPolygon& rBackNormals,
        BOOL bCreateNormals = TRUE,
        double fSurroundFactor=1.0,
        double fTextureStart=0.0,
        double fTextureDepth=1.0,
        BOOL bRotateTexture90=FALSE);

    // Geometrieerzeugung
    void AddGeometry(
        const basegfx::B3DPolyPolygon& rPolyPolygon,
        BOOL bHintIsComplex=TRUE,
        BOOL bOutline=FALSE);
    void AddGeometry(
        const basegfx::B3DPolyPolygon& rPolyPolygon,
        const basegfx::B3DPolyPolygon& rPolyPolygonNormal,
        BOOL bHintIsComplex=TRUE,
        BOOL bOutline=FALSE);
    void AddGeometry(
        const basegfx::B3DPolyPolygon& rPolyPolygon,
        const basegfx::B3DPolyPolygon& rPolyPolygonNormal,
        const basegfx::B2DPolyPolygon& rPolyPolygonTexture,
        BOOL bHintIsComplex=TRUE,
        BOOL bOutline=FALSE);
    void StartCreateGeometry();

    // Segmenterzeugung
    void ImpCreateSegment(
        const basegfx::B3DPolyPolygon& rFront,      // vorderes Polygon
        const basegfx::B3DPolyPolygon& rBack,           // hinteres Polygon
        const basegfx::B3DPolyPolygon* pPrev = 0L,  // smooth uebergang zu Vorgaenger
        const basegfx::B3DPolyPolygon* pNext = 0L,  // smooth uebergang zu Nachfolger
        BOOL bCreateFront = TRUE,           // vorderen Deckel erzeugen
        BOOL bCreateBack = TRUE,            // hinteren Deckel erzeugen
        double fPercentDiag = 0.05,         // Anteil des Deckels an der Tiefe
        BOOL bSmoothLeft = TRUE,            // Glaetten der umlaufenden Normalen links
        BOOL bSmoothRight = TRUE,           // Glaetten der umlaufenden Normalen rechts
        BOOL bSmoothFrontBack = FALSE,      // Glaetten der Abschlussflaechen
        double fSurroundFactor = 1.0,       // Wertebereich der Texturkoordinaten im Umlauf
        double fTextureStart = 0.0,         // TexCoor ueber Extrude-Tiefe
        double fTextureDepth = 1.0,         // TexCoor ueber Extrude-Tiefe
        BOOL bCreateTexture = TRUE,
        BOOL bCreateNormals = TRUE,
        BOOL bCharacterExtrude = FALSE,     // FALSE=exakt, TRUE=ohne Ueberschneidungen
        BOOL bRotateTexture90 = FALSE,      // Textur der Seitenflaechen um 90 Grad kippen
        // #i28528#
        basegfx::B3DPolyPolygon* pLineGeometryFront = 0L,   // For creation of line geometry front parts
        basegfx::B3DPolyPolygon* pLineGeometryBack = 0L,    // For creation of line geometry back parts
        basegfx::B3DPolyPolygon* pLineGeometry = 0L     // For creation of line geometry in-betweens
        );

    // #i28528#
    basegfx::B3DPolyPolygon ImpCompleteLinePolygon(const basegfx::B3DPolyPolygon& rLinePolyPoly, sal_uInt32 nPolysPerRun, sal_Bool bClosed);

    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    // convert given basegfx::B3DPolyPolygon to screen coor
    basegfx::B2DPolyPolygon TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate);

public :
    TYPEINFO();

    E3dCompoundObject();
    E3dCompoundObject(E3dDefaultAttributes& rDefault);
    virtual ~E3dCompoundObject();

    // DoubleSided: TRUE/FALSE
    BOOL GetDoubleSided() const
        { return ((const Svx3DDoubleSidedItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_DOUBLE_SIDED)).GetValue(); }

    // NormalsKind: 0 == FALSE/FALSE, 1 == TRUE/FALSE, else == TRUE/TRUE
    sal_uInt16 GetNormalsKind() const
        { return ((const Svx3DNormalsKindItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_KIND)).GetValue(); }

    // NormalsInvert: TRUE/FALSE
    BOOL GetNormalsInvert() const
        { return ((const Svx3DNormalsInvertItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_NORMALS_INVERT)).GetValue(); }

    // TextureProjX: 0 == FALSE/FALSE, 1 == TRUE/FALSE, else == TRUE/TRUE
    sal_uInt16 GetTextureProjectionX() const
        { return ((const Svx3DTextureProjectionXItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X)).GetValue(); }

    // TextureProjY: 0 == FALSE/FALSE, 1 == TRUE/FALSE, else == TRUE/TRUE
    sal_uInt16 GetTextureProjectionY() const
        { return ((const Svx3DTextureProjectionYItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y)).GetValue(); }

    // Shadow3D: TRUE/FALSE
    BOOL GetShadow3D() const
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

    // TextureKind: 1 == Base3DTextureLuminance, 2 == Base3DTextureIntensity, 3 == Base3DTextureColor
    Base3DTextureKind GetTextureKind() const
        { return (Base3DTextureKind)((const Svx3DTextureKindItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_KIND)).GetValue(); }

    // TextureMode: 1 == Base3DTextureReplace, 2 == Base3DTextureModulate, 3 == Base3DTextureBlend
    Base3DTextureMode GetTextureMode() const
        { return (Base3DTextureMode)((const Svx3DTextureModeItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_MODE)).GetValue(); }

    // TextureFilter: TRUE/FALSE
    BOOL GetTextureFilter() const
        { return ((const Svx3DTextureFilterItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_TEXTURE_FILTER)).GetValue(); }

    // #i28528#
    // Added extra Item (Bool) for chart2 to be able to show reduced line geometry
    BOOL GetReducedLineGeometry() const
        { return ((const Svx3DReducedLineGeometryItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)).GetValue(); }

    virtual UINT16 GetObjIdentifier() const;
    virtual void RecalcSnapRect();
    virtual void RecalcBoundRect();
    virtual const Volume3D& GetBoundVolume() const;

    // Hittest, wird an Geometrie weitergegeben
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    // #110988# test if given hit candidate point is inside bound volume of object. Used
    // from CheckHit.
    sal_Bool ImpIsInsideBoundVolume(const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, const Point& rPnt) const;

    // 3D-Zeichenmethode
    virtual void Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
        const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags=0);

    // Objekt als Kontur in das Polygon einfuegen
    virtual basegfx::B2DPolyPolygon ImpTakeContour3D() const;

    // Schatten fuer 3D-Objekte zeichnen
    virtual void DrawShadows(Base3D *pBase3D, XOutputDevice& rXOut,
        const Rectangle& rBound, const Volume3D& rVolume,
        const SdrPaintInfoRec& rInfoRec);

    // #78972#
    basegfx::B2DPolyPolygon ImpGetShadowPolygon() const;
    void ImpDrawShadowPolygon(const basegfx::B2DPolyPolygon& rPoly, XOutputDevice& rXOut);

    // Bitmaps fuer 3D-Darstellung von Gradients und Hatches holen
    AlphaMask GetAlphaMask(const SfxItemSet& rSet, const Size& rSizePixel);
    Bitmap GetGradientBitmap(const SfxItemSet&);
    Bitmap GetHatchBitmap(const SfxItemSet&);

    // Geometrieerzeugung
    void DestroyGeometry();
    virtual void CreateGeometry();
    void ReCreateGeometry();

    // Give out simple line geometry
    virtual basegfx::B3DPolyPolygon Get3DLineGeometry() const;

    // Parameter Geometrieerzeugung setzen/lesen
    BOOL GetCreateNormals() const { return bCreateNormals; }
    void SetCreateNormals(BOOL bNew);

    BOOL GetCreateTexture() const { return bCreateTexture; }
    void SetCreateTexture(BOOL bNew);

    // Copy-Operator
    virtual void operator=(const SdrObject&);

    // Ausgabeparameter an 3D-Kontext setzen
    void SetBase3DParams(XOutputDevice& rOut, Base3D*, BOOL& bDrawObject, BOOL& bDrawOutline, UINT16 nDrawFlags, BOOL bGhosted);
private:
    SVX_DLLPRIVATE sal_Bool ImpSet3DParForFill(XOutputDevice& rOut, Base3D* pBase3D, UINT16 nDrawFlags, BOOL bGhosted);
    SVX_DLLPRIVATE sal_Bool ImpSet3DParForLine(XOutputDevice& rOut, Base3D* pBase3D, UINT16 nDrawFlags, BOOL bGhosted);
public:

    // DisplayGeometry rausruecken
    const B3dGeometry& GetDisplayGeometry() const;

    // Schattenattribute holen
    Color GetShadowColor() const;
    BOOL DrawShadowAsOutline() const;
    INT32 GetShadowXDistance() const;
    INT32 GetShadowYDistance() const;
    UINT16 GetShadowTransparence() const;
    BOOL DoDrawShadow();

    // WireFrame Darstellung eines Objektes
    void DrawObjectWireframe(XOutputDevice& rOut);

    // Nromalen invertiert benutzen
private:
    SVX_DLLPRIVATE void SetInvertNormals(BOOL bNew);
public:

    // Material des Objektes
    const Color& GetMaterialAmbientColor() const { return aMaterialAmbientColor; }
    void SetMaterialAmbientColor(const Color& rColor);

    const B3dMaterial& GetBackMaterial() const { return aBackMaterial; }
    void SetBackMaterial(const B3dMaterial& rNew);
    BOOL GetUseDifferentBackMaterial() const { return bUseDifferentBackMaterial; }
    void SetUseDifferentBackMaterial(BOOL bNew);

    // #110988#
    double GetMinimalDepthInViewCoor(E3dScene& rScene) const;
    sal_Bool IsAOrdNumRemapCandidate(E3dScene*& prScene) const;
};

#endif          // _E3D_OBJ3D_HXX
