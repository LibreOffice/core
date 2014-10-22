#include <GL/glew.h>
#include <vcl/opengl/OpenGLHelper.hxx>
namespace oglcanvas
{
    class RenderHelper
    {
    public:
        RenderHelper();
        void renderVertexConstColor(GLfloat vertices[] , GLfloat color[4], GLenum mode) const ;
        void renderVertexUVTex(GLfloat vertices[], GLfloat uvCoordinates[], GLfloat color[4], GLenum mode) const ;
        void renderVertexTex(GLfloat vertices[], GLfloat, GLfloat, GLfloat color[4], GLenum mode) const;
        void dispose();
    private:
        void  InitOpenGL();
        GLuint                                            m_vertexBuffer;
        GLuint                                            m_uvBuffer;


        GLuint                                            m_simpleProgID;
        GLuint                                            m_simpleUvAttrb;
        GLuint                                            m_simplePosAttrb;
        GLuint                                            m_simpleTexUnf;
        GLuint                                            m_simpleColorUnf;

        GLuint                                            m_manTexUnf;
        GLuint                                            m_manPosAttrb;
        GLuint                                            m_manCordUnf;
        GLuint                                            m_manColorUnf;

        GLuint                                            m_texPosAttrb;
        GLuint                                            m_texColorUnf;
        GLuint                                            m_texManProgID;
        GLuint                                            m_texProgID;
    };
}