/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _B3D_BASE3D_HXX
#define _B3D_BASE3D_HXX

#include "hmatrix.hxx"

#ifndef _INC_FLOAT
#include <float.h>
#endif

#include <vcl/timer.hxx>

#include <osl/mutex.hxx>

// Vorausdeklarationen
//class B3dTexture;
//class OutputDevice;
//class Bitmap;
//class BitmapEx;
//class TextureAttributes;
//class B3dTransformationSet;

namespace binfilter {

/*************************************************************************
|*
|* Texturenverwaltung
|*
\************************************************************************/

//SV_DECL_PTRARR_DEL(B3dTextureStore, B3dTexture*, 0, 4);

/*************************************************************************
|*
|* Globale Daten fuer Base3D
|*
\************************************************************************/

//class B3dGlobalData
//{
//private:
//	// Texturverwaltung
//	Container				maTextureStore;
//	AutoTimer				maTimer;
//	osl::Mutex				maMutex;
//
//	// link for timer
//    DECL_LINK(TimerHdl, AutoTimer*);
//
//public:
//	B3dGlobalData();
//	virtual ~B3dGlobalData();
//
//	B3dTexture* ObtainTexture(TextureAttributes& rAtt);
//	void InsertTexture(B3dTexture* pNew);
//	void DeleteTexture(B3dTexture* pOld);
//	void DeleteAllTextures();
//};

/*************************************************************************
|*
|* Bisher vorhandene Renderer
|*
\************************************************************************/

//#define	BASE3D_TYPE_DEFAULT			0x0000
//#define	BASE3D_TYPE_OPENGL			0x0001
//#define	BASE3D_TYPE_PRINTER			0x0002

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

#define	SMALL_DVALUE					(0.0000001)

/*************************************************************************
|*
|* Die Basisklasse fuer alle 3D Ausgaben
|*
\************************************************************************/

//class Base3D : public OutDev3D
//{
//private:
//	// das OutputDevice, das Ziel dieser Base3D Engine ist. Wird bereits
//	// im Konstruktor uebergeben und kann nicht veraendert werden.
//	OutputDevice*			pDevice;
//
//	// ObjectCreationMode
//	Base3DObjectMode		eObjectMode;
//
//	// current color and alpha blending
//	Color					aCurrentColor;
//
//	// Komplexe Polygone
//	B3dComplexPolygon		aComplexPolygon;
//
//	// RenderMode
//	Base3DRenderMode		eRenderModeFront;
//	Base3DRenderMode		eRenderModeBack;
//
//	// ShadeMode fuer Polygone
//	Base3DShadeModel		eShadeModel;
//
//	// CullingMode
//	Base3DCullMode			eCullMode;
//
//	// PointSize, LineWidth
//	double					fPointSize;
//	double					fLineWidth;
//
//	// Scissor Region
//	Rectangle				aScissorRectangle;
//
//	// Texture store
//	B3dTexture*				pActiveTexture;
//
//	// Aktuelles TransformationSet
//	B3dTransformationSet*	pTransformationSet;
//
//	// Aktuelle Beleuchtung
//	B3dLightGroup*			pLightGroup;
//
//	// Materialien
//	B3dMaterial				aMaterialFront;
//	B3dMaterial				aMaterialBack;
//
//	// Darstellungsqualitaet (0..255)
//	UINT8					nDisplayQuality;
//
//	// EdgeFlag
//	unsigned				bEdgeFlag					: 1;
//
//	// Gueltigkeit des Kontextes, wird in
//	// den Konstruktoren gesetzt
//	unsigned				bContextIsValid				: 1;
//
//	// Variablen fuer PolygonOffset
//	unsigned				bPolyOffsetFill				: 1;
//	unsigned				bPolyOffsetLine				: 1;
//	unsigned				bPolyOffsetPoint			: 1;
//
//	// Scissor-Region gesetzt?
//	unsigned				bScissorRegionActive		: 1;
//
//	// Dithern?
//	unsigned				bDitherActive				: 1;
//
//	// Hint auf transparente Anteile in der Szene
//	unsigned				bTransparentParts			: 1;
//
//	// FlatMode force switch
//	unsigned				bForceFlat					: 1;
//
//protected:
//	// Konstruktor, Destruktor
//	Base3D(OutputDevice* pOutDev);
//	virtual ~Base3D();
//
//	// setze das ContextIsValid - Flag
//	void SetContextIsValid(BOOL bNew=TRUE)
//		{ bContextIsValid=bNew; }
//
//	static Base3D* CreateScreenRenderer(OutputDevice* pDev);
//
//public:
//	// Typbestimmung
//	virtual UINT16 GetBase3DType()=0;
//
//	// Zugriff auf das zu verwendende OutputDevice
//	OutputDevice* GetOutputDevice()
//		{ return pDevice; }
//
//	// Hint auf transparente Anteile in der Szene setzen/lesen
//	void SetTransparentPartsContainedHint(BOOL bNew) { bTransparentParts = bNew; }
//	BOOL GetTransparentPartsContainedHint() { return bTransparentParts; }
//
//	// Zugriff auf globale Daten von allen Base3D's
//private:
//	B3dGlobalData& GetGlobalData();
//public:
//
//	// Erzeugen bzw. Zerstoeren eines Base3D Kontextes
//	static Base3D* Create(OutputDevice* pOutDev, BOOL bForcePrinter=FALSE);
//	virtual void Destroy(OutputDevice *pOutDev);
//
//	// Gueltigkeit des Kontextes nach der Inkarnation
//	// ueberpruefen
//	BOOL IsContextValid()
//		{ return bContextIsValid; }
//#ifdef DBG_UTIL
//	void SetContextValid(BOOL bNew=TRUE) { bContextIsValid = bNew; }
//#endif
//
//	// TransformationSet setzen/lesen
//	virtual void SetTransformationSet(B3dTransformationSet* pSet);
//	B3dTransformationSet* GetTransformationSet() { return pTransformationSet; }
//
//	// Beleuchtung setzen/lesen
//	virtual void SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal=TRUE);
//	B3dLightGroup* GetLightGroup() { return pLightGroup; }
//
//	// Szenenverwaltung
//	virtual void StartScene() = 0;
//	virtual void EndScene() = 0;
//
//	// Scissoring
//	virtual void SetScissorRegionPixel(const Rectangle& rRect, BOOL bActivate=TRUE);
//	void SetScissorRegion(const Rectangle& rRect, BOOL bActivate=TRUE);
//	virtual void ActivateScissorRegion(BOOL bNew);
//	BOOL IsScissorRegionActive() { return bScissorRegionActive; }
//	const Rectangle& GetScissorRegionPixel() { return aScissorRectangle; }
//
//	// Dithering aktiv?
//	BOOL GetDither() { return bDitherActive; }
//	virtual void SetDither(BOOL bNew);
//
//	// Dithering aktiv?
//	BOOL GetForceFlat() { return bForceFlat; }
//	void SetForceFlat(BOOL bNew) { bForceFlat = bNew; }
//
//	// Attribute Color (und Alpha)
//	virtual void SetColor(Color aNew);
//	virtual Color GetColor();
//
//	// Materials
//	virtual void SetMaterial(Color aNew,
//		Base3DMaterialValue=Base3DMaterialAmbient,
//		Base3DMaterialMode=Base3DMaterialFrontAndBack);
//	Color GetMaterial(Base3DMaterialValue=Base3DMaterialAmbient,
//		Base3DMaterialMode=Base3DMaterialFrontAndBack);
//	virtual void SetShininess(UINT16 nExponent,
//		Base3DMaterialMode=Base3DMaterialFrontAndBack);
//	UINT16 GetShininess(Base3DMaterialMode=Base3DMaterialFrontAndBack);
//	void ResetMaterial(Base3DMaterialMode=Base3DMaterialFrontAndBack);
//
//	// Texturen
//	B3dTexture* ObtainTexture(TextureAttributes& rAtt);
//	B3dTexture* ObtainTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx);
//	void DeleteTexture(TextureAttributes& rAtt);
//	void DeleteAllTextures();
//private:
//	virtual B3dTexture* CreateTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx);
//protected:
//	virtual void DestroyTexture(B3dTexture*);
//public:
//	virtual void SetActiveTexture(B3dTexture* pTex=NULL);
//	B3dTexture* GetActiveTexture() { return pActiveTexture; }
//	BOOL IsTextureActive() { return (pActiveTexture != NULL) ? TRUE : FALSE; }
//
//	// Darstellungsqualitaet
//	virtual void SetDisplayQuality(UINT8 nNew);
//	UINT8 GetDisplayQuality();
//
//	// PolygonOffset
//	virtual void SetPolygonOffset(
//		Base3DPolygonOffset eNew=Base3DPolygonOffsetLine, BOOL bNew=FALSE);
//	BOOL GetPolygonOffset(Base3DPolygonOffset eNew=Base3DPolygonOffsetLine);
//
//	// RenderMode
//	virtual void SetRenderMode(Base3DRenderMode eNew,
//		Base3DMaterialMode=Base3DMaterialFrontAndBack);
//	Base3DRenderMode GetRenderMode(
//		Base3DMaterialMode=Base3DMaterialFrontAndBack);
//
//	// ShadeModel
//	virtual void SetShadeModel(Base3DShadeModel eNew);
//	Base3DShadeModel GetShadeModel();
//
//	// CullingMode
//	virtual void SetCullMode(Base3DCullMode eNew);
//	Base3DCullMode GetCullMode();
//
//	// EdgeFlagMode
//	BOOL GetEdgeFlag();
//	virtual void SetEdgeFlag(BOOL bNew=TRUE);
//
//	// PointSize, LineWidth
//	double GetPointSize();
//	virtual void SetPointSize(double fNew=1.0);
//	double GetLineWidth();
//	virtual void SetLineWidth(double fNew=1.0);
//
//	// geometrische Objekte hinzufuegen
//	// Modus setzen
//	virtual Base3DObjectMode GetObjectMode();
//
//	// geometrische Daten
//	void AddVertex(Vector3D& rVertex);
//	void AddVertex(Vector3D& rVertex,
//		Vector3D& rNormal);
//	void AddVertex(Vector3D& rVertex,
//		Vector3D& rNormal, Vector3D& rTexPos);
//	void AddVertex(B3dEntity& rEnt);
//	void PostAddVertex(B3dEntity& rEnt);
//
//	// Geometrische Daten uebergeben
//	virtual B3dEntity& GetFreeEntity();
//
//	// Primitiv beginnen/abschliessen
//	void StartPrimitive(Base3DObjectMode eMode);
//	void EndPrimitive();
//
//	// Ein Objekt in Form einer B3dGeometry direkt ausgeben
//	virtual void DrawPolygonGeometry(B3dGeometry& rGeometry, BOOL bOutline=FALSE);
//
//#ifdef DBG_UTIL
//	// Testfunktion um gezielte 3D-Ausgaben zu testen
//	void Test(const Rectangle& rBound, const Rectangle& rVisible);
//#endif
//
//protected:
//	// geometrische Daten uebergeben
//	virtual void ImplPostAddVertex(B3dEntity& rEnt) = 0;
//
//	// Direkter Zugriff auf B3dMaterial
//	// fuer abgeleitete Klassen
//	B3dMaterial& GetMaterialObject(Base3DMaterialMode=Base3DMaterialFront);
//
//	// Primitiv beginnen/abschliessen
//	virtual void ImplStartPrimitive() = 0;
//	virtual void ImplEndPrimitive() = 0;
//
//	// Geometrische Daten uebergeben
//	virtual B3dEntity& ImplGetFreeEntity() = 0;
//
//	// Funktionen, um mit hoeheren Primitiven umzugehen
//	void HandleComplexPolygon();
//};

}//end of namespace binfilter

#endif          // _B3D_BASE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
