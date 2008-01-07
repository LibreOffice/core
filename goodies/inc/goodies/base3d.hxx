/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: base3d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:59:31 $
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

#ifndef _B3D_BASE3D_HXX
#define _B3D_BASE3D_HXX

#ifndef _B3D_B3DLIGHT_HXX
#include <goodies/b3dlight.hxx>
#endif

//#ifndef _B3D_HMATRIX_HXX
//#include "hmatrix.hxx"
//#endif

#ifndef _B3D_B3DENTITY_HXX
#include <goodies/b3dentty.hxx>
#endif

#ifndef _B3D_B3DCOMPO_HXX
#include <goodies/b3dcompo.hxx>
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif

// MUSS-Aenderung
#ifndef _SV_OUTDEV3D_HXX
#include <vcl/outdev3d.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

// Vorausdeklarationen
class B3dTexture;
class OutputDevice;
class Bitmap;
class BitmapEx;
class TextureAttributes;
class B3dTransformationSet;

/*************************************************************************
|*
|* Texturenverwaltung
|*
\************************************************************************/

SV_DECL_PTRARR_DEL(B3dTextureStore, B3dTexture*, 0, 4)

/*************************************************************************
|*
|* Globale Daten fuer Base3D
|*
\************************************************************************/

class B3dGlobalData
{
private:
    // Texturverwaltung
    Container               maTextureStore;
    AutoTimer               maTimer;
    vos::OMutex             maMutex;

    // link for timer
    DECL_LINK(TimerHdl, AutoTimer*);

public:
    B3dGlobalData();
    virtual ~B3dGlobalData();

    B3dTexture* ObtainTexture(TextureAttributes& rAtt);
    void InsertTexture(B3dTexture* pNew);
    void DeleteTexture(B3dTexture* pOld);
    void DeleteAllTextures();
};

/*************************************************************************
|*
|* Bisher vorhandene Renderer
|*
\************************************************************************/

#define BASE3D_TYPE_DEFAULT         0x0000
#define BASE3D_TYPE_OPENGL          0x0001
#define BASE3D_TYPE_PRINTER         0x0002

/*************************************************************************
|*
|* Moegliche Objekttypen fuer Primitive
|*
\************************************************************************/

enum Base3DObjectMode
{

    // die folgenden Primitive muessen von jedem abgeleiteten Renderer
    // ausgefuehrt werden koennen

    Base3DPoints = 0,
    Base3DLines,
    Base3DLineLoop,
    Base3DLineStrip,
    Base3DTriangles,
    Base3DTriangleStrip,
    Base3DTriangleFan,
    Base3DQuads,
    Base3DQuadStrip,
    Base3DPolygon,

    // ab hier beginnen hoehere Primitive, die mit speziellen Methoden
    // innerhalb von Base3D auf die obenstehenden Primitive
    // abgebildet werden. Diese Methoden tun dies alles im object
    // coordinate system.

    // PolyPolygon ohne Selbstueberschneidungen und ohne
    // Ueberschneidungen der evtl. vorhandenen Teilpolygone
    Base3DComplexPolygon,

    // Beliebiges PolyPolygon mit Selbstueberschneidungen und
    // Ueberschneidungen der weiteren Teilpolygone
    Base3DComplexPolygonCut
};

/*************************************************************************
|*
|* Moegliche Darstellungsarten fuer Primitive
|*
\************************************************************************/

enum Base3DRenderMode
{
    Base3DRenderNone = 0,
    Base3DRenderPoint,
    Base3DRenderLine,
    Base3DRenderFill
};

/*************************************************************************
|*
|* Moegliche CullingModes fuer Polygone
|*
\************************************************************************/

enum Base3DCullMode
{
    Base3DCullNone = 0,
    Base3DCullFront,
    Base3DCullBack
};

/*************************************************************************
|*
|* Moegliche ShadeModels fuer Polygone
|*
\************************************************************************/

enum Base3DShadeModel
{
    Base3DSmooth = 0,
    Base3DFlat,
    Base3DPhong
};

/*************************************************************************
|*
|* Unterstuetzung PolygonOffset
|*
\************************************************************************/

enum Base3DPolygonOffset
{
    Base3DPolygonOffsetFill = 1,
    Base3DPolygonOffsetLine,
    Base3DPolygonOffsetPoint
};

/*************************************************************************
|*
|* Genauigkeit der Berechnungen; im Zweifelsfall wird dieser
|* Wert benutzt, um mit 0.0 zu vergleichen oder Abstaende von
|* Randvereichen von Wertebereichen zu testen
|*
\************************************************************************/

#define SMALL_DVALUE                    (0.0000001)

/*************************************************************************
|*
|* Die Basisklasse fuer alle 3D Ausgaben
|*
\************************************************************************/

