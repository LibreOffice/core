/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150 core

uniform float minCoordX;
in vec3 positionWorldspace;
in vec4 fragmentColor;
out vec4 actualColor;

void main()
{
    if (positionWorldspace.x <= minCoordX)
        discard;
    actualColor = fragmentColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
