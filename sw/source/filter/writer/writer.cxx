/*************************************************************************
 *
 *  $RCSfile: writer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-13 17:26:24 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>          // fuer SwBookmark ...
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

// Stringbuffer fuer die umgewandelten Zahlen
static sal_Char aNToABuf[] = "0000000000000000000000000";
#define NTOABUFLEN (sizeof(aNToABuf))

DECLARE_TABLE( SwBookmarkNodeTable, SvPtrarr* )

struct Writer_Impl
{
    SvStringsSortDtor *pSrcArr, *pDestArr;
    SvPtrarr* pFontRemoveLst, *pBkmkArr;
    SwBookmarkNodeTable* pBkmkNodePos;

    Writer_Impl( const SwDoc& rDoc );
    ~Writer_Impl();

    void RemoveFontList( SwDoc& rDoc );
    void InsertBkmk( const SwBookmark& rBkmk );
};

Writer_Impl::Writer_Impl( const SwDoc& rDoc )
    : pSrcArr( 0 ), pDestArr( 0 ), pFontRemoveLst( 0 ), pBkmkNodePos( 0 )
{
}

Writer_Impl::~Writer_Impl()
{
    delete pSrcArr;
    delete pDestArr;
    delete pFontRemoveLst;

    if( pBkmkNodePos )
    {
        for( SvPtrarr* p = pBkmkNodePos->First(); p; p = pBkmkNodePos->Next() )
            delete p;
        delete pBkmkNodePos;
    }
}

void Writer_Impl::RemoveFontList( SwDoc& rDoc )
{
    ASSERT( pFontRemoveLst, "wo ist die FontListe?" );
    for( USHORT i = pFontRemoveLst->Count(); i; )
    {
        SvxFontItem* pItem = (SvxFontItem*)(*pFontRemoveLst)[ --i ];
        rDoc.GetAttrPool().Remove( *pItem );
    }
}

void Writer_Impl::InsertBkmk( const SwBookmark& rBkmk )
{
    if( !pBkmkNodePos )
        pBkmkNodePos = new SwBookmarkNodeTable;

    ULONG nNd = rBkmk.GetPos().nNode.GetIndex();
    SvPtrarr* pArr = pBkmkNodePos->Get( nNd );
    if( !pArr )
    {
        pArr = new SvPtrarr( 1, 4 );
        pBkmkNodePos->Insert( nNd, pArr );
    }

    void* p = (void*)&rBkmk;
    pArr->Insert( p, pArr->Count() );

    if( rBkmk.GetOtherPos() && rBkmk.GetOtherPos()->nNode != nNd )
    {
        nNd = rBkmk.GetOtherPos()->nNode.GetIndex();
        pArr = pBkmkNodePos->Get( nNd );
        if( !pArr )
        {
            pArr = new SvPtrarr( 1, 4 );
            pBkmkNodePos->Insert( nNd, pArr );
        }
        pArr->Insert( p, pArr->Count() );
    }
}

/*
 * Dieses Modul ist die Zentrale-Sammelstelle fuer alle Write-Filter
 * und ist eine DLL !
 *
 * Damit der Writer mit den unterschiedlichen Writern arbeiten kann,
 * muessen fuer diese die Ausgabe-Funktionen der Inhalts tragenden
 * Objecte auf die verschiedenen Ausgabe-Funktionen gemappt werden.
 *
 * Dazu kann fuer jedes Object ueber den Which-Wert in einen Tabelle ge-
 * griffen werden, um seine Ausgabe-Funktion zu erfragen.
 * Diese Funktionen stehen in den entsprechenden Writer-DLL's.
 */

Writer::Writer()
    : pImpl( 0 ), pStrm( 0 ), pOrigPam( 0 ), pOrigFileName( 0 ),
    pCurPam(0), pDoc( 0 )
{
    bWriteAll = bShowProgress = bUCS2_WithStartChar = TRUE;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = FALSE;
}

Writer::~Writer()
{
}

