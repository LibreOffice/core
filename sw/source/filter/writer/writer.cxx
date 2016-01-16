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

#include <hintids.hxx>

#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/eeitem.hxx>
#include <o3tl/make_unique.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IMark.hxx>
#include <IDocumentSettingAccess.hxx>
#include <numrule.hxx>
#include <swerror.h>

using namespace css;

namespace
{
    SvStream& lcl_OutLongExt( SvStream& rStrm, sal_uLong nVal, bool bNeg )
    {
        sal_Char aBuf[28];

        int i = SAL_N_ELEMENTS(aBuf);
        aBuf[--i] = 0;
        do
        {
            aBuf[--i] = '0' + static_cast<sal_Char>(nVal % 10);
            nVal /= 10;
        } while (nVal);

        if (bNeg)
            aBuf[--i] = '-';

        return rStrm.WriteCharPtr( &aBuf[i] );
    }
}

typedef std::multimap<sal_uLong, const ::sw::mark::IMark*> SwBookmarkNodeTable;

struct Writer_Impl
{
    SvStream * m_pStream;

    std::vector<const SvxFontItem*> aFontRemoveLst;
    SwBookmarkNodeTable aBkmkNodePos;

    Writer_Impl();
    ~Writer_Impl();

    void RemoveFontList( SwDoc& rDoc );
    void InsertBkmk( const ::sw::mark::IMark& rBkmk );
};

Writer_Impl::Writer_Impl()
    : m_pStream(nullptr)
{
}

Writer_Impl::~Writer_Impl()
{
}

void Writer_Impl::RemoveFontList( SwDoc& rDoc )
{
    for( std::vector<const SvxFontItem*>::const_iterator it = aFontRemoveLst.begin();
        it != aFontRemoveLst.end(); ++it )
    {
        rDoc.GetAttrPool().Remove( **it );
    }
}

void Writer_Impl::InsertBkmk(const ::sw::mark::IMark& rBkmk)
{
    sal_uLong nNd = rBkmk.GetMarkPos().nNode.GetIndex();

    aBkmkNodePos.insert( SwBookmarkNodeTable::value_type( nNd, &rBkmk ) );

    if(rBkmk.IsExpanded() && rBkmk.GetOtherMarkPos().nNode != nNd)
    {
        nNd = rBkmk.GetOtherMarkPos().nNode.GetIndex();
        aBkmkNodePos.insert( SwBookmarkNodeTable::value_type( nNd, &rBkmk ));
    }
}

/*
 * This module is the central collection point for all writer-filters
 * and is a DLL !
 *
 * So that the Writer can work with different writers, the output-functions
 * of the content carrying objects have to be mapped to the various
 * output-functions.
 *
 * For that, to inquire its output function, every object can be gripped
 * via the which-value in a table.
 * These functions are available in the corresponding Writer-DLL's.
 */

Writer::Writer()
    : m_pImpl(o3tl::make_unique<Writer_Impl>())
    , pOrigPam(nullptr), pOrigFileName(nullptr), pDoc(nullptr), pCurPam(nullptr)
{
    bWriteAll = bShowProgress = bUCS2_WithStartChar = true;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = false;
    bExportPargraphNumbering = true;
}

Writer::~Writer()
{
}

/*
 * Document Interface Access
 */
IDocumentSettingAccess& Writer::getIDocumentSettingAccess() { return pDoc->getIDocumentSettingAccess(); }
const IDocumentSettingAccess& Writer::getIDocumentSettingAccess() const { return pDoc->getIDocumentSettingAccess(); }
IDocumentStylePoolAccess& Writer::getIDocumentStylePoolAccess() { return pDoc->getIDocumentStylePoolAccess(); }
const IDocumentStylePoolAccess& Writer::getIDocumentStylePoolAccess() const { return pDoc->getIDocumentStylePoolAccess(); }

