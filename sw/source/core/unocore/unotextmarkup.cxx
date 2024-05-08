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

#include <unotextmarkup.hxx>

#include <comphelper/servicehelper.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <svl/listener.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <SwSmartTagMgr.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/text/TextMarkupDescriptor.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XStringKeyMap.hpp>
#include <ndtxt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <TextCursorHelper.hxx>
#include <GrammarContact.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <pam.hxx>

#include <unotextrange.hxx>
#include <modeltoviewhelper.hxx>

using namespace ::com::sun::star;

struct SwXTextMarkup::Impl
    : public SvtListener
{
    SwTextNode* m_pTextNode;
    ModelToViewHelper const m_ConversionMap;

    Impl(SwTextNode* const pTextNode, ModelToViewHelper aMap)
        : m_pTextNode(pTextNode)
        , m_ConversionMap(std::move(aMap))
    {
        if(m_pTextNode)
            StartListening(pTextNode->GetNotifier());
    }

    virtual void Notify(const SfxHint& rHint) override;
};

SwXTextMarkup::SwXTextMarkup(
        SwTextNode *const pTextNode, const ModelToViewHelper& rMap)
    : m_pImpl(new Impl(pTextNode, rMap))
{
}

SwXTextMarkup::~SwXTextMarkup()
{
}

SwTextNode* SwXTextMarkup::GetTextNode()
{
    return m_pImpl->m_pTextNode;
}

void SwXTextMarkup::ClearTextNode()
{
    m_pImpl->m_pTextNode = nullptr;
    m_pImpl->EndListeningAll();
}

const ModelToViewHelper& SwXTextMarkup::GetConversionMap() const
{
    return m_pImpl->m_ConversionMap;
}

uno::Reference< container::XStringKeyMap > SAL_CALL SwXTextMarkup::getMarkupInfoContainer()
{
    return new SwXStringKeyMap;
}

void SAL_CALL SwXTextMarkup::commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const uno::Reference< text::XTextRange> & xRange,
                                                   const uno::Reference< container::XStringKeyMap > & xMarkupInfoContainer)
{
    SolarMutexGuard aGuard;

    if (auto pRange = dynamic_cast<SwXTextRange*>(xRange.get()))
    {
        SwDoc& rDoc = pRange->GetDoc();

        SwUnoInternalPaM aPam(rDoc);

        ::sw::XTextRangeToSwPaM(aPam, xRange);

        auto [startPos, endPos] = aPam.StartEnd(); // SwPosition*

        commitStringMarkup (nType, aIdentifier, startPos->GetContentIndex(), endPos->GetContentIndex() - startPos->GetContentIndex(), xMarkupInfoContainer);
    }
    else if (auto pCursor = dynamic_cast<OTextCursorHelper*>(xRange.get()))
    {
        SwPaM & rPam(*pCursor->GetPaM());

        auto [startPos, endPos] = rPam.StartEnd(); // SwPosition*

        commitStringMarkup (nType, aIdentifier, startPos->GetContentIndex(), endPos->GetContentIndex() - startPos->GetContentIndex(), xMarkupInfoContainer);
    }
}

