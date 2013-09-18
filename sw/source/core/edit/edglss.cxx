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
#include <pam.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <frmfmt.hxx>
#include <swundo.hxx>       // for UndoIds
#include <ndtxt.hxx>
#include <swtable.hxx>      // for table copying
#include <shellio.hxx>      // SwTextBlocks
#include <acorrect.hxx>
#include <swerror.h>        // SwTextBlocks

void SwEditShell::InsertGlossary( SwTextBlocks& rGlossary, const String& rStr )
{
    StartAllAction();
    GetDoc()->InsertGlossary( rGlossary, rStr, *GetCrsr(), this );
    EndAllAction();
}

/// convert current selection into text block and add to the text block document, incl. templates
sal_uInt16 SwEditShell::MakeGlossary( SwTextBlocks& rBlks, const String& rName, const String& rShortName,
                                    sal_Bool bSaveRelFile, const String* pOnlyTxt )
{
    SwDoc* pGDoc = rBlks.GetDoc();

    String sBase;
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
            rBlks.GetDoc()->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_DELETE_REDLINES );
            _CopySelToDoc( pGDoc );
            rBlks.GetDoc()->SetRedlineMode_intern( (RedlineMode_t)0 );
            nRet = rBlks.PutDoc();
        }
        else
            nRet = (sal_uInt16) -1;
    }

    return nRet;
}

sal_uInt16 SwEditShell::SaveGlossaryDoc( SwTextBlocks& rBlock,
                                    const String& rName,
                                    const String& rShortName,
                                    sal_Bool bSaveRelFile,
                                    sal_Bool bOnlyTxt )
{
    StartAllAction();

    SwDoc* pGDoc = rBlock.GetDoc();
    SwDoc* pMyDoc = GetDoc();

    String sBase;
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

        String sBuf;
        if( GetSelectedText( sBuf, GETSELTXT_PARABRK_TO_ONLYCR ) && sBuf.Len() )
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

            // dann bis zum Ende vom Nodes Array
            aCpyPam.GetPoint()->nNode = pMyDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pCntntNd = aCpyPam.GetCntntNode();
            aCpyPam.GetPoint()->nContent.Assign( pCntntNd, pCntntNd->Len() );

            aStt = pGDoc->GetNodes().GetEndOfExtras();
            pCntntNd = pGDoc->GetNodes().GoNext( &aStt );
            SwPosition aInsPos( aStt, SwIndex( pCntntNd ));
            pMyDoc->CopyRange( aCpyPam, aInsPos, false );

            nRet = rBlock.PutDoc();
        }
    }
    EndAllAction();
    return nRet;
}

/// copy all selections to the doc
sal_Bool SwEditShell::_CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pSttNd )
{
    OSL_ENSURE( pInsDoc, "no Ins.Document"  );

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwCntntNode * pNd = aIdx.GetNode().GetCntntNode();
    SwPosition aPos( aIdx, SwIndex( pNd, pNd->Len() ));

    // Should the index be reset to start?
    if( pSttNd )
    {
        *pSttNd = aPos.nNode;
        (*pSttNd)--;
    }

    sal_Bool bRet = sal_False;
    SET_CURR_SHELL( this );

    pInsDoc->LockExpFlds();

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
            sal_Bool bCpyTblNm = aBoxes.size() == pTblNd->GetTable().GetTabSortBoxes().size();
            if( bCpyTblNm )
            {
                const OUString rTblName = pTblNd->GetTable().GetFrmFmt()->GetName();
                const SwFrmFmts& rTblFmts = *pInsDoc->GetTblFrmFmts();
                for( sal_uInt16 n = rTblFmts.size(); n; )
                    if( rTblFmts[ --n ]->GetName() == rTblName )
                    {
                        bCpyTblNm = sal_False;
                        break;
                    }
            }
            bRet = pInsDoc->InsCopyOfTbl( aPos, aBoxes, 0, bCpyTblNm, sal_False );
        }
        else
            bRet = sal_False;
    }
    else
    {
        bool bColSel = _GetCrsr()->IsColumnSelection();
        if( bColSel && pInsDoc->IsClipBoard() )
            pInsDoc->SetColumnSelection( true );
        {
        FOREACHPAM_START(this)

            if( !PCURCRSR->HasMark() )
            {
                if( 0 != (pNd = PCURCRSR->GetCntntNode()) &&
                    ( bColSel || !pNd->GetTxtNode() ) )
                {
                    PCURCRSR->SetMark();
                    PCURCRSR->Move( fnMoveForward, fnGoCntnt );
                    bRet = GetDoc()->CopyRange( *PCURCRSR, aPos, false )
                        || bRet;
                    PCURCRSR->Exchange();
                    PCURCRSR->DeleteMark();
                }
            }
            else
            {
                bRet = GetDoc()->CopyRange( *PCURCRSR, aPos, false ) || bRet;
            }

        FOREACHPAM_END()
        }
    }

    pInsDoc->UnlockExpFlds();
    if( !pInsDoc->IsExpFldsLocked() )
        pInsDoc->UpdateExpFlds(NULL, true);

    // set the saved Node position back to the correct Node
    if( bRet && pSttNd )
        ++(*pSttNd);

    return bRet;
}

