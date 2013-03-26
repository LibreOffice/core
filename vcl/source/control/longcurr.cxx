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

#include <comphelper/string.hxx>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <tools/debug.hxx>
#include <tools/bigint.hxx>

#include <tools/rc.h>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/longcurr.hxx>

#include <svdata.hxx>

#include <unotools/localedatawrapper.hxx>

using namespace ::comphelper;

#define FORMAT_LONGCURRENCY      4

static BigInt ImplPower10( sal_uInt16 n )
{
    sal_uInt16 i;
    BigInt   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

static XubString ImplGetCurr( const LocaleDataWrapper& rLocaleDataWrapper, const BigInt &rNumber, sal_uInt16 nDigits, const String& rCurrSymbol, sal_Bool bShowThousandSep )
{
    DBG_ASSERT( nDigits < 10, "LongCurrency may only have 9 decimal places" );

    if ( rNumber.IsZero() || (long)rNumber )
        return rLocaleDataWrapper.getCurr( (long)rNumber, nDigits, rCurrSymbol, bShowThousandSep );

    BigInt aTmp( ImplPower10( nDigits ) );
    BigInt aInteger( rNumber );
    aInteger.Abs();
    aInteger  /= aTmp;
    BigInt aFraction( rNumber );
    aFraction.Abs();
    aFraction %= aTmp;
    if ( !aInteger.IsZero() )
    {
        aFraction += aTmp;
        aTmp       = 1000000000L;
    }
    if ( rNumber.IsNeg() )
        aFraction *= -1;

    XubString aTemplate = rLocaleDataWrapper.getCurr( (long)aFraction, nDigits, rCurrSymbol, bShowThousandSep );
    while( !aInteger.IsZero() )
    {
        aFraction  = aInteger;
        aFraction %= aTmp;
        aInteger  /= aTmp;
        if( !aInteger.IsZero() )
            aFraction += aTmp;

        XubString aFractionStr = rLocaleDataWrapper.getNum( (long)aFraction, 0 );

        xub_StrLen nSPos = aTemplate.Search( '1' );
        if ( aFractionStr.Len() == 1 )
            aTemplate.SetChar( nSPos, aFractionStr.GetChar( 0 ) );
        else
        {
            aTemplate.Erase( nSPos, 1 );
            aTemplate.Insert( aFractionStr, nSPos );
        }
    }

    return aTemplate;
}

static sal_Bool ImplNumericProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                        sal_Bool bStrictFormat, sal_Bool bThousandSep,
                                        const LocaleDataWrapper& rLocaleDataWrapper )
{
    if ( !bStrictFormat )
        return sal_False;
    else
    {
        sal_Unicode cChar = rKEvt.GetCharCode();
        sal_uInt16      nGroup = rKEvt.GetKeyCode().GetGroup();

        if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC) ||
             ((cChar >= '0') && (cChar <= '9')) ||
             (bThousandSep && string::equals(rLocaleDataWrapper.getNumThousandSep(), cChar)) ||
             (string::equals(rLocaleDataWrapper.getNumDecimalSep(), cChar) ) ||
             (cChar == '-') )
            return sal_False;
        else
            return sal_True;
    }
}

