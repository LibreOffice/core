/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150 core

uniform sampler2D TextTex;
in vec2 vTexCoord;
out vec4 actualColor;


void main()
{
    vec3 color = texture(TextTex, vTexCoord).rgb;
    actualColor = vec4(color, 1.0 - color.r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
