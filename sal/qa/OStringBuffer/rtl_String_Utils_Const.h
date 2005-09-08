/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtl_String_Utils_Const.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:17:26 $
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _RTL_STRING_UTILS_CONST_H_
#define _RTL_STRING_UTILS_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int32 kErrCompareAStringToUString     = -2;
static const sal_Int32 kErrCompareNAStringToUString    = -3;
static const sal_Int32 kErrCompareAStringToRTLUString  = -4;
static const sal_Int32 kErrCompareNAStringToRTLUString = -5;
static const sal_Int32 kErrAStringToByteStringCompare  = -6;
static const sal_Int32 kErrAStringToByteStringNCompare = -7;
static const sal_Int32 kErrCompareAStringToString      = -8;
static const sal_Int32 kErrCompareNAStringToString     = -9;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif /* _RTL_STRING_UTILS_CONST_H_ */



