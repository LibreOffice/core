/*************************************************************************
 *
 *  $RCSfile: b3dopngl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 14:47:42 $
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

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS3D_HXX
#include <svtools/options3d.hxx>
#endif

#ifndef _B3D_B3DOPNGL_HXX
#include "b3dopngl.hxx"
#endif

#ifndef _B3D_HMATRIX_HXX
#include "hmatrix.hxx"
#endif

#ifndef _B3D_B3DTEX_HXX
#include "b3dtex.hxx"
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _B3D_B3DGEOM_HXX
#include "b3dgeom.hxx"
#endif

/*************************************************************************
|*
|* Konstruktor Base3DOpenGL
|*
\************************************************************************/

Base3DOpenGL::Base3DOpenGL(OutputDevice* pOutDev)
:   Base3D(pOutDev),
    aOpenGL(pOutDev),
    aEmptyVector(0.0, 0.0, 0.0),
    aLastNormal(DBL_MAX, DBL_MAX, DBL_MAX),
    aLastTexCoor(DBL_MAX, DBL_MAX, DBL_MAX),
    fOffFacMul100((float)(-0.2 * 100.0)),
    fOffUniMul100((float)(-1.0 * 100.0)),
    aPhongBuffer(12),       // 4K
    nPhongDivideSize(20),
    bForceToSinglePrimitiveOutput(TRUE) // (#70626#)
{
    // create OpenGL context for pOutDev; pOutDev is NOT a printer,
    // so don't care about printers in this place
    if(aOpenGL.IsValid())
    {
        // setup default parameters
        aOpenGL.ClearDepth( 1.0 );
        aOpenGL.DepthFunc( GL_LEQUAL );
        aOpenGL.Enable( GL_DEPTH_TEST );
        aOpenGL.Enable( GL_DITHER );
        aOpenGL.Enable( GL_NORMALIZE );
        aOpenGL.Disable( GL_CULL_FACE );
        aOpenGL.Disable( GL_LIGHTING );
        aOpenGL.Disable( GL_LINE_SMOOTH );
        aOpenGL.Disable( GL_POINT_SMOOTH );
        aOpenGL.Disable( GL_POLYGON_SMOOTH );
        aOpenGL.Disable( GL_POLYGON_STIPPLE );
        aOpenGL.Disable( GL_LINE_STIPPLE );
        aOpenGL.Disable( GL_TEXTURE_1D );
        aOpenGL.Disable( GL_TEXTURE_2D );
        aOpenGL.Disable( GL_BLEND );
        aOpenGL.DepthMask( GL_TRUE );
        aOpenGL.ShadeModel( GL_SMOOTH );
        aOpenGL.EdgeFlag( GL_TRUE );
        aOpenGL.Disable( GL_SCISSOR_TEST );
    }
    SetContextIsValid(aOpenGL.IsValid());
    CalcInternPhongDivideSize();
    bForceToSinglePrimitiveOutput = SvtOptions3D().IsOpenGL_Faster();
}

/*************************************************************************
|*
|* Typbestimmung
|*
\************************************************************************/

UINT16 Base3DOpenGL::GetBase3DType()
{
    return BASE3D_TYPE_OPENGL;
}

/*************************************************************************
|*
|* Start der Szenenbeschreibung: Evtl. neuer HDC, Loesche Tiefenbuffer
|*
\************************************************************************/

void Base3DOpenGL::StartScene()
{
    // Falls Transparenz an war, diese zuruecknehmen
    aOpenGL.Disable( GL_BLEND );
    aOpenGL.DepthMask( TRUE );

    // OutputDevice setzen und ZBuffer loeschen
    aOpenGL.SetConnectOutputDevice(GetOutputDevice());
    aOpenGL.Clear( GL_DEPTH_BUFFER_BIT );
}

/*************************************************************************
|*
|* Scissoring Region setzen
|*
\************************************************************************/

void Base3DOpenGL::SetScissorRegionPixel(const Rectangle& rRect, BOOL bActivate)
{
    // OpenGL specifics
    aOpenGL.Scissor( rRect.Left(), rRect.Top(),
        rRect.GetWidth(), rRect.GetHeight());

    // call parent
    Base3D::SetScissorRegionPixel(rRect, bActivate);
}

/*************************************************************************
|*
|* Dithering
|*
\************************************************************************/

void Base3DOpenGL::SetDither(BOOL bNew)
{
    // call parent
    Base3D::SetDither(bNew);

    // OpenGL specifics
    if(GetDither())
        aOpenGL.Enable( GL_DITHER );
    else
        aOpenGL.Disable( GL_DITHER );
}

/*************************************************************************
|*
|* Scissoring aktivieren/deaktivieren
|*
\************************************************************************/

void Base3DOpenGL::ActivateScissorRegion(BOOL bNew)
{
    // OpenGL specifics
    if(bNew)
    {
        aOpenGL.Enable(GL_SCISSOR_TEST);
    }
    else
    {
        aOpenGL.Disable(GL_SCISSOR_TEST);
    }

    // call parent
    Base3D::ActivateScissorRegion(bNew);
}

/*************************************************************************
|*
|* Ende der Szenenbeschreibung: Erzwinge bisher nicht erzeugte Ausgaben
|*
\************************************************************************/

void Base3DOpenGL::EndScene()
{
    aOpenGL.Flush();
    aOpenGL.Finish();
}

/*************************************************************************
|*
|* Neuen freien Eintrag fuer naechste geometrische Daten liefern
|*
\************************************************************************/

B3dEntity& Base3DOpenGL::ImplGetFreeEntity()
{
    return aEntity;
}

