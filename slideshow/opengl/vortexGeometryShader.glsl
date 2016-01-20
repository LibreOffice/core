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
layout(triangle_strip, max_vertices=11) out;

in vec2 g_texturePosition[];
in vec3 g_normal[];
in mat4 modelViewMatrix[];
in mat4 transform[];
in float nTime[];
in float startTime[];
in float endTime[];

uniform mat4 u_projectionMatrix;

out vec2 v_texturePosition;
out vec3 v_normal;

void emitHexagonVertex(int index, vec3 translation, float fdsq)
{
    mat4 normalMatrix = transpose(inverse(modelViewMatrix[index]));

    vec4 pos = gl_in[index].gl_Position + vec4(translation, 0.0);

    // Apply our transform operations.
    pos = transform[index] * pos;

    v_normal = normalize(vec3(normalMatrix * transform[index] * vec4(g_normal[index], 0.0)));
    v_normal.z *= fdsq;

    gl_Position = u_projectionMatrix * modelViewMatrix[index] * pos;
    v_texturePosition = g_texturePosition[index];
    EmitVertex();
}

void main()
{
    const vec4 invalidPosition = vec4(-256.0, -256.0, -256.0, -256.0);
    const vec3 noTranslation = vec3(0.0, 0.0, 0.0);

    if (gl_in[0].gl_Position == invalidPosition)
        return;

    // Draw “walls” to the hexagons.
    if (nTime[0] > startTime[0] && nTime[0] <= endTime[0]) {
        const vec3 translation = vec3(0.0, 0.0, -0.02);

        emitHexagonVertex(2, noTranslation, 0.3);
        emitHexagonVertex(2, translation, 0.3);

        for (int i = 0; i < 3; ++i) {
            emitHexagonVertex(i, noTranslation, 0.3);
            emitHexagonVertex(i, translation, 0.3);
        }

        EndPrimitive();
    }

    // Draw the main quad part.
    for (int i = 0; i < 3; ++i) {
        emitHexagonVertex(i, noTranslation, 1.0);
    }

    EndPrimitive();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
