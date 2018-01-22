/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <opengl/program.hxx>
#include <opengl/RenderState.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

OpenGLProgram::OpenGLProgram() :
    mnId( 0 ),
    mnEnabledAttribs( 0 ),
    mnPositionAttrib( SAL_MAX_UINT32 ),
    mnTexCoordAttrib( SAL_MAX_UINT32 ),
    mnAlphaCoordAttrib( SAL_MAX_UINT32 ),
    mnMaskCoordAttrib( SAL_MAX_UINT32 ),
    mnExtrusionVectorsAttrib( SAL_MAX_UINT32 ),
    mnVertexColorsAttrib( SAL_MAX_UINT32 ),
    mbBlending(false),
    mfLastWidth(0.0),
    mfLastHeight(0.0),
    mfLastPixelOffset(0.0)
{
}

OpenGLProgram::~OpenGLProgram()
{
    maUniformLocations.clear();
    if( mnId != 0 )
    {
        glDeleteProgram( mnId );
        CHECK_GL_ERROR();
    }
}

bool OpenGLProgram::Load( const OUString& rVertexShader,
                          const OUString& rFragmentShader,
                          const rtl::OString& preamble,
                          const rtl::OString& rDigest )
{
    mnId = OpenGLHelper::LoadShaders( rVertexShader, rFragmentShader, preamble, rDigest );
    return ( mnId != 0 );
}

void OpenGLProgram::Reuse()
{
    mbBlending = false;
}

void OpenGLProgram::Use()
{
    if (!mnId)
        return;

    glUseProgram(mnId);
    CHECK_GL_ERROR();
    Reuse();
}

bool OpenGLProgram::Clean()
{
    // unbind all textures
    for (OpenGLTexture& rTexture : maTextures)
    {
        rTexture.Unbind();
    }
    maTextures.clear();

    // disable any enabled vertex attrib array
    if( mnEnabledAttribs )
    {
        for( int i = 0; i < 32; i++ )
        {
            if( mnEnabledAttribs & ( 1 << i ) )
            {
                glDisableVertexAttribArray( i );
                CHECK_GL_ERROR();
            }
        }
        mnEnabledAttribs = 0;
    }

    return true;
}

bool OpenGLProgram::EnableVertexAttrib(GLuint& rAttrib, const OString& rName)
{
    if( rAttrib == SAL_MAX_UINT32 )
    {
        GLint aLocation = glGetAttribLocation(mnId, rName.getStr());
        CHECK_GL_ERROR();
        if (aLocation < 0)
            return false;
        rAttrib = GLuint(aLocation);
    }
    if( (mnEnabledAttribs & ( 1 << rAttrib )) == 0 )
    {
        glEnableVertexAttribArray( rAttrib );
        CHECK_GL_ERROR();
        mnEnabledAttribs |= ( 1 << rAttrib );
    }
    return true;
}

void OpenGLProgram::SetVertexAttrib(GLuint& rAttrib, const OString& rName, GLint nSize,
                                    GLenum eType, GLboolean bNormalized, GLsizei aStride,
                                    const GLvoid* pPointer)
{
    if (EnableVertexAttrib(rAttrib, rName))
    {
        glVertexAttribPointer(rAttrib, nSize, eType, bNormalized, aStride, pPointer);
        CHECK_GL_ERROR();
    }
    else
    {
        VCL_GL_INFO("Vertex attribute '" << rName << "' doesn't exist in this program (" << mnId << ")");
    }
}

void OpenGLProgram::SetVertices( const GLvoid* pData )
{
    SetVertexAttrib(mnPositionAttrib, "position", 2, GL_FLOAT, GL_FALSE, 0, pData);
}

void OpenGLProgram::SetTextureCoord( const GLvoid* pData )
{
    SetVertexAttrib(mnTexCoordAttrib, "tex_coord_in", 2, GL_FLOAT, GL_FALSE, 0, pData);
}

void OpenGLProgram::SetAlphaCoord( const GLvoid* pData )
{
    SetVertexAttrib(mnAlphaCoordAttrib, "alpha_coord_in", 2, GL_FLOAT, GL_FALSE, 0, pData);
}

