/*************************************************************************
 *
 *  $RCSfile: clsids.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-15 07:57:12 $
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
#ifndef _SOT_CLSIDS_HXX
#define _SOT_CLSIDS_HXX

/*
 * StarWriter
 */

/* 3.0 */

#define SO3_SW_CLASSID_30 \
    0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61, 0x04, \
    0x02, 0x1C, 0x00, 0x70, 0x02

/* 4.0 */

#define SO3_SW_CLASSID_40 \
    0x8b04e9b0,  0x420e, 0x11d0, 0xa4, 0x5e, 0x0, \
    0xa0, 0x24, 0x9d, 0x57, 0xb1

/* 5.0 */

#define SO3_SW_CLASSID_50 \
    0xc20cf9d1, 0x85ae, 0x11d1, 0xaa, 0xb4, 0x0, \
    0x60, 0x97, 0xda, 0x56, 0x1a

 /* 6.0 */

#define SO3_SW_CLASSID_60 \
    0x8BC6B165, 0xB1B2, 0x4EDD, 0xAA, 0x47, 0xDA, \
    0xE2, 0xEE, 0x68, 0x9D, 0xD6

/* aktuell */

#define SO3_SW_CLASSID SO3_SW_CLASSID_60

/*
 * StarWriter/Web
 */

/* 4.0 */

#define SO3_SWWEB_CLASSID_40 \
    0xf0caa840, 0x7821, 0x11d0, 0xa4, 0xa7, 0x0, \
    0xa0, 0x24, 0x9d, 0x57, 0xb1

/* 5.0 */

#define SO3_SWWEB_CLASSID_50 \
    0xc20cf9d2, 0x85ae, 0x11d1, 0xaa, 0xb4, 0x0, \
    0x60, 0x97, 0xda, 0x56, 0x1a

/* 6.0 */

#define SO3_SWWEB_CLASSID_60 \
    0xA8BBA60C, 0x7C60, 0x4550, 0x91, 0xCE, 0x39, \
    0xC3, 0x90, 0x3F, 0xAC, 0x5E

/* aktuell */

#define SO3_SWWEB_CLASSID SO3_SWWEB_CLASSID_60


/*
 * Globaldokument
 */

/* 4.0 */

#define SO3_SWGLOB_CLASSID_40 \
    0x340ac970, 0xe30d, 0x11d0, 0xa5, 0x3f, 0x0, \
    0xa0, 0x24, 0x9d, 0x57, 0xb1

/* 5.0 */

#define SO3_SWGLOB_CLASSID_50 \
    0xc20cf9d3, 0x85ae, 0x11d1, 0xaa, 0xb4, 0x0, \
    0x60, 0x97, 0xda, 0x56, 0x1a

/* 6.0 */

#define SO3_SWGLOB_CLASSID_60 \
    0xB21A0A7C, 0xE403, 0x41FE, 0x95, 0x62, 0xBD, \
    0x13, 0xEA, 0x6F, 0x15, 0xA0

/* aktuell */

#define SO3_SWGLOB_CLASSID SO3_SWGLOB_CLASSID_60



//---------------------------------------------------

/*
 * StarCalc
 */

/* 3.0 */

#define SO3_SC_CLASSID_30 \
    0x3F543FA0L, 0xB6A6, 0x101B, 0x99, 0x61, 0x04, \
    0x02, 0x1C, 0x00, 0x70, 0x02

/* 4.0 */

#define SO3_SC_CLASSID_40 \
    0x6361d441L, 0x4235, 0x11d0, 0x89, 0xcb, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 5.0 */

#define SO3_SC_CLASSID_50 \
    0xc6a5b861L, 0x85d6, 0x11d1, 0x89, 0xcb, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 6.0 */

#define SO3_SC_CLASSID_60 \
    0x47BBB4CB, 0xCE4C, 0x4E80, 0xA5, 0x91, 0x42, \
    0xD9, 0xAE, 0x74, 0x95, 0x0F

/* aktuell */

#define SO3_SC_CLASSID SO3_SC_CLASSID_60



/****************************************************
* StarImpress
****************************************************/

/* 3.0 */

#define SO3_SIMPRESS_CLASSID_30 \
    0xAF10AAE0L, 0xB36D, 0x101B, 0x99, 0x61, 0x04, \
    0x02, 0x1C, 0x00, 0x70, 0x02

/* 4.0 */

#define SO3_SIMPRESS_CLASSID_40 \
    0x12d3cc0L, 0x4216, 0x11d0, 0x89, 0xcb, 0x0, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 5.0 */

