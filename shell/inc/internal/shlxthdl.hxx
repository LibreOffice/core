/*************************************************************************
 *
 *  $RCSfile: shlxthdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:54:09 $
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

#endif
