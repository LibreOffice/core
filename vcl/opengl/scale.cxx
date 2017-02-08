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

#include <vcl/bitmap.hxx>

#include "opengl/zone.hxx"
#include "opengl/salbmp.hxx"
#include "opengl/program.hxx"
#include "opengl/texture.hxx"
#include "opengl/RenderState.hxx"

#include <ResampleKernel.hxx>

using vcl::Kernel;
using vcl::Lanczos3Kernel;

bool OpenGLSalBitmap::ImplScaleFilter(
    const rtl::Reference< OpenGLContext > &xContext,
    const double& rScaleX,
    const double& rScaleY,
    GLenum        nFilter )
{
    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;
    GLenum nOldFilter;
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    pProgram = xContext->UseProgram( "textureVertexShader",
                                     "textureFragmentShader" );
    if( !pProgram )
        return false;

    OpenGLTexture aNewTex(nNewWidth, nNewHeight);
    pFramebuffer = xContext->AcquireFramebuffer( aNewTex );

    pProgram->SetTexture( "sampler", maTexture );
    nOldFilter = maTexture.GetFilter();
    maTexture.SetFilter( nFilter );
    pProgram->ApplyMatrix(mnWidth, mnHeight);
    pProgram->DrawTexture( maTexture );
    maTexture.SetFilter( nOldFilter );
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer( pFramebuffer );

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
    const rtl::Reference< OpenGLContext > &xContext,
    const double& rScaleX,
    const double& rScaleY,
    const Kernel& aKernel )
{
    OpenGLFramebuffer* pFramebuffer;
    OpenGLProgram* pProgram;
    GLfloat* pWeights( nullptr );
    sal_uInt32 nKernelSize;
    GLfloat aOffsets[32];
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    // TODO Make sure the framebuffer is alright

    pProgram = xContext->UseProgram( "textureVertexShader",
                                      "convolutionFragmentShader" );
    if( pProgram == nullptr )
        return false;

    // horizontal scaling in scratch texture
    if( mnWidth != nNewWidth )
    {
        OpenGLTexture aScratchTex(nNewWidth, nNewHeight);

        pFramebuffer = xContext->AcquireFramebuffer( aScratchTex );

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
        OpenGLContext::ReleaseFramebuffer( pFramebuffer );
    }

    // vertical scaling in final texture
    if( mnHeight != nNewHeight )
    {
        OpenGLTexture aScratchTex(nNewWidth, nNewHeight);

        pFramebuffer = xContext->AcquireFramebuffer( aScratchTex );

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
        OpenGLContext::ReleaseFramebuffer( pFramebuffer );
    }

    mnWidth = nNewWidth;
    mnHeight = nNewHeight;

    CHECK_GL_ERROR();
    return true;
}

