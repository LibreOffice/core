#include <GL/glew.h>
#include "ogl_renderHelper.hxx"
#include <vcl/opengl/OpenGLHelper.hxx>
namespace oglcanvas
{
    RenderHelper::RenderHelper()
    {
        InitOpenGL();
    }
    void RenderHelper::InitOpenGL()
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
        m_manColorUnf = glGetUniformLocation(m_texManProgID,"colorTex");
        m_simpleColorUnf = glGetUniformLocation(m_simpleProgID,"colorTex");
        //Gen Buffers for texturecoordinates/vertices
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_uvBuffer);
        m_manPosAttrb = glGetAttribLocation(m_texManProgID ,"vPosition");
        m_simpleUvAttrb = glGetAttribLocation(m_simpleProgID ,"UV");
        m_simplePosAttrb = glGetAttribLocation(m_simpleProgID ,"vPosition");
        m_texPosAttrb = glGetAttribLocation(m_texProgID ,"vPosition");
    }

    void RenderHelper::renderVertexConstColor(GLfloat vertices[], GLfloat color[4], GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glUseProgram(m_texProgID);

        glUniform4f(m_texColorUnf,color[0], color[1], color[2], color[3]);
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

        glDrawArrays(mode, 0, sizeof(vertices) / sizeof(vertices[0]) /2);

        glDisableVertexAttribArray(m_texPosAttrb);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    //Renders a TriangleStrip, Texture has to be stored in TextureUnit0
    void RenderHelper::renderVertexUVTex(GLfloat vertices[], GLfloat uvCoordinates[], GLfloat color[4], GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoordinates), uvCoordinates, GL_STATIC_DRAW);

        glUseProgram(m_simpleProgID);

        glUniform1i(m_simpleTexUnf, 0); //Use texture Unit 0
        glUniform4f(m_simpleColorUnf, color[0], color[1], color[2], color[3]);

        glEnableVertexAttribArray(m_simplePosAttrb);
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

        glDrawArrays(mode, 0, sizeof(vertices) / sizeof(vertices[0]) /2);

        glDisableVertexAttribArray(m_simplePosAttrb);
        glDisableVertexAttribArray(m_simpleUvAttrb);
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
    void RenderHelper::renderVertexTex(GLfloat vertices[], GLfloat fWidth, GLfloat fHeight, GLfloat color[4], GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glUseProgram(m_texManProgID);

        //Set Uniforms
        glUniform1i(m_manTexUnf, 0);
        glUniform2f(m_manCordUnf,fWidth,fHeight);
        glUniform4f(m_manColorUnf, color[0], color[1], color[2], color[3] );

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

        glDrawArrays(mode, 0, sizeof(vertices) / sizeof(vertices[0]) /2);

        glDisableVertexAttribArray(m_manPosAttrb);
        glUseProgram(0);
    }
}