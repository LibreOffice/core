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

#include <ftnidx.hxx>
#include <rootfrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <pam.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <UndoAttribute.hxx>
#include <hints.hxx>
#include <rolbck.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <ftninfo.hxx>

SwEndNoteInfo& SwEndNoteInfo::operator=(const SwEndNoteInfo& rInfo)
{
    pTextFormatColl = rInfo.pTextFormatColl;
    pPageDesc = rInfo.pPageDesc;
    pCharFormat = rInfo.pCharFormat;
    pAnchorFormat = rInfo.pAnchorFormat;
    aDepends.EndListeningAll();
    aDepends.StartListening(pTextFormatColl);
    aDepends.StartListening(pPageDesc);
    aDepends.StartListening(pCharFormat);
    aDepends.StartListening(pAnchorFormat);

    aFormat = rInfo.aFormat;
    nFootnoteOffset = rInfo.nFootnoteOffset;
    m_bEndNote = rInfo.m_bEndNote;
    sPrefix = rInfo.sPrefix;
    sSuffix = rInfo.sSuffix;
    return *this;
}

bool SwEndNoteInfo::operator==( const SwEndNoteInfo& rInfo ) const
{
    return
            pTextFormatColl == rInfo.pTextFormatColl &&
            pPageDesc == rInfo.pPageDesc &&
            pCharFormat == rInfo.pCharFormat &&
            pAnchorFormat == rInfo.pAnchorFormat &&
            aFormat.GetNumberingType() == rInfo.aFormat.GetNumberingType() &&
            nFootnoteOffset == rInfo.nFootnoteOffset &&
            m_bEndNote == rInfo.m_bEndNote &&
            sPrefix == rInfo.sPrefix &&
            sSuffix == rInfo.sSuffix;
}

SwEndNoteInfo::SwEndNoteInfo(const SwEndNoteInfo& rInfo) :
    SwClient(nullptr),
    aDepends(*this),
    pTextFormatColl(rInfo.pTextFormatColl),
    pPageDesc(rInfo.pPageDesc),
    pCharFormat(rInfo.pCharFormat),
    pAnchorFormat(rInfo.pAnchorFormat),
    sPrefix( rInfo.sPrefix ),
    sSuffix( rInfo.sSuffix ),
    m_bEndNote( true ),
    aFormat( rInfo.aFormat ),
    nFootnoteOffset( rInfo.nFootnoteOffset )
{
    aDepends.StartListening(pTextFormatColl);
    aDepends.StartListening(pPageDesc);
    aDepends.StartListening(pCharFormat);
    aDepends.StartListening(pAnchorFormat);
}

SwEndNoteInfo::SwEndNoteInfo() :
    SwClient(nullptr),
    aDepends(*this),
    pTextFormatColl(nullptr),
    pPageDesc(nullptr),
    pCharFormat(nullptr),
    pAnchorFormat(nullptr),
    m_bEndNote( true ),
    nFootnoteOffset( 0 )
{
    aFormat.SetNumberingType(SVX_NUM_ROMAN_LOWER);
}

SwPageDesc* SwEndNoteInfo::GetPageDesc(SwDoc& rDoc) const
{
    if(!pPageDesc)
    {
        pPageDesc = rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( static_cast<sal_uInt16>(
            m_bEndNote ? RES_POOLPAGE_ENDNOTE : RES_POOLPAGE_FOOTNOTE ) );
        aDepends.StartListening(pPageDesc);
    }
    return pPageDesc;
}

bool SwEndNoteInfo::KnowsPageDesc() const
{
    return pPageDesc != nullptr;
}

bool SwEndNoteInfo::DependsOn(const SwPageDesc* pDesc) const
{
    return pPageDesc == pDesc;
}

void SwEndNoteInfo::ChgPageDesc(SwPageDesc* pDesc)
{
    aDepends.EndListening(pPageDesc);
    pPageDesc = pDesc;
    aDepends.StartListening(pPageDesc);
}

