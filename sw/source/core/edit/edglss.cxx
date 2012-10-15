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

#include <comphelper/string.hxx>
#include <osl/endian.h>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <tools/cachestr.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <frmfmt.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <ndtxt.hxx>
#include <swtable.hxx>      // fuers kopieren von Tabellen
#include <shellio.hxx>      // SwTextBlocks
#include <acorrect.hxx>
#include <swerror.h>        // SwTextBlocks

/******************************************************************************
 *              jetzt mit einem verkappten Reader/Writer/Dokument
 ******************************************************************************/

void SwEditShell::InsertGlossary( SwTextBlocks& rGlossary, const String& rStr )
{
    StartAllAction();
    GetDoc()->InsertGlossary( rGlossary, rStr, *GetCrsr(), this );
    EndAllAction();
}


/******************************************************************************
 *              aktuelle Selektion zum Textbaustein machen und ins
 *          Textbausteindokument einfuegen, einschliesslich Vorlagen
 ******************************************************************************/


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

        // dann bis zum Ende vom Nodes Array
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

/******************************************************************************
 *                  kopiere alle Selectionen und das Doc
 ******************************************************************************/


sal_Bool SwEditShell::_CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pSttNd )
{
    OSL_ENSURE( pInsDoc, "kein Ins.Dokument"  );

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwCntntNode * pNd = aIdx.GetNode().GetCntntNode();
    SwPosition aPos( aIdx, SwIndex( pNd, pNd->Len() ));

    // soll der Index auf Anfang returnt werden ?
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
        // kopiere Teile aus einer Tabelle: lege eine Tabelle mit der Breite
        // von der Originalen an und kopiere die selectierten Boxen.
        // Die Groessen werden prozentual korrigiert.

        // lasse ueber das Layout die Boxen suchen
        SwTableNode* pTblNd;
        SwSelBoxes aBoxes;
        GetTblSel( *this, aBoxes );
        if( !aBoxes.empty() && 0 != (pTblNd = (SwTableNode*)aBoxes[0]
            ->GetSttNd()->FindTableNode() ))
        {
            // teste ob der TabellenName kopiert werden kann
            sal_Bool bCpyTblNm = aBoxes.size() == pTblNd->GetTable().GetTabSortBoxes().size();
            if( bCpyTblNm )
            {
                const String& rTblName = pTblNd->GetTable().GetFrmFmt()->GetName();
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

    // die gemerkte Node-Position wieder auf den richtigen Node
    if( bRet && pSttNd )
        ++(*pSttNd);


    return bRet;
}

/*------------------------------------------------------------------------
 Beschreibung:  Text innerhalb der Selektion erfragen
 Returnwert:    liefert sal_False, wenn der selektierte Bereich
                zu gross ist, um in den Stringpuffer kopiert zu werden.
------------------------------------------------------------------------*/

sal_Bool SwEditShell::GetSelectedText( String &rBuf, int nHndlParaBrk )
{
    GetCrsr();  // ggfs. alle Cursor erzeugen lassen
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
                rBuf += rtl::OUString("\015\012");
#endif
        }
    }
    else if( IsSelection() )
    {
        SvCacheStream aStream(20480);
#ifdef OSL_BIGENDIAN
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
        aStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
        WriterRef xWrt;
        SwReaderWriter::GetWriter( rtl::OUString(FILTER_TEXT), String(), xWrt );
        if( xWrt.Is() )
        {
                // Selektierte Bereiche in ein ASCII Dokument schreiben
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
                    rBuf = rtl::OUString(p);
                else
                {
                    using comphelper::string::rtl_uString_alloc;
                    rtl_uString *pStr = rtl_uString_alloc(lLen / sizeof( sal_Unicode ));
                    aStream.Seek( 0 );
                    aStream.ResetError();
                    //endian specific?, yipes!
                    aStream.Read(pStr->buffer, lLen);
                    rBuf = rtl::OUString(pStr, SAL_NO_ACQUIRE);
                }
            }
        }
    }

    return sal_True;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