void Writer::ResetWriter()
{
    if( pImpl && pImpl->pFontRemoveLst )
        pImpl->RemoveFontList( *pDoc );
    delete pImpl, pImpl = 0;

    if( pCurPam )
    {
        while( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }
    pCurPam = 0;
    pOrigFileName = 0;
    pDoc = 0;
    pStrm = 0;

    bShowProgress = bUCS2_WithStartChar = TRUE;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = FALSE;
}

BOOL Writer::CopyNextPam( SwPaM ** ppPam )
{
    if( (*ppPam)->GetNext() == pOrigPam )
    {
        *ppPam = pOrigPam;          // wieder auf den Anfangs-Pam setzen
        return FALSE;               // Ende vom Ring
    }

    // ansonsten kopiere den die Werte aus dem naechsten Pam
    *ppPam = ((SwPaM*)(*ppPam)->GetNext() );

    *pCurPam->GetPoint() = *(*ppPam)->Start();
    *pCurPam->GetMark() = *(*ppPam)->End();

    return TRUE;
}

// suche die naechste Bookmark-Position aus der Bookmark-Tabelle

USHORT Writer::FindPos_Bkmk( const SwPosition& rPos ) const
{
    USHORT nRet = USHRT_MAX;
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();

    if( rBkmks.Count() )
    {
        SwBookmark aBkmk( rPos );
        USHORT nPos;
        if( rBkmks.Seek_Entry( &aBkmk, &nPos ))
        {
            // suche abwaerts nach weiteren Bookmarks auf der Cursor-Position
            while( 0 < nPos &&
                rBkmks[ nPos-1 ]->IsEqualPos( aBkmk ))
                --nPos;
        }
        else if( nPos < rBkmks.Count() )
            nRet = nPos;
    }
    return nRet;
}


SwPaM* Writer::NewSwPaM( SwDoc & rDoc, ULONG nStartIdx, ULONG nEndIdx,
                        BOOL bNodesArray ) const
{
    SwNodes* pNds = bNodesArray ? &rDoc.GetNodes() : (SwNodes*)rDoc.GetUndoNds();

    SwNodeIndex aStt( *pNds, nStartIdx );
    SwCntntNode* pCNode = aStt.GetNode().GetCntntNode();
    if( !pCNode && 0 == ( pCNode = pNds->GoNext( &aStt )) )
        ASSERT( !this, "An StartPos kein ContentNode mehr" );

    SwPaM* pNew = new SwPaM( aStt );
    pNew->SetMark();
    aStt = nEndIdx;
    if( 0 == (pCNode = aStt.GetNode().GetCntntNode()) &&
        0 == (pCNode = pNds->GoPrevious( &aStt )) )
        ASSERT( !this, "An StartPos kein ContentNode mehr" );
    pCNode->MakeEndIndex( &pNew->GetPoint()->nContent );
    pNew->GetPoint()->nNode = aStt;
    return pNew;
}

/////////////////////////////////////////////////////////////////////////////

// Stream-spezifisches
#ifndef PRODUCT
SvStream& Writer::Strm()
{
    ASSERT( pStrm, "Oh-oh. Dies ist ein Storage-Writer. Gleich knallts!" );
    return *pStrm;
}
#endif


SvStream& Writer::OutHex( SvStream& rStrm, ULONG nHex, BYTE nLen )
{                                                  // in einen Stream aus
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);
    for( BYTE n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStrm << pStr;
}

SvStream& Writer::OutLong( SvStream& rStrm, long nVal )
{
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);

    int bNeg = nVal < 0;
    if( bNeg )
        nVal = -nVal;

    do {
        *(--pStr) = (sal_Char)(nVal % 10 ) + 48;
        nVal /= 10;
    } while( nVal );

    // Ist Zahl negativ, dann noch -
    if( bNeg )
        *(--pStr) = '-';

    return rStrm << pStr;
}

SvStream& Writer::OutULong( SvStream& rStrm, ULONG nVal )
{
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);

    do {
        *(--pStr) = (sal_Char)(nVal % 10 ) + 48;
        nVal /= 10;
    } while ( nVal );
    return rStrm << pStr;
}


