/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define M_PI 3.1415926535897932384626433832795

uniform float time;
uniform sampler2D permTexture;
attribute vec2 center;
attribute int tileXIndex;
attribute int tileYIndex;
attribute int vertexIndexInTile;
varying vec2 v_texturePosition;

float snoise(vec2 p)
{
    return texture2D(permTexture, p).r;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

mat4 translateMatrix(vec2 whereTo)
{
    return mat4(1, 0, 0, whereTo.x,
                0, 1, 0, whereTo.y,
                0, 0, 1,         0,
                0, 0, 0,         1);
}

void main( void )
{
    vec4 v = gl_Vertex;

    // Of course this is nothing like what it will eventually be; just
    // experimenting to get at least something.

    v -= vec4(center, 0, 0);
    if (time <= 0.5)
        v = rotationMatrix(vec3(0, 1, 0), time*M_PI) * v;
    else
        v = rotationMatrix(vec3(0, 1, 0), -(1-time)*M_PI) * v;
    v += vec4(center, 0, 0);

    // v.z += 10 * (snoise(vec2(tileXIndex, tileYIndex))-0.5) * (1 - abs(time-0.5)*2);

    v.z += ((((tileXIndex << 3) ^ tileYIndex) % 10) - 5) * (1 - abs(time-0.5)*2) + 0*vertexIndexInTile;

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * v;

    v_texturePosition = gl_MultiTexCoord0.xy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