/*************************************************************************
|*
|* starte primitiv auch in OpenGL
|*
\************************************************************************/

void Base3DOpenGL::ImplStartPrimitive()
{
    bPhongBufferedMode = (GetShadeModel() == Base3DPhong
        && GetRenderMode() == Base3DRenderFill
        && (GetObjectMode() == Base3DTriangles
            || GetObjectMode() == Base3DTriangleStrip
            || GetObjectMode() == Base3DTriangleFan
            || GetObjectMode() == Base3DQuads
            || GetObjectMode() == Base3DQuadStrip
            || GetObjectMode() == Base3DPolygon ));

    // Transparenz beachten
    if(GetMaterial(Base3DMaterialDiffuse).GetTransparency())
    {
        aOpenGL.Enable( GL_BLEND );
        aOpenGL.DepthMask( FALSE );
        aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
        aOpenGL.Disable( GL_BLEND );
        aOpenGL.DepthMask( TRUE );
    }

    if(bPhongBufferedMode)
    {
        // Phong-Modus: Ausgabe sammeln und in kleinere
        // Dreiecke zerlegt bei ImplEndPrimitive() rendern
        aPhongBuffer.Erase();
    }
    else
    {
        // OpenGL direkt benutzen
        aOpenGL.Begin(GetObjectMode());
    }
}

/*************************************************************************
|*
|* beende primitiv auch in OpenGL
|*
\************************************************************************/

void Base3DOpenGL::ImplEndPrimitive()
{
    if(bPhongBufferedMode)
    {
        DrawPhongPrimitives();
    }
    else
    {
        aOpenGL.End();
    }
}

/*************************************************************************
|*
|* Darstellungsqualitaet setzen
|*
\************************************************************************/

void Base3DOpenGL::SetDisplayQuality(UINT8 nNew)
{
    // call parent
    Base3D::SetDisplayQuality(nNew);

    // endgueltige Splittinggroesse neuberechnen
    CalcInternPhongDivideSize();
}

/*************************************************************************
|*
|* PhongMode Splitting-Groesse neu setzen
|*
\************************************************************************/

void Base3DOpenGL::SetPhongDivideSize(long nNew)
{
    nPhongDivideSize = nNew;

    // endgueltige Splittinggroesse neuberechnen
    CalcInternPhongDivideSize();
}

/*************************************************************************
|*
|* PhongMode interne Splitting-Groesse neu setzen
|*
\************************************************************************/

void Base3DOpenGL::CalcInternPhongDivideSize()
{
    if(GetDisplayQuality() != 255)
    {
        long nNew = GetPhongDivideSize() +
            ((255L - (INT32)GetDisplayQuality())>>2);
        nInternPhongDivideSize = nNew * nNew;
    }
    else
        nInternPhongDivideSize = GetPhongDivideSize() * GetPhongDivideSize();
}

/*************************************************************************
|*
|* Zeichne fuer Phong gebufferte primitive als Dreiecke
|*
\************************************************************************/

void Base3DOpenGL::DrawPhongPrimitives()
{
    UINT32 aCount = aPhongBuffer.Count();
    UINT32 aPos(0L);
    bPhongBufferedMode = FALSE;
    aOpenGL.Begin(Base3DTriangles);

    switch(GetObjectMode())
    {
        case Base3DTriangles :
        {
            while(aPos < aCount)
            {
                DrawPhongTriangle(aPos, aPos+1, aPos+2);
                aPos += 3;
            }
            break;
        }
        case Base3DTriangleStrip:
        {
            aPos = 1;
            while(aPos < aCount)
            {
                if(aPos%2)
                    DrawPhongTriangle(aPos-1, aPos, aPos+1);
                else
                    DrawPhongTriangle(aPos-1, aPos+1, aPos);
                aPos++;
            }
            break;
        }
        case Base3DTriangleFan:
        {
            aPos = 1;
            while(aPos < aCount)
            {
                DrawPhongTriangle(0, aPos, aPos+1);
                aPos++;
            }
            break;
        }
        case Base3DQuads:
        {
            while(aPos < aCount)
            {
                DrawPhongTriangle(aPos, aPos+1, aPos+2);
                DrawPhongTriangle(aPos+2, aPos+3, aPos);
                aPos += 4;
            }
            break;
        }
        case Base3DQuadStrip:
        {
            aPos = 1;
            while(aPos < aCount)
            {
                DrawPhongTriangle(aPos, aPos+1, aPos+3);
                DrawPhongTriangle(aPos, aPos+3, aPos+2);
                aPos+=2;
            }
            break;
        }
        case Base3DPolygon:
        {
            aPos = 2;
            while(aPos < aCount)
            {
                DrawPhongTriangle(0, aPos-1, aPos);
                aPos++;
            }
            break;
        }
    }
    aOpenGL.End();
}

/*************************************************************************
|*
|* Zeichne fuer Phong gebufferte Dreiecke
|*
\************************************************************************/

