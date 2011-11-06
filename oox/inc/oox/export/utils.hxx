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



#ifndef _OOX_EXPORT_UTILS_HXX_
#define _OOX_EXPORT_UTILS_HXX_

#define S(x) String( RTL_CONSTASCII_USTRINGPARAM( x ) )
#define US(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ))
#define I32S(x) OString::valueOf( (sal_Int32) x ).getStr()
#define I64S(x) OString::valueOf( (sal_Int64) x ).getStr()
#define H32S(x) OString::valueOf( (sal_Int32) x, 16 ).getStr()
#define H64S(x) OString::valueOf( (sal_Int64) x, 16 ).getStr()
#define IS(x) OString::valueOf( x ).getStr()
#define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()
#define ST(x) ByteString( x, RTL_TEXTENCODING_UTF8 ).GetBuffer()

#ifndef DBG
#  if OSL_DEBUG_LEVEL > 0
#    define DBG(x) x
#  else
#    define DBG(x)
#  endif
#endif

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 PPTtoEMU( INT32 nPPT )
{
    return (sal_Int64)( (double)nPPT * 1587.5 );
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 MM100toEMU( INT32 nMM100 )
{
    return (sal_Int64)nMM100 * 360;
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 TwipsToEMU( sal_Int32 nTwips )
{
    return sal_Int64( nTwips ) * 635;
}

#endif
