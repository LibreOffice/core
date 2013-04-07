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
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("Adjust") ) >>= nAlignment;
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
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("Adjust"), uno::makeAny( nAlignment ) );
}

uno::Reference< ::ooo::vba::word::XFont > SAL_CALL SwVbaListLevel::getFont() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( OUString("Not implemented"), uno::Reference< uno::XInterface >() );
}

void SAL_CALL SwVbaListLevel::setFont( const uno::Reference< ::ooo::vba::word::XFont >& /*_font*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( OUString("Not implemented"), uno::Reference< uno::XInterface >() );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getIndex() throw (uno::RuntimeException)
{
    return mnLevel + 1;
}

OUString SAL_CALL SwVbaListLevel::getLinkedStyle() throw (uno::RuntimeException)
{
    // TODO:
    return OUString();
}

void SAL_CALL SwVbaListLevel::setLinkedStyle( const OUString& /*_linkedstyle*/ ) throw (uno::RuntimeException)
{
    // TODO:
}

OUString SAL_CALL SwVbaListLevel::getNumberFormat() throw (uno::RuntimeException)
{
    // TODO::
    return OUString();
}

void SAL_CALL SwVbaListLevel::setNumberFormat( const OUString& /*_numberformat*/ ) throw (uno::RuntimeException)
{
    // TODO::
}

float SAL_CALL SwVbaListLevel::getNumberPosition() throw (uno::RuntimeException)
{
    // indentAt + firstlineindent
    sal_Int32 nIndentAt = 0;
    sal_Int32 nFirstLineIndent = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("IndentAt") ) >>= nIndentAt;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("FirstLineIndent") ) >>= nFirstLineIndent;

    sal_Int32 nResult = nIndentAt + nFirstLineIndent;

    return static_cast< float >( Millimeter::getInPoints( nResult ) );
}

void SAL_CALL SwVbaListLevel::setNumberPosition( float _numberposition ) throw (uno::RuntimeException)
{
    sal_Int32 nNumberPosition = Millimeter::getInHundredthsOfOneMillimeter( _numberposition );

    sal_Int32 nIndentAt = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("IndentAt") ) >>= nIndentAt;

    sal_Int32 nFirstLineIndent = nNumberPosition - nIndentAt;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("FirstLineIndent"), uno::makeAny( nFirstLineIndent ) );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getNumberStyle() throw (uno::RuntimeException)
{
    sal_Int16 nNumberingType = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("NumberingType") ) >>= nNumberingType;
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
            throw uno::RuntimeException( OUString("Not implemented"), uno::Reference< uno::XInterface >() );
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
            throw uno::RuntimeException( OUString("Not implemented"), uno::Reference< uno::XInterface >() );
        }
    }

    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("NumberingType"), uno::makeAny( nNumberingType ) );
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
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("StartWith") ) >>= nStartWith;
    return nStartWith;
}

void SAL_CALL SwVbaListLevel::setStartAt( ::sal_Int32 _startat ) throw (uno::RuntimeException)
{
    sal_Int16 nStartWith = (sal_Int16)_startat;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("StartWith"), uno::makeAny( nStartWith ) );
}

float SAL_CALL SwVbaListLevel::getTabPosition() throw (uno::RuntimeException)
{
    sal_Int32 nTabPosition = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("ListtabStopPosition") ) >>= nTabPosition;

    return static_cast< float >( Millimeter::getInPoints( nTabPosition ) );
}

void SAL_CALL SwVbaListLevel::setTabPosition( float _tabposition ) throw (uno::RuntimeException)
{
    sal_Int32 nTabPosition = Millimeter::getInHundredthsOfOneMillimeter( _tabposition );
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("ListtabStopPosition"), uno::makeAny( nTabPosition ) );
}

float SAL_CALL SwVbaListLevel::getTextPosition() throw (uno::RuntimeException)
{
    // indentAt
    sal_Int32 nIndentAt = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("IndentAt") ) >>= nIndentAt;

    return static_cast< float >( Millimeter::getInPoints( nIndentAt ) );
}

void SAL_CALL SwVbaListLevel::setTextPosition( float _textposition ) throw (uno::RuntimeException)
{
    sal_Int32 nIndentAt = 0;
    sal_Int32 nFirstLineIndent = 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("IndentAt") ) >>= nIndentAt;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("FirstLineIndent") ) >>= nFirstLineIndent;

    sal_Int32 nAlignedAt = nIndentAt + nFirstLineIndent;

    nIndentAt = Millimeter::getInHundredthsOfOneMillimeter( _textposition );
    nFirstLineIndent = nAlignedAt - nIndentAt;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("IndentAt"), uno::makeAny( nIndentAt ) );
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("FirstLineIndent"), uno::makeAny( nFirstLineIndent ) );
}

::sal_Int32 SAL_CALL SwVbaListLevel::getTrailingCharacter() throw (uno::RuntimeException)
{
    sal_Int16 nLabelFollowedBy= 0;
    pListHelper->getPropertyValueWithNameAndLevel( mnLevel, OUString("LabelFollowedBy") ) >>= nLabelFollowedBy;

    return nLabelFollowedBy;
}

void SAL_CALL SwVbaListLevel::setTrailingCharacter( ::sal_Int32 _trailingcharacter ) throw (uno::RuntimeException)
{
    sal_Int16 nLabelFollowedBy = (sal_Int16)_trailingcharacter;
    pListHelper->setPropertyValueWithNameAndLevel( mnLevel, OUString("LabelFollowedBy"), uno::makeAny( nLabelFollowedBy ) );
}

OUString
SwVbaListLevel::getServiceImplName()
{
    return OUString("SwVbaListLevel");
}

uno::Sequence< OUString >
SwVbaListLevel::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.word.ListLevel" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
