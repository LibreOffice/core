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
#include "opengl/zone.hxx"
#include "opengl/salbmp.hxx"
#include "opengl/RenderState.hxx"

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <stdlib.h>

class OpenGLFlushIdle : public Idle
{
    OpenGLSalGraphicsImpl *m_pImpl;
public:
    explicit OpenGLFlushIdle( OpenGLSalGraphicsImpl *pImpl )
        : Idle( "gl idle swap" )
        , m_pImpl( pImpl )
    {
        // We don't want to be swapping before we've painted.
        SetPriority( SchedulerPriority::POST_PAINT );
    }
    virtual ~OpenGLFlushIdle()
    {
    }
    virtual void Invoke() override
    {
        m_pImpl->doFlush();
        SetPriority( SchedulerPriority::HIGHEST );
        Stop();
    }
};

OpenGLSalGraphicsImpl::OpenGLSalGraphicsImpl(SalGraphics& rParent, SalGeometryProvider *pProvider)
    : mpContext(nullptr)
    , mrParent(rParent)
    , mpProvider(pProvider)
    , mpProgram(nullptr)
    , mpFlush(new OpenGLFlushIdle(this))
    , mbUseScissor(false)
    , mbUseStencil(false)
    , mbXORMode(false)
    , mnLineColor(SALCOLOR_NONE)
    , mnFillColor(SALCOLOR_NONE)
#ifdef DBG_UTIL
    , mProgramIsSolidColor(false)
#endif
    , mnDrawCount(0)
    , mnDrawCountAtFlush(0)
    , mProgramSolidColor(SALCOLOR_NONE)
    , mProgramSolidTransparency(0.0)
    , mpAccumulatedTextures(new AccumulatedTextures)
{
}

OpenGLSalGraphicsImpl::~OpenGLSalGraphicsImpl()
{
    if( !IsOffscreen() && mnDrawCountAtFlush != mnDrawCount )
        VCL_GL_INFO( "Destroying un-flushed on-screen graphics" );

    delete mpFlush;

    ReleaseContext();
}

rtl::Reference<OpenGLContext> OpenGLSalGraphicsImpl::GetOpenGLContext()
{
    if( !AcquireContext(true) )
        return nullptr;
    return mpContext;
}

bool OpenGLSalGraphicsImpl::AcquireContext( bool bForceCreate )
{
    mpContext = OpenGLContext::getVCLContext( false );

    if( !mpContext.is() && mpWindowContext.is() )
    {
        mpContext = mpWindowContext;
    }
    else if( bForceCreate && !IsOffscreen() )
    {
        mpWindowContext = CreateWinContext();
        mpContext = mpWindowContext;
    }

    if( !mpContext.is() )
        mpContext = OpenGLContext::getVCLContext();

    return mpContext.is();
}

bool OpenGLSalGraphicsImpl::ReleaseContext()
{
    mpContext.clear();

    return true;
}

void OpenGLSalGraphicsImpl::Init()
{
    // Our init phase is strange ::Init is called twice for vdevs.
    // the first time around with a NULL geometry provider.
    if( !mpProvider )
        return;

    // check if we can simply re-use the same context
    if( mpContext.is() )
    {
        if( !UseContext( mpContext ) )
            ReleaseContext();
    }

    // Always create the offscreen texture
    if( maOffscreenTex.GetWidth()  != GetWidth() ||
        maOffscreenTex.GetHeight() != GetHeight() )
    {
        // We don't want to be swapping before we've painted.
        mpFlush->SetPriority( SchedulerPriority::POST_PAINT );

        if( maOffscreenTex && // don't work to release empty textures
            mpContext.is() )  // valid context
        {
            mpContext->makeCurrent();
            mpContext->ReleaseFramebuffer( maOffscreenTex );
        }
        maOffscreenTex = OpenGLTexture();
        VCL_GL_INFO("::Init - re-size offscreen texture");
    }

    if( mpWindowContext.is() )
    {
        mpWindowContext->reset();
        mpWindowContext.clear();
    }
}

// Currently only used to get windows ordering right.
void OpenGLSalGraphicsImpl::DeInit()
{
    // tdf#93839:
    // Our window handles and resources are being free underneath us.
    // These can be bound into a context, which relies on them. So
    // let it know. Other eg. VirtualDevice contexts which have
    // references on and rely on this context continuing to work will
    // get a shiny new context in AcquireContext:: next PreDraw.
    if( mpWindowContext.is() )
    {
        mpWindowContext->reset();
        mpWindowContext.clear();
    }
    mpContext.clear();
}

void OpenGLSalGraphicsImpl::PreDraw(XOROption eOpt)
{
    FlushDeferredDrawing();

    InitializePreDrawState(eOpt);
}

void OpenGLSalGraphicsImpl::InitializePreDrawState(XOROption eOpt)
{
    OpenGLZone::enter();

    mnDrawCount++;

    if( !AcquireContext() )
    {
        SAL_WARN( "vcl.opengl", "Couldn't acquire context" );
        return;
    }

    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    CheckOffscreenTexture();
    CHECK_GL_ERROR();

    mpContext->state()->viewport(Rectangle(Point(0, 0), Size(GetWidth(), GetHeight())));

    ImplInitClipRegion();
    CHECK_GL_ERROR();

    if (eOpt == IMPLEMENT_XOR && mbXORMode)
    {
        glEnable(GL_COLOR_LOGIC_OP);
        CHECK_GL_ERROR();

        glLogicOp(GL_XOR);
    }
}

void OpenGLSalGraphicsImpl::PostDraw()
{
    if (mbXORMode)
    {
        glDisable(GL_COLOR_LOGIC_OP);
        CHECK_GL_ERROR();
    }

    if( mpProgram )
    {
        mpProgram->Clean();
        mpProgram = nullptr;
#ifdef DBG_UTIL
        mProgramIsSolidColor = false;
#endif
    }

    assert (maOffscreenTex);

    // Always queue the flush.
    if( !IsOffscreen() )
        flush();

    OpenGLZone::leave();
}

void OpenGLSalGraphicsImpl::ApplyProgramMatrices(float fPixelOffset)
{
    mpProgram->ApplyMatrix(GetWidth(), GetHeight(), fPixelOffset);
}

void OpenGLSalGraphicsImpl::freeResources()
{
    // TODO Delete shaders, programs and textures if not shared
    if( mpContext.is() && mpContext->isInitialized() )
    {
        VCL_GL_INFO( "freeResources" );
        mpContext->makeCurrent();
        FlushDeferredDrawing();
        mpContext->ReleaseFramebuffer( maOffscreenTex );
    }
    ReleaseContext();
}

void OpenGLSalGraphicsImpl::ImplSetClipBit( const vcl::Region& rClip, GLuint nMask )
{
    mpContext->state()->scissor().disable();
    mpContext->state()->stencil().enable();

    VCL_GL_INFO( "Adding complex clip / stencil" );
    GLuint nStencil = maOffscreenTex.StencilId();
    if( nStencil == 0 )
    {
        nStencil = maOffscreenTex.AddStencil();
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, nStencil );
        CHECK_GL_ERROR();
    }
    // else - we associated the stencil in
    //        AcquireFrameBuffer / AttachTexture

    CHECK_GL_ERROR();
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    CHECK_GL_ERROR();
    glStencilMask( nMask );
    CHECK_GL_ERROR();
    glStencilFunc( GL_NEVER, nMask, 0xFF );
    CHECK_GL_ERROR();
    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP );
    CHECK_GL_ERROR();

    glClear( GL_STENCIL_BUFFER_BIT );
    CHECK_GL_ERROR();
    if( UseSolid( MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ) ) )
    {
        if( rClip.getRegionBand() )
            DrawRegionBand( *rClip.getRegionBand() );
        else
            DrawPolyPolygon( rClip.GetAsB2DPolyPolygon(), true );
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    CHECK_GL_ERROR();
    glStencilMask( 0x00 );
    CHECK_GL_ERROR();

    mpContext->state()->stencil().disable();
}

void OpenGLSalGraphicsImpl::ImplInitClipRegion()
{
    // make sure the context has the right clipping set
    if (maClipRegion != mpContext->maClipRegion)
    {
        mpContext->maClipRegion = maClipRegion;
        if (mbUseStencil)
        {
            ImplSetClipBit(maClipRegion, 0x01);
        }
    }

    if (mbUseScissor)
    {
        Rectangle aRect(maClipRegion.GetBoundRect());
        mpContext->state()->scissor().set(aRect.Left(), GetHeight() - aRect.Bottom() - 1, aRect.GetWidth(), aRect.GetHeight());
        mpContext->state()->scissor().enable();
    }
    else
    {
        mpContext->state()->scissor().disable();
    }

    if (mbUseStencil)
    {
        glStencilFunc( GL_EQUAL, 1, 0x1 );
        CHECK_GL_ERROR();
        mpContext->state()->stencil().enable();
    }
    else
    {
        mpContext->state()->stencil().disable();
    }
}

const vcl::Region& OpenGLSalGraphicsImpl::getClipRegion() const
{
    return maClipRegion;
}

bool OpenGLSalGraphicsImpl::setClipRegion( const vcl::Region& rClip )
{
    if (maClipRegion == rClip)
    {
         VCL_GL_INFO("::setClipRegion (no change) " << rClip);
         return true;
    }

    FlushDeferredDrawing();

    VCL_GL_INFO("::setClipRegion " << rClip);

    maClipRegion = rClip;

    mbUseStencil = false;
    mbUseScissor = false;
    if (maClipRegion.IsRectangle())
        mbUseScissor = true;
    else if (!maClipRegion.IsEmpty())
        mbUseStencil = true;

    return true;
}

