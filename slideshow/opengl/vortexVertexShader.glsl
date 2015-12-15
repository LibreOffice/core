/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 140

#define M_PI 3.1415926535897932384626433832795

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;

varying vec2 v_texturePosition;
varying vec3 v_normal;

uniform float time;
uniform ivec2 numTiles;
uniform sampler2D permTexture;
attribute float tileInfo;
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
    vec4 v = vec4(a_position, 1.0);
    vec4 normal = vec4(a_normal, 1.0);

    // Not sure it this is like what it should eventually be; just
    // experimenting to get at least something.

    // Move the tile on a semicircular path so that it will end up at
    // the correct place. All tiles move the same direction around the
    // vertical centre axis.

    // Each tile moves during only half of the transition. The letmost
    // tiles start moving at the start and arrive at their end
    // position around time=0.5, when the tiles there (the rightmost
    // ones) start moving. (The exact time each tile is moving is
    // fuzzed a bit to make a more random appearance.)

    // In GLSL 1.20 we don't have any bitwise operators, sigh

    int tileXIndex = int(mod(int(tileInfo), 256));
    int tileYIndex = int(mod(int(tileInfo) / 256, 256));
    int vertexIndexInTile = int(mod(int(tileInfo) / (256*256), 256));

    // A semi-random number 0..1, different for neighbouring tiles.
    float fuzz = snoise(vec2(float(tileXIndex)/(numTiles.x-1), float(tileYIndex)/(numTiles.y-1)));

    // Semi-random rotation direction, identical for tiles that rotate into each other's location
    // so that they don't pass through each others in flight, which looks ugly.
    float direction = (snoise(vec2(floor(abs(float(numTiles.x-1)/2-tileXIndex))/(float(numTiles.x-1)/2), float(tileYIndex)/(numTiles.y-1))) < 0.5 ? -1 : 1);

    float startTime = float(tileXIndex)/(numTiles.x-1) * 0.5 + fuzz*0.2;
    float endTime = min(startTime + 0.5, 1.0);

    const float ALMOST_ONE = 0.999;

    if (time <= startTime)
    {
        // Still at start location, nothing needed
        v_textureSelect = 0;
    }
    else if (time > startTime && time <= endTime)
    {
        // Moving
        float rotation = direction * (time - startTime) / (endTime - startTime);

        // Avoid z fighting
        mat4 matrix = rotationMatrix(vec3(0, 1, 0), max(min(rotation, ALMOST_ONE), -ALMOST_ONE)*M_PI);
        v = matrix * v;
        normal = matrix * normal;

        v_textureSelect = float(rotation > 0.5 || rotation < -0.5);
    }
    else
    {
        // At end location. Tile is 180 degrees rotated

        // Avoid z fighting
        mat4 matrix = rotationMatrix(vec3(0, 1, 0), direction*ALMOST_ONE*M_PI);
        v = matrix * v;
        normal = matrix * normal;

        v_textureSelect = 1;
    }

    mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
    mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));
    gl_Position = u_projectionMatrix * modelViewMatrix * v;

    v_texturePosition = a_texCoord;
    v_normal = normalize(normalMatrix * vec3(normal));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
