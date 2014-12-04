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
    }

    // Renders a Polygon, Texture has to be stored in TextureUnit0
    // Uses fWidth,fHeight to generate texture coordinates in vertex-shader.
    void RenderHelper::renderVertexTex(const std::vector<glm::vec2>& rVertices, GLfloat fWidth, GLfloat fHeight,
                                       const glm::vec4& vColor, GLenum mode) const
    {

        glUseProgram(m_texManProgID);

        //Set Uniforms
        glUniform1i(m_manTexUnf, 0);
        glUniform2f(m_manCordUnf,fWidth,fHeight);

        setupColorMVP(m_texManProgID, vColor);
        const GLint nVertices = setupAttrb(rVertices,m_vertexBuffer, m_texManProgID, "vPosition");

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