void OpenGLProgram::SetMaskCoord(const GLvoid* pData)
{
    SetVertexAttrib(mnMaskCoordAttrib, "mask_coord_in", 2, GL_FLOAT, GL_FALSE, 0, pData);
}

void OpenGLProgram::SetExtrusionVectors(const GLvoid* pData)
{
    SetVertexAttrib(mnExtrusionVectorsAttrib, "extrusion_vectors", 3, GL_FLOAT, GL_FALSE, 0, pData);
}

void OpenGLProgram::SetVertexColors(std::vector<GLubyte>& rColorVector)
{
    SetVertexAttrib(mnVertexColorsAttrib, "vertex_color_in", 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, rColorVector.data());
}

void OpenGLProgram::SetShaderType(TextureShaderType eTextureShaderType)
{
    SetUniform1i("type", GLint(eTextureShaderType));
}

void OpenGLProgram::SetShaderType(DrawShaderType eDrawShaderType)
{
    SetUniform1i("type", GLint(eDrawShaderType));
}

GLuint OpenGLProgram::GetUniformLocation( const OString& rName )
{
    auto it = maUniformLocations.find( rName );
    if( it == maUniformLocations.end() )
    {
        GLuint nLocation = glGetUniformLocation( mnId, rName.getStr() );
        CHECK_GL_ERROR();
        maUniformLocations[rName] = nLocation;
        return nLocation;
    }

    return it->second;
}

void OpenGLProgram::DrawArrays(GLenum aMode, std::vector<GLfloat>& aVertices)
{
    if (!mbBlending)
        OpenGLContext::getVCLContext()->state().blend().disable();

    SetVertices(aVertices.data());
    glDrawArrays(aMode, 0, aVertices.size() / 2);
}

void OpenGLProgram::DrawElements(GLenum aMode, GLuint nNumberOfVertices)
{
    if (!mbBlending)
        OpenGLContext::getVCLContext()->state().blend().disable();

    glDrawElements(aMode, nNumberOfVertices, GL_UNSIGNED_INT, nullptr);
}

