/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlmode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:54:46 $
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
#ifndef _SVX_HTMLMODE_HXX
#define _SVX_HTMLMODE_HXX

// include ---------------------------------------------------------------

// define ----------------------------------------------------------------

#define HTMLMODE_ON                 0x0001
#define HTMLMODE_PARA_BORDER        0x0002 /* Absatzumrandungen */
#define HTMLMODE_PARA_DISTANCE      0x0004 /* bestimmte Absatzabstaende */
#define HTMLMODE_SMALL_CAPS         0x0008 /* Kapitaelchen */
#define HTMLMODE_FRM_COLUMNS        0x0010 /* spaltige Rahmen */
#define HTMLMODE_SOME_STYLES        0x0020 /* mind. MS IE */
#define HTMLMODE_FULL_STYLES        0x0040 /* == SW */
#define HTMLMODE_BLINK              0x0080 /* blinkende Zeichen*/
#define HTMLMODE_PARA_BLOCK         0x0100 /* Blocksatz */
#define HTMLMODE_DROPCAPS           0x0200 /* Initialen*/
#define HTMLMODE_FIRSTLINE          0x0400 /* Erstzeileneinzug mit Spacer == NS 3.0 */
#define HTMLMODE_GRAPH_POS          0x0800 /* Grafikpositionen Hintergrund */
#define HTMLMODE_FULL_ABS_POS       0x1000 /* abs. Rahmenpositionierung */
#define HTMLMODE_SOME_ABS_POS       0x2000 /* abs. Rahmenpositionierung vollst.*/
#define HTMLMODE_RESERVED1          0x4000
#define HTMLMODE_RESERVED0          0x8000



#endif



















