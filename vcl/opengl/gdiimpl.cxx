/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "openglgdiimpl.hxx"

#include <vcl/gradient.hxx>
#include <salframe.hxx>
#include "salvd.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>
#include "salgdi.hxx"
#include "svdata.hxx"
#include "opengl/salbmp.hxx"

#include <vector>

OpenGLSalGraphicsImpl::OpenGLSalGraphicsImpl(SalGeometryProvider* pParent)
    : mpContext(0)
    , mpParent(pParent)
    , mpFramebuffer(NULL)
    , mpProgram(NULL)
    , mbUseScissor(false)
    , mbUseStencil(false)
    , mbOffscreen(false)
    , mnLineColor(SALCOLOR_NONE)
    , mnFillColor(SALCOLOR_NONE)
{
}

OpenGLSalGraphicsImpl::~OpenGLSalGraphicsImpl()
{
    ReleaseContext();
}

OpenGLContext* OpenGLSalGraphicsImpl::GetOpenGLContext()
{
    if( !mpContext )
        AcquireContext();
    return mpContext;
}

OpenGLContext* OpenGLSalGraphicsImpl::GetDefaultContext()
{
    return ImplGetDefaultWindow()->GetGraphics()->GetOpenGLContext();
}

bool OpenGLSalGraphicsImpl::AcquireContext( )
{
    ImplSVData* pSVData = ImplGetSVData();

    if( mpContext )
        mpContext->DeRef();


    OpenGLContext* pContext = pSVData->maGDIData.mpLastContext;
    while( pContext )
    {
        // check if this context can be used by this SalGraphicsImpl instance
        if( UseContext( pContext )  )
            break;
        pContext = pContext->mpPrevContext;
    }

    if( pContext )
        pContext->AddRef();
    else
        pContext = mbOffscreen ? GetDefaultContext() : CreateWinContext();

    mpContext = pContext;
    return (mpContext != NULL);
}

bool OpenGLSalGraphicsImpl::ReleaseContext()
{
    if( mpContext )
        mpContext->DeRef();
    mpContext = NULL;
    return true;
}

void OpenGLSalGraphicsImpl::Init()
{
    mbOffscreen = IsOffscreen();

    // check if we can simply re-use the same context
    if( mpContext )
    {
        if( !UseContext( mpContext ) )
            ReleaseContext();
    }

    // reset the offscreen texture
    if( !mbOffscreen ||
        maOffscreenTex.GetWidth()  != GetWidth() ||
        maOffscreenTex.GetHeight() != GetHeight() )
    {
        maOffscreenTex = OpenGLTexture();
    }
}

