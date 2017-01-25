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
#include <osl/endian.h>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <frmfmt.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <shellio.hxx>
#include <acorrect.hxx>
#include <swerror.h>

void SwEditShell::InsertGlossary( SwTextBlocks& rGlossary, const OUString& rStr )
{
    StartAllAction();
    GetDoc()->InsertGlossary( rGlossary, rStr, *GetCursor(), this );
    EndAllAction();
}

/// convert current selection into text block and add to the text block document, incl. templates
sal_uInt16 SwEditShell::MakeGlossary( SwTextBlocks& rBlks, const OUString& rName, const OUString& rShortName,
                                    bool bSaveRelFile, const OUString* pOnlyText )
{
    SwDoc* pGDoc = rBlks.GetDoc();

    OUString sBase;
    if(bSaveRelFile)
    {
        INetURLObject aURL( rBlks.GetFileName() );
        sBase = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    rBlks.SetBaseURL( sBase );

    if( pOnlyText )
        return rBlks.PutText( rShortName, rName, *pOnlyText );

    rBlks.ClearDoc();
    if( rBlks.BeginPutDoc( rShortName, rName ) )
    {
        rBlks.GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::DeleteRedlines );
        CopySelToDoc( pGDoc );
        rBlks.GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::NONE );
        return rBlks.PutDoc();
    }

    return USHRT_MAX;
}

