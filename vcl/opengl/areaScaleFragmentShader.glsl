/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120
#if __VERSION__ < 130
int min( int a, int b ) { return a < b ? a : b; }
float min( float a, float b ) { return a < b ? a : b; }
#endif

/* TODO Use textureOffset for newest version of GLSL */

uniform sampler2D sampler;
uniform int swidth;
uniform int sheight;
uniform float xscale;
uniform float yscale;
uniform float xsrcconvert;
uniform float ysrcconvert;
uniform float xdestconvert;
uniform float ydestconvert;

varying vec2 tex_coord;

// This mode makes the scaling work like maskedTextureFragmentShader.glsl
// (instead of like plain textureVertexShader.glsl).
#ifdef MASKED
varying vec2 mask_coord;
uniform sampler2D mask;
#endif

void main(void)
{
    // Convert to pixel coordinates again.
    int dx = int( tex_coord.s * xdestconvert );
    int dy = int( tex_coord.t * ydestconvert );

    // Note: These values are always the same for the same X (or Y),
    // so they could be precalculated in C++ and passed to the shader,
    // but GLSL has limits on the size of uniforms passed to it,
    // so it'd need something like texture buffer objects from newer
    // GLSL versions, and it seems the hassle is not really worth it.

    // How much each column/row will contribute to the resulting pixel.
    // assert( xscale <= 100 ); assert( yscale <= 100 );
    float xratio[ 100 + 2 ];
    float yratio[ 100 + 2 ];
    // For finding the first and last source pixel.
    int xpixel[ 100 + 2 ];
    int ypixel[ 100 + 2 ];

    int xpos = 0;
    int ypos = 0;

    // Compute the range of source pixels which will make up this destination pixel.
    float fsx1 = dx * xscale;
    float fsx2 = fsx1 + xscale;
    // To whole pixel coordinates.
    int sx1 = int( ceil( fsx1 ) );
    int sx2 = int( floor( fsx2 ) );
    // Range checking.
    sx2 = min( sx2, swidth - 1 );
    sx1 = min( sx1, sx2 );

    // How much one full column contributes to the resulting pixel.
    float width = min( xscale, swidth - fsx1 );

    if( sx1 - fsx1 > 0.001 )
    {   // The first column contributes only partially.
        xpixel[ xpos ] = sx1 - 1;
        xratio[ xpos ] = ( sx1 - fsx1 ) / width;
        ++xpos;
    }
    for( int sx = sx1; sx < sx2; ++sx )
    {   // Columns that fully contribute to the resulting pixel.
        xpixel[ xpos ] = sx;
        xratio[ xpos ] = 1.0 / width;
        ++xpos;
    }
    if( fsx2 - sx2 > 0.001 )
    {   // The last column contributes only partially.
        xpixel[ xpos ] = sx2;
        xratio[ xpos ] = min( min( fsx2 - sx2, 1.0 ) / width, 1.0 );
        ++xpos;
    }

    // The same for Y.
    float fsy1 = dy * yscale;
    float fsy2 = fsy1 + yscale;
    int sy1 = int( ceil( fsy1 ) );
    int sy2 = int( floor( fsy2 ) );
    sy2 = min( sy2, sheight - 1 );
    sy1 = min( sy1, sy2 );

    float height = min( yscale, sheight - fsy1 );

    if( sy1 - fsy1 > 0.001 )
    {
        ypixel[ ypos ] = sy1 - 1;
        yratio[ ypos ] = ( sy1 - fsy1 ) / height;
        ++ypos;
    }
    for( int sy = sy1; sy < sy2; ++sy )
    {
        ypixel[ ypos ] = sy;
        yratio[ ypos ] = 1.0 / height;
        ++ypos;
    }
    if( fsy2 - sy2 > 0.001 )
    {
        ypixel[ ypos ] = sy2;
        yratio[ ypos ] = min( min( fsy2 - sy2, 1.0 ) / height, 1.0 );
        ++ypos;
    }

    int xstart = xpixel[ 0 ];
    int xend = xpixel[ xpos - 1 ];
    int ystart = ypixel[ 0 ];
    int yend = ypixel[ ypos - 1 ];

    vec4 sum = vec4( 0.0, 0.0, 0.0, 0.0 );

    ypos = 0;
    for( int y = ystart; y <= yend; ++y, ++ypos )
    {
        vec4 tmp = vec4( 0.0, 0.0, 0.0, 0.0 );
        xpos = 0;
        for( int x = xstart; x <= xend; ++x, ++xpos )
        {
            vec2 offset = vec2( x * xsrcconvert, y * ysrcconvert );
#ifndef MASKED
            tmp += texture2D( sampler, offset ) * xratio[ xpos ];
#else
            vec4 texel;
            texel = texture2D( sampler, offset );
            texel.a = 1.0 - texture2D( mask, offset ).r;
            tmp += texel * xratio[ xpos ];
#endif
        }
        sum += tmp * yratio[ ypos ];
    }

    gl_FragColor = sum;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
