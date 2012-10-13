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

#include <unotextmarkup.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <SwSmartTagMgr.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/text/TextMarkupDescriptor.hpp>
#include <com/sun/star/container/XStringKeyMap.hpp>
#include <ndtxt.hxx>
#include <SwGrammarMarkUp.hxx>

#include <IGrammarContact.hxx>

using namespace ::com::sun::star;

/*
 * SwXTextMarkup
 */
SwXTextMarkup::SwXTextMarkup( SwTxtNode& rTxtNode, const ModelToViewHelper& rMap )
    : mpTxtNode( &rTxtNode ), maConversionMap( rMap )
{
    // FME 2007-07-16 #i79641# SwXTextMarkup is allowed to be removed ...
    SetIsAllowedToBeRemovedInModifyCall(true);
    mpTxtNode->Add(this);
}

SwXTextMarkup::~SwXTextMarkup()
{
}

uno::Reference< container::XStringKeyMap > SAL_CALL SwXTextMarkup::getMarkupInfoContainer() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference< container::XStringKeyMap > xProp = new SwXStringKeyMap;
    return xProp;
}

void SAL_CALL SwXTextMarkup::commitTextMarkup(
    ::sal_Int32 nType,
    const ::rtl::OUString & rIdentifier,
    ::sal_Int32 nStart,
    ::sal_Int32 nLength,
    const uno::Reference< container::XStringKeyMap > & xMarkupInfoContainer)
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // paragraph already dead or modified?
    if ( !mpTxtNode || nLength <= 0 )
        return;

    if ( nType == text::TextMarkupType::SMARTTAG &&
        !SwSmartTagMgr::Get().IsSmartTagTypeEnabled( rIdentifier ) )
        return;

    // get appropriate list to use...
    SwWrongList* pWList = 0;
    bool bRepaint = false;
    if ( nType == text::TextMarkupType::SPELLCHECK )
    {
        pWList = mpTxtNode->GetWrong();
        if ( !pWList )
        {
            pWList = new SwWrongList( WRONGLIST_SPELL );
            mpTxtNode->SetWrong( pWList );
        }
    }
    else if ( nType == text::TextMarkupType::PROOFREADING || nType == text::TextMarkupType::SENTENCE )
    {
        IGrammarContact *pGrammarContact = getGrammarContact( *mpTxtNode );
        if( pGrammarContact )
        {
            pWList = pGrammarContact->getGrammarCheck( *mpTxtNode, true );
            OSL_ENSURE( pWList, "GrammarContact _has_ to deliver a wrong list" );
        }
        else
        {
            pWList = mpTxtNode->GetGrammarCheck();
            if ( !pWList )
            {
                mpTxtNode->SetGrammarCheck( new SwGrammarMarkUp() );
                pWList = mpTxtNode->GetGrammarCheck();
            }
        }
        bRepaint = pWList == mpTxtNode->GetGrammarCheck();
        if( pWList->GetBeginInv() < STRING_LEN )
            ((SwGrammarMarkUp*)pWList)->ClearGrammarList();
    }
    else if ( nType == text::TextMarkupType::SMARTTAG )
    {
        pWList = mpTxtNode->GetSmartTags();
        if ( !pWList )
        {
            pWList = new SwWrongList( WRONGLIST_SMARTTAG );
            mpTxtNode->SetSmartTags( pWList );
        }
    }
    else
    {
        OSL_FAIL( "Unknown mark-up type" );
        return;
    }


    const ModelToViewHelper::ModelPosition aStartPos =
            maConversionMap.ConvertToModelPosition( nStart );
    const ModelToViewHelper::ModelPosition aEndPos   =
            maConversionMap.ConvertToModelPosition( nStart + nLength - 1);

    const bool bStartInField = aStartPos.mbIsField;
    const bool bEndInField   = aEndPos.mbIsField;
    bool bCommit = false;

    if ( bStartInField && bEndInField && aStartPos.mnPos == aEndPos.mnPos )
    {
        nStart = aStartPos.mnSubPos;
        const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aStartPos.mnPos);
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
            const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aStartPos.mnPos);
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwWrongList* pSubList = pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_uInt32 nTmpStart = maConversionMap.ConvertToViewPosition( aStartPos.mnPos );
            const sal_uInt32 nTmpLen = maConversionMap.ConvertToViewPosition( aStartPos.mnPos + 1 )
                                       - nTmpStart - aStartPos.mnSubPos;
            if( nTmpLen > 0 )
            {
                if( nType == text::TextMarkupType::SENTENCE )
                {
                    ((SwGrammarMarkUp*)pSubList)->setSentence( static_cast< xub_StrLen >(aStartPos.mnSubPos) );
                    bCommit = false;
                }
                else
                    pSubList->Insert( rIdentifier, xMarkupInfoContainer,
                        static_cast< xub_StrLen >(aStartPos.mnSubPos), static_cast< xub_StrLen >(nTmpLen) );
            }
            ++nStart;
        }
        if( bEndInField && nType != text::TextMarkupType::SENTENCE )
        {
            const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aEndPos.mnPos);
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwWrongList* pSubList = pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_uInt32 nTmpLen = aEndPos.mnSubPos + 1;
            pSubList->Insert( rIdentifier, xMarkupInfoContainer, 0, static_cast< xub_StrLen >(nTmpLen) );
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
            ((SwGrammarMarkUp*)pWList)->setSentence( static_cast< xub_StrLen >(nStart) );
        else
            pWList->Insert( rIdentifier, xMarkupInfoContainer,
                static_cast< xub_StrLen >(nStart), static_cast< xub_StrLen >(nLength) );
    }

    if( bRepaint )
        finishGrammarCheck( *mpTxtNode );
}