class Base3D : public OutDev3D
{
private:
    // das OutputDevice, das Ziel dieser Base3D Engine ist. Wird bereits
    // im Konstruktor uebergeben und kann nicht veraendert werden.
    OutputDevice*           pDevice;

    // ObjectCreationMode
    Base3DObjectMode        eObjectMode;

    // current color and alpha blending
    Color                   aCurrentColor;

    // Komplexe Polygone
    B3dComplexPolygon       aComplexPolygon;

    // RenderMode
    Base3DRenderMode        eRenderModeFront;
    Base3DRenderMode        eRenderModeBack;

    // ShadeMode fuer Polygone
    Base3DShadeModel        eShadeModel;

    // CullingMode
    Base3DCullMode          eCullMode;

    // PointSize, LineWidth
    double                  fPointSize;
    double                  fLineWidth;

    // Scissor Region
    Rectangle               aScissorRectangle;

    // Texture store
    B3dTexture*             pActiveTexture;

    // Aktuelles TransformationSet
    B3dTransformationSet*   pTransformationSet;

    // Aktuelle Beleuchtung
    B3dLightGroup*          pLightGroup;

    // Materialien
    B3dMaterial             aMaterialFront;
    B3dMaterial             aMaterialBack;

    // Darstellungsqualitaet (0..255)
    UINT8                   nDisplayQuality;

     // #i71618#
     double                 mfPolygonOffset;

    // EdgeFlag
    unsigned                bEdgeFlag                   : 1;

    // Gueltigkeit des Kontextes, wird in
    // den Konstruktoren gesetzt
    unsigned                bContextIsValid             : 1;

    // Variablen fuer PolygonOffset
    unsigned                bPolyOffsetFill             : 1;
    unsigned                bPolyOffsetLine             : 1;
    unsigned                bPolyOffsetPoint            : 1;

    // Scissor-Region gesetzt?
    unsigned                bScissorRegionActive        : 1;

    // Dithern?
    unsigned                bDitherActive               : 1;

    // Hint auf transparente Anteile in der Szene
    unsigned                bTransparentParts           : 1;

    // FlatMode force switch
    unsigned                bForceFlat                  : 1;

protected:
    // Konstruktor, Destruktor
    Base3D(OutputDevice* pOutDev);
    virtual ~Base3D();

    // setze das ContextIsValid - Flag
    void SetContextIsValid(BOOL bNew=TRUE)
        { bContextIsValid=bNew; }

    static Base3D* CreateScreenRenderer(OutputDevice* pDev);

public:
    // Typbestimmung
    virtual UINT16 GetBase3DType()=0;

    // Zugriff auf das zu verwendende OutputDevice
    OutputDevice* GetOutputDevice() const { return pDevice; }

    // Hint auf transparente Anteile in der Szene setzen/lesen
    void SetTransparentPartsContainedHint(BOOL bNew) { bTransparentParts = bNew; }
    BOOL GetTransparentPartsContainedHint() const { return bTransparentParts; }

    // Zugriff auf globale Daten von allen Base3D's
private:
    B3dGlobalData& GetGlobalData();
public:

    // Erzeugen bzw. Zerstoeren eines Base3D Kontextes
    static Base3D* Create(OutputDevice* pOutDev, BOOL bForcePrinter=FALSE);
    virtual void Destroy(OutputDevice *pOutDev);

    // Gueltigkeit des Kontextes nach der Inkarnation
    // ueberpruefen
    BOOL IsContextValid()
        { return bContextIsValid; }
#ifdef DBG_UTIL
    void SetContextValid(BOOL bNew=TRUE) { bContextIsValid = bNew; }
#endif

    // TransformationSet setzen/lesen
    virtual void SetTransformationSet(B3dTransformationSet* pSet);
    B3dTransformationSet* GetTransformationSet() { return pTransformationSet; }

    // Beleuchtung setzen/lesen
    virtual void SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal=TRUE);
    B3dLightGroup* GetLightGroup() { return pLightGroup; }

    // Szenenverwaltung
    virtual void StartScene() = 0;
    virtual void EndScene() = 0;

    // Scissoring
    virtual void SetScissorRegionPixel(const Rectangle& rRect, BOOL bActivate=TRUE);
    void SetScissorRegion(const Rectangle& rRect, BOOL bActivate=TRUE);
    virtual void ActivateScissorRegion(BOOL bNew);
    BOOL IsScissorRegionActive() { return bScissorRegionActive; }
    const Rectangle& GetScissorRegionPixel() { return aScissorRectangle; }

    // Dithering aktiv?
    BOOL GetDither() const { return bDitherActive; }
    virtual void SetDither(BOOL bNew);

    // Dithering aktiv?
    BOOL GetForceFlat() const { return bForceFlat; }
    void SetForceFlat(BOOL bNew) { bForceFlat = bNew; }

    // Attribute Color (und Alpha)
    virtual void SetColor(Color aNew);
    virtual Color GetColor();

    // Materials
    virtual void SetMaterial(Color aNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    Color GetMaterial(Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DMaterialMode=Base3DMaterialFrontAndBack) const;
    virtual void SetShininess(UINT16 nExponent,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    UINT16 GetShininess(Base3DMaterialMode=Base3DMaterialFrontAndBack) const;
    void ResetMaterial(Base3DMaterialMode=Base3DMaterialFrontAndBack);

    // Texturen
    B3dTexture* ObtainTexture(TextureAttributes& rAtt);
    B3dTexture* ObtainTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx);
    void DeleteTexture(TextureAttributes& rAtt);
    void DeleteAllTextures();
private:
    virtual B3dTexture* CreateTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx);
