/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <epoxy/gl.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/verifyinput.hxx>
#include <tools/diagnose_ex.h>

#include "ogl_canvascustomsprite.hxx"
#include "ogl_canvastools.hxx"
#include "ogl_tools.hxx"

using namespace ::com::sun::star;

namespace oglcanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const css::geometry::RealSize2D& rSpriteSize,
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
        maCanvasHelper.init( *rRefDevice,
                             rDeviceHelper );
    }

    void CanvasCustomSprite::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteBaseT::disposeThis();
    }

    void SAL_CALL CanvasCustomSprite::setAlpha( double alpha )
    {
        canvas::tools::verifyRange( alpha, 0.0, 1.0 );

        ::osl::MutexGuard aGuard( m_aMutex );
        mfAlpha = alpha;
    }

    void SAL_CALL CanvasCustomSprite::move( const geometry::RealPoint2D&  aNewPos,
                                            const rendering::ViewState&   viewState,
                                            const rendering::RenderState& renderState )
    {
        canvas::tools::verifyArgs(aNewPos, viewState, renderState,
                                  OSL_THIS_FUNC,
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

    void SAL_CALL CanvasCustomSprite::transform( const geometry::AffineMatrix2D& aTransformation )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        maTransformation = aTransformation;
    }

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >& xClip )
    {
        mxClip = xClip;
    }

    void SAL_CALL CanvasCustomSprite::setPriority( double nPriority )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        mfPriority = nPriority;
    }

    void SAL_CALL CanvasCustomSprite::show()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if( mpSpriteCanvas.is() )
            mpSpriteCanvas->show(this);
    }

    void SAL_CALL CanvasCustomSprite::hide()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if( mpSpriteCanvas.is() )
            mpSpriteCanvas->hide(this);
    }

    uno::Reference< rendering::XCanvas > SAL_CALL CanvasCustomSprite::getContentCanvas()
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
                // drafts. Need to render to temp surface before, and then
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
                glBindTexture(GL_TEXTURE_2D, nTexture);

                glEnable(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA);

                // blend against fixed vertex color; texture alpha is multiplied in
                glColor4f(1,1,1,mfAlpha);

                if( mxClip.is() )
                {
                    const double fWidth=maSize.Width;
                    const double fHeight=maSize.Height;

                    // TODO(P3): buffer triangulation
                    const ::basegfx::triangulator::B2DTriangleVector rTriangulatedPolygon(
                        ::basegfx::triangulator::triangulate(
                            ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(mxClip)));

                    glBegin(GL_TRIANGLES);
                    for( size_t i=0; i<rTriangulatedPolygon.size(); i++ )
                    {
                        const::basegfx::triangulator::B2DTriangle& rCandidate(rTriangulatedPolygon[i]);
                        glTexCoord2f(
                            rCandidate.getA().getX()/fWidth,
                            rCandidate.getA().getY()/fHeight);
                        glVertex2d(
                            rCandidate.getA().getX(),
                            rCandidate.getA().getY());

                        glTexCoord2f(
                            rCandidate.getB().getX()/fWidth,
                            rCandidate.getB().getY()/fHeight);
                        glVertex2d(
                            rCandidate.getB().getX(),
                            rCandidate.getB().getY());

                        glTexCoord2f(
                            rCandidate.getC().getX()/fWidth,
                            rCandidate.getC().getY()/fHeight);
                        glVertex2d(
                            rCandidate.getC().getX(),
                            rCandidate.getC().getY());
                     }
                    glEnd();
                }
                else
                {
                    const double fWidth=maSize.Width/aSpriteSizePixel.getX();
                    const double fHeight=maSize.Height/aSpriteSizePixel.getY();

                    glBegin(GL_TRIANGLE_STRIP);
                    glTexCoord2f(0,0);            glVertex2d(0,0);
                    glTexCoord2f(0,fHeight);      glVertex2d(0, aSpriteSizePixel.getY());
                    glTexCoord2f(fWidth,0);       glVertex2d(aSpriteSizePixel.getX(),0);
                    glTexCoord2f(fWidth,fHeight); glVertex2d(aSpriteSizePixel.getX(),aSpriteSizePixel.getY());
                    glEnd();
                }

                glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_TEXTURE_2D);
            }
        }

        glColor4f(1,0,0,1);
        glBegin(GL_LINE_STRIP);
        glVertex2d(-2,-2);
        glVertex2d(-2,maSize.Height+4);
        glVertex2d(maSize.Width+4,maSize.Height+4);
        glVertex2d(maSize.Width+4,-2);
        glVertex2d(-2,-2);
        glVertex2d(maSize.Width+4,maSize.Height+4);
        glEnd();

        std::vector<double> aVec;
        aVec.push_back(mfAlpha);
        aVec.push_back(mfPriority);
        aVec.push_back(maCanvasHelper.getRecordedActionCount());
        renderOSD( aVec, 10 );

        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