static void lcl_commitGrammarMarkUp(
    const ModelToViewHelper& rConversionMap,
    SwGrammarMarkUp* pWList,
    ::sal_Int32 nType,
    const ::rtl::OUString & rIdentifier,
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
        const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aStartPos.mnPos);
        const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );

        SwGrammarMarkUp* pSubList = (SwGrammarMarkUp*)pWList->SubList( nInsertPos );
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
            const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aStartPos.mnPos);
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwGrammarMarkUp* pSubList = (SwGrammarMarkUp*)pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_uInt32 nTmpStart = rConversionMap.ConvertToViewPosition( aStartPos.mnPos );
            const sal_uInt32 nTmpLen = rConversionMap.ConvertToViewPosition( aStartPos.mnPos + 1 )
                                       - nTmpStart - aStartPos.mnSubPos;
            if( nTmpLen > 0 )
                pSubList->Insert( rIdentifier, xMarkupInfoContainer,
                    static_cast< xub_StrLen >(aStartPos.mnSubPos), static_cast< xub_StrLen >(nTmpLen) );
            ++nStart;
        }
        if( bEndInField && nType != text::TextMarkupType::SENTENCE )
        {
            const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aEndPos.mnPos);
            const sal_uInt16 nInsertPos = pWList->GetWrongPos( nFieldPosModel );
            SwGrammarMarkUp* pSubList = (SwGrammarMarkUp*)pWList->SubList( nInsertPos );
            if ( !pSubList )
            {
                pSubList = new SwGrammarMarkUp();
                pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
            }
            const sal_uInt32 nTmpLen = aEndPos.mnSubPos + 1;
            pSubList->Insert( rIdentifier, xMarkupInfoContainer, 0, static_cast< xub_StrLen >(nTmpLen) );
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
            ((SwGrammarMarkUp*)pWList)->setSentence( static_cast< xub_StrLen >(nStart+nLength) );
        else
            pWList->Insert( rIdentifier, xMarkupInfoContainer,
                static_cast< xub_StrLen >(nStart), static_cast< xub_StrLen >(nLength) );
    }
}