// set the clip region to empty
void OpenGLSalGraphicsImpl::ResetClipRegion()
{
    if (maClipRegion.IsEmpty())
    {
        VCL_GL_INFO("::ResetClipRegion (no change) ");
        return;
    }

    FlushDeferredDrawing();

    VCL_GL_INFO("::ResetClipRegion");

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
void OpenGLSalGraphicsImpl::SetXORMode( bool bSet, bool )
{
    mbXORMode = bSet;
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
    bool bClearTexture = false;

    VCL_GL_INFO( "Check Offscreen texture" );

    // Always create the offscreen texture
    if( maOffscreenTex )
    {
        if( maOffscreenTex.GetWidth()  != GetWidth() ||
            maOffscreenTex.GetHeight() != GetHeight() )
        {
            VCL_GL_INFO( "re-size offscreen texture " << maOffscreenTex.Id() );
            mpFlush->SetPriority( SchedulerPriority::POST_PAINT );
            mpContext->ReleaseFramebuffer( maOffscreenTex );
            maOffscreenTex = OpenGLTexture();
        }
    }

    if( !maOffscreenTex )
    {
        VCL_GL_INFO( "create texture of size "
                     << GetWidth() << " x " << GetHeight() );
        maOffscreenTex = OpenGLTexture( GetWidth(), GetHeight() );
        bClearTexture = true;
    }

    if( !maOffscreenTex.IsUnique() )
    {
        GLfloat fWidth = GetWidth();
        GLfloat fHeight = GetHeight();
        SalTwoRect aPosAry(0, 0, fWidth, fHeight, 0,0, fWidth, fHeight);

        // TODO: lfrb: User GL_ARB_copy_image?
        OpenGLTexture aNewTex = OpenGLTexture( GetWidth(), GetHeight() );

        mpContext->state()->scissor().disable();
        mpContext->state()->stencil().disable();

        mpContext->AcquireFramebuffer( aNewTex );
        DrawTexture( maOffscreenTex, aPosAry );
        maOffscreenTex = aNewTex;
    }
    else
    {
        mpContext->AcquireFramebuffer( maOffscreenTex );
        CHECK_GL_ERROR();

        if( bClearTexture )
        {
            glDrawBuffer( GL_COLOR_ATTACHMENT0 );
#if OSL_DEBUG_LEVEL > 0 // lets have some red debugging background.
            GLfloat clearColor[4] = { 1.0, 0, 0, 0 };
#else
            GLfloat clearColor[4] = { 1.0, 1.0, 1.0, 0 };
#endif
            glClearBufferfv( GL_COLOR, 0, clearColor );
            // FIXME: use glClearTexImage if we have it ?
        }
    }

    assert( maOffscreenTex );

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalGraphicsImpl::UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble )
{
    if( mpProgram != nullptr )
        mpProgram->Clean();
    mpProgram = mpContext->UseProgram( rVertexShader, rFragmentShader, preamble );
#ifdef DBG_UTIL
    mProgramIsSolidColor = false; // UseSolid() will set to true if needed
#endif
    return ( mpProgram != nullptr );
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor, sal_uInt8 nTransparency )
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !UseProgram( "dumbVertexShader", "solidFragmentShader" ) )
        return false;
    mpProgram->SetColor( "color", nColor, nTransparency );
#ifdef DBG_UTIL
    mProgramIsSolidColor = true;
#endif
    mProgramSolidColor = nColor;
    mProgramSolidTransparency = nTransparency / 100.0;

    return true;
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor, double fTransparency )
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !UseProgram( "dumbVertexShader", "solidFragmentShader" ) )
        return false;
    mpProgram->SetColorf( "color", nColor, fTransparency );
#ifdef DBG_UTIL
    mProgramIsSolidColor = true;
#endif
    mProgramSolidColor = nColor;
    mProgramSolidTransparency = fTransparency;
    return true;
}

bool OpenGLSalGraphicsImpl::UseInvert50()
{
    if( !UseProgram( "dumbVertexShader", "invert50FragmentShader" ) )
        return false;
    return true;
}

bool OpenGLSalGraphicsImpl::UseSolid( SalColor nColor )
{
    return UseSolid( nColor, 0.0f );
}

// Like UseSolid(), but sets up for AA drawing, which uses gradients to create the AA.
bool OpenGLSalGraphicsImpl::UseSolidAA( SalColor nColor, double fTransparency )
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !mrParent.getAntiAliasB2DDraw())
        return UseSolid( nColor );
    if( !UseProgram( "textureVertexShader", "linearGradientFragmentShader" ) )
        return false;
    mpProgram->SetColorf( "start_color", nColor, fTransparency );
    mpProgram->SetColorf( "end_color", nColor, 1.0f );
    return true;
}

bool OpenGLSalGraphicsImpl::UseSolidAA( SalColor nColor )
{
    return UseSolidAA( nColor, 0.0 );
}

