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


#ifndef _SOLTOOLS_TESTSHL_TUTIL_HXX_
#define _SOLTOOLS_TESTSHL_TUTIL_HXX__

#include    <osl/file.hxx>

using namespace std;

#include <vector>

// <namespace_tstutl>
namespace tstutl {

sal_uInt32 getEntriesFromFile( sal_Char* fName, vector< sal_Char* >& entries );
::rtl::OUString cnvrtPth( ::rtl::OString sysPth );

// string copy, cat, len methods
sal_Char* cpy( sal_Char** dest, const sal_Char* src );
sal_Char* cat( const sal_Char* str1, const sal_Char* str2 );
sal_uInt32 ln( const sal_Char* str );

} // </namespace_tstutl>

#endif
