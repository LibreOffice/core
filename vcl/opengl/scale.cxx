/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <vcl/opengl/OpenGLHelper.hxx>

#include "vcl/bitmap.hxx"

#include "opengl/bmpop.hxx"
#include "opengl/salbmp.hxx"
#include "opengl/texture.hxx"

class ScaleOp : public OpenGLSalBitmapOp
{
private:
    OpenGLSalBitmap*    mpBitmap;
    double              mfScaleX;
    double              mfScaleY;
    sal_uInt32          mnScaleFlag;

public:
    ScaleOp( OpenGLSalBitmap* pBitmap, const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag );

    bool Execute() SAL_OVERRIDE;
    void GetSize( Size& rSize ) const SAL_OVERRIDE;
};


GLuint OpenGLSalBitmap::ImplGetTextureProgram()
{
    if( mnTexProgram == 0 )
    {
        mnTexProgram = OpenGLHelper::LoadShaders( "textureVertexShader",
                                                  "textureFragmentShader" );
        if( mnTexProgram == 0 )
            return 0;

        glBindAttribLocation( mnTexProgram, 0, "position" );
        glBindAttribLocation( mnTexProgram, 1, "tex_coord_in" );
        mnTexSamplerUniform = glGetUniformLocation( mnTexProgram, "sampler" );
    }

    return mnTexProgram;
}

GLuint OpenGLSalBitmap::ImplGetConvolutionProgram()
{
    if( mnConvProgram == 0 )
    {
        mnConvProgram = OpenGLHelper::LoadShaders( "textureVertexShader",
                                                   "convolutionFragmentShader" );
        if( mnConvProgram == 0 )
            return 0;

        glBindAttribLocation( mnConvProgram, 0, "position" );
        glBindAttribLocation( mnConvProgram, 1, "tex_coord_in" );
        mnConvSamplerUniform = glGetUniformLocation( mnConvProgram, "sampler" );
        mnConvKernelUniform = glGetUniformLocation( mnConvProgram, "kernel" );
        mnConvOffsetsUniform = glGetUniformLocation( mnConvProgram, "offsets" );
    }

    return mnConvProgram;
}

bool OpenGLSalBitmap::ImplScaleFilter( GLenum nFilter )
{
    OpenGLTexture* pNewTex;
    GLuint nProgram;
    GLuint nFramebufferId;
    GLenum nOldFilter;

    nProgram = ImplGetTextureProgram();
    if( nProgram == 0 )
        return false;

    glGenFramebuffers( 1, &nFramebufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, nFramebufferId );
    glUseProgram( nProgram );
    glUniform1i( mnTexSamplerUniform, 0 );

    pNewTex = new OpenGLTexture( mnWidth, mnHeight );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pNewTex->Id(), 0 );

    mpTexture->Bind();
    nOldFilter = mpTexture->GetFilter();
    mpTexture->SetFilter( nFilter );
    mpTexture->Draw();
    mpTexture->SetFilter( nOldFilter );
    mpTexture->Unbind();

    glUseProgram( 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDeleteFramebuffers( 1, &nFramebufferId );

    mpTexture.reset( pNewTex );
    return true;
}

