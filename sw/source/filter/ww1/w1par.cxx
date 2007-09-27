/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: w1par.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:58:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>           // Ww1Reader
#endif
#ifndef _W1PAR_HXX
#include <w1par.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // StatLine...()
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>            // ERR_WW1_...
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif

//----------------------------------------
//    Initialisieren der Feld-FilterFlags
//----------------------------------------

static ULONG WW1_Read_FieldIniFlags()
{
//  USHORT i;
    static const sal_Char* aNames[ 1 ] = { "WinWord/WW1F" };
    sal_uInt32 aVal[ 1 ];
    SwFilterOptions aOpt( 1, aNames, aVal );
    ULONG nFieldFlags = aVal[ 0 ];

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

ULONG WW1Reader::Read(SwDoc& rDoc, const String& rBaseURL, SwPaM& rPam, const String& /*cName*/)
{
    ULONG nRet = ERR_SWG_READ_ERROR;
    ASSERT(pStrm!=NULL, "W1-Read ohne Stream");
    if (pStrm != NULL)
    {
        BOOL bNew = !bInsertMode;           // Neues Doc ( kein Einfuegen )

        // erstmal eine shell konstruieren: die ist schnittstelle
        // zum writer-dokument
        ULONG nFieldFlags = WW1_Read_FieldIniFlags();
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
Ww1Shell::Ww1Shell( SwDoc& rD, SwPaM& rPam, const String& rBaseURL, BOOL bNew, ULONG nFieldFlags)
    : SwFltShell(&rD, rPam, rBaseURL, bNew, nFieldFlags)
{
}


