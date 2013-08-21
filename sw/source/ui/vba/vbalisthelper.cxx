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
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdListGalleryType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static const sal_Int32 LIST_LEVEL_COUNT = 9;

static const char WORD_BULLET_GALLERY[] = "WdBullet";
static const char WORD_NUMBER_GALLERY[] = "WdNumber";
static const char WORD_OUTLINE_NUMBER_GALLERY[] = "WdOutlineNumber";

static const char UNO_NAME_PARENT_NUMBERING[] = "ParentNumbering";
static const char UNO_NAME_PREFIX[] = "Prefix";
static const char UNO_NAME_SUFFIX[] = "Suffix";
static const char UNO_NAME_CHAR_STYLE_NAME[] = "CharStyleName";
static const char UNO_NAME_NUMBERING_TYPE[] = "NumberingType";
static const char UNO_NAME_BULLET_CHAR[] = "BulletChar";

static const sal_Int16 CHAR_CLOSED_DOT = 8226;
static const sal_Int16 CHAR_EMPTY_DOT = 111;
static const sal_Int16 CHAR_SQUARE = 9632;
static const sal_Int16 CHAR_STAR_SYMBOL = 10026;
static const sal_Int16 CHAR_FOUR_DIAMONDS = 10070;
static const sal_Int16 CHAR_DIAMOND = 10022;
static const sal_Int16 CHAR_ARROW = 10146;
static const sal_Int16 CHAR_CHECK_MARK = 10003;

SwVbaListHelper::SwVbaListHelper( const css::uno::Reference< css::text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType ) throw( css::uno::RuntimeException ) : mxTextDocument( xTextDoc ), mnGalleryType( nGalleryType ), mnTemplateType( nTemplateType )
{
    Init();
}

void SwVbaListHelper::Init() throw( css::uno::RuntimeException )
{
    // set the numbering style name
    switch( mnGalleryType )
    {
        case word::WdListGalleryType::wdBulletGallery:
        {
            msStyleName = OUString(WORD_BULLET_GALLERY );
            break;
        }
        case word::WdListGalleryType::wdNumberGallery:
        {
            msStyleName = OUString(WORD_NUMBER_GALLERY );
            break;
        }
        case word::WdListGalleryType::wdOutlineNumberGallery:
        {
            msStyleName = OUString(WORD_OUTLINE_NUMBER_GALLERY );
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
    OSL_TRACE("SwVbaListHelper::Init: numbering style name: %s", OUStringToOString( msStyleName, RTL_TEXTENCODING_UTF8 ).getStr() );
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

void SwVbaListHelper::CreateListTemplate() throw( css::uno::RuntimeException )
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

void SwVbaListHelper::CreateBulletListTemplate() throw( css::uno::RuntimeException )
{
    // there is only 1 level for each bullet list in MSWord
    sal_Int32 nLevel = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    OUString sCharStyleName( "Bullet Symbols" );
    setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_CHAR_STYLE_NAME ), uno::makeAny( sCharStyleName ) );
    sal_Int16 nNumberingType = style::NumberingType::CHAR_SPECIAL;
    setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );

    OUString aBulletChar;
    switch( mnTemplateType )
    {
        case 1:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_CLOSED_DOT ) );
            break;
        }
        case 2:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_EMPTY_DOT ) );
            break;
        }
        case 3:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_SQUARE ) );
            break;
        }
        case 4:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_STAR_SYMBOL ) );
            break;
        }
        case 5:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_FOUR_DIAMONDS ) );
            break;
        }
        case 6:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_ARROW ) );
            break;
        }
        case 7:
        {
            aBulletChar = OUString( sal_Unicode( CHAR_CHECK_MARK ) );
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_BULLET_CHAR ), uno::makeAny( aBulletChar ) );

    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
}

void SwVbaListHelper::CreateNumberListTemplate() throw( css::uno::RuntimeException )
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
            sSuffix = OUString( sal_Unicode('.') );
            break;
        }
        case 2:
        {
            nNumberingType = style::NumberingType::ARABIC;
            sSuffix = OUString( sal_Unicode(')') );
            break;
        }
        case 3:
        {
            nNumberingType = style::NumberingType::ROMAN_UPPER;
            sSuffix = OUString( sal_Unicode('.') );
            break;
        }
        case 4:
        {
            nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
            sSuffix = OUString( sal_Unicode('.') );
            break;
        }
        case 5:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = OUString( sal_Unicode(')') );
            break;
        }
        case 6:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = OUString( sal_Unicode('.') );
            break;
        }
        case 7:
        {
            nNumberingType = style::NumberingType::ROMAN_LOWER;
            sSuffix = OUString( sal_Unicode('.') );
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
    setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_SUFFIX ), uno::makeAny( sSuffix ) );

    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
}

void SwVbaListHelper::CreateOutlineNumberListTemplate() throw( css::uno::RuntimeException )
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

void SwVbaListHelper::CreateOutlineNumberForType1() throw( css::uno::RuntimeException )
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
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PREFIX ), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_SUFFIX ), uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType2() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::ARABIC;
    sal_Int16 nParentNumbering = 0;
    OUString sSuffix = OUString( sal_Unicode('.') );
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_SUFFIX ), uno::makeAny( sSuffix ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PARENT_NUMBERING ), uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType3() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::CHAR_SPECIAL;
    OUString sCharStyleName( "Bullet Symbols" );
    OUString aBulletChar;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_CHAR_STYLE_NAME ), uno::makeAny( sCharStyleName ) );
        switch( nLevel )
        {
            case 0:
            {
                aBulletChar = OUString( sal_Unicode( CHAR_FOUR_DIAMONDS ) );
                break;
            }
            case 1:
            case 5:
            {
                aBulletChar = OUString( sal_Unicode( CHAR_ARROW ) );
                break;
            }
            case 2:
            case 6:
            {
                aBulletChar = OUString( sal_Unicode( CHAR_SQUARE ) );
                break;
            }
            case 3:
            case 7:
            {
                aBulletChar = OUString( sal_Unicode( CHAR_CLOSED_DOT ) );
                break;
            }
            case 4:
            case 8:
            {
                aBulletChar = OUString( sal_Unicode( CHAR_DIAMOND ) );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_BULLET_CHAR ), uno::makeAny( aBulletChar ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType4() throw( css::uno::RuntimeException )
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
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                sal_Int16 nParentNumbering = 0;
                setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PARENT_NUMBERING ), uno::makeAny( nParentNumbering ) );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PREFIX ), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_SUFFIX ), uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType5() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::ARABIC;
    sal_Int16 nParentNumbering = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PARENT_NUMBERING ), uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType6() throw( css::uno::RuntimeException )
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
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString();
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode(')') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = OUString( sal_Unicode('(') );
                sSuffix = OUString( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PREFIX ), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_SUFFIX ), uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType7() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::ARABIC;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    OUString sPrefix("Chapter ");

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_NUMBERING_TYPE ), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, OUString(UNO_NAME_PREFIX ), uno::makeAny( sPrefix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

uno::Any SwVbaListHelper::getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName ) throw( css::uno::RuntimeException )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    return getPropertyValue( aPropertyValues, sName );
}

void SwVbaListHelper::setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName, const css::uno::Any& aValue ) throw( css::uno::RuntimeException )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    setOrAppendPropertyValue( aPropertyValues, sName, aValue );
    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    mxStyleProps->setPropertyValue("NumberingRules", uno::makeAny( mxNumberingRules ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
