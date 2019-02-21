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

#include <memory>
#include "text.hxx"

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/processfactory.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>
#include <filter/msfilter/util.hxx>
#include <i18nutil/scripttypedetector.hxx>
#include <o3tl/any.hxx>
#include <svl/languageoptions.hxx>
#include <osl/diagnose.h>
#include <i18nlangtag/languagetag.hxx>

#include <vcl/settings.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

using namespace css;

static css::uno::Reference< css::i18n::XBreakIterator > xPPTBreakIter;

PortionObj::PortionObj(const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                FontCollection& rFontCollection)
    : meCharColor(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meCharHeight(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meFontName(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meAsianOrComplexFont(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meCharEscapement(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , mnCharAttrHard(0)
    , mnCharAttr(0)
    , mnFont(0)
    , mnAsianOrComplexFont(0xffff)
    , mnTextSize(0)
    , mbLastPortion(true)
{
    mXPropSet = rXPropSet;

    ImplGetPortionValues( rFontCollection, false );
}

PortionObj::PortionObj(css::uno::Reference< css::text::XTextRange > & rXTextRange,
                           bool bLast, FontCollection& rFontCollection)
    : meCharColor(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meCharHeight(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meFontName(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meAsianOrComplexFont(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meCharEscapement(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , mnCharAttrHard(0)
    , mnCharColor(0)
    , mnCharAttr(0)
    , mnCharHeight(0)
    , mnFont(0)
    , mnAsianOrComplexFont(0xffff)
    , mnCharEscapement(0)
    , mbLastPortion(bLast)
{
    OUString aString( rXTextRange->getString() );
    OUString aURL;

    mnTextSize = aString.getLength();
    if ( bLast )
        mnTextSize++;

    if ( !mnTextSize )
        return;

    bool bRTL_endingParen = false;
    mpFieldEntry = nullptr;
    sal_uInt32 nFieldType = 0;

    mXPropSet.set( rXTextRange, css::uno::UNO_QUERY );
    mXPropState.set( rXTextRange, css::uno::UNO_QUERY );

    bool bPropSetsValid = ( mXPropSet.is() && mXPropState.is() );
    if ( bPropSetsValid )
        nFieldType = ImplGetTextField( rXTextRange, mXPropSet, aURL );
    if ( nFieldType )
    {
        mpFieldEntry.reset( new FieldEntry( nFieldType, 0, mnTextSize ) );
        if ( nFieldType >> 28 == 4 )
        {
            mpFieldEntry->aRepresentation = aString;
            mpFieldEntry->aFieldUrl = aURL;
        }
    }
    bool bSymbol = false;

    if ( bPropSetsValid && ImplGetPropertyValue( "CharFontCharSet", false ) )
    {
        sal_Int16 nCharset = 0;
        mAny >>= nCharset;
        if ( nCharset == css::awt::CharSet::SYMBOL )
            bSymbol = true;
    }
    if ( mpFieldEntry && ( nFieldType & 0x800000 ) )    // placeholder ?
    {
        mnTextSize = 1;
        if ( bLast )
            mnTextSize++;
        mpText.reset( new sal_uInt16[ mnTextSize ] );
        mpText[ 0 ] = 0x2a;
    }
    else
    {
        // For i39516 - a closing parenthesis that ends an RTL string is displayed backwards by PPT
        // Solution: add a Unicode Right-to-Left Mark, following the method described in i18024
        if (bLast && !aString.isEmpty()
            && aString[aString.getLength() - 1] == ')'
            && FontCollection::GetScriptDirection(aString) == css::i18n::ScriptDirection::RIGHT_TO_LEFT)
        {
            mnTextSize++;
            bRTL_endingParen = true;
        }
        mpText.reset( new sal_uInt16[ mnTextSize ] );
        sal_uInt16 nChar;
        for ( sal_Int32 i = 0; i < aString.getLength(); i++ )
        {
            nChar = static_cast<sal_uInt16>(aString[ i ]);
            if ( nChar == 0xa )
                nChar++;
            else if ( !bSymbol )
            {
                switch ( nChar )
                {
                    // Currency
                    case 128:   nChar = 0x20AC; break;
                    // Punctuation and other
                    case 130:   nChar = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                    case 131:   nChar = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                    case 132:   nChar = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                          // LOW DOUBLE PRIME QUOTATION MARK
                    case 133:   nChar = 0x2026; break;// HORIZONTAL ELLIPSES
                    case 134:   nChar = 0x2020; break;// DAGGER
                    case 135:   nChar = 0x2021; break;// DOUBLE DAGGER
                    case 136:   nChar = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                    case 137:   nChar = 0x2030; break;// PER MILLE SIGN
                    case 138:   nChar = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                    case 139:   nChar = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                    case 140:   nChar = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                    case 142:   nChar = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                    case 145:   nChar = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                          // MODIFIER LETTER TURNED COMMA
                    case 146:   nChar = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                          // MODIFIER LETTER APOSTROPHE
                    case 147:   nChar = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                    case 148:   nChar = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                    case 149:   nChar = 0x2022; break;// BULLET
                    case 150:   nChar = 0x2013; break;// EN DASH
                    case 151:   nChar = 0x2014; break;// EM DASH
                    case 152:   nChar = 0x02DC; break;// SMALL TILDE
                    case 153:   nChar = 0x2122; break;// TRADE MARK SIGN
                    case 154:   nChar = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                    case 155:   nChar = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                    case 156:   nChar = 0x0153; break;// LATIN SMALL LIGATURE OE
                    case 158:   nChar = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                    case 159:   nChar = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
                }
            }
            mpText[ i ] = nChar;
        }
    }
    if ( bRTL_endingParen )
        mpText[ mnTextSize - 2 ] = 0x200F; // Unicode Right-to-Left mark

    if ( bLast )
        mpText[ mnTextSize - 1 ] = 0xd;

    if ( bPropSetsValid )
        ImplGetPortionValues( rFontCollection, true );
}

PortionObj::PortionObj( const PortionObj& rPortionObj )
: PropStateValue( rPortionObj )
{
    ImplConstruct( rPortionObj );
}

PortionObj::~PortionObj()
{
    ImplClear();
}

void PortionObj::Write( SvStream* pStrm, bool bLast )
{
    sal_uInt32 nCount = mnTextSize;
    if ( bLast && mbLastPortion )
        nCount--;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        pStrm->WriteUInt16( mpText[ i ] );
}

void PortionObj::ImplGetPortionValues( FontCollection& rFontCollection, bool bGetPropStateValue )
{

    bool bOk = ImplGetPropertyValue( "CharFontName", bGetPropStateValue );
    meFontName = ePropState;
    if ( bOk )
    {
        FontCollectionEntry aFontDesc( *o3tl::doAccess<OUString>(mAny) );
        sal_uInt32  nCount = rFontCollection.GetCount();
        mnFont = static_cast<sal_uInt16>(rFontCollection.GetId( aFontDesc ));
        if ( mnFont == nCount )
        {
            FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
            if ( ImplGetPropertyValue( "CharFontCharSet", false ) )
                mAny >>= rFontDesc.CharSet;
            if ( ImplGetPropertyValue( "CharFontFamily", false ) )
                mAny >>= rFontDesc.Family;
            if ( ImplGetPropertyValue( "CharFontPitch", false ) )
                mAny >>= rFontDesc.Pitch;
        }
    }

    sal_Int16 nScriptType = SvtLanguageOptions::FromSvtScriptTypeToI18N( SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() ) );
    if ( mpText && mnTextSize && xPPTBreakIter.is() )
    {
        OUString sT( reinterpret_cast<sal_Unicode *>(mpText.get()), mnTextSize );
        nScriptType = xPPTBreakIter->getScriptType( sT, 0 );
    }
    if ( nScriptType != css::i18n::ScriptType::COMPLEX )
    {
        bOk = ImplGetPropertyValue( "CharFontNameAsian", bGetPropStateValue );
        meAsianOrComplexFont = ePropState;
        if ( bOk )
        {
            FontCollectionEntry aFontDesc( *o3tl::doAccess<OUString>(mAny) );
            sal_uInt32  nCount = rFontCollection.GetCount();
            mnAsianOrComplexFont = static_cast<sal_uInt16>(rFontCollection.GetId( aFontDesc ));
            if ( mnAsianOrComplexFont == nCount )
            {
                FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
                if ( ImplGetPropertyValue( "CharFontCharSetAsian", false ) )
                    mAny >>= rFontDesc.CharSet;
                if ( ImplGetPropertyValue( "CharFontFamilyAsian", false ) )
                    mAny >>= rFontDesc.Family;
                if ( ImplGetPropertyValue( "CharFontPitchAsian", false ) )
                    mAny >>= rFontDesc.Pitch;
            }
        }
    }
    else
    {
        bOk = ImplGetPropertyValue( "CharFontNameComplex", bGetPropStateValue );
        meAsianOrComplexFont = ePropState;
        if ( bOk )
        {
            FontCollectionEntry aFontDesc( *o3tl::doAccess<OUString>(mAny) );
            sal_uInt32  nCount = rFontCollection.GetCount();
            mnAsianOrComplexFont = static_cast<sal_uInt16>(rFontCollection.GetId( aFontDesc ));
            if ( mnAsianOrComplexFont == nCount )
            {
                FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
                if ( ImplGetPropertyValue( "CharFontCharSetComplex", false ) )
                    mAny >>= rFontDesc.CharSet;
                if ( ImplGetPropertyValue( "CharFontFamilyComplex", false ) )
                    mAny >>= rFontDesc.Family;
                if ( ImplGetPropertyValue( "CharFontPitchComplex", false ) )
                    mAny >>= rFontDesc.Pitch;
            }
        }
    }

    OUString aCharHeightName, aCharWeightName, aCharLocaleName, aCharPostureName;
    switch( nScriptType )
    {
        case css::i18n::ScriptType::ASIAN :
        {
            aCharHeightName  = "CharHeightAsian";
            aCharWeightName  = "CharWeightAsian";
            aCharLocaleName  = "CharLocaleAsian";
            aCharPostureName = "CharPostureAsian";
            break;
        }
        case css::i18n::ScriptType::COMPLEX :
        {
            aCharHeightName  = "CharHeightComplex";
            aCharWeightName  = "CharWeightComplex";
            aCharLocaleName  = "CharLocaleComplex";
            aCharPostureName = "CharPostureComplex";
            break;
        }
        default:
        {
            aCharHeightName  = "CharHeight";
            aCharWeightName  = "CharWeight";
            aCharLocaleName  = "CharLocale";
            aCharPostureName = "CharPosture";
            break;
        }
    }

    mnCharHeight = 24;
    if ( GetPropertyValue( mAny, mXPropSet, aCharHeightName ) )
    {
        float fVal(0.0);
        if ( mAny >>= fVal )
        {
            mnCharHeight = static_cast<sal_uInt16>( fVal + 0.5 );
            meCharHeight = GetPropertyState( mXPropSet, aCharHeightName );
        }
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharWeightName ) )
    {
        float fFloat(0.0);
        if ( mAny >>= fFloat )
        {
            if ( fFloat >= css::awt::FontWeight::SEMIBOLD )
                mnCharAttr |= 1;
            if ( GetPropertyState( mXPropSet, aCharWeightName ) == css::beans::PropertyState_DIRECT_VALUE )
                mnCharAttrHard |= 1;
        }
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharLocaleName ) )
    {
        css::lang::Locale eLocale;
        if ( mAny >>= eLocale )
            meCharLocale = eLocale;
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharPostureName ) )
    {
        css::awt::FontSlant aFS;
        if ( mAny >>= aFS )
        {
            switch( aFS )
            {
                case css::awt::FontSlant_OBLIQUE :
                case css::awt::FontSlant_ITALIC :
                    mnCharAttr |= 2;
                    break;
                default:
                    break;
            }
            if ( GetPropertyState( mXPropSet, aCharPostureName ) == css::beans::PropertyState_DIRECT_VALUE )
                mnCharAttrHard |= 2;
        }
    }

    if ( ImplGetPropertyValue( "CharUnderline", bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        switch ( nVal )
        {
            case css::awt::FontUnderline::SINGLE :
            case css::awt::FontUnderline::DOUBLE :
            case css::awt::FontUnderline::DOTTED :
                mnCharAttr |= 4;
        }
    }
    if ( ePropState == css::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 4;

    if ( ImplGetPropertyValue( "CharShadowed", bGetPropStateValue ) )
    {
        bool bBool(false);
        mAny >>= bBool;
        if ( bBool )
            mnCharAttr |= 0x10;
    }
    if ( ePropState == css::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 16;

    if ( ImplGetPropertyValue( "CharRelief", bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        if ( nVal != css::text::FontRelief::NONE )
            mnCharAttr |= 512;
    }
    if ( ePropState == css::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 512;

    if ( ImplGetPropertyValue( "CharColor", bGetPropStateValue ) )
    {
        sal_uInt32 nSOColor = *( o3tl::doAccess<sal_uInt32>(mAny) );
        mnCharColor = nSOColor & 0xff00ff00;                            // green and hibyte
        mnCharColor |= static_cast<sal_uInt8>(nSOColor) << 16;                   // red and blue is switched
        mnCharColor |= static_cast<sal_uInt8>( nSOColor >> 16 );
    }
    meCharColor = ePropState;

    mnCharEscapement = 0;
    if ( ImplGetPropertyValue( "CharEscapement", bGetPropStateValue ) )
    {
        mAny >>= mnCharEscapement;
        if ( mnCharEscapement > 100 )
            mnCharEscapement = 33;
        else if ( mnCharEscapement < -100 )
            mnCharEscapement = -33;
    }
    meCharEscapement = ePropState;
}

void PortionObj::ImplClear()
{
    mpFieldEntry.reset();
    mpText.reset();
}

void PortionObj::ImplConstruct( const PortionObj& rPortionObj )
{
    meCharColor = rPortionObj.meCharColor;
    meCharHeight = rPortionObj.meCharHeight;
    meFontName = rPortionObj.meFontName;
    meAsianOrComplexFont = rPortionObj.meAsianOrComplexFont;
    meCharEscapement = rPortionObj.meCharEscapement;
    meCharLocale = rPortionObj.meCharLocale;
    mnCharAttrHard = rPortionObj.mnCharAttrHard;

    mbLastPortion = rPortionObj.mbLastPortion;
    mnTextSize = rPortionObj.mnTextSize;
    mnCharColor = rPortionObj.mnCharColor;
    mnCharEscapement = rPortionObj.mnCharEscapement;
    mnCharAttr = rPortionObj.mnCharAttr;
    mnCharHeight = rPortionObj.mnCharHeight;
    mnFont = rPortionObj.mnFont;
    mnAsianOrComplexFont = rPortionObj.mnAsianOrComplexFont;

    if ( rPortionObj.mpText )
    {
        mpText.reset( new sal_uInt16[ mnTextSize ] );
        memcpy( mpText.get(), rPortionObj.mpText.get(), mnTextSize << 1 );
    }

    if ( rPortionObj.mpFieldEntry )
        mpFieldEntry.reset( new FieldEntry( *( rPortionObj.mpFieldEntry ) ) );
}

sal_uInt32 PortionObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    if ( mpFieldEntry && ( !mpFieldEntry->nFieldStartPos ) )
    {
        mpFieldEntry->nFieldStartPos += nCurrentTextPosition;
        mpFieldEntry->nFieldEndPos += nCurrentTextPosition;
    }
    return mnTextSize;
}

// Return:                              0 = no TextField
//  bit28->31   text field type :
//                                      1 = Date
//                                      2 = Time
//                                      3 = SlideNumber
//                                      4 = Url
//                                      5 = DateTime
//                                      6 = header
//                                      7 = footer
//  bit24->27   text field sub type (optional)
//     23->     PPT Textfield needs a placeholder

sal_uInt32 PortionObj::ImplGetTextField( css::uno::Reference< css::text::XTextRange > & ,
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet, OUString& rURL )
{
    sal_uInt32 nRetValue = 0;
    sal_Int32 nFormat;
    css::uno::Any aAny;
    if ( GetPropertyValue( aAny, rXPropSet, "TextPortionType", true ) )
    {
        auto aTextFieldType = o3tl::doAccess<OUString>(aAny);
        if ( *aTextFieldType == "TextField" )
        {
            if ( GetPropertyValue( aAny, rXPropSet, *aTextFieldType, true ) )
            {
                css::uno::Reference< css::text::XTextField > aXTextField;
                if ( aAny >>= aXTextField )
                {
                    if ( aXTextField.is() )
                    {
                        css::uno::Reference< css::beans::XPropertySet > xFieldPropSet( aXTextField, css::uno::UNO_QUERY );
                        if ( xFieldPropSet.is() )
                        {
                            OUString aFieldKind( aXTextField->getPresentation( true ) );
                            if ( aFieldKind == "Date" )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, "IsFix", true ) )
                                {
                                    bool bBool = false;
                                    aAny >>= bBool;
                                    if ( !bBool )  // Fixed DateFields does not exist in PPT
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, "Format", true ) )
                                        {
                                            nFormat = *o3tl::doAccess<sal_Int32>(aAny);
                                            switch ( nFormat )
                                            {
                                                default:
                                                case 5 :
                                                case 4 :
                                                case 2 : nFormat = 0; break;
                                                case 8 :
                                                case 9 :
                                                case 3 : nFormat = 1; break;
                                                case 7 :
                                                case 6 : nFormat = 2; break;
                                            }
                                            nRetValue |= ( ( ( 1 << 4 ) | nFormat ) << 24 ) | 0x800000;
                                        }
                                    }
                                }
                            }
                            else if ( aFieldKind == "URL" )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, "URL", true ) )
                                    rURL = *o3tl::doAccess<OUString>(aAny);
                                nRetValue = 4 << 28;
                            }
                            else if ( aFieldKind == "Page" )
                            {
                                nRetValue = 3 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == "Pages" )
                            {

                            }
                            else if ( aFieldKind == "Time" )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, "IsFix", true ) )
                                {
                                    bool bBool = false;
                                    aAny >>= bBool;
                                    if ( !bBool )
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, "IsFix", true ) )
                                        {
                                            nFormat = *o3tl::doAccess<sal_Int32>(aAny);
                                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                                        }
                                    }
                                }
                            }
                            else if ( aFieldKind == "File" )
                            {

                            }
                            else if ( aFieldKind == "Table" )
                            {

                            }
                            else if ( aFieldKind == "ExtTime" )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, "IsFix", true ) )
                                {
                                    bool bBool = false;
                                    aAny >>= bBool;
                                    if ( !bBool )
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, "Format", true ) )
                                        {
                                            nFormat = *o3tl::doAccess<sal_Int32>(aAny);
                                            switch ( nFormat )
                                            {
                                                default:
                                                case 6 :
                                                case 7 :
                                                case 8 :
                                                case 2 : nFormat = 12; break;
                                                case 3 : nFormat = 9; break;
                                                case 5 :
                                                case 4 : nFormat = 10; break;

                                            }
                                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                                        }
                                    }
                                }
                            }
                            else if ( aFieldKind == "ExtFile" )
                            {

                            }
                            else if ( aFieldKind ==  "Author" )
                            {

                            }
                            else if ( aFieldKind == "DateTime" )
                            {
                                nRetValue = 5 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == "Header" )
                            {
                                nRetValue = 6 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == "Footer" )
                            {
                                nRetValue = 7 << 28 | 0x800000;
                            }
                        }
                    }
                }
            }
        }
    }
    return nRetValue;
}

