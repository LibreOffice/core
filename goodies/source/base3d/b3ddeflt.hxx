/*************************************************************************
 *
 *  $RCSfile: b3ddeflt.hxx,v $
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

#ifndef _B3D_B3DDEFLT_HXX
#define _B3D_B3DDEFLT_HXX

#ifndef _B3D_B3DCOMMN_HXX
#include "b3dcommn.hxx"
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_ALPHA_HXX
#include <vcl/alpha.hxx>
#endif

#ifndef _BXD_INTERPOLATOR_HXX
#include "bxdintpo.hxx"
#endif

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
    long                    nMaxPixels;

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

    // Booleans fuer Zeichenoperationen
    unsigned                bNormalsUsed                    : 1;
    unsigned                bTextureUsed                    : 1;

    // Bool fuer Detailreduzierung
    unsigned                bReducedDetail                  : 1;
    unsigned                bDetailBackedup                 : 1;

    // Funktionen fuer erlangen/freigeben der BitmapAccesses
    void AcquireAccess();
    void ReleaseAccess();

    // Umrechnung auf PixelCoor der Bitmaps und zurueck
    Point GetPixelCoor(B3dEntity& rEntity);
    Vector3D Get3DCoor(Point& rPnt, double fDepth);

    // ZBuffer funktionen
    inline BOOL IsInScissorRegion(long nX, long nY);
    inline BOOL IsVisibleAndScissor(long nX, long nY, UINT32 nDepth);
    inline void WritePixel(long nX, long nY, Color aColor, UINT32 nDepth);

    // Zeilenerzeuger fuer Polygon
    /*inline*/ void DrawLinePhongTexture(long nYPos, B3dMaterial& rMat);
    /*inline*/ void DrawLinePhong(long nYPos, B3dMaterial& rMat);
    /*inline*/ void DrawLineColorTexture(long nYPos);
    /*inline*/ void DrawLineColor(long nYPos);
    /*inline*/ void DrawLineTexture(long nYPos, Color& rCol);
    /*inline*/ void DrawLine(long nYPos, Color& rCol);

    inline void LoadLeft(long nSize);
    inline void LoadLeftTexture(long nSize);
    inline void LoadRight(long nSize);
    inline void LoadRightTexture(long nSize);

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
    void SetClearValue(UINT32 nNew) { aClearValue = Color(nNew); }

    // Detailstufe/Vergroeberung der Ausgabe setzen/lesen
    void SetDetail(double fNew=1.0);
    double GetDetail() { return fDetail; }
    void SetMaxPixels(long nNew=500000) { nMaxPixels = nNew; }
    long GetMaxPixels() { return nMaxPixels; }
    virtual void SetDisplayQuality(UINT8 nNew);

    // Callbacks bei Matrixaenderungen
    virtual void SetTransformationSet(B3dTransformationSet* pSet);

protected:

    // Geometrie
    virtual void Clipped3DPoint(UINT32 nInd);
    virtual void Clipped3DLine(UINT32 nInd1, UINT32 nInd2);
    virtual void Clipped3DTriangle(UINT32 nInd1, UINT32 nInd2,
        UINT32 nInd3);
};



#endif          // _B3D_B3DDEFLT_HXX
