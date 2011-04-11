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

#include "text.hxx"

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <svl/languageoptions.hxx>
#include <sfx2/app.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>
#include <unotools/fontcvt.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#include <oox/export/drawingml.hxx> // for SubstituteBullet

com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > xPPTBreakIter;
com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector > xScriptTypeDetector;

PortionObj::PortionObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                FontCollection& rFontCollection ) :
    mnCharAttrHard      ( 0 ),
    mnCharAttr          ( 0 ),
    mnFont              ( 0 ),
    mnAsianOrComplexFont( 0xffff ),
    mnTextSize          ( 0 ),
    mbLastPortion       ( sal_True ),
    mpText              ( NULL ),
    mpFieldEntry        ( NULL )
{
    mXPropSet = rXPropSet;

    ImplGetPortionValues( rFontCollection, sal_False );
}

PortionObj::PortionObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRange,
                            sal_Bool bLast, FontCollection& rFontCollection ) :
    mnCharAttrHard          ( 0 ),
    mnCharAttr              ( 0 ),
    mnFont                  ( 0 ),
    mnAsianOrComplexFont    ( 0xffff ),
    mbLastPortion           ( bLast ),
    mpText                  ( NULL ),
    mpFieldEntry            ( NULL )
{
    String aString( rXTextRange->getString() );
    String aURL;
    sal_Bool bRTL_endingParen = sal_False;

    mnTextSize = aString.Len();
    if ( bLast )
        mnTextSize++;

    if ( mnTextSize )
    {
        mpFieldEntry = NULL;
        sal_uInt32 nFieldType = 0;

        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );
        mXPropState = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyState >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );

        sal_Bool bPropSetsValid = ( mXPropSet.is() && mXPropState.is() );
        if ( bPropSetsValid )
            nFieldType = ImplGetTextField( rXTextRange, mXPropSet, aURL );
        if ( nFieldType )
        {
            mpFieldEntry = new FieldEntry( nFieldType, 0, mnTextSize );
            if ( ( nFieldType >> 28 == 4 ) )
            {
                mpFieldEntry->aRepresentation = aString;
                mpFieldEntry->aFieldUrl = aURL;
            }
        }
        sal_Bool bSymbol = sal_False;

        if ( bPropSetsValid && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), sal_False ) )
        {
            sal_Int16 nCharset = 0;
            mAny >>= nCharset;
            if ( nCharset == ::com::sun::star::awt::CharSet::SYMBOL )
                bSymbol = sal_True;
        }
        if ( mpFieldEntry && ( nFieldType & 0x800000 ) )    // placeholder ?
        {
            mnTextSize = 1;
            if ( bLast )
                mnTextSize++;
            mpText = new sal_uInt16[ mnTextSize ];
            mpText[ 0 ] = 0x2a;
        }
        else
        {
            const sal_Unicode* pText = aString.GetBuffer();
            // For i39516 - a closing parenthesis that ends an RTL string is displayed backwards by PPT
            // Solution: add a Unicode Right-to-Left Mark, following the method described in i18024
            if ( bLast && pText[ aString.Len() - 1 ] == sal_Unicode(')') && rFontCollection.GetScriptDirection( aString ) == com::sun::star::i18n::ScriptDirection::RIGHT_TO_LEFT )
            {
                mnTextSize++;
                bRTL_endingParen = sal_True;
            }
            mpText = new sal_uInt16[ mnTextSize ];
            sal_uInt16 nChar;
            for ( int i = 0; i < aString.Len(); i++ )
            {
                nChar = (sal_uInt16)pText[ i ];
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
            ImplGetPortionValues( rFontCollection, sal_True );
    }
}

PortionObj::PortionObj( PortionObj& rPortionObj )
: PropStateValue( rPortionObj )
{
    ImplConstruct( rPortionObj );
}

PortionObj::~PortionObj()
{
    ImplClear();
}

void PortionObj::Write( SvStream* pStrm, sal_Bool bLast )
{
    sal_uInt32 nCount = mnTextSize;
    if ( bLast && mbLastPortion )
        nCount--;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        *pStrm << (sal_uInt16)mpText[ i ];
}

