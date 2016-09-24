/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150 core

in vec2 vTexCoord;
uniform sampler2D TextTex;
uniform vec4 textColor;
out vec4 actualColor;
void main()
{
    vec3 color = texture2D(TextTex, vTexCoord).rgb;
    actualColor = ((1.0 - color.r) == 0.0) ? vec4(0.0, 0.0, 0.0, textColor.a) :
                    vec4(textColor.rgb, 1.0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