void Base3DOpenGL::DrawPhongTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3)
{
    Vector3D aPos1 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd1].Point().GetVector3D());
    double fXMin = aPos1.X();
    double fXMax = aPos1.X();
    double fYMin = aPos1.Y();
    double fYMax = aPos1.Y();
    Vector3D aPos2 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd2].Point().GetVector3D());
    if(aPos2.X() < fXMin)
        fXMin = aPos2.X();
    if(aPos2.X() > fXMax)
        fXMax = aPos2.X();
    if(aPos2.Y() < fYMin)
        fYMin = aPos2.Y();
    if(aPos2.Y() > fYMax)
        fYMax = aPos2.Y();
    aPos2 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd3].Point().GetVector3D());
    if(aPos2.X() < fXMin)
        fXMin = aPos2.X();
    if(aPos2.X() > fXMax)
        fXMax = aPos2.X();
    if(aPos2.Y() < fYMin)
        fYMin = aPos2.Y();
    if(aPos2.Y() > fYMax)
        fYMax = aPos2.Y();

    Size aPixelSize = GetOutputDevice()->LogicToPixel(
        Size((long)(fXMax - fXMin),(long)(fYMax - fYMin)));
    if(aPixelSize.Width() * aPixelSize.Height() > nInternPhongDivideSize)
    {
        UINT32 aCount = aPhongBuffer.Count();
        aPhongBuffer.Append();
        aPhongBuffer.Append();
        aPhongBuffer.Append();

        aPhongBuffer[aCount  ].CalcMiddle(aPhongBuffer[nInd1], aPhongBuffer[nInd2]);
        aPhongBuffer[aCount+1].CalcMiddle(aPhongBuffer[nInd2], aPhongBuffer[nInd3]);
        aPhongBuffer[aCount+2].CalcMiddle(aPhongBuffer[nInd3], aPhongBuffer[nInd1]);

        DrawPhongTriangle(nInd1, aCount, aCount+2);
        DrawPhongTriangle(aCount, nInd2, aCount+1);
        DrawPhongTriangle(aCount+1, nInd3, aCount+2);
        DrawPhongTriangle(aCount, aCount+1, aCount+2);

        aPhongBuffer.Remove();
        aPhongBuffer.Remove();
        aPhongBuffer.Remove();
    }
    else
    {
        ImplPostAddVertex(aPhongBuffer[nInd1]);
        ImplPostAddVertex(aPhongBuffer[nInd2]);
        ImplPostAddVertex(aPhongBuffer[nInd3]);
    }
}

/*************************************************************************
|*
|* Geometrische Daten direkt an OpenGL weitergeben
|* ACHTUNG! Es wird die aktuelle Farbe benutzt, NICHT die in Enttity3D
|* enthaltene!
|*
\************************************************************************/

void Base3DOpenGL::ImplPostAddVertex(B3dEntity& rEntity)
{
    if(bPhongBufferedMode)
    {
        aPhongBuffer.Append(rEntity);
    }
    else
    {
        // Normale setzen
        if(rEntity.IsNormalUsed())
        {
            if(GetForceFlat() || GetShadeModel() == Base3DFlat)
            {
                if(rEntity.PlaneNormal() != aLastNormal)
                {
                    aLastNormal = rEntity.PlaneNormal();
                    aOpenGL.Normal3dv(&aLastNormal.X());
                }
            }
            else
            {
                if(rEntity.Normal() != aLastNormal)
                {
                    aLastNormal = rEntity.Normal();
                    aOpenGL.Normal3dv(&aLastNormal.X());
                }
            }
        }
        else
        {
            if(aLastNormal != aEmptyVector)
            {
                aLastNormal = aEmptyVector;
                aOpenGL.Normal3dv(&aLastNormal.X());
            }
        }

        // Texturkoordinate setzen
        if(rEntity.IsTexCoorUsed())
        {
            if(rEntity.TexCoor() != aLastTexCoor)
            {
                aLastTexCoor = rEntity.TexCoor();
                aOpenGL.TexCoord3dv(&aLastTexCoor.X());
            }
        }
        else
        {
            if(aLastTexCoor != aEmptyVector)
            {
                aLastTexCoor = aEmptyVector;
                aOpenGL.TexCoord3dv(&aLastTexCoor.X());
            }
        }

        // Punkt erzeugen
        aOpenGL.Vertex3dv(&rEntity.Point().X());
    }
}

/*************************************************************************
|*
|* beim setzen von color und alpha reagieren
|*
\************************************************************************/

void Base3DOpenGL::SetColor(Color aNew)
{
    // call parent
    Base3D::SetColor(aNew);

    // Normale Farbausgabe
    aOpenGL.Color4ub(GetColor().GetRed(), GetColor().GetGreen(),
        GetColor().GetBlue(), 0xff - GetColor().GetTransparency());
}

/*************************************************************************
|*
|* Materialeigenschaften setzen
|*
\************************************************************************/

void Base3DOpenGL::SetMaterial(Color rNew, Base3DMaterialValue eVal,
    Base3DMaterialMode eMode)
{
    // call parent
    Base3D::SetMaterial(rNew, eVal, eMode);

    // OpenGL Specifics
    GLenum eFace = GL_FRONT_AND_BACK;
    if(eMode == Base3DMaterialFront)
        eFace = GL_FRONT;
    if(eMode == Base3DMaterialBack)
        eFace = GL_BACK;
    GLenum eName = GL_SPECULAR;
    if(eVal == Base3DMaterialAmbient)
        eName = GL_AMBIENT;
    if(eVal == Base3DMaterialDiffuse)
        eName = GL_DIFFUSE;
    if(eVal == Base3DMaterialEmission)
        eName = GL_EMISSION;

    // Array fuellen
    float fArray[4] = {
        ((float)GetMaterial(eVal, eMode).GetRed()) / (float)255.0,
        ((float)GetMaterial(eVal, eMode).GetGreen()) / (float)255.0,
        ((float)GetMaterial(eVal, eMode).GetBlue()) / (float)255.0,
        ((float)(255 - GetMaterial(eVal, eMode).GetTransparency())) / (float)255.0
    };

    aOpenGL.Materialfv(eFace, eName, fArray);
}

