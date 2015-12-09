/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150

#define M_PI 3.1415926535897932384626433832795

in vec3 a_position;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;

uniform float time;
uniform float selectedTexture;

out mat4 modelViewProjectionMatrix;

mat4 translationMatrix(vec3 axis)
{
    return mat4(1.0,    0.0,    0.0,    0.0,
                0.0,    1.0,    0.0,    0.0,
                0.0,    0.0,    1.0,    0.0,
                axis.x, axis.y, axis.z, 1.0);
}

mat4 scaleMatrix(vec3 axis)
{
    return mat4(axis.x, 0.0,    0.0,    0.0,
                0.0,    axis.y, 0.0,    0.0,
                0.0,    0.0,    axis.z, 0.0,
                0.0,    0.0,    0.0,    1.0);
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
    mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
    mat4 transformMatrix;

    // TODO: use the aspect ratio of the slide instead.
    mat4 slideScaleMatrix = scaleMatrix(vec3(0.75, 1, 1));
    mat4 invertSlideScaleMatrix = scaleMatrix(1.0 / vec3(0.75, 1, 1));

    if (selectedTexture > 0.5) {
        // Leaving texture
        transformMatrix = translationMatrix(vec3(0, 0, 6 * time))
            * scaleMatrix(vec3(1 + pow(2 * time, 2.1), 1 + pow(2 * time, 2.1), 0))
            * slideScaleMatrix
            * rotationMatrix(vec3(0.0, 0.0, 1.0), -pow(time, 3) * M_PI)
            * invertSlideScaleMatrix;
    } else {
        // Entering texture
        transformMatrix = translationMatrix(vec3(0, 0, 28 * (sqrt(time) - 1)))
            * slideScaleMatrix
            * rotationMatrix(vec3(0.0, 0.0, 1.0), pow(time - 1, 2) * M_PI)
            * invertSlideScaleMatrix;
    }
    modelViewProjectionMatrix = u_projectionMatrix * modelViewMatrix * transformMatrix;
    gl_Position = vec4(a_position, 1.0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
