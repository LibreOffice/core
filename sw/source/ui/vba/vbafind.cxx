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
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbareplacement.hxx"
#include <ooo/vba/word/WdFindWrap.hpp>
#include <ooo/vba/word/WdReplace.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFind::SwVbaFind( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ) throw ( uno::RuntimeException ) :
    SwVbaFind_BASE( rParent, rContext ), mxModel( xModel ), mxTextRange( xTextRange ), mbReplace( false ), mnReplaceType( word::WdReplace::wdReplaceOne ), mnWrap( word::WdFindWrap::wdFindStop )
{
    mxReplaceable.set( mxModel, uno::UNO_QUERY_THROW );
    mxPropertyReplace.set( mxReplaceable->createReplaceDescriptor(), uno::UNO_QUERY_THROW );
    mxTVC = word::getXTextViewCursor( mxModel );
    mxSelSupp.set( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
}

SwVbaFind::~SwVbaFind()
{
}

bool SwVbaFind::InRange( const uno::Reference< text::XTextRange >& xCurrentRange ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) >= 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) <= 0 )
        return true;
    return false;
}

bool SwVbaFind::InEqualRange( const uno::Reference< text::XTextRange >& xCurrentRange ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) == 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) == 0 )
        return true;
    return false;
}

void SwVbaFind::SetReplaceWith( const OUString& rText ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setReplaceString( rText );
    mbReplace = true;
}

OUString SwVbaFind::GetReplaceWith() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getReplaceString();
}
void SwVbaFind::SetReplace( sal_Int32 type )
{
    mnReplaceType = type;
    mbReplace = true;
}
uno::Reference< text::XTextRange > SwVbaFind::FindOneElement() throw ( uno::RuntimeException )
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
            mxTVC->gotoStart(sal_False);
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getStart(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );
        }
        else
        {
            mxTVC->gotoEnd( sal_False );
            xFoundOne.set( mxReplaceable->findNext( mxTextRange->getEnd(), uno::Reference< util::XSearchDescriptor >( mxPropertyReplace, uno::UNO_QUERY_THROW ) ), uno::UNO_QUERY );

        }
    }
    return xFoundOne;
}

bool SwVbaFind::SearchReplace() throw (uno::RuntimeException)
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
                    result = mxSelSupp->select( uno::makeAny( xFindOne ) );
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
            result = mxSelSupp->select( uno::makeAny( xFindOne ) );
    }

    return result;
}

OUString SAL_CALL SwVbaFind::getText() throw (uno::RuntimeException, std::exception)
{
    return mxPropertyReplace->getSearchString();
}

void SAL_CALL SwVbaFind::setText( const OUString& _text ) throw (uno::RuntimeException, std::exception)
{
    mxPropertyReplace->setSearchString( _text );
}

uno::Any SAL_CALL SwVbaFind::getReplacement() throw (uno::RuntimeException, std::exception)
{
    return uno::makeAny( uno::Reference< word::XReplacement >( new SwVbaReplacement( this, mxContext, mxPropertyReplace ) ) );
}

void SAL_CALL SwVbaFind::setReplacement( const uno::Any& /*_replacement */ ) throw (uno::RuntimeException, std::exception)
{
    throw uno::RuntimeException("Not implemented" );
}

sal_Bool SAL_CALL SwVbaFind::getForward() throw (uno::RuntimeException, std::exception)
{
    bool bBackward = false;
    mxPropertyReplace->getPropertyValue("SearchBackwards") >>= bBackward;
    return !bBackward;
}

void SAL_CALL SwVbaFind::setForward( sal_Bool _forward ) throw (uno::RuntimeException, std::exception)
{
    bool bBackward = !_forward;
    mxPropertyReplace->setPropertyValue("SearchBackwards", uno::makeAny( bBackward ) );
}

::sal_Int32 SAL_CALL SwVbaFind::getWrap() throw (uno::RuntimeException, std::exception)
{
    // seems not supported in Writer
    return mnWrap;
}

void SAL_CALL SwVbaFind::setWrap( ::sal_Int32 _wrap ) throw (uno::RuntimeException, std::exception)
{
    // seems not supported in Writer
    mnWrap = _wrap;
}

