/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

uniform sampler2D TextTex;	
uniform vec4 colorTex;
uniform mat4 texTrans;
//Texture Coordinates
varying vec2 textCoords;

void main()
{
		vec2 texCoord = (texTrans * vec4(textCoords, 0, 1)).xy;
        gl_FragColor = texture2D( TextTex, texCoord )*colorTex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