void SAL_CALL SwXTextMarkup::commitMultiTextMarkup(
    const uno::Sequence< text::TextMarkupDescriptor > &rMarkups )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // paragraph already dead or modified?
    if ( !mpTxtNode )
        return;

    // check for equal length of all sequnces
    sal_Int32 nLen = rMarkups.getLength();

    // for grammar checking there should be exactly one sentence markup
    // and 0..n grammar markups.
    // Different markups are not expected but may be applied anyway since
    // that should be no problem...
    // but it has to be implemented, at the moment only this function is for
    // grammar markups and sentence markup only!
    sal_Int32 nSentenceMarkUpIndex = -1;
    const text::TextMarkupDescriptor *pMarkups = rMarkups.getConstArray();
    sal_Int32 i;
    for( i = 0;  i < nLen;  ++i )
    {
        if (pMarkups[i].nType == text::TextMarkupType::SENTENCE)
        {
            if (nSentenceMarkUpIndex == -1)
                nSentenceMarkUpIndex = i;
            else    // there is already one sentence markup
                throw lang::IllegalArgumentException();
        }
        else if( pMarkups[i].nType != text::TextMarkupType::PROOFREADING )
            return;
    }

    if( nSentenceMarkUpIndex == -1 )
        return;

    // get appropriate list to use...
    SwGrammarMarkUp* pWList = 0;
    bool bRepaint = false;
    IGrammarContact *pGrammarContact = getGrammarContact( *mpTxtNode );
    if( pGrammarContact )
    {
        pWList = pGrammarContact->getGrammarCheck( *mpTxtNode, true );
        OSL_ENSURE( pWList, "GrammarContact _has_ to deliver a wrong list" );
    }
    else
    {
        pWList = mpTxtNode->GetGrammarCheck();
        if ( !pWList )
        {
            mpTxtNode->SetGrammarCheck( new SwGrammarMarkUp() );
            pWList = mpTxtNode->GetGrammarCheck();
            pWList->SetInvalid( 0, STRING_LEN );
        }
    }
    bRepaint = pWList == mpTxtNode->GetGrammarCheck();

    bool bAcceptGrammarError = false;
    if( pWList->GetBeginInv() < STRING_LEN )
    {
        const ModelToViewHelper::ModelPosition aSentenceEnd =
            maConversionMap.ConvertToModelPosition(
                pMarkups[nSentenceMarkUpIndex].nOffset + pMarkups[nSentenceMarkUpIndex].nLength );
        bAcceptGrammarError = (xub_StrLen)aSentenceEnd.mnPos > pWList->GetBeginInv();
        pWList->ClearGrammarList( (xub_StrLen)aSentenceEnd.mnPos );
    }

    if( bAcceptGrammarError )
    {
        for( i = 0;  i < nLen;  ++i )
        {
            const text::TextMarkupDescriptor &rDesc = pMarkups[i];
            lcl_commitGrammarMarkUp( maConversionMap, pWList, rDesc.nType,
                rDesc.aIdentifier, rDesc.nOffset, rDesc.nLength, rDesc.xMarkupInfoContainer );
        }
    }
    else
    {
        bRepaint = false;
        i = nSentenceMarkUpIndex;
        const text::TextMarkupDescriptor &rDesc = pMarkups[i];
        lcl_commitGrammarMarkUp( maConversionMap, pWList, rDesc.nType,
            rDesc.aIdentifier, rDesc.nOffset, rDesc.nLength, rDesc.xMarkupInfoContainer );
    }

    if( bRepaint )
        finishGrammarCheck( *mpTxtNode );

    return;
}


void SwXTextMarkup::Modify( const SfxPoolItem* /*pOld*/, const SfxPoolItem* /*pNew*/ )
{
    // FME 2007-07-16 #i79641# In my opinion this is perfectly legal,
    // therefore I remove the assertion in SwModify::_Remove()
    if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    SolarMutexGuard aGuard;
    mpTxtNode = 0;
}

/*
 * SwXStringKeyMap
 */
SwXStringKeyMap::SwXStringKeyMap()
{
}

uno::Any SAL_CALL SwXStringKeyMap::getValue(const ::rtl::OUString & aKey) throw (uno::RuntimeException, container::NoSuchElementException)
{
    std::map< rtl::OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter == maMap.end() )
        throw container::NoSuchElementException();

    return (*aIter).second;
}

::sal_Bool SAL_CALL SwXStringKeyMap::hasValue(const ::rtl::OUString & aKey) throw (uno::RuntimeException)
{
    return maMap.find( aKey ) != maMap.end();
}

void SAL_CALL SwXStringKeyMap::insertValue(const ::rtl::OUString & aKey, const uno::Any & aValue) throw (uno::RuntimeException, lang::IllegalArgumentException, container::ElementExistException)
{
    std::map< rtl::OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter != maMap.end() )
        throw container::ElementExistException();

    maMap[ aKey ] = aValue;
}

::sal_Int32 SAL_CALL SwXStringKeyMap::getCount() throw (uno::RuntimeException)
{
    return maMap.size();
}

::rtl::OUString SAL_CALL SwXStringKeyMap::getKeyByIndex(::sal_Int32 nIndex) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    if ( (sal_uInt32)nIndex >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return ::rtl::OUString();
}

uno::Any SAL_CALL SwXStringKeyMap::getValueByIndex(::sal_Int32 nIndex) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    if ( (sal_uInt32)nIndex >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return uno::Any();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
