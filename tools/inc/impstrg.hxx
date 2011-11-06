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



#ifndef _IMPSTRG_HXX
#define _IMPSTRG_HXX

#include <tools/tools.h>
#include <tools/string.hxx>

// ------------------------------
// - Debug-Methoden fuer String -
// ------------------------------

const char* DbgCheckByteString( const void* pString );
const char* DbgCheckUniString( const void* pString );

// -------------------------------
// - Hilfsfunktionen fuer String -
// -------------------------------

xub_StrLen ImplStringLen( const sal_Char* pStr );
xub_StrLen ImplStringLen( const sal_Unicode* pStr );

// ------------------------------------
// - Zugriff fuer International class -
// ------------------------------------

sal_Unicode* ImplGet1ByteUnicodeTab( rtl_TextEncoding eTextEncoding );

#endif // _IMPSTRG_HXX
