/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


sal_uInt16 SwEditShell::MakeGlossary( SwTextBlocks& rBlks, const OUString& rName, const OUString& rShortName,
                                    sal_Bool bSaveRelFile, const OUString* pOnlyTxt )
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
                                    const OUString& rName,
                                    const OUString& rShortName,
                                    sal_Bool bSaveRelFile,
                                    sal_Bool bOnlyTxt )
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


sal_Bool SwEditShell::_CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pSttNd )
{
    OSL_ENSURE( pInsDoc, "no Ins.Document"  );

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwCntntNode * pNd = aIdx.GetNode().GetCntntNode();
    SwPosition aPos( aIdx, SwIndex( pNd, pNd->Len() ));

    
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
        
        

        
        SwTableNode* pTblNd;
        SwSelBoxes aBoxes;
        GetTblSel( *this, aBoxes );
        if( !aBoxes.empty() && 0 != (pTblNd = (SwTableNode*)aBoxes[0]
            ->GetSttNd()->FindTableNode() ))
        {
            
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
        bool bSelectAll = StartsWithTable() && ExtendedSelectedAll(/*bFootnotes =*/ false);
        {
        FOREACHPAM_START(GetCrsr())

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
                
                
                
                SwPaM aPaM(*PCURCRSR);
                if (bSelectAll)
                    
                    
                    
                    aPaM.Start()->nNode = aPaM.Start()->nNode.GetNode().FindTableNode()->GetIndex();
                bRet = GetDoc()->CopyRange( aPaM, aPos, false ) || bRet;
            }

        FOREACHPAM_END()
        }
    }

    pInsDoc->UnlockExpFlds();
    if( !pInsDoc->IsExpFldsLocked() )
        pInsDoc->UpdateExpFlds(NULL, true);

    
    if( bRet && pSttNd )
        ++(*pSttNd);

    return bRet;
}

/** Get text in a Selection
 *
 * @return sal_False if the selected area is too big for being copied into the string buffer
 */
sal_Bool SwEditShell::GetSelectedText( OUString &rBuf, int nHndlParaBrk )
{
    GetCrsr();  
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
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
        WriterRef xWrt;
        SwReaderWriter::GetWriter( OUString(FILTER_TEXT), OUString(), xWrt );
        if( xWrt.Is() )
        {
            
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

            
            SwAsciiOptions aAsciiOpt( xWrt->GetAsciiOptions() );
            aAsciiOpt.SetCharSet( RTL_TEXTENCODING_UCS2 );
            xWrt->SetAsciiOptions( aAsciiOpt );
            xWrt->bUCS2_WithStartChar = false;

            if (!IsError(aWriter.Write(xWrt)))
            {
                aStream.WriteUInt16( (sal_Unicode)'\0' );

                const sal_Unicode *p = (sal_Unicode*)aStream.GetBuffer();
                if (p)
                    rBuf = OUString(p);
                else
                {
                    sal_Size nLen = aStream.GetSize();
                    rtl_uString *pStr = rtl_uString_alloc(nLen / sizeof( sal_Unicode ));
                    aStream.Seek( 0 );
                    aStream.ResetError();
                    
                    aStream.Read(pStr->buffer, nLen);
                    rBuf = OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