void OpenGLSalBitmap::ImplCreateKernel(
    const double& fScale,
    const Kernel& rKernel,
    GLfloat*& pWeights,
    sal_uInt32& aKernelSize )
{
    const double fSamplingRadius(rKernel.GetWidth());
    const double fScaledRadius((fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius);
    const double fFilterFactor((fScale < 1.0) ? fScale : 1.0);
    int aNumberOfContributions;
    double aSum( 0 );

    aNumberOfContributions = (static_cast< sal_uInt32 >(fabs(ceil(fScaledRadius))) * 2) + 1 - 6;
    aKernelSize = aNumberOfContributions / 2 + 1;

    pWeights = new GLfloat[16];
    memset( pWeights, 0, 16 * sizeof( GLfloat ) );

    for( sal_uInt32 i(0); i < aKernelSize; i++ )
    {
        const GLfloat aWeight( rKernel.Calculate( fFilterFactor * i ) );
        if( fabs( aWeight ) >= 0.0001 )
        {
            pWeights[i] = aWeight;
            aSum += i > 0 ? aWeight * 2 : aWeight;
        }
    }

    for( sal_uInt32 i(0); i < aKernelSize; i++ )
    {
        pWeights[i] /= aSum;
    }
}

bool OpenGLSalBitmap::ImplScaleConvolution(
    const double& rScaleX,
    const double& rScaleY,
    const Kernel& aKernel )
{
    OpenGLTexture* pScratchTex;
    OpenGLTexture* pNewTex;
    GLfloat* pWeights( 0 );
    GLuint nFramebufferId;
    GLuint nProgram;
    sal_uInt32 nKernelSize;
    GLfloat aOffsets[32];
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    // TODO Make sure the framebuffer is alright

    nProgram = ImplGetConvolutionProgram();
    if( nProgram == 0 )
        return false;

    glGenFramebuffers( 1, &nFramebufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, nFramebufferId );
    glUseProgram( nProgram );
    glUniform1i( mnConvSamplerUniform, 0 );
    CHECK_GL_ERROR();

    // horizontal scaling in scratch texture
    pScratchTex = new OpenGLTexture( nNewWidth, mnHeight );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pScratchTex->Id(), 0 );
    CHECK_GL_ERROR();

    for( sal_uInt32 i = 0; i < 16; i++ )
    {
        aOffsets[i * 2] = i / (double) mnWidth;
        aOffsets[i * 2 + 1] = 0;
    }
    ImplCreateKernel( rScaleX, aKernel, pWeights, nKernelSize );
    glUniform1fv( mnConvKernelUniform, 16, pWeights );
    CHECK_GL_ERROR();
    glUniform2fv( mnConvOffsetsUniform, 16, aOffsets );
    CHECK_GL_ERROR();

    glViewport( 0, 0, nNewWidth, mnHeight );
    mpTexture->Bind();
    mpTexture->Draw();
    mpTexture->Unbind();

    // vertical scaling in final texture
    pNewTex = new OpenGLTexture( nNewWidth, nNewHeight );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pNewTex->Id(), 0 );

    for( sal_uInt32 i = 0; i < 16; i++ )
    {
        aOffsets[i * 2] = 0;
        aOffsets[i * 2 + 1] = i / (double) mnHeight;
    }
    ImplCreateKernel( rScaleY, aKernel, pWeights, nKernelSize );
    glUniform1fv( mnConvKernelUniform, 16, pWeights );
    glUniform2fv( mnConvOffsetsUniform, 16, aOffsets );
    CHECK_GL_ERROR();

    glViewport( 0, 0, nNewWidth, nNewHeight );
    pScratchTex->Bind();
    pScratchTex->Draw();
    pScratchTex->Unbind();

    glUseProgram( 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDeleteFramebuffers( 1, &nFramebufferId );

    delete pScratchTex;
    mpTexture.reset( pNewTex );
    mnWidth = nNewWidth;
    mnHeight = nNewHeight;

    return true;
}

bool OpenGLSalBitmap::ImplScale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
    SAL_INFO( "vcl.opengl", "::ImplScale" );

    if( nScaleFlag == BMP_SCALE_FAST )
    {
        return ImplScaleFilter( GL_NEAREST );
    }
    if( nScaleFlag == BMP_SCALE_BILINEAR )
    {
        return ImplScaleFilter( GL_LINEAR );
    }
    else if( nScaleFlag == BMP_SCALE_SUPER )
    {
        const Lanczos3Kernel aKernel;

        return ImplScaleConvolution( rScaleX, rScaleY, aKernel );
    }
    else if( nScaleFlag == BMP_SCALE_LANCZOS )
    {
        const Lanczos3Kernel aKernel;

        return ImplScaleConvolution( rScaleX, rScaleY, aKernel );
    }

    SAL_WARN( "vcl.opengl", "Invalid flag for scaling operation" );
    return false;
}

ScaleOp::ScaleOp(
    OpenGLSalBitmap* pBitmap,
    const double& rScaleX,
    const double& rScaleY,
    sal_uInt32 nScaleFlag )
: mpBitmap( pBitmap )
, mfScaleX( rScaleX )
, mfScaleY( rScaleY )
, mnScaleFlag( nScaleFlag )
{
}

bool ScaleOp::Execute()
{
    SAL_INFO( "vcl.opengl", "::Execute" );
    return mpBitmap->ImplScale( mfScaleX, mfScaleY, mnScaleFlag );
}

void ScaleOp::GetSize( Size& rSize ) const
{
    SAL_INFO( "vcl.opengl", "::GetSize" );
    rSize.setWidth( rSize.Width() * mfScaleX );
    rSize.setHeight( rSize.Height() * mfScaleY );
}

bool OpenGLSalBitmap::Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
    SAL_INFO( "vcl.opengl", "::Scale " << nScaleFlag );

    if( nScaleFlag == BMP_SCALE_FAST ||
        nScaleFlag == BMP_SCALE_BILINEAR ||
        nScaleFlag == BMP_SCALE_SUPER ||
        nScaleFlag == BMP_SCALE_LANCZOS )
    {
        //TODO maUserBuffer.reset();
        if( mpContext == NULL )
        {
            SAL_INFO( "vcl.opengl", "Add ScaleOp to pending operations" );
            maPendingOps.push_back( new ScaleOp( this, rScaleX, rScaleY, nScaleFlag ) );
        }
        else
        {
            ImplScale( rScaleX, rScaleY, nScaleFlag );
        }
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
