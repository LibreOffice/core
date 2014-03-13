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

enum SfxChildAlignment
{
    SFX_ALIGN_HIGHESTTOP,       // for example Navigation bar
    SFX_ALIGN_LOWESTBOTTOM,
    SFX_ALIGN_FIRSTLEFT,
    SFX_ALIGN_LASTRIGHT,
    SFX_ALIGN_LEFT,             // for example SplitWindow
    SFX_ALIGN_RIGHT,
    SFX_ALIGN_LASTLEFT,
    SFX_ALIGN_FIRSTRIGHT,
    SFX_ALIGN_TOP,              // for example Hyperlink-Builder, SplitWindow
    SFX_ALIGN_BOTTOM,           // for example SplitWindow
    SFX_ALIGN_TOOLBOXTOP,       // for example Objekt bar
    SFX_ALIGN_TOOLBOXBOTTOM,
    SFX_ALIGN_LOWESTTOP,        // for example Calc bar
    SFX_ALIGN_HIGHESTBOTTOM,    // for example Status bar
    SFX_ALIGN_TOOLBOXLEFT,      // for example Tool bar
    SFX_ALIGN_TOOLBOXRIGHT,
    SFX_ALIGN_NOALIGNMENT       // all FloatingWindows
};

// Verifies that a valid alignment is used
inline bool SfxChildAlignValid( SfxChildAlignment eAlign )
{
    return ( eAlign >= SFX_ALIGN_HIGHESTTOP && eAlign <= SFX_ALIGN_NOALIGNMENT );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
