/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices=13) out;

in mat4 modelViewProjectionMatrix[];

uniform float hexagonSize;
uniform sampler2D permTexture;

out vec2 texturePosition;
out float fuzz;
out vec2 v_center;

const float expandFactor = 0.0318;

float snoise(vec2 p)
{
    return texture2D(permTexture, p).r;
}

void emitHexagonVertex(vec3 center, vec2 translation)
{
    vec4 pos = vec4(center + hexagonSize * expandFactor * vec3(translation, 0.0), 1.0);
    gl_Position = modelViewProjectionMatrix[0] * pos;
    texturePosition = vec2((pos.x + 1), (1 - pos.y)) / 2;
    EmitVertex();
}

void main()
{
    vec2 translateVectors[6];
    translateVectors[0] = vec2(-3, -2);
    translateVectors[1] = vec2(0, -4);
    translateVectors[2] = vec2(3, -2);
    translateVectors[3] = vec2(3, 2);
    translateVectors[4] = vec2(0, 4);
    translateVectors[5] = vec2(-3, 2);

    vec3 center = gl_in[0].gl_Position.xyz;

    v_center = (1 + center.xy) / 2;
    fuzz = snoise(center.xy);

    emitHexagonVertex(center, translateVectors[5]);

    for (int i = 0; i < 6; ++i) {
        emitHexagonVertex(center, translateVectors[i]);
        emitHexagonVertex(center, vec2(0, 0));
    }

    EndPrimitive();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