void PortionObj::ImplGetPortionValues( FontCollection& rFontCollection, sal_Bool bGetPropStateValue )
{

    sal_Bool bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), bGetPropStateValue );
    meFontName = ePropState;
    if ( bOk )
    {
        FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
        sal_uInt32  nCount = rFontCollection.GetCount();
        mnFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
        if ( mnFont == nCount )
        {
            FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), sal_False ) )
                mAny >>= rFontDesc.CharSet;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamily" ) ), sal_False ) )
                mAny >>= rFontDesc.Family;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitch" ) ), sal_False ) )
                mAny >>= rFontDesc.Pitch;
        }
    }

    sal_Int16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
    if ( mpText && mnTextSize && xPPTBreakIter.is() )
    {
        rtl::OUString sT( mpText, mnTextSize );
        nScriptType = xPPTBreakIter->getScriptType( sT, 0 );
    }
    if ( nScriptType != com::sun::star::i18n::ScriptType::COMPLEX )
    {
        bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontNameAsian" ) ), bGetPropStateValue );
        meAsianOrComplexFont = ePropState;
        if ( bOk )
        {
            FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
            sal_uInt32  nCount = rFontCollection.GetCount();
            mnAsianOrComplexFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
            if ( mnAsianOrComplexFont == nCount )
            {
                FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSetAsian" ) ), sal_False ) )
                    mAny >>= rFontDesc.CharSet;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamilyAsian" ) ), sal_False ) )
                    mAny >>= rFontDesc.Family;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitchAsian" ) ), sal_False ) )
                    mAny >>= rFontDesc.Pitch;
            }
        }
    }
    else
    {
        bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontNameComplex" ) ), bGetPropStateValue );
        meAsianOrComplexFont = ePropState;
        if ( bOk )
        {
            FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
            sal_uInt32  nCount = rFontCollection.GetCount();
            mnAsianOrComplexFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
            if ( mnAsianOrComplexFont == nCount )
            {
                FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSetComplex" ) ), sal_False ) )
                    mAny >>= rFontDesc.CharSet;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamilyComplex" ) ), sal_False ) )
                    mAny >>= rFontDesc.Family;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitchComplex" ) ), sal_False ) )
                    mAny >>= rFontDesc.Pitch;
            }
        }
    }

    rtl::OUString aCharHeightName, aCharWeightName, aCharLocaleName, aCharPostureName;
    switch( nScriptType )
    {
        case com::sun::star::i18n::ScriptType::ASIAN :
        {
            aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeightAsian" ) );
            aCharWeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeightAsian" ) );
            aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocaleAsian" ) );
            aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPostureAsian" ) );
            break;
        }
        case com::sun::star::i18n::ScriptType::COMPLEX :
        {
            aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeightComplex" ) );
            aCharWeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeightComplex" ) );
            aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocaleComplex" ) );
            aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPostureComplex" ) );
            break;
        }
        default:
        {
            aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) );
            aCharWeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) );
            aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocale" ) );
            aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) );
            break;
        }
    }

    mnCharHeight = 24;
    if ( GetPropertyValue( mAny, mXPropSet, aCharHeightName, sal_False ) )
    {
        float fVal(0.0);
        if ( mAny >>= fVal )
        {
            mnCharHeight = (sal_uInt16)( fVal + 0.5 );
            meCharHeight = GetPropertyState( mXPropSet, aCharHeightName );
        }
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharWeightName, sal_False ) )
    {
        float fFloat(0.0);
        if ( mAny >>= fFloat )
        {
            if ( fFloat >= ::com::sun::star::awt::FontWeight::SEMIBOLD )
                mnCharAttr |= 1;
            if ( GetPropertyState( mXPropSet, aCharWeightName ) == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                mnCharAttrHard |= 1;
        }
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharLocaleName, sal_False ) )
    {
        com::sun::star::lang::Locale eLocale;
        if ( mAny >>= eLocale )
            meCharLocale = eLocale;
    }
    if ( GetPropertyValue( mAny, mXPropSet, aCharPostureName, sal_False ) )
    {
        ::com::sun::star::awt::FontSlant aFS;
        if ( mAny >>= aFS )
        {
            switch( aFS )
            {
                case ::com::sun::star::awt::FontSlant_OBLIQUE :
                case ::com::sun::star::awt::FontSlant_ITALIC :
                    mnCharAttr |= 2;
                    break;
                default:
                    break;
            }
            if ( GetPropertyState( mXPropSet, aCharPostureName ) == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                mnCharAttrHard |= 2;
        }
    }

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        switch ( nVal )
        {
            case ::com::sun::star::awt::FontUnderline::SINGLE :
            case ::com::sun::star::awt::FontUnderline::DOUBLE :
            case ::com::sun::star::awt::FontUnderline::DOTTED :
                mnCharAttr |= 4;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 4;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ), bGetPropStateValue ) )
    {
        sal_Bool bBool(sal_False);
        mAny >>= bBool;
        if ( bBool )
            mnCharAttr |= 0x10;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 16;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharRelief" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        if ( nVal != ::com::sun::star::text::FontRelief::NONE )
            mnCharAttr |= 512;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 512;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSOColor = *( (sal_uInt32*)mAny.getValue() );
        mnCharColor = nSOColor & 0xff00ff00;                            // green and hibyte
        mnCharColor |= (sal_uInt8)( nSOColor ) << 16;                   // red and blue is switched
        mnCharColor |= (sal_uInt8)( nSOColor >> 16 );
    }
    meCharColor = ePropState;

    mnCharEscapement = 0;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), bGetPropStateValue ) )
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
    delete (FieldEntry*)mpFieldEntry;
    delete[] mpText;
}

