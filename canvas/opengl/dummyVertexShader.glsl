/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
 
uniform mat4 MVP;
attribute vec2 vPosition;
attribute vec2 vPosition;
varying vec2 v_textureCoords2d;
void main(void)
{
    gl_Position = MVP * vec4(vPosition,1,1);
    v_textureCoords2d = gl_MultiTexCoord0.st;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
