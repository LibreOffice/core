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
#include "precompiled_connectivity.hxx"

#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "LDebug.hxx"
#endif
#include <osl/diagnose.h>

void evo_traceStringMessage( const sal_Char* _pFormat, const ::rtl::OUString& _rAsciiString )
{
    ::rtl::OString sByteStringMessage( _rAsciiString.getStr(), _rAsciiString.getLength(), RTL_TEXTENCODING_ASCII_US );
    if ( !sByteStringMessage.getLength() )
        sByteStringMessage = "<empty>";
    OSL_TRACE( _pFormat, sByteStringMessage.getStr() );
}
