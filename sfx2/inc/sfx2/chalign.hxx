/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_CHILDALIGN_HXX
#define _SFX_CHILDALIGN_HXX

#include <tools/solar.h>

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
inline sal_Bool SfxChildAlignValid( SfxChildAlignment eAlign )
{
    return ( eAlign >= SFX_ALIGN_HIGHESTTOP && eAlign <= SFX_ALIGN_NOALIGNMENT );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
