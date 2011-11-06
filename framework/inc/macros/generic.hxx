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



#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#define __FRAMEWORK_MACROS_GENERIC_HXX_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <rtl/ustring.hxx>
#include <rtl/textenc.h>

//*****************************************************************************************************************
//  generic macros
//*****************************************************************************************************************

/*_________________________________________________________________________________________________________________
    DECLARE_ASCII( SASCIIVALUE )

    Use it to declare a constant ascii value at compile time in code.
    zB. OUSting sTest = DECLARE_ASCII( "Test" )
_________________________________________________________________________________________________________________*/

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

/*_________________________________________________________________________________________________________________
    U2B( SUNICODEVALUE )
    B2U( SASCIIVALUE )
    U2B_ENC( SUNICODEVALUE, AENCODING )
    B2U_ENC( SASCIIVALUE, AENCODING )

    Use it to convert unicode strings to ascii values and reverse ...
    We use UTF8 as default textencoding. If you will change this use U2B_ENC and B2U_ENC!
_________________________________________________________________________________________________________________*/

#define U2B( SUNICODEVALUE )                                                                                    \
    ::rtl::OUStringToOString( SUNICODEVALUE, RTL_TEXTENCODING_UTF8 )

#define B2U( SASCIIVALUE )                                                                                      \
    ::rtl::OStringToOUString( SASCIIVALUE, RTL_TEXTENCODING_UTF8 )

#define U2B_ENC( SUNICODEVALUE, AENCODING )                                                                     \
    ::rtl::OUStringToOString( SUNICODEVALUE, AENCODING )

#define B2U_ENC( SASCIIVALUE, AENCODING )                                                                       \
    ::rtl::OStringToOUString( SASCIIVALUE, AENCODING )

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  //  #ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
