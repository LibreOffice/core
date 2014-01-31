/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

attribute vec3 vPosition;
uniform mat4 MVP;
attribute vec2 texCoord;
varying vec2 vTexCoord;
void main()
{
    gl_Position =  MVP * vec4(vPosition, 1);
    vTexCoord = texCoord;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
