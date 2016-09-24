/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_PROGRAM_H
#define INCLUDED_VCL_INC_OPENGL_PROGRAM_H

#include <sal/config.h>

#include <list>

#include <GL/glew.h>
#include <vcl/dllapi.h>

#include <basegfx/point/b2dpoint.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <opengl/texture.hxx>

#include <unordered_map>

typedef std::unordered_map< OString, GLuint, OStringHash > UniformCache;

enum class TextureShaderType
{
    Normal = 0,
    Blend,
    Masked,
    Diff,
    MaskedColor
};

enum class DrawShaderType
{
    Normal = 0,
    Line
};

class VCL_PLUGIN_PUBLIC OpenGLProgram
{
private:
    GLuint          mnId;
    UniformCache    maUniformLocations;
    sal_uInt32      mnEnabledAttribs;
    GLuint          mnPositionAttrib;
    GLuint          mnTexCoordAttrib;
    GLuint          mnAlphaCoordAttrib;
    GLuint          mnMaskCoordAttrib;
    GLuint          mnExtrusionVectorsAttrib;
    GLuint          mnVertexColorsAttrib;

    std::list< OpenGLTexture >     maTextures;
    bool            mbBlending;

    float mfLastWidth;
    float mfLastHeight;
    float mfLastPixelOffset;


    OpenGLProgram(const OpenGLProgram &) = delete;
public:
    OpenGLProgram();
    ~OpenGLProgram();

    GLuint Id() { return mnId; }

    bool Load( const OUString& rVertexShader, const OUString& rFragmentShader,
               const rtl::OString& preamble, const rtl::OString& rDigest );
    bool Use();
    void Reuse();
    bool Clean();

    void SetVertices( const GLvoid* pData );
    void SetTextureCoord( const GLvoid* pData );
    void SetAlphaCoord( const GLvoid* pData );
    void SetMaskCoord(const GLvoid* pData);
    void SetExtrusionVectors(const GLvoid* pData);
    void SetVertexColors(std::vector<GLubyte>& rColorVector);

    void SetUniform1f( const OString& rName, GLfloat v1 );
    void SetUniform2f( const OString& rName, GLfloat v1, GLfloat v2 );
    void SetUniform1fv( const OString& rName, GLsizei nCount, GLfloat* aValues );
    void SetUniform2fv( const OString& rName, GLsizei nCount, GLfloat* aValues );
    void SetUniform1i( const OString& rName, GLint v1 );
    void SetColor( const OString& rName, const Color& rColor );
    void SetColor( const OString& rName, SalColor nColor, sal_uInt8 nTransparency );
    void SetColorf( const OString& rName, SalColor nColor, double fTransparency );
    void SetColorWithIntensity( const OString& rName, const Color& rColor, long nFactor );
    void SetTexture( const OString& rName, OpenGLTexture& rTexture );
    void SetTransform( const OString& rName, const OpenGLTexture& rTexture,
                       const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                       const basegfx::B2DPoint& rY );
    void SetIdentityTransform(const OString& rName);
    void SetShaderType(TextureShaderType eTextureShaderType);
    void SetShaderType(DrawShaderType eDrawShaderType);

    void SetBlendMode( GLenum nSFactor, GLenum nDFactor );

    void ApplyMatrix(float fWidth, float fHeight, float fPixelOffset = 0.0f);

    bool DrawTexture( const OpenGLTexture& rTexture );

    void DrawArrays(GLenum aMode, std::vector<GLfloat>& aVertices);
    void DrawElements(GLenum aMode, GLuint nNumberOfVertices);

    bool EnableVertexAttrib(GLuint& rAttrib, const OString& rName);

    void SetVertexAttrib(GLuint& rAttrib, const OString& rName, GLint nSize,
                         GLenum eType, GLboolean bNormalized, GLsizei aStride,
                         const GLvoid* pPointer);

protected:
    GLuint GetUniformLocation( const OString& rName );
};

#endif // INCLUDED_VCL_INC_OPENGL_PROGRAM_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
