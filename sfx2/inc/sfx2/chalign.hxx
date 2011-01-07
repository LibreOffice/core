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

// Neue Reihenfolge: zuerst Navigationsleiste, dann senkrecht angedockte
// ChildWindows (Explorer!), dann horizontal angedockte ChildWindows (Browser!),
// dann die Formatleiste, die Statuszeile, am Ende die Werkzeugleiste.

enum SfxChildAlignment
{
    SFX_ALIGN_HIGHESTTOP,       // z.B. Navigationsleiste
    SFX_ALIGN_LOWESTBOTTOM,
    SFX_ALIGN_FIRSTLEFT,
    SFX_ALIGN_LASTRIGHT,
    SFX_ALIGN_LEFT,             // z.B. SplitWindow
    SFX_ALIGN_RIGHT,
    SFX_ALIGN_LASTLEFT,
    SFX_ALIGN_FIRSTRIGHT,
    SFX_ALIGN_TOP,              // z.B. Hyperlink-Builder, SplitWindow
    SFX_ALIGN_BOTTOM,           // z.B. SplitWindow
    SFX_ALIGN_TOOLBOXTOP,       // z.B. Objektleiste
    SFX_ALIGN_TOOLBOXBOTTOM,
    SFX_ALIGN_LOWESTTOP,        // z.B. Calc-Rechenleiste
    SFX_ALIGN_HIGHESTBOTTOM,    // z.B. Statusleiste
    SFX_ALIGN_TOOLBOXLEFT,      // z.B. Werkzeugleiste
    SFX_ALIGN_TOOLBOXRIGHT,
    SFX_ALIGN_NOALIGNMENT       // alle FloatingWindows
};

// "Uberpr"uft, ob ein g"ultiges Alignment verwendet wird
inline sal_Bool SfxChildAlignValid( SfxChildAlignment eAlign )
{
    return ( eAlign >= SFX_ALIGN_HIGHESTTOP && eAlign <= SFX_ALIGN_NOALIGNMENT );
}

#endif