ULONG Writer::Write( SwPaM& rPaM, SvStream& rStrm, const String* pFName )
{
    pStrm = &rStrm;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;
    pImpl = new Writer_Impl( *pDoc );

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    ULONG nRet = WriteStream();

    ResetWriter();

    return nRet;
}

ULONG Writer::Write( SwPaM& rPam, SfxMedium& rMed, const String* pFileName )
{
    return IsStgWriter()
                ? Write( rPam, *rMed.GetStorage(), pFileName )
                : Write( rPam, *rMed.GetOutStream(), pFileName );
}

ULONG Writer::Write( SwPaM& rPam, SvStorage&, const String* )
{
    ASSERT( !this, "Schreiben in Storages auf einem Stream?" );
    return ERR_SWG_WRITE_ERROR;
}


BOOL Writer::CopyLocalFileToINet( String& rFileNm )
{
    BOOL bRet = FALSE;
    INetURLObject aFileUrl( rFileNm ), aTargetUrl( *pOrigFileName );

// JP 01.11.00: what is the correct question for the portal??
//  if( aFileUrl.GetProtocol() == aFileUrl.GetProtocol() )
//      return bRet;
// this is our old without the Mail-Export
    if( ! ( INET_PROT_FILE == aFileUrl.GetProtocol() &&
            INET_PROT_FILE != aTargetUrl.GetProtocol() &&
            INET_PROT_FTP <= aTargetUrl.GetProtocol() &&
            INET_PROT_NEWS >= aTargetUrl.GetProtocol() ) )
        return bRet;

    if( pImpl->pSrcArr )
    {
        // wurde die Datei schon verschoben
        USHORT nPos;
        if( pImpl->pSrcArr->Seek_Entry( &rFileNm, &nPos ))
        {
            rFileNm = *(*pImpl->pDestArr)[ nPos ];
            return TRUE;
        }
    }
    else
    {
        pImpl->pSrcArr = new SvStringsSortDtor( 4, 4 );
        pImpl->pDestArr = new SvStringsSortDtor( 4, 4 );
    }

    String *pSrc = new String( rFileNm );
    String *pDest = new String( aTargetUrl.GetPartBeforeLastName() );
    *pDest += aFileUrl.GetName();

    SfxMedium aSrcFile( *pSrc, STREAM_READ, FALSE );
    SfxMedium aDstFile( *pDest, STREAM_WRITE | STREAM_SHARE_DENYNONE, FALSE );

    *aDstFile.GetOutStream() << *aSrcFile.GetInStream();

    aSrcFile.Close();
    aDstFile.Commit();

    bRet = 0 == aDstFile.GetError();

    if( bRet )
    {
        pImpl->pSrcArr->Insert( pSrc );
        pImpl->pDestArr->Insert( pDest );
        rFileNm = *pDest;
    }
    else
    {
        delete pSrc;
        delete pDest;
    }

    return bRet;
}

