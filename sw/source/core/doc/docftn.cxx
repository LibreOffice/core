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
#include <fmtftntx.hxx>
#include <unoprnms.hxx>

SwEndNoteInfo& SwEndNoteInfo::operator=(const SwEndNoteInfo& rInfo)
{
    m_pTextFormatColl = rInfo.m_pTextFormatColl;
    m_pPageDesc = rInfo.m_pPageDesc;
    m_pCharFormat = rInfo.m_pCharFormat;
    m_pAnchorFormat = rInfo.m_pAnchorFormat;
    m_aDepends.EndListeningAll();
    m_aDepends.StartListening(m_pTextFormatColl);
    m_aDepends.StartListening(m_pPageDesc);
    m_aDepends.StartListening(m_pCharFormat);
    m_aDepends.StartListening(m_pAnchorFormat);

    m_aFormat = rInfo.m_aFormat;
    m_nFootnoteOffset = rInfo.m_nFootnoteOffset;
    m_bEndNote = rInfo.m_bEndNote;
    m_sPrefix = rInfo.m_sPrefix;
    m_sSuffix = rInfo.m_sSuffix;
    return *this;
}

bool SwEndNoteInfo::operator==( const SwEndNoteInfo& rInfo ) const
{
    return
            m_pTextFormatColl == rInfo.m_pTextFormatColl &&
            m_pPageDesc == rInfo.m_pPageDesc &&
            m_pCharFormat == rInfo.m_pCharFormat &&
            m_pAnchorFormat == rInfo.m_pAnchorFormat &&
            m_aFormat.GetNumberingType() == rInfo.m_aFormat.GetNumberingType() &&
            m_nFootnoteOffset == rInfo.m_nFootnoteOffset &&
            m_bEndNote == rInfo.m_bEndNote &&
            m_sPrefix == rInfo.m_sPrefix &&
            m_sSuffix == rInfo.m_sSuffix;
}

SwEndNoteInfo::SwEndNoteInfo(const SwEndNoteInfo& rInfo) :
    SwClient(nullptr),
    m_aDepends(*this),
    m_pTextFormatColl(rInfo.m_pTextFormatColl),
    m_pPageDesc(rInfo.m_pPageDesc),
    m_pCharFormat(rInfo.m_pCharFormat),
    m_pAnchorFormat(rInfo.m_pAnchorFormat),
    m_sPrefix( rInfo.m_sPrefix ),
    m_sSuffix( rInfo.m_sSuffix ),
    m_bEndNote( true ),
    m_aFormat( rInfo.m_aFormat ),
    m_nFootnoteOffset( rInfo.m_nFootnoteOffset )
{
    m_aDepends.StartListening(m_pTextFormatColl);
    m_aDepends.StartListening(m_pPageDesc);
    m_aDepends.StartListening(m_pCharFormat);
    m_aDepends.StartListening(m_pAnchorFormat);
}

SwEndNoteInfo::SwEndNoteInfo() :
    SwClient(nullptr),
    m_aDepends(*this),
    m_pTextFormatColl(nullptr),
    m_pPageDesc(nullptr),
    m_pCharFormat(nullptr),
    m_pAnchorFormat(nullptr),
    m_bEndNote( true ),
    m_nFootnoteOffset( 0 )
{
    m_aFormat.SetNumberingType(SVX_NUM_ROMAN_LOWER);
}

SwPageDesc* SwEndNoteInfo::GetPageDesc(SwDoc& rDoc) const
{
    if(!m_pPageDesc)
    {
        m_pPageDesc = rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( o3tl::narrowing<sal_uInt16>(
            m_bEndNote ? RES_POOLPAGE_ENDNOTE : RES_POOLPAGE_FOOTNOTE ) );
        m_aDepends.StartListening(m_pPageDesc);
    }
    return m_pPageDesc;
}

bool SwEndNoteInfo::KnowsPageDesc() const
{
    return m_pPageDesc != nullptr;
}

bool SwEndNoteInfo::DependsOn(const SwPageDesc* pDesc) const
{
    return m_pPageDesc == pDesc;
}

void SwEndNoteInfo::ChgPageDesc(SwPageDesc* pDesc)
{
    m_aDepends.EndListening(m_pPageDesc);
    m_pPageDesc = pDesc;
    m_aDepends.StartListening(m_pPageDesc);
}

SwSection* SwEndNoteInfo::GetSwSection(SwDoc& rDoc) const
{
    if (!m_pSwSection)
    {
        SwSectionFormat* pFormat = rDoc.MakeSectionFormat();
        pFormat->SetFormatName(UNO_NAME_ENDNOTE);
        pFormat->SetFormatAttr(SwFormatEndAtTextEnd(FTNEND_ATTXTEND));
        m_pSwSection.reset(new SwSection(SectionType::Content, pFormat->GetName(), *pFormat));
    }
    return m_pSwSection.get();
}

