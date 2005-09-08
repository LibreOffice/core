/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optab.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:23:32 $
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

#ifndef _OPTAB_H
#define _OPTAB_H

typedef void ( *OPCODE_FKT )( SvStream &aStream, USHORT nLaenge );

#define FKT_LIMIT   101

#define FKT_LIMIT123    101

#define LOTUS_EOF   0x01

#define LOTUS_FILEPASSWD 0x4b

#define LOTUS_PATTERN   0x284

#define LOTUS_FORMAT_INDEX 0x800

#define LOTUS_FORMAT_INFO 0x801

#define ROW_FORMAT_MARKER 0x106

#define COL_FORMAT_MARKER 0x107

#endif