/** Get text in a Selection
 *
 * @return sal_False if the selected area is too big for being copied into the string buffer
 */
sal_Bool SwEditShell::GetSelectedText( String &rBuf, int nHndlParaBrk )
{
    GetCrsr();  // creates all cursors if needed
    if( IsSelOnePara() )
    {
        rBuf = GetSelTxt();
        if( GETSELTXT_PARABRK_TO_BLANK == nHndlParaBrk )
        {
            xub_StrLen nPos = 0;
            while( STRING_NOTFOUND !=
                ( nPos = rBuf.SearchAndReplace( 0x0a, ' ', nPos )) )
                ;
        }
        else if( IsSelFullPara() &&
            GETSELTXT_PARABRK_TO_ONLYCR != nHndlParaBrk )
        {
#if defined(UNX)
                rBuf += '\012';
#else
                rBuf += OUString("\015\012");
#endif
        }
    }
    else if( IsSelection() )
    {
        SvMemoryStream aStream;
#ifdef OSL_BIGENDIAN
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
        WriterRef xWrt;
        SwReaderWriter::GetWriter( OUString(FILTER_TEXT), String(), xWrt );
        if( xWrt.Is() )
        {
            // write selected areas into a ASCII document
            SwWriter aWriter( aStream, *this);
            xWrt->SetShowProgress( sal_False );

            switch( nHndlParaBrk )
            {
            case GETSELTXT_PARABRK_TO_BLANK:
                xWrt->bASCII_ParaAsBlanc = sal_True;
                xWrt->bASCII_NoLastLineEnd = sal_True;
                break;

            case GETSELTXT_PARABRK_TO_ONLYCR:
                xWrt->bASCII_ParaAsCR = sal_True;
                xWrt->bASCII_NoLastLineEnd = sal_True;
                break;
            }

            //JP 09.05.00: write as UNICODE ! (and not as ANSI)
            SwAsciiOptions aAsciiOpt( xWrt->GetAsciiOptions() );
            aAsciiOpt.SetCharSet( RTL_TEXTENCODING_UCS2 );
            xWrt->SetAsciiOptions( aAsciiOpt );
            xWrt->bUCS2_WithStartChar = sal_False;

            long lLen;
            if( !IsError( aWriter.Write( xWrt ) ) &&
                STRING_MAXLEN > (( lLen  = aStream.GetSize() )
                                        / sizeof( sal_Unicode )) + 1 )
            {
                aStream << (sal_Unicode)'\0';

                const sal_Unicode *p = (sal_Unicode*)aStream.GetBuffer();
                if( p )
                    rBuf = OUString(p);
                else
                {
                    rtl_uString *pStr = rtl_uString_alloc(lLen / sizeof( sal_Unicode ));
                    aStream.Seek( 0 );
                    aStream.ResetError();
                    //endian specific?, yipes!
                    aStream.Read(pStr->buffer, lLen);
                    rBuf = OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
