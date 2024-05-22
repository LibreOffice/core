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
#include "vbafind.hxx"
#include "vbareplacement.hxx"
#include <ooo/vba/word/WdFindWrap.hpp>
#include <ooo/vba/word/WdReplace.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <doc.hxx>
#include <docsh.hxx>
#include "wordvbahelper.hxx"
#include <rtl/ref.hxx>
#include <sal/log.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFind::SwVbaFind( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< frame::XModel > xModel ) :
    SwVbaFind_BASE( rParent, rContext ), mxModel( std::move(xModel) ), mbReplace( false ), mnReplaceType( word::WdReplace::wdReplaceOne ), mnWrap( word::WdFindWrap::wdFindStop )
{
    mxReplaceable.set( mxModel, uno::UNO_QUERY_THROW );
    mxPropertyReplace.set( mxReplaceable->createReplaceDescriptor(), uno::UNO_QUERY_THROW );
    mxTVC = word::getXTextViewCursor( mxModel );
    mxSelSupp.set( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
}

SwVbaFind::~SwVbaFind()
{
}

uno::Reference< word::XFind > SwVbaFind::GetOrCreateFind(const uno::Reference< ooo::vba::XHelperInterface >& rParent,
                                                         const uno::Reference< uno::XComponentContext >& rContext,
                                                         const uno::Reference< frame::XModel >& xModel,
                                                         const uno::Reference< text::XTextRange >& xTextRange)
{
    rtl::Reference< SwVbaFind > xFind;
    SwDoc* pDoc = word::getDocShell( xModel )->GetDoc();
    if( pDoc )
        xFind = dynamic_cast<SwVbaFind *>( pDoc->getVbaFind().get() );
    if ( !xFind )
    {
        xFind = new SwVbaFind( rParent, rContext, xModel );
        if ( pDoc )
            pDoc->setVbaFind( xFind );
    }
    xFind->mxTextRange = xTextRange;

    return xFind;
}


bool SwVbaFind::InRange( const uno::Reference< text::XTextRange >& xCurrentRange )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    return xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) >= 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) <= 0;
}

bool SwVbaFind::InEqualRange( const uno::Reference< text::XTextRange >& xCurrentRange )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    return xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) == 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) == 0;
}

void SwVbaFind::SetReplaceWith( const OUString& rText )
{
    mxPropertyReplace->setReplaceString( rText );
    mbReplace = true;
}