static sal_Bool ImplNumericGetValue( const XubString& rStr, BigInt& rValue,
                                 sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper,
                                 sal_Bool bCurrency = sal_False )
{
    XubString aStr = rStr;
    XubString aStr1;
    rtl::OUStringBuffer aStr2;
    sal_uInt16 nDecPos;
    sal_Bool bNegative = sal_False;

    // On empty string
    if ( !rStr.Len() )
        return sal_False;

    // Trim leading and trailing spaces
    aStr = string::strip(aStr, ' ');

    // Find decimal sign's position
    nDecPos = aStr.Search( rLocaleDataWrapper.getNumDecimalSep() );

    if ( nDecPos != STRING_NOTFOUND )
    {
        aStr1 = aStr.Copy( 0, nDecPos );
        aStr2.append(aStr.Copy(nDecPos+1));
    }
    else
        aStr1 = aStr;

    // Negative?
    if ( bCurrency )
    {
        if ( (aStr.GetChar( 0 ) == '(') && (aStr.GetChar( aStr.Len()-1 ) == ')') )
            bNegative = sal_True;
        if ( !bNegative )
        {
            for (xub_StrLen i=0; i < aStr.Len(); i++ )
            {
                if ( (aStr.GetChar( i ) >= '0') && (aStr.GetChar( i ) <= '9') )
                    break;
                else if ( aStr.GetChar( i ) == '-' )
                {
                    bNegative = sal_True;
                    break;
                }
            }
        }
        if ( !bNegative && bCurrency && aStr.Len() )
        {
            sal_uInt16 nFormat = rLocaleDataWrapper.getCurrNegativeFormat();
            if ( (nFormat == 3) || (nFormat == 6)  ||
                 (nFormat == 7) || (nFormat == 10) )
            {
                for (xub_StrLen i = (sal_uInt16)(aStr.Len()-1); i > 0; i++ )
                {
                    if ( (aStr.GetChar( i ) >= '0') && (aStr.GetChar( i ) <= '9') )
                        break;
                    else if ( aStr.GetChar( i ) == '-' )
                    {
                        bNegative = sal_True;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if ( aStr1.GetChar( 0 ) == '-' )
            bNegative = sal_True;
    }

    // delete unwanted characters
    for (xub_StrLen i=0; i < aStr1.Len(); )
    {
        if ( (aStr1.GetChar( i ) >= '0') && (aStr1.GetChar( i ) <= '9') )
            i++;
        else
            aStr1.Erase( i, 1 );
    }
    for (sal_Int32 i=0; i < aStr2.getLength();)
    {
        if ((aStr2[i] >= '0') && (aStr2[i] <= '9'))
            ++i;
        else
            aStr2.remove(i, 1);
    }

    if (!aStr1.Len() && !aStr2.getLength())
        return sal_False;

    if ( !aStr1.Len() )
        aStr1.Insert( '0' );
    if ( bNegative )
        aStr1.Insert( '-', 0 );

    // Cut down decimal part and round while doing so
    bool bRound = false;
    if (aStr2.getLength() > nDecDigits)
    {
        if (aStr2[nDecDigits] >= '5')
            bRound = true;
        string::truncateToLength(aStr2, nDecDigits);
    }
    if (aStr2.getLength() < nDecDigits)
        string::padToLength(aStr2, nDecDigits, '0');

    aStr  = aStr1;
    aStr += aStr2.makeStringAndClear();

    // check range
    BigInt nValue( aStr );
    if ( bRound )
    {
        if ( !bNegative )
            nValue+=1;
        else
            nValue-=1;
    }

    rValue = nValue;

    return sal_True;
}

static sal_Bool ImplLongCurrencyProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                             sal_Bool, sal_Bool bUseThousandSep, const LocaleDataWrapper& rLocaleDataWrapper )
{
    // There's no StrictFormat that makes sense here, thus allow all chars
    return ImplNumericProcessKeyInput( pEdit, rKEvt, sal_False, bUseThousandSep, rLocaleDataWrapper  );
}

inline sal_Bool ImplLongCurrencyGetValue( const XubString& rStr, BigInt& rValue,
                                      sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper )
{
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rLocaleDataWrapper, sal_True );
}

sal_Bool ImplLongCurrencyReformat( const XubString& rStr, BigInt nMin, BigInt nMax,
                               sal_uInt16 nDecDigits,
                               const LocaleDataWrapper& rLocaleDataWrapper, String& rOutStr,
                               LongCurrencyFormatter& rFormatter )
{
    BigInt nValue;
    if ( !ImplNumericGetValue( rStr, nValue, nDecDigits, rLocaleDataWrapper, sal_True ) )
        return sal_True;
    else
    {
        BigInt nTempVal = nValue;
        if ( nTempVal > nMax )
            nTempVal = nMax;
        else if ( nTempVal < nMin )
            nTempVal = nMin;

        if ( rFormatter.GetErrorHdl().IsSet() && (nValue != nTempVal) )
        {
            rFormatter.mnCorrectedValue = nTempVal;
            if ( !rFormatter.GetErrorHdl().Call( &rFormatter ) )
            {
                rFormatter.mnCorrectedValue = 0;
                return sal_False;
            }
            else
            {
                rFormatter.mnCorrectedValue = 0;
            }
        }

        rOutStr = ImplGetCurr( rLocaleDataWrapper, nTempVal, nDecDigits, rFormatter.GetCurrencySymbol(), rFormatter.IsUseThousandSep() );
        return sal_True;
    }
}

void LongCurrencyFormatter::ImpInit()
{
    mnFieldValue        = 0;
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = 0x7FFFFFFF;
    mnMax              *= 0x7FFFFFFF;
    mnCorrectedValue    = 0;
    mnDecimalDigits     = 0;
    mnType              = FORMAT_LONGCURRENCY;
    mbThousandSep       = sal_True;
    SetDecimalDigits( 0 );
}

LongCurrencyFormatter::LongCurrencyFormatter()
{
    ImpInit();
}

LongCurrencyFormatter::~LongCurrencyFormatter()
{
}

void LongCurrencyFormatter::SetCurrencySymbol( const String& rStr )
{
    maCurrencySymbol= rStr;
    ReformatAll();
}

String LongCurrencyFormatter::GetCurrencySymbol() const
{
    return !maCurrencySymbol.isEmpty() ? maCurrencySymbol : GetLocaleDataWrapper().getCurrSymbol();
}

void LongCurrencyFormatter::SetValue( BigInt nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( sal_False );
}

void LongCurrencyFormatter::SetUserValue( BigInt nNewValue )
{
    if ( nNewValue > mnMax )
        nNewValue = mnMax;
    else if ( nNewValue < mnMin )
        nNewValue = mnMin;
    mnLastValue = nNewValue;

    if ( !GetField() )
        return;

    XubString aStr = ImplGetCurr( GetLocaleDataWrapper(), nNewValue, GetDecimalDigits(), GetCurrencySymbol(), IsUseThousandSep() );
    if ( GetField()->HasFocus() )
    {
        Selection aSelection = GetField()->GetSelection();
        GetField()->SetText( aStr );
        GetField()->SetSelection( aSelection );
    }
    else
        GetField()->SetText( aStr );
    MarkToBeReformatted( sal_False );
}

BigInt LongCurrencyFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    BigInt nTempValue;
    if ( ImplLongCurrencyGetValue( GetField()->GetText(), nTempValue, GetDecimalDigits(), GetLocaleDataWrapper() ) )
    {
        if ( nTempValue > mnMax )
            nTempValue = mnMax;
        else if ( nTempValue < mnMin )
            nTempValue = mnMin;
        return nTempValue;
    }
    else
        return mnLastValue;
}

void LongCurrencyFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( GetField()->GetText().isEmpty() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    sal_Bool bOK = ImplLongCurrencyReformat( GetField()->GetText(), mnMin, mnMax,
                                         GetDecimalDigits(), GetLocaleDataWrapper(), aStr, *this );
    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        GetField()->SetText( aStr );
        MarkToBeReformatted( sal_False );
        ImplLongCurrencyGetValue( aStr, mnLastValue, GetDecimalDigits(), GetLocaleDataWrapper() );
    }
    else
        SetValue( mnLastValue );
}

