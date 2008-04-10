/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3ddeflt.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _B3D_B3DDEFLT_HXX
#define _B3D_B3DDEFLT_HXX

#include "b3dcommn.hxx"
#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/alpha.hxx>
#include <goodies/bxdintpo.hxx>

/*************************************************************************
|*
|* Die Basisklasse fuer Standard 3D Ausgaben auf StarView Basis
|*
\************************************************************************/

class Base3DDefault : public Base3DCommon
{
private:
    // ZBuffer auch als BitMap
    Bitmap                  aZBuffer;

    // Bitmap zum Zeichnen im Hintergrund
    Bitmap                  aPicture;

    // Bitmaps fuer die Transparenz
    Bitmap                  aMonoTransparence;
    AlphaMask               aAlphaTransparence;

    // Position der oberen linken Ecke aus ImplSetViewport
    Rectangle               aSizePixel;

    // Mit faktoren umgerechnete echte Bitmap-Groesse
    Rectangle               aLocalSizePixel;

    // ZBuffer loeschwert
    BitmapColor             aClearValue;

    // Zugriffe auf die BitMaps
    BitmapWriteAccess*      pZBufferWrite;
    BitmapWriteAccess*      pPictureWrite;
    BitmapWriteAccess*      pTransparenceWrite;

    // Vergroeberungsstufe und dazugehoerige Variablen
    double                  fDetail;
    double                  fDetailBackup;
    sal_Int32               nMaxPixels;

    // Groesse der Textur, falls benutzt
    double                  fTexWidth;
    double                  fTexHeight;

    // Punktkoodinaten fuer Primitive
    Point                   aOutPointTop;
    Point                   aOutPointLeft;
    Point                   aOutPointRight;

    // Zeiger auf die Ursprungsdaten
    B3dEntity*              pEntTop;
    B3dEntity*              pEntLeft;
    B3dEntity*              pEntRight;

    // benoetigte Interpolatoren
    BxdInterpolator         aIntXPosLeft;
    BxdInterpolator         aIntXPosRight;

    BxdInterpolator         aIntDepthLeft;
    BxdInterpolator         aIntDepthRight;
    BxdInterpolator         aIntDepthLine;

    // #96837#
    // Added interpolators for real z coordinate in eye coor. In these,
    // 1/z is interpolated to have a base for perspective corrected texturing.
    BxdInterpolator         aRealDepthLeft;
    BxdInterpolator         aRealDepthRight;
    BxdInterpolator         aRealDepthLine;

    BxdInterpolator         aIntTexSLeft;
    BxdInterpolator         aIntTexSRight;
    BxdInterpolator         aIntTexTLeft;
    BxdInterpolator         aIntTexTRight;
    BxdInterpolator         aIntTexSLine;
    BxdInterpolator         aIntTexTLine;
    B3dVectorInterpolator   aIntVectorLeft;
    B3dVectorInterpolator   aIntVectorRight;
    B3dVectorInterpolator   aIntVectorLine;
    BxdColorInterpolator    aIntColorLeft;
    BxdColorInterpolator    aIntColorRight;
    BxdColorInterpolator    aIntColorLine;

    // Lokale Scissor Region (ohne TopLeft)
    Rectangle               aDefaultScissorRectangle;

    // #i71618#
    // derived from mfPolygonOffset in StartScene() using Z-Buffer depth
    sal_uInt32              maPolygonOffset;

    // bitfileld
    // Booleans fuer Zeichenoperationen
    unsigned                bNormalsUsed                    : 1;
    unsigned                bTextureUsed                    : 1;

    // Bool fuer Detailreduzierung
    unsigned                bReducedDetail                  : 1;
    unsigned                bDetailBackedup                 : 1;

    // #96837#
    // Bool for enabling/disabling perspective texture correction
    unsigned                mbPTCorrection                  : 1;

    // Funktionen fuer erlangen/freigeben der BitmapAccesses
    void AcquireAccess();
    void ReleaseAccess();

    // Umrechnung auf PixelCoor der Bitmaps und zurueck
    Point GetPixelCoor(B3dEntity& rEntity);
    basegfx::B3DPoint Get3DCoor(Point& rPnt, double fDepth);

    // ZBuffer funktionen
    inline sal_Bool IsInScissorRegion(sal_Int32 nX, sal_Int32 nY);
    inline sal_Bool IsVisibleAndScissor(sal_Int32 nX, sal_Int32 nY, sal_uInt32 nDepth);
    inline void WritePixel(sal_Int32 nX, sal_Int32 nY, Color aColor, sal_uInt32 nDepth);

    // Zeilenerzeuger fuer Polygon
    /*inline*/ void DrawLinePhongTexture(sal_Int32 nYPos, B3dMaterial& rMat);
    /*inline*/ void DrawLinePhong(sal_Int32 nYPos, B3dMaterial& rMat);
    /*inline*/ void DrawLineColorTexture(sal_Int32 nYPos);
    /*inline*/ void DrawLineColor(sal_Int32 nYPos);
    /*inline*/ void DrawLineTexture(sal_Int32 nYPos, Color& rCol);
    /*inline*/ void DrawLine(sal_Int32 nYPos, Color& rCol);

    inline void LoadLeft(sal_Int32 nSize);
    inline void LoadLeftTexture(sal_Int32 nSize);
    inline void LoadRight(sal_Int32 nSize);
    inline void LoadRightTexture(sal_Int32 nSize);

    inline void NextStepRight();
    inline void NextStepRightTexture();
    inline void NextStepLeft();
    inline void NextStepLeftTexture();

public:
    Base3DDefault(OutputDevice* pOutDev);
    virtual ~Base3DDefault();

    // Typbestimmung
    virtual UINT16 GetBase3DType();

    // Szenenverwaltung
    virtual void StartScene();
    virtual void EndScene();

    // Nullwert des ZBuffers setzen
    void SetClearValue(sal_uInt32 nNew) { aClearValue = Color(nNew); }

    // Detailstufe/Vergroeberung der Ausgabe setzen/lesen
    void SetDetail(double fNew=1.0);
    double GetDetail() { return fDetail; }
    void SetMaxPixels(sal_Int32 nNew=500000) { nMaxPixels = nNew; }
    sal_Int32 GetMaxPixels() { return nMaxPixels; }
    virtual void SetDisplayQuality(sal_uInt8 nNew);

    // Callbacks bei Matrixaenderungen
    virtual void SetTransformationSet(B3dTransformationSet* pSet);

protected:

    // Geometrie
    virtual void Clipped3DPoint(sal_uInt32 nInd);
    virtual void Clipped3DLine(sal_uInt32 nInd1, sal_uInt32 nInd2);
    virtual void Clipped3DTriangle(sal_uInt32 nInd1, sal_uInt32 nInd2, sal_uInt32 nInd3);
};



#endif          // _B3D_B3DDEFLT_HXX
