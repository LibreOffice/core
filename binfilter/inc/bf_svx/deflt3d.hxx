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

#ifndef _E3D_DEFLT3D_HXX
#define _E3D_DEFLT3D_HXX




#include <bf_goodies/base3d.hxx>
namespace binfilter {

class SfxItemSet;

/*************************************************************************
|*
|* Klasse zum verwalten der 3D-Default Attribute
|*
\************************************************************************/

class E3dDefaultAttributes
{
private:
    // Compound-Objekt
//-/	B3dMaterial			aDefaultFrontMaterial;
    Color				aDefaultAmbientColor;

//	B3dMaterial			aDefaultBackMaterial;
//-/	Base3DTextureKind	eDefaultTextureKind;
//-/	Base3DTextureMode	eDefaultTextureMode;
//-/	BOOL				bDefaultDoubleSided						;
    BOOL				bDefaultCreateNormals					;
    BOOL				bDefaultCreateTexture					;
//-/	BOOL				bDefaultUseStdNormals					;
//-/	BOOL				bDefaultUseStdNormalsUseSphere			;
//-/	BOOL				bDefaultInvertNormals					;
//-/	BOOL				bDefaultUseStdTextureX					;
//-/	BOOL				bDefaultUseStdTextureXUseSphere			;
//-/	BOOL				bDefaultUseStdTextureY					;
//-/	BOOL				bDefaultUseStdTextureYUseSphere			;
//-/	BOOL				bDefaultShadow3D						;
//-/	BOOL				bDefaultFilterTexture					;
    BOOL				bDefaultUseDifferentBackMaterial		;

    // Cube-Objekt
    Vector3D			aDefaultCubePos;
    Vector3D			aDefaultCubeSize;
    UINT16				nDefaultCubeSideFlags;
    BOOL				bDefaultCubePosIsCenter					;

    // Sphere-Objekt
//-/	long				nDefaultHSegments;
//-/	long				nDefaultVSegments;
    Vector3D			aDefaultSphereCenter;
    Vector3D			aDefaultSphereSize;

    // Lathe-Objekt
    long				nDefaultLatheEndAngle;
    double				fDefaultLatheScale;
//-/	double				fDefaultBackScale;
//-/	double				fDefaultPercentDiag;
    BOOL				bDefaultLatheSmoothed					;
    BOOL				bDefaultLatheSmoothFrontBack			;
    BOOL				bDefaultLatheCharacterMode				;
    BOOL				bDefaultLatheCloseFront;
    BOOL				bDefaultLatheCloseBack;

    // Extrude-Objekt
    double				fDefaultExtrudeScale;
//-/	double				fDefaultExtrudeDepth;
    BOOL				bDefaultExtrudeSmoothed					;
    BOOL				bDefaultExtrudeSmoothFrontBack			;
    BOOL				bDefaultExtrudeCharacterMode			;
    BOOL				bDefaultExtrudeCloseFront;
    BOOL				bDefaultExtrudeCloseBack;

    // Scene-Objekt
//-/	B3dLightGroup		aDefaultLightGroup;
//-/	Vector3D			aDefaultShadowPlaneDirection;
//-/	Base3DShadeModel	eDefaultShadeModel;
    BOOL				bDefaultDither							;
//-/	BOOL				bDefaultForceDraftShadeModel			;

public:
    // Konstruktor
    E3dDefaultAttributes();

    // Defaults zuruecksetzen
    void Reset();

