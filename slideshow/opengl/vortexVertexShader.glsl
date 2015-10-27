/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
