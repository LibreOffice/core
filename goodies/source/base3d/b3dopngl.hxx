/*************************************************************************
 *
 *  $RCSfile: b3dopngl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    Vector3D                    aLastNormal;
    Vector3D                    aLastTexCoor;
    Vector3D                    aEmptyVector;

    float                       fOffFacMul100;
    float                       fOffUniMul100;

    // Simulation Phong-Mode
    BOOL                        bPhongBufferedMode;
    B3dEntityBucket             aPhongBuffer;
    long                        nPhongDivideSize;
    long                        nInternPhongDivideSize;

    // flags (#70626#)
    BOOL                        bForceToSinglePrimitiveOutput;

    void DrawPhongPrimitives();
    void DrawPhongTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3);
    void CalcInternPhongDivideSize();

public:
    Base3DOpenGL(OutputDevice* pOutDev);

    // DivideSize bei Phong-Simulation
    long GetPhongDivideSize() { return nPhongDivideSize; }
    void SetPhongDivideSize(long nNew);

    // Get/Set force to single primitive output (#70626#)
    BOOL IsForceToSinglePrimitiveOutput() const { return bForceToSinglePrimitiveOutput; }
    void SetForceToSinglePrimitiveOutput(BOOL bNew) { bForceToSinglePrimitiveOutput = bNew; }

    // Typbestimmung
    virtual UINT16 GetBase3DType();

    virtual void StartScene();
    virtual void EndScene();

    // Scissoring
    virtual void SetScissorRegionPixel(const Rectangle& rRect, BOOL bActivate=TRUE);
    virtual void ActivateScissorRegion(BOOL bNew);

    // Dithering
    virtual void SetDither(BOOL bNew);

    // Farbe
    virtual void SetColor(Color aNew);

    // Material
    virtual void SetMaterial(Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    virtual void SetShininess(UINT16 nExponent,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);

    // Texturen
private:
    virtual B3dTexture* CreateTexture(TextureAttributes& rAtt, Bitmap& rBitmap);
    virtual void DestroyTexture(B3dTexture*);
public:
    virtual void SetActiveTexture(B3dTexture* pTex=NULL);

    // Darstellungsqualitaet
    virtual void SetDisplayQuality(UINT8 nNew);

    // PolygonOffset
    virtual void SetPolygonOffset(
        Base3DPolygonOffset eNew=Base3DPolygonOffsetLine, BOOL bNew=FALSE);

    // Beleuchtung setzen/lesen
    virtual void SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal=TRUE);

    virtual void SetRenderMode(Base3DRenderMode eNew,
        Base3DMaterialMode=Base3DMaterialFrontAndBack);
    virtual void SetShadeModel(Base3DShadeModel eNew);
    virtual void SetCullMode(Base3DCullMode eNew);

    virtual void SetEdgeFlag(BOOL bNew=TRUE);
    virtual void SetPointSize(double fNew=1.0);
    virtual void SetLineWidth(double fNew=1.0);

    // Ein Objekt in Form einer B3dGeometry direkt ausgeben
    virtual void DrawPolygonGeometry(B3dGeometry& rGeometry, BOOL bOutline=FALSE);

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
    void SetLocalViewer(BOOL bNew=TRUE);
    void SetModelTwoSide(BOOL bNew=FALSE);

    // Hauptschalter fuer die Beleuchtung
    void EnableLighting(BOOL bNew=TRUE);

    // Lichtquellen Interface
    void SetIntensity(const Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient,
        Base3DLightNumber=Base3DLight0);
    void SetPosition(const Vector3D& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetDirection(const Vector3D& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotDirection(const Vector3D& rNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotExponent(UINT16 nNew,
        Base3DLightNumber=Base3DLight0);
    void SetSpotCutoff(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetConstantAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetLinearAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void SetQuadraticAttenuation(double fNew,
        Base3DLightNumber=Base3DLight0);
    void Enable(BOOL bNew=TRUE,
        Base3DLightNumber=Base3DLight0);
};


#endif          // _B3D_B3DOPNGL_HXX