void SAL_CALL SwXTextMarkup::commitStringMarkup(
    ::sal_Int32 nType,
    const OUString & rIdentifier,
    ::sal_Int32 nStart,
    ::sal_Int32 nLength,
    const uno::Reference< container::XStringKeyMap > & xMarkupInfoContainer)
{
    SolarMutexGuard aGuard;

    // paragraph already dead or modified?
    if (!m_pImpl->m_pTextNode || nLength <= 0)
        return;

    if ( nType == text::TextMarkupType::SMARTTAG &&
        !SwSmartTagMgr::Get().IsSmartTagTypeEnabled( rIdentifier ) )
        return;

    // get appropriate list to use...
    SwWrongList* pWList = nullptr;
    bool bRepaint = false;
    if ( nType == text::TextMarkupType::SPELLCHECK )
    {
        pWList = m_pImpl->m_pTextNode->GetWrong();
        if ( !pWList )
        {
            pWList = new SwWrongList( WRONGLIST_SPELL );
            m_pImpl->m_pTextNode->SetWrong( std::unique_ptr<SwWrongList>(pWList) );
        }
    }
    else if ( nType == text::TextMarkupType::PROOFREADING || nType == text::TextMarkupType::SENTENCE )
    {
        sw::GrammarContact* pGrammarContact = sw::getGrammarContactFor(*m_pImpl->m_pTextNode);
        if( pGrammarContact )
        {
            pWList = pGrammarContact->getGrammarCheck(*m_pImpl->m_pTextNode, true);
            assert(pWList && "GrammarContact _has_ to deliver a wrong list");
        }
        else
        {
            pWList = m_pImpl->m_pTextNode->GetGrammarCheck();
            if ( !pWList )
            {
                m_pImpl->m_pTextNode->SetGrammarCheck( std::make_unique<SwGrammarMarkUp>() );
                pWList = m_pImpl->m_pTextNode->GetGrammarCheck();
            }
        }
        bRepaint = pWList == m_pImpl->m_pTextNode->GetGrammarCheck();
        if( pWList->GetBeginInv() < COMPLETE_STRING )
            static_cast<SwGrammarMarkUp*>(pWList)->ClearGrammarList();
    }
    else if ( nType == text::TextMarkupType::SMARTTAG )
    {
        pWList = m_pImpl->m_pTextNode->GetSmartTags();
        if ( !pWList )
        {
            pWList = new SwWrongList( WRONGLIST_SMARTTAG );
            m_pImpl->m_pTextNode->SetSmartTags( std::unique_ptr<SwWrongList>(pWList) );
        }
    }
    else
    {
        OSL_FAIL( "Unknown mark-up type" );
        return;
    }

    const ModelToViewHelper::ModelPosition aStartPos =
        m_pImpl->m_ConversionMap.ConvertToModelPosition( nStart );
    const ModelToViewHelper::ModelPosition aEndPos   =
        m_pImpl->m_ConversionMap.ConvertToModelPosition( nStart + nLength - 1);

    const bool bStartInField = aStartPos.mbIsField;
    const bool bEndInField   = aEndPos.mbIsField;
    bool bCommit = false;

    if ( bStartInField && bEndInField && aStartPos.mnPos == aEndPos.mnPos )
    {
        nStart = aStartPos.mnSubPos;
        const sal_Int32 nFieldPosModel = aStartPos.mnPos;
        const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );

        SwWrongList* pSubList = pWList->SubList( nInsertPos );
        if ( !pSubList )
        {
            if( nType == text::TextMarkupType::PROOFREADING || nType == text::TextMarkupType::SENTENCE )
                pSubList = new SwGrammarMarkUp();
            else
                pSubList = new SwWrongList( pWList->GetWrongListType() );
            pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
        }

        pWList = pSubList;
        bCommit = true;
    }
    else if ( !bStartInField && !bEndInField )
    {
        nStart = aStartPos.mnPos;
        bCommit = true;
        nLength = aEndPos.mnPos + 1 - aStartPos.mnPos;
    }
    else if( nType == text::TextMarkupType::PROOFREADING || nType == text::TextMarkupType::SENTENCE )
    {
        bCommit = true;
        nStart = aStartPos.mnPos;
        sal_Int32 nEnd = aEndPos.mnPos;
        if( bStartInField && nType != text::TextMarkupType::SENTENCE )
        {
            const sal_Int32 nFieldPosModel = aStartPos.mnPos;
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwWrongList* pSubList = pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_Int32 nTmpStart =
                m_pImpl->m_ConversionMap.ConvertToViewPosition(aStartPos.mnPos);
            const sal_Int32 nTmpLen =
                m_pImpl->m_ConversionMap.ConvertToViewPosition(aStartPos.mnPos + 1)
                                       - nTmpStart - aStartPos.mnSubPos;
            if( nTmpLen > 0 )
            {
                pSubList->Insert( rIdentifier, xMarkupInfoContainer, aStartPos.mnSubPos, nTmpLen );
            }
            ++nStart;
        }
        if( bEndInField && nType != text::TextMarkupType::SENTENCE )
        {
            const sal_Int32 nFieldPosModel = aEndPos.mnPos;
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwWrongList* pSubList = pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_Int32 nTmpLen = aEndPos.mnSubPos + 1;
            pSubList->Insert( rIdentifier, xMarkupInfoContainer, 0, nTmpLen );
        }
        else
            ++nEnd;
        if( nEnd > nStart )
            nLength = nEnd - nStart;
        else
            bCommit = false;
    }

    if ( bCommit )
    {
        if( nType == text::TextMarkupType::SENTENCE )
            static_cast<SwGrammarMarkUp*>(pWList)->setSentence( nStart );
        else
            pWList->Insert( rIdentifier, xMarkupInfoContainer, nStart, nLength );
    }

    if( bRepaint )
        sw::finishGrammarCheckFor(*m_pImpl->m_pTextNode);
}