OUString SwVbaFind::GetReplaceWith()
{
    return mxPropertyReplace->getReplaceString();
}
void SwVbaFind::SetReplace( sal_Int32 type )
{
    mnReplaceType = type;
    mbReplace = true;
}
uno::Reference< text::XTextRange > SwVbaFind::FindOneElement()
{
    uno::Reference< text::XTextRange > xFoundOne;
    if( !mxTVC->getString().isEmpty() )
    {
        if( getForward() )
        {
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getStart(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
        }
        else
        {
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getEnd(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
        }

        if( xFoundOne.is() && InEqualRange( xFoundOne ) )
        {
            xFoundOne.set( mxReplaceable->findNext( xFoundOne, uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
        }
        else if( xFoundOne.is() && !InRange( xFoundOne ) )
        {
            xFoundOne.clear();
        }
    }
    else
    {
        xFoundOne.set( mxReplaceable->findNext( mxTextRange, uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
    }

    if( !xFoundOne.is() && ( getWrap() == word::WdFindWrap::wdFindContinue || getWrap() == word::WdFindWrap::wdFindAsk ) )
    {
        if( getForward() )
        {
            mxTVC->gotoStart(false);
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getStart(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
        }
        else
        {
            mxTVC->gotoEnd( false );
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getEnd(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );

        }
    }
    return xFoundOne;
}

bool SwVbaFind::SearchReplace()
{
    bool result = false;

    // TODO: map wildcards in area to OOo wildcards

    if( mbReplace )
    {
        switch( mnReplaceType )
        {
            case word::WdReplace::wdReplaceNone:
            {
                result = true;
                break;
            }
            case word::WdReplace::wdReplaceOne:
            {
                uno::Reference< text::XTextRange > xFindOne = FindOneElement();
                if( xFindOne.is() )
                {
                    xFindOne->setString( GetReplaceWith() );
                    result = mxSelSupp->select( uno::Any( xFindOne ) );
                }
                break;
            }
            case word::WdReplace::wdReplaceAll:
            {
                uno::Reference< container::XIndexAccess > xIndexAccess = mxReplaceable->findAll( uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) );
                if( xIndexAccess->getCount() > 0 )
                {
                    for( sal_Int32 i = 0; i < xIndexAccess->getCount(); i++ )
                    {
                        uno::Reference< text::XTextRange > xTextRange( xIndexAccess->getByIndex( i ), uno::UNO_QUERY_THROW );
                        if( mnWrap == word::WdFindWrap::wdFindContinue || mnWrap == word::WdFindWrap::wdFindAsk || InRange( xTextRange ) )
                        {
                            xTextRange->setString( GetReplaceWith() );
                            result = true;
                        }
                    }
                }
                break;
            }
            default:
            {
                result = false;
            }
        }
    }
    else
    {
        uno::Reference< text::XTextRange > xFindOne = FindOneElement();
        if( xFindOne.is() )
            result = mxSelSupp->select( uno::Any( xFindOne ) );
    }

    return result;
}

OUString SAL_CALL SwVbaFind::getText()
{
    return mxPropertyReplace->getSearchString();
}

void SAL_CALL SwVbaFind::setText( const OUString& _text )
{
    mxPropertyReplace->setSearchString( _text );
}

uno::Any SAL_CALL SwVbaFind::getReplacement()
{
    return uno::Any( uno::Reference< word::XReplacement >( new SwVbaReplacement( this, mxContext, mxPropertyReplace ) ) );
}

void SAL_CALL SwVbaFind::setReplacement( const uno::Any& /*_replacement */ )
{
    throw uno::RuntimeException(u"Not implemented"_ustr );
}

sal_Bool SAL_CALL SwVbaFind::getForward()
{
    bool bBackward = false;
    mxPropertyReplace->getPropertyValue(u"SearchBackwards"_ustr) >>= bBackward;
    return !bBackward;
}

void SAL_CALL SwVbaFind::setForward( sal_Bool _forward )
{
    bool bBackward = !_forward;
    mxPropertyReplace->setPropertyValue(u"SearchBackwards"_ustr, uno::Any( bBackward ) );
}

::sal_Int32 SAL_CALL SwVbaFind::getWrap()
{
    // seems not supported in Writer
    return mnWrap;
}

void SAL_CALL SwVbaFind::setWrap( ::sal_Int32 _wrap )
{
    // seems not supported in Writer
    mnWrap = _wrap;
}

sal_Bool SAL_CALL SwVbaFind::getFormat()
{
    return mxPropertyReplace->getValueSearch();
}

void SAL_CALL SwVbaFind::setFormat( sal_Bool _format )
{
    mxPropertyReplace->setValueSearch( _format );
}

sal_Bool SAL_CALL SwVbaFind::getMatchCase()
{
    bool value = false;
    mxPropertyReplace->getPropertyValue(u"SearchCaseSensitive"_ustr) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchCase( sal_Bool _matchcase )
{
    mxPropertyReplace->setPropertyValue(u"SearchCaseSensitive"_ustr, uno::Any( _matchcase ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchWholeWord()
{
    bool value = false;
    mxPropertyReplace->getPropertyValue(u"SearchWords"_ustr) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWholeWord( sal_Bool _matchwholeword )
{
    mxPropertyReplace->setPropertyValue(u"SearchWords"_ustr, uno::Any( _matchwholeword ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchWildcards()
{
    bool value = false;
    mxPropertyReplace->getPropertyValue(u"SearchRegularExpression"_ustr) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWildcards( sal_Bool _matchwildcards )
{
    mxPropertyReplace->setPropertyValue(u"SearchRegularExpression"_ustr, uno::Any( _matchwildcards ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchSoundsLike()
{
    bool value = false;
    mxPropertyReplace->getPropertyValue(u"SearchSimilarity"_ustr) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchSoundsLike( sal_Bool _matchsoundslike )
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue(u"SearchSimilarity"_ustr, uno::Any( _matchsoundslike ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchAllWordForms()
{
    bool value = false;
    mxPropertyReplace->getPropertyValue(u"SearchSimilarity"_ustr) >>= value;
    if( value )
        mxPropertyReplace->getPropertyValue(u"SearchSimilarityRelax"_ustr) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchAllWordForms( sal_Bool _matchallwordforms )
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue(u"SearchSimilarity"_ustr, uno::Any( _matchallwordforms ) );
    mxPropertyReplace->setPropertyValue(u"SearchSimilarityRelax"_ustr, uno::Any( _matchallwordforms ) );
}

uno::Any SAL_CALL SwVbaFind::getStyle()
{
    throw uno::RuntimeException(u"Not implemented"_ustr );
}

void SAL_CALL SwVbaFind::setStyle( const uno::Any& /*_style */ )
{
    throw uno::RuntimeException(u"Not implemented"_ustr );
}

sal_Bool SAL_CALL
SwVbaFind::Execute( const uno::Any& FindText, const uno::Any& MatchCase, const uno::Any& MatchWholeWord, const uno::Any& MatchWildcards, const uno::Any& MatchSoundsLike, const uno::Any& MatchAllWordForms, const uno::Any& Forward, const uno::Any& Wrap, const uno::Any& Format, const uno::Any& ReplaceWith, const uno::Any& Replace, const uno::Any& /*MatchKashida*/, const uno::Any& /*MatchDiacritics*/, const uno::Any& /*MatchAlefHamza*/, const uno::Any& /*MatchControl*/, const uno::Any& /*MatchPrefix*/, const uno::Any& /*MatchSuffix*/, const uno::Any& /*MatchPhrase*/, const uno::Any& /*IgnoreSpace*/, const uno::Any& /*IgnorePunct*/ )
{
    bool result = false;
    if( FindText.hasValue() )
    {
        OUString sText;
        FindText >>= sText;
        setText( sText );
    }

    bool bValue = false;
    if( MatchCase.hasValue() )
    {
        MatchCase >>= bValue;
        setMatchCase( bValue );
    }

    if( MatchWholeWord.hasValue() )
    {
        MatchWholeWord >>= bValue;
        setMatchWholeWord( bValue );
    }

    if( MatchWildcards.hasValue() )
    {
        MatchWildcards >>= bValue;
        setMatchWildcards( bValue );
    }

    if( MatchSoundsLike.hasValue() )
    {
        MatchSoundsLike >>= bValue;
        setMatchSoundsLike( bValue );
    }

    if( MatchAllWordForms.hasValue() )
    {
        MatchAllWordForms >>= bValue;
        setMatchAllWordForms( bValue );
    }

    if( Forward.hasValue() )
    {
        Forward >>= bValue;
        setForward( bValue );
    }

    if( Wrap.hasValue() )
    {
        sal_Int32 nWrapType = 0;
        Wrap >>= nWrapType;
        setWrap( nWrapType );
    }

    if( Format.hasValue() )
    {
        Format >>= bValue;
        setFormat( bValue );
    }

    if( ReplaceWith.hasValue() )
    {
        OUString sValue;
        ReplaceWith >>= sValue;
        SetReplaceWith( sValue );
    }

    if( Replace.hasValue() )
    {
        sal_Int32 nValue(0);
        Replace >>= nValue;
        SetReplace( nValue );
    }

    result = SearchReplace();

    return result;
}

void SAL_CALL
SwVbaFind::ClearFormatting(  )
{
    uno::Sequence< beans::PropertyValue >  aSearchAttribs;
    mxPropertyReplace->setSearchAttributes( aSearchAttribs );
}

OUString
SwVbaFind::getServiceImplName()
{
    return u"SwVbaFind"_ustr;
}

uno::Sequence< OUString >
SwVbaFind::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Find"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