protected:
    virtual void DestroyTexture(B3dTexture*);
public:
    virtual void SetActiveTexture(B3dTexture* pTex=NULL);
    B3dTexture* GetActiveTexture() { return pActiveTexture; }
    BOOL IsTextureActive() const { return (pActiveTexture != NULL) ? TRUE : FALSE; }

    // Darstellungsqualitaet
    virtual void SetDisplayQuality(UINT8 nNew);
    UINT8 GetDisplayQuality() const;

    // PolygonOffset
    virtual void SetPolygonOffset(
        Base3DPolygonOffset eNew=Base3DPolygonOffsetLine, BOOL bNew=FALSE);
    BOOL GetPolygonOffset(Base3DPolygonOffset eNew=Base3DPolygonOffsetLine) const;

     // #i71618#
     // support for value for PolygonOffset. Value is relative to Z-Buffer depth. To keep
     // it implementation-independent, it is a value in the range [0.0 .. 1.0]. Default
     // is now 0,005 which expands to ca. 80.000 with 14bit Z-Buffer
     void setPolygonOffset(double fNew);
     double getPolygonOffset() const;

    // RenderMode
    virtual void SetRenderMode(Base3DRenderMode eNew,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    Base3DRenderMode GetRenderMode(
        Base3DMaterialMode=Base3DMaterialFrontAndBack) const;

    // ShadeModel
    virtual void SetShadeModel(Base3DShadeModel eNew);
    Base3DShadeModel GetShadeModel() const;

    // CullingMode
    virtual void SetCullMode(Base3DCullMode eNew);
    Base3DCullMode GetCullMode() const;

    // EdgeFlagMode
    BOOL GetEdgeFlag() const;
    virtual void SetEdgeFlag(BOOL bNew=TRUE);

    // PointSize, LineWidth
    double GetPointSize() const;
    virtual void SetPointSize(double fNew=1.0);
    double GetLineWidth() const;
    virtual void SetLineWidth(double fNew=1.0);

    // geometrische Objekte hinzufuegen
    // Modus setzen
    Base3DObjectMode GetObjectMode() const;

    // geometrische Daten
    void AddVertex(basegfx::B3DPoint& rVertex);
    void AddVertex(basegfx::B3DPoint& rVertex, basegfx::B3DVector& rNormal);
    void AddVertex(basegfx::B3DPoint& rVertex, basegfx::B3DVector& rNormal, basegfx::B2DPoint& rTexPos);
    void AddVertex(B3dEntity& rEnt);
    void PostAddVertex(B3dEntity& rEnt);

    // Geometrische Daten uebergeben
    virtual B3dEntity& GetFreeEntity();

    // Primitiv beginnen/abschliessen
    void StartPrimitive(Base3DObjectMode eMode);
    void EndPrimitive();

    // Ein Objekt in Form einer B3dGeometry direkt ausgeben
    virtual void DrawPolygonGeometry(const B3dGeometry& rGeometry, sal_Bool bOutline=FALSE);

#ifdef DBG_UTIL
    // Testfunktion um gezielte 3D-Ausgaben zu testen
    void Test(const Rectangle& rBound, const Rectangle& rVisible);
#endif

protected:
    // geometrische Daten uebergeben
    virtual void ImplPostAddVertex(B3dEntity& rEnt) = 0;

    // Direkter Zugriff auf B3dMaterial
    // fuer abgeleitete Klassen
    B3dMaterial& GetMaterialObject(Base3DMaterialMode=Base3DMaterialFront);

    // Primitiv beginnen/abschliessen
    virtual void ImplStartPrimitive() = 0;
    virtual void ImplEndPrimitive() = 0;

    // Geometrische Daten uebergeben
    virtual B3dEntity& ImplGetFreeEntity() = 0;

    // Funktionen, um mit hoeheren Primitiven umzugehen
    void HandleComplexPolygon();
};

#endif          // _B3D_BASE3D_HXX
