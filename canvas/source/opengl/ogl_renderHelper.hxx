/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RENDER_HELPER
#define RENDER_HELPER
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtx/transform.hpp"
#include <vector>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

namespace oglcanvas
{
    class RenderHelper
    {
    public:
        //renders vertices with a const color
        void renderVertexConstColor(const std::vector<glm::vec2>& rVertices, const glm::vec4& vColor, GLenum mode) const;
        //renders a texture bound in texture units 0, with the given texture coordinates
        void renderVertexUVTex(const std::vector<glm::vec2>& rVertices, const std::vector<glm::vec2>& rUVcoords,
                               const glm::vec4& vColor, GLenum mode) const;
        //renders a texture (texture unit 0) with texture coordinates at (vertice coord /(fWidth, fHeight))
        void renderVertexTex(const std::vector<glm::vec2>& rVertices, const GLfloat fWidth, const GLfloat gHeight,
                             const glm::vec4& vColor, GLenum mode) const;
        //see renderVertexTex description, just with an additional texture transformation matrix
        void renderTextureTransform(const std::vector<glm::vec2>& rVertices, GLfloat fWidth,
                                    GLfloat fHeight, const glm::vec4& color, GLenum mode, const glm::mat4& transform) const;
        //Gradients
        void renderLinearGradient(  const std::vector<glm::vec2>& rVertices,
                                    const GLfloat fWidth, const GLfloat fHeight,
                                    const GLenum mode,
                                    const ::com::sun::star::rendering::ARGBColor*    pColors,
                                    const ::com::sun::star::uno::Sequence< double >& rStops,
                                    const glm::mat3x2&                                 rTexTransform) const;

        void renderRadialGradient(  const std::vector<glm::vec2>& rVertices,
                                    const GLfloat fWidth, const GLfloat fHeight,
                                    const GLenum mode,
                                    const ::com::sun::star::rendering::ARGBColor*  pColors,
                                    const ::com::sun::star::uno::Sequence< double >& rStops,
                                    const glm::mat3x2&                                 rTexTransform) const;

        void renderRectangularGradient(  const std::vector<glm::vec2>& rVertices,
                                    const GLfloat fWidth, const GLfloat fHeight,
                                    const GLenum mode,
                                    const ::com::sun::star::rendering::ARGBColor*  pColors,
                                    const ::com::sun::star::uno::Sequence< double >& rStops,
                                    const glm::mat3x2&                               rTexTransform) const;
        RenderHelper();

        void SetVP(const float width, const float height);

        void SetVP(const glm::mat4 vp);

        glm::mat4 GetVP();

        void SetModelAndMVP(const glm::mat4& mat);
        void dispose();
        void  InitOpenGL();


    private:
        RenderHelper&  operator = (const RenderHelper& other);
        RenderHelper(const RenderHelper& other);
        void setupColorMVP(const unsigned int nProgramID, const glm::vec4& color) const;

        void setupGradientTransformation( unsigned int            nProgramId,
                                         const glm::mat3x2&                   rTexTransform,
                                          GLfloat fWidth, GLfloat fHeight) const;

        GLuint                                            m_vertexBuffer;
        GLuint                                            m_uvBuffer;

        GLuint                                            m_simpleProgID;
        GLuint                                            m_simpleTexUnf;

        GLuint                                            m_manTexUnf;
        GLuint                                            m_manCordUnf;

        GLuint                                            m_texManProgID;
        GLuint                                            m_texProgID;

        GLuint                                            m_texTransProgID;
        GLuint                                            m_transCordUnf;
        GLuint                                            m_transTexUnf;
        GLuint                                            m_transTexTransform;

        glm::mat4                                         m_VP;
        // Model matrix
        glm::mat4                                         m_Model;
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4                                         m_MVP;

        GLuint                                            mnLinearTwoColorGradientProgram;
        GLuint                                            mnLinearMultiColorGradientProgram;
        GLuint                                            mnRadialMultiColorGradientProgram;
        GLuint                                            mnRadialTwoColorGradientProgram;
        GLuint                                            mnRectangularMultiColorGradientProgram;
        GLuint                                            mnRectangularTwoColorGradientProgram;


    };
}

#endif /* RENDER_HELPER */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
