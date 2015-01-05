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
    GetDoc()->InsertGlossary( rGlossary, rStr, *GetCrsr(), this );
    EndAllAction();
}

/// convert current selection into text block and add to the text block document, incl. templates
sal_uInt16 SwEditShell::MakeGlossary( SwTextBlocks& rBlks, const OUString& rName, const OUString& rShortName,
                                    bool bSaveRelFile, const OUString* pOnlyTxt )
{
    SwDoc* pGDoc = rBlks.GetDoc();

    OUString sBase;
    if(bSaveRelFile)
    {
        INetURLObject aURL( rBlks.GetFileName() );
        sBase = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }
    rBlks.SetBaseURL( sBase );

    sal_uInt16 nRet;

    if( pOnlyTxt )
        nRet = rBlks.PutText( rShortName, rName, *pOnlyTxt );
    else
    {
        rBlks.ClearDoc();
        if( rBlks.BeginPutDoc( rShortName, rName ) )
        {
            rBlks.GetDoc()->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_DELETE_REDLINES );
            _CopySelToDoc( pGDoc );
            rBlks.GetDoc()->getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)0 );
            nRet = rBlks.PutDoc();
        }
        else
            nRet = (sal_uInt16) -1;
    }

    return nRet;
}

sal_uInt16 SwEditShell::SaveGlossaryDoc( SwTextBlocks& rBlock,
                                    const OUString& rName,
                                    const OUString& rShortName,
                                    bool bSaveRelFile,
                                    bool bOnlyTxt )
{
    StartAllAction();

    SwDoc* pGDoc = rBlock.GetDoc();
    SwDoc* pMyDoc = GetDoc();

    OUString sBase;
    if(bSaveRelFile)
    {
        INetURLObject aURL( rBlock.GetFileName() );
        sBase = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }
    rBlock.SetBaseURL( sBase );
    sal_uInt16 nRet = USHRT_MAX;

    if( bOnlyTxt )
    {
        KillPams();

        SwPaM* pCrsr = GetCrsr();

        SwNodeIndex aStt( pMyDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pCntntNd = pMyDoc->GetNodes().GoNext( &aStt );
        const SwNode* pNd = pCntntNd->FindTableNode();
        if( !pNd )
            pNd = pCntntNd;

        pCrsr->GetPoint()->nNode = *pNd;
        if( pNd == pCntntNd )
            pCrsr->GetPoint()->nContent.Assign( pCntntNd, 0 );
        pCrsr->SetMark();

        // then until the end of the Node array
        pCrsr->GetPoint()->nNode = pMyDoc->GetNodes().GetEndOfContent().GetIndex()-1;
        pCntntNd = pCrsr->GetCntntNode();
        if( pCntntNd )
            pCrsr->GetPoint()->nContent.Assign( pCntntNd, pCntntNd->Len() );

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
            SwCntntNode* pCntntNd = pMyDoc->GetNodes().GoNext( &aStt );
            const SwNode* pNd = pCntntNd->FindTableNode();
            if( !pNd ) pNd = pCntntNd;
            SwPaM aCpyPam( *pNd );
            aCpyPam.SetMark();

            // then until the end of the nodes array
            aCpyPam.GetPoint()->nNode = pMyDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pCntntNd = aCpyPam.GetCntntNode();
            aCpyPam.GetPoint()->nContent.Assign(
                   pCntntNd, (pCntntNd) ? pCntntNd->Len() : 0);

            aStt = pGDoc->GetNodes().GetEndOfExtras();
            pCntntNd = pGDoc->GetNodes().GoNext( &aStt );
            SwPosition aInsPos( aStt, SwIndex( pCntntNd ));
            pMyDoc->getIDocumentContentOperations().CopyRange( aCpyPam, aInsPos, false );

            nRet = rBlock.PutDoc();
        }
    }
    EndAllAction();
    return nRet;
}