PortionObj& PortionObj::operator=( const PortionObj& rPortionObj )
{
    if ( this != &rPortionObj )
    {
        ImplClear();
        ImplConstruct( rPortionObj );
    }
    return *this;
}

ParagraphObj::ParagraphObj(const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    PPTExBulletProvider* pProv)
    : PropStateValue()
    , SOParagraph()
    , mvPortions()
    , mnTextSize(0)
    , mbFirstParagraph(false)
    , mbLastParagraph(false)
    , mnTextAdjust(0)
    , mnLineSpacing(0)
    , mbFixedLineSpacing(false)
    , mnLineSpacingTop(0)
    , mnLineSpacingBottom(0)
    , mbForbiddenRules(false)
    , mbParagraphPunctation(false)
    , mnBiDi(0)
{
    mXPropSet = rXPropSet;

    bExtendedParameters = false;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    ImplGetParagraphValues( pProv, false );
}

ParagraphObj::ParagraphObj(css::uno::Reference< css::text::XTextContent > const & rXTextContent,
    ParaFlags aParaFlags, FontCollection& rFontCollection, PPTExBulletProvider& rProv )
    : PropStateValue()
    , SOParagraph()
    , mvPortions()
    , mnTextSize(0)
    , mbIsBullet(false)
    , mbFirstParagraph( aParaFlags.bFirstParagraph )
    , mbLastParagraph( aParaFlags.bLastParagraph )
    , meBullet(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meTextAdjust(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meLineSpacing(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meLineSpacingTop(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meLineSpacingBottom(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meForbiddenRules(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meParagraphPunctation(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , meBiDi(css::beans::PropertyState_AMBIGUOUS_VALUE)
    , mnTextAdjust(0)
    , mnLineSpacing(0)
    , mbFixedLineSpacing(false)
    , mnLineSpacingTop(0)
    , mnLineSpacingBottom(0)
    , mbForbiddenRules(false)
    , mbParagraphPunctation(false)
    , mnBiDi(0)
{
    bExtendedParameters = false;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    mXPropSet.set( rXTextContent, css::uno::UNO_QUERY );

    mXPropState.set( rXTextContent, css::uno::UNO_QUERY );

    if ( !(mXPropSet.is() && mXPropState.is()) )
        return;

    css::uno::Reference< css::container::XEnumerationAccess > aXTextPortionEA( rXTextContent, css::uno::UNO_QUERY );
    if ( aXTextPortionEA.is() )
    {
        css::uno::Reference< css::container::XEnumeration > aXTextPortionE( aXTextPortionEA->createEnumeration() );
        if ( aXTextPortionE.is() )
        {
            while ( aXTextPortionE->hasMoreElements() )
            {
                css::uno::Reference< css::text::XTextRange > aXCursorText;
                css::uno::Any aAny( aXTextPortionE->nextElement() );
                if ( aAny >>= aXCursorText )
                {
                    std::unique_ptr<PortionObj> pPortionObj(new PortionObj( aXCursorText, !aXTextPortionE->hasMoreElements(), rFontCollection ));
                    if ( pPortionObj->Count() )
                        mvPortions.push_back( std::move(pPortionObj) );
                }
            }
        }
    }
    ImplGetParagraphValues( &rProv, true );
}

ParagraphObj::~ParagraphObj()
{
    ImplClear();
}

void ParagraphObj::Write( SvStream* pStrm )
{
    for ( std::vector<std::unique_ptr<PortionObj> >::iterator it = mvPortions.begin(); it != mvPortions.end(); ++it )
        (*it)->Write( pStrm, mbLastParagraph );
}

void ParagraphObj::ImplClear()
{
    mvPortions.clear();
}

void ParagraphObj::CalculateGraphicBulletSize( sal_uInt16 nFontHeight )
{
    if ( !(( nNumberingType == SVX_NUM_BITMAP ) && ( nBulletId != 0xffff )) )
        return;

    // calculate the bullet real size for this graphic
    if ( aBuGraSize.Width() && aBuGraSize.Height() )
    {
        double fCharHeight = nFontHeight;
        double fLen = aBuGraSize.Height();
        fCharHeight = fCharHeight * 0.2540;
        double fQuo = fLen / fCharHeight;
        nBulletRealSize = static_cast<sal_Int16>( fQuo + 0.5 );
        if ( static_cast<sal_uInt16>(nBulletRealSize) > 400 )
            nBulletRealSize = 400;
    }
}

void ParagraphObj::ImplGetNumberingLevel( PPTExBulletProvider* pBuProv, sal_Int16 nNumberingDepth, bool bIsBullet, bool bGetPropStateValue )
{
    css::uno::Any aAny;
    if ( GetPropertyValue( aAny, mXPropSet, "ParaLeftMargin" ) )
    {
        sal_Int32 nVal(0);
        if ( aAny >>= nVal )
            nTextOfs = static_cast< sal_Int16 >( nVal / ( 2540.0 / 576 ) + 0.5 ) ;
    }
    if ( GetPropertyValue( aAny, mXPropSet, "ParaFirstLineIndent" ) )
    {
        if ( aAny >>= nBulletOfs )
            nBulletOfs = static_cast< sal_Int32 >( nBulletOfs / ( 2540.0 / 576 ) + 0.5 );
    }
    if ( GetPropertyValue( aAny, mXPropSet, "NumberingIsNumber" ) )
        aAny >>= bNumberingIsNumber;

    css::uno::Reference< css::container::XIndexReplace > aXIndexReplace;

    if ( bIsBullet && ImplGetPropertyValue( "NumberingRules", bGetPropStateValue ) )
    {
        if ( ( mAny >>= aXIndexReplace ) && nNumberingDepth < aXIndexReplace->getCount() )
        {
            mAny = aXIndexReplace->getByIndex( nNumberingDepth );
            auto aPropertySequence = o3tl::doAccess<css::uno::Sequence<css::beans::PropertyValue>>(mAny);

            const css::beans::PropertyValue* pPropValue = aPropertySequence->getConstArray();

            sal_Int32 nPropertyCount = aPropertySequence->getLength();
            if ( nPropertyCount )
            {
                bExtendedParameters = true;
                nBulletRealSize = 100;
                nMappedNumType = 0;

                uno::Reference<graphic::XGraphic> xGraphic;
                for ( sal_Int32 i = 0; i < nPropertyCount; i++ )
                {
                    OUString aPropName( pPropValue[ i ].Name );
                    if ( aPropName == "NumberingType" )
                        nNumberingType = static_cast<SvxNumType>(*o3tl::doAccess<sal_Int16>(pPropValue[i].Value));
                    else if ( aPropName == "Adjust" )
                        nHorzAdjust = *o3tl::doAccess<sal_Int16>(pPropValue[i].Value);
                    else if ( aPropName == "BulletChar" )
                    {
                        OUString aString( *o3tl::doAccess<OUString>(pPropValue[i].Value) );
                        if ( !aString.isEmpty() )
                            cBulletId = aString[ 0 ];
                    }
                    else if ( aPropName == "BulletFont" )
                    {
                        aFontDesc = *o3tl::doAccess<css::awt::FontDescriptor>(pPropValue[i].Value);

                        // Our numbullet dialog has set the wrong textencoding for our "StarSymbol" font,
                        // instead of a Unicode encoding the encoding RTL_TEXTENCODING_SYMBOL was used.
                        // Because there might exist a lot of damaged documemts I added this two lines
                        // which fixes the bullet problem for the export.
                        if ( aFontDesc.Name.equalsIgnoreAsciiCase("StarSymbol") )
                            aFontDesc.CharSet = RTL_TEXTENCODING_MS_1252;

                    }
                    else if ( aPropName == "GraphicBitmap" )
                    {
                        auto xBitmap = pPropValue[i].Value.get<uno::Reference<awt::XBitmap>>();
                        xGraphic.set(xBitmap, uno::UNO_QUERY);
                    }
                    else if ( aPropName == "GraphicSize" )
                    {
                        if (auto aSize = o3tl::tryAccess<css::awt::Size>(pPropValue[i].Value))
                        {
                            // don't cast awt::Size to Size as on 64-bits they are not the same.
                            aBuGraSize.setWidth( aSize->Width );
                            aBuGraSize.setHeight( aSize->Height );
                        }
                    }
                    else if ( aPropName == "StartWith" )
                        nStartWith = *o3tl::doAccess<sal_Int16>(pPropValue[i].Value);
                    else if ( aPropName == "LeftMargin" )
                        nTextOfs = nTextOfs + static_cast< sal_Int16 >( *o3tl::doAccess<sal_Int32>(pPropValue[i].Value) / ( 2540.0 / 576 ) );
                    else if ( aPropName == "FirstLineOffset" )
                        nBulletOfs += static_cast<sal_Int16>( *o3tl::doAccess<sal_Int32>(pPropValue[i].Value) / ( 2540.0 / 576 ) );
                    else if ( aPropName == "BulletColor" )
                    {
                        sal_uInt32 nSOColor = *o3tl::doAccess<sal_uInt32>(pPropValue[i].Value);
                        nBulletColor = nSOColor & 0xff00ff00;                       // green and hibyte
                        nBulletColor |= static_cast<sal_uInt8>(nSOColor) << 16;              // red
                        nBulletColor |= static_cast<sal_uInt8>( nSOColor >> 16 ) | 0xfe000000; // blue
                    }
                    else if ( aPropName == "BulletRelSize" )
                    {
                        nBulletRealSize = *o3tl::doAccess<sal_Int16>(pPropValue[i].Value);
                        nParaFlags |= 0x40;
                        nBulletFlags |= 8;
                    }
                    else if ( aPropName == "Prefix" )
                        sPrefix = *o3tl::doAccess<OUString>(pPropValue[i].Value);
                    else if ( aPropName == "Suffix" )
                        sSuffix = *o3tl::doAccess<OUString>(pPropValue[i].Value);
#ifdef DBG_UTIL
                    else if ( ! (
                            ( aPropName == "SymbolTextDistance" )
                        ||  ( aPropName == "GraphicBitmap" ) ) )
                    {
                        OSL_FAIL( "Unknown Property" );
                    }
#endif
                }

                if (xGraphic.is())
                {
                    if ( aBuGraSize.Width() && aBuGraSize.Height() )
                    {
                        Graphic aGraphic(xGraphic);
                        nBulletId = pBuProv->GetId(xGraphic, aBuGraSize );
                        if ( nBulletId != 0xffff )
                            bExtendedBulletsUsed = true;
                    }
                    else
                    {
                        nNumberingType = SVX_NUM_NUMBER_NONE;
                    }
                }

                CalculateGraphicBulletSize( ( mvPortions.empty() ) ? 24 : mvPortions.front()->mnCharHeight );

                switch( nNumberingType )
                {
                    case SVX_NUM_NUMBER_NONE : nParaFlags |= 0xf; break;

                    case SVX_NUM_CHAR_SPECIAL :                           // Bullet
                    {
                        if ( IsStarSymbol(aFontDesc.Name) )
                        {
                            rtl_TextEncoding eChrSet = aFontDesc.CharSet;
                            cBulletId = msfilter::util::bestFitOpenSymbolToMSFont(cBulletId, eChrSet, aFontDesc.Name);
                            aFontDesc.CharSet = eChrSet;
                        }

                        if ( !aFontDesc.Name.isEmpty() )
                        {
                            nParaFlags |= 0x90; // we define the font and charset
                        }

                        [[fallthrough]];
                    }
                    case SVX_NUM_CHARS_UPPER_LETTER :       // count from a-z, aa - az, ba - bz, ...
                    case SVX_NUM_CHARS_LOWER_LETTER :
                    case SVX_NUM_ROMAN_UPPER :
                    case SVX_NUM_ROMAN_LOWER :
                    case SVX_NUM_ARABIC :
                    case SVX_NUM_PAGEDESC :                 // numbering from the page template
                    case SVX_NUM_BITMAP :
                    case SVX_NUM_CHARS_UPPER_LETTER_N :     // count from a-z, aa-zz, aaa-zzz
                    case SVX_NUM_CHARS_LOWER_LETTER_N :
                    case SVX_NUM_NUMBER_UPPER_ZH:
                    case SVX_NUM_CIRCLE_NUMBER:
                    case SVX_NUM_NUMBER_UPPER_ZH_TW:
                    case SVX_NUM_NUMBER_LOWER_ZH:
                    case SVX_NUM_FULL_WIDTH_ARABIC:
                    {
                        if ( nNumberingType != SVX_NUM_CHAR_SPECIAL )
                        {
                            bExtendedBulletsUsed = true;
                            if ( nNumberingDepth & 1 )
                                cBulletId = 0x2013;         // defaulting bullet characters for ppt97
                            else if ( nNumberingDepth == 4 )
                                cBulletId = 0xbb;
                            else
                                cBulletId = 0x2022;

                            switch( nNumberingType )
                            {
                                case SVX_NUM_CHARS_UPPER_LETTER :
                                case SVX_NUM_CHARS_UPPER_LETTER_N :
                                {
                                    if ( sSuffix == ")" )
                                    {
                                        if ( sPrefix == "(" )
                                            nMappedNumType = 0xa0001;   // (A)
                                        else
                                            nMappedNumType = 0xb0001;   // A)
                                    }
                                    else
                                        nMappedNumType = 0x10001;       // A.
                                }
                                break;
                                case SVX_NUM_CHARS_LOWER_LETTER :
                                case SVX_NUM_CHARS_LOWER_LETTER_N :
                                {
                                    if ( sSuffix == ")" )
                                    {
                                        if ( sPrefix == "(" )
                                            nMappedNumType = 0x80001;   // (a)
                                        else
                                            nMappedNumType = 0x90001;   // a)
                                    }
                                    else
                                        nMappedNumType = 0x00001;       // a.
                                }
                                break;
                                case SVX_NUM_ROMAN_UPPER :
                                {
                                    if ( sSuffix == ")" )
                                    {
                                        if ( sPrefix == "(" )
                                            nMappedNumType = 0xe0001;   // (I)
                                        else
                                            nMappedNumType = 0xf0001;   // I)
                                    }
                                    else
                                        nMappedNumType = 0x70001;       // I.
                                }
                                break;
                                case SVX_NUM_ROMAN_LOWER :
                                {
                                    if ( sSuffix == ")" )
                                    {
                                        if ( sPrefix == "(" )
                                            nMappedNumType = 0x40001;   // (i)
                                        else
                                            nMappedNumType = 0x50001;   // i)
                                    }
                                    else
                                        nMappedNumType = 0x60001;       // i.
                                }
                                break;
                                case SVX_NUM_ARABIC :
                                {
                                    if ( sSuffix == ")" )
                                    {
                                        if ( sPrefix == "(" )
                                            nMappedNumType = 0xc0001;   // (1)
                                        else
                                            nMappedNumType = 0x20001;   // 1)
                                    }
                                    else
                                    {
                                        if ( sSuffix.isEmpty() && sPrefix.isEmpty() )
                                            nMappedNumType = 0xd0001;   // 1
                                        else
                                            nMappedNumType = 0x30001;   // 1.
                                    }
                                }
                                break;
                                case SVX_NUM_NUMBER_UPPER_ZH :
                                {
                                    if ( !sSuffix.isEmpty() )
                                        nMappedNumType = 0x110001;   // Simplified Chinese with single-byte period.
                                    else
                                        nMappedNumType = 0x100001;   // Simplified Chinese.
                                }
                                break;
                                case SVX_NUM_CIRCLE_NUMBER :
                                {
                                    nMappedNumType = 0x120001;   // Double byte circle numbers.
                                }
                                break;
                                case SVX_NUM_NUMBER_UPPER_ZH_TW :
                                {
                                    if ( !sSuffix.isEmpty() )
                                        nMappedNumType = 0x160001;   // Traditional Chinese with single-byte period.
                                    else
                                        nMappedNumType = 0x150001;   // Traditional Chinese.
                                }
                                break;
                                case SVX_NUM_NUMBER_LOWER_ZH :
                                {
                                    if ( sSuffix == OUStringLiteral1(0xff0e) )
                                        nMappedNumType = 0x260001;   // Japanese with double-byte period.
                                    else if ( !sSuffix.isEmpty() )
                                        nMappedNumType = 0x1B0001;   // Japanese/Korean with single-byte period.
                                    else
                                        nMappedNumType = 0x1A0001;   // Japanese/Korean.
                                }
                                break;
                                case SVX_NUM_FULL_WIDTH_ARABIC :
                                {
                                    if ( !sSuffix.isEmpty() )
                                        nMappedNumType = 0x1D0001;   // Double-byte Arabic numbers with double-byte period.
                                    else
                                        nMappedNumType = 0x1C0001;   // Double-byte Arabic numbers.
                                }
                                break;
                                default:
                                    break;
                            }
                        }
                        nParaFlags |= 0x2f;
                        nBulletFlags |= 6;
                        if ( mbIsBullet && bNumberingIsNumber )
                            nBulletFlags |= 1;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    nBulletOfs = nTextOfs + nBulletOfs;
    if ( nBulletOfs < 0 )
        nBulletOfs = 0;
}

void ParagraphObj::ImplGetParagraphValues( PPTExBulletProvider* pBuProv, bool bGetPropStateValue )
{
    css::uno::Any aAny;
    if ( GetPropertyValue( aAny, mXPropSet, "NumberingLevel", true ) )
    {
        if ( bGetPropStateValue )
            meBullet = GetPropertyState( mXPropSet, "NumberingLevel" );
        nDepth = *o3tl::doAccess<sal_Int16>(aAny);

        if ( nDepth < 0 )
        {
            mbIsBullet = false;
            nDepth = 0;
        }
        else
        {
            if ( nDepth > 4 )
                nDepth = 4;
            mbIsBullet = true;
        }
    }
    else
    {
        nDepth = 0;
        mbIsBullet = false;
    }
    ImplGetNumberingLevel( pBuProv, nDepth, mbIsBullet, bGetPropStateValue );

    if ( ImplGetPropertyValue( "ParaTabStops", bGetPropStateValue ) )
        maTabStop = *o3tl::doAccess<css::uno::Sequence<css::style::TabStop>>(mAny);
    sal_Int16 eTextAdjust = sal_Int16(css::style::ParagraphAdjust_LEFT);
    if ( GetPropertyValue( aAny, mXPropSet, "ParaAdjust", bGetPropStateValue ) )
        aAny >>= eTextAdjust;
    switch ( static_cast<css::style::ParagraphAdjust>(eTextAdjust) )
    {
        case css::style::ParagraphAdjust_CENTER :
            mnTextAdjust = 1;
        break;
        case css::style::ParagraphAdjust_RIGHT :
            mnTextAdjust = 2;
        break;
        case css::style::ParagraphAdjust_BLOCK :
            mnTextAdjust = 3;
        break;
        default :
        case css::style::ParagraphAdjust_LEFT :
            mnTextAdjust = 0;
        break;
    }
    meTextAdjust = ePropState;

    if ( ImplGetPropertyValue( "ParaLineSpacing", bGetPropStateValue ) )
    {
        css::style::LineSpacing aLineSpacing
            = *o3tl::doAccess<css::style::LineSpacing>(mAny);
        switch ( aLineSpacing.Mode )
        {
            case css::style::LineSpacingMode::FIX :
                mnLineSpacing = static_cast<sal_Int16>(-( aLineSpacing.Height ) );
                mbFixedLineSpacing = true;
                break;
            case css::style::LineSpacingMode::MINIMUM :
            case css::style::LineSpacingMode::LEADING :
                mnLineSpacing = static_cast<sal_Int16>(-( aLineSpacing.Height ) );
                mbFixedLineSpacing = false;
           break;

            case css::style::LineSpacingMode::PROP :
            default:
                mnLineSpacing = static_cast<sal_Int16>( aLineSpacing.Height );
            break;
        }
    }
    meLineSpacing = ePropState;

    if ( ImplGetPropertyValue( "ParaBottomMargin", bGetPropStateValue ) )
    {
        double fSpacing = *o3tl::doAccess<sal_uInt32>(mAny) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingBottom = static_cast<sal_Int16>(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingBottom = ePropState;

    if ( ImplGetPropertyValue( "ParaTopMargin", bGetPropStateValue ) )
    {
        double fSpacing = *o3tl::doAccess<sal_uInt32>(mAny) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingTop = static_cast<sal_Int16>(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingTop = ePropState;

    if ( ImplGetPropertyValue( "ParaIsForbiddenRules", bGetPropStateValue ) )
        mAny >>= mbForbiddenRules;
    meForbiddenRules = ePropState;

    if ( ImplGetPropertyValue( "ParaIsHangingPunctuation", bGetPropStateValue ) )
        mAny >>= mbParagraphPunctation;
    meParagraphPunctation = ePropState;

    mnBiDi = 0;
    if ( ImplGetPropertyValue( "WritingMode", bGetPropStateValue ) )
    {
        sal_Int16 nWritingMode = 0;
        mAny >>= nWritingMode;

        SvxFrameDirection eWritingMode = static_cast<SvxFrameDirection>(nWritingMode);
        if ( ( eWritingMode == SvxFrameDirection::Horizontal_RL_TB )
            || ( eWritingMode == SvxFrameDirection::Vertical_RL_TB ) )
        {
            mnBiDi = 1;
        }
    }
    meBiDi = ePropState;
}

void ParagraphObj::ImplConstruct( const ParagraphObj& rParagraphObj )
{
    mbIsBullet = rParagraphObj.mbIsBullet;
    meBullet = rParagraphObj.meBullet;
    meTextAdjust = rParagraphObj.meTextAdjust;
    meLineSpacing = rParagraphObj.meLineSpacing;
    meLineSpacingTop = rParagraphObj.meLineSpacingTop;
    meLineSpacingBottom = rParagraphObj.meLineSpacingBottom;
    meForbiddenRules = rParagraphObj.meForbiddenRules;
    meParagraphPunctation = rParagraphObj.meParagraphPunctation;
    meBiDi =rParagraphObj.meBiDi;
    mbFixedLineSpacing = rParagraphObj.mbFixedLineSpacing;
    mnTextSize = rParagraphObj.mnTextSize;
    mnTextAdjust = rParagraphObj.mnTextAdjust;
    mnLineSpacing = rParagraphObj.mnLineSpacing;
    mnLineSpacingTop = rParagraphObj.mnLineSpacingTop;
    mnLineSpacingBottom = rParagraphObj.mnLineSpacingBottom;
    mbFirstParagraph = rParagraphObj.mbFirstParagraph;
    mbLastParagraph = rParagraphObj.mbLastParagraph;
    mbParagraphPunctation = rParagraphObj.mbParagraphPunctation;
    mbForbiddenRules = rParagraphObj.mbForbiddenRules;
    mnBiDi = rParagraphObj.mnBiDi;

    for ( std::vector<std::unique_ptr<PortionObj> >::const_iterator it = rParagraphObj.begin(); it != rParagraphObj.end(); ++it )
        mvPortions.push_back( std::make_unique<PortionObj>( **it ) );

    maTabStop = rParagraphObj.maTabStop;
    bExtendedParameters = rParagraphObj.bExtendedParameters;
    nParaFlags = rParagraphObj.nParaFlags;
    nBulletFlags = rParagraphObj.nBulletFlags;
    sPrefix = rParagraphObj.sPrefix;
    sSuffix = rParagraphObj.sSuffix;
    sGraphicUrl = rParagraphObj.sGraphicUrl;            // String to a graphic
    aBuGraSize = rParagraphObj.aBuGraSize;
    nNumberingType = rParagraphObj.nNumberingType;      // this is actually a SvxEnum
    nHorzAdjust = rParagraphObj.nHorzAdjust;
    nBulletColor = rParagraphObj.nBulletColor;
    nBulletOfs = rParagraphObj.nBulletOfs;
    nStartWith = rParagraphObj.nStartWith;              // start of numbering
    nTextOfs = rParagraphObj.nTextOfs;
    nBulletRealSize = rParagraphObj.nBulletRealSize;    // scale in percent
    nDepth = rParagraphObj.nDepth;                      // actual depth
    cBulletId = rParagraphObj.cBulletId;                // if Numbering Type == CharSpecial
    aFontDesc = rParagraphObj.aFontDesc;

    bExtendedBulletsUsed = rParagraphObj.bExtendedBulletsUsed;
    nBulletId = rParagraphObj.nBulletId;
}

sal_uInt32 ParagraphObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    mnTextSize = 0;
    for ( std::vector<std::unique_ptr<PortionObj> >::iterator it = mvPortions.begin(); it != mvPortions.end(); ++it )
        mnTextSize += (*it)->ImplCalculateTextPositions( nCurrentTextPosition + mnTextSize );
    return mnTextSize;
}

ParagraphObj& ParagraphObj::operator=( const ParagraphObj& rParagraphObj )
{
    if ( this != &rParagraphObj )
    {
        ImplClear();
        ImplConstruct( rParagraphObj );
    }
    return *this;
}

struct ImplTextObj
{
    sal_uInt32      mnTextSize;
    int             mnInstance;
    std::vector<std::unique_ptr<ParagraphObj>> maList;
    bool        mbHasExtendedBullets;

    explicit ImplTextObj( int nInstance );
};

ImplTextObj::ImplTextObj( int nInstance )
  : maList()
{
    mnTextSize = 0;
    mnInstance = nInstance;
    mbHasExtendedBullets = false;
}

TextObj::TextObj( css::uno::Reference< css::text::XSimpleText > const & rXTextRef,
            int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rProv ):
    mpImplTextObj(new ImplTextObj(nInstance))
{
    css::uno::Reference< css::container::XEnumerationAccess > aXTextParagraphEA( rXTextRef, css::uno::UNO_QUERY );

    if ( aXTextParagraphEA.is()  )
    {
        css::uno::Reference< css::container::XEnumeration > aXTextParagraphE( aXTextParagraphEA->createEnumeration() );
        if ( aXTextParagraphE.is() )
        {
            ParaFlags aParaFlags;
            while ( aXTextParagraphE->hasMoreElements() )
            {
                css::uno::Reference< css::text::XTextContent > aXParagraph;
                css::uno::Any aAny( aXTextParagraphE->nextElement() );
                if ( aAny >>= aXParagraph )
                {
                    if ( !aXTextParagraphE->hasMoreElements() )
                        aParaFlags.bLastParagraph = true;
                    std::unique_ptr<ParagraphObj> pPara(new ParagraphObj( aXParagraph, aParaFlags, rFontCollection, rProv ));
                    mpImplTextObj->mbHasExtendedBullets |= pPara->bExtendedBulletsUsed;
                    mpImplTextObj->maList.push_back( std::move(pPara) );
                    aParaFlags.bFirstParagraph = false;
                }
            }
        }
    }
    ImplCalculateTextPositions();
}

void TextObj::ImplCalculateTextPositions()
{
    mpImplTextObj->mnTextSize = 0;
    for ( sal_uInt32 i = 0; i < ParagraphCount(); ++i )
        mpImplTextObj->mnTextSize += GetParagraph(i)->ImplCalculateTextPositions( mpImplTextObj->mnTextSize );
}

ParagraphObj* TextObj::GetParagraph(int idx)
{
    return mpImplTextObj->maList[idx].get();
}

sal_uInt32 TextObj::ParagraphCount() const
{
    return mpImplTextObj->maList.size();
}

sal_uInt32 TextObj::Count() const
{
    return mpImplTextObj->mnTextSize;
}

int TextObj::GetInstance() const
{
    return mpImplTextObj->mnInstance;
}

bool TextObj::HasExtendedBullets()
{
    return mpImplTextObj->mbHasExtendedBullets;
}

FontCollectionEntry::~FontCollectionEntry()
{
}

void FontCollectionEntry::ImplInit( const OUString& rName )
{
    OUString aSubstName( GetSubsFontName( rName, SubsFontFlags::ONLYONE | SubsFontFlags::MS ) );
    if ( !aSubstName.isEmpty() )
    {
        Name = aSubstName;
    }
    else
    {
        Name = rName;
    }
}

FontCollection::~FontCollection()
{
    pVDev.disposeAndClear();
    xPPTBreakIter = nullptr;
}

FontCollection::FontCollection() :
    pVDev ( nullptr )
{
    xPPTBreakIter = css::i18n::BreakIterator::create( ::comphelper::getProcessComponentContext() );
}

short FontCollection::GetScriptDirection( const OUString& rString )
{
    short nRet = ScriptTypeDetector::getScriptDirection( rString, 0, css::i18n::ScriptDirection::NEUTRAL );
    return nRet;
}

sal_uInt32 FontCollection::GetId( FontCollectionEntry& rEntry )
{
    if( !rEntry.Name.isEmpty() )
    {
        const sal_uInt32 nFonts = maFonts.size();

        for( sal_uInt32 i = 0; i < nFonts; i++ )
        {
            const FontCollectionEntry* pEntry = GetById( i );
            if( pEntry->Name == rEntry.Name )
                return i;
        }
        vcl::Font aFont;
        aFont.SetCharSet( rEntry.CharSet );
        aFont.SetFamilyName( rEntry.Original );
        aFont.SetFontHeight( 100 );

        if ( !pVDev )
            pVDev = VclPtr<VirtualDevice>::Create();

        pVDev->SetFont( aFont );
        FontMetric aMetric( pVDev->GetFontMetric() );

        sal_uInt16 nTxtHeight = static_cast<sal_uInt16>(aMetric.GetAscent()) + static_cast<sal_uInt16>(aMetric.GetDescent());

        if ( nTxtHeight )
        {
            double fScaling = static_cast<double>(nTxtHeight) / 120.0;
            if ( ( fScaling > 0.50 ) && ( fScaling < 1.5 ) )
                rEntry.Scaling = fScaling;
        }

        maFonts.push_back(rEntry);
        return nFonts;
    }
    return 0;
}

const FontCollectionEntry* FontCollection::GetById( sal_uInt32 nId )
{
    return nId < maFonts.size() ? &maFonts[nId] : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
