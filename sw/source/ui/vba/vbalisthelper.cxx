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

static const char UNO_NAME_ADJUST[] = "Adjust";
static const char UNO_NAME_PARENT_NUMBERING[] = "ParentNumbering";
static const char UNO_NAME_PREFIX[] = "Prefix";
static const char UNO_NAME_SUFFIX[] = "Suffix";
static const char UNO_NAME_CHAR_STYLE_NAME[] = "CharStyleName";
static const char UNO_NAME_START_WITH[] = "StartWith";
static const char UNO_NAME_POSITION_AND_SPACE_MODE[] = "PositionAndSpaceMode";
static const char UNO_NAME_LABEL_FOLLOWED_BY[] = "LabelFollowedBy";
static const char UNO_NAME_LIST_TAB_STOP_POSITION[] = "ListtabStopPosition";
static const char UNO_NAME_FIRST_LINE_INDENT[] = "FirstLineIndent";
static const char UNO_NAME_INDENT_AT[] = "IndentAt";
static const char UNO_NAME_NUMBERING_TYPE[] = "NumberingType";
static const char UNO_NAME_BULLET_ID[] = "BulletId";
static const char UNO_NAME_BULLET_CHAR[] = "BulletChar";
static const char UNO_NAME_BULLET_FONT_NAME[] = "BulletFontName";
static const char UNO_NAME_BULLET_FONT[] = "BulletFont";

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
            msStyleName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( WORD_BULLET_GALLERY ));
            break;
        }
        case word::WdListGalleryType::wdNumberGallery:
        {
            msStyleName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( WORD_NUMBER_GALLERY ));
            break;
        }
        case word::WdListGalleryType::wdOutlineNumberGallery:
        {
            msStyleName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( WORD_OUTLINE_NUMBER_GALLERY ));
            break;
        }
        default:
        {
            throw uno::RuntimeException();
        }
    }
    msStyleName += rtl::OUString::valueOf( mnTemplateType );

    // get the numbering style
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxTextDocument, uno::UNO_QUERY_THROW );
    mxStyleFamily.set( xStyleSupplier->getStyleFamilies()->getByName(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingStyles") ) ), uno::UNO_QUERY_THROW );
    OSL_TRACE("SwVbaListHelper::Init: numbering style name: %s", rtl::OUStringToOString( msStyleName, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( mxStyleFamily->hasByName( msStyleName ) )
    {
        mxStyleProps.set( mxStyleFamily->getByName( msStyleName ), uno::UNO_QUERY_THROW );
        mxNumberingRules.set( mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingRules") ) ), uno::UNO_QUERY_THROW );
    }
    else
    {
        // create new numbering style
        uno::Reference< lang::XMultiServiceFactory > xDocMSF( mxTextDocument, uno::UNO_QUERY_THROW );
        mxStyleProps.set( xDocMSF->createInstance(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.NumberingStyle") ) ), uno::UNO_QUERY_THROW );
        // insert this style into style family, or the property NumberingRules doesn't exist.
        mxStyleFamily->insertByName( msStyleName, uno::makeAny( mxStyleProps ) );
        mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingRules") ) ) >>= mxNumberingRules;

        CreateListTemplate();

        mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingRules") ) , uno::makeAny( mxNumberingRules ) );
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
    rtl::OUString sCharStyleName( RTL_CONSTASCII_USTRINGPARAM("Bullet Symbols") );
    setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_CHAR_STYLE_NAME )), uno::makeAny( sCharStyleName ) );
    sal_Int16 nNumberingType = style::NumberingType::CHAR_SPECIAL;
    setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );

    rtl::OUString aBulletChar;
    switch( mnTemplateType )
    {
        case 1:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_CLOSED_DOT ) );
            break;
        }
        case 2:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_EMPTY_DOT ) );
            break;
        }
        case 3:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_SQUARE ) );
            break;
        }
        case 4:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_STAR_SYMBOL ) );
            break;
        }
        case 5:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_FOUR_DIAMONDS ) );
            break;
        }
        case 6:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_ARROW ) );
            break;
        }
        case 7:
        {
            aBulletChar = rtl::OUString( sal_Unicode( CHAR_CHECK_MARK ) );
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_BULLET_CHAR )), uno::makeAny( aBulletChar ) );

    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
}

