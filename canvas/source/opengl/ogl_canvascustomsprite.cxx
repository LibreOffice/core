/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ogl_canvascustomsprite.hxx"
#include "ogl_canvastools.hxx"
#include "ogl_tools.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/verifyinput.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <GL/glew.h>

using namespace ::com::sun::star;

namespace oglcanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D& rSpriteSize,
                                            const SpriteCanvasRef&                        rRefDevice,
                                            SpriteDeviceHelper&                           rDeviceHelper ) :
        mpSpriteCanvas( rRefDevice ),
        maSize(rSpriteSize),
        mxClip(),
        maTransformation(),
        maPosition(),
        mfAlpha(0.0),
        mfPriority(0.0)
    {
        ENSURE_OR_THROW( rRefDevice.get(),
                         "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        ::canvas::tools::setIdentityAffineMatrix2D(maTransformation);
        maCanvasHelper.init( *rRefDevice.get(),
                             rDeviceHelper );
    }

    void CanvasCustomSprite::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();
        mRenderHelper.dispose();
        // forward to parent
        CanvasCustomSpriteBaseT::disposeThis();
    }

    void SAL_CALL CanvasCustomSprite::setAlpha( double alpha ) throw (lang::IllegalArgumentException,
                                                                      uno::RuntimeException, std::exception)
    {
        canvas::tools::verifyRange( alpha, 0.0, 1.0 );

        ::osl::MutexGuard aGuard( m_aMutex );
        mfAlpha = alpha;
    }

    void SAL_CALL CanvasCustomSprite::move( const geometry::RealPoint2D&  aNewPos,
                                            const rendering::ViewState&   viewState,
                                            const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException,
                                                                                               uno::RuntimeException, std::exception)
    {
        canvas::tools::verifyArgs(aNewPos, viewState, renderState,
                                  BOOST_CURRENT_FUNCTION,
                                  static_cast< ::cppu::OWeakObject* >(this));

        ::osl::MutexGuard aGuard( m_aMutex );
        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        // convert position to device pixel
        maPosition = ::basegfx::unotools::b2DPointFromRealPoint2D(aNewPos);
        maPosition *= aTransform;
    }

    void SAL_CALL CanvasCustomSprite::transform( const geometry::AffineMatrix2D& aTransformation ) throw (lang::IllegalArgumentException,
                                                                                                          uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        maTransformation = aTransformation;
    }

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >& xClip ) throw (uno::RuntimeException, std::exception)
    {
        mxClip = xClip;
    }

    void SAL_CALL CanvasCustomSprite::setPriority( double nPriority ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        mfPriority = nPriority;
    }

    void SAL_CALL CanvasCustomSprite::show() throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if( mpSpriteCanvas.is() )
            mpSpriteCanvas->show(this);
    }

    void SAL_CALL CanvasCustomSprite::hide() throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if( mpSpriteCanvas.is() )
            mpSpriteCanvas->hide(this);
    }

    uno::Reference< rendering::XCanvas > SAL_CALL CanvasCustomSprite::getContentCanvas() throw (uno::RuntimeException, std::exception)
    {
        return this;
    }


    bool CanvasCustomSprite::renderSprite() const
    {
        if( ::basegfx::fTools::equalZero( mfAlpha ) )
            return true;

        TransformationPreserver aPreserver1;
        const ::basegfx::B2IVector aSpriteSizePixel(
            ::canvas::tools::roundUp( maSize.Width ),
            ::canvas::tools::roundUp( maSize.Height ));
        // translate sprite to output position
        glTranslated(maPosition.getX(), maPosition.getY(), 0);

        {
            TransformationPreserver aPreserver2;

            // apply sprite content transformation matrix
            double aGLTransform[] =
                {
                    maTransformation.m00, maTransformation.m10, 0, 0,
                    maTransformation.m01, maTransformation.m11, 0, 0,
                    0,                    0,                    1, 0,
                    maTransformation.m02, maTransformation.m12, 0, 1
                };
            glMultMatrixd(aGLTransform);

            IBufferContextSharedPtr pBufferContext;
            if( mfAlpha != 1.0 || mxClip.is() )
            {
                // drats. need to render to temp surface before, and then
                // composite that to screen

                // TODO(P3): buffer texture
                pBufferContext = maCanvasHelper.getDeviceHelper()->createBufferContext(aSpriteSizePixel);
                pBufferContext->startBufferRendering();
            }

            // this ends up in pBufferContext, if that one's "current"
            if( !maCanvasHelper.renderRecordedActions() )
                return false;

            if( pBufferContext )
            {
                // content ended up in background buffer - compose to
                // screen now. Calls below switches us back to window
                // context, and binds to generated, dynamic texture
                pBufferContext->endBufferRendering();
                GLuint nTexture = pBufferContext->getTextureId();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, nTexture);

                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA);

                GLfloat color[] = {1, 1, 1, (float)mfAlpha};

                if( mxClip.is() )
                {
                    const double fWidth=maSize.Width;
                        const double fHeight=maSize.Height;

                    // TODO(P3): buffer triangulation
                    const ::basegfx::B2DPolygon& rTriangulatedPolygon(
                        ::basegfx::triangulator::triangulate(
                            ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(mxClip)));

                    basegfx::B2DPolygon rTriangleList(
                        basegfx::tools::clipTriangleListOnRange(
                            rTriangulatedPolygon,
                            basegfx::B2DRange(
                                0,0,
                                aSpriteSizePixel.getX(),
                                aSpriteSizePixel.getY())));

                    GLfloat vertices[rTriangulatedPolygon.count()*2];
                    for( sal_uInt32 i=0; i<rTriangulatedPolygon.count(); i++ )
                    {
                        const ::basegfx::B2DPoint& rPt( rTriangulatedPolygon.getB2DPoint(i) );
                        vertices[i*2]= rPt.getX();
                        vertices[i*2+1]= rPt.getY();
                    }
                    mRenderHelper.renderVertexTex( vertices, fWidth, fHeight,  color, GL_TRIANGLES);
                }
                else
                {
                    const double fWidth=maSize.Width/aSpriteSizePixel.getX();
                    const double fHeight=maSize.Height/aSpriteSizePixel.getY();

                    GLfloat vertices[] = {0, 0,
                                          0, (float) aSpriteSizePixel.getY(),
                                          (float) aSpriteSizePixel.getX(), 0,
                                          (float) aSpriteSizePixel.getX(), (float) aSpriteSizePixel.getY() };
                    GLfloat uvCoordinates[] = {0, 0,
                                               0, (float) fHeight,
                                               (float) fWidth, 0,
                                               (float) fWidth, (float) fHeight };

                    mRenderHelper.renderVertexUVTex(vertices,  uvCoordinates, color, GL_TRIANGLE_STRIP );
                }

                glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_TEXTURE_2D);
            }
        }
        GLfloat vertices[] = {-2, -2,
                              -2, (float) maSize.Height+4,
                              (float) maSize.Width+4, (float) maSize.Height+4,
                              (float) maSize.Width+4, -2,
                              -2, -2,
                              (float) maSize.Width+4, (float) maSize.Height+4 };
        GLfloat color[] = {1, 0, 0, 1};

        mRenderHelper.renderVertexConstColor(vertices, color, GL_LINE_STRIP);

        std::vector<double> aVec;
        aVec.push_back(mfAlpha);
        aVec.push_back(mfPriority);
        aVec.push_back(maCanvasHelper.getRecordedActionCount());
        renderOSD( aVec, 10 );

        return true;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
