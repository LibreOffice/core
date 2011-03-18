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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <hintids.hxx>

#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>

#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <svtools/filter.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/eeitem.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IMark.hxx>
#include <numrule.hxx>
#include <swerror.h>
#include <boost/bind.hpp>

using namespace ::com::sun::star;


// Stringbuffer fuer die umgewandelten Zahlen
static sal_Char aNToABuf[] = "0000000000000000000000000";
#define NTOABUFLEN (sizeof(aNToABuf))

DECLARE_TABLE( SwBookmarkNodeTable, SvPtrarr* )

struct Writer_Impl
{
    SvStream * m_pStream;

    SvStringsSortDtor *pSrcArr, *pDestArr;
    SvPtrarr* pFontRemoveLst, *pBkmkArr;
    SwBookmarkNodeTable* pBkmkNodePos;

    Writer_Impl();
    ~Writer_Impl();

    void RemoveFontList( SwDoc& rDoc );
    void InsertBkmk( const ::sw::mark::IMark& rBkmk );
};

Writer_Impl::Writer_Impl()
    : m_pStream(0)
    , pSrcArr( 0 ), pDestArr( 0 ), pFontRemoveLst( 0 )
    , pBkmkArr( 0 ), pBkmkNodePos( 0 )
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
    OSL_ENSURE( pFontRemoveLst, "wo ist die FontListe?" );
    for( sal_uInt16 i = pFontRemoveLst->Count(); i; )
    {
        SvxFontItem* pItem = (SvxFontItem*)(*pFontRemoveLst)[ --i ];
        rDoc.GetAttrPool().Remove( *pItem );
    }
}

