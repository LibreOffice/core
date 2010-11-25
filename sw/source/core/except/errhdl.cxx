/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#define _ERRHDL_CXX

#include "stdlib.h"
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sound.hxx>
#include <errhdl.hxx>
#include <error.h>              // fuer die defines von ERR_SW6MSG_ ...

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

/*------------------------------------------------------------------------
    Ausgabe einer Fehlermeldung inkl. Bedingung, Dateiname und Zeilennummer
    wo der Fehler auftrat.
    Die Funktion wird durch das OSL_ENSURE(Makro gerufen!
    Parameter:
                USHORT  nErrorId    Id fuer Fehlermeldung
                char    *pFileName  Filename in dem der Fehler auftrat
                USHORT  nLine       Zeilennummer in dem der Fehler auftrat
------------------------------------------------------------------------*/

void AssertFail( USHORT nErrorId, const sal_Char* pFileName )
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

    OSL_ENSURE( pMsg, pFileName );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