void OpenGLSalGraphicsImpl::PreDraw()
{
    if( !mpContext && !AcquireContext() )
    {
        SAL_WARN( "vcl.opengl", "Couldn't acquire context" );
        return;
    }

    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    if( !mbOffscreen )
        mpContext->AcquireDefaultFramebuffer();
    else
        CheckOffscreenTexture();
    CHECK_GL_ERROR();

    glViewport( 0, 0, GetWidth(), GetHeight() );
    ImplInitClipRegion();

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::PostDraw()
{
    if( !mbOffscreen && mpContext->mnPainting == 0 )
        glFlush();
    if( mbUseScissor )
        glDisable( GL_SCISSOR_TEST );
    if( mbUseStencil )
        glDisable( GL_STENCIL_TEST );
    if( mpProgram )
    {
        mpProgram->Clean();
        mpProgram = NULL;
    }

    mpContext->ReleaseFramebuffer( mpFramebuffer );
    mpFramebuffer = NULL;

    CHECK_GL_ERROR();

    // release the context as there is no guarantee the underlying window
    // will still be valid for the next draw operation
    if( mbOffscreen )
        ReleaseContext();
}

void OpenGLSalGraphicsImpl::freeResources()
{
    // TODO Delete shaders, programs and textures if not shared
}

void OpenGLSalGraphicsImpl::ImplSetClipBit( const vcl::Region& rClip, GLuint nMask )
{
    glEnable( GL_STENCIL_TEST );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    glStencilMask( nMask );
    glStencilFunc( GL_NEVER, nMask, 0xFF );
    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP );

    glClear( GL_STENCIL_BUFFER_BIT );
    if( UseSolid( MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ) ) )
    {
        if( rClip.getRegionBand() )
            DrawRegionBand( *rClip.getRegionBand() );
        else
            DrawPolyPolygon( rClip.GetAsB2DPolyPolygon() );
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilMask( 0x00 );
    glDisable( GL_STENCIL_TEST );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::ImplInitClipRegion()
{
    // make sure the context has the right clipping set
    if( maClipRegion != mpContext->maClipRegion )
    {
        mpContext->maClipRegion = maClipRegion;
        if( maClipRegion.IsRectangle() )
        {
            Rectangle aRect( maClipRegion.GetBoundRect() );
            glScissor( aRect.Left(), GetHeight() - aRect.Bottom() - 1, aRect.GetWidth() + 1, aRect.GetHeight() + 1 );
        }
        else if( !maClipRegion.IsEmpty() )
        {
            ImplSetClipBit( maClipRegion, 0x01 );
        }
    }

    if( mbUseScissor )
        glEnable( GL_SCISSOR_TEST );
    if( mbUseStencil )
    {
        glStencilFunc( GL_EQUAL, 1, 0x1 );
        glEnable( GL_STENCIL_TEST );
    }

    CHECK_GL_ERROR();
}

bool OpenGLSalGraphicsImpl::setClipRegion( const vcl::Region& rClip )
{
    SAL_INFO( "vcl.opengl", "::setClipRegion " << rClip );
    maClipRegion = rClip;

    mbUseStencil = false;
    mbUseScissor = false;
    if( maClipRegion.IsRectangle() )
        mbUseScissor = true;
    else if ( !maClipRegion.IsEmpty() )
        mbUseStencil = true;

    return true;
}

// set the clip region to empty
void OpenGLSalGraphicsImpl::ResetClipRegion()
{
    SAL_INFO( "vcl.opengl", "::ResetClipRegion" );
    maClipRegion.SetEmpty();
    mbUseScissor = false;
    mbUseStencil = false;
}

// get the depth of the device
sal_uInt16 OpenGLSalGraphicsImpl::GetBitCount() const
{
    return 32;
}

// get the width of the device
long OpenGLSalGraphicsImpl::GetGraphicsWidth() const
{
    return GetWidth();
}

// set the line color to transparent (= don't draw lines)
void OpenGLSalGraphicsImpl::SetLineColor()
{
    if( mnLineColor != SALCOLOR_NONE )
    {
        mnLineColor = SALCOLOR_NONE;
    }
}

// set the line color to a specific color
void OpenGLSalGraphicsImpl::SetLineColor( SalColor nSalColor )
{
    if( mnLineColor != nSalColor )
    {
        mnLineColor = nSalColor;
    }
}

// set the fill color to transparent (= don't fill)
void OpenGLSalGraphicsImpl::SetFillColor()
{
    if( mnFillColor != SALCOLOR_NONE )
    {
        mnFillColor = SALCOLOR_NONE;
    }
}

// set the fill color to a specific color, shapes will be
// filled accordingly
void OpenGLSalGraphicsImpl::SetFillColor( SalColor nSalColor )
{
    if( mnFillColor != nSalColor )
    {
        mnFillColor = nSalColor;
    }
}

// enable/disable XOR drawing
void OpenGLSalGraphicsImpl::SetXORMode( bool /*bSet*/, bool /*bInvertOnly*/ )
{
}

// set line color for raster operations
void OpenGLSalGraphicsImpl::SetROPLineColor( SalROPColor /*nROPColor*/ )
{
}

// set fill color for raster operations
void OpenGLSalGraphicsImpl::SetROPFillColor( SalROPColor /*nROPColor*/ )
{
}

bool OpenGLSalGraphicsImpl::CheckOffscreenTexture()
{
    if( !maOffscreenTex )
        maOffscreenTex = OpenGLTexture( GetWidth(), GetHeight() );

    if( !maOffscreenTex.IsUnique() )
    {
        GLfloat fWidth = GetWidth();
        GLfloat fHeight = GetHeight();
        SalTwoRect aPosAry(0, 0, fWidth, fHeight, 0,0, fWidth, fHeight);

        // TODO: lfrb: User GL_ARB_copy_image?
        OpenGLTexture aNewTex = OpenGLTexture( GetWidth(), GetHeight() );
        mpFramebuffer = mpContext->AcquireFramebuffer( aNewTex );
        DrawTexture( maOffscreenTex, aPosAry );
        maOffscreenTex = aNewTex;
    }
    else
    {
        mpFramebuffer = mpContext->AcquireFramebuffer( maOffscreenTex );
    }

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader )
{
    mpProgram = mpContext->UseProgram( rVertexShader, rFragmentShader );
    return ( mpProgram != NULL );
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor, sal_uInt8 nTransparency )
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !UseProgram( "dumbVertexShader", "solidFragmentShader" ) )
        return false;
    mpProgram->SetColor( "color", nColor, nTransparency );
    return true;
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor, double fTransparency )
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !UseProgram( "dumbVertexShader", "solidFragmentShader" ) )
        return false;
    mpProgram->SetColorf( "color", nColor, fTransparency );
    return true;
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor )
{
    return UseSolid( nColor, 0.0f );
}

bool OpenGLSalGraphicsImpl::UseInvert()
{
    if( !UseSolid( MAKE_SALCOLOR( 255, 255, 255 ) ) )
        return false;
    mpProgram->SetBlendMode( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
    return true;
}

void OpenGLSalGraphicsImpl::DrawPoint( long nX, long nY )
{
    GLfloat pPoint[2];

    pPoint[0] = 2 * nX / GetWidth() - 1.0f;
    pPoint[1] = 1.0f - 2 * nY / GetHeight();

    mpProgram->SetVertices( pPoint );
    glDrawArrays( GL_POINTS, 0, 1 );
}

void OpenGLSalGraphicsImpl::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    GLfloat pPoints[4];

    pPoints[0] = (2 * nX1) / GetWidth() - 1.0;
    pPoints[1] = 1.0f - 2 * nY1 / GetHeight();
    pPoints[2] = (2 * nX2) / GetWidth() - 1.0;;
    pPoints[3] = 1.0f - 2 * nY2 / GetHeight();

    mpProgram->SetVertices( pPoints );
    glDrawArrays( GL_LINES, 0, 2 );
}

void OpenGLSalGraphicsImpl::DrawLines( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose )
{
    std::vector<GLfloat> aPoints(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++ )
    {
        aPoints[j++] = (2 * pPtAry[i].mnX) / GetWidth()  - 1.0f;
        aPoints[j++] = 1.0f - (2 * pPtAry[i].mnY) / GetHeight();
    }

    mpProgram->SetVertices( &aPoints[0] );
    if( bClose )
        glDrawArrays( GL_LINE_LOOP, 0, nPoints );
    else
        glDrawArrays( GL_LINE_STRIP, 0, nPoints );
}

