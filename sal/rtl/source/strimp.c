/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "strimp.h"

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix )
{
    sal_Int16 n = -1;
    if ( (ch >= '0') && (ch <= '9') )
        n = ch-'0';
    else if ( (ch >= 'a') && (ch <= 'z') )
        n = ch-'a'+10;
    else if ( (ch >= 'A') && (ch <= 'Z') )
        n = ch-'A'+10;
    return (n < nRadix) ? n : -1;
}

sal_Bool rtl_ImplIsWhitespace( sal_Unicode c )
{
    /* Space or Control character? */
    if ( (c <= 32) && c )
        return sal_True;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ( (c < 0x2000) || (c > 0x206F) )
        return sal_False;

    if ( ((c >= 0x2000) && (c <= 0x200B)) ||    /* All Spaces           */
         (c == 0x2028) ||                       /* LINE SEPARATOR       */
         (c == 0x2029) )                        /* PARAGRAPH SEPARATOR  */
        return sal_True;

    return sal_False;
}
