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
#include "vbalisthelper.hxx"
#include <vbahelper/vbahelper.hxx>
#include <sal/log.hxx>
#include <ooo/vba/word/WdListGalleryType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int32 LIST_LEVEL_COUNT = 9;

constexpr OUStringLiteral UNO_NAME_PARENT_NUMBERING = u"ParentNumbering";
constexpr OUStringLiteral UNO_NAME_PREFIX = u"Prefix";
constexpr OUStringLiteral UNO_NAME_SUFFIX = u"Suffix";
constexpr OUStringLiteral UNO_NAME_CHAR_STYLE_NAME = u"CharStyleName";
constexpr OUStringLiteral UNO_NAME_NUMBERING_TYPE = u"NumberingType";
constexpr OUStringLiteral UNO_NAME_BULLET_CHAR = u"BulletChar";

const sal_Unicode CHAR_CLOSED_DOT[] = u"\u2022";
const char CHAR_EMPTY_DOT[] = "o";
const sal_Unicode CHAR_SQUARE[] = u"\u2540";
const sal_Unicode CHAR_STAR_SYMBOL[] = u"\u272A";
const sal_Unicode CHAR_FOUR_DIAMONDS[] = u"\u2756";
const sal_Unicode CHAR_DIAMOND[] = u"\u2726";
const sal_Unicode CHAR_ARROW[] = u"\u27A2";
const sal_Unicode CHAR_CHECK_MARK[] = u"\u2713";

SwVbaListHelper::SwVbaListHelper( const css::uno::Reference< css::text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType ) : mxTextDocument( xTextDoc ), mnGalleryType( nGalleryType ), mnTemplateType( nTemplateType )
{
    Init();
}

void SwVbaListHelper::Init()
{
    // set the numbering style name
    switch( mnGalleryType )
    {
        case word::WdListGalleryType::wdBulletGallery:
        {
            msStyleName = "WdBullet";
            break;
        }
        case word::WdListGalleryType::wdNumberGallery:
        {
            msStyleName = "WdNumber";
            break;
        }
        case word::WdListGalleryType::wdOutlineNumberGallery:
        {
            msStyleName = "WdOutlineNumber";
            break;
        }
        default:
        {
            throw uno::RuntimeException();
        }
    }
    msStyleName += OUString::number( mnTemplateType );

    // get the numbering style
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxTextDocument, uno::UNO_QUERY_THROW );
    mxStyleFamily.set( xStyleSupplier->getStyleFamilies()->getByName("NumberingStyles"), uno::UNO_QUERY_THROW );
    SAL_INFO("sw.vba", "numbering style name: " << msStyleName );
    if( mxStyleFamily->hasByName( msStyleName ) )
    {
        mxStyleProps.set( mxStyleFamily->getByName( msStyleName ), uno::UNO_QUERY_THROW );
        mxNumberingRules.set( mxStyleProps->getPropertyValue("NumberingRules"), uno::UNO_QUERY_THROW );
    }
    else
    {
        // create new numbering style
        uno::Reference< lang::XMultiServiceFactory > xDocMSF( mxTextDocument, uno::UNO_QUERY_THROW );
        mxStyleProps.set( xDocMSF->createInstance("com.sun.star.style.NumberingStyle"), uno::UNO_QUERY_THROW );
        // insert this style into style family, or the property NumberingRules doesn't exist.
        mxStyleFamily->insertByName( msStyleName, uno::makeAny( mxStyleProps ) );
        mxStyleProps->getPropertyValue("NumberingRules") >>= mxNumberingRules;

        CreateListTemplate();

        mxStyleProps->setPropertyValue("NumberingRules", uno::makeAny( mxNumberingRules ) );
    }
}

void SwVbaListHelper::CreateListTemplate()
{
    switch( mnGalleryType )
    {
        case word::WdListGalleryType::wdBulletGallery:
        {
            CreateBulletListTemplate();
            break;
        }
        case word::WdListGalleryType::wdNumberGallery:
        {
            CreateNumberListTemplate();
            break;
        }
        case word::WdListGalleryType::wdOutlineNumberGallery:
        {
            CreateOutlineNumberListTemplate();
            break;
        }
        default:
        {
            throw uno::RuntimeException();
        }
    }
}

void SwVbaListHelper::CreateBulletListTemplate()
{
    // there is only 1 level for each bullet list in MSWord
    sal_Int32 nLevel = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    setOrAppendPropertyValue( aPropertyValues, UNO_NAME_CHAR_STYLE_NAME, uno::makeAny( OUString( "Bullet Symbols" ) ) );
    setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( sal_Int16(style::NumberingType::CHAR_SPECIAL) ) );

    OUString aBulletChar;
    switch( mnTemplateType )
    {
        case 1:
        {
            aBulletChar = CHAR_CLOSED_DOT;
            break;
        }
        case 2:
        {
            aBulletChar = CHAR_EMPTY_DOT;
            break;
        }
        case 3:
        {
            aBulletChar = CHAR_SQUARE;
            break;
        }
        case 4:
        {
            aBulletChar = CHAR_STAR_SYMBOL;
            break;
        }
        case 5:
        {
            aBulletChar = CHAR_FOUR_DIAMONDS;
            break;
        }
        case 6:
        {
            aBulletChar = CHAR_ARROW;
            break;
        }
        case 7:
        {
            aBulletChar = CHAR_CHECK_MARK;
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, UNO_NAME_BULLET_CHAR, uno::makeAny( aBulletChar ) );

    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
}

