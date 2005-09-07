/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shlxthdl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:38:56 $
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

#ifndef SHLXTHDL_HXX_INCLUDED
#define SHLXTHDL_HXX_INCLUDED

#include <objbase.h>

// {087B3AE3-E237-4467-B8DB-5A38AB959AC9}
const CLSID CLSID_INFOTIP_HANDLER =
{0x87b3ae3, 0xe237, 0x4467, {0xb8, 0xdb, 0x5a, 0x38, 0xab, 0x95, 0x9a, 0xc9}};

// {C52AF81D-F7A0-4aab-8E87-F80A60CCD396}
const CLSID CLSID_COLUMN_HANDLER =
{ 0xc52af81d, 0xf7a0, 0x4aab, { 0x8e, 0x87, 0xf8, 0xa, 0x60, 0xcc, 0xd3, 0x96 } };

// {63542C48-9552-494a-84F7-73AA6A7C99C1}
const CLSID CLSID_PROPERTYSHEET_HANDLER =
{ 0x63542c48, 0x9552, 0x494a, { 0x84, 0xf7, 0x73, 0xaa, 0x6a, 0x7c, 0x99, 0xc1 } };

// {3B092F0C-7696-40e3-A80F-68D74DA84210}
const CLSID CLSID_THUMBVIEWER_HANDLER =
{ 0x3b092f0c, 0x7696, 0x40e3, { 0xa8, 0xf, 0x68, 0xd7, 0x4d, 0xa8, 0x42, 0x10 } };

#endif
