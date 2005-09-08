/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flttypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:19:03 $
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

#ifndef _FLTTYPES_HXX
#define _FLTTYPES_HXX

enum BiffTyp
{
    BiffX = 0x0000,
    Biff2 = 0x2000, Biff2M = 0x2002, Biff2C = 0x2004,
    Biff3 = 0x3000, Biff3W = 0x3001, Biff3M = 0x3002, Biff3C = 0x3004,
    Biff4 = 0x4000, Biff4W = 0x4001, Biff4M = 0x4002, Biff4C = 0x4004,
    Biff5 = 0x5000, Biff5W = 0x5001, Biff5V = 0x5002, Biff5C = 0x5004, Biff5M4 = 0x5008,
    Biff8 = 0x8000, Biff8W = 0x8001, Biff8V = 0x8002, Biff8C = 0x8004, Biff8M4 = 0x8008
};

enum Lotus123Typ
{
    Lotus_X,
    Lotus_WK1,
    Lotus_WK3,
    Lotus_WK4,
    Lotus_FM3
};

#endif

