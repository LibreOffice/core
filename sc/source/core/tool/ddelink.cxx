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
#include <osl/thread.h>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>

#include "ddelink.hxx"
#include "brdcst.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "rangeseq.hxx"
#include "sc.hrc"
#include "hints.hxx"


#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

bool ScDdeLink::bIsInUpdate = false;

ScDdeLink::ScDdeLink( ScDocument* pD, const OUString& rA, const OUString& rT, const OUString& rI,
                        sal_uInt8 nM ) :
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ALWAYS,SotClipboardFormatId::STRING),
    pDoc( pD ),
    aAppl( rA ),
    aTopic( rT ),
    aItem( rI ),
    nMode( nM ),
    bNeedUpdate( false ),
    pResult( nullptr )
{
}

ScDdeLink::~ScDdeLink()
{
    // cancel connection

    // pResult is refcounted
}

ScDdeLink::ScDdeLink( ScDocument* pD, const ScDdeLink& rOther ) :
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ALWAYS,SotClipboardFormatId::STRING),
    pDoc    ( pD ),
    aAppl   ( rOther.aAppl ),
    aTopic  ( rOther.aTopic ),
    aItem   ( rOther.aItem ),
    nMode   ( rOther.nMode ),
    bNeedUpdate( false ),
    pResult ( nullptr )
{
    if (rOther.pResult)
        pResult = rOther.pResult->Clone();
}

ScDdeLink::ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ALWAYS,SotClipboardFormatId::STRING),
    pDoc( pD ),
    bNeedUpdate( false ),
    pResult( nullptr )
{
    rHdr.StartEntry();

    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    aAppl = rStream.ReadUniOrByteString( eCharSet );
    aTopic = rStream.ReadUniOrByteString( eCharSet );
    aItem = rStream.ReadUniOrByteString( eCharSet );

    bool bHasValue;
    rStream.ReadCharAsBool( bHasValue );
    if ( bHasValue )
        pResult = new ScFullMatrix(0, 0);

    if (rHdr.BytesLeft())       // new in 388b and the 364w (RealTime Client) version
        rStream.ReadUChar( nMode );
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

    bool bHasValue = ( pResult != nullptr );
    rStream.WriteBool( bHasValue );

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )      // not with 4.0 Export
        rStream.WriteUChar( nMode );                                   // since 388b

    //  links with Mode != SC_DDE_DEFAULT are completely omitted in 4.0 Export
    //  (from ScDocument::SaveDdeLinks)

    rHdr.EndEntry();
}

sfx2::SvBaseLink::UpdateResult ScDdeLink::DataChanged(
    const OUString& rMimeType, const css::uno::Any & rValue )
{
    //  we only master strings...
    if ( SotClipboardFormatId::STRING != SotExchange::GetFormatIdFromMimeType( rMimeType ))
        return SUCCESS;

    OUString aLinkStr;
    ScByteSequenceToString::GetString( aLinkStr, rValue, DDE_TXT_ENCODING );
    aLinkStr = convertLineEnd(aLinkStr, LINEEND_LF);

    //  if string ends with line end, discard:

    sal_Int32 nLen = aLinkStr.getLength();
    if (nLen && aLinkStr[nLen-1] == '\n')
        aLinkStr = aLinkStr.copy(0, nLen-1);

    OUString aLine;
    SCSIZE nCols = 1;       // empty string -> an empty line
    SCSIZE nRows = 1;
    if (!aLinkStr.isEmpty())
    {
        nRows = static_cast<SCSIZE>(comphelper::string::getTokenCount(aLinkStr, '\n'));
        aLine = aLinkStr.getToken( 0, '\n' );
        if (!aLine.isEmpty())
            nCols = static_cast<SCSIZE>(comphelper::string::getTokenCount(aLine, '\t'));
    }

    if (!nRows || !nCols)               // no data
    {
        pResult.reset();
    }
    else                                // split data
    {
        //  always newly re-create matrix, so that bIsString doesn't get mixed up
        pResult = new ScFullMatrix(nCols, nRows, 0.0);

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        svl::SharedStringPool& rPool = pDoc->GetSharedStringPool();

        //  nMode determines how the text is interpreted (#44455#/#49783#):
        //  SC_DDE_DEFAULT - number format from cell template "Standard"
        //  SC_DDE_ENGLISH - standard number format for English/US
        //  SC_DDE_TEXT    - without NumberFormatter directly as string
        sal_uLong nStdFormat = 0;
        if ( nMode == SC_DDE_DEFAULT )
        {
            ScPatternAttr* pDefPattern = pDoc->GetDefPattern();     // contains standard template
            if ( pDefPattern )
                nStdFormat = pDefPattern->GetNumberFormat( pFormatter );
        }
        else if ( nMode == SC_DDE_ENGLISH )
            nStdFormat = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);

        OUString aEntry;
        for (SCSIZE nR=0; nR<nRows; nR++)
        {
            aLine = aLinkStr.getToken( (sal_Int32) nR, '\n' );
            for (SCSIZE nC=0; nC<nCols; nC++)
            {
                aEntry = aLine.getToken( (sal_Int32) nC, '\t' );
                sal_uInt32 nIndex = nStdFormat;
                double fVal = double();
                if ( nMode != SC_DDE_TEXT && pFormatter->IsNumberFormat( aEntry, nIndex, fVal ) )
                    pResult->PutDouble( fVal, nC, nR );
                else if (aEntry.isEmpty())
                    // empty cell
                    pResult->PutEmpty(nC, nR);
                else
                    pResult->PutString(rPool.intern(aEntry), nC, nR);
            }
        }
    }

    //  Something happened...

    if (HasListeners())
    {
        Broadcast(ScHint(SfxHintId::ScDataChanged, ScAddress()));
        pDoc->TrackFormulas();      // must happen immediatelly
        pDoc->StartTrackTimer();

        //  StartTrackTimer asynchronously calls TrackFormulas, Broadcast(FID_DATACHANGED),
        //  ResetChanged, SetModified and Invalidate(SID_SAVEDOC/SID_DOC_MODIFIED)
        //  TrackFormulas additionally once again immediatelly, so that, e.g., a formula still
        //  located in the FormulaTrack doesn't get calculated by IdleCalc (#61676#)

        //  notify Uno objects (for XRefreshListener)
        //  must be after TrackFormulas
        //TODO: do this asynchronously?
        ScLinkRefreshedHint aHint;
        aHint.SetDdeLink( aAppl, aTopic, aItem, nMode );
        pDoc->BroadcastUno( aHint );
    }

    return SUCCESS;
}

void ScDdeLink::ListenersGone()
{
    bool bWas = bIsInUpdate;
    bIsInUpdate = true;             // Remove() can trigger reschedule??!?

    ScDocument* pStackDoc = pDoc;   // member pDoc can't be used after removing the link

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    pLinkMgr->Remove( this);        // deletes this

    if ( pLinkMgr->GetLinks().empty() )            // deleted the last one ?
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
        bNeedUpdate = true;         // cannot be executed now
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