void Writer_Impl::InsertBkmk(const ::sw::mark::IMark& rBkmk)
{
    if( !pBkmkNodePos )
        pBkmkNodePos = new SwBookmarkNodeTable;

    sal_uLong nNd = rBkmk.GetMarkPos().nNode.GetIndex();
    SvPtrarr* pArr = pBkmkNodePos->Get( nNd );
    if( !pArr )
    {
        pArr = new SvPtrarr( 1, 4 );
        pBkmkNodePos->Insert( nNd, pArr );
    }

    void* p = (void*)&rBkmk;
    pArr->Insert( p, pArr->Count() );

    if(rBkmk.IsExpanded() && rBkmk.GetOtherMarkPos().nNode != nNd)
    {
        nNd = rBkmk.GetOtherMarkPos().nNode.GetIndex();
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
    : m_pImpl(new Writer_Impl)
    , pOrigPam(0), pOrigFileName(0), pDoc(0), pCurPam(0)
{
    bWriteAll = bShowProgress = bUCS2_WithStartChar = true;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = false;
    bExportPargraphNumbering = sal_True;
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
    if (m_pImpl->pFontRemoveLst)
    {
        m_pImpl->RemoveFontList( *pDoc );
    }
    m_pImpl.reset(new Writer_Impl);

    if( pCurPam )
    {
        while( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }
    pCurPam = 0;
    pOrigFileName = 0;
    pDoc = 0;

    bShowProgress = bUCS2_WithStartChar = sal_True;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = sal_False;
}

sal_Bool Writer::CopyNextPam( SwPaM ** ppPam )
{
    if( (*ppPam)->GetNext() == pOrigPam )
    {
        *ppPam = pOrigPam;          // wieder auf den Anfangs-Pam setzen
        return sal_False;               // Ende vom Ring
    }

    // ansonsten kopiere den die Werte aus dem naechsten Pam
    *ppPam = ((SwPaM*)(*ppPam)->GetNext() );

    *pCurPam->GetPoint() = *(*ppPam)->Start();
    *pCurPam->GetMark() = *(*ppPam)->End();

    return sal_True;
}

// suche die naechste Bookmark-Position aus der Bookmark-Tabelle

sal_Int32 Writer::FindPos_Bkmk(const SwPosition& rPos) const
{
    const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    const IDocumentMarkAccess::const_iterator_t ppBkmk = ::std::lower_bound(
        pMarkAccess->getMarksBegin(),
        pMarkAccess->getMarksEnd(),
        rPos,
        ::boost::bind(&::sw::mark::IMark::StartsBefore, _1, _2)); // find the first Mark that does not start before
    if(ppBkmk != pMarkAccess->getMarksEnd())
        return ppBkmk - pMarkAccess->getMarksBegin();
    return -1;
}


SwPaM *
Writer::NewSwPaM(SwDoc & rDoc, sal_uLong const nStartIdx, sal_uLong const nEndIdx)
{
    SwNodes *const pNds = &rDoc.GetNodes();

    SwNodeIndex aStt( *pNds, nStartIdx );
    SwCntntNode* pCNode = aStt.GetNode().GetCntntNode();
    if( !pCNode && 0 == ( pCNode = pNds->GoNext( &aStt )) )
    {
        OSL_FAIL( "An StartPos kein ContentNode mehr" );
    }

    SwPaM* pNew = new SwPaM( aStt );
    pNew->SetMark();
    aStt = nEndIdx;
    if( 0 == (pCNode = aStt.GetNode().GetCntntNode()) &&
        0 == (pCNode = pNds->GoPrevious( &aStt )) )
    {
        OSL_FAIL( "An StartPos kein ContentNode mehr" );
    }
    pCNode->MakeEndIndex( &pNew->GetPoint()->nContent );
    pNew->GetPoint()->nNode = aStt;
    return pNew;
}

/////////////////////////////////////////////////////////////////////////////

// Stream-spezifisches
SvStream& Writer::Strm()
{
    OSL_ENSURE( m_pImpl->m_pStream, "Oh-oh. Writer with no Stream!" );
    return *m_pImpl->m_pStream;
}

void Writer::SetStream(SvStream *const pStream)
{ m_pImpl->m_pStream = pStream; }


SvStream& Writer::OutHex( SvStream& rStrm, sal_uLong nHex, sal_uInt8 nLen )
{                                                  // in einen Stream aus
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);
    for( sal_uInt8 n = 0; n < nLen; ++n )
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

SvStream& Writer::OutULong( SvStream& rStrm, sal_uLong nVal )
{
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);

    do {
        *(--pStr) = (sal_Char)(nVal % 10 ) + 48;
        nVal /= 10;
    } while ( nVal );
    return rStrm << pStr;
}


sal_uLong Writer::Write( SwPaM& rPaM, SvStream& rStrm, const String* pFName )
{
    if ( IsStgWriter() )
    {
        SotStorageRef aRef = new SotStorage( rStrm );
        sal_uLong nResult = Write( rPaM, *aRef, pFName );
        if ( nResult == ERRCODE_NONE )
            aRef->Commit();
        return nResult;
    }

    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;
    m_pImpl->m_pStream = &rStrm;

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    sal_uLong nRet = WriteStream();

    ResetWriter();

    return nRet;
}

sal_uLong Writer::Write( SwPaM& rPam, SfxMedium& rMed, const String* pFileName )
{
    // This method must be overloaded in SwXMLWriter a storage from medium will be used there.
    // The microsoft format can write to storage but the storage will be based on the stream.
    return Write( rPam, *rMed.GetOutStream(), pFileName );
}

sal_uLong Writer::Write( SwPaM& /*rPam*/, SvStorage&, const String* )
{
    OSL_ENSURE( !this, "Schreiben in Storages auf einem Stream?" );
    return ERR_SWG_WRITE_ERROR;
}

sal_uLong Writer::Write( SwPaM&, const uno::Reference < embed::XStorage >&, const String*, SfxMedium* )
{
    OSL_ENSURE( !this, "Schreiben in Storages auf einem Stream?" );
    return ERR_SWG_WRITE_ERROR;
}

sal_Bool Writer::CopyLocalFileToINet( String& rFileNm )
{
    if( !pOrigFileName )                // can be happen, by example if we
        return sal_False;                   // write into the clipboard

    sal_Bool bRet = sal_False;
    INetURLObject aFileUrl( rFileNm ), aTargetUrl( *pOrigFileName );

// this is our old without the Mail-Export
    if( ! ( INET_PROT_FILE == aFileUrl.GetProtocol() &&
            INET_PROT_FILE != aTargetUrl.GetProtocol() &&
            INET_PROT_FTP <= aTargetUrl.GetProtocol() &&
            INET_PROT_NEWS >= aTargetUrl.GetProtocol() ) )
        return bRet;

    if (m_pImpl->pSrcArr)
    {
        // wurde die Datei schon verschoben
        sal_uInt16 nPos;
        if (m_pImpl->pSrcArr->Seek_Entry( &rFileNm, &nPos ))
        {
            rFileNm = *(*m_pImpl->pDestArr)[ nPos ];
            return sal_True;
        }
    }
    else
    {
        m_pImpl->pSrcArr = new SvStringsSortDtor( 4, 4 );
        m_pImpl->pDestArr = new SvStringsSortDtor( 4, 4 );
    }

    String *pSrc = new String( rFileNm );
    String *pDest = new String( aTargetUrl.GetPartBeforeLastName() );
    *pDest += String(aFileUrl.GetName());

    SfxMedium aSrcFile( *pSrc, STREAM_READ, sal_False );
    SfxMedium aDstFile( *pDest, STREAM_WRITE | STREAM_SHARE_DENYNONE, sal_False );

    *aDstFile.GetOutStream() << *aSrcFile.GetInStream();

    aSrcFile.Close();
    aDstFile.Commit();

    bRet = 0 == aDstFile.GetError();

    if( bRet )
    {
        m_pImpl->pSrcArr->Insert( pSrc );
        m_pImpl->pDestArr->Insert( pDest );
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
    const Font* pFont;
    const Font* pDefFont = &numfunc::GetDefBulletFont();
    // <--
    sal_Bool bCheck = sal_False;

    for( sal_uInt16 nGet = rListTbl.Count(); nGet; )
        if( pDoc->IsUsed( *(pRule = rListTbl[ --nGet ] )))
            for( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
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
                        bCheck = sal_True;

                    _AddFontItem( rPool, SvxFontItem( pFont->GetFamily(),
                                pFont->GetName(), pFont->GetStyleName(),
                                pFont->GetPitch(), pFont->GetCharSet(), RES_CHRATR_FONT ));
                }
}

void Writer::PutEditEngFontsInAttrPool( sal_Bool bIncl_CJK_CTL )
{
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
    SfxItemPool& rPool = pDoc->GetAttrPool();
    _AddFontItems( rPool, RES_CHRATR_CJK_FONT );
    _AddFontItems( rPool, RES_CHRATR_CTL_FONT );
}


void Writer::_AddFontItems( SfxItemPool& rPool, sal_uInt16 nW )
{
    const SvxFontItem* pFont = (const SvxFontItem*)&rPool.GetDefaultItem( nW );
    _AddFontItem( rPool, *pFont );

    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem( nW )) )
        _AddFontItem( rPool, *pFont );

    sal_uInt32 nMaxItem = rPool.GetItemCount2( nW );
    for( sal_uInt32 nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem2( nW, nGet )) )
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
        if (!m_pImpl->pFontRemoveLst)
        {
            m_pImpl->pFontRemoveLst = new SvPtrarr( 0, 10 );
        }

        void* p = (void*)pItem;
        m_pImpl->pFontRemoveLst->Insert( p, m_pImpl->pFontRemoveLst->Count() );
    }
}

