/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
