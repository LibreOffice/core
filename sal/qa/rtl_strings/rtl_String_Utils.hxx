/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
#
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
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







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
