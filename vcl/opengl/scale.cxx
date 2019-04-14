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
#include <sal/log.hxx>

#include <cmath>

#include <vcl/opengl/OpenGLHelper.hxx>

#include <vcl/bitmap.hxx>

#include <opengl/zone.hxx>
#include <opengl/salbmp.hxx>
#include <opengl/program.hxx>
#include <opengl/texture.hxx>
#include <opengl/RenderState.hxx>

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
    const double fFilterFactor(std::min(fScale, 1.0));
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
            aOffsets[i * 2] = i / static_cast<double>(mnWidth);
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
            aOffsets[i * 2 + 1] = i / static_cast<double>(mnHeight);
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
    bool fast = ( ixscale == std::trunc( ixscale ) && iyscale == std::trunc( iyscale )
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
        // The shader operates on pixels in the surrounding area, so it's necessary
        // to know the step in texture coordinates to get to the next pixel.
        // With a texture atlas the "texture" is just a subtexture of a larger texture,
        // so while with a normal texture we'd map between <0.0,1.0> and <0,mnWidth>,
        // with a subtexture the texture coordinates range is smaller.
        GLfloat srcCoords[ 8 ];
        maTexture.GetWholeCoord( srcCoords );
        pProgram->SetUniform1f( "xstep", ( srcCoords[ 4 ] - srcCoords[ 0 ] ) / mnWidth );
        pProgram->SetUniform1f( "ystep", ( srcCoords[ 5 ] - srcCoords[ 1 ] ) / mnHeight );
        pProgram->SetUniform1f( "ratio", 1.0 / ( ixscale * iyscale ));
    }
    else
    {
        pProgram->SetUniform1f( "xscale", ixscale );
        pProgram->SetUniform1f( "yscale", iyscale );
        pProgram->SetUniform1i( "swidth", mnWidth );
        pProgram->SetUniform1i( "sheight", mnHeight );
        // The shader internally actually operates on pixel coordinates,
        // so it needs to know how to convert to those from the texture coordinates.
        // With a simple texture that would mean converting e.g. between
        // <0,mnWidth-1> and <0.0,1.0> coordinates.
        // However with a texture atlas the "texture" is just a subtexture
        // of a larger texture, so the texture coordinates need offset and ratio
        // conversion too.
        GLfloat srcCoords[ 8 ];
        maTexture.GetWholeCoord( srcCoords );
        pProgram->SetUniform1f( "xoffset", srcCoords[ 0 ] );
        pProgram->SetUniform1f( "yoffset", srcCoords[ 1 ] );
        pProgram->SetUniform1f( "xtopixelratio", nNewWidth / ( srcCoords[ 4 ] - srcCoords[ 0 ] ));
        pProgram->SetUniform1f( "ytopixelratio", nNewHeight / ( srcCoords[ 5 ] - srcCoords[ 1 ] ));
        pProgram->SetUniform1f( "xfrompixelratio", ( srcCoords[ 4 ] - srcCoords[ 0 ] ) / mnWidth );
        pProgram->SetUniform1f( "yfrompixelratio", ( srcCoords[ 5 ] - srcCoords[ 1 ] ) / mnHeight );
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

        nNewWidth = round(mnWidth * rScaleX);
        nNewHeight = round(mnHeight * rScaleY);

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

        GLfloat srcCoords[ 8 ];
        aScratchTex.GetWholeCoord( srcCoords );
        pProgram->SetUniform1f( "xoffset", srcCoords[ 0 ] );
        pProgram->SetUniform1f( "yoffset", srcCoords[ 1 ] );
        pProgram->SetUniform1f( "xtopixelratio", nNewWidth / ( srcCoords[ 4 ] - srcCoords[ 0 ] ));
        pProgram->SetUniform1f( "ytopixelratio", nNewHeight / ( srcCoords[ 5 ] - srcCoords[ 1 ] ));
        pProgram->SetUniform1f( "xfrompixelratio", ( srcCoords[ 4 ] - srcCoords[ 0 ] ) / mnWidth );
        pProgram->SetUniform1f( "yfrompixelratio", ( srcCoords[ 5 ] - srcCoords[ 1 ] ) / mnHeight );

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

void OpenGLSalBitmap::ImplScale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
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
        ImplScaleFilter( xContext, rScaleX, rScaleY, GL_NEAREST );
    }
    else if( nScaleFlag == BmpScaleFlag::BiLinear )
    {
        ImplScaleFilter( xContext, rScaleX, rScaleY, GL_LINEAR );
    }
    else if( nScaleFlag == BmpScaleFlag::Default )
    {
        const Lanczos3Kernel aKernel;

        ImplScaleConvolution( xContext, rScaleX, rScaleY, aKernel );
    }
    else if( nScaleFlag == BmpScaleFlag::BestQuality && rScaleX <= 1 && rScaleY <= 1 )
    { // Use area scaling for best quality, but only if downscaling.
        ImplScaleArea( xContext, rScaleX, rScaleY );
    }
    else if( nScaleFlag == BmpScaleFlag::Lanczos || nScaleFlag == BmpScaleFlag::BestQuality  )
    {
        const Lanczos3Kernel aKernel;

        ImplScaleConvolution( xContext, rScaleX, rScaleY, aKernel );
    }
    else
        SAL_WARN( "vcl.opengl", "Invalid flag for scaling operation" );
}

bool OpenGLSalBitmap::ScalingSupported() const
{
    return true;
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
