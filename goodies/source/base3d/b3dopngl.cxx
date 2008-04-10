/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dopngl.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#include <vcl/outdev.hxx>
#include <tools/debug.hxx>
#include <svtools/options3d.hxx>
#include "b3dopngl.hxx"

//#ifndef _B3D_HMATRIX_HXX
//#include "hmatrix.hxx"
//#endif
#include "b3dtex.hxx"
#include "b3dtrans.hxx"
#include <goodies/b3dgeom.hxx>

/*************************************************************************
|*
|* Konstruktor Base3DOpenGL
|*
\************************************************************************/

Base3DOpenGL::Base3DOpenGL(OutputDevice* pOutDev)
:   Base3D(pOutDev),
    aOpenGL(pOutDev),
    aLastNormal(DBL_MAX, DBL_MAX, DBL_MAX),
    aLastTexCoor(DBL_MAX, DBL_MAX),
    fOffFacMul100((float)(-0.2 * 100.0)),
    fOffUniMul100((float)(-1.0 * 100.0)),
    aPhongBuffer(12),       // 4K
    nPhongDivideSize(20),
    bForceToSinglePrimitiveOutput(sal_True) // (#70626#)
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

sal_uInt16 Base3DOpenGL::GetBase3DType()
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
    aOpenGL.DepthMask( sal_True );

    // OutputDevice setzen und ZBuffer loeschen
    aOpenGL.SetConnectOutputDevice(GetOutputDevice());
    aOpenGL.Clear( GL_DEPTH_BUFFER_BIT );
}

/*************************************************************************
|*
|* Scissoring Region setzen
|*
\************************************************************************/

void Base3DOpenGL::SetScissorRegionPixel(const Rectangle& rRect, sal_Bool bActivate)
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

void Base3DOpenGL::SetDither(sal_Bool bNew)
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

void Base3DOpenGL::ActivateScissorRegion(sal_Bool bNew)
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
        aOpenGL.DepthMask( sal_False );
        aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
        aOpenGL.Disable( GL_BLEND );
        aOpenGL.DepthMask( sal_True );
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
    sal_uInt32 aCount = aPhongBuffer.Count();
    sal_uInt32 aPos(0L);
    bPhongBufferedMode = sal_False;
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
        default:
            break;  // -Wall multiple values not handled.
    }
    aOpenGL.End();
}

/*************************************************************************
|*
|* Zeichne fuer Phong gebufferte Dreiecke
|*
\************************************************************************/

void Base3DOpenGL::DrawPhongTriangle(sal_uInt32 nInd1, sal_uInt32 nInd2, sal_uInt32 nInd3)
{
    basegfx::B3DPoint aPos1 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd1].Point());
    double fXMin = aPos1.getX();
    double fXMax = aPos1.getX();
    double fYMin = aPos1.getY();
    double fYMax = aPos1.getY();
    basegfx::B3DPoint aPos2 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd2].Point());
    if(aPos2.getX() < fXMin)
        fXMin = aPos2.getX();
    if(aPos2.getX() > fXMax)
        fXMax = aPos2.getX();
    if(aPos2.getY() < fYMin)
        fYMin = aPos2.getY();
    if(aPos2.getY() > fYMax)
        fYMax = aPos2.getY();
    aPos2 = GetTransformationSet()->ObjectToViewCoor(aPhongBuffer[nInd3].Point());
    if(aPos2.getX() < fXMin)
        fXMin = aPos2.getX();
    if(aPos2.getX() > fXMax)
        fXMax = aPos2.getX();
    if(aPos2.getY() < fYMin)
        fYMin = aPos2.getY();
    if(aPos2.getY() > fYMax)
        fYMax = aPos2.getY();

    Size aPixelSize = GetOutputDevice()->LogicToPixel(
        Size((long)(fXMax - fXMin),(long)(fYMax - fYMin)));
    if(aPixelSize.Width() * aPixelSize.Height() > nInternPhongDivideSize)
    {
        sal_uInt32 aCount = aPhongBuffer.Count();
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

// hack to get the address of the mfX member for OpenGL
class LocVecAcc : public basegfx::B3DVector
{
public:
    double& getXDirect() { return mfX; }
};

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
                    aOpenGL.Normal3dv(&(((LocVecAcc*)&aLastNormal)->getXDirect()));
                }
            }
            else
            {
                if(rEntity.Normal() != aLastNormal)
                {
                    aLastNormal = rEntity.Normal();
                    aOpenGL.Normal3dv(&(((LocVecAcc*)&aLastNormal)->getXDirect()));
                }
            }
        }
        else
        {
            if(!aLastNormal.equalZero())
            {
                aLastNormal = basegfx::B3DVector(0.0, 0.0, 0.0);
                aOpenGL.Normal3dv(&(((LocVecAcc*)&aLastNormal)->getXDirect()));
            }
        }

        // Texturkoordinate setzen
        if(rEntity.IsTexCoorUsed())
        {
            if(rEntity.TexCoor() != aLastTexCoor)
            {
                aLastTexCoor = rEntity.TexCoor();
                aOpenGL.TexCoord2dv(&(((LocVecAcc*)&aLastTexCoor)->getXDirect()));
            }
        }
        else
        {
            if(!aLastTexCoor.equalZero())
            {
                aLastTexCoor = basegfx::B2DPoint(0.0, 0.0);
                aOpenGL.TexCoord2dv(&(((LocVecAcc*)&aLastTexCoor)->getXDirect()));
            }
        }

        // Punkt erzeugen
        aOpenGL.Vertex3dv(&(((LocVecAcc*)&(rEntity.Point()))->getXDirect()));
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

