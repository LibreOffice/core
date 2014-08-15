/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 330 core

#extension GL_EXT_texture_array : enable

uniform sampler2DArray texArray;
in vec3 vTexCoord;

out vec4 actualColor;
void main()
{
    vec3 color = texture(texArray, vTexCoord.xyz).rgb;
    actualColor = vec4(color, 1.0 - color.r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