bool OpenGLSalGraphicsImpl::UseInvert( SalInvert nFlags )
{
    OpenGLZone aZone;

    if( ( nFlags & SalInvert::N50 ) ||
        ( nFlags & SalInvert::TrackFrame ) )
    {
        // FIXME: Trackframe really should be 2 pix. on/off stipple.
        if( !UseInvert50() )
            return false;
        mpProgram->SetBlendMode( GL_ONE_MINUS_DST_COLOR,
                                 GL_ONE_MINUS_SRC_COLOR );
    }
    else
    {
        if( !UseSolid( MAKE_SALCOLOR( 255, 255, 255 ) ) )
            return false;
        mpProgram->SetBlendMode( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
    }
    return true;
}

void OpenGLSalGraphicsImpl::DrawPoint( long nX, long nY )
{
    OpenGLZone aZone;

    GLfloat pPoint[2];

    pPoint[0] = GLfloat(nX);
    pPoint[1] = GLfloat(nY);

    ApplyProgramMatrices(0.5f);
    mpProgram->SetVertices( pPoint );
    glDrawArrays( GL_POINTS, 0, 1 );
    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawLine( double nX1, double nY1, double nX2, double nY2 )
{
    OpenGLZone aZone;

    GLfloat pPoints[4];

    pPoints[0] = GLfloat(nX1);
    pPoints[1] = GLfloat(nY1);
    pPoints[2] = GLfloat(nX2);
    pPoints[3] = GLfloat(nY2);

    ApplyProgramMatrices(0.5f);
    mpProgram->SetVertices( pPoints );
    glDrawArrays( GL_LINES, 0, 2 );
    CHECK_GL_ERROR();
}

namespace
{

inline void addVertex(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors, glm::vec2 point, glm::vec2 extrusionVector, float length)
{
    rVertices.push_back(point.x);
    rVertices.push_back(point.y);

    rExtrusionVectors.push_back(extrusionVector.x);
    rExtrusionVectors.push_back(extrusionVector.y);
    rExtrusionVectors.push_back(length);
}

inline void addVertexPair(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors, const glm::vec2& point, const glm::vec2& extrusionVector, float length)
{
    addVertex(rVertices, rExtrusionVectors, point, -extrusionVector, -length);
    addVertex(rVertices, rExtrusionVectors, point,  extrusionVector,  length);
}

inline glm::vec2 normalize(const glm::vec2& vector)
{
    if (glm::length(vector) > 0.0)
        return glm::normalize(vector);
    return vector;
}

} // end anonymous namespace

void OpenGLSalGraphicsImpl::DrawLineCap(float x1, float y1, float x2, float y2, css::drawing::LineCap eLineCap, float fLineWidth)
{
    if (eLineCap != css::drawing::LineCap_ROUND && eLineCap != css::drawing::LineCap_SQUARE)
        return;

    OpenGLZone aZone;

    const int nRoundCapIteration = 12;

    std::vector<GLfloat> aVertices;
    std::vector<GLfloat> aExtrusionVectors;

    glm::vec2 p1(x1, y1);
    glm::vec2 p2(x2, y2);
    glm::vec2 lineVector = normalize(p2 - p1);
    glm::vec2 normal = glm::vec2(-lineVector.y, lineVector.x);

    if (eLineCap == css::drawing::LineCap_ROUND)
    {
        for (int nFactor = 0; nFactor <= nRoundCapIteration; nFactor++)
        {
            float angle = float(nFactor) * (M_PI / float(nRoundCapIteration));
            glm::vec2 roundNormal(normal.x * glm::cos(angle) - normal.y * glm::sin(angle),
                                  normal.x * glm::sin(angle) + normal.y * glm::cos(angle));

            addVertexPair(aVertices, aExtrusionVectors, p1, roundNormal, 1.0f);
        }
    }
    else if (eLineCap == css::drawing::LineCap_SQUARE)
    {
        glm::vec2 extrudedPoint = p1 + -lineVector * (fLineWidth / 2.0f);

        addVertexPair(aVertices, aExtrusionVectors, extrudedPoint, normal, 1.0f);
        addVertexPair(aVertices, aExtrusionVectors, p1, normal, 1.0f);
    }

    ApplyProgramMatrices(0.0f);
    mpProgram->SetExtrusionVectors(aExtrusionVectors.data());
    mpProgram->SetVertices(aVertices.data());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, aVertices.size() / 2);

    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawLineSegment(float x1, float y1, float x2, float y2)
{
    glm::vec2 p1(x1, y1);
    glm::vec2 p2(x2, y2);

    if (p1.x == p2.x && p1.y == p2.y)
        return;

    std::vector<GLfloat> aPoints;
    std::vector<GLfloat> aExtrusionVectors;

    OpenGLZone aZone;

    glm::vec2 lineVector = normalize(p2 - p1);
    glm::vec2 normal = glm::vec2(-lineVector.y, lineVector.x);

    addVertexPair(aPoints, aExtrusionVectors, p1, normal, 1.0f);
    addVertexPair(aPoints, aExtrusionVectors, p2, normal, 1.0f);

    ApplyProgramMatrices(0.0f);
    mpProgram->SetExtrusionVectors(aExtrusionVectors.data());
    mpProgram->SetVertices(aPoints.data());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, aPoints.size() / 2);

    CHECK_GL_ERROR();
}

/** Draw a simple (non bezier) polyline
 *
 * OpenGL polyline drawing algorithm inspired by:
 * - http://mattdesl.svbtle.com/drawing-lines-is-hard
 * - https://www.mapbox.com/blog/drawing-antialiased-lines/
 * - https://cesiumjs.org/2013/04/22/Robust-Polyline-Rendering-with-WebGL/
 * - http://artgrammer.blogspot.si/2011/05/drawing-nearly-perfect-2d-line-segments.html
 * - http://artgrammer.blogspot.si/2011/07/drawing-polylines-by-tessellation.html
 *
 */
void OpenGLSalGraphicsImpl::DrawPolyLine(const basegfx::B2DPolygon& rPolygon, float fLineWidth, basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap, float fMiterMinimumAngle)
{
    sal_uInt32 nPoints = rPolygon.count();
    bool bClosed = rPolygon.isClosed();

    if (!bClosed && nPoints >= 2)
    {
        // draw begin cap
        {
            glm::vec2 p1(rPolygon.getB2DPoint(0).getX(), rPolygon.getB2DPoint(0).getY());
            glm::vec2 p2(rPolygon.getB2DPoint(1).getX(), rPolygon.getB2DPoint(1).getY());
            DrawLineCap(p1.x, p1.y, p2.x, p2.y, eLineCap, fLineWidth);
        }

        // draw end cap
        {
            glm::vec2 p1(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
            glm::vec2 p2(rPolygon.getB2DPoint(nPoints - 2).getX(), rPolygon.getB2DPoint(nPoints - 2).getY());
            DrawLineCap(p1.x, p1.y, p2.x, p2.y, eLineCap, fLineWidth);
        }
    }

    if (nPoints == 2 || eLineJoin == basegfx::B2DLineJoin::NONE)
    {
        // If line joint is NONE or a simple line with 2 points, draw the polyline
        // each line segment separatly.
        for (int i = 0; i < int(nPoints) - 1; ++i)
        {
            glm::vec2 p1(rPolygon.getB2DPoint(i+0).getX(), rPolygon.getB2DPoint(i+0).getY());
            glm::vec2 p2(rPolygon.getB2DPoint(i+1).getX(), rPolygon.getB2DPoint(i+1).getY());
            DrawLineSegment(p1.x, p1.y, p2.x, p2.y);
        }
        if (bClosed)
        {
            glm::vec2 p1(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
            glm::vec2 p2(rPolygon.getB2DPoint(0).getX(), rPolygon.getB2DPoint(0).getY());
            DrawLineSegment(p1.x, p1.y, p2.x, p2.y);
        }
    }
    else if (nPoints > 2)
    {
        OpenGLZone aZone;

        int i = 0;
        int lastPoint = int(nPoints);

        std::vector<GLfloat> aVertices;
        std::vector<GLfloat> aExtrusionVectors;

        // First guess on the size, but we could know relatively exactly
        // how much vertices we need.
        aVertices.reserve(nPoints * 4);
        aExtrusionVectors.reserve(nPoints * 6);

        // Handle first point

        glm::vec2 nextLineVector;
        glm::vec2 previousLineVector;
        glm::vec2 normal; // perpendicular to the line vector

        glm::vec2 p0(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
        glm::vec2 p1(rPolygon.getB2DPoint(0).getX(), rPolygon.getB2DPoint(0).getY());
        glm::vec2 p2(rPolygon.getB2DPoint(1).getX(), rPolygon.getB2DPoint(1).getY());

        nextLineVector = normalize(p2 - p1);

        if (!bClosed)
        {
            normal = glm::vec2(-nextLineVector.y, nextLineVector.x); // make perpendicular
            addVertexPair(aVertices, aExtrusionVectors, p1, normal, 1.0f);

            i++; // first point done already
            lastPoint--; // last point will be calculated separatly from the loop

            p0 = p1;
            previousLineVector = nextLineVector;
        }
        else
        {
            lastPoint++; // we need to connect last point to first point so one more line segment to calculate

            previousLineVector = normalize(p1 - p0);
        }

        for (; i < lastPoint; ++i)
        {
            int index1 = (i + 0) % nPoints; // loop indices - important when polyline is closed
            int index2 = (i + 1) % nPoints;

            p1 = glm::vec2(rPolygon.getB2DPoint(index1).getX(), rPolygon.getB2DPoint(index1).getY());
            p2 = glm::vec2(rPolygon.getB2DPoint(index2).getX(), rPolygon.getB2DPoint(index2).getY());

            if (p1 == p2) // skip equal points, normals could div-by-0
                continue;

            nextLineVector = normalize(p2 - p1);

            if (eLineJoin == basegfx::B2DLineJoin::Miter)
            {
                float angle = std::atan2(previousLineVector.x * nextLineVector.y - previousLineVector.y * nextLineVector.x,
                                         previousLineVector.x * nextLineVector.x + previousLineVector.y * nextLineVector.y);

                angle = F_PI - std::fabs(angle);

                if (angle < fMiterMinimumAngle)
                    eLineJoin = basegfx::B2DLineJoin::Bevel;
            }

            if (eLineJoin == basegfx::B2DLineJoin::Miter)
            {
                // With miter join we calculate the extrusion vector by adding normals of
                // previous and next line segment. The vector shows the way but we also
                // need the length (otherwise the line will be deformed). Length factor is
                // calculated as dot product of extrusion vector and one of the normals.
                // The value we get is the inverse length (used in the shader):
                // length = line_width / dot(extrusionVector, normal)

                normal = glm::vec2(-previousLineVector.y, previousLineVector.x);

                glm::vec2 tangent = normalize(nextLineVector + previousLineVector);
                glm::vec2 extrusionVector(-tangent.y, tangent.x);
                GLfloat length = glm::dot(extrusionVector, normal);

                addVertexPair(aVertices, aExtrusionVectors, p1, extrusionVector, length);
            }
            else if (eLineJoin == basegfx::B2DLineJoin::Bevel)
            {
                // For bevel join we just add 2 additional vertices and use previous
                // line segment normal and next line segment normal as extrusion vector.
                // All the magic is done by the fact that we draw triangle strips, so we
                // cover the joins correctly.

                glm::vec2 previousNormal = glm::vec2(-previousLineVector.y, previousLineVector.x);
                glm::vec2 nextNormal = glm::vec2(-nextLineVector.y, nextLineVector.x);

                addVertexPair(aVertices, aExtrusionVectors, p1, previousNormal, 1.0f);
                addVertexPair(aVertices, aExtrusionVectors, p1, nextNormal, 1.0f);
            }
            else if (eLineJoin == basegfx::B2DLineJoin::Round)
            {
                // For round join we do a similar thing as in bevel, we add more intermediate
                // vertices and add normals to get extrusion vectors in the between the
                // both normals.

                // 3 additional extrusion vectors + normals are enough to make most
                // line joins look round. Ideally the number of vectors could be
                // calculated.

                glm::vec2 previousNormal = glm::vec2(-previousLineVector.y, previousLineVector.x);
                glm::vec2 nextNormal = glm::vec2(-nextLineVector.y, nextLineVector.x);

                glm::vec2 middle = normalize(previousNormal + nextNormal);
                glm::vec2 middleLeft  = normalize(previousNormal + middle);
                glm::vec2 middleRight = normalize(middle + nextNormal);

                addVertexPair(aVertices, aExtrusionVectors, p1, previousNormal, 1.0f);
                addVertexPair(aVertices, aExtrusionVectors, p1, middleLeft, 1.0f);
                addVertexPair(aVertices, aExtrusionVectors, p1, middle, 1.0f);
                addVertexPair(aVertices, aExtrusionVectors, p1, middleRight, 1.0f);
                addVertexPair(aVertices, aExtrusionVectors, p1, nextNormal, 1.0f);
            }
            p0 = p1;
            previousLineVector = nextLineVector;
        }

        if (!bClosed)
        {
            // Create vertices for the last point. There is no line join so just
            // use the last line segment normal as the extrusion vector.

            p1 = glm::vec2(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());

            normal = glm::vec2(-previousLineVector.y, previousLineVector.x);

            addVertexPair(aVertices, aExtrusionVectors, p1, normal, 1.0f);
        }

        ApplyProgramMatrices(0.0f);
        mpProgram->SetExtrusionVectors(aExtrusionVectors.data());
        mpProgram->SetVertices(aVertices.data());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, aVertices.size() / 2);

        CHECK_GL_ERROR();
    }
}

bool OpenGLSalGraphicsImpl::UseLine(SalColor nColor, double fTransparency, GLfloat fLineWidth)
{
    if( nColor == SALCOLOR_NONE )
        return false;
    if( !UseProgram( "lineVertexShader", "lineFragmentShader" ) )
        return false;
    mpProgram->SetColorf("color", nColor, fTransparency);
    mpProgram->SetUniform1f("line_width", fLineWidth);
    // The width of the feather - area we make lineary transparent in VS.
    // Good AA value is 0.5
    mpProgram->SetUniform1f("feather", 0.5f);
    // We need blending or AA won't work correctly
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#ifdef DBG_UTIL
    mProgramIsSolidColor = true;
#endif
    mProgramSolidColor = nColor;
    mProgramSolidTransparency = fTransparency;
    return true;
}

void OpenGLSalGraphicsImpl::DrawLineAA( double nX1, double nY1, double nX2, double nY2 )
{
    OpenGLZone aZone;

    if( !mrParent.getAntiAliasB2DDraw())
        return DrawLine( nX1, nY1, nX2, nY2 );

    if( nX1 == nX2 || nY1 == nY2 )
    {   // Horizontal/vertical, no need for AA, both points have normal color.

        // Still set up for the trivial "gradients", because presumably UseSolidAA() has been called.
        GLfloat aTexCoord[4] = { 0, 1, 1, 1 };
        mpProgram->SetTextureCoord( aTexCoord );
        DrawLine(nX1, nY1, nX2, nY2);

        return;
    }
    ImplDrawLineAA( nX1, nY1, nX2, nY2 );
}

void OpenGLSalGraphicsImpl::ImplDrawLineAA( double nX1, double nY1, double nX2, double nY2, bool edge )
{
    // Draw the line anti-aliased. Based on code with the following notice:
    /* Drawing nearly perfect 2D line segments in OpenGL
     * You can use this code however you want.
     * I just hope you to cite my name and the page of this technique:
     * http://artgrammer.blogspot.com/2011/05/drawing-nearly-perfect-2d-line-segments.html
     * http://www.codeproject.com/KB/openGL/gllinedraw.aspx
     *
     * Enjoy. Chris Tsang.*/

    double x1 = nX1;
    double y1 = nY1;
    double x2 = nX2;
    double y2 = nY2;

    // A special hack for drawing lines that are in fact AA edges of a shape. Make the line somewhat
    // wider, but (done further below) draw the entire width as a gradient. This would be wrong for a line
    // (too wide and seemingly less straight), but it makes the edges look smoother and the width difference
    // is almost unnoticeable.
    const double w = edge ? 1.4 : 1.0;

    double t(0.0);
    double R(0.0);
    double f = w - static_cast<int>(w);
    //determine parameters t,R
    if ( w>=0.0 && w<1.0 )
    {
        t=0.05;
        R=0.48+0.32*f;
    }
    else if ( w>=1.0 && w<2.0 )
    {
        t=0.05+f*0.33;
        R=0.768+0.312*f;
    }
    else if ( w>=2.0 && w<3.0 )
    {
        t=0.38+f*0.58;
        R=1.08;
    }
    else if ( w>=3.0 && w<4.0 )
    {
        t=0.96+f*0.48;
        R=1.08;
    }
    else if ( w>=4.0 && w<5.0 )
    {
        t=1.44+f*0.46;
        R=1.08;
    }
    else if ( w>=5.0 && w<6.0 )
    {
        t=1.9+f*0.6;
        R=1.08;
    }
    else if ( w>=6.0 )
    {
        double ff=w-6.0;
        t=2.5+ff*0.50;
        R=1.08;
    }

    //determine angle of the line to horizontal
    double tx=0,ty=0; //core thinkness of a line
    double Rx=0,Ry=0; //fading edge of a line
    double dx=x2-x1;
    double dy=y2-y1;
    if ( w < 3 )
    {   //approximate to make things even faster
        double m=dy/dx;
        //and calculate tx,ty,Rx,Ry
        if ( m>-0.4142 && m<=0.4142)
        {
            // -22.5< angle <= 22.5, approximate to 0 (degree)
            tx=t*0.1; ty=t;
            Rx=R*0.6; Ry=R;
        }
        else if ( m>0.4142 && m<=2.4142)
        {
            // 22.5< angle <= 67.5, approximate to 45 (degree)
            tx=t*-0.7071; ty=t*0.7071;
            Rx=R*-0.7071; Ry=R*0.7071;
        }
        else if ( m>2.4142 || m<=-2.4142)
        {
            // 67.5 < angle <=112.5, approximate to 90 (degree)
            tx=t; ty=t*0.1;
            Rx=R; Ry=R*0.6;
        }
        else if ( m>-2.4142 && m<-0.4142)
        {
            // 112.5 < angle < 157.5, approximate to 135 (degree)
            tx=t*0.7071; ty=t*0.7071;
            Rx=R*0.7071; Ry=R*0.7071;
        }
        else
            assert( false );
    }
    else
    { //calculate to exact
        dx=y1-y2;
        dy=x2-x1;
        double L=sqrt(dx*dx+dy*dy);
        dx/=L;
        dy/=L;
        tx=t*dx; ty=t*dy;
        Rx=R*dx; Ry=R*dy;
    }

    if( edge )
    {   // See above.
        Rx += tx;
        Ry += ty;
        tx = ty = 0;
    }

    GLfloat vertices[]=
    {
        GLfloat(x1-tx-Rx), GLfloat(y1-ty-Ry), //fading edge1
        GLfloat(x2-tx-Rx), GLfloat(y2-ty-Ry),
        GLfloat(x1-tx),    GLfloat(y1-ty),    //core
        GLfloat(x2-tx),    GLfloat(y2-ty),
        GLfloat(x1+tx),    GLfloat(y1+ty),
        GLfloat(x2+tx),    GLfloat(y2+ty),
        GLfloat(x1+tx+Rx), GLfloat(y1+ty+Ry), //fading edge2
        GLfloat(x2+tx+Rx), GLfloat(y2+ty+Ry)
    };

    ApplyProgramMatrices(0.0f);
    GLfloat aTexCoord[16] = { 0, 0, 1, 0, 2, 1, 3, 1, 4, 1, 5, 1, 6, 0, 7, 0 };
    mpProgram->SetTextureCoord( aTexCoord );
    mpProgram->SetVertices( vertices );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
    CHECK_GL_ERROR();
}


void OpenGLSalGraphicsImpl::DrawLinesAA( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose )
{
    for( int i = 0; i < int(nPoints) - 1; ++i )
        DrawLineAA( pPtAry[ i ].mnX, pPtAry[ i ].mnY, pPtAry[ i + 1 ].mnX, pPtAry[ i + 1 ].mnY );
    if( bClose )
        DrawLineAA( pPtAry[ nPoints - 1 ].mnX, pPtAry[ nPoints - 1 ].mnY, pPtAry[ 0 ].mnX, pPtAry[ 0 ].mnY );
}

void OpenGLSalGraphicsImpl::DrawEdgeAA( double nX1, double nY1, double nX2, double nY2 )
{
    assert( mrParent.getAntiAliasB2DDraw());
    if( nX1 == nX2 || nY1 == nY2 )
        return; //horizontal/vertical, no need for AA
    ImplDrawLineAA( nX1, nY1, nX2, nY2, true );
}

void OpenGLSalGraphicsImpl::DrawConvexPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry, bool blockAA )
{
    OpenGLZone aZone;

    std::vector<GLfloat> aVertices(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++, j += 2 )
    {
        aVertices[j]   = GLfloat(pPtAry[i].mnX);
        aVertices[j+1] = GLfloat(pPtAry[i].mnY);
    }

    ApplyProgramMatrices();
    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
    CHECK_GL_ERROR();

    if( !blockAA && mrParent.getAntiAliasB2DDraw())
    {
        // Make the edges antialiased by drawing the edge lines again with AA.
        // TODO: If transparent drawing is set up, drawing the lines themselves twice
        // may be a problem, if that is a real problem, the polygon areas itself needs to be
        // masked out for this or something.
#ifdef DBG_UTIL
        assert( mProgramIsSolidColor );
#endif
        SalColor lastSolidColor = mProgramSolidColor;
        double lastSolidTransparency = mProgramSolidTransparency;
        if( UseSolidAA( lastSolidColor, lastSolidTransparency ))
        {
            for( i = 0; i < nPoints; ++i )
            {
                const SalPoint& rPt1 = pPtAry[ i ];
                const SalPoint& rPt2 = pPtAry[ ( i + 1 ) % nPoints ];
                DrawEdgeAA( rPt1.mnX, rPt1.mnY, rPt2.mnX, rPt2.mnY );
            }
            UseSolid( lastSolidColor, lastSolidTransparency );
        }
    }
}

void OpenGLSalGraphicsImpl::DrawConvexPolygon( const tools::Polygon& rPolygon, bool blockAA )
{
    OpenGLZone aZone;

    sal_uInt16 nPoints = rPolygon.GetSize() - 1;
    std::vector<GLfloat> aVertices(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++, j += 2 )
    {
        const Point& rPt = rPolygon.GetPoint( i );
        aVertices[j]   = GLfloat(rPt.X());
        aVertices[j+1] = GLfloat(rPt.Y());
    }

    ApplyProgramMatrices();
    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
    CHECK_GL_ERROR();

    if( !blockAA && mrParent.getAntiAliasB2DDraw())
    {
        // Make the edges antialiased by drawing the edge lines again with AA.
        // TODO: If transparent drawing is set up, drawing the lines themselves twice
        // may be a problem, if that is a real problem, the polygon areas itself needs to be
        // masked out for this or something.
#ifdef DBG_UTIL
        assert( mProgramIsSolidColor );
#endif
        SalColor lastSolidColor = mProgramSolidColor;
        double lastSolidTransparency = mProgramSolidTransparency;
        if( UseSolidAA( lastSolidColor, lastSolidTransparency ))
        {
            for( i = 0; i < nPoints; ++i )
            {
                const Point& rPt1 = rPolygon.GetPoint( i );
                const Point& rPt2 = rPolygon.GetPoint(( i + 1 ) % nPoints );
                DrawEdgeAA( rPt1.getX(), rPt1.getY(), rPt2.getX(), rPt2.getY());
            }
            UseSolid( lastSolidColor, lastSolidTransparency );
        }
    }
}

void OpenGLSalGraphicsImpl::DrawTrapezoid( const basegfx::B2DTrapezoid& trapezoid, bool blockAA )
{
    OpenGLZone aZone;

    const basegfx::B2DPolygon& rPolygon = trapezoid.getB2DPolygon();
    sal_uInt16 nPoints = rPolygon.count();
    std::vector<GLfloat> aVertices(nPoints * 2);
    sal_uInt32 i, j;

    for( i = 0, j = 0; i < nPoints; i++, j += 2 )
    {
        const basegfx::B2DPoint& rPt = rPolygon.getB2DPoint( i );
        aVertices[j]   = GLfloat(rPt.getX());
        aVertices[j+1] = GLfloat(rPt.getY());
    }

    if (!mpProgram)
    {
        SAL_WARN("vcl.opengl", "OpenGLSalGraphicsImpl::DrawTrapezoid: mpProgram is 0");
        return;
    }

    ApplyProgramMatrices();
    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, nPoints );
    CHECK_GL_ERROR();

    if( !blockAA && mrParent.getAntiAliasB2DDraw())
    {
        // Make the edges antialiased by drawing the edge lines again with AA.
        // TODO: If transparent drawing is set up, drawing the lines themselves twice
        // may be a problem, if that is a real problem, the polygon areas itself needs to be
        // masked out for this or something.
#ifdef DBG_UTIL
        assert( mProgramIsSolidColor );
#endif
        SalColor lastSolidColor = mProgramSolidColor;
        double lastSolidTransparency = mProgramSolidTransparency;
        if( UseSolidAA( lastSolidColor, lastSolidTransparency ))
        {
            for( i = 0; i < nPoints; ++i )
            {
                const basegfx::B2DPoint& rPt1 = rPolygon.getB2DPoint( i );
                const basegfx::B2DPoint& rPt2 = rPolygon.getB2DPoint(( i + 1 ) % nPoints );
                DrawEdgeAA( rPt1.getX(), rPt1.getY(), rPt2.getX(), rPt2.getY());
            }
            UseSolid( lastSolidColor, lastSolidTransparency );
        }
    }
}

void OpenGLSalGraphicsImpl::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    long nX1( nX );
    long nY1( nY );
    long nX2( nX + nWidth );
    long nY2( nY + nHeight );
    const SalPoint aPoints[] = { { nX1, nY2 }, { nX1, nY1 },
                                 { nX2, nY1 }, { nX2, nY2 }};

    DrawConvexPolygon( 4, aPoints, true );
}