void Writer::PutNumFmtFontsInAttrPool()
{
    // dann gibt es noch in den NumRules ein paar Fonts
    // Diese in den Pool putten. Haben sie danach einen RefCount > 1
    // kann es wieder entfernt werden - ist schon im Pool
    SfxItemPool& rPool = pDoc->GetAttrPool();
    const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();
    const SwNumRule* pRule;
    const SwNumFmt* pFmt;
    const Font *pFont, *pDefFont = &SwNumRule::GetDefBulletFont();
    BOOL bCheck = FALSE;

    for( USHORT nGet = rListTbl.Count(); nGet; )
        if( pDoc->IsUsed( *(pRule = rListTbl[ --nGet ] )))
            for( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
                if( SVX_NUM_CHAR_SPECIAL == (pFmt = &pRule->Get( nLvl ))->eType ||
                    SVX_NUM_BITMAP == pFmt->eType )
                {
                    if( 0 == ( pFont = pFmt->GetBulletFont() ) )
                        pFont = pDefFont;

                    if( bCheck )
                    {
                        if( *pFont == *pDefFont )
                            continue;
                    }
                    else if( *pFont == *pDefFont )
                        bCheck = TRUE;

                    _AddFontItem( rPool, SvxFontItem( pFont->GetFamily(),
                                pFont->GetName(), pFont->GetStyleName(),
                                pFont->GetPitch(), pFont->GetCharSet() ));
                }
}

void Writer::PutEditEngFontsInAttrPool( BOOL bIncl_CJK_CTL )
{
    SfxItemPool* pPool = pDoc->GetAttrPool().GetSecondaryPool();
    if( pPool )
    {
        _AddFontItems( *pPool, EE_CHAR_FONTINFO );
        if( bIncl_CJK_CTL )
        {
            _AddFontItems( *pPool, EE_CHAR_FONTINFO_CJK );
            _AddFontItems( *pPool, EE_CHAR_FONTINFO_CTL );
        }
    }
}

void Writer::PutCJKandCTLFontsInAttrPool()
{
    SfxItemPool& rPool = pDoc->GetAttrPool();
    _AddFontItems( rPool, RES_CHRATR_CJK_FONT );
    _AddFontItems( rPool, RES_CHRATR_CTL_FONT );
}


void Writer::_AddFontItems( SfxItemPool& rPool, USHORT nW )
{
    const SvxFontItem* pFont = (const SvxFontItem*)&rPool.GetDefaultItem( nW );
    _AddFontItem( rPool, *pFont );

    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem( nW )) )
        _AddFontItem( rPool, *pFont );

    USHORT nMaxItem = rPool.GetItemCount( nW );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem( nW, nGet )) )
            _AddFontItem( rPool, *pFont );
}

void Writer::_AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont )
{
    const SvxFontItem* pItem;
    if( RES_CHRATR_FONT != rFont.Which() )
    {
        SvxFontItem aFont( rFont );
        aFont.SetWhich( RES_CHRATR_FONT );
        pItem = (SvxFontItem*)&rPool.Put( aFont );
    }
    else
        pItem = (SvxFontItem*)&rPool.Put( rFont );

    if( 1 < pItem->GetRef() )
        rPool.Remove( *pItem );
    else
    {
        if( !pImpl->pFontRemoveLst )
            pImpl->pFontRemoveLst = new SvPtrarr( 0, 10 );

        void* p = (void*)pItem;
        pImpl->pFontRemoveLst->Insert( p, pImpl->pFontRemoveLst->Count() );
    }
}

// build a bookmark table, which is sort by the node position. The
// OtherPos of the bookmarks also inserted.
void Writer::CreateBookmarkTbl()
{
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    for( USHORT n = rBkmks.Count(); n; )
    {
        const SwBookmark& rBkmk = *rBkmks[ --n ];
        if( rBkmk.IsBookMark() )
            pImpl->InsertBkmk( rBkmk );
    }
}


// search alle Bookmarks in the range and return it in the Array
USHORT Writer::GetBookmarks( const SwCntntNode& rNd, xub_StrLen nStt,
                             xub_StrLen nEnd, SvPtrarr& rArr )
{
    ASSERT( !rArr.Count(), "es sind noch Eintraege vorhanden" );

    ULONG nNd = rNd.GetIndex();
    SvPtrarr* pArr = pImpl->pBkmkNodePos ? pImpl->pBkmkNodePos->Get( nNd ) : 0;
    if( pArr )
    {
        // there exist some bookmarks, search now all which is in the range
        if( !nStt && nEnd == rNd.Len() )
            // all
            rArr.Insert( pArr, 0 );
        else
        {
            USHORT n;
            xub_StrLen nCntnt;
            for( n = 0; n < pArr->Count(); ++n )
            {
                void* p = (*pArr)[ n ];
                const SwBookmark& rBkmk = *(SwBookmark*)p;
                if( rBkmk.GetPos().nNode == nNd &&
                    (nCntnt = rBkmk.GetPos().nContent.GetIndex() ) >= nStt &&
                    nCntnt < nEnd )
                {
                    rArr.Insert( p, rArr.Count() );
                }
                else if( rBkmk.GetOtherPos() && nNd ==
                        rBkmk.GetOtherPos()->nNode.GetIndex() && (nCntnt =
                        rBkmk.GetOtherPos()->nContent.GetIndex() ) >= nStt &&
                        nCntnt < nEnd )
                {
                    rArr.Insert( p, rArr.Count() );
                }
            }
        }
    }
    return rArr.Count();
}