void SwEndNoteInfo::SetFootnoteTextColl(SwTextFormatColl& rFormat)
{
    aDepends.EndListening(pTextFormatColl);
    pTextFormatColl = &rFormat;
    aDepends.StartListening(pTextFormatColl);
}

SwCharFormat* SwEndNoteInfo::GetCharFormat(SwDoc& rDoc) const
{
    auto pCharFormatFromDoc = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( static_cast<sal_uInt16>(
        m_bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE ) );
    if (pCharFormat != pCharFormatFromDoc)
    {
        aDepends.EndListening(pCharFormat);
        aDepends.StartListening(pCharFormatFromDoc);
        pCharFormat = pCharFormatFromDoc;
    }
    return pCharFormat;
}

namespace
{
    void lcl_ResetPoolIdForDocAndSync(const sal_uInt16 nId, SwCharFormat* pFormat, const SwEndNoteInfo& rInfo)
    {
        auto pDoc = pFormat->GetDoc();
        if(!pDoc)
            return;
        for(auto pDocFormat : *pDoc->GetCharFormats())
        {
            if(pDocFormat == pFormat)
                pDocFormat->SetPoolFormatId(nId);
            else if(pDocFormat->GetPoolFormatId() == nId)
                pDocFormat->SetPoolFormatId(0);
        }
        rInfo.GetCharFormat(*pDoc);
        rInfo.GetAnchorCharFormat(*pDoc);
    }
}

void SwEndNoteInfo::SetCharFormat(SwCharFormat* pFormat)
{
    lcl_ResetPoolIdForDocAndSync(
            static_cast<sal_uInt16>(m_bEndNote
                    ? RES_POOLCHR_ENDNOTE
                    : RES_POOLCHR_FOOTNOTE),
            pFormat,
            *this);
}

SwCharFormat* SwEndNoteInfo::GetAnchorCharFormat(SwDoc& rDoc) const
{
    auto pAnchorFormatFromDoc = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( static_cast<sal_uInt16>(
        m_bEndNote ? RES_POOLCHR_ENDNOTE_ANCHOR : RES_POOLCHR_FOOTNOTE_ANCHOR ) );
    if(pAnchorFormat != pAnchorFormatFromDoc)
    {
        aDepends.EndListening(pAnchorFormat);
        aDepends.StartListening(pAnchorFormatFromDoc);
        pAnchorFormat = pAnchorFormatFromDoc;
    }
    return pAnchorFormat;
}

void SwEndNoteInfo::SetAnchorCharFormat(SwCharFormat* pFormat)
{
    lcl_ResetPoolIdForDocAndSync(
            static_cast<sal_uInt16>(m_bEndNote
                    ? RES_POOLCHR_ENDNOTE_ANCHOR
                    : RES_POOLCHR_FOOTNOTE_ANCHOR),
            pFormat,
            *this);
}

SwCharFormat* SwEndNoteInfo::GetCurrentCharFormat(const bool bAnchor) const
{
    return bAnchor
        ? pAnchorFormat
        : pCharFormat;
}

