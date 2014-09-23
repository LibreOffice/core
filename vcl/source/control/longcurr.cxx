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

namespace
{

#define FORMAT_LONGCURRENCY      4

static BigInt ImplPower10( sal_uInt16 n )
{
    sal_uInt16 i;
    BigInt   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

static OUString ImplGetCurr( const LocaleDataWrapper& rLocaleDataWrapper, const BigInt &rNumber, sal_uInt16 nDigits, const OUString& rCurrSymbol, bool bShowThousandSep )
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

    OUStringBuffer aTemplate = rLocaleDataWrapper.getCurr( (long)aFraction, nDigits, rCurrSymbol, bShowThousandSep );
    while( !aInteger.IsZero() )
    {
        aFraction  = aInteger;
        aFraction %= aTmp;
        aInteger  /= aTmp;
        if( !aInteger.IsZero() )
            aFraction += aTmp;

        OUString aFractionStr = rLocaleDataWrapper.getNum( (long)aFraction, 0 );

        sal_Int32 nSPos = aTemplate.indexOf( '1' );
        if ( aFractionStr.getLength() == 1 )
            aTemplate[ nSPos ] = aFractionStr[0];
        else
        {
            aTemplate.remove( nSPos, 1 );
            aTemplate.insert( nSPos, aFractionStr  );
        }
    }

    return aTemplate.makeStringAndClear();
}

static bool ImplNumericProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                        bool bStrictFormat, bool bThousandSep,
                                        const LocaleDataWrapper& rLocaleDataWrapper )
{
    if ( !bStrictFormat )
        return false;
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
            return false;
        else
            return true;
    }
}

static bool ImplNumericGetValue( const OUString& rStr, BigInt& rValue,
                                 sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper,
                                 bool bCurrency = false )
{
    OUString aStr = rStr;
    OUStringBuffer aStr1;
    OUStringBuffer aStr2;
    sal_Int32 nDecPos;
    bool bNegative = false;

    // On empty string
    if ( rStr.isEmpty() )
        return false;

    // Trim leading and trailing spaces
    aStr = string::strip(aStr, ' ');

    // Find decimal sign's position
    nDecPos = aStr.indexOf( rLocaleDataWrapper.getNumDecimalSep() );

    if ( nDecPos != -1 )
    {
        aStr1 = aStr.copy( 0, nDecPos );
        aStr2.append(aStr.copy(nDecPos+1));
    }
    else
        aStr1 = aStr;

    // Negative?
    if ( bCurrency )
    {
        if ( (aStr[ 0 ] == '(') && (aStr[ aStr.getLength()-1 ] == ')') )
            bNegative = true;
        if ( !bNegative )
        {
            for (sal_Int32 i=0; i < aStr.getLength(); i++ )
            {
                if ( (aStr[ i ] >= '0') && (aStr[ i ] <= '9') )
                    break;
                else if ( aStr[ i ] == '-' )
                {
                    bNegative = true;
                    break;
                }
            }
        }
        if ( !bNegative && bCurrency && !aStr.isEmpty() )
        {
            sal_uInt16 nFormat = rLocaleDataWrapper.getCurrNegativeFormat();
            if ( (nFormat == 3) || (nFormat == 6)  ||
                 (nFormat == 7) || (nFormat == 10) )
            {
                for (sal_Int32 i = aStr.getLength()-1; i > 0; i++ )
                {
                    if ( (aStr[ i ] >= '0') && (aStr[ i ] <= '9') )
                        break;
                    else if ( aStr[ i ] == '-' )
                    {
                        bNegative = true;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if ( aStr1[ 0 ] == '-' )
            bNegative = true;
    }

    // delete unwanted characters
    for (sal_Int32 i=0; i < aStr1.getLength(); )
    {
        if ( (aStr1[ i ] >= '0') && (aStr1[ i ] <= '9') )
            i++;
        else
            aStr1.remove( i, 1 );
    }
    for (sal_Int32 i=0; i < aStr2.getLength(); )
    {
        if ((aStr2[i] >= '0') && (aStr2[i] <= '9'))
            ++i;
        else
            aStr2.remove(i, 1);
    }

    if ( aStr1.isEmpty() && aStr2.isEmpty())
        return false;

    if ( aStr1.isEmpty() )
        aStr1 = "0";
    if ( bNegative )
        aStr1.insert( 0, '-');

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

    aStr  = aStr1.makeStringAndClear();
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

    return true;
}

static bool ImplLongCurrencyProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                             bool, bool bUseThousandSep, const LocaleDataWrapper& rLocaleDataWrapper )
{
    // There's no StrictFormat that makes sense here, thus allow all chars
    return ImplNumericProcessKeyInput( pEdit, rKEvt, false, bUseThousandSep, rLocaleDataWrapper  );
}

} // namespace

inline bool ImplLongCurrencyGetValue( const OUString& rStr, BigInt& rValue,
                                      sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper )
{
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rLocaleDataWrapper, true );
}

bool ImplLongCurrencyReformat( const OUString& rStr, BigInt nMin, BigInt nMax,
                               sal_uInt16 nDecDigits,
                               const LocaleDataWrapper& rLocaleDataWrapper, OUString& rOutStr,
                               LongCurrencyFormatter& rFormatter )
{
    BigInt nValue;
    if ( !ImplNumericGetValue( rStr, nValue, nDecDigits, rLocaleDataWrapper, true ) )
        return true;
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
                return false;
            }
            else
            {
                rFormatter.mnCorrectedValue = 0;
            }
        }

        rOutStr = ImplGetCurr( rLocaleDataWrapper, nTempVal, nDecDigits, rFormatter.GetCurrencySymbol(), rFormatter.IsUseThousandSep() );
        return true;
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
    mbThousandSep       = true;
    SetDecimalDigits( 0 );
}

