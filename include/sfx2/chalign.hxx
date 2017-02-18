/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_CHALIGN_HXX
#define INCLUDED_SFX2_CHALIGN_HXX

// New order: first, navigation bar, then vertically docked child windows
// (Explorer!), then horizontally docked child windows (Browser!),
// then the format bar, status bar, and at the end of the toolbar.

enum class SfxChildAlignment
{
    HIGHESTTOP,       // for example Navigation bar
    LOWESTBOTTOM,
    FIRSTLEFT,
    LASTRIGHT,
    LEFT,             // for example SplitWindow
    RIGHT,
    LASTLEFT,
    FIRSTRIGHT,
    TOP,              // for example Hyperlink-Builder, SplitWindow
    BOTTOM,           // for example SplitWindow
    TOOLBOXTOP,       // for example Object bar
    TOOLBOXBOTTOM,
    LOWESTTOP,        // for example Calc bar
    HIGHESTBOTTOM,    // for example Status bar
    TOOLBOXLEFT,      // for example Tool bar
    TOOLBOXRIGHT,
    NOALIGNMENT       // all FloatingWindows
};

// Verifies that a valid alignment is used
inline bool SfxChildAlignValid( SfxChildAlignment eAlign )
{
    return ( eAlign >= SfxChildAlignment::HIGHESTTOP && eAlign <= SfxChildAlignment::NOALIGNMENT );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
