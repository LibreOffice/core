/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: decl.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:15:08 $
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


#ifndef __DECL_H
#define __DECL_H

#include <tools/solar.h>
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

enum WKTYP { eWK_UNKNOWN = -2, eWK_1 = 0, eWK_2, eWK3, eWK4, eWK_Error, eWK123 };
typedef void ( BEARBFKT )( void );
typedef sal_Char STRING16[ 16 ];
typedef sal_Char STRING14[ 14 ];
typedef sal_Char STRING8[ 8 ];
typedef sal_Char STRING6[ 6 ];
typedef USHORT USHORT4[ 4 ];
//typedef unsigned short USHORT4[ 4 ];
#endif

