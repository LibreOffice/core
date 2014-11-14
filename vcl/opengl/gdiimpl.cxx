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
#include "opengl/salbmp.hxx"

#include <vector>

#define GL_ATTRIB_POS 0
#define GL_ATTRIB_TEX 1

#define glUniformColor(nUniform, nColor, nTransparency)    \
    glUniform4f( nUniform,                                 \
                 ((float) SALCOLOR_RED( nColor )) / 255,   \
                 ((float) SALCOLOR_GREEN( nColor )) / 255, \
                 ((float) SALCOLOR_BLUE( nColor )) / 255,  \
                 (100 - nTransparency) * (1.0 / 100) )

#define glUniformColorf(nUniform, nColor, fTransparency)   \
    glUniform4f( nUniform,                                 \
                 ((float) SALCOLOR_RED( nColor )) / 255,   \
                 ((float) SALCOLOR_GREEN( nColor )) / 255, \
                 ((float) SALCOLOR_BLUE( nColor )) / 255,  \
                 (1.0f - fTransparency) )

#define glUniformColorIntensity(nUniform, aColor, nFactor)      \
    glUniform4f( nUniform,                                      \
                 ((float) aColor.GetRed()) * nFactor / 25500.0,   \
                 ((float) aColor.GetGreen()) * nFactor / 25500.0, \
                 ((float) aColor.GetBlue()) * nFactor / 25500.0,  \
                 1.0f )

OpenGLSalGraphicsImpl::OpenGLSalGraphicsImpl()
    : mpFrame(NULL)
    , mnPainting(0)
    , mbUseScissor(false)
    , mbUseStencil(false)
    , mbOffscreen(false)
    , mnFramebufferId(0)
    , mnLineColor(SALCOLOR_NONE)
    , mnFillColor(SALCOLOR_NONE)
    , mnSolidProgram(0)
    , mnColorUniform(0)
    , mnTextureProgram(0)
    , mnSamplerUniform(0)
    , mnMaskedTextureProgram(0)
    , mnMaskedSamplerUniform(0)
    , mnMaskSamplerUniform(0)
    , mnMaskProgram(0)
    , mnMaskUniform(0)
    , mnMaskColorUniform(0)
    , mnLinearGradientProgram(0)
    , mnLinearGradientStartColorUniform(0)
    , mnLinearGradientEndColorUniform(0)
    , mnRadialGradientProgram(0)
    , mnRadialGradientStartColorUniform(0)
    , mnRadialGradientEndColorUniform(0)
    , mnRadialGradientCenterUniform(0)
{
}

OpenGLSalGraphicsImpl::~OpenGLSalGraphicsImpl()
{
}

void OpenGLSalGraphicsImpl::PreDraw()
{
    maContext.makeCurrent();
    // TODO: lfrb: make sure the render target has the right size
    if( mbOffscreen )
        glBindFramebuffer( GL_FRAMEBUFFER, mnFramebufferId );
    glViewport( 0, 0, GetWidth(), GetHeight() );
    if( mbUseScissor )
        glEnable( GL_SCISSOR_TEST );
    if( mbUseStencil )
    {
        glStencilFunc( GL_EQUAL, 1, 0x1 );
        glEnable( GL_STENCIL_TEST );
    }

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::PostDraw()
{
    if( mbOffscreen )
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    else if( mnPainting == 0 )
        glFlush();
    if( mbUseScissor )
        glDisable( GL_SCISSOR_TEST );
    if( mbUseStencil )
        glDisable( GL_STENCIL_TEST );
    CHECK_GL_ERROR();
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
    BeginSolid( MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ) );
    DrawPolyPolygon( rClip.GetAsB2DPolyPolygon() );
    EndSolid();

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilMask( 0x00 );
    glDisable( GL_STENCIL_TEST );
}

