/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 330 core

in vec3 vertexPositionModelspace;
in vec3 vertexNormalModelspace;
in mat4 M;
in mat3 normalMatrix;
in vec4 barColor;

out vec3 positionWorldspace;
out vec3 normalCameraspace;
out vec4 fragBarColor;

uniform mat4 P;
uniform mat4 V;

void main()
{
    gl_Position =  P * V * M * vec4(vertexPositionModelspace,1);

    positionWorldspace = (M * vec4(vertexPositionModelspace,1)).xyz;

    normalCameraspace = normalize(mat3(V) * normalMatrix * vertexNormalModelspace);
	
    fragBarColor = barColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
