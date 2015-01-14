/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>
#include "ogl_renderHelper.hxx"
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/GLMHelper.hxx>

namespace oglcanvas
{
    RenderHelper::RenderHelper():
        m_Model(glm::mat4(1.0f))
    {

    }
    void RenderHelper::InitOpenGL()
    {
        //Load Shaders //
        m_texManProgID = OpenGLHelper::LoadShaders("textManipulatingVertexShader", "textFragmentShader");
        m_simpleProgID = OpenGLHelper::LoadShaders("simpleVertexShader", "textFragmentShader");
        m_texProgID = OpenGLHelper::LoadShaders("texVertexShader", "constantFragmentShader");
        m_texTransProgID = OpenGLHelper::LoadShaders("textManipulatingVertexShader", "transformationFragmentShader");
        mnLinearMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "linearMultiColorGradientFragmentShader");

        mnLinearTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "linearTwoColorGradientFragmentShader");
        mnRadialMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "radialMultiColorGradientFragmentShader");
        mnRadialTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "radialTwoColorGradientFragmentShader");
        mnRectangularMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "rectangularMultiColorGradientFragmentShader");
        mnRectangularTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("textManipulatingVertexShader", "rectangularTwoColorGradientFragmentShader");

        // Get a handle for uniforms
        m_manTexUnf = glGetUniformLocation(m_texManProgID, "TextTex");
        m_simpleTexUnf = glGetUniformLocation(m_simpleProgID, "TextTex");
        m_transTexUnf = glGetUniformLocation(m_texTransProgID, "TextTex");

        m_manCordUnf = glGetUniformLocation(m_texManProgID, "texCord");
        m_transCordUnf = glGetUniformLocation(m_texTransProgID, "texCord");

        m_transTexTransform = glGetUniformLocation(m_texTransProgID,"texTrans");

        //Gen Buffers for texturecoordinates/vertices
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_uvBuffer);
    }

    void RenderHelper::setupColorMVP(const unsigned int nProgramId, const glm::vec4& vColor) const
    {

        const GLint nMVPLocation = glGetUniformLocation(nProgramId,"MVP");
        glUniformMatrix4fv(nMVPLocation, 1, GL_FALSE, &m_MVP[0][0]);

        const GLint ncolorTexLocation = glGetUniformLocation(nProgramId,"colorTex");
        glUniform4fv(ncolorTexLocation, 1, &vColor[0]);
    }

    static GLint setupAttrb(const std::vector<glm::vec2>& vattrb_data, const unsigned int buffer, const unsigned int nProgramId, const char sattribName[])
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, vattrb_data.size()*sizeof(float)*2, &vattrb_data [0].x, GL_STATIC_DRAW);

        const GLint nAttrbLocation  = glGetAttribLocation(nProgramId, sattribName);

        glEnableVertexAttribArray(nAttrbLocation); //vertices
        glVertexAttribPointer(
            nAttrbLocation,
            2,                            // size
            GL_FLOAT,                     // type
            GL_FALSE,                     // normalized?
            0,                            // stride
            (void*)0                      // array buffer offset
        );
        return nAttrbLocation;
    }

    void RenderHelper::SetVP(const float nwidth, const float nheight)
    {
        m_VP = glm::ortho(0.0f, nwidth, nheight, 0.0f);

    void RenderHelper::SetVP(const glm::mat4 vp)
    {
        m_VP = vp;
    }

    glm::mat4 RenderHelper::GetVP()
    {
        return m_VP;
    }

    static void cleanUp()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    void RenderHelper::SetModelAndMVP(const glm::mat4& mat)
    {
        m_Model = mat;
        m_MVP = m_VP*m_Model ;
    }

    void RenderHelper::renderVertexConstColor(const std::vector<glm::vec2>& rVertices, const glm::vec4& vColor,
                                              GLenum mode) const
    {
        glUseProgram(m_texProgID);

        setupColorMVP(m_texProgID, vColor);
        const GLint nVertices = setupAttrb(rVertices, m_vertexBuffer, m_texProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());
        glDisableVertexAttribArray(nVertices);
        cleanUp();
    }
    //Renders a TriangleStrip, Texture has to be stored in TextureUnit0
    void RenderHelper::renderVertexUVTex(const std::vector<glm::vec2>& rVertices, const std::vector<glm::vec2>& rUVcoords,
                                         const glm::vec4& vColor, GLenum mode) const
    {

        glUseProgram(m_simpleProgID);

        glUniform1i(m_simpleTexUnf, 0); //Use texture Unit 0

        setupColorMVP(m_simpleProgID, vColor);
        const GLint nUVAttrb = setupAttrb(rUVcoords,m_uvBuffer, m_simpleProgID, "UV");
        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_simpleProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(nVertices);
        glDisableVertexAttribArray(nUVAttrb);

        cleanUp();
    }

    //Clean up
    void RenderHelper::dispose()
    {
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_uvBuffer);
        glDeleteProgram( m_texManProgID);
        glDeleteProgram( m_simpleProgID);
        glDeleteProgram( m_texProgID);
        glDeleteProgram( m_texTransProgID);
        glDeleteProgram( mnRectangularTwoColorGradientProgram );
        glDeleteProgram( mnRectangularMultiColorGradientProgram );
        glDeleteProgram( mnRadialTwoColorGradientProgram );
        glDeleteProgram( mnRadialMultiColorGradientProgram );
        glDeleteProgram( mnLinearTwoColorGradientProgram );
        glDeleteProgram( mnLinearMultiColorGradientProgram );
    }

    void RenderHelper::renderVertexTex(const std::vector<glm::vec2>& rVertices, const GLfloat fWidth, const GLfloat fHeight,
                                       const glm::vec4& vColor, GLenum mode) const
    {

        glUseProgram(m_texManProgID);

        //Set Uniforms
        glUniform1i(m_manTexUnf, 0);
        glUniform2f(m_manCordUnf,fWidth,fHeight);

        setupColorMVP(m_texManProgID, vColor);
        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_texManProgID, "vPosition");
        glDisableVertexAttribArray(nVertices);
        glDrawArrays(mode, 0, rVertices.size());

        cleanUp();
    }

    // Uses fWidth,fHeight to generate texture coordinates in vertex-shader.
    void RenderHelper::renderTextureTransform(const std::vector<glm::vec2>& rVertices, GLfloat fWidth, GLfloat fHeight,
                                              const glm::vec4& vColor, GLenum mode, const glm::mat4& transform) const
    {

        glUseProgram(m_texTransProgID);

        //Set Uniforms
        glUniform1i(m_transTexUnf, 0);
        glUniform2f(m_transCordUnf,fWidth,fHeight);

        glUniformMatrix4fv(m_transTexTransform, 1, GL_FALSE, &transform[0][0]);

        setupColorMVP(m_texTransProgID, vColor);
        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_texTransProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(nVertices);
        cleanUp();
    }

    void RenderHelper::setupGradientTransformation( unsigned int                          nProgramId,
                                      const glm::mat3x2&                                  rTexTransform,
                                      GLfloat fWidth, GLfloat fHeight) const
    {
        const GLint nTransformLocation = glGetUniformLocation(nProgramId,
                                                             "m_transform" );
        glUniformMatrix3x2fv(nTransformLocation,1,false,&rTexTransform[0][0]);

        const GLint nMVPLocation = glGetUniformLocation(nProgramId,"MVP");
        glUniformMatrix4fv(nMVPLocation, 1, GL_FALSE, &m_MVP[0][0]);

        const GLint nTextTexLocation = glGetUniformLocation(mnLinearTwoColorGradientProgram, "texCord");
        glUniform2f(nTextTexLocation, fWidth, fHeight);
    }


    static void setupGradientUniform( unsigned int                   nProgramId,
                               const ::com::sun::star::rendering::ARGBColor*    pColors,
                               const ::com::sun::star::uno::Sequence< double >& rStops)
    {
        glUseProgram(nProgramId);

        GLuint nColorsTexture;
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &nColorsTexture);
        glBindTexture(GL_TEXTURE_1D, nColorsTexture);

        const sal_Int32 nColors=rStops.getLength();
        glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, nColors, 0, GL_RGBA, GL_DOUBLE, pColors );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        GLuint nStopsTexture;
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &nStopsTexture);
        glBindTexture(GL_TEXTURE_1D, nStopsTexture);

        glTexImage1D( GL_TEXTURE_1D, 0, GL_ALPHA, nColors, 0, GL_ALPHA, GL_DOUBLE, rStops.getConstArray() );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        const GLint nColorArrayLocation = glGetUniformLocation(nProgramId,
                                                               "t_colorArray4d" );
        glUniform1i( nColorArrayLocation, 0 ); // unit 0

        const GLint nStopArrayLocation = glGetUniformLocation(nProgramId,
                                                              "t_stopArray1d" );
        glUniform1i( nStopArrayLocation, 1 ); // unit 1

        const GLint nNumColorLocation = glGetUniformLocation(nProgramId,
                                                             "i_nColors" );
        glUniform1i( nNumColorLocation, nColors-1 );
    }

    static void setupGradientUniform( unsigned int                   nProgramId,
                               const ::com::sun::star::rendering::ARGBColor&    rStartColor,
                               const ::com::sun::star::rendering::ARGBColor&    rEndColor)
    {
        glUseProgram(nProgramId);

        const GLint nStartColorLocation = glGetUniformLocation(nProgramId,
                                                               "v_startColor4d" );
        glUniform4f(nStartColorLocation,
                    rStartColor.Red,
                    rStartColor.Green,
                    rStartColor.Blue,
                    rStartColor.Alpha);

        const GLint nEndColorLocation = glGetUniformLocation(nProgramId,
                                                             "v_endColor4d" );
        glUniform4f(nEndColorLocation,
                    rEndColor.Red,
                    rEndColor.Green,
                    rEndColor.Blue,
                    rEndColor.Alpha);
    }

    void RenderHelper::renderLinearGradient(const std::vector<glm::vec2>& rVertices,
                                            const GLfloat fWidth, const GLfloat fHeight,
                                            const GLenum mode,
                                            const ::com::sun::star::rendering::ARGBColor*  pColors,
                                            const ::com::sun::star::uno::Sequence< double >& rStops,
                                            const glm::mat3x2&                                 rTexTransform) const
    {
        if( rStops.getLength() > 2 )
        {
            setupGradientUniform(mnLinearMultiColorGradientProgram, pColors, rStops);
            setupGradientTransformation(mnLinearMultiColorGradientProgram, rTexTransform, fWidth, fHeight);
        }
        else
        {
            setupGradientUniform(mnLinearTwoColorGradientProgram, pColors[0], pColors[1]);
            setupGradientTransformation(mnLinearTwoColorGradientProgram, rTexTransform, fWidth, fHeight);
        }
        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_simpleProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(nVertices);
        cleanUp();
    }

    void RenderHelper::renderRadialGradient(const std::vector<glm::vec2>& rVertices,
                                            const GLfloat fWidth, const GLfloat fHeight,
                                            const GLenum mode,
                                            const ::com::sun::star::rendering::ARGBColor*  pColors,
                                            const ::com::sun::star::uno::Sequence< double >& rStops,
                                            const glm::mat3x2&                               rTexTransform) const
    {
        if( rStops.getLength() > 2 )
        {
            setupGradientUniform(mnRadialMultiColorGradientProgram, pColors, rStops);
            setupGradientTransformation(mnRadialMultiColorGradientProgram, rTexTransform, fWidth, fHeight);
        }
        else
        {
            setupGradientUniform(mnRadialTwoColorGradientProgram, pColors[0], pColors[1]);
            setupGradientTransformation(mnRadialTwoColorGradientProgram , rTexTransform, fWidth, fHeight);
        }

        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_simpleProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(nVertices);
        cleanUp();
    }

    void RenderHelper::renderRectangularGradient(const std::vector<glm::vec2>& rVertices,
                                            const GLfloat fWidth, const GLfloat fHeight,
                                            const GLenum mode,
                                            const ::com::sun::star::rendering::ARGBColor*  pColors,
                                            const ::com::sun::star::uno::Sequence< double >& rStops,
                                            const glm::mat3x2&                               rTexTransform) const
    {
        if( rStops.getLength() > 2 )
        {
            setupGradientUniform(mnRectangularMultiColorGradientProgram, pColors, rStops);
            setupGradientTransformation(mnRectangularMultiColorGradientProgram, rTexTransform, fWidth, fHeight);
        }
        else
        {
            setupGradientUniform(mnRectangularTwoColorGradientProgram, pColors[0], pColors[1]);
            setupGradientTransformation(mnRectangularTwoColorGradientProgram , rTexTransform, fWidth, fHeight);
        }

        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_simpleProgID, "vPosition");

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(nVertices);
        cleanUp();
    }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
