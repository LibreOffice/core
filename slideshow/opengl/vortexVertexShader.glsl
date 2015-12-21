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
uniform float slide;

varying vec4 debug;

float snoise(vec2 p)
{
    return texture2D(permTexture, p).r;
}

mat4 identityMatrix(void)
{
    return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

mat4 translationMatrix(vec3 axis)
{
    mat4 matrix = identityMatrix();
    matrix[3] = vec4(axis, 1.0);
    return matrix;
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

    // Each tile moves during only half of the transition. The letmost
    // tiles start moving at the start and arrive at their end
    // position around time=0.5, when the tiles there (the rightmost
    // ones) start moving. (The exact time each tile is moving is
    // fuzzed a bit to make a more random appearance.)

    // In GLSL 1.20 we don't have any bitwise operators, sigh

    int tileXIndex = int(mod(int(tileInfo), 256));
    int tileYIndex = int(mod(int(tileInfo) / 256, 256));

    // A semi-random number 0..1, different for neighbouring tiles, to know when they should start moving.
    float startTimeFuzz = snoise(vec2(float(tileXIndex)/(numTiles.x-1), float(tileYIndex)/(numTiles.y-1)));

    // A semi-random number -1.5..1.5, different for neighbouring tiles, to specify their rotation center.
    // The additional 0.5 on each side is because we want some tiles to rotate outside.
    float rotationFuzz = snoise(vec2(float(numTiles.x + tileXIndex)/(numTiles.x-1), float(tileYIndex)/(numTiles.y-1))) * 3.0 - 1.5;

    float startTime = float(tileXIndex)/(numTiles.x-1) * 0.2 + startTimeFuzz * 0.2;
    float endTime = min(startTime + 0.7, 1.0);

    bool isLeavingSlide = (slide < 0.5);
    const vec4 invalidPosition = vec4(-256.0, -256.0, -256.0, -256.0);

    float nTime;

    // Don’t display the tile before or after its rotation, depending on the slide.
    if (!isLeavingSlide)
    {
        if (time < max(0.3, startTime))
        {
            gl_Position = invalidPosition;
            return;
        }
        nTime = 1.0 - time;
    }
    else
    {
        if (time > endTime)
        {
            gl_Position = invalidPosition;
            return;
        }
        nTime = time;
    }

    mat4 transform = identityMatrix();
    if (nTime > startTime && nTime <= endTime)
    {
        // We are in the rotation part.
        float rotation = -(nTime - startTime) / (endTime - startTime);

        // Translation vector to set the origin of the rotation.
        vec3 translationVector = vec3(rotationFuzz, 0.0, 0.0);

        // Compute the actual rotation matrix.
        transform = translationMatrix(translationVector)
                  * rotationMatrix(vec3(0, 1, 0), clamp(rotation, -1.0, 1.0) * M_PI)
                  * translationMatrix(-translationVector)
                  * transform;

        // Add a translation movement to the leaving slide so it doesn’t exactly mirror the entering one.
        if (isLeavingSlide && nTime > 0.3)
        {
            float movement = smoothstep(0.0, 1.0, (nTime - 0.3) * 2.0);
            transform = translationMatrix(vec3(-movement, 0.0, -0.5 * movement)) * transform;
        }
    }

    // Apply our transform operations.
    v = transform * v;
    normal = transform * normal;

    mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
    mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));
    gl_Position = u_projectionMatrix * modelViewMatrix * v;

    v_texturePosition = a_texCoord;
    v_normal = normalize(normalMatrix * vec3(normal));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
