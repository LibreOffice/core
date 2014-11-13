#include <GL/glew.h>
#include "ogl_renderHelper.hxx"
#include <vcl/opengl/OpenGLHelper.hxx>
namespace oglcanvas
{
    RenderHelper::RenderHelper()
    : m_iWidth(1600) //Why this dimensions?
    , m_iHeight(900) //Whole window, see spritedevicehelper
    , m_Model(glm::mat4(1.0f))
    {
    }
    void RenderHelper::InitOpenGL()
    {
        GLenum err = glewInit();
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

        //glViewport(0, 0, m_iWidth, m_iHeight);
    }
    //Todo figgure out, which parameters i should use :)
    void RenderHelper::SetVP(int width, int height)
    {
        m_Projection = glm::ortho(0.f, float(m_iWidth), 0.f, float(m_iHeight), -4.f, 3.f);
        m_Projection = m_Projection * glm::scale(glm::vec3((float)width / m_iWidth, -(float)height / m_iHeight, 1.0f));

        m_View       = glm::lookAt(glm::vec3(0,m_iHeight,1),
                                   glm::vec3(0,m_iHeight,0),
                                   glm::vec3(0,1,0) );
    }
    void RenderHelper::SetModelAndMVP(glm::mat4 mat)
    {
        m_Model = mat;
        m_MVP = m_Projection * m_View * m_Model;
    }
    void RenderHelper::renderVertexConstColor(GLfloat vertices[], glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glUseProgram(m_texProgID);

        glUniform4fv(m_texColorUnf, 1, glm::value_ptr(color));
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
    void RenderHelper::renderVertexUVTex(GLfloat vertices[], GLfloat uvCoordinates[], glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoordinates), uvCoordinates, GL_STATIC_DRAW);

        glUseProgram(m_simpleProgID);

        glUniform1i(m_simpleTexUnf, 0); //Use texture Unit 0
        glUniform4fv(m_simpleColorUnf, 1, glm::value_ptr(color));
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
    void RenderHelper::renderVertexTex(GLfloat vertices[], GLfloat fWidth, GLfloat fHeight, glm::vec4 color, GLenum mode) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glUseProgram(m_texManProgID);

        //Set Uniforms
        glUniform1i(m_manTexUnf, 0);
        glUniform2f(m_manCordUnf,fWidth,fHeight);
        glUniform4fv(m_manColorUnf, 1,  glm::value_ptr(color));
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

        glDrawArrays(mode, 0, sizeof(vertices) / sizeof(vertices[0]) /2);

        glDisableVertexAttribArray(m_manPosAttrb);
        glUseProgram(0);
    }
}