sal_Bool SAL_CALL SwVbaFind::getFormat() throw (uno::RuntimeException, std::exception)
{
    return mxPropertyReplace->getValueSearch();
}

void SAL_CALL SwVbaFind::setFormat( sal_Bool _format ) throw (uno::RuntimeException, std::exception)
{
    mxPropertyReplace->setValueSearch( _format );
}

sal_Bool SAL_CALL SwVbaFind::getMatchCase() throw (uno::RuntimeException, std::exception)
{
    bool value = false;
    mxPropertyReplace->getPropertyValue("SearchCaseSensitive") >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchCase( sal_Bool _matchcase ) throw (uno::RuntimeException, std::exception)
{
    mxPropertyReplace->setPropertyValue("SearchCaseSensitive", uno::makeAny( _matchcase ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchWholeWord() throw (uno::RuntimeException, std::exception)
{
    bool value = false;
    mxPropertyReplace->getPropertyValue("SearchWords") >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWholeWord( sal_Bool _matchwholeword ) throw (uno::RuntimeException, std::exception)
{
    mxPropertyReplace->setPropertyValue("SearchWords", uno::makeAny( _matchwholeword ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchWildcards() throw (uno::RuntimeException, std::exception)
{
    bool value = false;
    mxPropertyReplace->getPropertyValue("SearchRegularExpression") >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWildcards( sal_Bool _matchwildcards ) throw (uno::RuntimeException, std::exception)
{
    mxPropertyReplace->setPropertyValue("SearchRegularExpression", uno::makeAny( _matchwildcards ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchSoundsLike() throw (uno::RuntimeException, std::exception)
{
    bool value = false;
    mxPropertyReplace->getPropertyValue("SearchSimilarity") >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchSoundsLike( sal_Bool _matchsoundslike ) throw (uno::RuntimeException, std::exception)
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue("SearchSimilarity", uno::makeAny( _matchsoundslike ) );
}

sal_Bool SAL_CALL SwVbaFind::getMatchAllWordForms() throw (uno::RuntimeException, std::exception)
{
    bool value = false;
    mxPropertyReplace->getPropertyValue("SearchSimilarity") >>= value;
    if( value )
        mxPropertyReplace->getPropertyValue("SearchSimilarityRelax") >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchAllWordForms( sal_Bool _matchallwordforms ) throw (uno::RuntimeException, std::exception)
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue("SearchSimilarity", uno::makeAny( _matchallwordforms ) );
    mxPropertyReplace->setPropertyValue("SearchSimilarityRelax", uno::makeAny( _matchallwordforms ) );
}

uno::Any SAL_CALL SwVbaFind::getStyle() throw (uno::RuntimeException, std::exception)
{
    throw uno::RuntimeException("Not implemented" );
}

void SAL_CALL SwVbaFind::setStyle( const uno::Any& /*_style */ ) throw (uno::RuntimeException, std::exception)
{
    throw uno::RuntimeException("Not implemented" );
}

sal_Bool SAL_CALL
SwVbaFind::Execute( const uno::Any& FindText, const uno::Any& MatchCase, const uno::Any& MatchWholeWord, const uno::Any& MatchWildcards, const uno::Any& MatchSoundsLike, const uno::Any& MatchAllWordForms, const uno::Any& Forward, const uno::Any& Wrap, const uno::Any& Format, const uno::Any& ReplaceWith, const uno::Any& Replace, const uno::Any& /*MatchKashida*/, const uno::Any& /*MatchDiacritics*/, const uno::Any& /*MatchAlefHamza*/, const uno::Any& /*MatchControl*/, const uno::Any& /*MatchPrefix*/, const uno::Any& /*MatchSuffix*/, const uno::Any& /*MatchPhrase*/, const uno::Any& /*IgnoreSpace*/, const uno::Any& /*IgnorePunct*/ ) throw (uno::RuntimeException, std::exception)
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
SwVbaFind::ClearFormatting(  ) throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< beans::PropertyValue >  aSearchAttribs;
    mxPropertyReplace->setSearchAttributes( aSearchAttribs );
}

OUString
SwVbaFind::getServiceImplName()
{
    return OUString("SwVbaFind");
}

uno::Sequence< OUString >
SwVbaFind::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Find";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
