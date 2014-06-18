/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

uniform sampler2D TextTex;
varying vec2 vTexCoord;
void main()
{
    vec3 color = texture2D(TextTex, vTexCoord).rgb;
    gl_FragColor = vec4(color, 1.0 - color.r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