void OpenGLSalGraphicsImpl::DrawConvexPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    std::vector<GLfloat> aVertices(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++, j += 2 )
    {
        aVertices[j] = (2 * pPtAry[i].mnX) / GetWidth() - 1.0;
        aVertices[j+1] = 1.0 - (2 * pPtAry[i].mnY / GetHeight());
    }

    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
}

void OpenGLSalGraphicsImpl::DrawConvexPolygon( const Polygon& rPolygon )
{
    sal_uInt16 nPoints = rPolygon.GetSize() - 1;
    std::vector<GLfloat> aVertices(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++, j += 2 )
    {
        const Point& rPt = rPolygon.GetPoint( i );
        aVertices[j] = (2 * rPt.X()) / GetWidth() - 1.0;
        aVertices[j+1] = 1.0 - (2 * rPt.Y() / GetHeight());
    }

    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
}

void OpenGLSalGraphicsImpl::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    long nX1( nX );
    long nY1( nY );
    long nX2( nX + nWidth );
    long nY2( nY + nHeight );
    const SalPoint aPoints[] = { { nX1, nY2 }, { nX1, nY1 },
                                 { nX2, nY1 }, { nX2, nY2 }};

    DrawConvexPolygon( 4, aPoints );
}

void OpenGLSalGraphicsImpl::DrawRect( const Rectangle& rRect )
{
    long nX1( rRect.Left() );
    long nY1( rRect.Top() );
    long nX2( rRect.Right() );
    long nY2( rRect.Bottom() );
    const SalPoint aPoints[] = { { nX1, nY2 }, { nX1, nY1 },
                                 { nX2, nY1 }, { nX2, nY2 }};

    DrawConvexPolygon( 4, aPoints );
}

void OpenGLSalGraphicsImpl::DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    ::basegfx::B2DPolygon aPolygon;

    for( sal_uInt32 i = 0; i < nPoints; i++ )
        aPolygon.append( ::basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
    aPolygon.setClosed( true );

    if( ::basegfx::tools::isConvex( aPolygon ) )
    {
        if( nPoints > 2L )
            DrawConvexPolygon( nPoints, pPtAry );
    }
    else
    {
        const ::basegfx::B2DPolyPolygon aPolyPolygon( aPolygon );
        DrawPolyPolygon( aPolyPolygon );
    }
}

void OpenGLSalGraphicsImpl::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    ::std::vector< GLfloat > aVertices;
    GLfloat nWidth = GetWidth();
    GLfloat nHeight = GetHeight();
    const ::basegfx::B2DPolyPolygon& aSimplePolyPolygon = ::basegfx::tools::solveCrossovers( rPolyPolygon );

    for( sal_uInt32 i = 0; i < aSimplePolyPolygon.count(); i++ )
    {
        const basegfx::B2DPolygon& rPolygon( aSimplePolyPolygon.getB2DPolygon( i ) );
        const ::basegfx::B2DPolygon& aResult(
            ::basegfx::triangulator::triangulate( rPolygon ) );

        for( sal_uInt32 j = 0; j < aResult.count(); j++ )
        {
            const ::basegfx::B2DPoint& rPt( aResult.getB2DPoint( j ) );
            aVertices.push_back( 2 * rPt.getX() / nWidth - 1.0f );
            aVertices.push_back( 1.0f - 2 * rPt.getY() / nHeight );
        }
    }

    mpProgram->SetVertices( aVertices.data() );
    glDrawArrays( GL_TRIANGLES, 0, aVertices.size() / 2 );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawRegionBand( const RegionBand& rRegion )
{
    RectangleVector aRects;
    std::vector<GLfloat> aVertices;
    rRegion.GetRegionRectangles( aRects );

    if( aRects.empty() )
        return;

#define ADD_VERTICE(pt) \
    aVertices.push_back( 2 * pt.X() / GetWidth() - 1.0 ); \
    aVertices.push_back( 1.0 - (2 * pt.Y() / GetHeight()) );

    for( size_t i = 0; i < aRects.size(); ++i )
    {
        aRects[i].Bottom() += 1;
        aRects[i].Right() += 1;
        ADD_VERTICE( aRects[i].TopLeft() );
        ADD_VERTICE( aRects[i].TopRight() );
        ADD_VERTICE( aRects[i].BottomLeft() );
        ADD_VERTICE( aRects[i].BottomLeft() );
        ADD_VERTICE( aRects[i].TopRight() );
        ADD_VERTICE( aRects[i].BottomRight() );
    }

#undef ADD_VERTICE

    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLES, 0, aVertices.size() / 2 );
}

void OpenGLSalGraphicsImpl::DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted )
{
    GLfloat aTexCoord[8];
    rTexture.GetCoord( aTexCoord, rPosAry, bInverted );
    mpProgram->SetTextureCoord( aTexCoord );
    DrawRect( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight );
}