void OpenGLSalGraphicsImpl::DrawRect( const Rectangle& rRect )
{
    long nX1( rRect.Left() );
    long nY1( rRect.Top() );
    long nX2( rRect.Right() );
    long nY2( rRect.Bottom() );
    const SalPoint aPoints[] = { { nX1, nY2 }, { nX1, nY1 },
                                 { nX2, nY1 }, { nX2, nY2 }};

    DrawConvexPolygon( 4, aPoints, true );
}

void OpenGLSalGraphicsImpl::DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    basegfx::B2DPolygon aPolygon;

    for( sal_uInt32 i = 0; i < nPoints; i++ )
        aPolygon.append( basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
    aPolygon.setClosed( true );

    if( basegfx::tools::isConvex( aPolygon ) )
    {
        if( nPoints > 2L )
            DrawConvexPolygon( nPoints, pPtAry );
    }
    else
    {
        const basegfx::B2DPolyPolygon aPolyPolygon( aPolygon );
        DrawPolyPolygon( aPolyPolygon );
    }
}

void OpenGLSalGraphicsImpl::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, bool blockAA )
{
    const basegfx::B2DPolyPolygon& aSimplePolyPolygon = ::basegfx::tools::solveCrossovers( rPolyPolygon );
    basegfx::B2DTrapezoidVector aB2DTrapVector;
    basegfx::tools::trapezoidSubdivide( aB2DTrapVector, aSimplePolyPolygon );
    // draw tesselation result
    if( aB2DTrapVector.size())
    {
        for( size_t i = 0; i < aB2DTrapVector.size(); ++i )
            DrawTrapezoid( aB2DTrapVector[ i ], blockAA );
    }
}