// build a bookmark table, which is sort by the node position. The
// OtherPos of the bookmarks also inserted.
void Writer::CreateBookmarkTbl()
{
    const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    for(IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->getBookmarksBegin();
        ppBkmk != pMarkAccess->getBookmarksEnd();
        ++ppBkmk)
    {
        m_pImpl->InsertBkmk(**ppBkmk);
    }
}


// search alle Bookmarks in the range and return it in the Array
sal_uInt16 Writer::GetBookmarks(const SwCntntNode& rNd, xub_StrLen nStt,
    xub_StrLen nEnd, SvPtrarr& rArr)
{
    OSL_ENSURE( !rArr.Count(), "es sind noch Eintraege vorhanden" );

    sal_uLong nNd = rNd.GetIndex();
    SvPtrarr* pArr = (m_pImpl->pBkmkNodePos) ?
        m_pImpl->pBkmkNodePos->Get( nNd ) : 0;
    if( pArr )
    {
        // there exist some bookmarks, search now all which is in the range
        if( !nStt && nEnd == rNd.Len() )
            // all
            rArr.Insert( pArr, 0 );
        else
        {
            sal_uInt16 n;
            xub_StrLen nCntnt;
            for( n = 0; n < pArr->Count(); ++n )
            {
                void* p = (*pArr)[ n ];
                const ::sw::mark::IMark& rBkmk = *(::sw::mark::IMark *)p;
                if( rBkmk.GetMarkPos().nNode == nNd &&
                    (nCntnt = rBkmk.GetMarkPos().nContent.GetIndex() ) >= nStt &&
                    nCntnt < nEnd )
                {
                    rArr.Insert( p, rArr.Count() );
                }
                else if( rBkmk.IsExpanded() && nNd ==
                        rBkmk.GetOtherMarkPos().nNode.GetIndex() && (nCntnt =
                        rBkmk.GetOtherMarkPos().nContent.GetIndex() ) >= nStt &&
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

sal_uLong StgWriter::WriteStream()
{
    OSL_ENSURE( !this, "Schreiben in Streams auf einem Storage?" );
    return ERR_SWG_WRITE_ERROR;
}

sal_uLong StgWriter::Write( SwPaM& rPaM, SvStorage& rStg, const String* pFName )
{
    SetStream(0);
    pStg = &rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    sal_uLong nRet = WriteStorage();

    pStg = NULL;
    ResetWriter();

    return nRet;
}

sal_uLong StgWriter::Write( SwPaM& rPaM, const uno::Reference < embed::XStorage >& rStg, const String* pFName, SfxMedium* pMedium )
{
    SetStream(0);
    pStg = 0;
    xStg = rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;

    // PaM kopieren, damit er veraendert werden kann
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // zum Vergleich auf den akt. Pam sichern
    pOrigPam = &rPaM;

    sal_uLong nRet = pMedium ? WriteMedium( *pMedium ) : WriteStorage();

    pStg = NULL;
    ResetWriter();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
