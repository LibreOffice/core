/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

uniform sampler2D leavingSlideTexture;
uniform sampler2D enteringSlideTexture;
uniform float time;
varying vec2 v_texturePosition;
varying float v_textureSelect;
varying vec3 v_normal;

void main()
{
    vec3 lightVector = vec3(0.0, 0.0, 1.0);
    float light = abs(dot(lightVector, v_normal));
    vec4 fragment;

    if (v_textureSelect == 0)
    {
        fragment = texture2D(leavingSlideTexture, v_texturePosition);
    }
    else
    {
        vec2 pos = v_texturePosition;
        pos.x = 1 - pos.x;
        fragment = texture2D(enteringSlideTexture, pos);
    }

    vec4 black = vec4(0.0, 0.0, 0.0, fragment.a);
    gl_FragColor = mix(black, fragment, light);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