void LongCurrencyFormatter::ReformatAll()
{
    Reformat();
}

void LongCurrencyFormatter::SetMin( BigInt nNewMin )
{
    mnMin = nNewMin;
    ReformatAll();
}

void LongCurrencyFormatter::SetMax( BigInt nNewMax )
{
    mnMax = nNewMax;
    ReformatAll();
}

void LongCurrencyFormatter::SetUseThousandSep( sal_Bool b )
{
    mbThousandSep = b;
    ReformatAll();
}

void LongCurrencyFormatter::SetDecimalDigits( sal_uInt16 nDigits )
{
    if ( nDigits > 9 )
        nDigits = 9;

    mnDecimalDigits = nDigits;
    ReformatAll();
}

sal_uInt16 LongCurrencyFormatter::GetDecimalDigits() const
{
    return mnDecimalDigits;
}

void ImplNewLongCurrencyFieldValue( LongCurrencyField* pField, BigInt nNewValue )
{
    Selection aSelect = pField->GetSelection();
    aSelect.Justify();
    XubString aText = pField->GetText();
    sal_Bool bLastSelected = ((xub_StrLen)aSelect.Max() == aText.Len()) ? sal_True : sal_False;

    BigInt nOldLastValue  = pField->mnLastValue;
    pField->SetUserValue( nNewValue );
    pField->mnLastValue  = nOldLastValue;

    if ( bLastSelected )
    {
        if ( !aSelect.Len() )
            aSelect.Min() = SELECTION_MAX;
        aSelect.Max() = SELECTION_MAX;
    }
    pField->SetSelection( aSelect );
    pField->SetModifyFlag();
    pField->Modify();
}

LongCurrencyField::LongCurrencyField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    mnSpinSize   = 1;
    mnFirst      = mnMin;
    mnLast       = mnMax;

    Reformat();
}


LongCurrencyField::~LongCurrencyField()
{
}


long LongCurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), GetLocaleDataWrapper() ) )
            return 1;
    }
    return SpinField::PreNotify( rNEvt );
}


long LongCurrencyField::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( sal_False );
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            Reformat();
            SpinField::Modify();
        }
    }
    return SpinField::Notify( rNEvt );
}


void LongCurrencyField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}


void LongCurrencyField::Up()
{
    BigInt nValue = GetValue();
    nValue += mnSpinSize;
    if ( nValue > mnMax )
        nValue = mnMax;

    ImplNewLongCurrencyFieldValue( this, nValue );
    SpinField::Up();
}

void LongCurrencyField::Down()
{
    BigInt nValue = GetValue();
    nValue -= mnSpinSize;
    if ( nValue < mnMin )
        nValue = mnMin;

    ImplNewLongCurrencyFieldValue( this, nValue );
    SpinField::Down();
}


void LongCurrencyField::First()
{
    ImplNewLongCurrencyFieldValue( this, mnFirst );
    SpinField::First();
}


void LongCurrencyField::Last()
{
    ImplNewLongCurrencyFieldValue( this, mnLast );
    SpinField::Last();
}

LongCurrencyBox::LongCurrencyBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

LongCurrencyBox::~LongCurrencyBox()
{
}

long LongCurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), GetLocaleDataWrapper() ) )
            return 1;
    }
    return ComboBox::PreNotify( rNEvt );
}


long LongCurrencyBox::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( sal_False );
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            Reformat();
            ComboBox::Modify();
        }
    }
    return ComboBox::Notify( rNEvt );
}

void LongCurrencyBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

void LongCurrencyBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplLongCurrencyReformat( GetEntry( i ), mnMin, mnMax,
                                  GetDecimalDigits(), GetLocaleDataWrapper(),
                                  aStr, *this );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    LongCurrencyFormatter::Reformat();
    SetUpdateMode( sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