void SwVbaListHelper::CreateNumberListTemplate()
{
    // there is only 1 level for each bullet list in MSWord
    sal_Int32 nLevel = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;

    sal_Int16 nNumberingType = 0;
    OUString sSuffix;
    switch( mnTemplateType )
    {
        case 1:
        {
            nNumberingType = style::NumberingType::ARABIC;
            sSuffix = ".";
            break;
        }
        case 2:
        {
            nNumberingType = style::NumberingType::ARABIC;
            sSuffix = ")";
            break;
        }
        case 3:
        {
            nNumberingType = style::NumberingType::ROMAN_UPPER;
            sSuffix = ".";
            break;
        }
        case 4:
        {
            nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
            sSuffix = ".";
            break;
        }
        case 5:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = ")";
            break;
        }
        case 6:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = ".";
            break;
        }
        case 7:
        {
            nNumberingType = style::NumberingType::ROMAN_LOWER;
            sSuffix = ".";
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( nNumberingType ) );
    setOrAppendPropertyValue( aPropertyValues, UNO_NAME_SUFFIX, uno::makeAny( sSuffix ) );

    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
}

void SwVbaListHelper::CreateOutlineNumberListTemplate()
{
    switch( mnTemplateType )
    {
        case 1:
        {
            CreateOutlineNumberForType1();
            break;
        }
        case 2:
        {
            CreateOutlineNumberForType2();
            break;
        }
        case 3:
        {
            CreateOutlineNumberForType3();
            break;
        }
        case 4:
        {
            CreateOutlineNumberForType4();
            break;
        }
        case 5:
        {
            CreateOutlineNumberForType5();
            break;
        }
        case 6:
        {
            CreateOutlineNumberForType6();
            break;
        }
        case 7:
        {
            CreateOutlineNumberForType7();
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
}

void SwVbaListHelper::CreateOutlineNumberForType1()
{
    sal_Int16 nNumberingType = 0;
    OUString sPrefix;
    OUString sSuffix;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        switch( nLevel )
        {
            case 0:
            case 1:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
        }
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PREFIX, uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_SUFFIX, uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType2()
{
    sal_Int16 nParentNumbering = 0;
    OUString sSuffix( '.' );
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( sal_Int16(style::NumberingType::ARABIC) ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_SUFFIX, uno::makeAny( sSuffix ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PARENT_NUMBERING, uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType3()
{
    OUString aBulletChar;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( sal_Int16(style::NumberingType::CHAR_SPECIAL) ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_CHAR_STYLE_NAME, uno::makeAny( OUString("Bullet Symbols") ) );
        switch( nLevel )
        {
            case 0:
            {
                aBulletChar = CHAR_FOUR_DIAMONDS;
                break;
            }
            case 1:
            case 5:
            {
                aBulletChar = CHAR_ARROW;
                break;
            }
            case 2:
            case 6:
            {
                aBulletChar = CHAR_SQUARE;
                break;
            }
            case 3:
            case 7:
            {
                aBulletChar = CHAR_CLOSED_DOT;
                break;
            }
            case 4:
            case 8:
            {
                aBulletChar = CHAR_DIAMOND;
                break;
            }
        }
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_BULLET_CHAR, uno::makeAny( aBulletChar ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType4()
{
    sal_Int16 nNumberingType = 0;
    OUString sPrefix;
    OUString sSuffix;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        switch( nLevel )
        {
            case 0:
            {
                nNumberingType = style::NumberingType::ROMAN_UPPER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix.clear();
                sSuffix = ".";
                setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PARENT_NUMBERING, uno::makeAny( sal_Int16(0) ) );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
        }
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PREFIX, uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_SUFFIX, uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType5()
{
    sal_Int16 nParentNumbering = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( sal_Int16(style::NumberingType::ARABIC) ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PARENT_NUMBERING, uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType6()
{
    sal_Int16 nNumberingType = 0;
    OUString sPrefix;
    OUString sSuffix;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        switch( nLevel )
        {
            case 0:
            {
                nNumberingType = style::NumberingType::ROMAN_UPPER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
                sPrefix.clear();
                sSuffix = ".";
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix.clear();
                sSuffix = ")";
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = "(";
                sSuffix = ")";
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = "(";
                sSuffix = ".";
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = "(";
                sSuffix = ".";
                break;
            }
        }
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PREFIX, uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_SUFFIX, uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType7()
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_NUMBERING_TYPE, uno::makeAny( sal_Int16(style::NumberingType::ARABIC) ) );
        setOrAppendPropertyValue( aPropertyValues, UNO_NAME_PREFIX, uno::makeAny( OUString("Chapter ") ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

uno::Any SwVbaListHelper::getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    return getPropertyValue( aPropertyValues, sName );
}

void SwVbaListHelper::setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName, const css::uno::Any& aValue )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    setOrAppendPropertyValue( aPropertyValues, sName, aValue );
    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    mxStyleProps->setPropertyValue("NumberingRules", uno::makeAny( mxNumberingRules ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