void SwEndNoteInfo::SwClientNotify( const SwModify& rModify, const SfxHint& rHint)
{
    if (auto pLegacyHint = dynamic_cast<const sw::LegacyModifyHint*>(&rHint))
    {
        const sal_uInt16 nWhich = pLegacyHint->m_pOld ? pLegacyHint->m_pOld->Which() : pLegacyHint->m_pNew ? pLegacyHint->m_pNew->Which() : 0 ;
        if (RES_ATTRSET_CHG == nWhich || RES_FMT_CHG == nWhich)
        {
            auto pFormat = GetCurrentCharFormat(pCharFormat == nullptr);
            if (!pFormat || !aDepends.IsListeningTo(pFormat) || pFormat->IsFormatInDTOR())
                return;
            SwDoc* pDoc = pFormat->GetDoc();
            SwFootnoteIdxs& rFootnoteIdxs = pDoc->GetFootnoteIdxs();
            for( size_t nPos = 0; nPos < rFootnoteIdxs.size(); ++nPos )
            {
                SwTextFootnote *pTextFootnote = rFootnoteIdxs[ nPos ];
                const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
                if ( rFootnote.IsEndNote() == m_bEndNote )
                {
                    pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rFootnote.GetNumStr());
                }
            }
        }
        else
            CheckRegistration( pLegacyHint->m_pOld );
    }
    else if (auto pModifyChangedHint = dynamic_cast<const sw::ModifyChangedHint*>(&rHint))
    {
        auto pNew = const_cast<SwModify*>(pModifyChangedHint->m_pNew);
        if(pAnchorFormat == &rModify)
            pAnchorFormat = static_cast<SwCharFormat*>(pNew);
        else if(pCharFormat == &rModify)
            pCharFormat = static_cast<SwCharFormat*>(pNew);
        else if(pPageDesc == &rModify)
            pPageDesc = static_cast<SwPageDesc*>(pNew);
        else if(pTextFormatColl == &rModify)
            pTextFormatColl = static_cast<SwTextFormatColl*>(pNew);
    }
}

SwFootnoteInfo& SwFootnoteInfo::operator=(const SwFootnoteInfo& rInfo)
{
    SwEndNoteInfo::operator=(rInfo);
    aQuoVadis =  rInfo.aQuoVadis;
    aErgoSum = rInfo.aErgoSum;
    ePos = rInfo.ePos;
    eNum = rInfo.eNum;
    return *this;
}

bool SwFootnoteInfo::operator==( const SwFootnoteInfo& rInfo ) const
{
    return  ePos == rInfo.ePos &&
            eNum == rInfo.eNum &&
            SwEndNoteInfo::operator==(rInfo) &&
            aQuoVadis == rInfo.aQuoVadis &&
            aErgoSum == rInfo.aErgoSum;
}

SwFootnoteInfo::SwFootnoteInfo(const SwFootnoteInfo& rInfo) :
    SwEndNoteInfo( rInfo ),
    aQuoVadis( rInfo.aQuoVadis ),
    aErgoSum( rInfo.aErgoSum ),
    ePos( rInfo.ePos ),
    eNum( rInfo.eNum )
{
    m_bEndNote = false;
}

SwFootnoteInfo::SwFootnoteInfo() :
    SwEndNoteInfo(),
    ePos( FTNPOS_PAGE ),
    eNum( FTNNUM_DOC )
{
    aFormat.SetNumberingType(SVX_NUM_ARABIC);
    m_bEndNote = false;
}

