/*************************************************************************
 *
 *  $RCSfile: errhdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#define _ERRHDL_CXX

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "stdlib.h"
#ifdef WIN
#include <svwin.h>              // fuer die Goodies der Windows User
#include <dos.h>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWERROR_H
#include <error.h>              // fuer die defines von ERR_SW6MSG_ ...
#endif

// break into CodeView
#if defined(ZTC) && defined(WIN)
#define CVBREAK     asm( 0xCC );
#endif
#if defined(MSC) && defined(WIN)
#define CVBREAK     __asm int 3;
#endif
#ifndef CVBREAK
#define CVBREAK
#endif

BOOL bAssertFail = FALSE;           // ist gerade eine Assertbox oben ?
BOOL bAssert = FALSE;               // TRUE, wenn mal ein ASSERT kam.

/*------------------------------------------------------------------------
    Ausgabe einer Fehlermeldung inkl. Bedingung, Dateiname und Zeilennummer
    wo der Fehler auftrat.
    Die Funktion wird durch das ASSERT Makro gerufen!
    Parameter:
                char    *pError     Fehlermeldung
                char    *pFileName  Filename in dem der Fehler auftrat
                USHORT  nLine       Zeilennummer in dem der Fehler auftrat
------------------------------------------------------------------------*/

void AssertFail( const sal_Char* pError, const sal_Char* pFileName, USHORT nLine )
{
    CVBREAK;
    // NOTE4("ASSERT: %s at %d: %s\n", pFileName, nLine, pError);
    bAssert = TRUE;

    if( !bAssertFail && GetpApp() && GetpApp()->IsInMain() )
    {
        bAssertFail = TRUE;
        ByteString  aErr;
        aErr = "Assertion failed\n==================\nFILE      :  ";
        aErr += pFileName;
        aErr += " at line ";
        aErr += ByteString::CreateFromInt32( nLine );
        aErr += "\nERROR :  ";
        aErr += pError;

        ByteString aTmp( getenv( "SW_NOBEEP" ) );
        if ( aTmp != "TRUE" )
            Sound::Beep(SOUND_ERROR);

#if defined( UNX ) && !defined( DBG_UTIL )
        DBG_ERROR( aErr.GetBuffer() ); // DbgErr ist in UNIX-nicht Produkt-Versionen nicht definiert
#else
        DbgError( aErr.GetBuffer() );
#endif
        bAssertFail = FALSE;
    }
    else
    {
        Sound::Beep(SOUND_ERROR);
        Sound::Beep(SOUND_ERROR);
        Sound::Beep(SOUND_ERROR);
        if( !bAssertFail )
#if defined( MAC )
        if( !bAssertFail )
            *(short *)1 = 4711;         // odd address error erzeugen
#endif
        if( !bAssertFail )
            *(short *)0 = 4711;         // UAE ausloesen
    }
}

/*------------------------------------------------------------------------
    Ausgabe einer Fehlermeldung inkl. Bedingung, Dateiname und Zeilennummer
    wo der Fehler auftrat.
    Die Funktion wird durch das ASSERT Makro gerufen!
    Parameter:
                USHORT  nErrorId    Id fuer Fehlermeldung
                char    *pFileName  Filename in dem der Fehler auftrat
                USHORT  nLine       Zeilennummer in dem der Fehler auftrat
------------------------------------------------------------------------*/

void AssertFail( USHORT nErrorId, const sal_Char* pFileName, USHORT nLine )
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


