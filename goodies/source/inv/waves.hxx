/*************************************************************************
 *
 *  $RCSfile: waves.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
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
#ifndef _WAVES_HXX
#define _WAVES_HXX

#include <tools/solar.h>

#define MAX_LEN     250
#define WAVES       10
#define RANMAX      60

#ifdef WIN
#pragma data_seg( "MY_DATA", "DATA" )
#endif

USHORT ranpos[RANMAX*2+1] =
{   60,
    10,20,50,20,90,20,130,20,170,20,210,20,250,20,290,20,330,20,370,20,
    30,50,70,50,110,50,150,50,190,50,230,50,270,50,310,50,350,50,390,50,
    10,80,50,80,90,80,130,80,170,80,210,80,250,80,290,80,330,80,370,80,
    30,110,70,110,110,110,150,110,190,110,230,110,270,110,310,110,350,
    110,390,110,
    10,140,50,140,90,140,130,140,170,140,210,140,250,140,290,140,330,140,
    370,140,
    30,170,70,170,110,170,150,170,190,170,230,170,270,170,310,170,350,
    170,390,170
};

USHORT levelpuf[RANMAX*3+2] =
{   0, 0, 0, 0, 0};

BYTE waves[WAVES][MAX_LEN] = {
{ 11, 0, 10, 20, 0, 50, 20, 0, 90, 20, 0, 130, 20, 0, 170, 20,
    0, 30, 50, 0, 70, 50, 0, 110, 50, 0, 150, 50, 0, 190, 50,
    4,100, 200,
    240, 240, 0 },
{ 12, 1, 10, 20, 1, 50, 20, 1, 90, 20, 1, 130, 20, 1, 170, 20,
    1, 30, 50, 1, 70, 50, 1, 110, 50, 1, 150, 50, 1, 190, 50,
    4,80, 200, 4,120,200,
    15, 15, 15 },
{ 13, 2, 10, 20, 2, 50, 20, 2, 90, 20, 2, 130, 20, 2, 170, 20,
    2, 30, 50, 2, 70, 50, 2, 110, 50, 2, 150, 50, 2, 190, 50,
    4,5,250, 4,100,190, 4,200,230,
    240, 240, 0 },
{ 14, 3, 10, 20, 3, 50, 20, 3, 90, 20, 3, 130, 20, 3, 170, 20,
    3, 30, 50, 3, 70, 50, 3, 110, 50, 3, 150, 50, 3, 190, 50,
    4, 10,190, 4,50,240, 4,120,250, 4, 180,210,
    15, 15, 15 },
{ 25, 0, 10, 50, 0, 10, 90, 0, 10, 130,
      1, 50, 30, 1, 50, 70, 1, 50, 110,
      2, 90, 10, 1, 90, 50, 2, 90, 90,
      3,130, 10, 3,130, 50, 3,130, 90,
      2,170, 10, 1,170, 50, 2,170, 90,
      1,210, 30, 1,210, 70, 1,210, 110,
      0,250, 50, 0,250, 90, 0,250, 130,
      4, 211, 211, 4, 215, 115, 4, 230,230, 4,198, 210,
      15, 6, 15 },
{ 18, 3, 10, 10, 3, 50, 10, 3, 90, 10, 3, 130, 10, 3, 170, 10,
    2, 30, 50, 2, 70 , 50, 2, 110, 50, 2, 150, 50,
        4,10,90,1, 50, 90, 1, 90, 90, 1, 130, 90,
            0, 70, 130, 0, 110, 130,4,150,110,
                0, 90, 170, 4, 120, 200,
    240, 255, 0 },
{ 15, 0, 50, 50, 0, 100, 50, 0, 150, 50, 0, 200, 50, 0, 250, 50,
    2, 50, 100, 2, 150, 100, 2, 250, 100,
    3, 100, 150, 3, 200, 150,
    0, 50, 200, 1, 100, 200, 0, 150, 200, 1, 200, 200, 0, 250 ,200,
    255, 249, 15 },
{ 20, 1, 10, 20, 1, 50, 20, 1, 90, 20, 1, 130, 20, 1, 170, 20,
    1, 30, 50, 1, 70, 50, 1, 110, 50, 1, 150, 50, 1, 190, 50,
    2, 10, 80, 2, 50, 80, 2, 90, 80, 2, 130, 80, 2, 170, 80,
    2, 30, 110, 2, 70, 110, 2, 110, 110, 2, 150, 110, 2, 190, 110,
    240, 240, 0 },

{ 40, 0, 10, 20, 0, 50, 20, 0, 90, 20, 0, 130, 20, 0, 170, 20,
    0, 30, 50, 0, 70, 50, 0, 110, 50, 0, 150, 50, 0, 190, 50,
    1, 10, 80, 1, 50, 80, 1, 90, 80, 1, 130, 80, 1, 170, 80,
    1, 30, 110, 1, 70, 110, 1, 110, 110, 1, 150, 110, 1, 190, 110,
    2, 10, 140, 2, 50, 140, 2, 90, 140, 2, 130, 140, 2, 170, 140,
    0, 30, 170, 0, 70, 170, 0, 110, 170, 0, 150, 170, 0, 190, 170,
    3, 10, 200, 3, 50, 200, 3, 90, 200, 3, 130, 200, 3, 170, 200,
    0, 30, 230, 0, 70, 230, 0, 110, 230, 0, 150, 230, 0, 190, 230,
    15, 15, 15 },
{ 64, 0, 5, 5, 0, 40, 5, 0, 75, 5, 0, 110, 5, 0, 145, 5, 0, 180, 5, 0, 215, 5, 0, 250, 5,
    0, 5, 40, 0, 40, 40, 0, 75, 40, 0, 110, 40, 0, 145, 40, 0, 180, 40, 0, 215, 40, 0, 250, 40,
    0, 5, 75, 1, 40, 75, 2, 75, 75, 2, 110, 75, 2, 145, 75, 2, 180, 75, 1, 215, 75, 0, 250, 75,
    0, 5, 110, 1, 40, 110, 2, 75, 110, 0, 110, 110, 3, 145, 110, 2, 180, 110, 1, 215, 110, 0, 250, 110,
    0, 5, 145, 1, 40, 145, 2, 75, 145, 3, 110, 145, 0, 145, 145, 2, 180, 145, 1, 215, 145, 0, 250, 145,
    0, 5, 180, 1, 40, 180, 2, 75, 180, 2, 110, 180, 2, 145, 180, 2, 180, 180, 1, 215, 180, 0, 250, 180,
    0, 5, 215, 0, 40, 215, 0, 75, 215, 0, 110, 215, 0, 145, 215, 0, 180, 215, 0, 215, 215, 0, 250, 215,
    0, 5, 250, 0, 40, 250, 0, 75, 250, 0, 110, 250, 0, 145, 250, 0, 180, 250, 0, 215, 250, 0, 250, 250,
    255, 255, 255 },
};
#endif