bool OpenGLSalGraphicsImpl::setClipRegion( const vcl::Region& rClip )
{
    SAL_INFO( "vcl.opengl", "::setClipRegion " << rClip );

    if( rClip.IsEmpty() )
    {
        ResetClipRegion();
        return true;
    }

    if( rClip.IsRectangle() )
    {
        Rectangle aRect( rClip.GetBoundRect() );

        mbUseStencil = false;
        mbUseScissor = true;
        maContext.makeCurrent();
        glScissor( aRect.Left(), GetHeight() - aRect.Top(), aRect.GetWidth(), aRect.GetHeight() );
    }
    else
    {
        mbUseStencil = true;
        mbUseScissor = false;
        maContext.makeCurrent();
        glViewport( 0, 0, GetWidth(), GetHeight() );
        ImplSetClipBit( rClip, 0x01 );
    }

    return true;
}

// set the clip region to empty
void OpenGLSalGraphicsImpl::ResetClipRegion()
{
    SAL_INFO( "vcl.opengl", "::ResetClipRegion" );
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

// enable/disbale offscreen rendering
void OpenGLSalGraphicsImpl::SetOffscreen( bool bOffscreen )
{
    if( bOffscreen == mbOffscreen )
    {
        // Already disabled
        if( !mbOffscreen )
            return;

        // Already enabled and same size
        if( maOffscreenTex.GetWidth()  == GetWidth() &&
            maOffscreenTex.GetHeight() == GetHeight() )
            return;
    }
    else
    {
        mbOffscreen = bOffscreen;
        if( bOffscreen )
            glGenFramebuffers( 1, &mnFramebufferId );
        else
            glDeleteFramebuffers( 1, &mnFramebufferId );
    }

    if( mbOffscreen )
    {
        glBindFramebuffer( GL_FRAMEBUFFER, mnFramebufferId );
        maOffscreenTex = OpenGLTexture( GetWidth(), GetHeight() );
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, maOffscreenTex.Id(), 0 );
    }

    CHECK_GL_ERROR();
}

