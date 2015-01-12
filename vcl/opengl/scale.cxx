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
#include "opengl/program.hxx"
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

bool OpenGLSalBitmap::ImplScaleFilter(
    const double& rScaleX,
    const double& rScaleY,
    GLenum        nFilter )
{
    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;
    GLenum nOldFilter;
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    pProgram = mpContext->UseProgram( "textureVertexShader",
                                      "textureFragmentShader" );
    if( !pProgram )
        return false;

    OpenGLTexture aNewTex = OpenGLTexture( nNewWidth, nNewHeight );
    pFramebuffer = mpContext->AcquireFramebuffer( aNewTex );

    pProgram->SetTexture( "sampler", maTexture );
    nOldFilter = maTexture.GetFilter();
    maTexture.SetFilter( nFilter );
    pProgram->DrawTexture( maTexture );
    maTexture.SetFilter( nOldFilter );
    pProgram->Clean();

    mpContext->ReleaseFramebuffer( pFramebuffer );

    mnWidth = nNewWidth;
    mnHeight = nNewHeight;
    maTexture = aNewTex;

    CHECK_GL_ERROR();
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

    // avoid a crash for now; re-think me.
    if (aKernelSize > 16)
        aKernelSize = 16;

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
    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;
    GLfloat* pWeights( 0 );
    sal_uInt32 nKernelSize;
    GLfloat aOffsets[32];
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    // TODO Make sure the framebuffer is alright

    pProgram = mpContext->UseProgram( "textureVertexShader",
                                      "convolutionFragmentShader" );
    if( pProgram == 0 )
        return false;

    // horizontal scaling in scratch texture
    if( mnWidth != nNewWidth )
    {
        OpenGLTexture aScratchTex = OpenGLTexture( nNewWidth, mnHeight );
        pFramebuffer = mpContext->AcquireFramebuffer( aScratchTex );

        for( sal_uInt32 i = 0; i < 16; i++ )
        {
            aOffsets[i * 2] = i / (double) mnWidth;
            aOffsets[i * 2 + 1] = 0;
        }
        ImplCreateKernel( rScaleX, aKernel, pWeights, nKernelSize );
        pProgram->SetUniform1fv( "kernel", 16, pWeights );
        pProgram->SetUniform2fv( "offsets", 16, aOffsets );
        pProgram->SetTexture( "sampler", maTexture );
        pProgram->DrawTexture( maTexture );
        pProgram->Clean();

        maTexture = aScratchTex;
        mpContext->ReleaseFramebuffer( pFramebuffer );
    }

    // vertical scaling in final texture
    if( mnHeight != nNewHeight )
    {
        OpenGLTexture aScratchTex = OpenGLTexture( nNewWidth, nNewHeight );
        pFramebuffer = mpContext->AcquireFramebuffer( aScratchTex );

        for( sal_uInt32 i = 0; i < 16; i++ )
        {
            aOffsets[i * 2] = 0;
            aOffsets[i * 2 + 1] = i / (double) mnHeight;
        }
        ImplCreateKernel( rScaleY, aKernel, pWeights, nKernelSize );
        pProgram->SetUniform1fv( "kernel", 16, pWeights );
        pProgram->SetUniform2fv( "offsets", 16, aOffsets );
        pProgram->SetTexture( "sampler", maTexture );
        pProgram->DrawTexture( maTexture );
        pProgram->Clean();

        maTexture = aScratchTex;
        mpContext->ReleaseFramebuffer( pFramebuffer );
    }

    mnWidth = nNewWidth;
    mnHeight = nNewHeight;

    CHECK_GL_ERROR();
    return true;
}

bool OpenGLSalBitmap::ImplScale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
    SAL_INFO( "vcl.opengl", "::ImplScale" );

    maUserBuffer.reset();
    makeCurrent();

    if( nScaleFlag == BMP_SCALE_FAST )
    {
        return ImplScaleFilter( rScaleX, rScaleY, GL_NEAREST );
    }
    if( nScaleFlag == BMP_SCALE_BILINEAR )
    {
        return ImplScaleFilter( rScaleX, rScaleY, GL_LINEAR );
    }
    else if( nScaleFlag == BMP_SCALE_SUPER || nScaleFlag == BMP_SCALE_DEFAULT )
    {
        const Lanczos3Kernel aKernel;

        return ImplScaleConvolution( rScaleX, rScaleY, aKernel );
    }
    else if( nScaleFlag == BMP_SCALE_LANCZOS || nScaleFlag == BMP_SCALE_BESTQUALITY  )
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
        nScaleFlag == BMP_SCALE_LANCZOS ||
        nScaleFlag == BMP_SCALE_DEFAULT ||
        nScaleFlag == BMP_SCALE_BESTQUALITY )
    {
        makeCurrent();
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
