/*************************************************************************
 *
 *  $RCSfile: htmlmode.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



