/*
 "Area" scaling algorithm, which seems to give better results for downscaling
 than other algorithms. The principle (taken from opencv, see resize.cl)
 is that each resulting pixel is the average of all the source pixel values
 it represents. Which is trivial in the case of exact multiples for downscaling,
 the generic case needs to also consider that some source pixels contribute
 only partially to their resulting pixels (because of non-integer multiples).
*/
bool OpenGLSalBitmap::ImplScaleArea( const rtl::Reference< OpenGLContext > &xContext,
                                     double rScaleX, double rScaleY )
{
    int nNewWidth( mnWidth * rScaleX );
    int nNewHeight( mnHeight * rScaleY );

    if( nNewWidth == mnWidth && nNewHeight == mnHeight )
        return true;

    double ixscale = 1 / rScaleX;
    double iyscale = 1 / rScaleY;
    bool fast = ( ixscale == int( ixscale ) && iyscale == int( iyscale )
        && int( nNewWidth * ixscale ) == mnWidth && int( nNewHeight * iyscale ) == mnHeight );

    bool bTwoPasses = false;

    // The generic case has arrays only up to 100 ratio downscaling, which is hopefully enough
    // in practice, but protect against buffer overflows in case such an extreme case happens
    // (and in such case the precision of the generic algorithm probably doesn't matter anyway).
    if( ixscale > 100 || iyscale > 100 )
    {
        fast = true;
    }
    else
    {
        if (ixscale > 16 || iyscale > 16)
        {
            ixscale = std::floor(std::sqrt(ixscale));
            iyscale = std::floor(std::sqrt(iyscale));
            nNewWidth = int(mnWidth / ixscale);
            rScaleX *= ixscale; // second pass x-scale factor
            nNewHeight = int(mnHeight / iyscale);
            rScaleY *= iyscale; // second pass y-scale factor
            bTwoPasses = true;
        }
    }

    // TODO Make sure the framebuffer is alright

    OString sUseReducedRegisterVariantDefine;
    if (xContext->getOpenGLCapabilitySwitch().mbLimitedShaderRegisters)
        sUseReducedRegisterVariantDefine = OString("#define USE_REDUCED_REGISTER_VARIANT\n");

    OpenGLProgram* pProgram = xContext->UseProgram( "textureVertexShader",
        fast ? OUString( "areaScaleFastFragmentShader" ) : OUString( "areaScaleFragmentShader" ),
        sUseReducedRegisterVariantDefine);

    if( pProgram == nullptr )
        return false;

    OpenGLTexture aScratchTex(nNewWidth, nNewHeight);

    OpenGLFramebuffer* pFramebuffer = xContext->AcquireFramebuffer( aScratchTex );

    // NOTE: This setup is also done in OpenGLSalGraphicsImpl::DrawTransformedTexture().
    if( fast )
    {
        pProgram->SetUniform1i( "xscale", ixscale );
        pProgram->SetUniform1i( "yscale", iyscale );
        pProgram->SetUniform1f( "xstep", 1.0 / mnWidth );
        pProgram->SetUniform1f( "ystep", 1.0 / mnHeight );
        pProgram->SetUniform1f( "ratio", 1.0 / ( ixscale * iyscale ));
    }
    else
    {
        pProgram->SetUniform1f( "xscale", ixscale );
        pProgram->SetUniform1f( "yscale", iyscale );
        pProgram->SetUniform1i( "swidth", mnWidth );
        pProgram->SetUniform1i( "sheight", mnHeight );
        // For converting between <0,mnWidth-1> and <0.0,1.0> coordinate systems.
        pProgram->SetUniform1f( "xsrcconvert", 1.0 / ( mnWidth - 1 ));
        pProgram->SetUniform1f( "ysrcconvert", 1.0 / ( mnHeight - 1 ));
        pProgram->SetUniform1f( "xdestconvert", 1.0 * ( nNewWidth - 1 ));
        pProgram->SetUniform1f( "ydestconvert", 1.0 * ( nNewHeight - 1 ));
    }

    pProgram->SetTexture( "sampler", maTexture );
    pProgram->DrawTexture( maTexture );
    pProgram->Clean();

    OpenGLContext::ReleaseFramebuffer(pFramebuffer);

    CHECK_GL_ERROR();

    if (bTwoPasses)
    {
        mnWidth = nNewWidth;
        mnHeight = nNewHeight;

        nNewWidth = int(mnWidth * rScaleX);
        nNewHeight = int (mnHeight * rScaleY);

        ixscale = 1 / rScaleX;
        iyscale = 1 / rScaleY;

        pProgram = xContext->UseProgram("textureVertexShader", "areaScaleFragmentShader", sUseReducedRegisterVariantDefine);
        if (pProgram == nullptr)
            return false;

        OpenGLTexture aScratchTex2(nNewWidth, nNewHeight);

        pFramebuffer = xContext->AcquireFramebuffer(aScratchTex2);

        pProgram->SetUniform1f("xscale", ixscale);
        pProgram->SetUniform1f("yscale", iyscale);
        pProgram->SetUniform1i("swidth", mnWidth);
        pProgram->SetUniform1i("sheight", mnHeight);
        // For converting between <0,mnWidth-1> and <0.0,1.0> coordinate systems.
        pProgram->SetUniform1f("xsrcconvert", 1.0 / (mnWidth - 1));
        pProgram->SetUniform1f("ysrcconvert", 1.0 / (mnHeight - 1));
        pProgram->SetUniform1f("xdestconvert", 1.0 * (nNewWidth - 1));
        pProgram->SetUniform1f("ydestconvert", 1.0 * (nNewHeight - 1));

        pProgram->SetTexture("sampler", aScratchTex);
        pProgram->DrawTexture(aScratchTex);
        pProgram->Clean();

        OpenGLContext::ReleaseFramebuffer(pFramebuffer);

        CHECK_GL_ERROR();

        maTexture = aScratchTex2;
        mnWidth = nNewWidth;
        mnHeight = nNewHeight;
    }
    else
    {
        maTexture = aScratchTex;
        mnWidth = nNewWidth;
        mnHeight = nNewHeight;
    }

    return true;
}

bool OpenGLSalBitmap::ImplScale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    VCL_GL_INFO( "::ImplScale" );

    mpUserBuffer.reset();
    OpenGLVCLContextZone aContextZone;
    rtl::Reference<OpenGLContext> xContext = OpenGLContext::getVCLContext();
    xContext->state().scissor().disable();
    xContext->state().stencil().disable();

    if (rScaleX <= 1 && rScaleY <= 1)
    {
        nScaleFlag = BmpScaleFlag::BestQuality;
    }

    if( nScaleFlag == BmpScaleFlag::Fast )
    {
        return ImplScaleFilter( xContext, rScaleX, rScaleY, GL_NEAREST );
    }
    if( nScaleFlag == BmpScaleFlag::BiLinear )
    {
        return ImplScaleFilter( xContext, rScaleX, rScaleY, GL_LINEAR );
    }
    else if( nScaleFlag == BmpScaleFlag::Default )
    {
        const Lanczos3Kernel aKernel;

        return ImplScaleConvolution( xContext, rScaleX, rScaleY, aKernel );
    }
    else if( nScaleFlag == BmpScaleFlag::BestQuality && rScaleX <= 1 && rScaleY <= 1 )
    { // Use are scaling for best quality, but only if downscaling.
        return ImplScaleArea( xContext, rScaleX, rScaleY );
    }
    else if( nScaleFlag == BmpScaleFlag::Lanczos || nScaleFlag == BmpScaleFlag::BestQuality  )
    {
        const Lanczos3Kernel aKernel;

        return ImplScaleConvolution( xContext, rScaleX, rScaleY, aKernel );
    }

    SAL_WARN( "vcl.opengl", "Invalid flag for scaling operation" );
    return false;
}

bool OpenGLSalBitmap::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    OpenGLVCLContextZone aContextZone;

    VCL_GL_INFO("::Scale " << int(nScaleFlag)
             << " from " << mnWidth << "x" << mnHeight
             << " to " << (mnWidth * rScaleX) << "x" << (mnHeight * rScaleY) );

    if( nScaleFlag == BmpScaleFlag::Fast ||
        nScaleFlag == BmpScaleFlag::BiLinear ||
        nScaleFlag == BmpScaleFlag::Lanczos ||
        nScaleFlag == BmpScaleFlag::Default ||
        nScaleFlag == BmpScaleFlag::BestQuality )
    {
        ImplScale( rScaleX, rScaleY, nScaleFlag );
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