sal_uInt16 SwEditShell::SaveGlossaryDoc( SwTextBlocks& rBlock,
                                    const OUString& rName,
                                    const OUString& rShortName,
                                    bool bSaveRelFile,
                                    bool bOnlyText )
{
    StartAllAction();

    SwDoc* pGDoc = rBlock.GetDoc();
    SwDoc* pMyDoc = GetDoc();

    OUString sBase;
    if(bSaveRelFile)
    {
        INetURLObject aURL( rBlock.GetFileName() );
        sBase = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    rBlock.SetBaseURL( sBase );
    sal_uInt16 nRet = USHRT_MAX;

    if( bOnlyText )
    {
        KillPams();

        SwPaM* pCursor = GetCursor();

        SwNodeIndex aStt( pMyDoc->GetNodes().GetEndOfExtras(), 1 );
        SwContentNode* pContentNd = pMyDoc->GetNodes().GoNext( &aStt );
        const SwNode* pNd = pContentNd->FindTableNode();
        if( !pNd )
            pNd = pContentNd;

        pCursor->GetPoint()->nNode = *pNd;
        if( pNd == pContentNd )
            pCursor->GetPoint()->nContent.Assign( pContentNd, 0 );
        pCursor->SetMark();

        // then until the end of the Node array
        pCursor->GetPoint()->nNode = pMyDoc->GetNodes().GetEndOfContent().GetIndex()-1;
        pContentNd = pCursor->GetContentNode();
        if( pContentNd )
            pCursor->GetPoint()->nContent.Assign( pContentNd, pContentNd->Len() );

        OUString sBuf;
        if( GetSelectedText( sBuf, GETSELTXT_PARABRK_TO_ONLYCR ) && !sBuf.isEmpty() )
            nRet = rBlock.PutText( rShortName, rName, sBuf );
    }
    else
    {
        rBlock.ClearDoc();
        if( rBlock.BeginPutDoc( rShortName, rName ) )
        {
            SwNodeIndex aStt( pMyDoc->GetNodes().GetEndOfExtras(), 1 );
            SwContentNode* pContentNd = pMyDoc->GetNodes().GoNext( &aStt );
            const SwNode* pNd = pContentNd->FindTableNode();
            if( !pNd ) pNd = pContentNd;
            SwPaM aCpyPam( *pNd );
            aCpyPam.SetMark();

            // then until the end of the nodes array
            aCpyPam.GetPoint()->nNode = pMyDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pContentNd = aCpyPam.GetContentNode();
            aCpyPam.GetPoint()->nContent.Assign(
                   pContentNd, (pContentNd) ? pContentNd->Len() : 0);

            aStt = pGDoc->GetNodes().GetEndOfExtras();
            pContentNd = pGDoc->GetNodes().GoNext( &aStt );
            SwPosition aInsPos( aStt, SwIndex( pContentNd ));
            pMyDoc->getIDocumentContentOperations().CopyRange( aCpyPam, aInsPos, /*bCopyAll=*/false, /*bCheckPos=*/true );

            nRet = rBlock.PutDoc();
        }
    }
    EndAllAction();
    return nRet;
}

/// copy all selections to the doc
bool SwEditShell::CopySelToDoc( SwDoc* pInsDoc )
{
    OSL_ENSURE( pInsDoc, "no Ins.Document"  );

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwContentNode *const pContentNode = aIdx.GetNode().GetContentNode();
    SwPosition aPos( aIdx,
        SwIndex(pContentNode, (pContentNode) ? pContentNode->Len() : 0));

    bool bRet = false;
    SET_CURR_SHELL( this );

    pInsDoc->getIDocumentFieldsAccess().LockExpFields();

    if( IsTableMode() )
    {
        // Copy parts of a table: create a table with the width of the original one and copy the
        // selected boxes. The sizes are corrected on a percentage basis.

        // search boxes using the layout
        SwTableNode* pTableNd;
        SwSelBoxes aBoxes;
        GetTableSel( *this, aBoxes );
        if( !aBoxes.empty() && nullptr != (pTableNd = const_cast<SwTableNode*>(aBoxes[0]
            ->GetSttNd()->FindTableNode()) ))
        {
            // check if the table name can be copied
            bool bCpyTableNm = aBoxes.size() == pTableNd->GetTable().GetTabSortBoxes().size();
            if( bCpyTableNm )
            {
                const OUString rTableName = pTableNd->GetTable().GetFrameFormat()->GetName();
                const SwFrameFormats& rTableFormats = *pInsDoc->GetTableFrameFormats();
                for( auto n = rTableFormats.size(); n; )
                    if( rTableFormats[ --n ]->GetName() == rTableName )
                    {
                        bCpyTableNm = false;
                        break;
                    }
            }
            bRet = pInsDoc->InsCopyOfTable( aPos, aBoxes, nullptr, bCpyTableNm );
        }
        else
            bRet = false;
    }
    else
    {
        bool bColSel = GetCursor_()->IsColumnSelection();
        if( bColSel && pInsDoc->IsClipBoard() )
            pInsDoc->SetColumnSelection( true );
        bool bSelectAll = StartsWithTable() && ExtendedSelectedAll();
        {
            for(SwPaM& rPaM : GetCursor()->GetRingContainer())
            {
                if( !rPaM.HasMark() )
                {
                    SwContentNode *const pNd = rPaM.GetContentNode();
                    if (nullptr != pNd &&
                        ( bColSel || !pNd->GetTextNode() ) )
                    {
                        rPaM.SetMark();
                        rPaM.Move( fnMoveForward, GoInContent );
                        bRet = GetDoc()->getIDocumentContentOperations().CopyRange( rPaM, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true )
                            || bRet;
                        rPaM.Exchange();
                        rPaM.DeleteMark();
                    }
                }
                else
                {
                    // Make a copy, so that in case we need to adjust the selection
                    // for the purpose of copying, our shell cursor is not touched.
                    // (Otherwise we would have to restore it.)
                    SwPaM aPaM(*rPaM.GetMark(), *rPaM.GetPoint());
                    if (bSelectAll)
                    {
                        // Selection starts at the first para of the first cell,
                        // but we want to copy the table and the start node before
                        // the first cell as well.
                        aPaM.Start()->nNode = aPaM.Start()->nNode.GetNode().FindTableNode()->GetIndex();
                        aPaM.Start()->nContent.Assign(nullptr, 0);
                    }
                    bRet = GetDoc()->getIDocumentContentOperations().CopyRange( aPaM, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true ) || bRet;
                }
            }
        }
    }

    pInsDoc->getIDocumentFieldsAccess().UnlockExpFields();
    if( !pInsDoc->getIDocumentFieldsAccess().IsExpFieldsLocked() )
        pInsDoc->getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

    return bRet;
}

/** Get text in a Selection
 *
 * @return false if the selected area is too big for being copied into the string buffer
 */
bool SwEditShell::GetSelectedText( OUString &rBuf, int nHndlParaBrk )
{
    GetCursor();  // creates all cursors if needed
    if( IsSelOnePara() )
    {
        rBuf = GetSelText();
        if( GETSELTXT_PARABRK_TO_BLANK == nHndlParaBrk )
        {
            rBuf = rBuf.replaceAll("\x0a", " ");
        }
        else if( IsSelFullPara() &&
            GETSELTXT_PARABRK_TO_ONLYCR != nHndlParaBrk )
        {
#ifdef _WIN32
                rBuf += "\015\012";
#else
                rBuf += "\012";
#endif
        }
    }
    else if( IsSelection() )
    {
        SvMemoryStream aStream;
#ifdef OSL_BIGENDIAN
        aStream.SetEndian( SvStreamEndian::BIG );
#else
        aStream.SetEndian( SvStreamEndian::LITTLE );
#endif
        WriterRef xWrt;
        SwReaderWriter::GetWriter( FILTER_TEXT, OUString(), xWrt );
        if( xWrt.is() )
        {
            // write selected areas into a ASCII document
            SwWriter aWriter( aStream, *this);
            xWrt->SetShowProgress(false);

            switch( nHndlParaBrk )
            {
            case GETSELTXT_PARABRK_TO_BLANK:
                xWrt->bASCII_ParaAsBlanc = true;
                xWrt->bASCII_NoLastLineEnd = true;
                break;

            case GETSELTXT_PARABRK_TO_ONLYCR:
                xWrt->bASCII_ParaAsCR = true;
                xWrt->bASCII_NoLastLineEnd = true;
                break;
            }

            //JP 09.05.00: write as UNICODE ! (and not as ANSI)
            SwAsciiOptions aAsciiOpt( xWrt->GetAsciiOptions() );
            aAsciiOpt.SetCharSet( RTL_TEXTENCODING_UCS2 );
            xWrt->SetAsciiOptions( aAsciiOpt );
            xWrt->bUCS2_WithStartChar = false;

            if (!IsError(aWriter.Write(xWrt)))
            {
                aStream.WriteUInt16( '\0' );

                const sal_Unicode *p = static_cast<sal_Unicode const *>(aStream.GetBuffer());
                if (p)
                    rBuf = OUString(p);
                else
                {
                    const sal_uInt64 nLen = aStream.GetSize();
                    OSL_ENSURE( nLen/sizeof( sal_Unicode )<static_cast<sal_uInt64>(SAL_MAX_INT32), "Stream can't fit in OUString" );
                    rtl_uString *pStr = rtl_uString_alloc(static_cast<sal_Int32>(nLen / sizeof( sal_Unicode )));
                    aStream.Seek( 0 );
                    aStream.ResetError();
                    //endian specific?, yipes!
                    aStream.ReadBytes(pStr->buffer, nLen);
                    rBuf = OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
