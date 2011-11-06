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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <rtl/ustring.hxx>

int _cdecl main( int argc, char * argv[] )
{
    rtl::OUString lang = rtl::OUString::createFromAscii( "de" );
    rtl::OUString country = rtl::OUString::createFromAscii( "DE" );
    rtl_TextEncoding rtlTextEnc;

    rtl_Locale* rtlLocale =
        rtl_locale_register( lang.getStr( ), country.getStr( ), NULL );

    if ( rtlLocale )
        rtlTextEnc = osl_getTextEncodingFromLocale( rtlLocale );

    return(0);
}