/*************************************************************************
|*
|* Materialeigenschaften setzen, exponent der specular-Eigenschaft
|*
\************************************************************************/

void Base3DOpenGL::SetShininess(UINT16 nExponent,
    Base3DMaterialMode eMode)
{
    // call parent
    Base3D::SetShininess(nExponent, eMode);

    // OpenGL Specifics
    GLenum eFace = GL_FRONT_AND_BACK;
    if(eMode == Base3DMaterialFront)
        eFace = GL_FRONT;
    if(eMode == Base3DMaterialBack)
        eFace = GL_BACK;
    aOpenGL.Materialf(eFace, GL_SHININESS, (float)nExponent);
}

/*************************************************************************
|*
|* Aktuell zu benutzende Textur setzen
|*
\************************************************************************/

void Base3DOpenGL::SetActiveTexture(B3dTexture* pTex)
{
    // call parent
    Base3D::SetActiveTexture(pTex);

    // make current texture, cast ist unkritisch, da innerhalb von
    // Base3DOpenGL nur Texturen von diesem Typ angelegt worden
    // sein koennen
    if(GetActiveTexture())
    {
        aOpenGL.Enable(GL_TEXTURE_2D);
        ((B3dTextureOpenGL*)GetActiveTexture())->MakeCurrentTexture(aOpenGL);
    }
    else
    {
        aOpenGL.BindTexture(GL_TEXTURE_2D, 0);
        aOpenGL.Disable(GL_TEXTURE_2D);
    }
}

/*************************************************************************
|*
|* Ein Textur-Objekt inkarnieren
|*
\************************************************************************/

B3dTexture* Base3DOpenGL::CreateTexture(TextureAttributes& rAtt, Bitmap& rBitmap)
{
    // Hier Parent NICHT rufen! Sonst wird auch noch eine normale Textur erzeugt
    B3dTextureOpenGL* pRetval = new B3dTextureOpenGL(rAtt, rBitmap, aOpenGL);
    DBG_ASSERT(pRetval,"AW: Kein Speicher fuer OpenGL-Textur bekommen!");
    return pRetval;
}

/*************************************************************************
|*
|* OpenGL - Textur loeschen
|*
\************************************************************************/

void Base3DOpenGL::DestroyTexture(B3dTexture* pTex)
{
    // Spezielle Loeschaufgaben im Zusammenhang mit OpenGL
    ((B3dTextureOpenGL*)pTex)->DestroyOpenGLTexture(aOpenGL);

    // call parent, endgueltig loeschen
    Base3D::DestroyTexture(pTex);
}

/*************************************************************************
|*
|* PolygonOffset setzen
|*
\************************************************************************/

void Base3DOpenGL::SetPolygonOffset(Base3DPolygonOffset eNew, BOOL bNew)
{
    // call parent
    Base3D::SetPolygonOffset(eNew, bNew);

    if(GetPolygonOffset())
        aOpenGL.PolygonOffset((float)(fOffFacMul100 / 100.0), (float)(fOffUniMul100 / 100.0));
    else
        aOpenGL.PolygonOffset((float)0.0, (float)0.0);

    // OpenGL Specifics
    switch(eNew)
    {
        case Base3DPolygonOffsetFill :
            if(bNew)
                aOpenGL.Enable( GL_POLYGON_OFFSET_FILL );
            else
                aOpenGL.Disable( GL_POLYGON_OFFSET_FILL );
            break;

        case Base3DPolygonOffsetLine :
            if(bNew)
                aOpenGL.Enable( GL_POLYGON_OFFSET_LINE );
            else
                aOpenGL.Disable( GL_POLYGON_OFFSET_LINE );
            break;

        case Base3DPolygonOffsetPoint :
            if(bNew)
                aOpenGL.Enable( GL_POLYGON_OFFSET_POINT );
            else
                aOpenGL.Disable( GL_POLYGON_OFFSET_POINT );
            break;
    }
}

/*************************************************************************
|*
|* Beleuchtung setzen/lesen
|*
\************************************************************************/