void OpenGLProgram::SetUniform1f( const OString& rName, GLfloat v1 )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform1f( nUniform, v1 );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetUniform2f( const OString& rName, GLfloat v1, GLfloat v2 )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform2f( nUniform, v1, v2 );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetUniform1fv( const OString& rName, GLsizei nCount, GLfloat const * aValues )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform1fv( nUniform, nCount, aValues );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetUniform2fv( const OString& rName, GLsizei nCount, GLfloat const * aValues )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform2fv( nUniform, nCount, aValues );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetUniform1i( const OString& rName, GLint v1 )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform1i( nUniform, v1 );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetColor( const OString& rName, SalColor nColor, sal_uInt8 nTransparency )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform4f( nUniform,
                 static_cast<float>(SALCOLOR_RED( nColor )) / 255,
                 static_cast<float>(SALCOLOR_GREEN( nColor )) / 255,
                 static_cast<float>(SALCOLOR_BLUE( nColor )) / 255,
                 (100 - nTransparency) * (1.0 / 100) );
    CHECK_GL_ERROR();

    if( nTransparency > 0 )
        SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void OpenGLProgram::SetColorf( const OString& rName, SalColor nColor, double fTransparency )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform4f( nUniform,
                 static_cast<float>(SALCOLOR_RED( nColor )) / 255,
                 static_cast<float>(SALCOLOR_GREEN( nColor )) / 255,
                 static_cast<float>(SALCOLOR_BLUE( nColor )) / 255,
                 (1.0f - fTransparency) );
    CHECK_GL_ERROR();

    if( fTransparency > 0.0 )
        SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void OpenGLProgram::SetColor( const OString& rName, const Color& rColor )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform4f( nUniform,
                 static_cast<float>(rColor.GetRed()) / 255,
                 static_cast<float>(rColor.GetGreen()) / 255,
                 static_cast<float>(rColor.GetBlue()) / 255,
                 1.0f - static_cast<float>(rColor.GetTransparency()) / 255 );
    CHECK_GL_ERROR();

    if( rColor.GetTransparency() > 0 )
        SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void OpenGLProgram::SetColorWithIntensity( const OString& rName, const Color& rColor, long nFactor )
{
    GLuint nUniform = GetUniformLocation( rName );
    glUniform4f( nUniform,
                 static_cast<float>(rColor.GetRed()) * nFactor / 25500.0,
                 static_cast<float>(rColor.GetGreen()) * nFactor / 25500.0,
                 static_cast<float>(rColor.GetBlue()) * nFactor / 25500.0,
                 1.0f );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetTexture( const OString& rName, OpenGLTexture& rTexture )
{
    GLuint nUniform = GetUniformLocation( rName );
    int nIndex = maTextures.size();

    glUniform1i( nUniform, nIndex );
    CHECK_GL_ERROR();

    OpenGLContext::getVCLContext()->state().texture().active(nIndex);

    rTexture.Bind();
    maTextures.push_back(rTexture);
}

void OpenGLProgram::SetTransform(
    const OString& rName,
    const OpenGLTexture& rTexture,
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY )
{
    auto nTexWidth = rTexture.GetWidth();
    auto nTexHeight = rTexture.GetHeight();
    if (nTexWidth == 0 || nTexHeight == 0)
        return;

    GLuint nUniform = GetUniformLocation( rName );
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    const float aValues[] = {
        static_cast<float>(aXRel.getX())/nTexWidth,  static_cast<float>(aXRel.getY())/nTexWidth,  0, 0,
        static_cast<float>(aYRel.getX())/nTexHeight, static_cast<float>(aYRel.getY())/nTexHeight, 0, 0,
        0,                               0,                               1, 0,
        static_cast<float>(rNull.getX()),            static_cast<float>(rNull.getY()),            0, 1 };
    glm::mat4 aMatrix = glm::make_mat4( aValues );
    glUniformMatrix4fv( nUniform, 1, GL_FALSE, glm::value_ptr( aMatrix ) );
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetIdentityTransform(const OString& rName)
{
    GLuint nUniform = GetUniformLocation(rName);
    glm::mat4 aMatrix = glm::mat4();
    glUniformMatrix4fv(nUniform, 1, GL_FALSE, glm::value_ptr( aMatrix ) );
    CHECK_GL_ERROR();
}

void OpenGLProgram::ApplyMatrix(float fWidth, float fHeight, float fPixelOffset)
{

    if (mfLastWidth == fWidth && mfLastHeight == fHeight && mfLastPixelOffset == fPixelOffset)
        return;

    mfLastWidth = fWidth;
    mfLastHeight = fHeight;
    mfLastPixelOffset = fPixelOffset;

    GLuint nUniform = GetUniformLocation("mvp");

    glm::mat4 aMVP = glm::ortho(0.0f, fWidth, fHeight, 0.0f, 0.0f, 1.0f);

    if (fPixelOffset != 0.0f)
        aMVP = glm::translate(aMVP, glm::vec3(fPixelOffset, fPixelOffset, 0.0f));

    glUniformMatrix4fv(nUniform, 1, GL_FALSE, glm::value_ptr(aMVP));
    CHECK_GL_ERROR();
}

void OpenGLProgram::SetBlendMode(GLenum nSFactor, GLenum nDFactor)
{
    OpenGLContext::getVCLContext()->state().blend().enable();
    OpenGLContext::getVCLContext()->state().blend().func(nSFactor, nDFactor);
    mbBlending = true;
}

void OpenGLProgram::DrawTexture( const OpenGLTexture& rTexture )
{
    if (!rTexture)
        return;

    float fWidth = rTexture.GetWidth();
    float fHeight = rTexture.GetHeight();

    float fMinX = 0.0f;
    float fMaxX = fWidth;
    float fMinY = 0.0f;
    float fMaxY = fHeight;

    std::vector<GLfloat> aPosition {
        fMinX, fMaxY,
        fMinX, fMinY,
        fMaxX, fMinY,
        fMaxX, fMaxY
    };
    GLfloat aTexCoord[8];

    rTexture.GetWholeCoord( aTexCoord );
    SetTextureCoord( aTexCoord );
    ApplyMatrix(fWidth, fHeight);
    DrawArrays(GL_TRIANGLE_FAN, aPosition);
    CHECK_GL_ERROR();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
