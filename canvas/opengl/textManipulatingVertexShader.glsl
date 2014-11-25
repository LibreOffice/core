/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

attribute vec2 vPosition;
varying vec2 textCoords;
uniform vec2 texCord;
uniform mat4 MVP;

void main(void)
{
     gl_Position = MVP * vec4(vPosition,1,1);
     textCoords = vPosition/texCord;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