void SwDoc::SetFootnoteInfo(const SwFootnoteInfo& rInfo)
{
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();
    if( GetFootnoteInfo() == rInfo )
        return;

    const SwFootnoteInfo &rOld = GetFootnoteInfo();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoFootNoteInfo>(rOld, this) );
    }

    bool bFootnotePos  = rInfo.ePos != rOld.ePos;
    bool bFootnoteDesc = rOld.ePos == FTNPOS_CHAPTER &&
                        rInfo.GetPageDesc( *this ) != rOld.GetPageDesc( *this );
    bool bExtra   = rInfo.aQuoVadis != rOld.aQuoVadis ||
                        rInfo.aErgoSum != rOld.aErgoSum ||
                        rInfo.aFormat.GetNumberingType() != rOld.aFormat.GetNumberingType() ||
                        rInfo.GetPrefix() != rOld.GetPrefix() ||
                        rInfo.GetSuffix() != rOld.GetSuffix();
    SwCharFormat *pOldChrFormat = rOld.GetCharFormat( *this ),
              *pNewChrFormat = rInfo.GetCharFormat( *this );
    bool bFootnoteChrFormats = pOldChrFormat != pNewChrFormat;

    *mpFootnoteInfo = rInfo;

    if (pTmpRoot)
    {
        std::set<SwRootFrame*> aAllLayouts = GetAllLayouts();
        if ( bFootnotePos )
            for( auto aLayout : aAllLayouts )
                aLayout->AllRemoveFootnotes();
        else
        {
            for( auto aLayout : aAllLayouts )
                aLayout->UpdateFootnoteNums();
            if ( bFootnoteDesc )
                for( auto aLayout : aAllLayouts )
                    aLayout->CheckFootnotePageDescs(false);
            if ( bExtra )
            {
                // For messages regarding ErgoSum etc. we save the extra code and use the
                // available methods.
                SwFootnoteIdxs& rFootnoteIdxs = GetFootnoteIdxs();
                for( size_t nPos = 0; nPos < rFootnoteIdxs.size(); ++nPos )
                {
                    SwTextFootnote *pTextFootnote = rFootnoteIdxs[ nPos ];
                    const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
                    if ( !rFootnote.IsEndNote() )
                        pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rFootnote.GetNumStr());
                }
            }
        }
    }
    if( FTNNUM_PAGE != rInfo.eNum )
        GetFootnoteIdxs().UpdateAllFootnote();
    else if( bFootnoteChrFormats )
    {
        SwFormatChg aOld( pOldChrFormat );
        SwFormatChg aNew( pNewChrFormat );
        mpFootnoteInfo->ModifyNotification( &aOld, &aNew );
    }

    // #i81002# no update during loading
    if ( !IsInReading() )
    {
        getIDocumentFieldsAccess().UpdateRefFields();
    }
    getIDocumentState().SetModified();

}

void SwDoc::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
{
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();
    if( GetEndNoteInfo() == rInfo )
        return;

    if(GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoEndNoteInfo>( GetEndNoteInfo(), this ) );
    }

    bool bNumChg  = rInfo.nFootnoteOffset != GetEndNoteInfo().nFootnoteOffset;
    // this seems to be an optimization: UpdateAllFootnote() is only called
    // if the offset changes; if the offset is the same,
    // but type/prefix/suffix changes, just set new numbers.
    bool const bExtra = !bNumChg &&
            (   (rInfo.aFormat.GetNumberingType() !=
                    GetEndNoteInfo().aFormat.GetNumberingType())
            ||  (rInfo.GetPrefix() != GetEndNoteInfo().GetPrefix())
            ||  (rInfo.GetSuffix() != GetEndNoteInfo().GetSuffix())
            );
    bool bFootnoteDesc = rInfo.GetPageDesc( *this ) !=
                        GetEndNoteInfo().GetPageDesc( *this );
    SwCharFormat *pOldChrFormat = GetEndNoteInfo().GetCharFormat( *this ),
              *pNewChrFormat = rInfo.GetCharFormat( *this );
    bool bFootnoteChrFormats = pOldChrFormat != pNewChrFormat;

    *mpEndNoteInfo = rInfo;

    if ( pTmpRoot )
    {
        if ( bFootnoteDesc )
        {
            for( auto aLayout : GetAllLayouts() )
                aLayout->CheckFootnotePageDescs(true);
        }
        if ( bExtra )
        {
            // For messages regarding ErgoSum etc. we save the extra code and use the
            // available methods.
            SwFootnoteIdxs& rFootnoteIdxs = GetFootnoteIdxs();
            for( size_t nPos = 0; nPos < rFootnoteIdxs.size(); ++nPos )
            {
                SwTextFootnote *pTextFootnote = rFootnoteIdxs[ nPos ];
                const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
                if ( rFootnote.IsEndNote() )
                    pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rFootnote.GetNumStr());
            }
        }
    }
    if( bNumChg )
        GetFootnoteIdxs().UpdateAllFootnote();
    else if( bFootnoteChrFormats )
    {
        SwFormatChg aOld( pOldChrFormat );
        SwFormatChg aNew( pNewChrFormat );
        mpEndNoteInfo->ModifyNotification( &aOld, &aNew );
    }

    // #i81002# no update during loading
    if ( !IsInReading() )
    {
        getIDocumentFieldsAccess().UpdateRefFields();
    }
    getIDocumentState().SetModified();

}

