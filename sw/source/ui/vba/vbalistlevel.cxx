/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbalistlevel.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/style/NumberingType.hpp>
#include <ooo/vba/word/WdListNumberStyle.hpp>
#include <ooo/vba/word/WdTrailingCharacter.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <ooo/vba/word/WdListLevelAlignment.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaListLevel::SwVbaListLevel( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, SwVbaListHelperRef pHelper, sal_Int32 nLevel ) throw ( uno::RuntimeException ) : SwVbaListLevel_BASE( rParent, rContext ), pListHelper( pHelper ), mnLevel( nLevel )
{
}

SwVbaListLevel::~SwVbaListLevel()
{
}

::sal_Int32 SAL_CALL SwVbaListLevel::getAlignment() throw (uno::RuntimeException)
{
    sal_Int16 nAlignment = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Adjust") ) ) >>= nAlignment;
    switch( nAlignment )
    {
        case text::HoriOrientation::LEFT:
        {
            nAlignment = word::WdListLevelAlignment::wdListLevelAlignLeft;
            break;
        }
        case text::HoriOrientation::RIGHT:
        {
            nAlignment = word::WdListLevelAlignment::wdListLevelAlignRight;
            break;
        }
        case text::HoriOrientation::CENTER:
        {
            nAlignment = word::WdListLevelAlignment::wdListLevelAlignCenter;
            break;
        }
        default:
        {
            throw uno::RuntimeException();
        }
    }
    return nAlignment;
}

void SAL_CALL SwVbaListLevel::setAlignment( ::sal_Int32 _alignment ) throw (uno::RuntimeException)
{
    sal_Int16 nAlignment = text::HoriOrientation::LEFT;
    switch( _alignment )
    {
        case word::WdListLevelAlignment::wdListLevelAlignLeft:
        {
            nAlignment = text::HoriOrientation::LEFT;
            break;
        }
        case word::WdListLevelAlignment::wdListLevelAlignRight:
        {
            nAlignment = text::HoriOrientation::RIGHT;
            break;
        }
        case word::WdListLevelAlignment::wdListLevelAlignCenter:
        {
            nAlignment = text::HoriOrientation::CENTER;
            break;
        }
        default:
        {
            throw uno::RuntimeException();
        }
    }
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Adjust") ), uno::makeAny( nAlignment ) );
}

uno::Reference< ::ooo::vba::word::XFont > SAL_CALL SwVbaListLevel::getFont() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    return uno::Reference< word::XFont >();
}

void SAL_CALL SwVbaListLevel::setFont( const uno::Reference< ::ooo::vba::word::XFont >& /*_font*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getIndex() throw (uno::RuntimeException)
{
    return mnLevel + 1;
}

::rtl::OUString SAL_CALL SwVbaListLevel::getLinkedStyle() throw (uno::RuntimeException)
{
    // TODO:
    return rtl::OUString();
}

void SAL_CALL SwVbaListLevel::setLinkedStyle( const ::rtl::OUString& /*_linkedstyle*/ ) throw (uno::RuntimeException)
{
    // TODO:
}

::rtl::OUString SAL_CALL SwVbaListLevel::getNumberFormat() throw (uno::RuntimeException)
{
    // TODO::
    return rtl::OUString();
}

void SAL_CALL SwVbaListLevel::setNumberFormat( const ::rtl::OUString& /*_numberformat*/ ) throw (uno::RuntimeException)
{
    // TODO::
}

float SAL_CALL SwVbaListLevel::getNumberPosition() throw (uno::RuntimeException)
{
    // indentAt + firstlineindent
    sal_Int32 nIndentAt = 0;
    sal_Int32 nFirstLineIndent = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IndentAt") ) ) >>= nIndentAt;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstLineIndent") ) ) >>= nFirstLineIndent;

    sal_Int32 nResult = nIndentAt + nFirstLineIndent;

    return static_cast< float >( Millimeter::getInPoints( nResult ) );
}

void SAL_CALL SwVbaListLevel::setNumberPosition( float _numberposition ) throw (uno::RuntimeException)
{
    sal_Int32 nNumberPosition = Millimeter::getInHundredthsOfOneMillimeter( _numberposition );

    sal_Int32 nIndentAt = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IndentAt") ) ) >>= nIndentAt;

    sal_Int32 nFirstLineIndent = nNumberPosition - nIndentAt;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstLineIndent") ), uno::makeAny( nFirstLineIndent ) );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getNumberStyle() throw (uno::RuntimeException)
{
    sal_Int16 nNumberingType = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingType") ) ) >>= nNumberingType;
    switch( nNumberingType )
    {
        case style::NumberingType::CHAR_SPECIAL:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleBullet;
            break;
        }
        case style::NumberingType::CHARS_UPPER_LETTER:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleUppercaseLetter;
            break;
        }
        case style::NumberingType::CHARS_LOWER_LETTER:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleLowercaseLetter;
            break;
        }
        case style::NumberingType::ROMAN_UPPER:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleUppercaseRoman;
            break;
        }
        case style::NumberingType::ROMAN_LOWER:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleLowercaseRoman;
            break;
        }
        case style::NumberingType::ARABIC:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleArabic;
            break;
        }
        case style::NumberingType::NUMBER_NONE:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleNone;
            break;
        }
        case style::NumberingType::FULLWIDTH_ARABIC:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleArabicFullWidth;
            break;
        }
        case style::NumberingType::CIRCLE_NUMBER:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleNumberInCircle;
            break;
        }
        case style::NumberingType::CHARS_ARABIC:
        {
            nNumberingType = word::WdListNumberStyle::wdListNumberStyleCardinalText;
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
        }
    }
    return nNumberingType;
}

