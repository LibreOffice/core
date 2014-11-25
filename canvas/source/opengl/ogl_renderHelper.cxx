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
        // Get a handle for uniforms
        m_manTexUnf = glGetUniformLocation(m_texManProgID, "TextTex");
        m_simpleTexUnf = glGetUniformLocation(m_simpleProgID, "TextTex");

        m_manCordUnf = glGetUniformLocation(m_texManProgID, "texCord");
        m_texColorUnf = glGetUniformLocation(m_texProgID, "constantColor");

        m_manColorUnf = glGetUniformLocation(m_texManProgID,"colorTex");
        m_simpleColorUnf = glGetUniformLocation(m_simpleProgID,"colorTex");

        m_texMVPUnf = glGetUniformLocation(m_texProgID, "MVP");
        m_manMVPUnf = glGetUniformLocation(m_texManProgID, "MVP");
        m_simpleMVPUnf = glGetUniformLocation(m_simpleProgID, "MVP");
        //Gen Buffers for texturecoordinates/vertices
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_uvBuffer);
        m_manPosAttrb = glGetAttribLocation(m_texManProgID ,"vPosition");
        m_simpleUvAttrb = glGetAttribLocation(m_simpleProgID ,"UV");
        m_simplePosAttrb = glGetAttribLocation(m_simpleProgID ,"vPosition");
        m_texPosAttrb = glGetAttribLocation(m_texProgID ,"vPosition");

    }

    void RenderHelper::SetVP(const float width, const float height)
    {
        m_VP = glm::ortho(0.0f, width, 0.0f, height);

    }

    void RenderHelper::SetModelAndMVP(const glm::mat4& mat)
    {
        m_Model = mat;
        m_MVP = m_Model * m_VP;
    }

    void RenderHelper::renderVertexConstColor(const std::vector<glm::vec2>& rVertices, glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, rVertices.size()*2, &rVertices[0].x, GL_STATIC_DRAW);

        glUseProgram(m_texProgID);

        glUniform4fv(m_texColorUnf, 1, &color[0]);
        glUniformMatrix4fv(m_texMVPUnf, 1, GL_FALSE, &m_MVP[0][0]);
        glEnableVertexAttribArray(m_texPosAttrb); //vertices

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glVertexAttribPointer(
                        m_texPosAttrb,
                        2,                            // size
                        GL_FLOAT,                     // type
                        GL_FALSE,                     // normalized?
                        0,                            // stride
                        (void*)0                      // array buffer offset
        );

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(m_texPosAttrb);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

    }
    //Renders a TriangleStrip, Texture has to be stored in TextureUnit0
    void RenderHelper::renderVertexUVTex(const std::vector<glm::vec2>& rVertices, const std::vector<glm::vec2>& rUVcoords, glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, rVertices.size()*2, &rVertices[0].x, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, rUVcoords.size()*2, &rUVcoords[0].x, GL_STATIC_DRAW);

        glUseProgram(m_simpleProgID);

        glUniform1i(m_simpleTexUnf, 0); //Use texture Unit 0

        glUniform4fv(m_simpleColorUnf, 1, &color[0]);
        glUniformMatrix4fv(m_simpleMVPUnf, 1, GL_FALSE, &m_MVP[0][0]);

        glEnableVertexAttribArray(m_simplePosAttrb);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glVertexAttribPointer(
            m_simplePosAttrb,
            2,                            // size
            GL_FLOAT,                     // type
            GL_FALSE,                     // normalized?v
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

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(m_simplePosAttrb);
        glDisableVertexAttribArray(m_simpleUvAttrb);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
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
    void RenderHelper::renderVertexTex(const std::vector<glm::vec2>& rVertices, GLfloat fWidth, GLfloat fHeight, glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, rVertices.size()*2, &rVertices[0].x, GL_STATIC_DRAW);

        glUseProgram(m_texManProgID);

        //Set Uniforms
        glUniform1i(m_manTexUnf, 0);
        glUniform2f(m_manCordUnf,fWidth,fHeight);
        glUniform4fv(m_manColorUnf, 1, &color[0]);
        glUniformMatrix4fv(m_manMVPUnf, 1, GL_FALSE, &m_MVP[0][0]);

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

        glDrawArrays(mode, 0, rVertices.size());

        glDisableVertexAttribArray(m_manPosAttrb);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