void SwEndNoteInfo::ResetSwSection()
{
    m_pSwSection.reset();
}

void SwEndNoteInfo::SetFootnoteTextColl(SwTextFormatColl& rFormat)
{
    m_aDepends.EndListening(m_pTextFormatColl);
    m_pTextFormatColl = &rFormat;
    m_aDepends.StartListening(m_pTextFormatColl);
}

SwCharFormat* SwEndNoteInfo::GetCharFormat(SwDoc& rDoc) const
{
    auto pCharFormatFromDoc = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( o3tl::narrowing<sal_uInt16>(
        m_bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE ) );
    if (m_pCharFormat != pCharFormatFromDoc)
    {
        m_aDepends.EndListening(m_pCharFormat);
        m_aDepends.StartListening(pCharFormatFromDoc);
        m_pCharFormat = pCharFormatFromDoc;
    }
    return m_pCharFormat;
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
            o3tl::narrowing<sal_uInt16>(m_bEndNote
                    ? RES_POOLCHR_ENDNOTE
                    : RES_POOLCHR_FOOTNOTE),
            pFormat,
            *this);
}

SwCharFormat* SwEndNoteInfo::GetAnchorCharFormat(SwDoc& rDoc) const
{
    auto pAnchorFormatFromDoc = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( o3tl::narrowing<sal_uInt16>(
        m_bEndNote ? RES_POOLCHR_ENDNOTE_ANCHOR : RES_POOLCHR_FOOTNOTE_ANCHOR ) );
    if(m_pAnchorFormat != pAnchorFormatFromDoc)
    {
        m_aDepends.EndListening(m_pAnchorFormat);
        m_aDepends.StartListening(pAnchorFormatFromDoc);
        m_pAnchorFormat = pAnchorFormatFromDoc;
    }
    return m_pAnchorFormat;
}

void SwEndNoteInfo::SetAnchorCharFormat(SwCharFormat* pFormat)
{
    lcl_ResetPoolIdForDocAndSync(
            o3tl::narrowing<sal_uInt16>(m_bEndNote
                    ? RES_POOLCHR_ENDNOTE_ANCHOR
                    : RES_POOLCHR_FOOTNOTE_ANCHOR),
            pFormat,
            *this);
}

SwCharFormat* SwEndNoteInfo::GetCurrentCharFormat(const bool bAnchor) const
{
    return bAnchor
        ? m_pAnchorFormat
        : m_pCharFormat;
}

void SwEndNoteInfo::UpdateFormatOrAttr()
{
    auto pFormat = GetCurrentCharFormat(m_pCharFormat == nullptr);
    if (!pFormat || !m_aDepends.IsListeningTo(pFormat) || pFormat->IsFormatInDTOR())
        return;
    SwDoc* pDoc = pFormat->GetDoc();
    SwFootnoteIdxs& rFootnoteIdxs = pDoc->GetFootnoteIdxs();
    for(auto pTextFootnote : rFootnoteIdxs)
    {
        const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
        if(rFootnote.IsEndNote() == m_bEndNote)
            pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rFootnote.GetNumStr());
    }
}


void SwEndNoteInfo::SwClientNotify( const SwModify& rModify, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
        switch(pLegacyHint->GetWhich())
        {
            case RES_ATTRSET_CHG:
            case RES_FMT_CHG:
                UpdateFormatOrAttr();
                break;
            default:
                CheckRegistration( pLegacyHint->m_pOld );
        }
    }
    else if (rHint.GetId() == SfxHintId::SwModifyChanged)
    {
        auto pModifyChangedHint = static_cast<const sw::ModifyChangedHint*>(&rHint);
        auto pNew = const_cast<sw::BroadcastingModify*>(static_cast<const sw::BroadcastingModify*>(pModifyChangedHint->m_pNew));
        if(m_pAnchorFormat == &rModify)
            m_pAnchorFormat = static_cast<SwCharFormat*>(pNew);
        else if(m_pCharFormat == &rModify)
            m_pCharFormat = static_cast<SwCharFormat*>(pNew);
        else if(m_pPageDesc == &rModify)
            m_pPageDesc = static_cast<SwPageDesc*>(pNew);
        else if(m_pTextFormatColl == &rModify)
            m_pTextFormatColl = static_cast<SwTextFormatColl*>(pNew);
    }
}

SwFootnoteInfo& SwFootnoteInfo::operator=(const SwFootnoteInfo& rInfo)
{
    SwEndNoteInfo::operator=(rInfo);
    m_aQuoVadis =  rInfo.m_aQuoVadis;
    m_aErgoSum = rInfo.m_aErgoSum;
    m_ePos = rInfo.m_ePos;
    m_eNum = rInfo.m_eNum;
    return *this;
}

