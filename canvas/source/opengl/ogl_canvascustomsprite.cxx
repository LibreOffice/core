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
#include <vcl/opengl/OpenGLHelper.hxx>
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
        InitOpenGL();
    }

    void CanvasCustomSprite::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_uvBuffer);
        glDeleteProgram( m_texManProgID);
        glDeleteProgram( m_simpleProgID);
        glDeleteProgram( m_texProgID);
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

    void CanvasCustomSprite::InitOpenGL()
    {
        //Load Shaders //
        m_texManProgID = OpenGLHelper::LoadShaders("textManipulatingVertexShader", "textFragmentShader");
        m_simpleProgID = OpenGLHelper::LoadShaders("simpleVertexShader", "textFragmentShader");
        m_texProgID = OpenGLHelper::LoadShaders("texVertrexShader", "constantFragmentShader");
        // Get a handle for uniforms
        m_manTexUnf = glGetUniformLocation(m_texManProgID, "TextTex");
        m_simpleTexUnf = glGetUniformLocation(m_simpleProgID, "TextTex");
        m_manCordUnf = glGetUniformLocation(m_texManProgID, "texCord");
        m_texColorUnf = glGetUniformLocation(m_texProgID, "constantColor");
        //Gen Buffers for texturecoordinates/vertices
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_uvBuffer);
        m_manPosAttrb = glGetAttribLocation(m_texManProgID ,"vPosition");
        m_simpleUvAttrb = glGetAttribLocation(m_simpleProgID ,"UV");
        m_simplePosAttrb = glGetAttribLocation(m_simpleProgID ,"vPosition");
        m_texPosAttrb = glGetAttribLocation(m_texProgID ,"vPosition");
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

                // blend against fixed vertex color; texture alpha is multiplied in
                glColor4f(1,1,1,mfAlpha);

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

                    //Bind Buffers
                    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                    glUseProgram(m_texManProgID);
                    //Set Uniforms
                    glUniform1i(m_manTexUnf, 0);
                    glUniform2f(m_manCordUnf,fWidth,fHeight);
                    glEnableVertexAttribArray(m_manPosAttrb);
                    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
                    glVertexAttribPointer(
                        m_manPosAttrb,
                        2,                            // size
                        GL_FLOAT,                     // type
                        GL_FALSE,                     // normalized?
                        0,                            // stride
                        (void*)0                      // array buffer offset
                    );
                    glDrawArrays(GL_TRIANGLES, 0, rTriangulatedPolygon.count());
                    glDisableVertexAttribArray(m_manPosAttrb);
                    glUseProgram(0);
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
                    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoordinates), uvCoordinates, GL_STATIC_DRAW);

                    glUseProgram(m_simpleProgID);
                    glUniform1i(m_simpleTexUnf, 0);

                    glEnableVertexAttribArray(m_simplePosAttrb); //richtige ID herausfinden
                    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
                    glVertexAttribPointer(
                        m_simplePosAttrb,
                        2,                            // size
                        GL_FLOAT,                     // type
                        GL_FALSE,                     // normalized?
                        0,                            // stride
                        (void*)0                      // array buffer offset
                    );

                    glEnableVertexAttribArray(m_simpleUvAttrb);
                    glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
                    glVertexAttribPointer(
                        m_simpleUvAttrb,
                        2,                            // size
                        GL_FLOAT,                     // type
                        GL_FALSE,                     // normalized?
                        0,                            // stride
                        (void*)0                      // array buffer offset
                    );
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glDisableVertexAttribArray(m_simplePosAttrb);
                    glDisableVertexAttribArray(m_simpleUvAttrb);
                    glUseProgram(0);
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
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glUseProgram(m_texProgID);
        glUniform4f(m_texColorUnf, 1, 0, 0, 1);
        glEnableVertexAttribArray(m_texPosAttrb); //richtige ID herausfinden
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glVertexAttribPointer(
                        m_texPosAttrb,
                        2,                            // size
                        GL_FLOAT,                     // type
                        GL_FALSE,                     // normalized?
                        0,                            // stride
                        (void*)0                      // array buffer offset
        );
        glDrawArrays(GL_LINE_STRIP, 0, 6);
        glDisableVertexAttribArray(m_texPosAttrb);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        std::vector<double> aVec;
        aVec.push_back(mfAlpha);
        aVec.push_back(mfPriority);
        aVec.push_back(maCanvasHelper.getRecordedActionCount());
        renderOSD( aVec, 10 );

        return true;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
