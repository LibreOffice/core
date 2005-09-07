/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datecode.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:29:47 $
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

#ifndef __DATECODE_H__
#define __DATACODE_H__

static hchar defaultform[] =
{
    '1', 0x9165, 32, '2', 0xB6A9, 32, '3', 0xB7A9, 0
};
static hchar kor_week[] =
{
    0xB7A9, 0xB6A9, 0xD1C1, 0xAE81, 0xA1A2, 0x8B71, 0xC9A1
};
static hchar china_week[] =
{
    0x4CC8, 0x4BE4, 0x525A, 0x48D8, 0x45AB, 0x4270, 0x50B4
};
static char eng_week[] = { "SunMonTueWedThuFriSat" };
static char eng_mon[] = { "JanFebMarAprMayJunJulAugSepOctNovDec" };
static const char *en_mon[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};
static const char *en_week[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};
#endif
