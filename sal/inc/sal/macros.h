/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macros.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:47:02 $
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

#ifndef _SAL_MACROS_H_
#define _SAL_MACROS_H_

#ifndef SAL_MAX
#   define SAL_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef SAL_MIN
#   define SAL_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef SAL_FIELDOFFSET
#   define SAL_FIELDOFFSET(type, field) ((sal_Int32)(&((type *)16)->field) - 16)
#endif

#ifndef SAL_BOUND
#   define SAL_BOUND(x,l,h)        ((x) <= (l) ? (l) : ((x) >= (h) ? (h) : (x)))
#endif

#ifndef SAL_SWAP
#   define SAL_SWAP(a,b)           ((a) ^= (b) ^= (a) ^= (b))
#endif


#endif