void OpenGLSalGraphicsImpl::DrawRegionBand( const RegionBand& rRegion )
{
    OpenGLZone aZone;

    RectangleVector aRects;
    std::vector<GLfloat> aVertices;
    rRegion.GetRegionRectangles( aRects );

    if( aRects.empty() )
        return;

#define ADD_VERTICE(pt) \
    aVertices.push_back(GLfloat(pt.X())); \
    aVertices.push_back(GLfloat(pt.Y()));

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

    ApplyProgramMatrices();
    mpProgram->SetVertices( &aVertices[0] );
    glDrawArrays( GL_TRIANGLES, 0, aVertices.size() / 2 );
    CHECK_GL_ERROR();
}

void OpenGLSalGraphicsImpl::DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted )
{
    OpenGLZone aZone;

    SAL_INFO("vcl.opengl", "draw texture rect");

    GLfloat aTexCoord[8];
    rTexture.GetCoord( aTexCoord, rPosAry, bInverted );
    mpProgram->SetTextureCoord( aTexCoord );
    DrawRect( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight );
}

void OpenGLSalGraphicsImpl::DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& pPosAry, bool bInverted )
{
    OpenGLZone aZone;

    SAL_INFO("vcl.opengl", "draw texture");

    if( !UseProgram( "textureVertexShader", "textureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    DrawTextureRect( rTexture, pPosAry, bInverted );
    mpProgram->Clean();
}

namespace {

bool scaleTexture(const rtl::Reference< OpenGLContext > &xContext,
    OpenGLTexture& rOutTexture, const double& ixscale, const double& iyscale, OpenGLTexture& rTexture)
{
    int nWidth = rTexture.GetWidth();
    int nHeight = rTexture.GetHeight();
    int nNewWidth = nWidth / ixscale;
    int nNewHeight = nHeight / iyscale;

    OpenGLProgram* pProgram = xContext->UseProgram("textureVertexShader", "areaScaleFragmentShader");
    if (pProgram == nullptr)
        return false;

    OpenGLTexture aScratchTex(nNewWidth, nNewHeight);
    OpenGLFramebuffer* pFramebuffer = xContext->AcquireFramebuffer(aScratchTex);

    pProgram->SetUniform1f("xscale", ixscale);
    pProgram->SetUniform1f("yscale", iyscale);
    pProgram->SetUniform1i("swidth", nWidth);
    pProgram->SetUniform1i("sheight", nHeight);
    // For converting between <0,nWidth-1> and <0.0,1.0> coordinate systems.
    pProgram->SetUniform1f("xsrcconvert", 1.0 / (nWidth - 1));
    pProgram->SetUniform1f("ysrcconvert", 1.0 / (nHeight - 1));
    pProgram->SetUniform1f("xdestconvert", 1.0 * (nNewWidth - 1));
    pProgram->SetUniform1f("ydestconvert", 1.0 * (nNewHeight - 1));

    pProgram->SetTexture("sampler", rTexture);
    pProgram->DrawTexture(rTexture);
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();

    rOutTexture = aScratchTex;
    return true;
}

}

void OpenGLSalGraphicsImpl::DrawTransformedTexture(
    OpenGLTexture& rTexture,
    OpenGLTexture& rMask,
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY )
{
    OpenGLZone aZone;

    GLfloat aVertices[8] = {
        0, (float) rTexture.GetHeight(), 0, 0,
        (float) rTexture.GetWidth(), 0, (float) rTexture.GetWidth(), (float) rTexture.GetHeight() };
    GLfloat aTexCoord[8];

    const long nDestWidth = basegfx::fround(basegfx::B2DVector(rX - rNull).getLength());
    const long nDestHeight = basegfx::fround(basegfx::B2DVector(rY - rNull).getLength());

    // Invisibly small images shouldn't divide by zero.
    if( nDestHeight == 0 || nDestWidth == 0 )
        return;

    // inverted scale ratios
    double ixscale = rTexture.GetWidth()  / double(nDestWidth);
    double iyscale = rTexture.GetHeight() / double(nDestHeight);

    // If downscaling at a higher scale ratio, use the area scaling algorithm rather
    // than plain OpenGL's scaling (texture mapping), for better results.
    // See OpenGLSalBitmap::ImplScaleArea().
    bool areaScaling = false;
    bool fastAreaScaling = false;
    OUString textureFragmentShader;
    if( ixscale >= 2 && iyscale >= 2 )  // scale ratio less than 50%
    {
        areaScaling = true;
        fastAreaScaling = ( ixscale == int( ixscale ) && iyscale == int( iyscale ));
        // The generic case has arrays only up to 16 ratio downscaling and is performed in 2 passes,
        // when the ratio is in the 16-100 range, which is hopefully enough in practice, but protect
        // against buffer overflows in case such an extreme case happens (and in such case the precision
        // of the generic algorithm probably doesn't matter anyway).
        if( ixscale > 100 || iyscale > 100 )
            fastAreaScaling = true;
        if( fastAreaScaling )
            textureFragmentShader = "areaScaleFastFragmentShader";
        else
            textureFragmentShader = "areaScaleFragmentShader";
    }

    OpenGLTexture aInTexture = rTexture;
    OpenGLTexture aInMask = rMask;

    // When using the area scaling algorithm we need to reduce the texture size in 2 passes
    // in order to not use a big array inside the fragment shader.
    if (areaScaling && !fastAreaScaling)
    {
        // Perform a first texture downscaling by an inverted scale ratio equal to
        // the square root of the whole inverted scale ratio.
        if (ixscale > 16 || iyscale > 16)
        {
            // The scissor area is set to the current window size in PreDraw,
            // so if we do not disable the scissor test, the texture produced
            // by the first downscaling is clipped to the current window size.
            mpContext->state()->scissor().disable();
            mpContext->state()->stencil().disable();

            // the square root of the whole inverted scale ratio
            double ixscalesqrt = std::floor(std::sqrt(ixscale));
            double iyscalesqrt = std::floor(std::sqrt(iyscale));
            ixscale /= ixscalesqrt; // second pass inverted x-scale factor
            iyscale /= iyscalesqrt; // second pass inverted y-scale factor

            scaleTexture(mpContext, aInTexture, ixscalesqrt, iyscalesqrt, rTexture);

            if (rMask) // we need to downscale the mask too
            {
                scaleTexture(mpContext, aInMask, ixscalesqrt, iyscalesqrt, rMask);
            }

            // We need to re-acquire the off-screen texture.
            CheckOffscreenTexture();
            CHECK_GL_ERROR();

            // Re-enable scissor and stencil tests if needed.
            if (mbUseScissor)
                mpContext->state()->scissor().enable();

            if (mbUseStencil)
                mpContext->state()->stencil().enable();
        }
    }

    if( aInMask  )
    {
        if( !UseProgram( "transformedTextureVertexShader",
                textureFragmentShader.isEmpty() ? "maskedTextureFragmentShader" : textureFragmentShader,
                "#define MASKED" ) )
            return;
        mpProgram->SetTexture( "mask", aInMask );
        GLfloat aMaskCoord[8];
        aInMask.GetWholeCoord(aMaskCoord);
        mpProgram->SetMaskCoord(aMaskCoord);
        aInMask.SetFilter( GL_LINEAR );
        mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
        if( !UseProgram( "transformedTextureVertexShader",
                textureFragmentShader.isEmpty() ? "textureFragmentShader" : textureFragmentShader ) )
            return;
    }

    if(areaScaling)
    {
        int nWidth = aInTexture.GetWidth();
        int nHeight = aInTexture.GetHeight();

        // From OpenGLSalBitmap::ImplScaleArea().
        if (fastAreaScaling && nWidth && nHeight)
        {
            mpProgram->SetUniform1i( "xscale", ixscale );
            mpProgram->SetUniform1i( "yscale", iyscale );
            mpProgram->SetUniform1f( "xstep", 1.0 / nWidth );
            mpProgram->SetUniform1f( "ystep", 1.0 / nHeight );
            mpProgram->SetUniform1f( "ratio", 1.0 / ( ixscale * iyscale ));
        }
        else if (nHeight > 1 && nWidth > 1)
        {
            mpProgram->SetUniform1f( "xscale", ixscale );
            mpProgram->SetUniform1f( "yscale", iyscale );
            mpProgram->SetUniform1i( "swidth", nWidth );
            mpProgram->SetUniform1i( "sheight", nHeight );
            // For converting between <0,nWidth-1> and <0.0,1.0> coordinate systems.
            mpProgram->SetUniform1f( "xsrcconvert", 1.0 / ( nWidth - 1 ));
            mpProgram->SetUniform1f( "ysrcconvert", 1.0 / ( nHeight - 1 ));
            mpProgram->SetUniform1f( "xdestconvert", 1.0 * (( nWidth / ixscale ) - 1 ));
            mpProgram->SetUniform1f( "ydestconvert", 1.0 * (( nHeight / iyscale ) - 1 ));
        }
    }

    ApplyProgramMatrices();
    mpProgram->SetUniform2f( "viewport", GetWidth(), GetHeight() );
    // Here, in order to get the correct transformation we need to pass the original texture,
    // since it has been used for initializing the rectangle vertices.
    mpProgram->SetTransform( "transform", rTexture, rNull, rX, rY );
    aInTexture.GetWholeCoord(aTexCoord);
    mpProgram->SetTexture("sampler", aInTexture);
    aInTexture.SetFilter(GL_LINEAR);
    mpProgram->SetTextureCoord( aTexCoord );
    mpProgram->SetVertices( aVertices );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

    CHECK_GL_ERROR();
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawAlphaTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted, bool bPremultiplied )
{
    OpenGLZone aZone;

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
    OpenGLZone aZone;

    if( !UseProgram( "maskedTextureVertexShader", "diffTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "texture", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLfloat aMaskCoord[8];
    rMask.GetCoord(aMaskCoord, rPosAry, bInverted);
    mpProgram->SetMaskCoord(aMaskCoord);

    DrawTextureRect( rTexture, rPosAry, bInverted );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry )
{
    OpenGLZone aZone;

    if( !UseProgram( "maskedTextureVertexShader", "maskedTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLfloat aTexCoord[8];
    rTexture.GetCoord(aTexCoord, rPosAry);
    mpProgram->SetTextureCoord(aTexCoord);

    GLfloat aMaskCoord[8];
    rMask.GetCoord(aMaskCoord, rPosAry);
    mpProgram->SetMaskCoord(aMaskCoord);

    DrawRect(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawBlendedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, OpenGLTexture& rAlpha, const SalTwoRect& rPosAry )
{
    OpenGLZone aZone;

    if( !UseProgram( "blendedTextureVertexShader", "blendedTextureFragmentShader" ) )
        return;
    mpProgram->SetTexture( "sampler", rTexture );
    mpProgram->SetTexture( "mask", rMask );
    mpProgram->SetTexture( "alpha", rAlpha );

    GLfloat aAlphaCoord[8];
    rAlpha.GetCoord(aAlphaCoord, rPosAry);
    mpProgram->SetAlphaCoord(aAlphaCoord);

    GLfloat aMaskCoord[8];
    rMask.GetCoord(aMaskCoord, rPosAry);
    mpProgram->SetMaskCoord(aMaskCoord);

    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rTexture, rPosAry );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DrawMask( OpenGLTexture& rMask, SalColor nMaskColor, const SalTwoRect& pPosAry )
{
    OpenGLZone aZone;

    if( !UseProgram( "textureVertexShader", "maskFragmentShader" ) )
        return;
    mpProgram->SetColor( "color", nMaskColor, 0 );
    mpProgram->SetTexture( "sampler", rMask );
    mpProgram->SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    DrawTextureRect( rMask, pPosAry );
    mpProgram->Clean();
}

void OpenGLSalGraphicsImpl::DeferredTextDraw(OpenGLTexture& rTexture, SalColor aMaskColor, const SalTwoRect& rPosAry)
{
    mpAccumulatedTextures->insert(rTexture, aMaskColor, rPosAry);
}

void OpenGLSalGraphicsImpl::FlushDeferredDrawing()
{
    if (mpAccumulatedTextures->empty())
        return;

    InitializePreDrawState();

    VCL_GL_INFO("FlushDeferredDrawing");

    OpenGLZone aZone;

#if 0 // Draw a background rect under text for debugging - same color shows text from the same texture
    static sal_uInt8 r = 0xBE;
    static sal_uInt8 g = 0xF0;
    static sal_uInt8 b = 0xFF;
    static std::unordered_map<GLuint, Color> aColorForTextureMap;


    for (auto& rPair : mpAccumulatedTextures->getAccumulatedTexturesMap())
    {
        OpenGLTexture& rTexture = rPair.second->maTexture;
        Color aUseColor;
        if (aColorForTextureMap.find(rTexture.Id()) == aColorForTextureMap.end())
        {
            Color aColor(r, g, b);
            sal_uInt16 h,s,br;
            aColor.RGBtoHSB(h, s, br);
            aColor = Color::HSBtoRGB((h + 40) % 360, s, br);
            r = aColor.GetRed();
            g = aColor.GetGreen();
            b = aColor.GetBlue();
            aColorForTextureMap[rTexture.Id()] = aColor;
        }
        aUseColor = aColorForTextureMap[rTexture.Id()];

        if (!UseSolid(MAKE_SALCOLOR(aUseColor.GetRed(), aUseColor.GetGreen(), aUseColor.GetBlue())))
            return;
        for (auto rColorTwoRectPair: rPair.second->maColorTextureDrawParametersMap)
        {
            TextureDrawParameters& rParameters = rColorTwoRectPair.second;
            ApplyProgramMatrices();
            mpProgram->SetTextureCoord(rParameters.maTextureCoords.data());
            mpProgram->SetVertices(rParameters.maVertices.data());
            glDrawArrays(GL_TRIANGLES, 0, rParameters.getNumberOfVertices());
        }
    }
#endif

    if( !UseProgram( "textureVertexShader", "maskFragmentShader" ) )
        return;

    mpProgram->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& rPair : mpAccumulatedTextures->getAccumulatedTexturesMap())
    {
        OpenGLTexture& rTexture = rPair.second->maTexture;
        mpProgram->SetTexture("sampler", rTexture);
        for (auto& rColorTwoRectPair: rPair.second->maColorTextureDrawParametersMap)
        {
            mpProgram->SetColor("color", rColorTwoRectPair.first, 0);
            TextureDrawParameters& rParameters = rColorTwoRectPair.second;
            ApplyProgramMatrices();
            mpProgram->SetTextureCoord(rParameters.maTextureCoords.data());
            mpProgram->SetVertices(rParameters.maVertices.data());
            glDrawArrays(GL_TRIANGLES, 0, rParameters.getNumberOfVertices());
        }
    }
    mpProgram->Clean();
    mpAccumulatedTextures->clear();

    PostDraw();
}

void OpenGLSalGraphicsImpl::DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    OpenGLZone aZone;

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
    tools::Polygon aPoly( aBoundRect );
    aPoly.Rotate( aCenter, rGradient.GetAngle() % 3600 );

    GLfloat aTexCoord[8] = { 0, 1, 1, 1, 1, 0, 0, 0 };
    GLfloat fMin = 1.0 - 100.0 / (100.0 - rGradient.GetBorder());
    aTexCoord[5] = aTexCoord[7] = fMin;
    mpProgram->SetTextureCoord( aTexCoord );
    DrawConvexPolygon( aPoly, true );
}

void OpenGLSalGraphicsImpl::DrawAxialGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    OpenGLZone aZone;

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

    tools::Polygon aPoly( 7 );
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
    DrawConvexPolygon( aPoly, true );
}

void OpenGLSalGraphicsImpl::DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect )
{
    OpenGLZone aZone;

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
    VCL_GL_INFO( "::drawPixel" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw( XOROption::IMPLEMENT_XOR );
        if( UseSolid( mnLineColor ) )
            DrawPoint( nX, nY );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawPixel( long nX, long nY, SalColor nSalColor )
{
    VCL_GL_INFO( "::drawPixel" );
    if( nSalColor != SALCOLOR_NONE )
    {
        PreDraw( XOROption::IMPLEMENT_XOR );
        if( UseSolid( nSalColor ) )
            DrawPoint( nX, nY );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    VCL_GL_INFO( "::drawLine" );
    if( mnLineColor != SALCOLOR_NONE )
    {
        PreDraw( XOROption::IMPLEMENT_XOR );
        if( UseSolidAA( mnLineColor ) )
            DrawLineAA( nX1, nY1, nX2, nY2 );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    VCL_GL_INFO( "::drawRect" );
    PreDraw( XOROption::IMPLEMENT_XOR );

    if( UseSolid( mnFillColor ) )
        DrawRect( nX, nY, nWidth, nHeight );

    if( UseSolid( mnLineColor ) )
    {
        GLfloat fX1(nX);
        GLfloat fY1(nY);
        GLfloat fX2(nX + nWidth - 1);
        GLfloat fY2(nY + nHeight - 1);

        GLfloat pPoints[8];

        pPoints[0] = fX1;
        pPoints[1] = fY1;
        pPoints[2] = fX2;
        pPoints[3] = fY1;
        pPoints[4] = fX2;
        pPoints[5] = fY2;
        pPoints[6] = fX1;
        pPoints[7] = fY2;

        ApplyProgramMatrices(0.5f);
        mpProgram->SetVertices(pPoints);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        CHECK_GL_ERROR();
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    VCL_GL_INFO( "::drawPolyLine" );

    if( mnLineColor != SALCOLOR_NONE && nPoints > 1 )
    {
        PreDraw( XOROption::IMPLEMENT_XOR );
        if( UseSolidAA( mnLineColor ) )
            DrawLinesAA( nPoints, pPtAry, false );
        PostDraw();
    }
}

void OpenGLSalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    VCL_GL_INFO( "::drawPolygon" );
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

    PreDraw( XOROption::IMPLEMENT_XOR );

    if( UseSolid( mnFillColor ) )
        DrawPolygon( nPoints, pPtAry );

    if( UseSolidAA( mnLineColor ) )
        DrawLinesAA( nPoints, pPtAry, true );

    PostDraw();
}

void OpenGLSalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
    VCL_GL_INFO( "::drawPolyPolygon" );
    if( nPoly <= 0 )
        return;

    PreDraw( XOROption::IMPLEMENT_XOR );

    if( UseSolid( mnFillColor ) )
    {
        if( nPoly == 1 )
            DrawPolygon( pPoints[ 0 ], pPtAry[ 0 ] );
        else
        {
            basegfx::B2DPolyPolygon polyPolygon;
            for( sal_uInt32 i = 0; i < nPoly; ++i )
            {
                basegfx::B2DPolygon polygon;
                for( sal_uInt32 j = 0; j < pPoints[ i ]; ++j )
                    polygon.append( basegfx::B2DPoint( pPtAry[i][j].mnX, pPtAry[i][j].mnY ) );
                polygon.setClosed( true );
                polyPolygon.append( polygon );
            }
            DrawPolyPolygon( polyPolygon );
        }
    }

    if( mnLineColor != mnFillColor && UseSolidAA( mnLineColor ) )
    {
        // TODO Use glMultiDrawElements or primitive restart
        for( sal_uInt32 i = 0; i < nPoly; i++ )
            DrawLinesAA( pPoints[i], pPtAry[i], true );
    }

    PostDraw();
}

bool OpenGLSalGraphicsImpl::drawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency )
{
    VCL_GL_INFO( "::drawPolyPolygon trans " << fTransparency );
    if( rPolyPolygon.count() <= 0 )
        return true;

    PreDraw( XOROption::IMPLEMENT_XOR );

    if( UseSolid( mnFillColor, fTransparency ) )
        DrawPolyPolygon( rPolyPolygon );

    if( mnLineColor != mnFillColor && UseSolid( mnLineColor, fTransparency ))
    {
        basegfx::B2DTrapezoidVector aB2DTrapVector;
        basegfx::tools::createLineTrapezoidFromB2DPolyPolygon( aB2DTrapVector, rPolyPolygon );
        for( size_t i = 0; i < aB2DTrapVector.size(); ++i )
            DrawTrapezoid( aB2DTrapVector[ i ] );
    }

    PostDraw();

    return true;
}

bool OpenGLSalGraphicsImpl::drawPolyLine(
            const basegfx::B2DPolygon& rPolygon,
            double fTransparency,
            const basegfx::B2DVector& rLineWidth,
            basegfx::B2DLineJoin eLineJoin,
            css::drawing::LineCap eLineCap,
            double fMiterMinimumAngle)
{
    VCL_GL_INFO( "::drawPolyLine trans " << fTransparency );
    if( mnLineColor == SALCOLOR_NONE )
        return true;

    const bool bIsHairline = (rLineWidth.getX() == rLineWidth.getY()) && (rLineWidth.getX() <= 1.2);
    const float fLineWidth = bIsHairline ? 1.0f : rLineWidth.getX();

    PreDraw(XOROption::IMPLEMENT_XOR);

    if (UseLine(mnLineColor, 0.0f, fLineWidth))
    {
        basegfx::B2DPolygon aPolygon(rPolygon);

        if (aPolygon.areControlPointsUsed())
            aPolygon = basegfx::tools::polygonSubdivide(aPolygon, 5 * F_PI180);
        else
            aPolygon.removeDoublePoints();

        DrawPolyLine(aPolygon, fLineWidth, eLineJoin, eLineCap, fMiterMinimumAngle);
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
    VCL_GL_INFO( "::copyArea " << nSrcX << "," << nSrcY << " >> " << nDestX << "," << nDestY << " (" << nSrcWidth << "," << nSrcHeight << ")" );
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
    VCL_GL_INFO( "::copyBits" );

    rImpl.FlushDeferredDrawing();

    if( !rImpl.maOffscreenTex )
    {
        VCL_GL_INFO( "::copyBits - skipping copy of un-initialized framebuffer contents of size "
                     << rImpl.GetWidth() << "x" << rImpl.GetHeight() );
        return;
    }

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

    PreDraw();
    DrawTexture( rImpl.maOffscreenTex, rPosAry );
    PostDraw();
}

void OpenGLSalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    // check that carefully only in the debug mode
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture = rBitmap.GetTexture();

    VCL_GL_INFO( "::drawBitmap" );
    PreDraw();
    DrawTexture( rTexture, rPosAry );
    PostDraw();
}

void OpenGLSalGraphicsImpl::drawBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap,
            const SalBitmap& rMaskBitmap )
{
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rMaskBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    const OpenGLSalBitmap& rMask = static_cast<const OpenGLSalBitmap&>(rMaskBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture& rMaskTex( rMask.GetTexture() );

    VCL_GL_INFO( "::drawBitmap with MASK" );
    PreDraw();
    DrawTextureWithMask( rTexture, rMaskTex, rPosAry );
    PostDraw();
}

void OpenGLSalGraphicsImpl::drawMask(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalBitmap,
            SalColor nMaskColor )
{
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );

    VCL_GL_INFO( "::drawMask" );
    PreDraw();
    DrawMask( rTexture, nMaskColor, rPosAry );
    PostDraw();
}

SalBitmap* OpenGLSalGraphicsImpl::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    OpenGLZone aZone;

    OpenGLSalBitmap* pBitmap = new OpenGLSalBitmap;
    VCL_GL_INFO( "::getBitmap " << nX << "," << nY <<
              " " << nWidth << "x" << nHeight );
    //TODO really needed?
    PreDraw();
    if( !pBitmap->Create( maOffscreenTex, nX, nY, nWidth, nHeight ) )
    {
        delete pBitmap;
        pBitmap = nullptr;
    }
    PostDraw();
    return pBitmap;
}

SalColor OpenGLSalGraphicsImpl::getPixel( long nX, long nY )
{
    FlushDeferredDrawing();

    char pixel[3] = { 0, 0, 0 };

    PreDraw( XOROption::IMPLEMENT_XOR );
    nY = GetHeight() - nY - 1;
    glReadPixels( nX, nY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    CHECK_GL_ERROR();
    PostDraw();

    return MAKE_SALCOLOR( pixel[0], pixel[1], pixel[2] );
}

// invert --> ClipRegion (only Windows or VirDevs)
void OpenGLSalGraphicsImpl::invert(
            long nX, long nY,
            long nWidth, long nHeight,
            SalInvert nFlags)
{
    PreDraw();

    if( UseInvert( nFlags ) )
    {
        if( nFlags & SalInvert::TrackFrame )
        { // FIXME: could be more efficient.
            DrawRect( nX, nY, nWidth, 1 );
            DrawRect( nX, nY + nHeight, nWidth, 1 );
            DrawRect( nX, nY, 1, nHeight );
            DrawRect( nX + nWidth, nY, 1, nHeight );
        }
        else
            DrawRect( nX, nY, nWidth, nHeight );
    }

    PostDraw();
}

void OpenGLSalGraphicsImpl::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
    PreDraw();

    if( UseInvert( nFlags ) )
        DrawPolygon( nPoints, pPtAry );

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
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );

    VCL_GL_INFO( "::blendBitmap" );
    PreDraw();
    glEnable( GL_BLEND );
    CHECK_GL_ERROR();
    glBlendFunc( GL_ZERO, GL_SRC_COLOR );
    CHECK_GL_ERROR();
    DrawTexture( rTexture, rPosAry );
    glDisable( GL_BLEND );
    CHECK_GL_ERROR();
    PostDraw();
    return true;
}

bool OpenGLSalGraphicsImpl::blendAlphaBitmap(
            const SalTwoRect& rPosAry,
            const SalBitmap& rSalSrcBitmap,
            const SalBitmap& rSalMaskBitmap,
            const SalBitmap& rSalAlphaBitmap )
{
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalSrcBitmap));
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalMaskBitmap));
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalAlphaBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rSrcBitmap = static_cast<const OpenGLSalBitmap&>(rSalSrcBitmap);
    const OpenGLSalBitmap& rMaskBitmap = static_cast<const OpenGLSalBitmap&>(rSalMaskBitmap);
    const OpenGLSalBitmap& rAlphaBitmap = static_cast<const OpenGLSalBitmap&>(rSalAlphaBitmap);
    OpenGLTexture& rTexture( rSrcBitmap.GetTexture() );
    OpenGLTexture& rMask( rMaskBitmap.GetTexture() );
    OpenGLTexture& rAlpha( rAlphaBitmap.GetTexture() );

    VCL_GL_INFO( "::blendAlphaBitmap" );
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
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSalBitmap));
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rAlphaBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSalBitmap);
    const OpenGLSalBitmap& rAlpha = static_cast<const OpenGLSalBitmap&>(rAlphaBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture& rAlphaTex( rAlpha.GetTexture() );

    VCL_GL_INFO( "::drawAlphaBitmap" );
    PreDraw();
    DrawTextureWithMask( rTexture, rAlphaTex, rPosAry );
    PostDraw();
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
    assert(dynamic_cast<const OpenGLSalBitmap*>(&rSrcBitmap));
    assert(!pAlphaBitmap || dynamic_cast<const OpenGLSalBitmap*>(pAlphaBitmap));

    OpenGLZone aZone;

    const OpenGLSalBitmap& rBitmap = static_cast<const OpenGLSalBitmap&>(rSrcBitmap);
    const OpenGLSalBitmap* pMaskBitmap = static_cast<const OpenGLSalBitmap*>(pAlphaBitmap);
    OpenGLTexture& rTexture( rBitmap.GetTexture() );
    OpenGLTexture aMask; // no texture

    if( pMaskBitmap != nullptr )
        aMask = pMaskBitmap->GetTexture();

    VCL_GL_INFO( "::drawTransformedBitmap" );
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
    VCL_GL_INFO( "::drawAlphaRect" );
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

    VCL_GL_INFO( "::drawGradient" );

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

    PreDraw( XOROption::IMPLEMENT_XOR );

#define FIXME_BROKEN_STENCIL_FOR_GRADIENTS 0
#if FIXME_BROKEN_STENCIL_FOR_GRADIENTS
    ImplSetClipBit( vcl::Region( rPolyPoly ), 0x02 );
    if( mbUseStencil )
    {
        glEnable( GL_STENCIL_TEST );
        CHECK_GL_ERROR();
        glStencilFunc( GL_EQUAL, 3, 0xFF );
        CHECK_GL_ERROR();
    }
    else
    {
        glEnable( GL_STENCIL_TEST );
        CHECK_GL_ERROR();
        glStencilFunc( GL_EQUAL, 2, 0xFF );
        CHECK_GL_ERROR();
    }
#endif

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

#if FIXME_BROKEN_STENCIL_FOR_GRADIENTS
    if( !mbUseStencil )
    {
        glDisable( GL_STENCIL_TEST );
        CHECK_GL_ERROR();
    }
#endif
    PostDraw();

    return true;
}

