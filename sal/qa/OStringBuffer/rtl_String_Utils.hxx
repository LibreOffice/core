/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _RTL_STRING_UTILS_HXX_
#define _RTL_STRING_UTILS_HXX_

#ifdef __cplusplus

#include <math.h>
#include <stdlib.h>

#include <sal/types.h>
#include <rtl/ustring.h>
#include <rtl/string.hxx>

sal_Char* cpystr( sal_Char* dst, const sal_Char* src );
sal_Char* cpynstr( sal_Char* dst, const sal_Char* src, sal_uInt32 cnt );

sal_Bool cmpstr( const sal_Char* str1, const sal_Char* str2, sal_uInt32 len );
sal_Bool cmpstr( const sal_Char* str1, const sal_Char* str2 );
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2, sal_uInt32 len );
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2 );

sal_Char* createName( sal_Char* dst, const sal_Char* src, sal_uInt32 cnt );


sal_uInt32 AStringLen( const sal_Char *pAStr );

//------------------------------------------------------------------------

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

#endif /* __cplusplus */

#endif /* _RTL_STRING_UTILS_HXX */







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
