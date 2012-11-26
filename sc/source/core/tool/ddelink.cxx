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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include <tools/list.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>

#include "ddelink.hxx"
#include "brdcst.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "rangeseq.hxx"
#include "sc.hrc"
#include "hints.hxx"

#define DDE_TXT_ENCODING    gsl_getSystemTextEncoding()

sal_Bool ScDdeLink::bIsInUpdate = sal_False;

//------------------------------------------------------------------------

ScDdeLink::ScDdeLink( ScDocument* pD, const String& rA, const String& rT, const String& rI,
                        sal_uInt8 nM ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    aAppl( rA ),
    aTopic( rT ),
    aItem( rI ),
    nMode( nM ),
    bNeedUpdate( sal_False ),
    pResult( NULL )
{
}

__EXPORT ScDdeLink::~ScDdeLink()
{
    // Verbindung aufheben

    // pResult is refcounted
}

ScDdeLink::ScDdeLink( ScDocument* pD, const ScDdeLink& rOther ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc    ( pD ),
    aAppl   ( rOther.aAppl ),
    aTopic  ( rOther.aTopic ),
    aItem   ( rOther.aItem ),
    nMode   ( rOther.nMode ),
    bNeedUpdate( sal_False ),
    pResult ( NULL )
{
    if (rOther.pResult)
        pResult = rOther.pResult->Clone();
}

ScDdeLink::ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    bNeedUpdate( sal_False ),
    pResult( NULL )
{
    rHdr.StartEntry();

    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    rStream.ReadByteString( aAppl, eCharSet );
    rStream.ReadByteString( aTopic, eCharSet );
    rStream.ReadByteString( aItem, eCharSet );

    sal_Bool bHasValue;
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

    sal_Bool bHasValue = ( pResult != NULL );
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

    String aLinkStr;
    ScByteSequenceToString::GetString( aLinkStr, rValue, DDE_TXT_ENCODING );
    aLinkStr.ConvertLineEnd(LINEEND_LF);

    //  wenn String mit Zeilenende aufhoert, streichen:

    xub_StrLen nLen = aLinkStr.Len();
    if (nLen && aLinkStr.GetChar(nLen-1) == '\n')
        aLinkStr.Erase(nLen-1);

    String aLine;
    SCSIZE nCols = 1;       // Leerstring -> eine leere Zelle
    SCSIZE nRows = 1;
    if (aLinkStr.Len())
    {
        nRows = static_cast<SCSIZE>(aLinkStr.GetTokenCount( '\n' ));
        aLine = aLinkStr.GetToken( 0, '\n' );
        if (aLine.Len())
            nCols = static_cast<SCSIZE>(aLine.GetTokenCount( '\t' ));
    }

    if (!nRows || !nCols)               // keine Daten
    {
        pResult.Clear();
    }
    else                                // Daten aufteilen
    {
        //  Matrix immer neu anlegen, damit bIsString nicht durcheinanderkommt
        pResult = new ScMatrix( nCols, nRows );

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

        //  nMode bestimmt, wie der Text interpretiert wird (#44455#/#49783#):
        //  SC_DDE_DEFAULT - Zahlformat aus Zellvorlage "Standard"
        //  SC_DDE_ENGLISH - Standard-Zahlformat fuer English/US
        //  SC_DDE_TEXT    - ohne NumberFormatter direkt als String
        sal_uLong nStdFormat = 0;
        if ( nMode == SC_DDE_DEFAULT )
        {
            ScPatternAttr* pDefPattern = pDoc->GetDefPattern();     // enthaelt Standard-Vorlage
            if ( pDefPattern )
                nStdFormat = pDefPattern->GetNumberFormat( pFormatter );
        }
        else if ( nMode == SC_DDE_ENGLISH )
            nStdFormat = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);

        String aEntry;
        for (SCSIZE nR=0; nR<nRows; nR++)
        {
            aLine = aLinkStr.GetToken( (xub_StrLen) nR, '\n' );
            for (SCSIZE nC=0; nC<nCols; nC++)
            {
                aEntry = aLine.GetToken( (xub_StrLen) nC, '\t' );
                sal_uInt32 nIndex = nStdFormat;
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
        Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );
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

void ScDdeLink::ResetValue()
{
    pResult.Clear();

    //  Es hat sich was getan...
    //  Tracking, FID_DATACHANGED etc. passiert von aussen

    if (HasListeners())
        Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );
}

void __EXPORT ScDdeLink::ListenersGone()
{
    sal_Bool bWas = bIsInUpdate;
    bIsInUpdate = sal_True;             // Remove() kann Reschedule ausloesen??!?

    ScDocument* pStackDoc = pDoc;   // member pDoc can't be used after removing the link

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
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
        bNeedUpdate = sal_True;         // kann jetzt nicht ausgefuehrt werden
    else
    {
        bIsInUpdate = sal_True;
        //Application::Reschedule();    //! OS/2-Simulation
        pDoc->IncInDdeLinkUpdate();
        Update();
        pDoc->DecInDdeLinkUpdate();
        bIsInUpdate = sal_False;
        bNeedUpdate = sal_False;
    }
}


