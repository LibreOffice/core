#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtx/transform.hpp"
namespace oglcanvas
{
    class RenderHelper
    {
    public:

        void renderVertexConstColor(GLfloat vertices[] , GLfloat color[4], GLenum mode) const ;
        void renderVertexUVTex(GLfloat vertices[], GLfloat uvCoordinates[], GLfloat color[4], GLenum mode) const ;
        void renderVertexTex(GLfloat vertices[], GLfloat, GLfloat, GLfloat color[4], GLenum mode) const;

        RenderHelper();

        void SetVP(int width, int height);
        void SetModelAndMVP(glm::mat4 mat);
        void dispose();

    private:
        RenderHelper&  operator = (const RenderHelper& other);
        RenderHelper(const RenderHelper& other);

        void  InitOpenGL();

        GLuint                                            m_vertexBuffer;
        GLuint                                            m_uvBuffer;


        GLuint                                            m_simpleProgID;
        GLuint                                            m_simpleUvAttrb;
        GLuint                                            m_simplePosAttrb;
        GLuint                                            m_simpleTexUnf;
        GLuint                                            m_simpleColorUnf;
        GLuint                                            m_simpleMVPUnf;

        GLuint                                            m_manTexUnf;
        GLuint                                            m_manPosAttrb;
        GLuint                                            m_manCordUnf;
        GLuint                                            m_manColorUnf;
        GLuint                                            m_manMVPUnf;

        GLuint                                            m_texPosAttrb;
        GLuint                                            m_texColorUnf;
        GLuint                                            m_texManProgID;
        GLuint                                            m_texProgID;
        GLuint                                            m_texMVPUnf;
        //dimension
        int                                               m_iWidth;
        int                                               m_iHeight;

        // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4                                         m_Projection;
        // Camera matrix
        glm::mat4                                         m_View;
        // Model matrix : an identity matrix (model will be at the origin
        glm::mat4                                         m_Model;
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4                                         m_MVP;

    };
}