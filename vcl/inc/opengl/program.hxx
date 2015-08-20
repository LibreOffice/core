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
typedef std::list< OpenGLTexture > TextureList;

class VCL_PLUGIN_PUBLIC OpenGLProgram
{
private:
    GLuint          mnId;
    UniformCache    maUniformLocations;
    sal_uInt32      mnEnabledAttribs;
    GLuint          mnPositionAttrib;
    GLuint          mnTexCoordAttrib;
    GLuint          mnAlphaCoordAttrib;
    TextureList     maTextures;
    bool            mbBlending;

    float mfLastWidth;
    float mfLastHeight;
    float mfLastPixelOffset;


    OpenGLProgram(const OpenGLProgram &) SAL_DELETED_FUNCTION;
public:
    OpenGLProgram();
    ~OpenGLProgram();

    bool Load( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    bool Use();
    bool Clean();

    void SetVertices( const GLvoid* pData );
    void SetTextureCoord( const GLvoid* pData );
    void SetAlphaCoord( const GLvoid* pData );

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
    void SetBlendMode( GLenum nSFactor, GLenum nDFactor );

    void ApplyMatrix(float fWidth, float fHeight, float fPixelOffset = 0.0f);

    bool DrawTexture( OpenGLTexture& rTexture );

protected:
    void SetVertexAttrib( GLuint& rAttrib, const OString& rName, const GLvoid* pData );
    GLuint GetUniformLocation( const OString& rName );
};

#endif // INCLUDED_VCL_INC_OPENGL_PROGRAM_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