    // Getter/Setter fuer Default-Werte aller 3D-Objekte
    // Compound-Objekt
//-/	const B3dMaterial& GetDefaultFrontMaterial() { return aDefaultFrontMaterial; }
//-/	void SetDefaultFrontMaterial(const B3dMaterial& rNew) { aDefaultFrontMaterial = rNew; }
    const Color& GetDefaultAmbientColor() const { return aDefaultAmbientColor; }
    void SetDefaultAmbientColor(const Color& rNew) { aDefaultAmbientColor = rNew; }

//	const B3dMaterial& GetDefaultBackMaterial() { return aDefaultBackMaterial; }
//	void SetDefaultBackMaterial(const B3dMaterial& rNew) { aDefaultBackMaterial = rNew; }
//-/	const Base3DTextureKind GetDefaultTextureKind() { return eDefaultTextureKind; }
//-/	void SetDefaultTextureKind(const Base3DTextureKind eNew) { eDefaultTextureKind = eNew; }
//-/	const Base3DTextureMode GetDefaultTextureMode() { return eDefaultTextureMode; }
//-/	void SetDefaultTextureMode(const Base3DTextureMode eNew) { eDefaultTextureMode = eNew; }
//-/	const BOOL GetDefaultDoubleSided() { return bDefaultDoubleSided; }
//-/	void SetDefaultDoubleSided(const BOOL bNew) { bDefaultDoubleSided = bNew; }
    BOOL GetDefaultCreateNormals() const { return bDefaultCreateNormals; }
    void SetDefaultCreateNormals(const BOOL bNew) { bDefaultCreateNormals = bNew; }
    BOOL GetDefaultCreateTexture() const { return bDefaultCreateTexture; }
    void SetDefaultCreateTexture(const BOOL bNew) { bDefaultCreateTexture = bNew; }
//-/	const BOOL GetDefaultUseStdNormals() { return bDefaultUseStdNormals; }
//-/	void SetDefaultUseStdNormals(const BOOL bNew) { bDefaultUseStdNormals = bNew; }
//-/	const BOOL GetDefaultUseStdNormalsUseSphere() { return bDefaultUseStdNormalsUseSphere; }
//-/	void SetDefaultUseStdNormalsUseSphere(const BOOL bNew) { bDefaultUseStdNormalsUseSphere = bNew; }
//-/	const BOOL GetDefaultInvertNormals() { return bDefaultInvertNormals; }
//-/	void SetDefaultInvertNormals(const BOOL bNew) { bDefaultInvertNormals = bNew; }
//-/	const BOOL GetDefaultUseStdTextureX() { return bDefaultUseStdTextureX; }
//-/	void SetDefaultUseStdTextureX(const BOOL bNew) { bDefaultUseStdTextureX = bNew; }
//-/	const BOOL GetDefaultUseStdTextureXUseSphere() { return bDefaultUseStdTextureXUseSphere; }
//-/	void SetDefaultUseStdTextureXUseSphere(const BOOL bNew) { bDefaultUseStdTextureXUseSphere = bNew; }
//-/	const BOOL GetDefaultUseStdTextureY() { return bDefaultUseStdTextureY; }
//-/	void SetDefaultUseStdTextureY(const BOOL bNew) { bDefaultUseStdTextureY = bNew; }
//-/	const BOOL GetDefaultUseStdTextureYUseSphere() { return bDefaultUseStdTextureYUseSphere; }
//-/	void SetDefaultUseStdTextureYUseSphere(const BOOL bNew) { bDefaultUseStdTextureYUseSphere = bNew; }
//-/	const BOOL GetDefaultShadow3D() { return bDefaultShadow3D; }
//-/	void SetDefaultShadow3D(const BOOL bNew) { bDefaultShadow3D = bNew; }
//-/	const BOOL GetDefaultFilterTexture() { return bDefaultFilterTexture; }
//-/	void SetDefaultFilterTexture(const BOOL bNew) { bDefaultFilterTexture = bNew; }
    BOOL GetDefaultUseDifferentBackMaterial() const { return bDefaultUseDifferentBackMaterial; }
    void SetDefaultUseDifferentBackMaterial(const BOOL bNew) { bDefaultUseDifferentBackMaterial = bNew; }

    // Cube-Objekt
    const Vector3D& GetDefaultCubePos() const { return aDefaultCubePos; }
    void SetDefaultCubePos(const Vector3D& rNew) { aDefaultCubePos = rNew; }
    const Vector3D& GetDefaultCubeSize() const { return aDefaultCubeSize; }
    void SetDefaultCubeSize(const Vector3D& rNew) { aDefaultCubeSize = rNew; }
    UINT16 GetDefaultCubeSideFlags() const { return nDefaultCubeSideFlags; }
    void SetDefaultCubeSideFlags(const UINT16 nNew) { nDefaultCubeSideFlags = nNew; }
    BOOL GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }
    void SetDefaultCubePosIsCenter(const BOOL bNew) { bDefaultCubePosIsCenter = bNew; }

    // Sphere-Objekt
//-/	const long GetDefaultHSegments() { return nDefaultHSegments; }
//-/	void SetDefaultHSegments(const long nNew) { nDefaultHSegments = nNew; }
//-/	const long GetDefaultVSegments() { return nDefaultVSegments; }
//-/	void SetDefaultVSegments(const long nNew) { nDefaultVSegments = nNew; }
    const Vector3D& GetDefaultSphereCenter() const { return aDefaultSphereCenter; }
    void SetDefaultSphereCenter(const Vector3D& rNew) { aDefaultSphereCenter = rNew; }
    const Vector3D& GetDefaultSphereSize() const { return aDefaultSphereSize; }
    void SetDefaultSphereSize(const Vector3D& rNew) { aDefaultSphereSize = rNew; }

