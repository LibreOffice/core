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


#ifndef _ERRHDL_HXX
#define _ERRHDL_HXX

#ifdef DBG_UTIL

#include <tools/solar.h>
#include <sal/types.h>
#include "swdllapi.h"

extern sal_Bool bAssert;                // sal_True, wenn eine ASSERT-Box hochkam


// -----------------------------------------------------------------------
// Ausgabe einer Fehlermeldung inkl. Dateiname und Zeilennummer
// wo der Fehler auftrat.
// Die Funktion darf nicht direkt benutzt werden!
// -----------------------------------------------------------------------
SW_DLLPUBLIC void AssertFail( const sal_Char*, const sal_Char*, sal_uInt16 );
SW_DLLPUBLIC void AssertFail( sal_uInt16, const sal_Char*, sal_uInt16 );

#define ASSERT( cond, message ) \
    if( !(cond) ) { \
        const char   *_pErrorText = #message; \
        const char   *_pFileName  = __FILE__; \
       ::AssertFail( _pErrorText, _pFileName, __LINE__ ); \
    }

// -----------------------------------------------------------------------
// Prueft ob die angegebene Bedingung wahr ist, wenn nicht wird eine
// Fehlermeldung die ueber die ID Identifiziert wird, ausgegeben.
// -----------------------------------------------------------------------
#define ASSERT_ID( cond, id ) \
    if( !(cond) ) { \
        const char   *_pFileName  = __FILE__; \
       ::AssertFail( (sal_uInt16)id, _pFileName, __LINE__ ); \
    }


// -----------------------------------------------------------------------
// Beim Bilden der Produktversion werden alle Debug-Utilities automatisch
// ignoriert
// -----------------------------------------------------------------------
#else
#define ASSERT( cond, message )     ;
#define ASSERT_ID( cond, id )       ;
#endif // PRODUCT



#endif
