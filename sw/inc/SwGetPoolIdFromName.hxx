/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwGetPoolIdFromName.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:29:21 $
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
#ifndef _GETPOOLIDFROMNAMEENUM_HXX
#define _GETPOOLIDFROMNAMEENUM_HXX

/* When using the NameMapper to translate pool ids to UI or programmatic
 * names, this enum is used to define which family is required */

enum SwGetPoolIdFromName
{
    GET_POOLID_TXTCOLL  = 0x01,
    GET_POOLID_CHRFMT   = 0x02,
    GET_POOLID_FRMFMT   = 0x04,
    GET_POOLID_PAGEDESC = 0x08,
    GET_POOLID_NUMRULE  = 0x10
};

#endif