void OpenGLSalGraphicsImpl::DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& pPosAry, bool bInverted )
{
    if( !UseProgram( "textureVertexShader", "textureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    DrawTextureRect( rTexture, pPosAry, bInverted );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawTransformedTexture(
    OpenGLTexture& rTexture,
    OpenGLTexture& rMask,
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY )
{
    GLfloat aVertices[8] = {
        0, (float) rTexture.GetHeight(), 0, 0,
        (float) rTexture.GetWidth(), 0, (float) rTexture.GetWidth(), (float) rTexture.GetHeight() };
    GLfloat aTexCoord[8];

    if( rMask )
    {
        if( !UseProgram( "transformedTextureVertexShader", "maskedTextureFragmentShader" ) )
            return;
        mpProgram->SetTexture( "mask", rMask );
        rMask.SetFilter( GL_LINEAR );
        mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
        if( !UseProgram( "transformedTextureVertexShader", "textureFragmentShader" ) )
            return;
    }

    mpProgram->SetUniform2f( "viewport", GetWidth(), GetHeight() );
    mpProgram->SetTransform( "transform", rTexture, rNull, rX, rY );
    rTexture.GetWholeCoord( aTexCoord );
    mpProgram->SetTexture( "sampler", rTexture );
    rTexture.SetFilter( GL_LINEAR );
    mpProgram->SetTextureCoord( aTexCoord );
    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawAlphaTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted, bool bPremultiplied )
{
    if( !UseProgram( "textureVertexShader", "textureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    mpProgram->SetBlendMode( bPremultiplied ? GL_ONE : GL_SRC_ALPHA,
                             GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, rPosAry, bInverted );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawTextureDiff( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry, bool bInverted )
{
    if( !UseProgram( "textureVertexShader", "diffTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "texture", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, rPosAry, bInverted );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& pPosAry )
{
    if( !UseProgram( "textureVertexShader", "maskedTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, pPosAry );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawBlendedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, OpenGLTexture& rAlpha, const SalTwoRect& rPosAry )
{
    GLfloat aTexCoord[8];
    if( !UseProgram( "blendedTextureVertexShader", "blendedTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetTexture( "alpha", rAlpha );
    rAlpha.GetCoord( aTexCoord, rPosAry );
    mpProgram->SetAlphaCoord( aTexCoord );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, rPosAry );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawMask( OpenGLTexture& rMask, SalColor nMaskColor, const SalTwoRect& pPosAry )
{
    if( !UseProgram( "textureVertexShader", "maskFragmentShader" ) )
        return;
    mpProgram->SetColor( "color", nMaskColor, 0 );
    mpProgram->SetTexture( "sampler", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rMask, pPosAry );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    if( !UseProgram( "textureVertexShader", "linearGradientFragmentShader" ) )
        return;
    Color aStartCol = rGradient.GetStartColor();
    Color aEndCol = rGradient.GetEndColor();
    long nFactor = rGradient.GetStartIntensity();
    mpProgram->SetColorWithIntensity( "start_color", aStartCol, nFactor );
    nFactor = rGradient.GetEndIntensity();
    mpProgram->SetColorWithIntensity( "end_color", aEndCol, nFactor );

    Rectangle aBoundRect;
    Point aCenter;
    rGradient.GetBoundRect( rRect, aBoundRect, aCenter );
    Polygon aPoly( aBoundRect );
    aPoly.Rotate( aCenter, rGradient.GetAngle() % 3600 );

    GLfloat aTexCoord[8] = { 0, 1, 1, 1, 1, 0, 0, 0 };
    GLfloat fMin = 1.0 - 100.0 / (100.0 - rGradient.GetBorder());
    aTexCoord[5] = aTexCoord[7] = fMin;
    mpProgram->SetTextureCoord( aTexCoord );
    DrawConvexPolygon( aPoly );
}

void OpenGLSalGraphicsImpl::DrawAxialGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    if( !UseProgram( "textureVertexShader", "linearGradientFragmentShader" ) )
        return;
    Color aStartCol = rGradient.GetStartColor();
    Color aEndCol = rGradient.GetEndColor();
    long nFactor = rGradient.GetStartIntensity();
    mpProgram->SetColorWithIntensity( "start_color", aStartCol, nFactor );
    nFactor = rGradient.GetEndIntensity();
    mpProgram->SetColorWithIntensity( "end_color", aEndCol, nFactor );

    /**
     * Draw two rectangles with linear gradient.
     *
     *  1 *---* 2
     *    |  /|
     *    | / |     Points 0 and 3 have start color
     *  0 |/__| 3   Points 1, 2, 4 and 5 have end color
     *    |\  |
     *    | \ |
     *    |  \|
     *  5 *---* 4
     *
     */

    Rectangle aRect;
    Point aCenter;
    rGradient.GetBoundRect( rRect, aRect, aCenter );

    // determine points 0 and 3
    Point aPt0( aRect.Left(), (aRect.Top() + aRect.Bottom() + 1) / 2 );
    Point aPt3( aRect.Right(), (aRect.Top() + aRect.Bottom() + 1) / 2 );

    Polygon aPoly( 7 );
    aPoly.SetPoint( aPt0,                0 );
    aPoly.SetPoint( aRect.TopLeft(),     1 );
    aPoly.SetPoint( aRect.TopRight(),    2 );
    aPoly.SetPoint( aPt3,                3 );
    aPoly.SetPoint( aRect.BottomRight(), 4 );
    aPoly.SetPoint( aRect.BottomLeft(),  5 );
    aPoly.SetPoint( aPt0,                6 );
    aPoly.Rotate( aCenter, rGradient.GetAngle() % 3600 );

    GLfloat aTexCoord[12] = { 0, 1, 1, 0, 2, 0, 3, 1, 4, 0, 5, 0 };
    GLfloat fMin = 1.0 - 100.0 / (100.0 - rGradient.GetBorder());
    aTexCoord[3] = aTexCoord[5] = aTexCoord[9] = aTexCoord[11] = fMin;
    mpProgram->SetTextureCoord( aTexCoord );
    DrawConvexPolygon( aPoly );
}

void OpenGLSalGraphicsImpl::DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    if( !UseProgram( "textureVertexShader", "radialGradientFragmentShader" ) )
        return;
    Color aStartCol = rGradient.GetStartColor();
    Color aEndCol = rGradient.GetEndColor();
    long nFactor = rGradient.GetStartIntensity();
    mpProgram->SetColorWithIntensity( "start_color", aStartCol, nFactor );
    nFactor = rGradient.GetEndIntensity();
    mpProgram->SetColorWithIntensity( "end_color", aEndCol, nFactor );

    Rectangle aRect;
    Point aCenter;
    rGradient.GetBoundRect( rRect, aRect, aCenter );

    // adjust coordinates so that radius has distance equals to 1.0
    double fRadius = aRect.GetWidth() / 2.0f;
    GLfloat fWidth = rRect.GetWidth() / fRadius;
    GLfloat fHeight = rRect.GetHeight() / fRadius;
    GLfloat aTexCoord[8] = { 0, 0, 0, fHeight, fWidth, fHeight, fWidth, 0 };
    mpProgram->SetTextureCoord( aTexCoord );
    mpProgram->SetUniform2f( "center", (aCenter.X() - rRect.Left()) / fRadius,
                                       (aCenter.Y() - rRect.Top())  / fRadius );
    DrawRect( rRect );
}


// draw --> LineColor and FillColor and RasterOp and ClipRegion
void OpenGLSalGraphicsImpl::drawPixel( long nX, long nY )
{
    SAL_INFO( "vcl.opengl", "::drawPixel" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw();
        if( UseSolid( mnLineColor ) )
            DrawPoint( nX, nY );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawPixel( long nX, long nY, SalColor nSalColor )
{
    SAL_INFO( "vcl.opengl", "::drawPixel" );
    if( nSalColor != SALCOLOR_NONE )
    {
        PreDraw();
        if( UseSolid( nSalColor ) )
            DrawPoint( nX, nY );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    SAL_INFO( "vcl.opengl", "::drawLine" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw();
        if( UseSolid( mnLineColor ) )
            DrawLine( nX1, nY1, nX2, nY2 );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    SAL_INFO( "vcl.opengl", "::drawRect" );
    PreDraw();

    if( UseSolid( mnFillColor ) )
        DrawRect( nX, nY, nWidth, nHeight );

    if( UseSolid( mnLineColor ) )
    {
        const long nX1( nX );
        const long nY1( nY );
        const long nX2( nX + nWidth );
        const long nY2( nY + nHeight );
        const SalPoint aPoints[] = { { nX1, nY1 }, { nX2, nY1 },
                                     { nX2, nY2 }, { nX1, nY2 } };
        DrawLines( 4, aPoints, true );
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    SAL_INFO( "vcl.opengl", "::drawPolyLine" );

    if( mnLineColor != SALCOLOR_NONE && nPoints > 1 )
    {
        PreDraw();
        if( UseSolid( mnLineColor ) )
            DrawLines( nPoints, pPtAry, false );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    SAL_INFO( "vcl.opengl", "::drawPolygon" );
    if( nPoints == 0 )
        return;
    if( nPoints == 1 )
    {
        drawPixel( pPtAry[0].mnX, pPtAry[0].mnY );
        return;
    }
    if( nPoints == 2 )
    {
        drawLine( pPtAry[0].mnX, pPtAry[0].mnY,
                  pPtAry[1].mnX, pPtAry[1].mnY );
        return;
    }

    PreDraw();

    if( UseSolid( mnFillColor ) )
        DrawPolygon( nPoints, pPtAry );

    if( UseSolid( mnLineColor ) )
        DrawLines( nPoints, pPtAry, true );

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
    SAL_INFO( "vcl.opengl", "::drawPolyPolygon" );
    if( nPoly <= 0 )
        return;

    PreDraw();

    if( UseSolid( mnFillColor ) )
    {
        for( sal_uInt32 i = 0; i < nPoly; i++ )
            DrawPolygon( pPoints[i], pPtAry[i] );
    }

    if( UseSolid( mnLineColor ) )
    {
        // TODO Use glMultiDrawElements or primitive restart
        for( sal_uInt32 i = 0; i < nPoly; i++ )
            DrawLines( pPoints[i], pPtAry[i], true );
    }

    PostDraw();
}

bool OpenGLSalGraphicsImpl::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency )
{
    SAL_INFO( "vcl.opengl", "::drawPolyPolygon trans " << fTransparency );
    if( rPolyPolygon.count() <= 0 )
        return true;

    PreDraw();

    if( UseSolid( mnFillColor, fTransparency ) )
    {
        for( sal_uInt32 i = 0; i < rPolyPolygon.count(); i++ )
        {
            const ::basegfx::B2DPolyPolygon aOnePoly( rPolyPolygon.getB2DPolygon( i ) );
            DrawPolyPolygon( aOnePoly );
        }
    }

    PostDraw();

    return true;
}

bool OpenGLSalGraphicsImpl::drawPolyLine(
            const ::basegfx::B2DPolygon& rPolygon,
            double fTransparency,
            const ::basegfx::B2DVector& rLineWidth,
            basegfx::B2DLineJoin eLineJoin,
            com::sun::star::drawing::LineCap eLineCap)
{
    SAL_INFO( "vcl.opengl", "::drawPolyLine trans " << fTransparency );
    if( mnLineColor == SALCOLOR_NONE )
        return true;

    const bool bIsHairline = (rLineWidth.getX() == rLineWidth.getY()) && (rLineWidth.getX() <= 1.2);

    // #i101491#
    if( !bIsHairline && (rPolygon.count() > 1000) )
    {
        // the used basegfx::tools::createAreaGeometry is simply too
        // expensive with very big polygons; fallback to caller (who
        // should use ImplLineConverter normally)
        // AW: ImplLineConverter had to be removed since it does not even
        // know LineJoins, so the fallback will now prepare the line geometry
        // the same way.
        return false;
    }

    // #i11575#desc5#b adjust B2D tesselation result to raster positions
    basegfx::B2DPolygon aPolygon = rPolygon;
    const double fHalfWidth = 0.5 * rLineWidth.getX();

    // #i122456# This is probably thought to happen to align hairlines to pixel positions, so
    // it should be a 0.5 translation, not more. It will definitely go wrong with fat lines
    aPolygon.transform( basegfx::tools::createTranslateB2DHomMatrix(0.5, 0.5) );

    // shortcut for hairline drawing to improve performance
    //bool bDrawnOk = true;
    if( bIsHairline )
    {
        // hairlines can benefit from a simplified tesselation
        // e.g. for hairlines the linejoin style can be ignored
        /*basegfx::B2DTrapezoidVector aB2DTrapVector;
        basegfx::tools::createLineTrapezoidFromB2DPolygon( aB2DTrapVector, aPolygon, rLineWidth.getX() );

        // draw tesselation result
        const int nTrapCount = aB2DTrapVector.size();
        if( nTrapCount > 0 )
            bDrawnOk = drawFilledTrapezoids( &aB2DTrapVector[0], nTrapCount, fTransparency );

        return bDrawnOk;*/
    }

    // get the area polygon for the line polygon
    if( (rLineWidth.getX() != rLineWidth.getY())
    && !basegfx::fTools::equalZero( rLineWidth.getY() ) )
    {
        // prepare for createAreaGeometry() with anisotropic linewidth
        aPolygon.transform( basegfx::tools::createScaleB2DHomMatrix(1.0, rLineWidth.getX() / rLineWidth.getY()));
    }

    // create the area-polygon for the line
    const basegfx::B2DPolyPolygon aAreaPolyPoly( basegfx::tools::createAreaGeometry(aPolygon, fHalfWidth, eLineJoin, eLineCap) );

    if( (rLineWidth.getX() != rLineWidth.getY())
    && !basegfx::fTools::equalZero( rLineWidth.getX() ) )
    {
        // postprocess createAreaGeometry() for anisotropic linewidth
        aPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(1.0, rLineWidth.getY() / rLineWidth.getX()));
    }

    PreDraw();
    if( UseSolid( mnLineColor, fTransparency ) )
    {
        for( sal_uInt32 i = 0; i < aAreaPolyPoly.count(); i++ )
        {
            const ::basegfx::B2DPolyPolygon aOnePoly( aAreaPolyPoly.getB2DPolygon( i ) );
            DrawPolyPolygon( aOnePoly );
        }
    }
    PostDraw();

    return true;
}

bool OpenGLSalGraphicsImpl::drawPolyLineBezier(
            sal_uInt32 /*nPoints*/,
            const SalPoint* /*pPtAry*/,
            const sal_uInt8* /*pFlgAry*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolygonBezier(
            sal_uInt32 /*nPoints*/,
            const SalPoint* /*pPtAry*/,
            const sal_uInt8* /*pFlgAry*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolyPolygonBezier(
            sal_uInt32 /*nPoly*/,
            const sal_uInt32* /*pPoints*/,
            const SalPoint* const* /*pPtAry*/,
            const sal_uInt8* const* /*pFlgAry*/ )
{
    return false;
}

// CopyArea --> No RasterOp, but ClipRegion
void OpenGLSalGraphicsImpl::copyArea(
            long nDestX, long nDestY,
            long nSrcX, long nSrcY,
            long nSrcWidth, long nSrcHeight,
            sal_uInt16 /*nFlags*/ )
{
    SAL_INFO( "vcl.opengl", "::copyArea " << nSrcX << "," << nSrcY << " >> " << nDestX << "," << nDestY << " (" << nSrcWidth << "," << nSrcHeight << ")" );
    OpenGLTexture aTexture;
    SalTwoRect aPosAry(0, 0, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);

    PreDraw();
    // TODO offscreen case
    aTexture = OpenGLTexture( nSrcX, GetHeight() - nSrcY - nSrcHeight,
                              nSrcWidth, nSrcHeight );
    DrawTexture( aTexture, aPosAry );
    PostDraw();
}

// CopyBits and DrawBitmap --> RasterOp and ClipRegion
// CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
void OpenGLSalGraphicsImpl::DoCopyBits( const SalTwoRect& rPosAry, OpenGLSalGraphicsImpl& rImpl )
{
    SAL_INFO( "vcl.opengl", "::copyBits" );

    if( &rImpl == this &&
        (rPosAry.mnSrcWidth == rPosAry.mnDestWidth) &&
        (rPosAry.mnSrcHeight == rPosAry.mnDestHeight))
    {
        // short circuit if there is nothing to do
        if( (rPosAry.mnSrcX == rPosAry.mnDestX) &&
            (rPosAry.mnSrcY == rPosAry.mnDestY))
            return;
        // use copyArea() if source and destination context are identical
        copyArea( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
            rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, 0 );
        return;
    }

    if( rImpl.mbOffscreen )
    {
        PreDraw();
        DrawTexture( rImpl.maOffscreenTex, rPosAry );
        PostDraw();
        return;
    }

    SAL_WARN( "vcl.opengl", "*** NOT IMPLEMENTED *** copyBits" );
    // TODO: Copy from one FBO to the other (glBlitFramebuffer)
    //       ie. copying from one visible window to another visible window
}

void OpenGLSalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    // check that carefully only in the debug mode
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture = rBitmap.GetTexture();

    SAL_INFO( "vcl.opengl", "::drawBitmap" );
    PreDraw();
    DrawTexture( rTexture, rPosAry );
    PostDraw();
}

void OpenGLSalGraphicsImpl::drawBitmap(
            const SalTwoRect& /*rPosAry*/,
            const SalBitmap& /*rSalBitmap*/,
            SalColor /*nTransparentColor*/ )
{
    OSL_FAIL( "::DrawBitmap with transparent color not supported" );
}

void OpenGLSalGraphicsImpl::drawBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap,
            const SalBitmap& rMaskBitmap )
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    const OpenGLSalBitmap& rMask = static_cast<const OpenGLSalBitmap&>(rMaskBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture& rMaskTex( rMask.GetTexture() );

    SAL_INFO( "vcl.opengl", "::drawBitmap with MASK" );
    PreDraw();
    DrawTextureWithMask( rTexture, rMaskTex, rPosAry );
    PostDraw();
}

void OpenGLSalGraphicsImpl::drawMask(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap,
            SalColor nMaskColor )
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );

    SAL_INFO( "vcl.opengl", "::drawMask" );
    PreDraw();
    DrawMask( rTexture, nMaskColor, rPosAry );
    PostDraw();
}

SalBitmap* OpenGLSalGraphicsImpl::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    OpenGLSalBitmap* pBitmap = new OpenGLSalBitmap;
    SAL_INFO( "vcl.opengl", "::getBitmap " << nX << "," << nY <<
              " " << nWidth << "x" << nHeight );
    //TODO really needed?
    PreDraw();
    if( !pBitmap->Create( maOffscreenTex, nX, nY, nWidth, nHeight ) )
    {
        delete pBitmap;
        pBitmap = NULL;
    }
    PostDraw();
    return pBitmap;
}

SalColor OpenGLSalGraphicsImpl::getPixel( long nX, long nY )
{
    char pixel[3] = { 0, 0, 0 };

    PreDraw();
    nY = GetHeight() - nY;
    glReadPixels( nX, nY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    PostDraw();

    CHECK_GL_ERROR();
    return MAKE_SALCOLOR( pixel[0], pixel[1], pixel[2] );
}

// invert --> ClipRegion (only Windows or VirDevs)
void OpenGLSalGraphicsImpl::invert(
            long nX, long nY,
            long nWidth, long nHeight,
            SalInvert nFlags)
{
    // TODO Figure out what are those:
    //   * SAL_INVERT_50 (50/50 pattern?)
    //   * SAL_INVERT_TRACKFRAME (dash-line rectangle?)

    PreDraw();

    if( nFlags & SAL_INVERT_TRACKFRAME )
    {

    }
    else if( nFlags & SAL_INVERT_50 )
    {

    }
    else // just invert
    {
        if( UseInvert() )
            DrawRect( nX, nY, nWidth, nHeight );
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
    PreDraw();

    if( nFlags & SAL_INVERT_TRACKFRAME )
    {

    }
    else if( nFlags & SAL_INVERT_50 )
    {

    }
    else // just invert
    {
        if( UseInvert() )
            DrawPolygon( nPoints, pPtAry );
    }

    PostDraw();
}

bool OpenGLSalGraphicsImpl::drawEPS(
            long /*nX*/, long /*nY*/,
            long /*nWidth*/, long /*nHeight*/,
            void* /*pPtr*/,
            sal_uLong /*nSize*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::blendBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap )
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );

    SAL_INFO( "vcl.opengl", "::blendBitmap" );
    PreDraw();
    glEnable( GL_BLEND );
    glBlendFunc( GL_ZERO, GL_SRC_COLOR );
    DrawTexture( rTexture, rPosAry );
    glDisable( GL_BLEND );
    PostDraw();
    return true;
}

bool OpenGLSalGraphicsImpl::blendAlphaBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalSrcBitmap,
            const SalBitmap& rSalMaskBitmap,
            const SalBitmap& rSalAlphaBitmap )
{
    const OpenGLSalBitmap& rSrcBitmap = static_cast<const OpenGLSalBitmap&>(rSalSrcBitmap);
    const OpenGLSalBitmap& rMaskBitmap = static_cast<const OpenGLSalBitmap&>(rSalMaskBitmap);
    const OpenGLSalBitmap& rAlphaBitmap = static_cast<const OpenGLSalBitmap&>(rSalAlphaBitmap);
    OpenGLTexture& rTexture( rSrcBitmap.GetTexture() );
    OpenGLTexture& rMask( rMaskBitmap.GetTexture() );
    OpenGLTexture& rAlpha( rAlphaBitmap.GetTexture() );

    SAL_INFO( "vcl.opengl", "::blendAlphaBitmap" );
    PreDraw();
    DrawBlendedTexture( rTexture, rMask, rAlpha, rPosAry );
    PostDraw();
    return true;
}

/** Render bitmap with alpha channel

    @param rSourceBitmap
    Source bitmap to blit

    @param rAlphaBitmap
    Alpha channel to use for blitting

    @return true, if the operation succeeded, and false
    otherwise. In this case, clients should try to emulate alpha
    compositing themselves
 */
bool OpenGLSalGraphicsImpl::drawAlphaBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap,
            const SalBitmap& rAlphaBitmap )
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    const OpenGLSalBitmap& rAlpha = static_cast<const OpenGLSalBitmap&>(rAlphaBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture& rAlphaTex( rAlpha.GetTexture() );

    SAL_INFO( "vcl.opengl", "::drawAlphaBitmap" );
    PreDraw();
    DrawTextureWithMask( rTexture, rAlphaTex, rPosAry );
    PostDraw();
    return true;
}

bool OpenGLSalGraphicsImpl::drawAlphaBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap )
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );

    SAL_INFO( "vcl.opengl", "::drawAlphaBitmap" );
    PreDraw();
    DrawAlphaTexture( rTexture, rPosAry );
    PostDraw();

    CHECK_GL_ERROR();
    return true;
}

/** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
bool OpenGLSalGraphicsImpl::drawTransformedBitmap(
            const basegfx::B2DPoint& rNull,
            const basegfx::B2DPoint& rX,
            const basegfx::B2DPoint& rY,
            const SalBitmap& rSrcBitmap,
            const SalBitmap* pAlphaBitmap)
{
    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSrcBitmap);
    const OpenGLSalBitmap* pMaskBitmap = static_cast<const OpenGLSalBitmap*>(pAlphaBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture aMask; // no texture

    if( pMaskBitmap != NULL )
        aMask = pMaskBitmap->GetTexture();

    SAL_INFO( "vcl.opengl", "::drawTransformedBitmap" );
    PreDraw();
    DrawTransformedTexture( rTexture, aMask, rNull, rX, rY );
    PostDraw();

    return true;
}

/** Render solid rectangle with given transparency

    @param nTransparency
    Transparency value (0-255) to use. 0 blits and opaque, 255 a
    fully transparent rectangle
 */
bool OpenGLSalGraphicsImpl::drawAlphaRect(
                long nX, long nY,
                long nWidth, long nHeight,
                sal_uInt8 nTransparency )
{
    SAL_INFO( "vcl.opengl", "::drawAlphaRect" );
    if( mnFillColor != SALCOLOR_NONE && nTransparency < 100 )
    {
        PreDraw();
        UseSolid( mnFillColor, nTransparency );
        DrawRect( nX, nY, nWidth, nHeight );
        PostDraw();
    }

    return true;
}

bool OpenGLSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPoly,
        const Gradient& rGradient)
{
    Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

    SAL_INFO( "vcl.opengl", "::drawGradient" );

    if( aBoundRect.IsEmpty() )
        return true;

    if( rGradient.GetStyle() != GradientStyle_LINEAR &&
        rGradient.GetStyle() != GradientStyle_AXIAL &&
        rGradient.GetStyle() != GradientStyle_RADIAL )
        return false;

    aBoundRect.Left()--;
    aBoundRect.Top()--;
    aBoundRect.Right()++;
    aBoundRect.Bottom()++;

    PreDraw();

    ImplSetClipBit( vcl::Region( rPolyPoly ), 0x02 );
    if( mbUseStencil )
    {
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_EQUAL, 3, 0xFF );
    }
    else
    {
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_EQUAL, 2, 0xFF );
    }

    // if border >= 100%, draw solid rectangle with start color
    if( rGradient.GetBorder() >= 100.0 )
    {
        Color aCol = rGradient.GetStartColor();
        long nF = rGradient.GetStartIntensity();
        if( UseSolid( MAKE_SALCOLOR( aCol.GetRed() * nF / 100,
                                     aCol.GetGreen() * nF / 100,
                                     aCol.GetBlue() * nF / 100 ) ) )
            DrawRect( aBoundRect );
    }
    else if( rGradient.GetStyle() == GradientStyle_LINEAR )
    {
        DrawLinearGradient( rGradient, aBoundRect );
    }
    else if( rGradient.GetStyle() == GradientStyle_AXIAL )
    {
        DrawAxialGradient( rGradient, aBoundRect );
    }
    else if( rGradient.GetStyle() == GradientStyle_RADIAL )
    {
        DrawRadialGradient( rGradient, aBoundRect );
    }

    if( !mbUseStencil )
        glDisable( GL_STENCIL_TEST );
    PostDraw();

    CHECK_GL_ERROR();
    return true;
}

void OpenGLSalGraphicsImpl::beginPaint()
{
    if( !mpContext && !AcquireContext() )
        return;

    mpContext->mnPainting++;
}

void OpenGLSalGraphicsImpl::endPaint()
{
    if( !mpContext && !AcquireContext() )
        return;

    mpContext->mnPainting--;
    assert( mpContext->mnPainting >= 0 );
    if( mpContext->mnPainting == 0 && !mbOffscreen )
    {
        mpContext->makeCurrent();
        glFlush();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
