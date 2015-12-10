/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 130

#define M_PI 3.1415926535897932384626433832795

attribute vec3 a_position;
attribute vec3 a_normal;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;

varying vec2 v_texturePosition;
varying vec3 v_normal;

attribute vec3 center;
uniform float time;
uniform ivec2 numTiles;
uniform sampler2D permTexture;
varying float angle;

float snoise(vec2 p)
{
    return texture2D(permTexture, p).r;
}

mat4 translationMatrix(vec3 axis)
{
    return mat4(1.0,    0.0,    0.0,    0.0,
                0.0,    1.0,    0.0,    0.0,
                0.0,    0.0,    1.0,    0.0,
                axis.x, axis.y, axis.z, 1.0);
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
    // There are 18 vertices in an hexagon
    int instanceID = gl_VertexID / 18;

    vec2 pos = (center.xy + 1) / 2;
    float fuzz = snoise(pos);
    float startTime = pos.x * 0.5 + fuzz * 0.25;
    float endTime = startTime + 0.25;
    float actualTime = clamp((time - startTime) / (endTime - startTime), 0, 1);
    angle = actualTime * M_PI * 2;

    mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
    mat4 transformMatrix = translationMatrix(center) * rotationMatrix(vec3(0, 1, 0), angle) * translationMatrix(-center);

    mat3 normalMatrix = mat3(transpose(inverse(transformMatrix)));
    gl_Position = u_projectionMatrix * modelViewMatrix * transformMatrix * vec4(a_position, 1.0);
    v_texturePosition = vec2((a_position.x + 1) / 2, (1 - a_position.y) / 2);
    v_normal = normalize(normalMatrix * a_normal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
