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

#include <algorithm>

#include <com/sun/star/i18n/ScriptType.hpp>

#include <editeng/langitem.hxx>
#include <osl/diagnose.h>
#include <svl/languageoptions.hxx>
#include <vcl/commandevent.hxx>

#include <hintids.hxx>
#include <extinput.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <index.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>

using namespace ::com::sun::star;

SwExtTextInput::SwExtTextInput( const SwPaM& rPam, Ring* pRing )
    : SwPaM( *rPam.GetPoint(), static_cast<SwPaM*>(pRing) ),
    m_eInputLanguage(LANGUAGE_DONTKNOW)
{
    m_bIsOverwriteCursor = false;
    m_bInsText = true;
}

SwExtTextInput::~SwExtTextInput()
{
    SwDoc& rDoc = GetDoc();
    if (rDoc.IsInDtor()) { return; /* #i58606# */ }

    SwTextNode* pTNd = GetPoint()->nNode.GetNode().GetTextNode();
    if( !pTNd )
        return;

    SwIndex& rIdx = GetPoint()->nContent;
    sal_Int32 nSttCnt = rIdx.GetIndex();
    sal_Int32 nEndCnt = GetMark()->nContent.GetIndex();
    if( nEndCnt == nSttCnt )
        return;

    // Prevent IME edited text being grouped with non-IME edited text.
    bool bKeepGroupUndo = rDoc.GetIDocumentUndoRedo().DoesGroupUndo();
    rDoc.GetIDocumentUndoRedo().DoGroupUndo(false);
    if( nEndCnt < nSttCnt )
    {
        std::swap(nSttCnt, nEndCnt);
    }

    // In order to get Undo/Redlining etc. working correctly,
    // we need to go through the Doc interface
    rIdx = nSttCnt;
    const OUString sText( pTNd->GetText().copy(nSttCnt, nEndCnt - nSttCnt));
    if( m_bIsOverwriteCursor && !m_sOverwriteText.isEmpty() )
    {
        const sal_Int32 nLen = sText.getLength();
        const sal_Int32 nOWLen = m_sOverwriteText.getLength();
        if( nLen > nOWLen )
        {
            rIdx += nOWLen;
            pTNd->EraseText( rIdx, nLen - nOWLen );
            rIdx = nSttCnt;
            pTNd->ReplaceText( rIdx, nOWLen, m_sOverwriteText );
            if( m_bInsText )
            {
                rIdx = nSttCnt;
                rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::OVERWRITE, nullptr );
                rDoc.getIDocumentContentOperations().Overwrite( *this, sText.copy( 0, nOWLen ) );
                rDoc.getIDocumentContentOperations().InsertString( *this, sText.copy( nOWLen ) );
                rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::OVERWRITE, nullptr );
            }
        }
        else
        {
            pTNd->ReplaceText( rIdx, nLen, m_sOverwriteText.copy( 0, nLen ));
            if( m_bInsText )
            {
                rIdx = nSttCnt;
                rDoc.getIDocumentContentOperations().Overwrite( *this, sText );
            }
        }
    }
    else
    {
        pTNd->EraseText( rIdx, nEndCnt - nSttCnt );

        if( m_bInsText )
        {
            rDoc.getIDocumentContentOperations().InsertString( *this, sText );
        }
    }
    rDoc.GetIDocumentUndoRedo().DoGroupUndo(bKeepGroupUndo);
    if (m_eInputLanguage == LANGUAGE_DONTKNOW)
        return;

    sal_uInt16 nWhich = RES_CHRATR_LANGUAGE;
    sal_Int16 nScriptType = SvtLanguageOptions::GetI18NScriptTypeOfLanguage(m_eInputLanguage);
    switch(nScriptType)
    {
        case  i18n::ScriptType::ASIAN:
            nWhich = RES_CHRATR_CJK_LANGUAGE; break;
        case  i18n::ScriptType::COMPLEX:
            nWhich = RES_CHRATR_CTL_LANGUAGE; break;
    }
    // #i41974# Only set language attribute for CJK/CTL scripts.
    if (RES_CHRATR_LANGUAGE != nWhich && pTNd->GetLang( nSttCnt, nEndCnt-nSttCnt, nScriptType) != m_eInputLanguage)
    {
        SvxLanguageItem aLangItem( m_eInputLanguage, nWhich );
        rIdx = nSttCnt;
        GetMark()->nContent = nEndCnt;
        rDoc.getIDocumentContentOperations().InsertPoolItem(*this, aLangItem );
    }
}