/// copy all selections to the doc
bool SwEditShell::_CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pSttNd )
{
    OSL_ENSURE( pInsDoc, "no Ins.Document"  );

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwCntntNode *const pContentNode = aIdx.GetNode().GetCntntNode();
    SwPosition aPos( aIdx,
        SwIndex(pContentNode, (pContentNode) ? pContentNode->Len() : 0));

    // Should the index be reset to start?
    if( pSttNd )
    {
        *pSttNd = aPos.nNode;
        (*pSttNd)--;
    }

    bool bRet = false;
    SET_CURR_SHELL( this );

    pInsDoc->getIDocumentFieldsAccess().LockExpFlds();

    if( IsTableMode() )
    {
        // Copy parts of a table: create a table with the width of the original one and copy the
        // selected boxes. The sizes are corrected on a percentage basis.

        // search boxes using the layout
        SwTableNode* pTblNd;
        SwSelBoxes aBoxes;
        GetTblSel( *this, aBoxes );
        if( !aBoxes.empty() && 0 != (pTblNd = (SwTableNode*)aBoxes[0]
            ->GetSttNd()->FindTableNode() ))
        {
            // check if the table name can be copied
            bool bCpyTblNm = aBoxes.size() == pTblNd->GetTable().GetTabSortBoxes().size();
            if( bCpyTblNm )
            {
                const OUString rTblName = pTblNd->GetTable().GetFrmFmt()->GetName();
                const SwFrmFmts& rTblFmts = *pInsDoc->GetTblFrmFmts();
                for( sal_uInt16 n = rTblFmts.size(); n; )
                    if( rTblFmts[ --n ]->GetName() == rTblName )
                    {
                        bCpyTblNm = false;
                        break;
                    }
            }
            bRet = pInsDoc->InsCopyOfTbl( aPos, aBoxes, 0, bCpyTblNm, false );
        }
        else
            bRet = false;
    }
    else
    {
        bool bColSel = _GetCrsr()->IsColumnSelection();
        if( bColSel && pInsDoc->IsClipBoard() )
            pInsDoc->SetColumnSelection( true );
        bool bSelectAll = StartsWithTable() && ExtendedSelectedAll(/*bFootnotes =*/ false);
        {
            for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
            {
                if( !rPaM.HasMark() )
                {
                    SwCntntNode *const pNd = rPaM.GetCntntNode();
                    if (0 != pNd &&
                        ( bColSel || !pNd->GetTxtNode() ) )
                    {
                        rPaM.SetMark();
                        rPaM.Move( fnMoveForward, fnGoCntnt );
                        bRet = GetDoc()->getIDocumentContentOperations().CopyRange( rPaM, aPos, false )
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
                    SwPaM aPaM(rPaM);
                    if (bSelectAll)
                    {
                        // Selection starts at the first para of the first cell,
                        // but we want to copy the table and the start node before
                        // the first cell as well.
                        aPaM.Start()->nNode = aPaM.Start()->nNode.GetNode().FindTableNode()->GetIndex();
                    }
                    bRet = GetDoc()->getIDocumentContentOperations().CopyRange( aPaM, aPos, false ) || bRet;
                }
            }
        }
    }

    pInsDoc->getIDocumentFieldsAccess().UnlockExpFlds();
    if( !pInsDoc->getIDocumentFieldsAccess().IsExpFldsLocked() )
        pInsDoc->getIDocumentFieldsAccess().UpdateExpFlds(NULL, true);

    // set the saved Node position back to the correct Node
    if( bRet && pSttNd )
        ++(*pSttNd);

    return bRet;
}

/** Get text in a Selection
 *
 * @return false if the selected area is too big for being copied into the string buffer
 */
bool SwEditShell::GetSelectedText( OUString &rBuf, int nHndlParaBrk )
{
    GetCrsr();  // creates all cursors if needed
    if( IsSelOnePara() )
    {
        rBuf = GetSelTxt();
        if( GETSELTXT_PARABRK_TO_BLANK == nHndlParaBrk )
        {
            rBuf = rBuf.replaceAll(OUString(0x0a), " ");
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
        SwReaderWriter::GetWriter( OUString(FILTER_TEXT), OUString(), xWrt );
        if( xWrt.Is() )
        {
            // write selected areas into a ASCII document
            SwWriter aWriter( aStream, *this);
            xWrt->SetShowProgress( false );

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

                const sal_Unicode *p = (sal_Unicode*)aStream.GetBuffer();
                if (p)
                    rBuf = OUString(p);
                else
                {
                    sal_Size nLen = aStream.GetSize();
                    rtl_uString *pStr = rtl_uString_alloc(nLen / sizeof( sal_Unicode ));
                    aStream.Seek( 0 );
                    aStream.ResetError();
                    //endian specific?, yipes!
                    aStream.Read(pStr->buffer, nLen);
                    rBuf = OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
