/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

uniform sampler2D leavingSlideTexture;
uniform sampler2D enteringSlideTexture;
uniform sampler2D permTexture;
uniform float time;
varying vec2 v_texturePosition;

float snoise(vec2 p)
{
    return texture2D(permTexture, p).r;
}

void main()
{
    if (time <= 0.5)
        gl_FragColor = texture2D(leavingSlideTexture, v_texturePosition);
    else
        gl_FragColor = texture2D(enteringSlideTexture, v_texturePosition);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
