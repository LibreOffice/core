/*************************************************************************
 *
 *  $RCSfile: ddelink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 20:47:36 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#include <svx/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/zforlist.hxx>

#include "ddelink.hxx"
#include "brdcst.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "sc.hrc"
#include "hints.hxx"

TYPEINIT2(ScDdeLink,::so3::SvBaseLink,SfxBroadcaster);

#ifdef PM2
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_IBM_850
#else
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_MS_1252
#endif

BOOL ScDdeLink::bIsInUpdate = FALSE;

//------------------------------------------------------------------------

ScDdeLink::ScDdeLink( ScDocument* pD, const String& rA, const String& rT, const String& rI,
                        BYTE nM ) :
    ::so3::SvBaseLink(LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    aAppl( rA ),
    aTopic( rT ),
    aItem( rI ),
    nMode( nM ),
    pResult( NULL ),
    bNeedUpdate( FALSE )
{
}

__EXPORT ScDdeLink::~ScDdeLink()
{
    // Verbindung aufheben

    delete pResult;
}

ScDdeLink::ScDdeLink( ScDocument* pD, const ScDdeLink& rOther ) :
    ::so3::SvBaseLink(LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc    ( pD ),
    aAppl   ( rOther.aAppl ),
    aTopic  ( rOther.aTopic ),
    aItem   ( rOther.aItem ),
    nMode   ( rOther.nMode ),
    pResult ( NULL ),
    bNeedUpdate( FALSE )
{
    if (rOther.pResult)
        pResult = rOther.pResult->Clone();
}

ScDdeLink::ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
    ::so3::SvBaseLink(LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    pResult( NULL ),
    bNeedUpdate( FALSE )
{
    rHdr.StartEntry();

    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    rStream.ReadByteString( aAppl, eCharSet );
    rStream.ReadByteString( aTopic, eCharSet );
    rStream.ReadByteString( aItem, eCharSet );

    BOOL bHasValue;
    rStream >> bHasValue;
    if ( bHasValue )
        pResult = new ScMatrix( rStream );

    if (rHdr.BytesLeft())       // neu in 388b und der 364w (RealTime-Client) Version
        rStream >> nMode;
    else
        nMode = SC_DDE_DEFAULT;

    rHdr.EndEntry();
}

void ScDdeLink::Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    rHdr.StartEntry();

    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    rStream.WriteByteString( aAppl, eCharSet );
    rStream.WriteByteString( aTopic, eCharSet );
    rStream.WriteByteString( aItem, eCharSet );

    BOOL bHasValue = ( pResult != NULL );
    rStream << bHasValue;
    if (bHasValue)
        pResult->Store( rStream );

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )      // nicht bei 4.0 Export
        rStream << nMode;                                   // seit 388b

    //  Links mit Mode != SC_DDE_DEFAULT werden bei 4.0 Export komplett weggelassen
    //  (aus ScDocument::SaveDdeLinks)

    rHdr.EndEntry();
}

void __EXPORT ScDdeLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
{
    //  wir koennen nur Strings...
    if ( FORMAT_STRING != SotExchange::GetFormatIdFromMimeType( rMimeType ))
        return;

    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    rValue >>= aSeq;
    String aLinkStr( (sal_Char*)aSeq.getConstArray(), aSeq.getLength(),
                          DDE_TXT_ENCODING   );
    aLinkStr.ConvertLineEnd(LINEEND_LF);

    //  wenn String mit Zeilenende aufhoert, streichen:

    xub_StrLen nLen = aLinkStr.Len();
    if (nLen && aLinkStr.GetChar(nLen-1) == '\n')
        aLinkStr.Erase(nLen-1);

    String aLine;
    USHORT nCols = 1;       // Leerstring -> eine leere Zelle
    USHORT nRows = 1;
    if (aLinkStr.Len())
    {
        nRows = (USHORT) aLinkStr.GetTokenCount( '\n' );
        aLine = aLinkStr.GetToken( 0, '\n' );
        if (aLine.Len())
            nCols = (USHORT) aLine.GetTokenCount( '\t' );
    }

    if (!nRows || !nCols)               // keine Daten
    {
        DELETEZ(pResult);
    }
    else                                // Daten aufteilen
    {
        //  Matrix immer neu anlegen, damit bIsString nicht durcheinanderkommt
        delete pResult;
        pResult = new ScMatrix( nCols, nRows );

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

        //  nMode bestimmt, wie der Text interpretiert wird (#44455#/#49783#):
        //  SC_DDE_DEFAULT - Zahlformat aus Zellvorlage "Standard"
        //  SC_DDE_ENGLISH - Standard-Zahlformat fuer English/US
        //  SC_DDE_TEXT    - ohne NumberFormatter direkt als String
        ULONG nStdFormat = 0;
        if ( nMode == SC_DDE_DEFAULT )
        {
            ScPatternAttr* pDefPattern = pDoc->GetDefPattern();     // enthaelt Standard-Vorlage
            if ( pDefPattern )
                nStdFormat = pDefPattern->GetNumberFormat( pFormatter );
        }
        else if ( nMode == SC_DDE_ENGLISH )
            nStdFormat = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);

        String aEntry;
        for (USHORT nR=0; nR<nRows; nR++)
        {
            aLine = aLinkStr.GetToken( (xub_StrLen) nR, '\n' );
            for (USHORT nC=0; nC<nCols; nC++)
            {
                aEntry = aLine.GetToken( (xub_StrLen) nC, '\t' );
                ULONG nIndex = nStdFormat;
                double fVal;
                if ( nMode != SC_DDE_TEXT && pFormatter->IsNumberFormat( aEntry, nIndex, fVal ) )
                    pResult->PutDouble( fVal, nC, nR );
                else
                    pResult->PutString( aEntry, nC, nR );
            }
        }
    }

    //  Es hat sich was getan...

    if (HasListeners())
    {
        Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress( 0 ), NULL ) );
        pDoc->TrackFormulas();      // muss sofort passieren
        pDoc->StartTrackTimer();

        //  StartTrackTimer ruft asynchron TrackFormulas, Broadcast(FID_DATACHANGED),
        //  ResetChanged, SetModified und Invalidate(SID_SAVEDOC/SID_DOC_MODIFIED)
        //  TrackFormulas zusaetzlich nochmal sofort, damit nicht z.B. durch IdleCalc
        //  eine Formel berechnet wird, die noch im FormulaTrack steht (#61676#)

        //  notify Uno objects (for XRefreshListener)
        //  must be after TrackFormulas
        //! do this asynchronously?
        ScLinkRefreshedHint aHint;
        aHint.SetDdeLink( aAppl, aTopic, aItem, nMode );
        pDoc->BroadcastUno( aHint );
    }
}

void ScDdeLink::NewData(USHORT nCols, USHORT nRows)
{
    delete pResult;
    pResult = new ScMatrix( nCols, nRows );
}

void ScDdeLink::ResetValue()
{
    DELETEZ(pResult);

    //  Es hat sich was getan...
    //  Tracking, FID_DATACHANGED etc. passiert von aussen

    if (HasListeners())
        Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress( 0 ), NULL ) );
}

void __EXPORT ScDdeLink::ListenersGone()
{
    BOOL bWas = bIsInUpdate;
    bIsInUpdate = TRUE;             // Remove() kann Reschedule ausloesen??!?

    ScDocument* pStackDoc = pDoc;   // member pDoc can't be used after removing the link

    SvxLinkManager* pLinkMgr = pDoc->GetLinkManager();
    pLinkMgr->Remove( this);        // deletes this

    if ( !pLinkMgr->GetLinks().Count() )            // letzten geloescht ?
    {
        SfxBindings* pBindings = pStackDoc->GetViewBindings();      // don't use member pDoc!
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }

    bIsInUpdate = bWas;
}

void ScDdeLink::TryUpdate()
{
    if (bIsInUpdate)
        bNeedUpdate = TRUE;         // kann jetzt nicht ausgefuehrt werden
    else
    {
        bIsInUpdate = TRUE;
        //Application::Reschedule();    //! OS/2-Simulation
        Update();
        bIsInUpdate = FALSE;
        bNeedUpdate = FALSE;
    }
}