void PortionObj::ImplConstruct( PortionObj& rPortionObj )
{
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
        mpText = new sal_uInt16[ mnTextSize ];
        memcpy( mpText, rPortionObj.mpText, mnTextSize << 1 );
    }
    else
        mpText = NULL;

    if ( rPortionObj.mpFieldEntry )
        mpFieldEntry = new FieldEntry( *( rPortionObj.mpFieldEntry ) );
    else
        mpFieldEntry = NULL;
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

//  -----------------------------------------------------------------------
// Rueckgabe:                           0 = kein TextField
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

sal_uInt32 PortionObj::ImplGetTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & ,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet, String& rURL )
{
    sal_uInt32 nRetValue = 0;
    sal_Int32 nFormat;
    ::com::sun::star::uno::Any aAny;
    if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextPortionType" ) ), sal_True ) )
    {
        String  aTextFieldType( *(::rtl::OUString*)aAny.getValue() );
        if ( aTextFieldType == String( RTL_CONSTASCII_USTRINGPARAM( "TextField" ) ) )
        {
            if ( GetPropertyValue( aAny, rXPropSet, aTextFieldType, sal_True ) )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField > aXTextField;
                if ( aAny >>= aXTextField )
                {
                    if ( aXTextField.is() )
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                            xFieldPropSet( aXTextField, ::com::sun::star::uno::UNO_QUERY );
                        if ( xFieldPropSet.is() )
                        {
                            String aFieldKind( aXTextField->getPresentation( sal_True ) );
                            if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Date" ) ) )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
                                {
                                    sal_Bool bBool = sal_False;
                                    aAny >>= bBool;
                                    if ( !bBool )  // Fixed DateFields gibt es in PPT nicht
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ), sal_True )
                                        {
                                            nFormat = *(sal_Int32*)aAny.getValue();
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
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) ), sal_True )
                                    rURL = String( *(::rtl::OUString*)aAny.getValue() );
                                nRetValue = 4 << 28;
                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Page" ) ) )
                            {
                                nRetValue = 3 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) )
                            {

                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Time" ) ) )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
                                {
                                    sal_Bool bBool = sal_False;
                                    aAny >>= bBool;
                                    if ( !bBool )
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
                                        {
                                            nFormat = *(sal_Int32*)aAny.getValue();
                                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                                        }
                                    }
                                }
                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) )
                            {

                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Table" ) ) )
                            {

                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtTime" ) ) )
                            {
                                if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
                                {
                                    sal_Bool bBool = sal_False;
                                    aAny >>= bBool;
                                    if ( !bBool )
                                    {
                                        if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ), sal_True )
                                        {
                                            nFormat = *(sal_Int32*)aAny.getValue();
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
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtFile" ) ) )
                            {

                            }
                            else if ( aFieldKind ==  String( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ) )
                            {

                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "DateTime" ) ) )
                            {
                                nRetValue = 5 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Header" ) ) )
                            {
                                nRetValue = 6 << 28 | 0x800000;
                            }
                            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Footer" ) ) )
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

PortionObj& PortionObj::operator=( PortionObj& rPortionObj )
{
    if ( this != &rPortionObj )
    {
        ImplClear();
        ImplConstruct( rPortionObj );
    }
    return *this;
}

ParagraphObj::ParagraphObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) )
{
    mXPropSet = rXPropSet;

    bExtendedParameters = sal_False;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    ImplGetParagraphValues( rProv, sal_False );
}

    ParagraphObj::ParagraphObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & rXTextContent,
                    ParaFlags aParaFlags, FontCollection& rFontCollection, PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) ),
    mbFirstParagraph    ( aParaFlags.bFirstParagraph ),
    mbLastParagraph     ( aParaFlags.bLastParagraph )
{
    bExtendedParameters = sal_False;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    mXPropState = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertyState >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    if ( mXPropSet.is() && mXPropState.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
            aXTextPortionEA( rXTextContent, ::com::sun::star::uno::UNO_QUERY );
        if ( aXTextPortionEA.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                aXTextPortionE( aXTextPortionEA->createEnumeration() );
            if ( aXTextPortionE.is() )
            {
                while ( aXTextPortionE->hasMoreElements() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > aXCursorText;
                    ::com::sun::star::uno::Any aAny( aXTextPortionE->nextElement() );
                    if ( aAny >>= aXCursorText )
                    {
                        PortionObj* pPortionObj = new PortionObj( aXCursorText, !aXTextPortionE->hasMoreElements(), rFontCollection );
                        if ( pPortionObj->Count() )
                            Insert( pPortionObj, LIST_APPEND );
                        else
                            delete pPortionObj;
                    }
                }
            }
        }
        ImplGetParagraphValues( rProv, sal_True );
    }
}

