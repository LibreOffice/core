/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dopngl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 16:08:13 $
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

#ifndef _B3D_B3DOPNGL_HXX
#define _B3D_B3DOPNGL_HXX

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _SV_OPENGL_HXX
#include <vcl/opengl.hxx>
#endif

/*************************************************************************
|*
|* Die Basisklasse fuer Standard 3D Ausgaben mittels OpenGL unter
|* Windows (Win95 und Windows NT)
|*
\************************************************************************/

class Base3DOpenGL : public Base3D
{
private:
    // Datenuebergabe
    B3dEntity                   aEntity;

    // OpenGL Objekt
    OpenGL                      aOpenGL;

    // Letzte Normale und letzte Texturkoordinate
    basegfx::B3DVector      aLastNormal;
    basegfx::B2DPoint           aLastTexCoor;

    float                       fOffFacMul100;
    float                       fOffUniMul100;

    // Simulation Phong-Mode
    sal_Bool                    bPhongBufferedMode;
    B3dEntityBucket             aPhongBuffer;
    long                        nPhongDivideSize;
    long                        nInternPhongDivideSize;

    // flags (#70626#)
    sal_Bool                    bForceToSinglePrimitiveOutput;

    void DrawPhongPrimitives();
    void DrawPhongTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3);
    void CalcInternPhongDivideSize();

public:
    Base3DOpenGL(OutputDevice* pOutDev);

    // DivideSize bei Phong-Simulation
    long GetPhongDivideSize() { return nPhongDivideSize; }
    void SetPhongDivideSize(long nNew);

    // Get/Set force to single primitive output (#70626#)
    sal_Bool IsForceToSinglePrimitiveOutput() const { return bForceToSinglePrimitiveOutput; }
    void SetForceToSinglePrimitiveOutput(sal_Bool bNew) { bForceToSinglePrimitiveOutput = bNew; }

    // Typbestimmung
    virtual sal_uInt16 GetBase3DType();

    virtual void StartScene();
    virtual void EndScene();

    // Scissoring
    virtual void SetScissorRegionPixel(const Rectangle& rRect, sal_Bool bActivate=sal_True);
    virtual void ActivateScissorRegion(sal_Bool bNew);

    // Dithering
    virtual void SetDither(sal_Bool bNew);

    // Farbe
    virtual void SetColor(Color aNew);

    // Material
    virtual void SetMaterial(Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    virtual void SetShininess(sal_uInt16 nExponent,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);

    // Texturen
private:
    virtual B3dTexture* CreateTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx);
    virtual void DestroyTexture(B3dTexture*);
public:
    virtual void SetActiveTexture(B3dTexture* pTex=NULL);

    // Darstellungsqualitaet
    virtual void SetDisplayQuality(UINT8 nNew);

    // PolygonOffset
    virtual void SetPolygonOffset(
        Base3DPolygonOffset eNew=Base3DPolygonOffsetLine, sal_Bool bNew=sal_False);

    // Beleuchtung setzen/lesen
    virtual void SetLightGroup(B3dLightGroup* pSet, sal_Bool bSetGlobal=sal_True);

    virtual void SetRenderMode(Base3DRenderMode eNew,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    virtual void SetShadeModel(Base3DShadeModel eNew);
    virtual void SetCullMode(Base3DCullMode eNew);

    virtual void SetEdgeFlag(sal_Bool bNew=sal_True);
    virtual void SetPointSize(double fNew=1.0);
    virtual void SetLineWidth(double fNew=1.0);

    // Ein Objekt in Form einer B3dGeometry direkt ausgeben
    virtual void DrawPolygonGeometry(const B3dGeometry& rGeometry, sal_Bool bOutline=sal_False);

    // Callbacks bei Matrixaenderungen
    virtual void SetTransformationSet(B3dTransformationSet* pSet);

protected:
    // Geometrische Daten uebergeben
    virtual B3dEntity& ImplGetFreeEntity();
    virtual void ImplPostAddVertex(B3dEntity& rEnt);

    virtual void ImplStartPrimitive();
    virtual void ImplEndPrimitive();

    // Callbacks bei Matrixaenderungen
    void PostSetObjectOrientation(B3dTransformationSet* pCaller);
    void PostSetProjection(B3dTransformationSet* pCaller);
    void PostSetTexture(B3dTransformationSet* pCaller);
    void PostSetViewport(B3dTransformationSet* pCaller);

    // lokale Parameter des LightModels
    void SetGlobalAmbientLight(const Color rNew);
    void SetLocalViewer(sal_Bool bNew=sal_True);
    void SetModelTwoSide(sal_Bool bNew=sal_False);

    // Hauptschalter fuer die Beleuchtung
    void EnableLighting(sal_Bool bNew=sal_True);

    // Lichtquellen Interface
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    void SetPosition(const basegfx::B3DPoint& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetDirection(const basegfx::B3DVector& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotDirection(const basegfx::B3DVector& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotExponent(sal_uInt16 nNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotCutoff(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetConstantAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetLinearAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetQuadraticAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void Enable(sal_Bool bNew=sal_True,
        Base3DLightNumber=Base3DLight0);
};


#endif          // _B3D_B3DOPNGL_HXX
