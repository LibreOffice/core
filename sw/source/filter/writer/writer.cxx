/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writer.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:45:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>

#include <sot/storage.hxx>
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
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>          // fuer SwBookmark ...
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif


using namespace ::com::sun::star;


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

Writer_Impl::Writer_Impl( const SwDoc& /*rDoc*/ )
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

    ULONG nNd = rBkmk.GetBookmarkPos().nNode.GetIndex();
    SvPtrarr* pArr = pBkmkNodePos->Get( nNd );
    if( !pArr )
    {
        pArr = new SvPtrarr( 1, 4 );
        pBkmkNodePos->Insert( nNd, pArr );
    }

    void* p = (void*)&rBkmk;
    pArr->Insert( p, pArr->Count() );

    if( rBkmk.GetOtherBookmarkPos() && rBkmk.GetOtherBookmarkPos()->nNode != nNd )
    {
        nNd = rBkmk.GetOtherBookmarkPos()->nNode.GetIndex();
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
    : pImpl(0), pStrm(0), pOrigPam(0), pOrigFileName(0), pDoc(0), pCurPam(0)
{
    bWriteAll = bShowProgress = bUCS2_WithStartChar = true;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = false;
}

Writer::~Writer()
{
}

/*
 * Document Interface Access
 */
IDocumentSettingAccess* Writer::getIDocumentSettingAccess() { return pDoc; }
const IDocumentSettingAccess* Writer::getIDocumentSettingAccess() const { return pDoc; }
IDocumentStylePoolAccess* Writer::getIDocumentStylePoolAccess() { return pDoc; }
const IDocumentStylePoolAccess* Writer::getIDocumentStylePoolAccess() const { return pDoc; }

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
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = FALSE;
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
    const SwBookmarks& rBkmks = pDoc->getBookmarks();

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
    {
        ASSERT( !this, "An StartPos kein ContentNode mehr" );
    }

    SwPaM* pNew = new SwPaM( aStt );
    pNew->SetMark();
    aStt = nEndIdx;
    if( 0 == (pCNode = aStt.GetNode().GetCntntNode()) &&
        0 == (pCNode = pNds->GoPrevious( &aStt )) )
    {
        ASSERT( !this, "An StartPos kein ContentNode mehr" );
    }
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
    if ( IsStgWriter() )
    {
        SotStorageRef aRef = new SotStorage( rStrm );
        ULONG nResult = Write( rPaM, *aRef, pFName );
        if ( nResult == ERRCODE_NONE )
            aRef->Commit();
        return nResult;
    }

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
    // This method must be overloaded in SwXMLWriter a storage from medium will be used there.
    // The microsoft format can write to storage but the storage will be based on the stream.
    return Write( rPam, *rMed.GetOutStream(), pFileName );

    // return IsStgWriter()
    //            ? Write( rPam, rMed.GetStorage(), pFileName )
    //          : Write( rPam, *rMed.GetOutStream(), pFileName );
}

ULONG Writer::Write( SwPaM& /*rPam*/, SvStorage&, const String* )
{
    ASSERT( !this, "Schreiben in Storages auf einem Stream?" );
    return ERR_SWG_WRITE_ERROR;
}

ULONG Writer::Write( SwPaM&, const uno::Reference < embed::XStorage >&, const String*, SfxMedium* )
{
    ASSERT( !this, "Schreiben in Storages auf einem Stream?" );
    return ERR_SWG_WRITE_ERROR;
}

BOOL Writer::CopyLocalFileToINet( String& rFileNm )
{
    if( !pOrigFileName )                // can be happen, by example if we
        return FALSE;                   // write into the clipboard

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
    *pDest += String(aFileUrl.GetName());

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
    if( !pImpl )
        pImpl = new Writer_Impl( *pDoc );

    // dann gibt es noch in den NumRules ein paar Fonts
    // Diese in den Pool putten. Haben sie danach einen RefCount > 1
    // kann es wieder entfernt werden - ist schon im Pool
    SfxItemPool& rPool = pDoc->GetAttrPool();
    const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();
    const SwNumRule* pRule;
    const SwNumFmt* pFmt;
    // --> OD 2006-06-27 #b644095#
//    const Font *pFont, *pDefFont = &SwNumRule::GetDefBulletFont();
    const Font* pFont;
    const Font* pDefFont = &numfunc::GetDefBulletFont();
    // <--
    BOOL bCheck = FALSE;

    for( USHORT nGet = rListTbl.Count(); nGet; )
        if( pDoc->IsUsed( *(pRule = rListTbl[ --nGet ] )))
            for( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
                if( SVX_NUM_CHAR_SPECIAL == (pFmt = &pRule->Get( nLvl ))->GetNumberingType() ||
                    SVX_NUM_BITMAP == pFmt->GetNumberingType() )
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
                                pFont->GetPitch(), pFont->GetCharSet(), RES_CHRATR_FONT ));
                }
}

void Writer::PutEditEngFontsInAttrPool( BOOL bIncl_CJK_CTL )
{
    if( !pImpl )
        pImpl = new Writer_Impl( *pDoc );

    SfxItemPool& rPool = pDoc->GetAttrPool();
    if( rPool.GetSecondaryPool() )
    {
        _AddFontItems( rPool, EE_CHAR_FONTINFO );
        if( bIncl_CJK_CTL )
        {
            _AddFontItems( rPool, EE_CHAR_FONTINFO_CJK );
            _AddFontItems( rPool, EE_CHAR_FONTINFO_CTL );
        }
    }
}

void Writer::PutCJKandCTLFontsInAttrPool()
{
    if( !pImpl )
        pImpl = new Writer_Impl( *pDoc );

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

    if( 1 < pItem->GetRefCount() )
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
    const SwBookmarks& rBkmks = pDoc->getBookmarks();
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
                if( rBkmk.GetBookmarkPos().nNode == nNd &&
                    (nCntnt = rBkmk.GetBookmarkPos().nContent.GetIndex() ) >= nStt &&
                    nCntnt < nEnd )
                {
                    rArr.Insert( p, rArr.Count() );
                }
                else if( rBkmk.GetOtherBookmarkPos() && nNd ==
                        rBkmk.GetOtherBookmarkPos()->nNode.GetIndex() && (nCntnt =
                        rBkmk.GetOtherBookmarkPos()->nContent.GetIndex() ) >= nStt &&
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

ULONG StgWriter::Write( SwPaM& rPaM, const uno::Reference < embed::XStorage >& rStg, const String* pFName, SfxMedium* pMedium )
{
    pStrm = 0;
    pStg = 0;
    xStg = rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;
    pImpl = new Writer_Impl( *pDoc );

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    ULONG nRet = pMedium ? WriteMedium( *pMedium ) : WriteStorage();

    pStg = NULL;
    ResetWriter();

    return nRet;
}