void Base3DOpenGL::SetShininess(sal_uInt16 nExponent,
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

B3dTexture* Base3DOpenGL::CreateTexture(TextureAttributes& rAtt, BitmapEx& rBitmapEx)
{
    // Hier Parent NICHT rufen! Sonst wird auch noch eine normale Textur erzeugt
    B3dTextureOpenGL* pRetval = new B3dTextureOpenGL(rAtt, rBitmapEx, aOpenGL);
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

void Base3DOpenGL::SetPolygonOffset(Base3DPolygonOffset eNew, sal_Bool bNew)
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

void Base3DOpenGL::SetLightGroup(B3dLightGroup* pSet, sal_Bool bSetGlobal)
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
        sal_uInt16 nNumAlloc = 0;

        sal_uInt16 a;
        for(a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
        {
            Base3DLightNumber eNum = (Base3DLightNumber)(Base3DLight0 + a);
            B3dLight& rLight = GetLightGroup()->GetLightObject(eNum);

            if(rLight.IsEnabled())
            {
                Base3DLightNumber eNumAlloc = (Base3DLightNumber)(Base3DLight0 + nNumAlloc);
                nNumAlloc++;

                Enable(sal_True, eNumAlloc);

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
            Enable(sal_False, eNum);
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

void Base3DOpenGL::SetLocalViewer(sal_Bool bNew)
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

void Base3DOpenGL::SetModelTwoSide(sal_Bool bNew)
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

void Base3DOpenGL::EnableLighting(sal_Bool bNew)
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

void Base3DOpenGL::SetPosition(const basegfx::B3DPoint& rNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew.getX(), (float)rNew.getY(), (float)rNew.getZ(), (float)1.0
    };
    aOpenGL.Lightfv(eLight, GL_POSITION, fArray);
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetDirection(const basegfx::B3DVector& rNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew.getX(), (float)rNew.getY(), (float)rNew.getZ(), (float)0.0
    };
    aOpenGL.Lightfv(eLight, GL_POSITION, fArray);
}

/*************************************************************************
|*
|* Die Richtung einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetSpotDirection(const basegfx::B3DVector& rNew, Base3DLightNumber eNum)
{
    // OpenGL Specifics
    GLenum eLight = GL_LIGHT0 + (eNum - Base3DLight0);
    float fArray[4] = {
        (float)rNew.getX(), (float)rNew.getY(), (float)rNew.getZ(), (float)0.0
    };
    aOpenGL.Lightfv(eLight, GL_SPOT_DIRECTION, fArray);
}

/*************************************************************************
|*
|* Den SpotExponent einer Lichtquelle setzen
|*
\************************************************************************/

void Base3DOpenGL::SetSpotExponent(sal_uInt16 nNew, Base3DLightNumber eNum)
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

void Base3DOpenGL::Enable(sal_Bool bNew, Base3DLightNumber eNum)
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

void Base3DOpenGL::SetEdgeFlag(sal_Bool bNew)
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
    basegfx::B3DHomMatrix aMat = pCaller->GetObjectTrans();
    aMat *= pCaller->GetOrientation();
    double fBuffer[16] = {
        aMat.get(0, 0), aMat.get(1, 0), aMat.get(2, 0), aMat.get(3, 0),
        aMat.get(0, 1), aMat.get(1, 1), aMat.get(2, 1), aMat.get(3, 1),
        aMat.get(0, 2), aMat.get(1, 2), aMat.get(2, 2), aMat.get(3, 2),
        aMat.get(0, 3), aMat.get(1, 3), aMat.get(2, 3), aMat.get(3, 3)
    };
    aOpenGL.MatrixMode(GL_MODELVIEW);
    aOpenGL.LoadMatrixd(fBuffer);
}

void Base3DOpenGL::PostSetProjection(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    const basegfx::B3DHomMatrix& rMat = pCaller->GetProjection();
    double fBuffer[16] = {
        rMat.get(0, 0), rMat.get(1, 0), rMat.get(2, 0), rMat.get(3, 0),
        rMat.get(0, 1), rMat.get(1, 1), rMat.get(2, 1), rMat.get(3, 1),
        rMat.get(0, 2), rMat.get(1, 2), rMat.get(2, 2), rMat.get(3, 2),
        rMat.get(0, 3), rMat.get(1, 3), rMat.get(2, 3), rMat.get(3, 3)
    };
    aOpenGL.MatrixMode(GL_PROJECTION);
    aOpenGL.LoadMatrixd(fBuffer);
}

void Base3DOpenGL::PostSetTexture(B3dTransformationSet* pCaller)
{
    // OpenGL specifics
    const basegfx::B2DHomMatrix& rMat = pCaller->GetTexture();
    double fBuffer[16] = {
        rMat.get(0, 0), rMat.get(1, 0),            0.0, rMat.get(2, 0),
        rMat.get(0, 1), rMat.get(1, 1),            0.0, rMat.get(2, 1),
                   0.0,            0.0,            0.0,            0.0,
        rMat.get(0, 2), rMat.get(1, 2),            0.0, rMat.get(3, 3)
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

void Base3DOpenGL::DrawPolygonGeometry(const B3dGeometry& rGeometry, sal_Bool bOutline)
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
        const B3dEntityBucket& rEntityBucket = rGeometry.GetEntityBucket();
        const GeometryIndexValueBucket& rIndexBucket = rGeometry.GetIndexBucket();

        if(rEntityBucket.Count() && rIndexBucket.Count())
        {
            // Arrays aktivieren
            aOpenGL.EnableClientState(GL_VERTEX_ARRAY);

            sal_uInt32 nPolyCounter = 0;
            sal_uInt32 nEntityCounter = 0;
            sal_uInt32 nArrayStartIndex = 0;
            sal_uInt32 nUpperBound;

            // Pointer setzen
            sal_uInt16 nArray = 0;
            aOpenGL.VertexPointer(3, GL_DOUBLE, rEntityBucket.GetSlotSize(), &rEntityBucket[0].Point());

            if(bOutline)
            {
                // Transparenz Linien beachten
                if(GetColor().GetTransparency())
                {
                    aOpenGL.Enable( GL_BLEND );
                    aOpenGL.DepthMask( sal_False );
                    aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }
                else
                {
                    aOpenGL.Disable( GL_BLEND );
                    aOpenGL.DepthMask( sal_True );
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
                    sal_Bool bLineMode = (rIndexBucket[nPolyCounter++].GetMode() == B3D_INDEX_MODE_LINE);

                    if(nUpperBound >> rEntityBucket.GetBlockShift() != nArray)
                    {
                        // Einzelschritt, Bereichsueberschreitung im Buffer
                        // Als Polygon ausgeben
                        aOpenGL.Begin(bLineMode ? Base3DLineStrip : Base3DPolygon);

                        // Polygon ausgeben
                        while(nEntityCounter < nUpperBound)
                        {
                            const B3dEntity& rEntity = rEntityBucket[nEntityCounter++];
                            aOpenGL.EdgeFlag(rEntity.IsEdgeVisible() ? GL_TRUE : GL_FALSE);
                            aOpenGL.Vertex3dv((const double *)(&rEntity.Point()));
                        }

                        // Primitiv abschliessen
                        aOpenGL.End();

                        // NUR auf neues Array setzen, wenn noch was da ist (#59941#)
                        if(nEntityCounter < rEntityBucket.Count())
                        {
                            // Pointer auf neues Array setzen
                            nArray = (sal_uInt16)(nEntityCounter >> rEntityBucket.GetBlockShift());
                            nArrayStartIndex = nEntityCounter;
                            const B3dEntity& rStart = rEntityBucket[nEntityCounter];

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
                if(GetMaterial(Base3DMaterialDiffuse).GetTransparency()
                    || (GetActiveTexture() && !!(GetActiveTexture()->GetAlphaMask())))
                {
                    aOpenGL.Enable( GL_BLEND );
                    aOpenGL.DepthMask( sal_False );
                    aOpenGL.BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }
                else
                {
                    aOpenGL.Disable( GL_BLEND );
                    aOpenGL.DepthMask( sal_True );
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
                    sal_Bool bLineMode = (rIndexBucket[nPolyCounter++].GetMode() == B3D_INDEX_MODE_LINE);

                    if(nUpperBound >> rEntityBucket.GetBlockShift() != nArray)
                    {
                        // Einzelschritt, Bereichsueberschreitung im Buffer
                        // Als Polygon ausgeben
                        aOpenGL.Begin(bLineMode ? Base3DLineStrip : Base3DPolygon);

                        // Polygon ausgeben
                        while(nEntityCounter < nUpperBound)
                        {
                            const B3dEntity& rEntity = rEntityBucket[nEntityCounter++];
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
                            nArray = (sal_uInt16)(nEntityCounter >> rEntityBucket.GetBlockShift());
                            nArrayStartIndex = nEntityCounter;
                            const B3dEntity& rStart = rEntityBucket[nEntityCounter];

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