    // Lathe-Objekt
    long GetDefaultLatheEndAngle() const { return nDefaultLatheEndAngle; }
    void SetDefaultLatheEndAngle(const long nNew) { nDefaultLatheEndAngle = nNew; }
    double GetDefaultLatheScale() const { return fDefaultLatheScale; }
    void SetDefaultLatheScale(const double fNew) { fDefaultLatheScale = fNew; }
//-/	const double GetDefaultBackScale() { return fDefaultBackScale; }
//-/	void SetDefaultBackScale(const double fNew) { fDefaultBackScale = fNew; }
//-/	const double GetDefaultPercentDiag() { return fDefaultPercentDiag; }
//-/	void SetDefaultPercentDiag(const double fNew) { fDefaultPercentDiag = fNew; }
    BOOL GetDefaultLatheSmoothed() const { return bDefaultLatheSmoothed; }
    void SetDefaultLatheSmoothed(const BOOL bNew) { bDefaultLatheSmoothed = bNew; }
    BOOL GetDefaultLatheSmoothFrontBack() const { return bDefaultLatheSmoothFrontBack; }
    void SetDefaultLatheSmoothFrontBack(const BOOL bNew) { bDefaultLatheSmoothFrontBack = bNew; }
    BOOL GetDefaultLatheCharacterMode() const { return bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const BOOL bNew) { bDefaultLatheCharacterMode = bNew; }
    BOOL GetDefaultLatheCloseFront() const { return bDefaultLatheCloseFront; }
    void SetDefaultLatheCloseFront(const BOOL bNew) { bDefaultLatheCloseFront = bNew; }
    BOOL GetDefaultLatheCloseBack() const { return bDefaultLatheCloseBack; }
    void SetDefaultLatheCloseBack(const BOOL bNew) { bDefaultLatheCloseBack = bNew; }

    // Extrude-Objekt
    double GetDefaultExtrudeScale() const { return fDefaultExtrudeScale; }
    void SetDefaultExtrudeScale(const double fNew) { fDefaultExtrudeScale = fNew; }
//-/	const double GetDefaultExtrudeDepth() { return fDefaultExtrudeDepth; }
//-/	void SetDefaultExtrudeDepth(const double fNew) { fDefaultExtrudeDepth = fNew; }
    BOOL GetDefaultExtrudeSmoothed() const { return bDefaultExtrudeSmoothed; }
    void SetDefaultExtrudeSmoothed(const BOOL bNew) { bDefaultExtrudeSmoothed = bNew; }
    BOOL GetDefaultExtrudeSmoothFrontBack() const { return bDefaultExtrudeSmoothFrontBack; }
    void SetDefaultExtrudeSmoothFrontBack(const BOOL bNew) { bDefaultExtrudeSmoothFrontBack = bNew; }
    BOOL GetDefaultExtrudeCharacterMode() const { return bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const BOOL bNew) { bDefaultExtrudeCharacterMode = bNew; }
    BOOL GetDefaultExtrudeCloseFront() const { return bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const BOOL bNew) { bDefaultExtrudeCloseFront = bNew; }
    BOOL GetDefaultExtrudeCloseBack() const { return bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const BOOL bNew) { bDefaultExtrudeCloseBack = bNew; }

    // Scene-Objekt
//-/	const B3dLightGroup& GetDefaultLightGroup() { return aDefaultLightGroup; }
//-/	void SetDefaultLightGroup(const B3dLightGroup& rNew) { aDefaultLightGroup = rNew; }
//-/	const Vector3D& GetDefaultShadowPlaneDirection() { return aDefaultShadowPlaneDirection; }
//-/	void SetDefaultShadowPlaneDirection(const Vector3D& rNew) { aDefaultShadowPlaneDirection = rNew; }
//-/	const Base3DShadeModel GetDefaultShadeModel() { return eDefaultShadeModel; }
//-/	void SetDefaultShadeModel(const Base3DShadeModel eNew) { eDefaultShadeModel = eNew; }
    BOOL GetDefaultDither() const { return bDefaultDither; }
    void SetDefaultDither(const BOOL bNew) { bDefaultDither = bNew; }
//-/	const BOOL GetDefaultForceDraftShadeModel() { return bDefaultForceDraftShadeModel; }
//-/	void SetDefaultForceDraftShadeModel(const BOOL bNew) { bDefaultForceDraftShadeModel = bNew; }

    // Default-Attribute setzen/lesen
//-/	void SetDefaultValues(const SfxItemSet& rAttr);
//-/	void TakeDefaultValues(SfxItemSet& rAttr);
//-/
//-/protected:
};

}//end of namespace binfilter
#endif			// _E3D_DEFLT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