void SAL_CALL SwVbaListLevel::setNumberStyle( ::sal_Int32 _numberstyle ) throw (uno::RuntimeException)
{
    sal_Int16 nNumberingType = 0;
    switch( _numberstyle )
    {
        case word::WdListNumberStyle::wdListNumberStyleBullet:
        {
            nNumberingType = style::NumberingType::CHAR_SPECIAL;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleUppercaseLetter:
        {
            nNumberingType = style::NumberingType::CHARS_UPPER_LETTER_N;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleLowercaseLetter:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER_N;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleUppercaseRoman:
        {
            nNumberingType = style::NumberingType::ROMAN_UPPER;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleLowercaseRoman:
        {
            nNumberingType = style::NumberingType::ROMAN_LOWER;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleArabic:
        {
            nNumberingType = style::NumberingType::ARABIC;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleNone:
        {
            nNumberingType = style::NumberingType::NUMBER_NONE;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleArabicFullWidth:
        {
            nNumberingType = style::NumberingType::FULLWIDTH_ARABIC;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleNumberInCircle:
        {
            nNumberingType = style::NumberingType::CIRCLE_NUMBER;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleCardinalText:
        {
            nNumberingType = style::NumberingType::CHARS_ARABIC;
            break;
        }
        case word::WdListNumberStyle::wdListNumberStyleOrdinal:
        case word::WdListNumberStyle::wdListNumberStyleOrdinalText:
        case word::WdListNumberStyle::wdListNumberStyleKanji:
        case word::WdListNumberStyle::wdListNumberStyleKanjiDigit:
        case word::WdListNumberStyle::wdListNumberStyleAiueoHalfWidth:
        case word::WdListNumberStyle::wdListNumberStyleIrohaHalfWidth:
        {
            nNumberingType = style::NumberingType::ARABIC;
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
        }
    }

    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingType") ), uno::makeAny( nNumberingType ) );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getResetOnHigher() throw (uno::RuntimeException)
{
    //seems not support?
    return 0;
}

void SAL_CALL SwVbaListLevel::setResetOnHigher( ::sal_Int32 /*_resetonhigher*/ ) throw (uno::RuntimeException)
{
    //seems not support?
}

::sal_Int32 SAL_CALL SwVbaListLevel::getStartAt() throw (uno::RuntimeException)
{
    sal_Int16 nStartWith = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StartWith") ) ) >>= nStartWith;
    return nStartWith;
}

void SAL_CALL SwVbaListLevel::setStartAt( ::sal_Int32 _startat ) throw (uno::RuntimeException)
{
    sal_Int16 nStartWith = (sal_Int16)_startat;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StartWith") ), uno::makeAny( nStartWith ) );
}

float SAL_CALL SwVbaListLevel::getTabPosition() throw (uno::RuntimeException)
{
    sal_Int32 nTabPosition = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ListtabStopPosition") ) ) >>= nTabPosition;

    return static_cast< float >( Millimeter::getInPoints( nTabPosition ) );
}

void SAL_CALL SwVbaListLevel::setTabPosition( float _tabposition ) throw (uno::RuntimeException)
{
    sal_Int32 nTabPosition = Millimeter::getInHundredthsOfOneMillimeter( _tabposition );
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ListtabStopPosition") ), uno::makeAny( nTabPosition ) );
}

float SAL_CALL SwVbaListLevel::getTextPosition() throw (uno::RuntimeException)
{
    // indentAt
    sal_Int32 nIndentAt = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IndentAt") ) ) >>= nIndentAt;

    return static_cast< float >( Millimeter::getInPoints( nIndentAt ) );
}

void SAL_CALL SwVbaListLevel::setTextPosition( float _textposition ) throw (uno::RuntimeException)
{
    sal_Int32 nIndentAt = 0;
    sal_Int32 nFirstLineIndent = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IndentAt") ) ) >>= nIndentAt;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstLineIndent") ) ) >>= nFirstLineIndent;

    sal_Int32 nAlignedAt = nIndentAt + nFirstLineIndent;

    nIndentAt = Millimeter::getInHundredthsOfOneMillimeter( _textposition );
    nFirstLineIndent = nAlignedAt - nIndentAt;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IndentAt") ), uno::makeAny( nIndentAt ) );
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstLineIndent") ), uno::makeAny( nFirstLineIndent ) );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getTrailingCharacter() throw (uno::RuntimeException)
{
    sal_Int16 nLabelFollowedBy= 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LabelFollowedBy") ) ) >>= nLabelFollowedBy;

    return nLabelFollowedBy;
}

void SAL_CALL SwVbaListLevel::setTrailingCharacter( ::sal_Int32 _trailingcharacter ) throw (uno::RuntimeException)
{
    sal_Int16 nLabelFollowedBy = (sal_Int16)_trailingcharacter;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LabelFollowedBy") ), uno::makeAny( nLabelFollowedBy ) );
}

rtl::OUString&
SwVbaListLevel::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaListLevel") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaListLevel::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.ListLevel" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
