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
#include "precompiled_sw.hxx"

#ifdef DBG_UTIL

#define _ERRHDL_CXX


#include "stdlib.h"
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sound.hxx>
#include <errhdl.hxx>
#include <error.h>              // fuer die defines von ERR_SW6MSG_ ...

#ifndef CVBREAK
#define CVBREAK
#endif

sal_Bool bAssertFail = sal_False;           // ist gerade eine Assertbox oben ?
sal_Bool bAssert = sal_False;               // sal_True, wenn mal ein ASSERT kam.

/*------------------------------------------------------------------------
    Ausgabe einer Fehlermeldung inkl. Bedingung, Dateiname und Zeilennummer
    wo der Fehler auftrat.
    Die Funktion wird durch das ASSERT Makro gerufen!
    Parameter:
                char    *pError     Fehlermeldung
                char    *pFileName  Filename in dem der Fehler auftrat
                sal_uInt16  nLine       Zeilennummer in dem der Fehler auftrat
------------------------------------------------------------------------*/

void AssertFail( const sal_Char* pError, const sal_Char* pFileName, sal_uInt16 nLine )
{
    CVBREAK;
    // NOTE4("ASSERT: %s at %d: %s\n", pFileName, nLine, pError);
    bAssert = sal_True;

    if( !bAssertFail && GetpApp() && GetpApp()->IsInMain() )
    {
        bAssertFail = sal_True;
        ByteString  aErr;
        aErr = "Assertion failed\n==================\nFILE      :  ";
        aErr += pFileName;
        aErr += " at line ";
        aErr += ByteString::CreateFromInt32( nLine );
        aErr += "\nERROR :  ";
        aErr += pError;

        ByteString aTmp( getenv( "SW_NOBEEP" ) );
        if ( aTmp != "sal_True" )
            Sound::Beep(SOUND_ERROR);

#if defined( UNX ) && !defined( DBG_UTIL )
        DBG_ERROR( aErr.GetBuffer() ); // DbgErr ist in UNIX-nicht Produkt-Versionen nicht definiert
#else
        DbgError( aErr.GetBuffer() );
#endif
        bAssertFail = sal_False;
    }
    else
    {
        Sound::Beep(SOUND_ERROR);
        Sound::Beep(SOUND_ERROR);
        Sound::Beep(SOUND_ERROR);
        if( !bAssertFail )
            *(short *)0 = 4711;         // UAE ausloesen
    }
}

/*------------------------------------------------------------------------
    Ausgabe einer Fehlermeldung inkl. Bedingung, Dateiname und Zeilennummer
    wo der Fehler auftrat.
    Die Funktion wird durch das ASSERT Makro gerufen!
    Parameter:
                sal_uInt16  nErrorId    Id fuer Fehlermeldung
                char    *pFileName  Filename in dem der Fehler auftrat
                sal_uInt16  nLine       Zeilennummer in dem der Fehler auftrat
------------------------------------------------------------------------*/

void AssertFail( sal_uInt16 nErrorId, const sal_Char* pFileName, sal_uInt16 nLine )
{
    // Umsetzung der ErrorId in eine Fehlermeldung
    static const sal_Char
        /* Error Fehlermeldungen zugriffe ausserhalb eines Bereiches */
        sERR_VAR_IDX[]      = "Op[]",
        sERR_OUTOFSCOPE[]   = "Zugriff ausserhalb des Bereiches",
        /* Error Codes fuer Numerierungsregeln */
        sERR_NUMLEVEL[]     = "Falscher Num-Level",
        /* Error Codes fuer TxtNode */
        sERR_NOHINTS[]      = "Zugriff auf ungueltiges HintsArray",
        sERR_UNKNOWN[]      = "???";

    static const sal_Char* aErrStrTab[ ERR_SWGMSG_END - ERR_SWGMSG_START +1 ] =
    {
        sERR_VAR_IDX, sERR_OUTOFSCOPE, sERR_NUMLEVEL, sERR_NOHINTS
    };

    const sal_Char* pMsg;
    if( nErrorId >= ERR_SWGMSG_START && nErrorId < ERR_SWGMSG_END )
        pMsg = aErrStrTab[ nErrorId - ERR_SWGMSG_START ];
    else
        pMsg = sERR_UNKNOWN;

    AssertFail( pMsg, pFileName, nLine );
}

#endif // DBG_UTIL