static void lcl_commitGrammarMarkUp(
    const ModelToViewHelper& rConversionMap,
    SwGrammarMarkUp* pWList,
    ::sal_Int32 nType,
    const OUString & rIdentifier,
    ::sal_Int32 nStart,
    ::sal_Int32 nLength,
    const uno::Reference< container::XStringKeyMap > & xMarkupInfoContainer)
{
    OSL_ENSURE( nType == text::TextMarkupType::PROOFREADING || nType == text::TextMarkupType::SENTENCE, "Wrong mark-up type" );
    const ModelToViewHelper::ModelPosition aStartPos =
            rConversionMap.ConvertToModelPosition( nStart );
    const ModelToViewHelper::ModelPosition aEndPos   =
            rConversionMap.ConvertToModelPosition( nStart + nLength - 1);

    const bool bStartInField = aStartPos.mbIsField;
    const bool bEndInField   = aEndPos.mbIsField;
    bool bCommit = false;

    if ( bStartInField && bEndInField && aStartPos.mnPos == aEndPos.mnPos )
    {
        nStart = aStartPos.mnSubPos;
        const sal_Int32 nFieldPosModel = aStartPos.mnPos;
        const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );

        SwGrammarMarkUp* pSubList = static_cast<SwGrammarMarkUp*>(pWList->SubList( nInsertPos ));
        if ( !pSubList )
        {
            pSubList = new SwGrammarMarkUp();
            pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
        }

        pWList = pSubList;
        bCommit = true;
    }
    else if ( !bStartInField && !bEndInField )
    {
        nStart = aStartPos.mnPos;
        bCommit = true;
        nLength = aEndPos.mnPos + 1 - aStartPos.mnPos;
    }
    else
    {
        bCommit = true;
        nStart = aStartPos.mnPos;
        sal_Int32 nEnd = aEndPos.mnPos;
        if( bStartInField && nType != text::TextMarkupType::SENTENCE )
        {
            const sal_Int32 nFieldPosModel = aStartPos.mnPos;
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwGrammarMarkUp* pSubList = static_cast<SwGrammarMarkUp*>(pWList->SubList( nInsertPos ));
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_Int32 nTmpStart = rConversionMap.ConvertToViewPosition( aStartPos.mnPos );
            const sal_Int32 nTmpLen = rConversionMap.ConvertToViewPosition( aStartPos.mnPos + 1 )
                                       - nTmpStart - aStartPos.mnSubPos;
            if( nTmpLen > 0 )
                pSubList->Insert( rIdentifier, xMarkupInfoContainer, aStartPos.mnSubPos, nTmpLen );
            ++nStart;
        }
        if( bEndInField && nType != text::TextMarkupType::SENTENCE )
        {
            const sal_Int32 nFieldPosModel = aEndPos.mnPos;
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwGrammarMarkUp* pSubList = static_cast<SwGrammarMarkUp*>(pWList->SubList( nInsertPos ));
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_Int32 nTmpLen = aEndPos.mnSubPos + 1;
            pSubList->Insert( rIdentifier, xMarkupInfoContainer, 0, nTmpLen );
        }
        else
            ++nEnd;
        if( nEnd > nStart )
            nLength = nEnd - nStart;
        else
            bCommit = false;
    }

    if ( bCommit )
    {
        if( nType == text::TextMarkupType::SENTENCE )
            pWList->setSentence( nStart+nLength );
        else
            pWList->Insert( rIdentifier, xMarkupInfoContainer, nStart, nLength );
    }
}

