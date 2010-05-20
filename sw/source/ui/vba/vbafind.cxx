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
    SwVbaFind_BASE( rParent, rContext ), mxModel( xModel ), mxTextRange( xTextRange ), mbReplace( sal_False ), mnReplaceType( word::WdReplace::wdReplaceOne ), mnWrap( word::WdFindWrap::wdFindStop )
{
    mxReplaceable.set( mxModel, uno::UNO_QUERY_THROW );
    mxPropertyReplace.set( mxReplaceable->createReplaceDescriptor(), uno::UNO_QUERY_THROW );
    mxTVC = word::getXTextViewCursor( mxModel );
    mxSelSupp.set( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
}

SwVbaFind::~SwVbaFind()
{
}

sal_Bool SwVbaFind::InRange( const uno::Reference< text::XTextRange >& xCurrentRange ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) >= 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) <= 0 )
        return sal_True;
    return sal_False;
}

sal_Bool SwVbaFind::InEqualRange( const uno::Reference< text::XTextRange >& xCurrentRange ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextRange->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( mxTextRange, xCurrentRange ) == 0 && xTRC->compareRegionEnds( mxTextRange, xCurrentRange ) == 0 )
        return sal_True;
    return sal_False;
}

void SwVbaFind::SetReplaceWith( const rtl::OUString& rText ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setReplaceString( rText );
    mbReplace = sal_True;
}

rtl::OUString SwVbaFind::GetReplaceWith() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getReplaceString();
}
void SwVbaFind::SetReplace( sal_Int32 type )
{
    mnReplaceType = type;
    mbReplace = sal_True;
}
#ifdef TOMORROW
rtl::OUString SwVbaFind::ReplaceWildcards( const rtl::OUString& /*rText*/ ) throw ( uno::RuntimeException )
{
    // TODO:
    return rtl::OUString();
}
#endif
uno::Reference< text::XTextRange > SwVbaFind::FindOneElement() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xFoundOne;
    if( mxTVC->getString().getLength() > 0 )
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
            xFoundOne = uno::Reference< text::XTextRange >();
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

sal_Bool SwVbaFind::SearchReplace() throw (uno::RuntimeException)
{
    sal_Bool result = sal_False;

    // TODO: map wildcards in area to OOo wildcards

    if( mbReplace )
    {
        switch( mnReplaceType )
        {
            case word::WdReplace::wdReplaceNone:
            {
                result = sal_True;
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
                            result = sal_True;
                        }
                    }
                }
                break;
            }
            default:
            {
                result = sal_False;
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

::rtl::OUString SAL_CALL SwVbaFind::getText() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getSearchString();
}

void SAL_CALL SwVbaFind::setText( const ::rtl::OUString& _text ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setSearchString( _text );
}

uno::Any SAL_CALL SwVbaFind::getReplacement() throw (uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< word::XReplacement >( new SwVbaReplacement( this, mxContext, mxPropertyReplace ) ) );
}

void SAL_CALL SwVbaFind::setReplacement( const uno::Any& /*_replacement */ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

::sal_Bool SAL_CALL SwVbaFind::getForward() throw (uno::RuntimeException)
{
    sal_Bool bBackward = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchBackwards") ) ) >>= bBackward;
    return !bBackward;
}

void SAL_CALL SwVbaFind::setForward( ::sal_Bool _forward ) throw (uno::RuntimeException)
{
    sal_Bool bBackward = !_forward;
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchBackwards") ), uno::makeAny( bBackward ) );
}

::sal_Int32 SAL_CALL SwVbaFind::getWrap() throw (uno::RuntimeException)
{
    // seems not supported in Writer
    return mnWrap;
}

void SAL_CALL SwVbaFind::setWrap( ::sal_Int32 _wrap ) throw (uno::RuntimeException)
{
    // seems not supported in Writer
    mnWrap = _wrap;
}

::sal_Bool SAL_CALL SwVbaFind::getFormat() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getValueSearch();
}

void SAL_CALL SwVbaFind::setFormat( ::sal_Bool _format ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setValueSearch( _format );
}

::sal_Bool SAL_CALL SwVbaFind::getMatchCase() throw (uno::RuntimeException)
{
    sal_Bool value = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchCaseSensitive") ) ) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchCase( ::sal_Bool _matchcase ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchCaseSensitive") ), uno::makeAny( _matchcase ) );
}

::sal_Bool SAL_CALL SwVbaFind::getMatchWholeWord() throw (uno::RuntimeException)
{
    sal_Bool value = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchWords") ) ) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWholeWord( ::sal_Bool _matchwholeword ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchWords") ), uno::makeAny( _matchwholeword ) );
}

::sal_Bool SAL_CALL SwVbaFind::getMatchWildcards() throw (uno::RuntimeException)
{
    sal_Bool value = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchRegularExpression") ) ) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchWildcards( ::sal_Bool _matchwildcards ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchRegularExpression") ), uno::makeAny( _matchwildcards ) );
}

::sal_Bool SAL_CALL SwVbaFind::getMatchSoundsLike() throw (uno::RuntimeException)
{
    sal_Bool value = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ) ) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchSoundsLike( ::sal_Bool _matchsoundslike ) throw (uno::RuntimeException)
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ), uno::makeAny( _matchsoundslike ) );
}

::sal_Bool SAL_CALL SwVbaFind::getMatchAllWordForms() throw (uno::RuntimeException)
{
    sal_Bool value = sal_False;
    mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ) ) >>= value;
    if( value )
        mxPropertyReplace->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRelax") ) ) >>= value;
    return value;
}

void SAL_CALL SwVbaFind::setMatchAllWordForms( ::sal_Bool _matchallwordforms ) throw (uno::RuntimeException)
{
    // seems not accurate
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ), uno::makeAny( _matchallwordforms ) );
    mxPropertyReplace->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRelax") ), uno::makeAny( _matchallwordforms ) );
}

uno::Any SAL_CALL SwVbaFind::getStyle() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

void SAL_CALL SwVbaFind::setStyle( const uno::Any& /*_style */ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

sal_Bool SAL_CALL
SwVbaFind::Execute( const uno::Any& FindText, const uno::Any& MatchCase, const uno::Any& MatchWholeWord, const uno::Any& MatchWildcards, const uno::Any& MatchSoundsLike, const uno::Any& MatchAllWordForms, const uno::Any& Forward, const uno::Any& Wrap, const uno::Any& Format, const uno::Any& ReplaceWith, const uno::Any& Replace, const uno::Any& /*MatchKashida*/, const uno::Any& /*MatchDiacritics*/, const uno::Any& /*MatchAlefHamza*/, const uno::Any& /*MatchControl*/, const uno::Any& /*MatchPrefix*/, const uno::Any& /*MatchSuffix*/, const uno::Any& /*MatchPhrase*/, const uno::Any& /*IgnoreSpace*/, const uno::Any& /*IgnorePunct*/ ) throw (uno::RuntimeException)
{
    sal_Bool result = sal_False;
    if( FindText.hasValue() )
    {
        rtl::OUString sText;
        FindText >>= sText;
        setText( sText );
    }

    sal_Bool bValue = sal_False;
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
        rtl::OUString sValue;
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
SwVbaFind::ClearFormatting(  ) throw (uno::RuntimeException)
{
    uno::Sequence< beans::PropertyValue >  aSearchAttribs;
    mxPropertyReplace->setSearchAttributes( aSearchAttribs );
}

rtl::OUString&
SwVbaFind::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaFind") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaFind::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Find" ) );
    }
    return aServiceNames;
}