void OpenGLSalGraphicsImpl::flush()
{
    FlushDeferredDrawing();

    if( IsOffscreen() )
        return;

    if( !Application::IsInExecute() )
    {
        // otherwise nothing would trigger idle rendering
        doFlush();
    }
    else if( !mpFlush->IsActive() )
        mpFlush->Start();
}

void OpenGLSalGraphicsImpl::doFlush()
{
    FlushDeferredDrawing();

    if (OpenGLContext::hasCurrent())
    {
        mpContext->state()->scissor().disable();
        mpContext->state()->stencil().disable();
    }

    if( IsOffscreen() )
        return;

    if( !maOffscreenTex )
    {
        VCL_GL_INFO( "flushAndSwap - odd no texture !" );
        return;
    }

    if( mnDrawCountAtFlush == mnDrawCount )
    {
        VCL_GL_INFO( "eliding redundant flushAndSwap, no drawing since last!" );
        return;
    }

    mnDrawCountAtFlush = mnDrawCount;

    OpenGLZone aZone;

    VCL_GL_INFO( "flushAndSwap" );

    if( !mpWindowContext.is() )
    {
        // ensure everything is released from the old context.
        OpenGLContext::clearCurrent();
        mpWindowContext = CreateWinContext();
        VCL_GL_INFO( "late creation of window context" );
    }

    assert( mpWindowContext.is() );

    // Interesting ! -> this destroys a context [ somehow ] ...
    mpWindowContext->makeCurrent();
    CHECK_GL_ERROR();

    VCL_GL_INFO( "flushAndSwap - acquire default framebuffer" );

    mpWindowContext->state()->sync();

    mpWindowContext->AcquireDefaultFramebuffer();
    CHECK_GL_ERROR();

    mpWindowContext->state()->viewport(Rectangle(Point(0, 0), Size(GetWidth(), GetHeight())));
    mpWindowContext->state()->scissor().disable();
    mpWindowContext->state()->stencil().disable();

#if OSL_DEBUG_LEVEL > 0 // random background glClear
    glClearColor((float)rand()/RAND_MAX, (float)rand()/RAND_MAX,
                 (float)rand()/RAND_MAX, 1.0);
#else
    glClearColor(1.0, 1.0, 1.0, 1.0);
#endif
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    CHECK_GL_ERROR();

    VCL_GL_INFO( "Texture height " << maOffscreenTex.GetHeight() << " vs. window height " << GetHeight() );

    OpenGLProgram *pProgram =
        mpWindowContext->UseProgram( "textureVertexShader", "textureFragmentShader", "// flush shader\n" ); // flush helps profiling
    if( !pProgram )
        VCL_GL_INFO( "Can't compile simple copying shader !" );
    else
    {
        pProgram->SetTexture( "sampler", maOffscreenTex );

        SalTwoRect aPosAry( 0, 0, maOffscreenTex.GetWidth(), maOffscreenTex.GetHeight(),
                            0, 0, maOffscreenTex.GetWidth(), maOffscreenTex.GetHeight() );

        GLfloat aTexCoord[8];
        maOffscreenTex.GetCoord( aTexCoord, aPosAry );
        pProgram->SetTextureCoord( aTexCoord );

        GLfloat fWidth( maOffscreenTex.GetWidth() );
        GLfloat fHeight( maOffscreenTex.GetHeight() );
        const GLfloat aVertices[] = { 0, fHeight,
                                      0, 0,
                                      fWidth, 0,
                                      fWidth, fHeight };

        pProgram->ApplyMatrix(GetWidth(), GetHeight(), 0.0);
        pProgram->SetVertices( &aVertices[0] );
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        pProgram->Clean();

        maOffscreenTex.Unbind();

        static bool bNoSwap = getenv("SAL_GL_NO_SWAP");
        if (!bNoSwap)
            mpWindowContext->swapBuffers();
    }

    VCL_GL_INFO( "flushAndSwap - end." );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