void SAL_CALL SwXTextMarkup::commitMultiTextMarkup(
    const uno::Sequence< text::TextMarkupDescriptor > &rMarkups )
{
    SolarMutexGuard aGuard;

    // paragraph already dead or modified?
    if (!m_pImpl->m_pTextNode)
        return;

    // for grammar checking there should be exactly one sentence markup
    // and 0..n grammar markups.
    // Different markups are not expected but may be applied anyway since
    // that should be no problem...
    // but it has to be implemented, at the moment only this function is for
    // grammar markups and sentence markup only!
    const text::TextMarkupDescriptor *pSentenceMarkUp = nullptr;
    for( const text::TextMarkupDescriptor &rDesc : rMarkups )
    {
        if (rDesc.nType == text::TextMarkupType::SENTENCE)
        {
            if (pSentenceMarkUp != nullptr)
                throw lang::IllegalArgumentException(); // there is already one sentence markup
            pSentenceMarkUp = &rDesc;
        }
        else if( rDesc.nType != text::TextMarkupType::PROOFREADING )
            return;
    }

    if( pSentenceMarkUp == nullptr )
        return;

    // get appropriate list to use...
    SwGrammarMarkUp* pWList = nullptr;
    bool bRepaint = false;
    sw::GrammarContact* pGrammarContact = sw::getGrammarContactFor(*m_pImpl->m_pTextNode);
    if( pGrammarContact )
    {
        pWList = pGrammarContact->getGrammarCheck(*m_pImpl->m_pTextNode, true);
        assert(pWList && "GrammarContact _has_ to deliver a wrong list");
    }
    else
    {
        pWList = m_pImpl->m_pTextNode->GetGrammarCheck();
        if ( !pWList )
        {
            m_pImpl->m_pTextNode->SetGrammarCheck( std::make_unique<SwGrammarMarkUp>() );
            pWList = m_pImpl->m_pTextNode->GetGrammarCheck();
            pWList->SetInvalid( 0, COMPLETE_STRING );
        }
    }
    bRepaint = pWList == m_pImpl->m_pTextNode->GetGrammarCheck();

    bool bAcceptGrammarError = false;
    if( pWList->GetBeginInv() < COMPLETE_STRING )
    {
        const ModelToViewHelper::ModelPosition aSentenceEnd =
            m_pImpl->m_ConversionMap.ConvertToModelPosition(
                pSentenceMarkUp->nOffset + pSentenceMarkUp->nLength );
        bAcceptGrammarError = aSentenceEnd.mnPos > pWList->GetBeginInv();
        pWList->ClearGrammarList( aSentenceEnd.mnPos );
    }

    if( bAcceptGrammarError )
    {
        for( const text::TextMarkupDescriptor &rDesc : rMarkups )
        {
            lcl_commitGrammarMarkUp(m_pImpl->m_ConversionMap, pWList, rDesc.nType,
                rDesc.aIdentifier, rDesc.nOffset, rDesc.nLength, rDesc.xMarkupInfoContainer );
        }
    }
    else
    {
        bRepaint = false;
        const text::TextMarkupDescriptor &rDesc = *pSentenceMarkUp;
        lcl_commitGrammarMarkUp(m_pImpl->m_ConversionMap, pWList, rDesc.nType,
            rDesc.aIdentifier, rDesc.nOffset, rDesc.nLength, rDesc.xMarkupInfoContainer );
    }

    if( bRepaint )
        sw::finishGrammarCheckFor(*m_pImpl->m_pTextNode);
}

void SwXTextMarkup::Impl::Notify(const SfxHint& rHint)
{
    DBG_TESTSOLARMUTEX();
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pTextNode = nullptr;
    }
}

SwXStringKeyMap::SwXStringKeyMap()
{
}

uno::Any SAL_CALL SwXStringKeyMap::getValue(const OUString & aKey)
{
    std::map< OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter == maMap.end() )
        throw container::NoSuchElementException();

    return (*aIter).second;
}

sal_Bool SAL_CALL SwXStringKeyMap::hasValue(const OUString & aKey)
{
    return maMap.contains(aKey);
}

void SAL_CALL SwXStringKeyMap::insertValue(const OUString & aKey, const uno::Any & aValue)
{
    std::map< OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter != maMap.end() )
        throw container::ElementExistException();

    maMap[ aKey ] = aValue;
}

::sal_Int32 SAL_CALL SwXStringKeyMap::getCount()
{
    return maMap.size();
}

OUString SAL_CALL SwXStringKeyMap::getKeyByIndex(::sal_Int32 nIndex)
{
    if ( o3tl::make_unsigned(nIndex) >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return OUString();
}

uno::Any SAL_CALL SwXStringKeyMap::getValueByIndex(::sal_Int32 nIndex)
{
    if ( o3tl::make_unsigned(nIndex) >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return uno::Any();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
