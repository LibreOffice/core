/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chalign.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:16:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_CHILDALIGN_HXX
#define _SFX_CHILDALIGN_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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
inline BOOL SfxChildAlignValid( SfxChildAlignment eAlign )
{
    return ( eAlign >= SFX_ALIGN_HIGHESTTOP && eAlign <= SFX_ALIGN_NOALIGNMENT );
}

#endif
