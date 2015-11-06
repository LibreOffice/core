/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D leavingSlideTexture;
uniform sampler2D enteringSlideTexture;
uniform float time;
uniform vec2 center;
varying vec2 v_texturePosition;

void main()
{
    float d = length(v_texturePosition - center);
    float w = 0;
    w = max(w, length(center - vec2(0, 0)));
    w = max(w, length(center - vec2(1, 0)));
    w = max(w, length(center - vec2(1, 1)));
    w = max(w, length(center - vec2(0, 1)));
    float v = 0.2;
    float smoothtime = smoothstep(0, 1, time);
    float a = smoothstep(smoothtime*w-v, smoothtime*w+v, d);
    a += (0.5 - abs(a-0.5))*sin(d*M_PI*30);
    gl_FragColor = mix(texture2D(enteringSlideTexture, v_texturePosition), texture2D(leavingSlideTexture, v_texturePosition), a);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