void Writer::ResetWriter()
{
    m_pImpl->RemoveFontList( *pDoc );
    m_pImpl.reset(new Writer_Impl);

    if( pCurPam )
    {
        while( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }
    pCurPam = nullptr;
    pOrigFileName = nullptr;
    pDoc = nullptr;

    bShowProgress = bUCS2_WithStartChar = true;
    bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
        bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
        bOrganizerMode = false;
}

bool Writer::CopyNextPam( SwPaM ** ppPam )
{
    if( (*ppPam)->GetNext() == pOrigPam )
    {
        *ppPam = pOrigPam;          // set back to the beginning pam
        return false;           // end of the ring
    }

    // otherwise copy the next value from the next Pam
    *ppPam = static_cast<SwPaM*>((*ppPam)->GetNext() );

    *pCurPam->GetPoint() = *(*ppPam)->Start();
    *pCurPam->GetMark() = *(*ppPam)->End();

    return true;
}

// search the next Bookmark-Position from the Bookmark-Table

sal_Int32 Writer::FindPos_Bkmk(const SwPosition& rPos) const
{
    const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    const IDocumentMarkAccess::const_iterator_t ppBkmk = ::std::lower_bound(
        pMarkAccess->getAllMarksBegin(),
        pMarkAccess->getAllMarksEnd(),
        rPos,
        sw::mark::CompareIMarkStartsBefore()); // find the first Mark that does not start before
    if(ppBkmk != pMarkAccess->getAllMarksEnd())
        return ppBkmk - pMarkAccess->getAllMarksBegin();
    return -1;
}

SwPaM *
Writer::NewSwPaM(SwDoc & rDoc, sal_uLong const nStartIdx, sal_uLong const nEndIdx)
{
    SwNodes *const pNds = &rDoc.GetNodes();

    SwNodeIndex aStt( *pNds, nStartIdx );
    SwContentNode* pCNode = aStt.GetNode().GetContentNode();
    if( !pCNode && nullptr == pNds->GoNext( &aStt ) )
    {
        OSL_FAIL( "No more ContentNode at StartPos" );
    }

    SwPaM* pNew = new SwPaM( aStt );
    pNew->SetMark();
    aStt = nEndIdx;
    pCNode = aStt.GetNode().GetContentNode();
    if (!pCNode)
        pCNode = SwNodes::GoPrevious(&aStt);
    assert(pCNode && "No more ContentNode at StartPos");
    pCNode->MakeEndIndex( &pNew->GetPoint()->nContent );
    pNew->GetPoint()->nNode = aStt;
    return pNew;
}

// Stream-specific
SvStream& Writer::Strm()
{
    assert(m_pImpl->m_pStream && "Oh-oh. Writer with no Stream!");
    return *m_pImpl->m_pStream;
}

void Writer::SetStream(SvStream *const pStream)
{
    m_pImpl->m_pStream = pStream;
}

SvStream& Writer::OutLong( SvStream& rStrm, long nVal )
{
    const bool bNeg = nVal < 0;
    if (bNeg)
        nVal = -nVal;

    return lcl_OutLongExt(rStrm, static_cast<sal_uLong>(nVal), bNeg);
}

SvStream& Writer::OutULong( SvStream& rStrm, sal_uLong nVal )
{
    return lcl_OutLongExt(rStrm, nVal, false);
}

sal_uLong Writer::Write( SwPaM& rPaM, SvStream& rStrm, const OUString* pFName )
{
    if ( IsStgWriter() )
    {
        sal_uLong nResult = ERRCODE_ABORT;
        try
        {
            tools::SvRef<SotStorage> aRef = new SotStorage( rStrm );
            nResult = Write( rPaM, *aRef, pFName );
            if ( nResult == ERRCODE_NONE )
                aRef->Commit();
        }
        catch (const css::ucb::ContentCreationException &e)
        {
            SAL_WARN("sw", "Writer::Write caught " << e.Message);
        }
        return nResult;
    }

    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;
    m_pImpl->m_pStream = &rStrm;

    // Copy PaM, so that it can be modified
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // for comparison secure to the current Pam
    pOrigPam = &rPaM;

    sal_uLong nRet = WriteStream();

    ResetWriter();

    return nRet;
}

void Writer::SetupFilterOptions(SfxMedium& /*rMedium*/)
{}

sal_uLong Writer::Write( SwPaM& rPam, SfxMedium& rMedium, const OUString* pFileName )
{
    SetupFilterOptions(rMedium);
    // This method must be overridden in SwXMLWriter a storage from medium will be used there.
    // The microsoft format can write to storage but the storage will be based on the stream.
    return Write( rPam, *rMedium.GetOutStream(), pFileName );
}

sal_uLong Writer::Write( SwPaM& /*rPam*/, SotStorage&, const OUString* )
{
    OSL_ENSURE( false, "Write in Storages on a stream?" );
    return ERR_SWG_WRITE_ERROR;
}

sal_uLong Writer::Write( SwPaM&, const uno::Reference < embed::XStorage >&, const OUString*, SfxMedium* )
{
    OSL_ENSURE( false, "Write in Storages on a stream?" );
    return ERR_SWG_WRITE_ERROR;
}

void Writer::PutNumFormatFontsInAttrPool()
{
    // then there are a few fonts in the NumRules
    // These put into the Pool. After this does they have a RefCount > 1
    // it can be removed - it is already in the Pool
    SfxItemPool& rPool = pDoc->GetAttrPool();
    const SwNumRuleTable& rListTable = pDoc->GetNumRuleTable();
    const SwNumRule* pRule;
    const SwNumFormat* pFormat;
    const vcl::Font* pFont;
    const vcl::Font* pDefFont = &numfunc::GetDefBulletFont();
    bool bCheck = false;

    for( size_t nGet = rListTable.size(); nGet; )
        if( SwDoc::IsUsed( *(pRule = rListTable[ --nGet ] )))
            for( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
                if( SVX_NUM_CHAR_SPECIAL == (pFormat = &pRule->Get( nLvl ))->GetNumberingType() ||
                    SVX_NUM_BITMAP == pFormat->GetNumberingType() )
                {
                    if( nullptr == ( pFont = pFormat->GetBulletFont() ) )
                        pFont = pDefFont;

                    if( bCheck )
                    {
                        if( *pFont == *pDefFont )
                            continue;
                    }
                    else if( *pFont == *pDefFont )
                        bCheck = true;

                    _AddFontItem( rPool, SvxFontItem( pFont->GetFamily(),
                                pFont->GetFamilyName(), pFont->GetStyleName(),
                                pFont->GetPitch(), pFont->GetCharSet(), RES_CHRATR_FONT ));
                }
}

void Writer::PutEditEngFontsInAttrPool( bool bIncl_CJK_CTL )
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

void Writer::_AddFontItems( SfxItemPool& rPool, sal_uInt16 nW )
{
    const SvxFontItem* pFont = static_cast<const SvxFontItem*>(&rPool.GetDefaultItem( nW ));
    _AddFontItem( rPool, *pFont );

    if( nullptr != ( pFont = static_cast<const SvxFontItem*>(rPool.GetPoolDefaultItem( nW ))) )
        _AddFontItem( rPool, *pFont );

    sal_uInt32 nMaxItem = rPool.GetItemCount2( nW );
    for( sal_uInt32 nGet = 0; nGet < nMaxItem; ++nGet )
        if( nullptr != (pFont = static_cast<const SvxFontItem*>(rPool.GetItem2( nW, nGet ))) )
            _AddFontItem( rPool, *pFont );
}

void Writer::_AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont )
{
    const SvxFontItem* pItem;
    if( RES_CHRATR_FONT != rFont.Which() )
    {
        SvxFontItem aFont( rFont );
        aFont.SetWhich( RES_CHRATR_FONT );
        pItem = static_cast<const SvxFontItem*>(&rPool.Put( aFont ));
    }
    else
        pItem = static_cast<const SvxFontItem*>(&rPool.Put( rFont ));

    if( 1 < pItem->GetRefCount() )
        rPool.Remove( *pItem );
    else
    {
        m_pImpl->aFontRemoveLst.push_back( pItem );
    }
}

// build a bookmark table, which is sort by the node position. The
// OtherPos of the bookmarks also inserted.
void Writer::CreateBookmarkTable()
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
bool Writer::GetBookmarks(const SwContentNode& rNd, sal_Int32 nStt,
    sal_Int32 nEnd, std::vector< const ::sw::mark::IMark* >& rArr)
{
    OSL_ENSURE( rArr.empty(), "there are still entries available" );

    sal_uLong nNd = rNd.GetIndex();
    std::pair<SwBookmarkNodeTable::const_iterator, SwBookmarkNodeTable::const_iterator> aIterPair
        = m_pImpl->aBkmkNodePos.equal_range( nNd );
    if( aIterPair.first != aIterPair.second )
    {
        // there exist some bookmarks, search now all which is in the range
        if( !nStt && nEnd == rNd.Len() )
            // all
            for( SwBookmarkNodeTable::const_iterator it = aIterPair.first; it != aIterPair.second; ++it )
                rArr.push_back( it->second );
        else
        {
            for( SwBookmarkNodeTable::const_iterator it = aIterPair.first; it != aIterPair.second; ++it )
            {
                const ::sw::mark::IMark& rBkmk = *(it->second);
                sal_Int32 nContent;
                if( rBkmk.GetMarkPos().nNode == nNd &&
                    (nContent = rBkmk.GetMarkPos().nContent.GetIndex() ) >= nStt &&
                    nContent < nEnd )
                {
                    rArr.push_back( &rBkmk );
                }
                else if( rBkmk.IsExpanded() && nNd ==
                        rBkmk.GetOtherMarkPos().nNode.GetIndex() && (nContent =
                        rBkmk.GetOtherMarkPos().nContent.GetIndex() ) >= nStt &&
                        nContent < nEnd )
                {
                    rArr.push_back( &rBkmk );
                }
            }
        }
    }
    return !rArr.empty();
}

// Storage-specific
sal_uLong StgWriter::WriteStream()
{
    OSL_ENSURE( false, "Write in Storages on a stream?" );
    return ERR_SWG_WRITE_ERROR;
}

sal_uLong StgWriter::Write( SwPaM& rPaM, SotStorage& rStg, const OUString* pFName )
{
    SetStream(nullptr);
    pStg = &rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;

    // Copy PaM, so that it can be modified
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // for comparison secure to the current Pam
    pOrigPam = &rPaM;

    sal_uLong nRet = WriteStorage();

    pStg = nullptr;
    ResetWriter();

    return nRet;
}

sal_uLong StgWriter::Write( SwPaM& rPaM, const uno::Reference < embed::XStorage >& rStg, const OUString* pFName, SfxMedium* pMedium )
{
    SetStream(nullptr);
    pStg = nullptr;
    xStg = rStg;
    pDoc = rPaM.GetDoc();
    pOrigFileName = pFName;

    // Copy PaM, so that it can be modified
    pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
    // for comparison secure to the current Pam
    pOrigPam = &rPaM;

    sal_uLong nRet = pMedium ? WriteMedium( *pMedium ) : WriteStorage();

    pStg = nullptr;
    ResetWriter();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