ParagraphObj::ParagraphObj( ParagraphObj& rObj )
: List()
, PropStateValue()
, SOParagraph()
{
    ImplConstruct( rObj );
}

ParagraphObj::~ParagraphObj()
{
    ImplClear();
}

void ParagraphObj::Write( SvStream* pStrm )
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((PortionObj*)pPtr)->Write( pStrm, mbLastParagraph );
}

void ParagraphObj::ImplClear()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (PortionObj*)pPtr;
}

void ParagraphObj::CalculateGraphicBulletSize( sal_uInt16 nFontHeight )
{
    if ( ( (SvxExtNumType)nNumberingType == SVX_NUM_BITMAP ) && ( nBulletId != 0xffff ) )
    {
        // calculate the bulletrealsize for this grafik
        if ( aBuGraSize.Width() && aBuGraSize.Height() )
        {
            double fCharHeight = nFontHeight;
            double fLen = aBuGraSize.Height();
            fCharHeight = fCharHeight * 0.2540;
            double fQuo = fLen / fCharHeight;
            nBulletRealSize = (sal_Int16)( fQuo + 0.5 );
            if ( (sal_uInt16)nBulletRealSize > 400 )
                nBulletRealSize = 400;
        }
    }
}

// from sw/source/filter/ww8/wrtw8num.cxx for default bullets to export to MS intact
static void lcl_SubstituteBullet(String& rNumStr, rtl_TextEncoding& rChrSet, String& rFontName)
{
    sal_Unicode cChar = rNumStr.GetChar(0);
    StarSymbolToMSMultiFont *pConvert = CreateStarSymbolToMSMultiFont();
    String sFont = pConvert->ConvertChar(cChar);
    delete pConvert;
    if (sFont.Len())
    {
        rNumStr = static_cast< sal_Unicode >(cChar | 0xF000);
        rFontName = sFont;
        rChrSet = RTL_TEXTENCODING_SYMBOL;
    }
    else if ( (rNumStr.GetChar(0) < 0xE000 || rNumStr.GetChar(0) > 0xF8FF) )
    {
        /*
        Ok we can't fit into a known windows unicode font, but
        we are not in the private area, so we are a
        standardized symbol, so turn off the symbol bit and
        let words own font substitution kick in
        */
        rChrSet = RTL_TEXTENCODING_UNICODE;
        rFontName = ::GetFontToken(rFontName, 0);
    }
    else
    {
        /*
        Well we don't have an available substition, and we're
        in our private area, so give up and show a standard
        bullet symbol
        */
        rFontName.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Wingdings"));
        rNumStr = static_cast< sal_Unicode >(0x6C);
     }
}

