/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150 core

in vec3 vPosition;
uniform mat4 MVP;
uniform mat4 M;
uniform vec4 vColor;
uniform float minCoordX;
uniform float maxCoordX;
out vec4 fragmentColor;
out vec3 positionWorldspace;
void main()
{
    positionWorldspace = (M * vec4(vPosition,1)).xyz;
	  positionWorldspace.x = clamp(positionWorldspace.x, minCoordX, maxCoordX);
    gl_Position =  MVP * vec4(vPosition, 1);
    fragmentColor = vColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