#define SO3_SIMPRESS_CLASSID_50 \
    0x565c7221L, 0x85bc, 0x11d1, 0x89, 0xd0, 0x0, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 6.0 */

#define SO3_SIMPRESS_CLASSID_60 \
    0x9176E48A, 0x637A, 0x4D1F, 0x80, 0x3B, 0x99, \
    0xD9, 0xBF, 0xAC, 0x10, 0x47

/* aktuell */

#define SO3_SIMPRESS_CLASSID  SO3_SIMPRESS_CLASSID_60



/****************************************************
* StarDraw
****************************************************/

/* 5.0 */

#define SO3_SDRAW_CLASSID_50 \
    0x2e8905a0L, 0x85bd, 0x11d1, 0x89, 0xd0, 0x0, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 6.0 */
#define SO3_SDRAW_CLASSID_60 \
    0x4BAB8970, 0x8A3B, 0x45B3, 0x99, 0x1C, 0xCB, \
    0xEE, 0xAC, 0x6B, 0xD5, 0xE3

/* aktuell */

#define SO3_SDRAW_CLASSID  SO3_SDRAW_CLASSID_60



/****************************************************
* StarChart
****************************************************/

/* 3.0 */

#define SO3_SCH_CLASSID_30 \
    0xFB9C99E0L, 0x2C6D, 0x101C, 0x8E, 0x2C, 0x00, \
    0x00, 0x1B, 0x4C, 0xC7, 0x11

/* 4.0 */

#define SO3_SCH_CLASSID_40 \
    0x2b3b7e0L, 0x4225, 0x11d0, 0x89, 0xca, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 5.0 */

#define SO3_SCH_CLASSID_50 \
    0xbf884321L, 0x85dd, 0x11d1, 0x89, 0xd0, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 6.0 */

#define SO3_SCH_CLASSID_60 \
    0x12DCAE26, 0x281F, 0x416F, 0xA2, 0x34, 0xC3, \
    0x08, 0x61, 0x27, 0x38, 0x2E

/* aktuell */

#define SO3_SCH_CLASSID  SO3_SCH_CLASSID_60



/****************************************************
* StarImage
****************************************************/

/* 3.0 */

#define SO3_SIM_CLASSID_30 \
    0xEA60C941L, 0x2C6C, 0x101C, 0x8E, 0x2C, 0x00, \
    0x00, 0x1B, 0x4C, 0xC7, 0x11

/* 4.0 */

#define SO3_SIM_CLASSID_40 \
    0x447BB8A0L, 0x41FB, 0x11D0, 0x89, 0xCA, 0x00, \
    0x80, 0x29, 0xE4, 0xB0, 0xB1

/* 5.0 */

#define SO3_SIM_CLASSID_50 \
    0x65c68d00L, 0x85de, 0x11d1, 0x89, 0xd0, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* aktuell */

#define SO3_SIM_CLASSID  SO3_SIM_CLASSID_50



/***************************************************
* StarMath
***************************************************/

/* 3.0 */

#define SO3_SM_CLASSID_30 \
    0xD4590460L, 0x35FD, 0x101C, 0xB1, 0x2A, 0x04, \
    0x02, 0x1C, 0x00, 0x70, 0x02

/* 4.0 */

#define SO3_SM_CLASSID_40 \
    0x2b3b7e1L, 0x4225, 0x11d0, 0x89, 0xca, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 5.0 */

#define SO3_SM_CLASSID_50 \
    0xffb5e640L, 0x85de, 0x11d1, 0x89, 0xd0, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

/* 6.0 */

#define SO3_SM_CLASSID_60 \
    0x078B7ABA, 0x54FC, 0x457F, 0x85, 0x51, 0x61, \
    0x47, 0xE7, 0x76, 0xA9, 0x97

/* aktuell */

#define SO3_SM_CLASSID  SO3_SM_CLASSID_60

#define SO3_OUT_CLASSID \
    0x970b1e82, 0xcf2d, 0x11cf, 0x89, 0xca, 0x00, \
    0x80, 0x29, 0xe4, 0xb0, 0xb1

#define SO3_APPLET_CLASSID \
    0x970b1e81, 0xcf2d, 0x11cf, \
    0x89,0xca,0x00,0x80,0x29,0xe4,0xb0,0xb1

#define SO3_PLUGIN_CLASSID \
    0x4caa7761, 0x6b8b, 0x11cf, \
    0x89,0xca,0x0,0x80,0x29,0xe4,0xb0,0xb1

#define SO3_IFRAME_CLASSID \
    0x1a8a6701, 0xde58, 0x11cf, \
    0x89, 0xca, 0x0, 0x80, 0x29, 0xe4, 0xb0, 0xb1
#endif

