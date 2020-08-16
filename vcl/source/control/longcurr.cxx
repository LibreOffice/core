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

#include <sal/config.h>

#include <string_view>

#include <comphelper/string.hxx>
#include <tools/bigint.hxx>
#include <sal/log.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/longcurr.hxx>
#include <vcl/weldutils.hxx>

#include <unotools/localedatawrapper.hxx>

using namespace ::comphelper;

namespace
{

BigInt ImplPower10( sal_uInt16 n )
{
    sal_uInt16 i;
    BigInt   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

OUString ImplGetCurr( const LocaleDataWrapper& rLocaleDataWrapper, const BigInt &rNumber, sal_uInt16 nDigits, const OUString& rCurrSymbol, bool bShowThousandSep )
{
    SAL_WARN_IF( nDigits >= 10, "vcl", "LongCurrency may only have 9 decimal places" );

    if ( rNumber.IsZero() || static_cast<long>(rNumber) )
        return rLocaleDataWrapper.getCurr( static_cast<long>(rNumber), nDigits, rCurrSymbol, bShowThousandSep );

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
        aTmp       = 1000000000;
    }
    if ( rNumber.IsNeg() )
        aFraction *= -1;

    OUStringBuffer aTemplate = rLocaleDataWrapper.getCurr( static_cast<long>(aFraction), nDigits, rCurrSymbol, bShowThousandSep );
    while( !aInteger.IsZero() )
    {
        aFraction  = aInteger;
        aFraction %= aTmp;
        aInteger  /= aTmp;
        if( !aInteger.IsZero() )
            aFraction += aTmp;

        OUString aFractionStr = rLocaleDataWrapper.getNum( static_cast<long>(aFraction), 0 );

        sal_Int32 nSPos = aTemplate.indexOf( '1' );
        if (nSPos == -1)
            break;
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

bool ImplCurrencyGetValue( const OUString& rStr, BigInt& rValue,
                                 sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper )
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
    if (nDecPos < 0 && !rLocaleDataWrapper.getNumDecimalSepAlt().isEmpty())
        nDecPos = aStr.indexOf( rLocaleDataWrapper.getNumDecimalSepAlt() );

    if ( nDecPos != -1 )
    {
        aStr1 = aStr.copy( 0, nDecPos );
        aStr2.append(std::u16string_view(aStr).substr(nDecPos+1));
    }
    else
        aStr1 = aStr;

    // Negative?
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
    if ( !bNegative && !aStr.isEmpty() )
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

    aStr1.append(aStr2);
    aStr  = aStr1.makeStringAndClear();

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

} // namespace

static bool ImplLongCurrencyGetValue( const OUString& rStr, BigInt& rValue,
                                      sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper )
{
    return ImplCurrencyGetValue( rStr, rValue, nDecDigits, rLocaleDataWrapper );
}

namespace weld
{
    IMPL_LINK_NOARG(LongCurrencyFormatter, FormatOutputHdl, LinkParamNone*, bool)
    {
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
        const OUString& rCurrencySymbol = !m_aCurrencySymbol.isEmpty() ? m_aCurrencySymbol : rLocaleDataWrapper.getCurrSymbol();
        double fValue = GetValue() * weld::SpinButton::Power10(GetDecimalDigits());
        OUString aText = ImplGetCurr(rLocaleDataWrapper, fValue, GetDecimalDigits(), rCurrencySymbol, m_bThousandSep);
        ImplSetTextImpl(aText, nullptr);
        return true;
    }

    IMPL_LINK(LongCurrencyFormatter, ParseInputHdl, sal_Int64*, result, TriState)
    {
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();

        BigInt value;
        bool bRet = ImplLongCurrencyGetValue(GetEntryText(), value, GetDecimalDigits(), rLocaleDataWrapper);

        if (bRet)
            *result = double(value);

        return bRet ? TRISTATE_TRUE : TRISTATE_FALSE;
    }
}

bool ImplLongCurrencyReformat( const OUString& rStr, BigInt const & nMin, BigInt const & nMax,
                               sal_uInt16 nDecDigits,
                               const LocaleDataWrapper& rLocaleDataWrapper, OUString& rOutStr,
                               LongCurrencyFormatter const & rFormatter )
{
    BigInt nValue;
    if ( !ImplCurrencyGetValue( rStr, nValue, nDecDigits, rLocaleDataWrapper ) )
        return true;
    else
    {
        BigInt nTempVal = nValue;
        if ( nTempVal > nMax )
            nTempVal = nMax;
        else if ( nTempVal < nMin )
            nTempVal = nMin;

        rOutStr = ImplGetCurr( rLocaleDataWrapper, nTempVal, nDecDigits, rFormatter.GetCurrencySymbol(), /*IsUseThousandSep*/true );
        return true;
    }
}

void LongCurrencyFormatter::ImpInit()
{
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = 0x7FFFFFFF;
    mnMax              *= 0x7FFFFFFF;
    mnDecimalDigits     = 0;
    SetDecimalDigits( 0 );
}

LongCurrencyFormatter::LongCurrencyFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
{
    ImpInit();
}

LongCurrencyFormatter::~LongCurrencyFormatter()
{
}

OUString const & LongCurrencyFormatter::GetCurrencySymbol() const
{
    return GetLocaleDataWrapper().getCurrSymbol();
}

void LongCurrencyFormatter::SetValue(const BigInt& rNewValue)
{
    SetUserValue(rNewValue);
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

    OUString aStr = ImplGetCurr( GetLocaleDataWrapper(), nNewValue, GetDecimalDigits(), GetCurrencySymbol(), /*UseThousandSep*/true );
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

void LongCurrencyFormatter::SetDecimalDigits( sal_uInt16 nDigits )
{
    if ( nDigits > 9 )
        nDigits = 9;

    mnDecimalDigits = nDigits;
    ReformatAll();
}



LongCurrencyBox::LongCurrencyBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox(pParent, nWinStyle)
    , LongCurrencyFormatter(this)
{
    Reformat();
}

bool LongCurrencyBox::EventNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
    {
        MarkToBeReformatted( false );
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            Reformat();
            ComboBox::Modify();
        }
    }
    return ComboBox::EventNotify( rNEvt );
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
    const sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; ++i )
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