////////////////////////////////////////////////////////////////////////////

// Storage-spezifisches

ULONG StgWriter::WriteStream()
{
    ASSERT( !this, "Schreiben in Streams auf einem Storage?" );
    return ERR_SWG_WRITE_ERROR;
}

ULONG StgWriter::Write( SwPaM& rPaM, SvStorage& rStg, const String* pFName )
{
    pStrm = 0;
    pStg = &rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;
    pImpl = new Writer_Impl( *pDoc );

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    ULONG nRet = WriteStorage();

    pStg = NULL;
    ResetWriter();

    return nRet;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/writer/writer.cxx,v 1.4 2000-11-13 17:26:24 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.3  2000/11/01 19:31:51  jp
      Writer:CopyLocalFileToINet: export of mail graphics removed and SvFileStream access removed

      Revision 1.2  2000/10/17 15:13:50  os
      Change: SfxMedium Ctor

      Revision 1.1.1.1  2000/09/18 17:14:57  hr
      initial import

      Revision 1.103  2000/09/18 16:04:55  willem.vandorp
      OpenOffice header added.

      Revision 1.102  2000/08/04 16:25:50  jp
      read/write unicode ascii files

      Revision 1.101  2000/06/26 13:01:26  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.100  2000/06/13 09:43:46  os
      using UCB

      Revision 1.99  2000/05/08 16:51:53  jp
      Changes for Unicode

      Revision 1.98  1999/10/13 10:38:29  jp
      PutEditEngFontsInAtrPool - check if the editenginepool exist

      Revision 1.97  1999/10/12 20:04:37  jp
      Writer: put EditEngine fonts into the writer pool

      Revision 1.96  1999/08/12 10:19:56  MIB
      Don\'t delete impl if it hasn\'t been set (for XML filter)


      Rev 1.95   12 Aug 1999 12:19:56   MIB
   Don't delete impl if it hasn't been set (for XML filter)

      Rev 1.94   30 Jun 1999 18:53:20   JP
   dtor for impl class

      Rev 1.93   24 Jun 1999 22:51:34   JP
   new: Writer with internal Impl-structur, build Bookmark table sortet by nodes

      Rev 1.92   23 Jun 1999 19:13:40   JP
   interface of Writer::FindPos_Bkmk has changed

      Rev 1.91   16 Jun 1999 19:52:14   JP
   Change interface of base class Writer

      Rev 1.90   28 Jan 1999 14:21:28   JP
   GraphicFilter-SS hat sich geaendert - Task #59174#

      Rev 1.89   30 Oct 1998 18:30:02   JP
   Task #58596#: neues Flag an der Writerklasse -> schreibe nur die 1. Tabelle

      Rev 1.88   27 Jun 1998 16:02:08   JP
   Writer mit neuen Flags; fuer den ASCII-Writer, etwas aufgeraeumt

      Rev 1.87   23 Apr 1998 19:07:44   MIB
   fix #49729#: Stream beim kopieren von Grafiken ins INet schliessen

      Rev 1.86   22 Jan 1998 20:02:56   JP
   CTOR des SwPaM umgestellt

      Rev 1.85   26 Nov 1997 14:29:14   MA
   headerfiles

      Rev 1.84   03 Nov 1997 14:12:48   MA
   precomp entfernt

      Rev 1.83   13 Oct 1997 16:06:34   JP
   pNext vom Ring wurde privat; zugriff ueber GetNext()

      Rev 1.82   09 Oct 1997 14:26:24   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.81   03 Sep 1997 11:59:44   JP
   zusaetzliches include von docary

*************************************************************************/

