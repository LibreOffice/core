/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 120

varying vec4 fragmentColor;
uniform int shape;

void main()
{
    vec2 p = gl_PointCoord * 2.0 - vec2(1.0); // (0,0) in the center
    if(shape == 0)
    {
    }
    else if(shape == 1) //diamon
    {
        if (abs(p.x) + abs(p.y) > 1)
            discard;
    }
    else if(shape == 2) // arrow
    {
        if(p.y < 0 && (abs(p.x) + abs(p.y)) > 1)
            discard;
        else if(p.y > 0 && abs(p.x) > 0.5)
            discard;
    }
    else if(shape == 3) //arrow up
    {
        if(p.y > 0 && (abs(p.x) + abs(p.y)) > 1)
            discard;
        else if(p.y < 0 && abs(p.x) > 0.5)
            discard;
    }
    else if(shape == 4)
    {
        if(p.x > 0 && (abs(p.x) + abs(p.y)) > 1)
            discard;
        else if(p.x < 0 && abs(p.y) > 0.5)
            discard;
    }
    else if(shape == 5)
    {
        if(p.x < 0 && (abs(p.x) + abs(p.y)) > 1)
            discard;
        else if(p.x > 0 && abs(p.y) > 0.5)
            discard;
    }
    else if(shape == 6)
    {
        if(abs(p.x) < abs(p.y))
            discard;
    }
    else if(shape == 7)
    {
        if(abs(p.y) < abs(p.x))
            discard;
    }
    else if(shape == 8)
    {
        if(dot(p.x, p.y) > 1)
            discard;
    }
    else if(shape == 9)
    {
    }
    
    gl_FragColor = fragmentColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
