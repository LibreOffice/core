/*************************************************************************
#
#   $RCSfile: rtl_String_Utils.hxx,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2003-04-01 13:25:58 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************/

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _RTL_STRING_UTILS_HXX_
#define _RTL_STRING_UTILS_HXX_

#ifdef __cplusplus

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#include <math.h>
#include <stdlib.h>

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H_
    #include <rtl/ustring.h>
#endif

#ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
sal_Char* cpystr( sal_Char* dst, const sal_Char* src );
sal_Char* cpynstr( sal_Char* dst, const sal_Char* src, sal_uInt32 cnt );

sal_Bool cmpstr( const sal_Char* str1, const sal_Char* str2, sal_uInt32 len );
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2, sal_uInt32 len );
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2 );

sal_Char* createName( sal_Char* dst, const sal_Char* src, sal_uInt32 cnt );
void makeComment(char *com, const char *str1, const char *str2, sal_Int32 sgn);


sal_uInt32 AStringLen( const sal_Char *pAStr );

sal_uInt32 UStringLen( const sal_Unicode *pUStr );

//------------------------------------------------------------------------

sal_Bool AStringToFloatCompare ( const sal_Char  *pStr,
                                 const float      nX,
                                 const float      nEPS
                                );

sal_Bool AStringToDoubleCompare ( const sal_Char  *pStr,
                                  const double     nX,
                                  const double     nEPS
                                );

//------------------------------------------------------------------------

sal_Bool AStringIsValid( const sal_Char  *pAStr );

sal_Bool AStringNIsValid( const sal_Char   *pAStr,
                          const sal_uInt32  nStrLen
                        );

//------------------------------------------------------------------------

sal_Int32 AStringToUStringCompare( const sal_Unicode *pUStr,
                                   const sal_Char    *pAStr
                                 );

sal_Int32 AStringToUStringNCompare( const sal_Unicode  *pUStr,
                                    const sal_Char     *pAStr,
                                    const sal_uInt32    nAStrCount
                                   );

sal_Int32 AStringToRTLUStringCompare( const rtl_uString  *pRTLUStr,
                                      const sal_Char     *pAStr
                                    );

sal_Int32 AStringToRTLUStringNCompare( const rtl_uString  *pRTLUStr,
                                       const sal_Char     *pAStr,
                                       const sal_uInt32    nAStrCount
                                     );

//------------------------------------------------------------------------

sal_Bool AStringToUStringCopy( sal_Unicode     *pDest,
                               const sal_Char  *pSrc
                             );

sal_Bool AStringToUStringNCopy( sal_Unicode       *pDest,
                                const sal_Char    *pSrc,
                                const sal_uInt32   nSrcLen
                              );

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* _RTL_STRING_UTILS_HXX */







