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
#ifndef _SVX_HTMLMODE_HXX
#define _SVX_HTMLMODE_HXX
namespace binfilter {

// include ---------------------------------------------------------------

// define ----------------------------------------------------------------

#define HTMLMODE_ON         		0x0001
#define HTMLMODE_PARA_BORDER		0x0002 /* Absatzumrandungen */
#define HTMLMODE_PARA_DISTANCE      0x0004 /* bestimmte Absatzabstaende */
#define HTMLMODE_SMALL_CAPS         0x0008 /* Kapitaelchen */
#define HTMLMODE_FRM_COLUMNS        0x0010 /* spaltige Rahmen */
#define HTMLMODE_SOME_STYLES 	    0x0020 /* mind. MS IE */
#define HTMLMODE_FULL_STYLES 	    0x0040 /* == SW */
#define HTMLMODE_BLINK          	0x0080 /* blinkende Zeichen*/
#define HTMLMODE_PARA_BLOCK         0x0100 /* Blocksatz */
#define HTMLMODE_DROPCAPS          	0x0200 /* Initialen*/
#define HTMLMODE_FIRSTLINE          0x0400 /* Erstzeileneinzug mit Spacer == NS 3.0 */
#define HTMLMODE_GRAPH_POS          0x0800 /* Grafikpositionen Hintergrund */
#define HTMLMODE_FULL_ABS_POS       0x1000 /* abs. Rahmenpositionierung */
#define HTMLMODE_SOME_ABS_POS       0x2000 /* abs. Rahmenpositionierung vollst.*/
#define HTMLMODE_RESERVED1          0x4000
#define HTMLMODE_RESERVED0          0x8000

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