bool OpenGLSalGraphicsImpl::CreateSolidProgram( void )
{
    SAL_INFO( "vcl.opengl", "::CreateSolidProgram" );
    mnSolidProgram = OpenGLHelper::LoadShaders( "solidVertexShader", "solidFragmentShader" );
    if( mnSolidProgram == 0 )
        return false;

    SAL_INFO( "vcl.opengl", "Solid Program Created" );
    glBindAttribLocation( mnSolidProgram, GL_ATTRIB_POS, "position" );
    mnColorUniform = glGetUniformLocation( mnSolidProgram, "color" );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::CreateTextureProgram( void )
{
    mnTextureProgram = OpenGLHelper::LoadShaders( "textureVertexShader", "textureFragmentShader" );
    if( mnTextureProgram == 0 )
        return false;

    glBindAttribLocation( mnTextureProgram, GL_ATTRIB_POS, "position" );
    glBindAttribLocation( mnTextureProgram, GL_ATTRIB_TEX, "tex_coord_in" );
    mnSamplerUniform = glGetUniformLocation( mnTextureProgram, "sampler" );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::CreateMaskedTextureProgram( void )
{
    mnMaskedTextureProgram = OpenGLHelper::LoadShaders( "maskedTextureVertexShader", "maskedTextureFragmentShader" );
    if( mnMaskedTextureProgram == 0 )
        return false;

    glBindAttribLocation( mnMaskedTextureProgram, GL_ATTRIB_POS, "position" );
    glBindAttribLocation( mnMaskedTextureProgram, GL_ATTRIB_TEX, "tex_coord_in" );
    mnMaskedSamplerUniform = glGetUniformLocation( mnMaskedTextureProgram, "sampler" );
    mnMaskSamplerUniform = glGetUniformLocation( mnMaskedTextureProgram, "mask" );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::CreateMaskProgram( void )
{
    mnMaskedTextureProgram = OpenGLHelper::LoadShaders( "maskVertexShader", "maskFragmentShader" );
    if( mnMaskedTextureProgram == 0 )
        return false;

    glBindAttribLocation( mnTextureProgram, GL_ATTRIB_POS, "position" );
    glBindAttribLocation( mnTextureProgram, GL_ATTRIB_TEX, "tex_coord_in" );
    mnMaskUniform = glGetUniformLocation( mnMaskProgram, "sampler" );
    mnMaskColorUniform = glGetUniformLocation( mnMaskProgram, "mask" );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::CreateLinearGradientProgram( void )
{
    mnLinearGradientProgram = OpenGLHelper::LoadShaders( "textureVertexShader", "linearGradientFragmentShader" );
    if( mnLinearGradientProgram == 0 )
        return false;

    glBindAttribLocation( mnLinearGradientProgram, GL_ATTRIB_POS, "position" );
    glBindAttribLocation( mnLinearGradientProgram, GL_ATTRIB_TEX, "tex_coord_in" );
    mnLinearGradientStartColorUniform = glGetUniformLocation( mnLinearGradientProgram, "start_color" );
    mnLinearGradientEndColorUniform = glGetUniformLocation( mnLinearGradientProgram, "end_color" );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::CreateRadialGradientProgram( void )
{
    mnRadialGradientProgram = OpenGLHelper::LoadShaders( "textureVertexShader", "radialGradientFragmentShader" );
    if( mnRadialGradientProgram == 0 )
        return false;

    glBindAttribLocation( mnRadialGradientProgram, GL_ATTRIB_POS, "position" );
    glBindAttribLocation( mnRadialGradientProgram, GL_ATTRIB_TEX, "tex_coord_in" );
    mnRadialGradientStartColorUniform = glGetUniformLocation( mnRadialGradientProgram, "start_color" );
    mnRadialGradientEndColorUniform = glGetUniformLocation( mnRadialGradientProgram, "end_color" );
    mnRadialGradientCenterUniform = glGetUniformLocation( mnRadialGradientProgram, "center" );

    CHECK_GL_ERROR();
    return true;
}

void OpenGLSalGraphicsImpl::BeginSolid( SalColor nColor, sal_uInt8 nTransparency )
{
    if( mnSolidProgram == 0 )
    {
        glClearColor( 1, 1, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT );
        if( !CreateSolidProgram() )
            return;
    }

    if( nTransparency > 0 )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    glUseProgram( mnSolidProgram );
    glUniformColor( mnColorUniform, nColor, nTransparency );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::BeginSolid( SalColor nColor, double fTransparency )
{
    if( mnSolidProgram == 0 )
    {
        if( !CreateSolidProgram() )
            return;
    }

    if( fTransparency > 0.0f )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    glUseProgram( mnSolidProgram );
    glUniformColorf( mnColorUniform, nColor, fTransparency );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::BeginSolid( SalColor nColor )
{
    BeginSolid( nColor, 0.0f );
}

void OpenGLSalGraphicsImpl::EndSolid( void )
{
    glUseProgram( 0 );
    glDisable( GL_BLEND );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::BeginInvert( void )
{
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
    BeginSolid( MAKE_SALCOLOR( 255, 255, 255 ) );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::EndInvert( void )
{
    EndSolid();
    glDisable( GL_BLEND );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawPoint( long nX, long nY )
{
    GLfloat pPoint[2];

    pPoint[0] = 2 * nX / GetWidth() - 1.0f;
    pPoint[1] = 1.0f - 2 * nY / GetHeight();

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, pPoint );
    glDrawArrays( GL_POINTS, 0, 1 );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    GLfloat pPoints[4];

    pPoints[0] = (2 * nX1) / GetWidth() - 1.0;
    pPoints[1] = 1.0f - 2 * nY1 / GetHeight();
    pPoints[2] = (2 * nX2) / GetWidth() - 1.0;;
    pPoints[3] = 1.0f - 2 * nY2 / GetHeight();

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, pPoints );
    glDrawArrays( GL_LINES, 0, 2 );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
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

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, &aPoints[0] );
    if( bClose )
        glDrawArrays( GL_LINE_LOOP, 0, nPoints );
    else
        glDrawArrays( GL_LINE_STRIP, 0, nPoints );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
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

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
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

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
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
    sal_uInt32 i, j;
    ::std::vector< GLfloat > pVertices;
    GLfloat nWidth = GetWidth();
    GLfloat nHeight = GetHeight();
    const ::basegfx::B2DPolyPolygon& aSimplePolyPolygon = ::basegfx::tools::solveCrossovers( rPolyPolygon );

    for( i = 0; i < aSimplePolyPolygon.count(); i++ )
    {
        const basegfx::B2DPolygon& rPolygon( aSimplePolyPolygon.getB2DPolygon( i ) );
        const ::basegfx::B2DPolygon& aResult(
            ::basegfx::triangulator::triangulate( rPolygon ) );

        for( j = 0; j < aResult.count(); j++ )
        {
            const ::basegfx::B2DPoint& rPt( aResult.getB2DPoint( j ) );
            pVertices.push_back( 2 * rPt.getX() / nWidth - 1.0f );
            pVertices.push_back( 1.0f - 2 * rPt.getY() / nHeight );
        }
    }

    glEnableVertexAttribArray( GL_ATTRIB_POS );
    glVertexAttribPointer( GL_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, pVertices.data() );
    glDrawArrays( GL_TRIANGLES, 0, pVertices.size() / 2 );
    glDisableVertexAttribArray( GL_ATTRIB_POS );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted )
{
    GLfloat aTexCoord[8];

    rTexture.GetCoord( aTexCoord, rPosAry, bInverted );
    glEnableVertexAttribArray( GL_ATTRIB_TEX );
    glVertexAttribPointer( GL_ATTRIB_TEX, 2, GL_FLOAT, GL_FALSE, 0, aTexCoord );

    DrawRect( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight );

    glDisableVertexAttribArray( GL_ATTRIB_TEX );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& pPosAry, bool bInverted )
{
    if( mnTextureProgram == 0 )
    {
        if( !CreateTextureProgram() )
            return;
    }

    glUseProgram( mnTextureProgram );
    glUniform1i( mnSamplerUniform, 0 );
    glActiveTexture( GL_TEXTURE0 );
    CHECK_GL_ERROR();

    rTexture.Bind();
    DrawTextureRect( rTexture, pPosAry, bInverted );
    rTexture.Unbind();
    CHECK_GL_ERROR();

    glUseProgram( 0 );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& pPosAry )
{
    if( mnMaskedTextureProgram == 0 )
    {
        if( !CreateMaskedTextureProgram() )
            return;
    }

    glUseProgram( mnMaskedTextureProgram );
    glUniform1i( mnMaskedSamplerUniform, 0 );
    glUniform1i( mnMaskSamplerUniform, 1 );
    glActiveTexture( GL_TEXTURE0 );
    rTexture.Bind();
    glActiveTexture( GL_TEXTURE1 );
    rMask.Bind();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, pPosAry );
    glDisable( GL_BLEND );

    glActiveTexture( GL_TEXTURE1 );
    rMask.Unbind();
    glActiveTexture( GL_TEXTURE0 );
    rTexture.Unbind();
    glUseProgram( 0 );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawMask( OpenGLTexture& rMask, SalColor nMaskColor, const SalTwoRect& /*pPosAry*/ )
{
    if( mnMaskProgram == 0 )
    {
        if( !CreateMaskProgram() )
            return;
    }

    glUseProgram( mnMaskProgram );
    glUniformColor( mnMaskColorUniform, nMaskColor, 0 );
    glUniform1i( mnMaskUniform, 0 );
    glActiveTexture( GL_TEXTURE0 );
    rMask.Bind();

    //DrawTextureRect( rMask, pPosAry );

    rMask.Unbind();
    glUseProgram( 0 );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    if( mnLinearGradientProgram == 0 )
    {
        if( !CreateLinearGradientProgram() )
            return;
    }

    glUseProgram( mnLinearGradientProgram );

    Color aStartCol = rGradient.GetStartColor();
    Color aEndCol = rGradient.GetEndColor();
    long nFactor = rGradient.GetStartIntensity();
    glUniformColorIntensity( mnLinearGradientStartColorUniform, aStartCol, nFactor );
    nFactor = rGradient.GetEndIntensity();
    glUniformColorIntensity( mnLinearGradientEndColorUniform, aEndCol, nFactor );

    Rectangle aBoundRect;
    Point aCenter;
    rGradient.GetBoundRect( rRect, aBoundRect, aCenter );
    Polygon aPoly( aBoundRect );
    aPoly.Rotate( aCenter, rGradient.GetAngle() % 3600 );

    GLfloat aTexCoord[8] = { 0, 1, 1, 1, 1, 0, 0, 0 };
    GLfloat fMin = 1.0 - 100.0 / (100.0 - rGradient.GetBorder());
    aTexCoord[5] = aTexCoord[7] = fMin;
    glEnableVertexAttribArray( GL_ATTRIB_TEX );
    glVertexAttribPointer( GL_ATTRIB_TEX, 2, GL_FLOAT, GL_FALSE, 0, aTexCoord );

    DrawConvexPolygon( aPoly );

    glDisableVertexAttribArray( GL_ATTRIB_TEX );
    CHECK_GL_ERROR();

    glUseProgram( 0 );

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    if( mnRadialGradientProgram == 0 )
    {
        if( !CreateRadialGradientProgram() )
            return;
    }

    glUseProgram( mnRadialGradientProgram );

    Color aStartCol = rGradient.GetStartColor();
    Color aEndCol = rGradient.GetEndColor();
    long nFactor = rGradient.GetStartIntensity();
    glUniformColorIntensity( mnRadialGradientStartColorUniform, aStartCol, nFactor );
    nFactor = rGradient.GetEndIntensity();
    glUniformColorIntensity( mnRadialGradientEndColorUniform, aEndCol, nFactor );

    Rectangle aRect;
    Point aCenter;
    rGradient.GetBoundRect( rRect, aRect, aCenter );

    // adjust coordinates so that radius has distance equals to 1.0
    double fRadius = aRect.GetWidth() / 2.0f;
    GLfloat fWidth = rRect.GetWidth() / fRadius;
    GLfloat fHeight = rRect.GetHeight() / fRadius;
    glUniform2f( mnRadialGradientCenterUniform, (aCenter.X() -rRect.Left()) / fRadius, (aCenter.Y() - rRect.Top()) / fRadius );

    GLfloat aTexCoord[8] = { 0, 0, 0, fHeight, fWidth, fHeight, fWidth, 0 };
    glEnableVertexAttribArray( GL_ATTRIB_TEX );
    glVertexAttribPointer( GL_ATTRIB_TEX, 2, GL_FLOAT, GL_FALSE, 0, aTexCoord );

    DrawRect( rRect );

    glDisableVertexAttribArray( GL_ATTRIB_TEX );
    glUseProgram( 0 );

    CHECK_GL_ERROR();
}


// draw --> LineColor and FillColor and RasterOp and ClipRegion
void OpenGLSalGraphicsImpl::drawPixel( long nX, long nY )
{
    SAL_INFO( "vcl.opengl", "::drawPixel" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw();
        BeginSolid( mnLineColor );
        DrawPoint( nX, nY );
        EndSolid();
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawPixel( long nX, long nY, SalColor nSalColor )
{
    SAL_INFO( "vcl.opengl", "::drawPixel" );
    if( nSalColor != SALCOLOR_NONE )
    {
        PreDraw();
        BeginSolid( nSalColor );
        DrawPoint( nX, nY );
        EndSolid();
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    SAL_INFO( "vcl.opengl", "::drawLine" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw();
        BeginSolid( mnLineColor );
        DrawLine( nX1, nY1, nX2, nY2 );
        EndSolid();
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    SAL_INFO( "vcl.opengl", "::drawRect" );
    PreDraw();

    if( mnFillColor != SALCOLOR_NONE )
    {
        BeginSolid( mnFillColor );
        DrawRect( nX, nY, nWidth, nHeight );
        EndSolid();
    }

    if( mnLineColor != SALCOLOR_NONE )
    {
        const long nX1( nX );
        const long nY1( nY );
        const long nX2( nX + nWidth );
        const long nY2( nY + nHeight );
        const SalPoint aPoints[] = { { nX1, nY1 }, { nX2, nY1 },
                                     { nX2, nY2 }, { nX1, nY2 } };

        BeginSolid( mnLineColor );
        DrawLines( 4, aPoints, true );
        EndSolid();
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    SAL_INFO( "vcl.opengl", "::drawPolyLine" );

    if( mnLineColor != SALCOLOR_NONE && nPoints > 1 )
    {
        PreDraw();
        BeginSolid( mnLineColor );
        DrawLines( nPoints, pPtAry, false );
        EndSolid();
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

    if( mnFillColor != SALCOLOR_NONE )
    {
        BeginSolid( mnFillColor );
        DrawPolygon( nPoints, pPtAry );
        EndSolid();
    }

    if( mnLineColor != SALCOLOR_NONE )
    {
        BeginSolid( mnLineColor );
        DrawLines( nPoints, pPtAry, true );
        EndSolid();
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
    SAL_INFO( "vcl.opengl", "::drawPolyPolygon" );
    if( nPoly <= 0 )
        return;

    PreDraw();

    if( mnFillColor != SALCOLOR_NONE )
    {
        BeginSolid( mnFillColor );
        for( sal_uInt32 i = 0; i < nPoly; i++ )
            DrawPolygon( pPoints[i], pPtAry[i] );
        EndSolid();
    }

    if( mnLineColor != SALCOLOR_NONE )
    {
        // TODO Use glMultiDrawElements or primitive restart
        BeginSolid( mnLineColor );
        for( sal_uInt32 i = 0; i < nPoly; i++ )
            DrawLines( pPoints[i], pPtAry[i], true );
        EndSolid();
    }

    PostDraw();
}

bool OpenGLSalGraphicsImpl::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency )
{
    SAL_INFO( "vcl.opengl", "::drawPolyPolygon trans " << fTransparency );
    if( rPolyPolygon.count() <= 0 )
        return true;

    PreDraw();

    if( mnFillColor != SALCOLOR_NONE )
    {
        BeginSolid( mnFillColor, fTransparency );
        for( sal_uInt32 i = 0; i < rPolyPolygon.count(); i++ )
        {
            const ::basegfx::B2DPolyPolygon aOnePoly( rPolyPolygon.getB2DPolygon( i ) );
            DrawPolyPolygon( aOnePoly );
        }
        EndSolid();
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
    BeginSolid( mnLineColor, fTransparency );
    for( sal_uInt32 i = 0; i < aAreaPolyPoly.count(); i++ )
    {
        const ::basegfx::B2DPolyPolygon aOnePoly( aAreaPolyPoly.getB2DPolygon( i ) );
        DrawPolyPolygon( aOnePoly );
    }
    EndSolid();
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
            long /*nDestX*/, long /*nDestY*/,
            long /*nSrcX*/, long /*nSrcY*/,
            long /*nSrcWidth*/, long /*nSrcHeight*/,
            sal_uInt16 /*nFlags*/ )
{
    SAL_INFO( "vcl.opengl", "::copyArea" );
}

// CopyBits and DrawBitmap --> RasterOp and ClipRegion
// CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
void OpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? dynamic_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : NULL;

    SAL_INFO( "vcl.opengl", "::copyBits" );

    if( pImpl == this &&
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

    if( pImpl->mbOffscreen )
    {
        Size aSize( pImpl->GetWidth(), pImpl->GetHeight() );
        PreDraw();
        DrawTexture( pImpl->maOffscreenTex, rPosAry );
        PostDraw();
        return;
    }

    SAL_WARN( "vcl.opengl", "*** NOT IMPLEMENTED *** copyBits" );
    // TODO Copy from one FBO to the other (glBlitFramebuffer)
}

void OpenGLSalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    // check that carefully only in the debug mode
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture = rBitmap.GetTexture( maContext );

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
    OpenGLTexture& rTexture( rBitmap.GetTexture( maContext ) );
    OpenGLTexture& rMaskTex( rMask.GetTexture( maContext ) );

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
    OpenGLTexture& rTexture( rBitmap.GetTexture( maContext ) );

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
    PreDraw();
    if( !pBitmap->Create( maContext, maOffscreenTex, nX, nY, nWidth, nHeight ) )
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
        BeginInvert();
        DrawRect( nX, nY, nWidth, nHeight );
        EndInvert();
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
        BeginInvert();
        DrawPolygon( nPoints, pPtAry );
        EndInvert();
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
    OpenGLTexture& rTexture( rBitmap.GetTexture( maContext ) );
    OpenGLTexture& rAlphaTex( rAlpha.GetTexture( maContext ) );

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
    OpenGLTexture& rTexture( rBitmap.GetTexture( maContext ) );

    SAL_INFO( "vcl.opengl", "::drawAlphaBitmap" );
    PreDraw();
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTexture( rTexture, rPosAry );
    glDisable( GL_BLEND );
    PostDraw();

    CHECK_GL_ERROR();
    return true;
}

/** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
bool OpenGLSalGraphicsImpl::drawTransformedBitmap(
            const basegfx::B2DPoint& /*rNull*/,
            const basegfx::B2DPoint& /*rX*/,
            const basegfx::B2DPoint& /*rY*/,
            const SalBitmap& /*rSourceBitmap*/,
            const SalBitmap* /*pAlphaBitmap*/)
{
    return false;
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
        BeginSolid( mnFillColor, nTransparency );
        DrawRect( nX, nY, nWidth, nHeight );
        EndSolid();
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
        rGradient.GetStyle() != GradientStyle_RADIAL )
        return false;

    aBoundRect.Left()--;
    aBoundRect.Top()--;
    aBoundRect.Right()++;
    aBoundRect.Bottom()++;
    //TODO: lfrb: some missing transformation with the polygon in outdev

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
        BeginSolid( MAKE_SALCOLOR( aCol.GetRed() * nF / 100,
                                   aCol.GetGreen() * nF / 100,
                                   aCol.GetBlue() * nF / 100 ) );
        DrawRect( aBoundRect );
    }
    else if( rGradient.GetStyle() == GradientStyle_LINEAR )
    {
        DrawLinearGradient( rGradient, aBoundRect );
    }
    else if( rGradient.GetStyle() == GradientStyle_RADIAL )
    {
        DrawRadialGradient( rGradient, aBoundRect );
    }

    if( !mbUseStencil )
        glDisable( GL_STENCIL_TEST );
    PostDraw();

    return true;
}

void OpenGLSalGraphicsImpl::beginPaint()
{
    mnPainting++;
    SAL_INFO( "vcl.opengl", "BEGIN PAINT " << this );
}

void OpenGLSalGraphicsImpl::endPaint()
{
    mnPainting--;
    SAL_INFO( "vcl.opengl", "END PAINT " << this );
    if( mnPainting == 0 )
    {
        maContext.makeCurrent();
        glFlush();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