void Base3DOpenGL::SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal)
{
    // call parent
    Base3D::SetLightGroup(pSet, bSetGlobal);

    if(GetLightGroup())
    {
        // Allgemeine Parameter setzen
        SetGlobalAmbientLight(GetLightGroup()->GetGlobalAmbientLight());
        SetLocalViewer(GetLightGroup()->GetLocalViewer());
        SetModelTwoSide(GetLightGroup()->GetModelTwoSide());
        EnableLighting(GetLightGroup()->IsLightingEnabled());

        // Einzelne Lampen setzen
        if(GetTransformationSet() && bSetGlobal)
        {
            aOpenGL.MatrixMode(GL_MODELVIEW);
            aOpenGL.LoadIdentity();
        }

        // Set and enable lights from the beginning of array in
        // OpenGL
        UINT16 nNumAlloc = 0;

        UINT16 a;
        for(a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
        {
            Base3DLightNumber eNum = (Base3DLightNumber)(Base3DLight0 + a);
            B3dLight& rLight = GetLightGroup()->GetLightObject(eNum);

            if(rLight.IsEnabled())
            {
                Base3DLightNumber eNumAlloc = (Base3DLightNumber)(Base3DLight0 + nNumAlloc);
                nNumAlloc++;

                Enable(TRUE, eNumAlloc);

                SetIntensity(rLight.GetIntensity(Base3DMaterialAmbient),
                    Base3DMaterialAmbient, eNumAlloc);
                SetIntensity(rLight.GetIntensity(Base3DMaterialDiffuse),
                    Base3DMaterialDiffuse, eNumAlloc);
                SetIntensity(rLight.GetIntensity(Base3DMaterialSpecular),
                    Base3DMaterialSpecular, eNumAlloc);

                if(rLight.IsDirectionalSource())
                {
                    SetDirection(rLight.GetPosition(), eNumAlloc);
                }
                else
                {
                    SetPosition(rLight.GetPosition(), eNumAlloc);
                    SetSpotDirection(rLight.GetSpotDirection(), eNumAlloc);
                    SetSpotExponent(rLight.GetSpotExponent(), eNumAlloc);
                    SetSpotCutoff(rLight.GetSpotCutoff(), eNumAlloc);
                }

                SetConstantAttenuation(rLight.GetConstantAttenuation(), eNumAlloc);
                SetLinearAttenuation(rLight.GetLinearAttenuation(), eNumAlloc);
                SetQuadraticAttenuation(rLight.GetQuadraticAttenuation(), eNumAlloc);
            }
        }

        for(a=nNumAlloc;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
        {
            Base3DLightNumber eNum = (Base3DLightNumber)(Base3DLight0 + a);
            Enable(FALSE, eNum);
        }

        if(GetTransformationSet() && bSetGlobal)
            PostSetObjectOrientation(GetTransformationSet());
    }
}

/*************************************************************************
|*
|* globales Umgebungslicht setzen
|*
\************************************************************************/

void Base3DOpenGL::SetGlobalAmbientLight(const Color rNew)
{
    // OpenGL Specifics
    Color aSource;
    if(GetOutputDevice()->GetDrawMode() & DRAWMODE_GRAYFILL)
    {
        // Graustufen
        UINT8 nLuminance = rNew.GetLuminance();
        aSource.SetRed(nLuminance);
        aSource.SetGreen(nLuminance);
        aSource.SetBlue(nLuminance);
        aSource.SetTransparency(rNew.GetTransparency());
    }
    else if(GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
    {
        // Keine Ausgabe, hier Weiss als Farbe setzen
        aSource = Color(COL_WHITE);
    }
    else
    {
        // Normale Farbausgabe
        aSource = rNew;
    }

    // Array fuellen
    float fArray[4] = {
        ((float)aSource.GetRed()) / (float)255.0,
        ((float)aSource.GetGreen()) / (float)255.0,
        ((float)aSource.GetBlue()) / (float)255.0,
        ((float)aSource.GetTransparency()) / (float)255.0
    };
    aOpenGL.LightModelfv(GL_LIGHT_MODEL_AMBIENT, fArray);
}

/*************************************************************************
|*
|* Modus globaler Viewer bei Berechnung specular reflection setzen
|*
\************************************************************************/

void Base3DOpenGL::SetLocalViewer(BOOL bNew)
{
    // OpenGL Specifics
    aOpenGL.LightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,
        bNew ? (float)0.0 : (float)1.0);
}

/*************************************************************************
|*
|* Modus Beleuchtungsmodell beidseitig anwenden setzen
|*
\************************************************************************/

void Base3DOpenGL::SetModelTwoSide(BOOL bNew)
{
    // OpenGL Specifics
    aOpenGL.LightModelf(GL_LIGHT_MODEL_TWO_SIDE,
        bNew ? (float)1.0 : (float)0.0);
}

/*************************************************************************
|*
|* Beleuchtungsmodell aktivieren/deaktivieren
|*
\************************************************************************/

void Base3DOpenGL::EnableLighting(BOOL bNew)
{
    // OpenGL Specifics
    if(bNew)
        aOpenGL.Enable( GL_LIGHTING );
    else
        aOpenGL.Disable( GL_LIGHTING );
}

/*************************************************************************
|*
|* Die Intensitaet eines bestimmten Aspekts einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetIntensity(const Color rNew,
    Base3DMaterialValue eVal, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    Color aSource;
    if(GetOutputDevice()->GetDrawMode() & DRAWMODE_GRAYFILL)
    {
        // Graustufen
        UINT8 nLuminance = rNew.GetLuminance();
        aSource.SetRed(nLuminance);
        aSource.SetGreen(nLuminance);
        aSource.SetBlue(nLuminance);
        aSource.SetTransparency(rNew.GetTransparency());
    }
    else if(GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
    {
        // Keine Ausgabe, hier Weiss als Farbe setzen
        aSource = Color(COL_WHITE);
    }
    else
    {
        // Normale Farbausgabe
        aSource = rNew;
    }

    // Array fuellen
    float fArray[4] = {
        ((float)aSource.GetRed()) / (float)255.0,
        ((float)aSource.GetGreen()) / (float)255.0,
        ((float)aSource.GetBlue()) / (float)255.0,
        ((float)aSource.GetTransparency()) / (float)255.0
    };

    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    GLenum eName = GL_SPECULAR;
    if(eVal == Base3DMaterialAmbient)
        eName = GL_AMBIENT;
    if(eVal == Base3DMaterialDiffuse)
        eName = GL_DIFFUSE;
    aOpenGL.Lightfv(eLight, eName, fArray);
}

/*************************************************************************
|*
|* Die Position einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetPosition(const Vector3D& rNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew[0], (float)rNew[1], (float)rNew[2], (float)1.0
    };
    aOpenGL.Lightfv(eLight, GL_POSITION, fArray);
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetDirection(const Vector3D& rNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew[0], (float)rNew[1], (float)rNew[2], (float)0.0
    };
    aOpenGL.Lightfv(eLight, GL_POSITION, fArray);
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetSpotDirection(const Vector3D& rNew,
    Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew[0], (float)rNew[1], (float)rNew[2], (float)0.0
    };
    aOpenGL.Lightfv(eLight, GL_SPOT_DIRECTION, fArray);
}

/*************************************************************************
|*
|* Den SpotExponent einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetSpotExponent(UINT16 nNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    aOpenGL.Lightf(eLight, GL_SPOT_EXPONENT, (float)nNew);
}

/*************************************************************************
|*
|* Die Einengung des Lichtkegels einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetSpotCutoff(double fNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    aOpenGL.Lightf(eLight, GL_SPOT_CUTOFF, (float)fNew);
}

/*************************************************************************
|*
|* Den konstanten AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetConstantAttenuation(double fNew,
    Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    aOpenGL.Lightf(eLight, GL_CONSTANT_ATTENUATION, (float)fNew);
}

/*************************************************************************
|*
|* Den linearen AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetLinearAttenuation(double fNew,
    Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    aOpenGL.Lightf(eLight, GL_LINEAR_ATTENUATION, (float)fNew);
}

/*************************************************************************
|*
|* Den quadratischen AttenuationFactor einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetQuadraticAttenuation(double fNew,
    Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    aOpenGL.Lightf(eLight, GL_QUADRATIC_ATTENUATION, (float)fNew);
}

/*************************************************************************
|*
|* Eine Lichtquelle aktivieren/deaktivieren
|*
\************************************************************************/

void Base3DOpenGL::Enable(BOOL bNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    if(bNew)
        aOpenGL.Enable(eLight);
    else
        aOpenGL.Disable(eLight);
}

/*************************************************************************
|*
|* RenderMode setzen
|*
\************************************************************************/

void Base3DOpenGL::SetRenderMode(Base3DRenderMode eNew,
    Base3DMaterialMode eMode)
{
    // call parent
    Base3D::SetRenderMode(eNew, eMode);

    // OpenGL Specifics
    GLenum eFace = GL_FRONT_AND_BACK;
    if(eMode == Base3DMaterialFront)
        eFace = GL_FRONT;
    if(eMode == Base3DMaterialBack)
        eFace = GL_BACK;

    switch(eNew)
    {
        case Base3DRenderNone :
        {
            break;
        }
        case Base3DRenderPoint :
        {
            aOpenGL.PolygonMode(eFace, GL_POINT);
            break;
        }
        case Base3DRenderLine :
        {
            aOpenGL.PolygonMode(eFace, GL_LINE);
            break;
        }
        case Base3DRenderFill :
        {
            aOpenGL.PolygonMode(eFace, GL_FILL);
            break;
        }
    }
}

/*************************************************************************
|*
|* ShadeModel setzen
|*
\************************************************************************/

void Base3DOpenGL::SetShadeModel(Base3DShadeModel eNew)
{
    // call parent
    Base3D::SetShadeModel(eNew);

    switch(eNew)
    {
        case Base3DSmooth :
        case Base3DPhong :
        {
            aOpenGL.ShadeModel(GL_SMOOTH);
            break;
        }
        case Base3DFlat :
        {
            aOpenGL.ShadeModel(GL_FLAT);
            break;
        }
    }
}

/*************************************************************************
|*
|* CullingMode setzen
|*
\************************************************************************/

void Base3DOpenGL::SetCullMode(Base3DCullMode eNew)
{
    // call parent
    Base3D::SetCullMode(eNew);

    switch(eNew)
    {
        case Base3DCullFront :
        {
            aOpenGL.CullFace(GL_FRONT);
            aOpenGL.Enable(GL_CULL_FACE);
            break;
        }
        case Base3DCullBack :
        {
            aOpenGL.CullFace(GL_BACK);
            aOpenGL.Enable(GL_CULL_FACE);
            break;
        }
        case Base3DCullNone :
        {
            aOpenGL.Disable(GL_CULL_FACE);
            break;
        }
    }
}

/*************************************************************************
|*
|* EdgeFlag schreiben
|*
\************************************************************************/

void Base3DOpenGL::SetEdgeFlag(BOOL bNew)
{
    // EdgeFlag fuer OpenGL setzen
    if(bNew)
        aOpenGL.EdgeFlag(GL_TRUE);
    else
        aOpenGL.EdgeFlag(GL_FALSE);

    // call parent
    Base3D::SetEdgeFlag(bNew);
}

/*************************************************************************
|*
|* PointSize schreiben
|*
\************************************************************************/

void Base3DOpenGL::SetPointSize(double fNew)
{
    // PointSize fuer OpenGL setzen
    aOpenGL.PointSize((GLfloat)fNew);

    // call parent
    Base3D::SetPointSize(fNew);
}

/*************************************************************************
|*
|* LineWidth schreiben
|*
\************************************************************************/

void Base3DOpenGL::SetLineWidth(double fNew)
{
    // LineWidth fuer OpenGL setzen
    aOpenGL.LineWidth((GLfloat)fNew);

    // call parent
    Base3D::SetLineWidth(fNew);
}

/*************************************************************************
|*
|* Callbacks bei Matrixaenderungen, Default-Implementierungen
|*
\************************************************************************/

void Base3DOpenGL::SetTransformationSet(B3dTransformationSet* pSet)
{
    // call parent
    Base3D::SetTransformationSet(pSet);

    if(GetTransformationSet())
    {
        PostSetObjectOrientation(GetTransformationSet());
        PostSetProjection(GetTransformationSet());
        PostSetTexture(GetTransformationSet());
        PostSetViewport(GetTransformationSet());
    }
}

void Base3DOpenGL::PostSetObjectOrientation(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    Matrix4D aMat = pCaller->GetObjectTrans();
    aMat *= pCaller->GetOrientation();
    double fBuffer[16] = {
        aMat[0][0], aMat[1][0], aMat[2][0], aMat[3][0],
        aMat[0][1], aMat[1][1], aMat[2][1], aMat[3][1],
        aMat[0][2], aMat[1][2], aMat[2][2], aMat[3][2],
        aMat[0][3], aMat[1][3], aMat[2][3], aMat[3][3]
    };
    aOpenGL.MatrixMode(GL_MODELVIEW);
    aOpenGL.LoadMatrixd(fBuffer);
}

void Base3DOpenGL::PostSetProjection(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    const Matrix4D& rMat = pCaller->GetProjection();
    double fBuffer[16] = {
        rMat[0][0], rMat[1][0], rMat[2][0], rMat[3][0],
        rMat[0][1], rMat[1][1], rMat[2][1], rMat[3][1],
        rMat[0][2], rMat[1][2], rMat[2][2], rMat[3][2],
        rMat[0][3], rMat[1][3], rMat[2][3], rMat[3][3]
    };
    aOpenGL.MatrixMode(GL_PROJECTION);
    aOpenGL.LoadMatrixd(fBuffer);
}

void Base3DOpenGL::PostSetTexture(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    const Matrix4D& rMat = pCaller->GetTexture();
    double fBuffer[16] = {
        rMat[0][0], rMat[1][0], rMat[2][0], rMat[3][0],
        rMat[0][1], rMat[1][1], rMat[2][1], rMat[3][1],
        rMat[0][2], rMat[1][2], rMat[2][2], rMat[3][2],
        rMat[0][3], rMat[1][3], rMat[2][3], rMat[3][3]
    };
    aOpenGL.MatrixMode(GL_TEXTURE);
    aOpenGL.LoadMatrixd(fBuffer);
}

void Base3DOpenGL::PostSetViewport(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    Rectangle aBoundPixel(GetOutputDevice()->
        LogicToPixel(pCaller->GetLogicalViewportBounds()));
    aOpenGL.Viewport( aBoundPixel.Left(), aBoundPixel.Top(),
        aBoundPixel.GetWidth() - 1, aBoundPixel.GetHeight() - 1);
}

/*************************************************************************
|*
|* Ein Objekt in Form einer B3dGeometry direkt ausgeben
|*
\************************************************************************/

void Base3DOpenGL::DrawPolygonGeometry(B3dGeometry& rGeometry, BOOL bOutline)
{
    // bForceToSinglePrimitiveOutput: (#70626#)
    if(bForceToSinglePrimitiveOutput || (GetShadeModel() == Base3DPhong && GetRenderMode() == Base3DRenderFill))
    {
        // call parent, render with many primitives
        Base3D::DrawPolygonGeometry(rGeometry, bOutline);
    }
    else
    {
        // Buckets der Geometrie holen
        B3dEntityBucket& rEntityBucket = rGeometry.GetEntityBucket();
        GeometryIndexValueBucket& rIndexBucket = rGeometry.GetIndexBucket();

        if(rEntityBucket.Count() && rIndexBucket.Count())
        {
            // Arrays aktivieren
            aOpenGL.EnableClientState(GL_VERTEX_ARRAY);

            UINT32 nPolyCounter = 0;
            UINT32 nEntityCounter = 0;
            UINT32 nArrayStartIndex = 0;
            UINT32 nUpperBound;

            // Pointer setzen
            UINT16 nArray = 0;
            aOpenGL.VertexPointer(3, GL_DOUBLE, rEntityBucket.GetSlotSize(), &rEntityBucket[0].Point());

            if(bOutline)
            {
                // Transparenz Linien beachten
                if(GetColor().GetTransparency())
                {
                    aOpenGL.Enable( GL_BLEND );
                    aOpenGL.DepthMask( FALSE );
                    aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }
                else
                {
                    aOpenGL.Disable( GL_BLEND );
                    aOpenGL.DepthMask( TRUE );
                }

                // ALLE Linien Zeichnen
                aOpenGL.Disable(GL_CULL_FACE);

                // Polygone als Outline ausgeben
                aOpenGL.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                aOpenGL.PolygonOffset((float)(fOffFacMul100 / 100.0), (float)(fOffUniMul100 / 100.0));
                aOpenGL.Enable( GL_POLYGON_OFFSET_LINE );
                aOpenGL.EnableClientState(GL_EDGE_FLAG_ARRAY);
                aOpenGL.EdgeFlagPointer(rEntityBucket.GetSlotSize(), &rEntityBucket[0].EdgeFlag());

                while(nPolyCounter < rIndexBucket.Count())
                {
                    // Naechstes Primitiv
                    nUpperBound = rIndexBucket[nPolyCounter].GetIndex();
                    BOOL bLineMode = (rIndexBucket[nPolyCounter++].GetMode() == B3D_INDEX_MODE_LINE);

                    if(nUpperBound >> rEntityBucket.GetBlockShift() != nArray)
                    {
                        // Einzelschritt, Bereichsueberschreitung im Buffer
                        // Als Polygon ausgeben
                        aOpenGL.Begin(bLineMode ? Base3DLineStrip : Base3DPolygon);

                        // Polygon ausgeben
                        while(nEntityCounter < nUpperBound)
                        {
                            B3dEntity& rEntity = rEntityBucket[nEntityCounter++];
                            aOpenGL.EdgeFlag(rEntity.IsEdgeVisible() ? GL_TRUE : GL_FALSE);
                            aOpenGL.Vertex3dv((const double *)(&rEntity.Point()));
                        }

                        // Primitiv abschliessen
                        aOpenGL.End();

                        // NUR auf neues Array setzen, wenn noch was da ist (#59941#)
                        if(nEntityCounter < rEntityBucket.Count())
                        {
                            // Pointer auf neues Array setzen
                            nArray = (UINT16)(nEntityCounter >> rEntityBucket.GetBlockShift());
                            nArrayStartIndex = nEntityCounter;
                            B3dEntity& rStart = rEntityBucket[nEntityCounter];

                            aOpenGL.VertexPointer(3, GL_DOUBLE, rEntityBucket.GetSlotSize(), &rStart);
                            aOpenGL.EdgeFlagPointer(rEntityBucket.GetSlotSize(), &(rStart.EdgeFlag()));
                        }
                    }
                    else
                    {
                        // Primitiv komplett raushauen, liegt in einem Buffer
                        aOpenGL.DrawArrays(bLineMode ? Base3DLineStrip : Base3DPolygon,
                            nEntityCounter - nArrayStartIndex,
                            nUpperBound - nEntityCounter);
                        nEntityCounter = nUpperBound;
                    }
                }

                // Arrays deaktivieren
                aOpenGL.DisableClientState(GL_VERTEX_ARRAY);
                aOpenGL.DisableClientState(GL_EDGE_FLAG_ARRAY);
            }
            else
            {
                // Transparenz Flaechen beachten
                if(GetMaterial(Base3DMaterialDiffuse).GetTransparency())
                {
                    aOpenGL.Enable( GL_BLEND );
                    aOpenGL.DepthMask( FALSE );
                    aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }
                else
                {
                    aOpenGL.Disable( GL_BLEND );
                    aOpenGL.DepthMask( TRUE );
                }

                // Polygone gefuellt ausgeben
                aOpenGL.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                aOpenGL.EnableClientState(GL_NORMAL_ARRAY);
                aOpenGL.EnableClientState(GL_TEXTURE_COORD_ARRAY);
                if(GetForceFlat() || GetShadeModel() == Base3DFlat)
                    aOpenGL.NormalPointer(GL_DOUBLE, rEntityBucket.GetSlotSize(), &rEntityBucket[0].PlaneNormal());
                else
                    aOpenGL.NormalPointer(GL_DOUBLE, rEntityBucket.GetSlotSize(), &rEntityBucket[0].Normal());
                aOpenGL.TexCoordPointer(2, GL_DOUBLE, rEntityBucket.GetSlotSize(), &rEntityBucket[0].TexCoor());

                while(nPolyCounter < rIndexBucket.Count())
                {
                    // Naechstes Primitiv
                    nUpperBound = rIndexBucket[nPolyCounter].GetIndex();
                    BOOL bLineMode = (rIndexBucket[nPolyCounter++].GetMode() == B3D_INDEX_MODE_LINE);

                    if(nUpperBound >> rEntityBucket.GetBlockShift() != nArray)
                    {
                        // Einzelschritt, Bereichsueberschreitung im Buffer
                        // Als Polygon ausgeben
                        aOpenGL.Begin(bLineMode ? Base3DLineStrip : Base3DPolygon);

                        // Polygon ausgeben
                        while(nEntityCounter < nUpperBound)
                        {
                            B3dEntity& rEntity = rEntityBucket[nEntityCounter++];
                            if(GetForceFlat() || GetShadeModel() == Base3DFlat)
                                aOpenGL.Normal3dv((const double *)(&rEntity.PlaneNormal()));
                            else
                                aOpenGL.Normal3dv((const double *)(&rEntity.Normal()));
                            aOpenGL.TexCoord3dv((const double *)(&rEntity.TexCoor()));
                            aOpenGL.Vertex3dv((const double *)(&rEntity.Point()));
                        }

                        // Primitiv abschliessen
                        aOpenGL.End();

                        // NUR auf neues Array setzen, wenn noch was da ist (#58702#)
                        if(nEntityCounter < rEntityBucket.Count())
                        {
                            // Pointer auf neues Array setzen
                            nArray = (UINT16)(nEntityCounter >> rEntityBucket.GetBlockShift());
                            nArrayStartIndex = nEntityCounter;
                            B3dEntity& rStart = rEntityBucket[nEntityCounter];

                            aOpenGL.VertexPointer(3, GL_DOUBLE, rEntityBucket.GetSlotSize(), &rStart);
                            if(GetForceFlat() || GetShadeModel() == Base3DFlat)
                                aOpenGL.NormalPointer(GL_DOUBLE, rEntityBucket.GetSlotSize(), &(rStart.PlaneNormal()));
                            else
                                aOpenGL.NormalPointer(GL_DOUBLE, rEntityBucket.GetSlotSize(), &(rStart.Normal()));
                            aOpenGL.TexCoordPointer(2, GL_DOUBLE, rEntityBucket.GetSlotSize(), &(rStart.TexCoor()));
                        }
                    }
                    else
                    {
                        // Primitiv komplett raushauen, liegt in einem Buffer
                        aOpenGL.DrawArrays(bLineMode ? Base3DLineStrip : Base3DPolygon,
                            nEntityCounter - nArrayStartIndex,
                            nUpperBound - nEntityCounter);
                        nEntityCounter = nUpperBound;
                    }
                }

                // Arrays deaktivieren
                aOpenGL.DisableClientState(GL_VERTEX_ARRAY);
                aOpenGL.DisableClientState(GL_NORMAL_ARRAY);
                aOpenGL.DisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
    }
}

