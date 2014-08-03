/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
attribute vec3 vertexPositionModelspace;
attribute vec3 vertexNormalModelspace;

varying vec3 positionWorldspace;
varying vec3 normalCameraspace;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform mat3 normalMatrix;
uniform float minCoordX;
uniform float maxCoordX;

void main()
{
	  positionWorldspace = (M * vec4(vertexPositionModelspace,1)).xyz;

	  positionWorldspace.x = clamp(positionWorldspace.x, minCoordX, maxCoordX);

	  gl_Position =  P * V * vec4(positionWorldspace,1);

    normalCameraspace = normalize(mat3(V) * normalMatrix * vertexNormalModelspace);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
