/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <comphelper/string.hxx>
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

TYPEINIT2(ScDdeLink,::sfx2::SvBaseLink,SfxBroadcaster);

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

bool ScDdeLink::bIsInUpdate = false;

//------------------------------------------------------------------------

ScDdeLink::ScDdeLink( ScDocument* pD, const String& rA, const String& rT, const String& rI,
                        sal_uInt8 nM ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    aAppl( rA ),
    aTopic( rT ),
    aItem( rI ),
    nMode( nM ),
    bNeedUpdate( false ),
    pResult( NULL )
{
}

ScDdeLink::~ScDdeLink()
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
    bNeedUpdate( false ),
    pResult ( NULL )
{
    if (rOther.pResult)
        pResult = rOther.pResult->Clone();
}

ScDdeLink::ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ALWAYS,FORMAT_STRING),
    pDoc( pD ),
    bNeedUpdate( false ),
    pResult( NULL )
{
    rHdr.StartEntry();

    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    aAppl = rStream.ReadUniOrByteString( eCharSet );
    aTopic = rStream.ReadUniOrByteString( eCharSet );
    aItem = rStream.ReadUniOrByteString( eCharSet );

    sal_Bool bHasValue;
    rStream >> bHasValue;
    if ( bHasValue )
        pResult = new ScMatrix(0, 0);

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
    rStream.WriteUniOrByteString( aAppl, eCharSet );
    rStream.WriteUniOrByteString( aTopic, eCharSet );
    rStream.WriteUniOrByteString( aItem, eCharSet );

    sal_Bool bHasValue = ( pResult != 0 );
    rStream << bHasValue;

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )      // nicht bei 4.0 Export
        rStream << nMode;                                   // seit 388b

    //  Links mit Mode != SC_DDE_DEFAULT werden bei 4.0 Export komplett weggelassen
    //  (aus ScDocument::SaveDdeLinks)

    rHdr.EndEntry();
}

sfx2::SvBaseLink::UpdateResult ScDdeLink::DataChanged(
    const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue )
{
    //  wir koennen nur Strings...
    if ( FORMAT_STRING != SotExchange::GetFormatIdFromMimeType( rMimeType ))
        return SUCCESS;

    OUString aLinkStr;
    ScByteSequenceToString::GetString( aLinkStr, rValue, DDE_TXT_ENCODING );
    aLinkStr = convertLineEnd(aLinkStr, LINEEND_LF);

    //  wenn String mit Zeilenende aufhoert, streichen:

    sal_Int32 nLen = aLinkStr.getLength();
    if (nLen && aLinkStr[nLen-1] == '\n')
        aLinkStr = aLinkStr.copy(0, nLen-1);

    String aLine;
    SCSIZE nCols = 1;       // Leerstring -> eine leere Zelle
    SCSIZE nRows = 1;
    if (!aLinkStr.isEmpty())
    {
        nRows = static_cast<SCSIZE>(comphelper::string::getTokenCount(aLinkStr, '\n'));
        aLine = aLinkStr.getToken( 0, '\n' );
        if (aLine.Len())
            nCols = static_cast<SCSIZE>(comphelper::string::getTokenCount(aLine, '\t'));
    }

    if (!nRows || !nCols)               // keine Daten
    {
        pResult.reset();
    }
    else                                // Daten aufteilen
    {
        //  Matrix immer neu anlegen, damit bIsString nicht durcheinanderkommt
        pResult = new ScMatrix(nCols, nRows, 0.0);

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
            aLine = aLinkStr.getToken( (sal_Int32) nR, '\n' );
            for (SCSIZE nC=0; nC<nCols; nC++)
            {
                aEntry = aLine.GetToken( (xub_StrLen) nC, '\t' );
                sal_uInt32 nIndex = nStdFormat;
                double fVal = double();
                if ( nMode != SC_DDE_TEXT && pFormatter->IsNumberFormat( aEntry, nIndex, fVal ) )
                    pResult->PutDouble( fVal, nC, nR );
                else if (aEntry.Len() == 0)
                    // empty cell
                    pResult->PutEmpty(nC, nR);
                else
                    pResult->PutString( aEntry, nC, nR );
            }
        }
    }

    //  Es hat sich was getan...

    if (HasListeners())
    {
        Broadcast(ScHint(SC_HINT_DATACHANGED, ScAddress()));
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

    return SUCCESS;
}

void ScDdeLink::ListenersGone()
{
    bool bWas = bIsInUpdate;
    bIsInUpdate = true;             // Remove() kann Reschedule ausloesen??!?

    ScDocument* pStackDoc = pDoc;   // member pDoc can't be used after removing the link

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    pLinkMgr->Remove( this);        // deletes this

    if ( pLinkMgr->GetLinks().empty() )            // letzten geloescht ?
    {
        SfxBindings* pBindings = pStackDoc->GetViewBindings();      // don't use member pDoc!
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }

    bIsInUpdate = bWas;
}

const ScMatrix* ScDdeLink::GetResult() const
{
    return pResult.get();
}

void ScDdeLink::SetResult( const ScMatrixRef& pRes )
{
    pResult = pRes;
}

void ScDdeLink::TryUpdate()
{
    if (bIsInUpdate)
        bNeedUpdate = true;         // kann jetzt nicht ausgefuehrt werden
    else
    {
        bIsInUpdate = true;
        pDoc->IncInDdeLinkUpdate();
        Update();
        pDoc->DecInDdeLinkUpdate();
        bIsInUpdate = false;
        bNeedUpdate = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