void ParagraphObj::ImplGetNumberingLevel( PPTExBulletProvider& rBuProv, sal_Int16 nNumberingDepth, sal_Bool bIsBullet, sal_Bool bGetPropStateValue )
{
    ::com::sun::star::uno::Any aAny;
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaLeftMargin" ) ) ) )
    {
        sal_Int32 nVal(0);
        if ( aAny >>= nVal )
            nTextOfs = static_cast< sal_Int16 >( nVal / ( 2540.0 / 576 ) + 0.5 ) ;
    }
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaFirstLineIndent" ) ) ) )
    {
        if ( aAny >>= nBulletOfs )
            nBulletOfs = static_cast< sal_Int32 >( nBulletOfs / ( 2540.0 / 576 ) + 0.5 );
    }
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "NumberingIsNumber" ) ) ) )
        aAny >>= bNumberingIsNumber;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > aXIndexReplace;

    if ( bIsBullet && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ), bGetPropStateValue ) )
    {
        if ( ( mAny >>= aXIndexReplace ) && nNumberingDepth < aXIndexReplace->getCount() )
        {
            mAny <<= aXIndexReplace->getByIndex( nNumberingDepth );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                aPropertySequence( *( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>*)mAny.getValue() );

            const ::com::sun::star::beans::PropertyValue* pPropValue = aPropertySequence.getArray();

            sal_Int32 nPropertyCount = aPropertySequence.getLength();
            if ( nPropertyCount )
            {
                bExtendedParameters = sal_True;
                nBulletRealSize = 100;
                nMappedNumType = 0;

                String aGraphicURL;
                for ( sal_Int32 i = 0; i < nPropertyCount; i++ )
                {
                    const void* pValue = pPropValue[ i ].Value.getValue();
                    if ( pValue )
                    {
                        ::rtl::OUString aPropName( pPropValue[ i ].Name );
                        if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "NumberingType" ) ) )
                            nNumberingType = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Adjust" ) ) )
                            nHorzAdjust = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletChar" ) ) )
                        {
                            String aString( *( (String*)pValue ) );
                            if ( aString.Len() )
                                cBulletId = aString.GetChar( 0 );
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletFont" ) ) )
                        {
                            aFontDesc = *( (::com::sun::star::awt::FontDescriptor*)pValue );

                            // Our numbullet dialog has set the wrong textencoding for our "StarSymbol" font,
                            // instead of a Unicode encoding the encoding RTL_TEXTENCODING_SYMBOL was used.
                            // Because there might exist a lot of damaged documemts I added this two lines
                            // which fixes the bullet problem for the export.
                            if ( aFontDesc.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StarSymbol" ) ) )
                                aFontDesc.CharSet = RTL_TEXTENCODING_MS_1252;

                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicURL" ) ) )
                            aGraphicURL = ( *(::rtl::OUString*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicSize" ) ) )
                        {
                            if ( pPropValue[ i ].Value.getValueType() == ::getCppuType( (::com::sun::star::awt::Size*)0) )
                            {
                                // don't cast awt::Size to Size as on 64-bits they are not the same.
                                ::com::sun::star::awt::Size aSize;
                                pPropValue[ i ].Value >>= aSize;
                                aBuGraSize.nA = aSize.Width;
                                aBuGraSize.nB = aSize.Height;
                            }
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StartWith" ) ) )
                            nStartWith = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LeftMargin" ) ) )
                            nTextOfs = nTextOfs + static_cast< sal_Int16 >( *( (sal_Int32*)pValue ) / ( 2540.0 / 576 ) );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FirstLineOffset" ) ) )
                            nBulletOfs += (sal_Int16)( *( (sal_Int32*)pValue ) / ( 2540.0 / 576 ) );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletColor" ) ) )
                        {
                            sal_uInt32 nSOColor = *( (sal_uInt32*)pValue );
                            nBulletColor = nSOColor & 0xff00ff00;                       // green and hibyte
                            nBulletColor |= (sal_uInt8)( nSOColor ) << 16;              // red
                            nBulletColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000; // blue
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletRelSize" ) ) )
                        {
                            nBulletRealSize = *( (sal_Int16*)pValue );
                            nParaFlags |= 0x40;
                            nBulletFlags |= 8;
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Prefix" ) ) )
                            sPrefix = ( *(::rtl::OUString*)pValue );
                        else if  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Suffix" ) ) )
                            sSuffix = ( *(::rtl::OUString*)pValue );
#ifdef DBG_UTIL
                        else if ( ! (
                                ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SymbolTextDistance" ) ) )
                            ||  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Graphic" ) ) ) ) )
                        {
                            OSL_FAIL( "Unbekanntes Property" );
                        }
#endif
                    }
                }

                if ( aGraphicURL.Len() )
                {
                    if ( aBuGraSize.Width() && aBuGraSize.Height() )
                    {
                        xub_StrLen nIndex = aGraphicURL.Search( (sal_Unicode)':', 0 );
                        if ( nIndex != STRING_NOTFOUND )
                        {
                            nIndex++;
                            if ( aGraphicURL.Len() > nIndex  )
                            {
                                ByteString aUniqueId( aGraphicURL, nIndex, aGraphicURL.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
                                if ( aUniqueId.Len() )
                                {
                                    nBulletId = rBuProv.GetId( aUniqueId, aBuGraSize );
                                    if ( nBulletId != 0xffff )
                                        bExtendedBulletsUsed = sal_True;
                                }
                            }
                        }
                    }
                    else
                    {
                        nNumberingType = SVX_NUM_NUMBER_NONE;
                    }
                }

                PortionObj* pPortion = (PortionObj*)First();
                CalculateGraphicBulletSize( ( pPortion ) ? pPortion->mnCharHeight : 24 );

                switch( (SvxExtNumType)nNumberingType )
                {
                    case SVX_NUM_NUMBER_NONE : nParaFlags |= 0xf; break;

                    case SVX_NUM_CHAR_SPECIAL :                           // Bullet
                    {
                        if ( aFontDesc.Name.equalsIgnoreAsciiCaseAscii("starsymbol") ||
                            aFontDesc.Name.equalsIgnoreAsciiCaseAscii("opensymbol") )
                        {
                            String sFontName = aFontDesc.Name;
                            String sNumStr = cBulletId;
                            rtl_TextEncoding eChrSet = aFontDesc.CharSet;
                            lcl_SubstituteBullet(sNumStr,eChrSet,sFontName);
                            aFontDesc.Name = sFontName;
                            cBulletId = sNumStr.GetChar( 0 );
                            aFontDesc.CharSet = eChrSet;
                         }

                        if ( aFontDesc.Name.getLength() )
                        {
                            nParaFlags |= 0x90; // wir geben den Font und den Charset vor
                        }
                    }
                    case SVX_NUM_CHARS_UPPER_LETTER :       // zaehlt von a-z, aa - az, ba - bz, ...
                    case SVX_NUM_CHARS_LOWER_LETTER :
                    case SVX_NUM_ROMAN_UPPER :
                    case SVX_NUM_ROMAN_LOWER :
                    case SVX_NUM_ARABIC :
                    case SVX_NUM_PAGEDESC :                 // Numerierung aus der Seitenvorlage
                    case SVX_NUM_BITMAP :
                    case SVX_NUM_CHARS_UPPER_LETTER_N :     // zaehlt von  a-z, aa-zz, aaa-zzz
                    case SVX_NUM_CHARS_LOWER_LETTER_N :
                    {
                        if ( nNumberingType != SVX_NUM_CHAR_SPECIAL )
                        {
                            bExtendedBulletsUsed = sal_True;
                            if ( nNumberingDepth & 1 )
                                cBulletId = 0x2013;         // defaulting bullet characters for ppt97
                            else if ( nNumberingDepth == 4 )
                                cBulletId = 0xbb;
                            else
                                cBulletId = 0x2022;

                            switch( (SvxExtNumType)nNumberingType )
                            {
                                case SVX_NUM_CHARS_UPPER_LETTER :
                                case SVX_NUM_CHARS_UPPER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
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
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
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
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
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
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
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
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xc0001;   // (1)
                                        else
                                            nMappedNumType = 0x20001;   // 1)
                                    }
                                    else
                                    {
                                        if ( ! ( sSuffix.Len() + sPrefix.Len() ) )
                                            nMappedNumType = 0xd0001;   // 1
                                        else
                                            nMappedNumType = 0x30001;   // 1.
                                    }
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
                    }
                }
            }
        }
    }
    nBulletOfs = nTextOfs + nBulletOfs;
    if ( nBulletOfs < 0 )
        nBulletOfs = 0;
}

void ParagraphObj::ImplGetParagraphValues( PPTExBulletProvider& rBuProv, sal_Bool bGetPropStateValue )
{
    static String sNumberingLevel   ( RTL_CONSTASCII_USTRINGPARAM( "NumberingLevel" ) );

    ::com::sun::star::uno::Any aAny;
    if ( GetPropertyValue( aAny, mXPropSet, sNumberingLevel, sal_True ) )
    {
        if ( bGetPropStateValue )
            meBullet = GetPropertyState( mXPropSet, sNumberingLevel );
        nDepth = *( (sal_Int16*)aAny.getValue() );

        if ( nDepth < 0 )
        {
            mbIsBullet = sal_False;
            nDepth = 0;
        }
        else
        {
            if ( nDepth > 4 )
                nDepth = 4;
            mbIsBullet = sal_True;
        }
    }
    else
    {
        nDepth = 0;
        mbIsBullet = sal_False;
    }
    ImplGetNumberingLevel( rBuProv, nDepth, mbIsBullet, bGetPropStateValue );

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTabStops" ) ), bGetPropStateValue ) )
        maTabStop = *( ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop>*)mAny.getValue();
    sal_Int16 eTextAdjust( ::com::sun::star::style::ParagraphAdjust_LEFT );
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaAdjust" ) ), bGetPropStateValue ) )
        aAny >>= eTextAdjust;
    switch ( (::com::sun::star::style::ParagraphAdjust)eTextAdjust )
    {
        case ::com::sun::star::style::ParagraphAdjust_CENTER :
            mnTextAdjust = 1;
        break;
        case ::com::sun::star::style::ParagraphAdjust_RIGHT :
            mnTextAdjust = 2;
        break;
        case ::com::sun::star::style::ParagraphAdjust_BLOCK :
            mnTextAdjust = 3;
        break;
        default :
        case ::com::sun::star::style::ParagraphAdjust_LEFT :
            mnTextAdjust = 0;
        break;
    }
    meTextAdjust = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaLineSpacing" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::style::LineSpacing aLineSpacing
            = *( (::com::sun::star::style::LineSpacing*)mAny.getValue() );
        switch ( aLineSpacing.Mode )
        {
            case ::com::sun::star::style::LineSpacingMode::MINIMUM :
            case ::com::sun::star::style::LineSpacingMode::LEADING :
            case ::com::sun::star::style::LineSpacingMode::FIX :
                mnLineSpacing = (sal_Int16)(-( aLineSpacing.Height ) );
            break;

            case ::com::sun::star::style::LineSpacingMode::PROP :
            default:
                mnLineSpacing = (sal_Int16)( aLineSpacing.Height );
            break;
        }
    }
    meLineSpacing = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaBottomMargin" ) ), bGetPropStateValue ) )
    {
        double fSpacing = *( (sal_uInt32*)mAny.getValue() ) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingBottom = (sal_Int16)(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingBottom = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTopMargin" ) ), bGetPropStateValue ) )
    {
        double fSpacing = *( (sal_uInt32*)mAny.getValue() ) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingTop = (sal_Int16)(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingTop = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaIsForbiddenRules" ) ), bGetPropStateValue ) )
        mAny >>= mbForbiddenRules;
    meForbiddenRules = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaIsHangingPunctuation" ) ), bGetPropStateValue ) )
        mAny >>= mbParagraphPunctation;
    meParagraphPunctation = ePropState;

    mnBiDi = 0;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "WritingMode" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nWritingMode = 0;
        mAny >>= nWritingMode;

        SvxFrameDirection eWritingMode( (SvxFrameDirection)nWritingMode );
        if ( ( eWritingMode == FRMDIR_HORI_RIGHT_TOP )
            || ( eWritingMode == FRMDIR_VERT_TOP_RIGHT ) )
        {
            mnBiDi = 1;
        }
    }
    meBiDi = ePropState;
}

void ParagraphObj::ImplConstruct( ParagraphObj& rParagraphObj )
{
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

    for ( void* pPtr = rParagraphObj.First(); pPtr; pPtr = rParagraphObj.Next() )
        Insert( new PortionObj( *(PortionObj*)pPtr ), LIST_APPEND );

    maTabStop = rParagraphObj.maTabStop;
    bExtendedParameters = rParagraphObj.bExtendedParameters;
    nParaFlags = rParagraphObj.nParaFlags;
    nBulletFlags = rParagraphObj.nBulletFlags;
    sPrefix = rParagraphObj.sPrefix;
    sSuffix = rParagraphObj.sSuffix;
    sGraphicUrl = rParagraphObj.sGraphicUrl;            // String auf eine Graphic
    aBuGraSize = rParagraphObj.aBuGraSize;
    nNumberingType = rParagraphObj.nNumberingType;      // in wirlichkeit ist dies ein SvxEnum
    nHorzAdjust = rParagraphObj.nHorzAdjust;
    nBulletColor = rParagraphObj.nBulletColor;
    nBulletOfs = rParagraphObj.nBulletOfs;
    nStartWith = rParagraphObj.nStartWith;              // Start der nummerierung
    nTextOfs = rParagraphObj.nTextOfs;
    nBulletRealSize = rParagraphObj.nBulletRealSize;    // GroessenVerhaeltnis in Proz
    nDepth = rParagraphObj.nDepth;                      // aktuelle tiefe
    cBulletId = rParagraphObj.cBulletId;                // wenn Numbering Type == CharSpecial
    aFontDesc = rParagraphObj.aFontDesc;

    bExtendedBulletsUsed = rParagraphObj.bExtendedBulletsUsed;
    nBulletId = rParagraphObj.nBulletId;
}

::com::sun::star::awt::Size ParagraphObj::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );
    if ( !aSize.Width() )
        aSize.Width()++;
    if ( !aSize.Height() )
        aSize.Height()++;
    return ::com::sun::star::awt::Size( aSize.Width(), aSize.Height() );
}

sal_uInt32 ParagraphObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mnTextSize += ((PortionObj*)pPtr)->ImplCalculateTextPositions( nCurrentTextPosition + mnTextSize );
    return mnTextSize;
}

ParagraphObj& ParagraphObj::operator=( ParagraphObj& rParagraphObj )
{
    if ( this != &rParagraphObj )
    {
        ImplClear();
        ImplConstruct( rParagraphObj );
    }
    return *this;
}

ImplTextObj::ImplTextObj( int nInstance )
{
    mnRefCount = 1;
    mnTextSize = 0;
    mnInstance = nInstance;
    mpList = new List;
    mbHasExtendedBullets = sal_False;
    mbFixedCellHeightUsed = sal_False;
}

ImplTextObj::~ImplTextObj()
{
    for ( ParagraphObj* pPtr = (ParagraphObj*)mpList->First(); pPtr; pPtr = (ParagraphObj*)mpList->Next() )
        delete pPtr;
    delete mpList;
}

TextObj::TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > & rXTextRef,
            int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rProv )
{
    mpImplTextObj = new ImplTextObj( nInstance );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
        aXTextParagraphEA( rXTextRef, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextParagraphEA.is()  )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
            aXTextParagraphE( aXTextParagraphEA->createEnumeration() );
        if ( aXTextParagraphE.is() )
        {
            ParaFlags aParaFlags;
            while ( aXTextParagraphE->hasMoreElements() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > aXParagraph;
                ::com::sun::star::uno::Any aAny( aXTextParagraphE->nextElement() );
                if ( aAny >>= aXParagraph )
                {
                    if ( !aXTextParagraphE->hasMoreElements() )
                        aParaFlags.bLastParagraph = sal_True;
                    ParagraphObj* pPara = new ParagraphObj( aXParagraph, aParaFlags, rFontCollection, rProv );
                    mpImplTextObj->mbHasExtendedBullets |= pPara->bExtendedBulletsUsed;
                    mpImplTextObj->mpList->Insert( pPara, LIST_APPEND );
                    aParaFlags.bFirstParagraph = sal_False;
                }
            }
        }
    }
    ImplCalculateTextPositions();
}

TextObj::TextObj( TextObj& rTextObj )
{
    mpImplTextObj = rTextObj.mpImplTextObj;
    mpImplTextObj->mnRefCount++;
}

TextObj::~TextObj()
{
    if ( ! ( --mpImplTextObj->mnRefCount ) )
        delete mpImplTextObj;
}

void TextObj::Write( SvStream* pStrm )
{
    sal_uInt32 nSize, nPos = pStrm->Tell();
    *pStrm << (sal_uInt32)( EPP_TextCharsAtom << 16 ) << (sal_uInt32)0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((ParagraphObj*)pPtr)->Write( pStrm );
    nSize = pStrm->Tell() - nPos;
    pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
    *pStrm << (sal_uInt32)( nSize - 8 );
    pStrm->SeekRel( nSize - 8 );
}

void TextObj::ImplCalculateTextPositions()
{
    mpImplTextObj->mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mpImplTextObj->mnTextSize += ((ParagraphObj*)pPtr)->ImplCalculateTextPositions( mpImplTextObj->mnTextSize );
}

TextObj& TextObj::operator=( TextObj& rTextObj )
{
    if ( this != &rTextObj )
    {
        if ( ! ( --mpImplTextObj->mnRefCount ) )
            delete mpImplTextObj;
        mpImplTextObj = rTextObj.mpImplTextObj;
        mpImplTextObj->mnRefCount++;
    }
    return *this;
}

FontCollectionEntry::~FontCollectionEntry()
{
}

void FontCollectionEntry::ImplInit( const String& rName )
{
    String aSubstName( GetSubsFontName( rName, SUBSFONT_ONLYONE | SUBSFONT_MS ) );
    if ( aSubstName.Len() )
    {
        Name = aSubstName;
        bIsConverted = sal_True;
    }
    else
    {
        Name = rName;
        bIsConverted = sal_False;
    }
}

FontCollection::~FontCollection()
{
    for( void* pStr = List::First(); pStr; pStr = List::Next() )
        delete (FontCollectionEntry*)pStr;
    delete pVDev;
    xPPTBreakIter = NULL;
    xScriptTypeDetector = NULL;
}

FontCollection::FontCollection() :
    pVDev ( NULL )
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        xMSF = ::comphelper::getProcessServiceFactory();
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        xInterface = xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.BreakIterator" ) ) );
    if ( xInterface.is() )
        xPPTBreakIter = com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator >
            ( xInterface, com::sun::star::uno::UNO_QUERY );

    xInterface = xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.ScriptTypeDetector" ) ) );
    if ( xInterface.is() )
        xScriptTypeDetector = com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector >
            ( xInterface, com::sun::star::uno::UNO_QUERY );
}

short FontCollection::GetScriptDirection( const String& rString ) const
{
    short nRet = com::sun::star::i18n::ScriptDirection::NEUTRAL;
    if ( xScriptTypeDetector.is() )
    {
        const rtl::OUString sT( rString );
        nRet = xScriptTypeDetector->getScriptDirection( sT, 0, com::sun::star::i18n::ScriptDirection::NEUTRAL );
    }
    return nRet;
}

sal_uInt32 FontCollection::GetId( FontCollectionEntry& rEntry )
{
    if( rEntry.Name.Len() )
    {
        const sal_uInt32 nFonts = GetCount();

        for( sal_uInt32 i = 0; i < nFonts; i++ )
        {
            const FontCollectionEntry* pEntry = GetById( i );
            if( pEntry->Name == rEntry.Name )
                return i;
        }
        Font aFont;
        aFont.SetCharSet( rEntry.CharSet );
        aFont.SetName( rEntry.Original );
        aFont.SetHeight( 100 );

        if ( !pVDev )
            pVDev = new VirtualDevice;

        pVDev->SetFont( aFont );
        FontMetric aMetric( pVDev->GetFontMetric() );

        sal_uInt16 nTxtHeight = (sal_uInt16)aMetric.GetAscent() + (sal_uInt16)aMetric.GetDescent();

        if ( nTxtHeight )
        {
            double fScaling = (double)nTxtHeight / 120.0;
            if ( ( fScaling > 0.50 ) && ( fScaling < 1.5 ) )
                rEntry.Scaling = fScaling;
        }

        List::Insert( new FontCollectionEntry( rEntry ), LIST_APPEND );
        return nFonts;
    }
    return 0;
}

const FontCollectionEntry* FontCollection::GetById( sal_uInt32 nId )
{
    return (FontCollectionEntry*)List::GetObject( nId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