bool SwFootnoteInfo::operator==( const SwFootnoteInfo& rInfo ) const
{
    return  m_ePos == rInfo.m_ePos &&
            m_eNum == rInfo.m_eNum &&
            SwEndNoteInfo::operator==(rInfo) &&
            m_aQuoVadis == rInfo.m_aQuoVadis &&
            m_aErgoSum == rInfo.m_aErgoSum;
}

SwFootnoteInfo::SwFootnoteInfo(const SwFootnoteInfo& rInfo) :
    SwEndNoteInfo( rInfo ),
    m_aQuoVadis( rInfo.m_aQuoVadis ),
    m_aErgoSum( rInfo.m_aErgoSum ),
    m_ePos( rInfo.m_ePos ),
    m_eNum( rInfo.m_eNum )
{
    m_bEndNote = false;
}

SwFootnoteInfo::SwFootnoteInfo() :
    m_ePos( FTNPOS_PAGE ),
    m_eNum( FTNNUM_DOC )
{
    m_aFormat.SetNumberingType(SVX_NUM_ARABIC);
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
        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoFootNoteInfo>(rOld, *this) );
    }

    bool bFootnotePos  = rInfo.m_ePos != rOld.m_ePos;
    bool bFootnoteDesc = rOld.m_ePos == FTNPOS_CHAPTER &&
                        rInfo.GetPageDesc( *this ) != rOld.GetPageDesc( *this );
    bool bExtra   = rInfo.m_aQuoVadis != rOld.m_aQuoVadis ||
                        rInfo.m_aErgoSum != rOld.m_aErgoSum ||
                        rInfo.m_aFormat.GetNumberingType() != rOld.m_aFormat.GetNumberingType() ||
                        rInfo.GetPrefix() != rOld.GetPrefix() ||
                        rInfo.GetSuffix() != rOld.GetSuffix();
    SwCharFormat *pOldChrFormat = rOld.GetCharFormat( *this ),
              *pNewChrFormat = rInfo.GetCharFormat( *this );
    bool bFootnoteChrFormats = pOldChrFormat != pNewChrFormat;

    *mpFootnoteInfo = rInfo;

    if (pTmpRoot)
    {
        o3tl::sorted_vector<SwRootFrame*> aAllLayouts = GetAllLayouts();
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
    if( FTNNUM_PAGE != rInfo.m_eNum )
        GetFootnoteIdxs().UpdateAllFootnote();
    else if( bFootnoteChrFormats )
    {
        mpFootnoteInfo->UpdateFormatOrAttr();
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
            std::make_unique<SwUndoEndNoteInfo>( GetEndNoteInfo(), *this ) );
    }

    bool bNumChg  = rInfo.m_nFootnoteOffset != GetEndNoteInfo().m_nFootnoteOffset;
    // this seems to be an optimization: UpdateAllFootnote() is only called
    // if the offset changes; if the offset is the same,
    // but type/prefix/suffix changes, just set new numbers.
    bool const bExtra = !bNumChg &&
            (   (rInfo.m_aFormat.GetNumberingType() !=
                    GetEndNoteInfo().m_aFormat.GetNumberingType())
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
        mpEndNoteInfo->UpdateFormatOrAttr();
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

    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*
    const SwNodeOffset nSttNd = pStt->GetNodeIndex();
    const sal_Int32 nSttCnt = pStt->GetContentIndex();
    const SwNodeOffset nEndNd = pEnd->GetNodeIndex();
    const sal_Int32 nEndCnt = pEnd->GetContentIndex();

    size_t nPos = 0;
    rFootnoteArr.SeekEntry( pStt->GetNode(), &nPos );

    std::unique_ptr<SwUndoChangeFootNote> pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo far below, so leave it
        pUndo.reset(new SwUndoChangeFootNote( rPam, rNumStr, bIsEndNote ));
    }

    bool bChg = false;
    bool bTypeChgd = false;
    const size_t nPosSave = nPos;
    while( nPos < rFootnoteArr.size() )
    {
        SwTextFootnote* pTextFootnote = rFootnoteArr[ nPos++ ];
        SwNodeOffset nIdx = SwTextFootnote_GetIndex(pTextFootnote);
        if( nIdx >= nEndNd &&
            ( nIdx != nEndNd || nEndCnt < pTextFootnote->GetStart() ) )
            continue;
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
                    pUndo->GetHistory().AddFootnote(*pTextFootnote);
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
    }

    nPos = nPosSave;       // There are more in the front!
    while( nPos )
    {
        SwTextFootnote* pTextFootnote = rFootnoteArr[ --nPos ];
        SwNodeOffset nIdx = SwTextFootnote_GetIndex(pTextFootnote);
        if( nIdx <= nSttNd &&
            ( nIdx != nSttNd || nSttCnt > pTextFootnote->GetStart() ) )
            continue;
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
                    pUndo->GetHistory().AddFootnote(*pTextFootnote);
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
        if( FTNNUM_PAGE != GetFootnoteInfo().m_eNum )
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
