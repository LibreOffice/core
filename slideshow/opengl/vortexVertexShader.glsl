/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

#define M_PI 3.1415926535897932384626433832795

uniform float time;
uniform ivec2 numTiles;
uniform sampler2D permTexture;
attribute float tileInfo;
varying vec2 v_texturePosition;
varying float v_textureSelect;

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

void main( void )
{
    vec4 v = gl_Vertex;

    // Of course this is nothing like what it will eventually be; just
    // experimenting to get at least something.

    // Move the tile on a semicircular path so that it will end up at the correct place
    // Move half the tiles one way, half the other.

    // Each tile moves during only half of the transition. The letmost
    // tiles start moving immediately and arrive at their end position
    // at time=0.5, when the tiles there (the rightmost ones) start
    // moving.

    // In GLSL 1.20 we don't have any bitwise operators, sigh

    int tileXIndex = int(mod(int(tileInfo), 256));
    int tileYIndex = int(mod(int(tileInfo) / 256, 256));
    int vertexIndexInTile = int(mod(int(tileInfo) / (256*256), 256));

    float startTime = float(tileXIndex)/(numTiles.x-1) * 0.5;
    float endTime = startTime + 0.5;

    vec2 tileCenter = vec2(-1 + 1.5 * tileXIndex * (2.0/numTiles.x), -1 + 1.5 * tileYIndex * (2.0/numTiles.y));

    if (time <= startTime)
    {
        // Still at start location, nothing needed
        v_textureSelect = 0;
    }
    else if (time > startTime && time <= endTime)
    {
        // Moving
        float moveTime = (time - startTime) * 2;

        // First: Rotate the tile around its Y axis,
        // It rotates 180 degrees during the transition.
        // Translate to origin, rotate.

        v -= vec4(tileCenter, 0, 0);

        // A semi-random number 0..1, different for neighbouring tiles
        float fuzz = snoise(256*vec2(float(tileXIndex)/(numTiles.x-1), float(tileYIndex)/(numTiles.y-1)));

        float rotation = moveTime;

        // experiment: perturb rotation a bit randomly
        // rotation = moveTime - fuzz*(0.5-abs(time - 0.5));

        v = rotationMatrix(vec3(0, 1, 0), rotation*M_PI) * v;

        v.x += tileCenter.x * cos(moveTime*M_PI);
        v.y += tileCenter.y;
        v.z += (fuzz < 0.5 ? -1 : 1) * tileCenter.x * sin(moveTime*M_PI);

        // Perturb z a bit randomly
        v.z += (fuzz - 0.5) * 5 * (1 - abs(time-0.5)*2);

        v_textureSelect = float(rotation > 0.5);
    }
    else
    {
        // At end location. Tile is 180 degrees rotated

        v -= vec4(tileCenter, 0, 0);
        v = rotationMatrix(vec3(0, 1, 0), M_PI) * v;
        v += vec4(-tileCenter.x, tileCenter.y, 0, 0);

        v_textureSelect = 1;
    }

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * v;

    v_texturePosition = gl_MultiTexCoord0.xy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