void SwVbaListHelper::CreateNumberListTemplate() throw( css::uno::RuntimeException )
{
    // there is only 1 level for each bullet list in MSWord
    sal_Int32 nLevel = 0;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;

    sal_Int16 nNumberingType = 0;
    rtl::OUString sSuffix;
    switch( mnTemplateType )
    {
        case 1:
        {
            nNumberingType = style::NumberingType::ARABIC;
            sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
            break;
        }
        case 2:
        {
            nNumberingType = style::NumberingType::ARABIC;
            sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
            break;
        }
        case 3:
        {
            nNumberingType = style::NumberingType::ROMAN_UPPER;
            sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
            break;
        }
        case 4:
        {
            nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
            sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
            break;
        }
        case 5:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
            break;
        }
        case 6:
        {
            nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
            sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
            break;
        }
        case 7:
        {
            nNumberingType = style::NumberingType::ROMAN_LOWER;
            sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
            break;
        }
        default:
        {
            // we only support 7 types template now
            throw css::uno::RuntimeException();
        }
    }
    setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
    setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_SUFFIX )), uno::makeAny( sSuffix ) );

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
    rtl::OUString sPrefix;
    rtl::OUString sSuffix;
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
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PREFIX )), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_SUFFIX )), uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType2() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::ARABIC;
    sal_Int16 nParentNumbering = 0;
    rtl::OUString sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_SUFFIX )), uno::makeAny( sSuffix ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PARENT_NUMBERING )), uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType3() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::CHAR_SPECIAL;
    rtl::OUString sCharStyleName( RTL_CONSTASCII_USTRINGPARAM("Bullet Symbols") );
    rtl::OUString aBulletChar;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_CHAR_STYLE_NAME )), uno::makeAny( sCharStyleName ) );
        switch( nLevel )
        {
            case 0:
            {
                aBulletChar = rtl::OUString( sal_Unicode( CHAR_FOUR_DIAMONDS ) );
                break;
            }
            case 1:
            case 5:
            {
                aBulletChar = rtl::OUString( sal_Unicode( CHAR_ARROW ) );
                break;
            }
            case 2:
            case 6:
            {
                aBulletChar = rtl::OUString( sal_Unicode( CHAR_SQUARE ) );
                break;
            }
            case 3:
            case 7:
            {
                aBulletChar = rtl::OUString( sal_Unicode( CHAR_CLOSED_DOT ) );
                break;
            }
            case 4:
            case 8:
            {
                aBulletChar = rtl::OUString( sal_Unicode( CHAR_DIAMOND ) );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_BULLET_CHAR )), uno::makeAny( aBulletChar ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType4() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = 0;
    rtl::OUString sPrefix;
    rtl::OUString sSuffix;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        switch( nLevel )
        {
            case 0:
            {
                nNumberingType = style::NumberingType::ROMAN_UPPER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                sal_Int16 nParentNumbering = 0;
                setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PARENT_NUMBERING )), uno::makeAny( nParentNumbering ) );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PREFIX )), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_SUFFIX )), uno::makeAny( sSuffix ) );
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
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        if( nLevel != 0 )
        {
            nParentNumbering = sal_Int16( nLevel - 1 );
            setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PARENT_NUMBERING )), uno::makeAny( nParentNumbering ) );
        }
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType6() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = 0;
    rtl::OUString sPrefix;
    rtl::OUString sSuffix;
    uno::Sequence< beans::PropertyValue > aPropertyValues;

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        switch( nLevel )
        {
            case 0:
            {
                nNumberingType = style::NumberingType::ROMAN_UPPER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 1:
            {
                nNumberingType = style::NumberingType::CHARS_UPPER_LETTER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 2:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 3:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString();
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 4:
            {
                nNumberingType = style::NumberingType::ARABIC;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 5:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 6:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode(')') );
                break;
            }
            case 7:
            {
                nNumberingType = style::NumberingType::CHARS_LOWER_LETTER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            case 8:
            {
                nNumberingType = style::NumberingType::ROMAN_LOWER;
                sPrefix = rtl::OUString::valueOf( sal_Unicode('(') );
                sSuffix = rtl::OUString::valueOf( sal_Unicode('.') );
                break;
            }
            default:
            {
                throw uno::RuntimeException();
            }
        }
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PREFIX )), uno::makeAny( sPrefix ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_SUFFIX )), uno::makeAny( sSuffix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

void SwVbaListHelper::CreateOutlineNumberForType7() throw( css::uno::RuntimeException )
{
    sal_Int16 nNumberingType = style::NumberingType::ARABIC;
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    rtl::OUString sPrefix(RTL_CONSTASCII_USTRINGPARAM("Chapter "));

    for( sal_Int32 nLevel = 0; nLevel < LIST_LEVEL_COUNT; nLevel++ )
    {
        mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_NUMBERING_TYPE )), uno::makeAny( nNumberingType ) );
        setOrAppendPropertyValue( aPropertyValues, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_PREFIX )), uno::makeAny( sPrefix ) );
        mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    }
}

uno::Any SwVbaListHelper::getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const rtl::OUString& sName ) throw( css::uno::RuntimeException )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    return getPropertyValue( aPropertyValues, sName );
}

void SwVbaListHelper::setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const rtl::OUString& sName, const css::uno::Any& aValue ) throw( css::uno::RuntimeException )
{
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    mxNumberingRules->getByIndex( nLevel ) >>= aPropertyValues;
    setOrAppendPropertyValue( aPropertyValues, sName, aValue );
    mxNumberingRules->replaceByIndex( nLevel, uno::makeAny( aPropertyValues ) );
    mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingRules") ) , uno::makeAny( mxNumberingRules ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