LongCurrencyFormatter::LongCurrencyFormatter()
{
    ImpInit();
}

LongCurrencyFormatter::~LongCurrencyFormatter()
{
}

void LongCurrencyFormatter::SetCurrencySymbol( const OUString& rStr )
{
    maCurrencySymbol= rStr;
    ReformatAll();
}

OUString LongCurrencyFormatter::GetCurrencySymbol() const
{
    return !maCurrencySymbol.isEmpty() ? maCurrencySymbol : GetLocaleDataWrapper().getCurrSymbol();
}

void LongCurrencyFormatter::SetValue( BigInt nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( false );
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

    OUString aStr = ImplGetCurr( GetLocaleDataWrapper(), nNewValue, GetDecimalDigits(), GetCurrencySymbol(), IsUseThousandSep() );
    if ( GetField()->HasFocus() )
    {
        Selection aSelection = GetField()->GetSelection();
        GetField()->SetText( aStr );
        GetField()->SetSelection( aSelection );
    }
    else
        GetField()->SetText( aStr );
    MarkToBeReformatted( false );
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

    OUString aStr;
    bool bOK = ImplLongCurrencyReformat( GetField()->GetText(), mnMin, mnMax,
                                         GetDecimalDigits(), GetLocaleDataWrapper(), aStr, *this );
    if ( !bOK )
        return;

    if ( !aStr.isEmpty() )
    {
        GetField()->SetText( aStr );
        MarkToBeReformatted( false );
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

void LongCurrencyFormatter::SetUseThousandSep( bool b )
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


void ImplNewLongCurrencyFieldValue( LongCurrencyField* pField, BigInt nNewValue )
{
    Selection aSelect = pField->GetSelection();
    aSelect.Justify();
    OUString aText = pField->GetText();
    bool bLastSelected = aSelect.Max() == aText.getLength();

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

LongCurrencyField::LongCurrencyField( vcl::Window* pParent, WinBits nWinStyle ) :
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

bool LongCurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), GetLocaleDataWrapper() ) )
            return true;
    }
    return SpinField::PreNotify( rNEvt );
}

bool LongCurrencyField::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( false );
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
    MarkToBeReformatted( true );
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

LongCurrencyBox::LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

LongCurrencyBox::~LongCurrencyBox()
{
}

bool LongCurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), GetLocaleDataWrapper() ) )
            return true;
    }
    return ComboBox::PreNotify( rNEvt );
}

bool LongCurrencyBox::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( false );
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
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void LongCurrencyBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( false );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplLongCurrencyReformat( GetEntry( i ), mnMin, mnMax,
                                  GetDecimalDigits(), GetLocaleDataWrapper(),
                                  aStr, *this );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    LongCurrencyFormatter::Reformat();
    SetUpdateMode( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
