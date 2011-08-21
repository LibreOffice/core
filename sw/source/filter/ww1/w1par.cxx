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


#include <pam.hxx>              // fuer SwPam
#include <doc.hxx>
#include <ndtxt.hxx>            // class SwTxtNode
#include <fltini.hxx>           // Ww1Reader
#include <w1par.hxx>
#include <swfltopt.hxx>
#include <mdiexp.hxx>           // StatLine...()
#include <swerror.h>            // ERR_WW1_...
#include <statstr.hrc>          // ResId fuer Statusleiste

//----------------------------------------
//    Initialisieren der Feld-FilterFlags
//----------------------------------------

static sal_uLong WW1_Read_FieldIniFlags()
{
//  sal_uInt16 i;
    static const sal_Char* aNames[ 1 ] = { "WinWord/WW1F" };
    sal_uInt32 aVal[ 1 ];
    SwFilterOptions aOpt( 1, aNames, aVal );
    sal_uLong nFieldFlags = aVal[ 0 ];

    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwFltSetFlag( nFieldFlags, SwFltControlStack::BOOK_TO_VAR_REF );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
    }
    return nFieldFlags;
}

////////////////////////////////////////////////// StarWriter-Interface
//
// Eine Methode liefern die call-Schnittstelle fuer den Writer.
// Read() liest eine Datei. hierzu werden zwei Objekte erzeugt, die Shell,
// die die Informationen aufnimmt und der Manager der sie aus der Datei liest.
// Diese werden dann einfach per Pipe 'uebertragen'.
//

sal_uLong WW1Reader::Read(SwDoc& rDoc, const String& rBaseURL, SwPaM& rPam, const String& /*cName*/)
{
    sal_uLong nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE(pStrm!=NULL, "W1-Read ohne Stream");
    if (pStrm != NULL)
    {
        sal_Bool bNew = !bInsertMode;           // Neues Doc ( kein Einfuegen )

        // erstmal eine shell konstruieren: die ist schnittstelle
        // zum writer-dokument
        sal_uLong nFieldFlags = WW1_Read_FieldIniFlags();
        Ww1Shell* pRdr = new Ww1Shell( rDoc, rPam, rBaseURL, bNew, nFieldFlags );
        if( pRdr )
        {
            // dann den manager, der liest die struktur des word-streams
            Ww1Manager* pMan = new Ww1Manager( *pStrm, nFieldFlags );
            if( pMan )
            {
                if( !pMan->GetError() )
                {
                    ::StartProgress( STR_STATSTR_W4WREAD, 0, 100,
                                        rDoc.GetDocShell() );
                    ::SetProgressState( 0, rDoc.GetDocShell() );
                    // jetzt nur noch alles rueberschieben
                    *pRdr << *pMan;
                    if( !pMan->GetError() )
                        // und nur hier, wenn kein fehler auftrat
                        // fehlerfreiheit melden
                        nRet = 0; // besser waere: WARN_SWG_FEATURES_LOST;
                    ::EndProgress( rDoc.GetDocShell() );
                }
                else
                {
                    if( pMan->GetFib().GetFIB().fComplexGet() )
                        //!!! ACHTUNG: hier muss eigentlich ein Error
                        // wegen Fastsave kommen, das der PMW-Filter
                        // das nicht unterstuetzt. Stattdessen temporaer
                        // nur eine Warnung, bis die entsprechende
                        // Meldung und Behandlung weiter oben eingebaut ist.
//                      nRet = WARN_WW6_FASTSAVE_ERR;
                        // Zum Einchecken mit neuem String:
                        nRet = ERR_WW6_FASTSAVE_ERR;
                }
            }
            delete pMan;
        }
        delete pRdr;
    }
    Ww1Sprm::DeinitTab();
    return nRet;
}

///////////////////////////////////////////////////////////////// Shell
//
// Die Shell ist die Schnittstelle vom Filter zum Writer. Sie ist
// abgeleitet von der mit ww-filter gemeinsam benutzten Shell
// SwFltShell und enthaelt alle fuer ww1 noetigen Erweiterungen. Wie
// in einen Stream werden alle Informationen, die aus der Datei
// gelesen werden, in die shell ge'piped'.
//
Ww1Shell::Ww1Shell( SwDoc& rD, SwPaM& rPam, const String& rBaseURL, sal_Bool bNew, sal_uLong nFieldFlags)
    : SwFltShell(&rD, rPam, rBaseURL, bNew, nFieldFlags)
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