void SwExtTextInput::SetInputData( const CommandExtTextInputData& rData )
{
    SwTextNode* pTNd = GetPoint()->nNode.GetNode().GetTextNode();
    if( !pTNd )
        return;

    sal_Int32 nSttCnt = GetPoint()->nContent.GetIndex();
    sal_Int32 nEndCnt = GetMark()->nContent.GetIndex();
    if( nEndCnt < nSttCnt )
    {
        std::swap(nSttCnt, nEndCnt);
    }

    SwIndex aIdx( pTNd, nSttCnt );
    const OUString& rNewStr = rData.GetText();

    if( m_bIsOverwriteCursor && !m_sOverwriteText.isEmpty() )
    {
        sal_Int32 nReplace = nEndCnt - nSttCnt;
        const sal_Int32 nNewLen = rNewStr.getLength();
        if( nNewLen < nReplace )
        {
            // We have to insert some characters from the saved original text
            nReplace -= nNewLen;
            aIdx += nNewLen;
            pTNd->ReplaceText( aIdx, nReplace,
                        m_sOverwriteText.copy( nNewLen, nReplace ));
            aIdx = nSttCnt;
            nReplace = nNewLen;
        }
        else
        {
            const sal_Int32 nOWLen = m_sOverwriteText.getLength();
            if( nOWLen < nReplace )
            {
                aIdx += nOWLen;
                pTNd->EraseText( aIdx, nReplace-nOWLen );
                aIdx = nSttCnt;
                nReplace = nOWLen;
            }
            else
            {
                nReplace = std::min(nOWLen, nNewLen);
            }
        }

        pTNd->ReplaceText( aIdx, nReplace, rNewStr );
        if( !HasMark() )
            SetMark();
        GetMark()->nContent = aIdx;
    }
    else
    {
        if( nSttCnt < nEndCnt )
        {
            pTNd->EraseText( aIdx, nEndCnt - nSttCnt );
        }

        pTNd->InsertText( rNewStr, aIdx,
                SwInsertFlags::EMPTYEXPAND );
        if( !HasMark() )
            SetMark();
    }

    GetPoint()->nContent = nSttCnt;

    m_aAttrs.clear();
    if( rData.GetTextAttr() )
    {
        const ExtTextInputAttr *pAttrs = rData.GetTextAttr();
        m_aAttrs.insert( m_aAttrs.begin(), pAttrs, pAttrs + rData.GetText().getLength() );
    }
}

void SwExtTextInput::SetOverwriteCursor( bool bFlag )
{
    m_bIsOverwriteCursor = bFlag;
    if (!m_bIsOverwriteCursor)
        return;

    const SwTextNode *const pTNd = GetPoint()->nNode.GetNode().GetTextNode();
    if (!pTNd)
        return;

    const sal_Int32 nSttCnt = GetPoint()->nContent.GetIndex();
    const sal_Int32 nEndCnt = GetMark()->nContent.GetIndex();
    m_sOverwriteText = pTNd->GetText().copy( std::min(nSttCnt, nEndCnt) );
    if( m_sOverwriteText.isEmpty() )
        return;

    const sal_Int32 nInPos = m_sOverwriteText.indexOf( CH_TXTATR_INWORD );
    const sal_Int32 nBrkPos = m_sOverwriteText.indexOf( CH_TXTATR_BREAKWORD );

    // Find the first attr found, if any.
    sal_Int32 nPos = std::min(nInPos, nBrkPos);
    if (nPos<0)
    {
        nPos = std::max(nInPos, nBrkPos);
    }
    if (nPos>=0)
    {
        m_sOverwriteText = m_sOverwriteText.copy( 0, nPos );
    }
}

// The Doc interfaces

SwExtTextInput* SwDoc::CreateExtTextInput( const SwPaM& rPam )
{
    SwExtTextInput* pNew = new SwExtTextInput( rPam, mpExtInputRing );
    if( !mpExtInputRing )
        mpExtInputRing = pNew;
    pNew->SetMark();
    return pNew;
}

void SwDoc::DeleteExtTextInput( SwExtTextInput* pDel )
{
    if( pDel == mpExtInputRing )
    {
        if( pDel->GetNext() != mpExtInputRing )
            mpExtInputRing = pDel->GetNext();
        else
            mpExtInputRing = nullptr;
    }
    delete pDel;
}

SwExtTextInput* SwDoc::GetExtTextInput( const SwNode& rNd,
                                        sal_Int32 nContentPos ) const
{
    SwExtTextInput* pRet = nullptr;
    if( mpExtInputRing )
    {
        sal_uLong nNdIdx = rNd.GetIndex();
        SwExtTextInput* pTmp = mpExtInputRing;
        do {
            sal_uLong nPt = pTmp->GetPoint()->nNode.GetIndex(),
                  nMk = pTmp->GetMark()->nNode.GetIndex();
            sal_Int32 nPtCnt = pTmp->GetPoint()->nContent.GetIndex();
            sal_Int32 nMkCnt = pTmp->GetMark()->nContent.GetIndex();

            if( nPt < nMk || ( nPt == nMk && nPtCnt < nMkCnt ))
            {
                sal_uLong nTmp = nMk; nMk = nPt; nPt = nTmp;
                sal_Int32 nTmp2 = nMkCnt; nMkCnt = nPtCnt; nPtCnt = nTmp2;
            }

            if( nMk <= nNdIdx && nNdIdx <= nPt &&
                ( nContentPos<0 ||
                    ( nMkCnt <= nContentPos && nContentPos <= nPtCnt )))
            {
                pRet = pTmp;
                break;
            }
            pTmp = pTmp->GetNext();
        } while ( pTmp!=mpExtInputRing );
    }
    return pRet;
}

SwExtTextInput* SwDoc::GetExtTextInput() const
{
    OSL_ENSURE( !mpExtInputRing || !mpExtInputRing->IsMultiSelection(),
            "more than one InputEngine available" );
    return mpExtInputRing;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