bool SwDoc::SetCurFootnote( const SwPaM& rPam, const OUString& rNumStr,
                       bool bIsEndNote)
{
    SwFootnoteIdxs& rFootnoteArr = GetFootnoteIdxs();
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();

    const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
    const sal_uLong nSttNd = pStt->nNode.GetIndex();
    const sal_Int32 nSttCnt = pStt->nContent.GetIndex();
    const sal_uLong nEndNd = pEnd->nNode.GetIndex();
    const sal_Int32 nEndCnt = pEnd->nContent.GetIndex();

    size_t nPos = 0;
    rFootnoteArr.SeekEntry( pStt->nNode, &nPos );

    std::unique_ptr<SwUndoChangeFootNote> pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo far below, so leave it
        pUndo.reset(new SwUndoChangeFootNote( rPam, rNumStr, bIsEndNote ));
    }

    SwTextFootnote* pTextFootnote;
    sal_uLong nIdx;
    bool bChg = false;
    bool bTypeChgd = false;
    const size_t nPosSave = nPos;
    while( nPos < rFootnoteArr.size() &&
            (( nIdx = SwTextFootnote_GetIndex((pTextFootnote = rFootnoteArr[ nPos++ ] )))
                < nEndNd || ( nIdx == nEndNd &&
                nEndCnt >= pTextFootnote->GetStart() )) )
        if( nIdx > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= pTextFootnote->GetStart() ) )
        {
            const SwFormatFootnote& rFootnote = pTextFootnote->GetFootnote();
            if( rFootnote.GetNumStr() != rNumStr ||
                rFootnote.IsEndNote() != bIsEndNote )
            {
                bChg = true;
                if ( pUndo )
                {
                    pUndo->GetHistory().Add( *pTextFootnote );
                }

                pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rNumStr);
                if( rFootnote.IsEndNote() != bIsEndNote )
                {
                    const_cast<SwFormatFootnote&>(rFootnote).SetEndNote( bIsEndNote );
                    bTypeChgd = true;
                    pTextFootnote->CheckCondColl();
                    //#i11339# dispose UNO wrapper when a footnote is changed to an endnote or vice versa
                    const_cast<SwFormatFootnote&>(rFootnote).InvalidateFootnote();
                }
            }
        }

    nPos = nPosSave;       // There are more in the front!
    while( nPos &&
            (( nIdx = SwTextFootnote_GetIndex((pTextFootnote = rFootnoteArr[ --nPos ] )))
                > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= pTextFootnote->GetStart() )) )
        if( nIdx < nEndNd || ( nIdx == nEndNd &&
            nEndCnt >= pTextFootnote->GetStart() ) )
        {
            const SwFormatFootnote& rFootnote = pTextFootnote->GetFootnote();
            if( rFootnote.GetNumStr() != rNumStr ||
                rFootnote.IsEndNote() != bIsEndNote )
            {
                bChg = true;
                if ( pUndo )
                {
                    pUndo->GetHistory().Add( *pTextFootnote );
                }

                pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rNumStr);
                if( rFootnote.IsEndNote() != bIsEndNote )
                {
                    const_cast<SwFormatFootnote&>(rFootnote).SetEndNote( bIsEndNote );
                    bTypeChgd = true;
                    pTextFootnote->CheckCondColl();
                }
            }
        }

    // Who needs to be triggered?
    if( bChg )
    {
        if( pUndo )
        {
            GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
        }

        if ( bTypeChgd )
            rFootnoteArr.UpdateAllFootnote();
        if( FTNNUM_PAGE != GetFootnoteInfo().eNum )
        {
            if ( !bTypeChgd )
                rFootnoteArr.UpdateAllFootnote();
        }
        else if( pTmpRoot )
        {
            for( auto aLayout : GetAllLayouts() )
                aLayout->UpdateFootnoteNums();
        }
        getIDocumentState().SetModified();
    }
    return bChg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
