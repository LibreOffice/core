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

#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <comphelper/string.hxx>


#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <strings.hrc>
#include <svdata.hxx>

#include <i18nutil/unicode.hxx>

#include <rtl/math.hxx>

#include <unotools/localedatawrapper.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

namespace
{

sal_Int64 ImplPower10( sal_uInt16 n )
{
    sal_uInt16  i;
    sal_Int64   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

bool ImplNumericProcessKeyInput( const KeyEvent& rKEvt,
                                 bool bStrictFormat, bool bThousandSep,
                                 const LocaleDataWrapper& rLocaleDataWrapper )
{
    if ( !bStrictFormat )
        return false;
    else
    {
        sal_Unicode cChar = rKEvt.GetCharCode();
        sal_uInt16      nGroup = rKEvt.GetKeyCode().GetGroup();

        return !((nGroup == KEYGROUP_FKEYS) ||
                 (nGroup == KEYGROUP_CURSOR) ||
                 (nGroup == KEYGROUP_MISC) ||
                 ((cChar >= '0') && (cChar <= '9')) ||
                 string::equals(rLocaleDataWrapper.getNumDecimalSep(), cChar) ||
                 (bThousandSep && string::equals(rLocaleDataWrapper.getNumThousandSep(), cChar)) ||
                 string::equals(rLocaleDataWrapper.getNumDecimalSepAlt(), cChar) ||
                 (cChar == '-'));
    }
}

bool ImplNumericGetValue( const OUString& rStr, sal_Int64& rValue,
                                 sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper,
                                 bool bCurrency = false )
{
    OUString            aStr = rStr;
    OUStringBuffer      aStr1, aStr2, aStrFrac, aStrNum, aStrDenom;
    bool                bNegative = false;
    bool                bFrac = false;
    sal_Int32           nDecPos, nFracDivPos, nFracNumPos;
    sal_Int64           nValue;

    // react on empty string
    if ( rStr.isEmpty() )
        return false;

    // remove leading and trailing spaces
    aStr = aStr.trim();


    // find position of decimal point
    nDecPos = aStr.indexOf( rLocaleDataWrapper.getNumDecimalSep() );
    if (nDecPos < 0 && !rLocaleDataWrapper.getNumDecimalSepAlt().isEmpty())
        nDecPos = aStr.indexOf( rLocaleDataWrapper.getNumDecimalSepAlt() );
    // find position of fraction
    nFracDivPos = aStr.indexOf( '/' );

    // parse fractional strings
    if (nFracDivPos > 0)
    {
        bFrac = true;
        nFracNumPos = aStr.lastIndexOf(' ', nFracDivPos);

        // If in "a b/c" format.
        if(nFracNumPos != -1 )
        {
            aStr1.append(std::u16string_view(aStr).substr(0, nFracNumPos));
            aStrNum.append(std::u16string_view(aStr).substr(nFracNumPos+1, nFracDivPos-nFracNumPos-1));
            aStrDenom.append(std::u16string_view(aStr).substr(nFracDivPos+1));
        }
        // "a/b" format, or not a fraction at all
        else
        {
            aStrNum.append(std::u16string_view(aStr).substr(0, nFracDivPos));
            aStrDenom.append(std::u16string_view(aStr).substr(nFracDivPos+1));
        }

    }
    // parse decimal strings
    else if ( nDecPos >= 0)
    {
        aStr1.append(std::u16string_view(aStr).substr(0, nDecPos));
        aStr2.append(std::u16string_view(aStr).substr(nDecPos+1));
    }
    else
        aStr1 = aStr;

    // negative?
    if ( bCurrency )
    {
        if ( aStr.startsWith("(") && aStr.endsWith(")") )
            bNegative = true;
        if ( !bNegative )
        {
            for (sal_Int32 i=0; i < aStr.getLength(); i++ )
            {
                if ( (aStr[i] >= '0') && (aStr[i] <= '9') )
                    break;
                else if ( aStr[i] == '-' )
                {
                    bNegative = true;
                    break;
                }
            }
        }
        if (!bNegative && !aStr.isEmpty())
        {
            sal_uInt16 nFormat = rLocaleDataWrapper.getCurrNegativeFormat();
            if ( (nFormat == 3) || (nFormat == 6)  || // $1- || 1-$
                 (nFormat == 7) || (nFormat == 10) )  // 1$- || 1 $-
            {
                for (sal_Int32 i = aStr.getLength()-1; i > 0; --i )
                {
                    if ( (aStr[i] >= '0') && (aStr[i] <= '9') )
                        break;
                    else if ( aStr[i] == '-' )
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
        if ( !aStr1.isEmpty() && aStr1[0] == '-')
            bNegative = true;
        if ( !aStrNum.isEmpty() && aStrNum[0] == '-') // For non-mixed fractions
            bNegative = true;
    }

    // remove all unwanted characters
    // For whole number
    for (sal_Int32 i=0; i < aStr1.getLength(); )
    {
        if ( (aStr1[i] >= '0') && (aStr1[i] <= '9') )
            i++;
        else
            aStr1.remove( i, 1 );
    }
    // For decimal
    if (!bFrac) {
        for (sal_Int32 i=0; i < aStr2.getLength(); )
        {
            if ((aStr2[i] >= '0') && (aStr2[i] <= '9'))
                ++i;
            else
                aStr2.remove(i, 1);
        }
    }
    else {
        // for numerator
        for (sal_Int32 i=0; i < aStrNum.getLength(); )
        {
            if ((aStrNum[i] >= '0') && (aStrNum[i] <= '9'))
                ++i;
            else
                aStrNum.remove(i, 1);
        }
        // for denominator
        for (sal_Int32 i=0; i < aStrDenom.getLength(); )
        {
            if ((aStrDenom[i] >= '0') && (aStrDenom[i] <= '9'))
                ++i;
            else
                aStrDenom.remove(i, 1);
        }
    }


    if ( !bFrac && aStr1.isEmpty() && aStr2.isEmpty() )
        return false;
    else if ( bFrac && aStr1.isEmpty() && (aStrNum.isEmpty() || aStrDenom.isEmpty()) )
        return false;

    if ( aStr1.isEmpty() )
        aStr1 = "0";
    if ( bNegative )
        aStr1.insert(0, "-");

    // Convert fractional strings
    if (bFrac) {
        // Convert to fraction
        sal_Int64 nWholeNum = aStr1.makeStringAndClear().toInt64();
        sal_Int64 nNum = aStrNum.makeStringAndClear().toInt64();
        sal_Int64 nDenom = aStrDenom.makeStringAndClear().toInt64();
        if (nDenom == 0) return false; // Division by zero
        double nFrac2Dec = nWholeNum + static_cast<double>(nNum)/nDenom; // Convert to double for floating point precision
        aStrFrac.append(nFrac2Dec);
        // Reconvert division result to string and parse
        nDecPos = aStrFrac.indexOf('.');
        if ( nDecPos >= 0)
        {
            aStr1.append(aStrFrac.getStr(), nDecPos);
            aStr2.append(aStrFrac.getStr()+nDecPos+1);
        }
        else
            aStr1 = aStrFrac;
    }

    // prune and round fraction
    bool bRound = false;
    if (aStr2.getLength() > nDecDigits)
    {
        if (aStr2[nDecDigits] >= '5')
            bRound = true;
        string::truncateToLength(aStr2, nDecDigits);
    }
    if (aStr2.getLength() < nDecDigits)
        string::padToLength(aStr2, nDecDigits, '0');

    aStr  = aStr1.makeStringAndClear() + aStr2.makeStringAndClear();

    // check range
    nValue = aStr.toInt64();
    if( nValue == 0 )
    {
        // check if string is equivalent to zero
        sal_Int32 nIndex = bNegative ? 1 : 0;
        while (nIndex < aStr.getLength() && aStr[nIndex] == '0')
            ++nIndex;
        if( nIndex < aStr.getLength() )
        {
            rValue = bNegative ? SAL_MIN_INT64 : SAL_MAX_INT64;
            return true;
        }
    }
    if (bRound)
    {
        if ( !bNegative )
            nValue++;
        else
            nValue--;
    }

    rValue = nValue;

    return true;
}

void ImplUpdateSeparatorString( OUString& io_rText,
                                       const OUString& rOldDecSep, const OUString& rNewDecSep,
                                       const OUString& rOldThSep, const OUString& rNewThSep )
{
    OUStringBuffer aBuf( io_rText.getLength() );
    sal_Int32 nIndexDec = 0, nIndexTh = 0, nIndex = 0;

    const sal_Unicode* pBuffer = io_rText.getStr();
    while( nIndex != -1 )
    {
        nIndexDec = io_rText.indexOf( rOldDecSep, nIndex );
        nIndexTh = io_rText.indexOf( rOldThSep, nIndex );
        if(   (nIndexTh != -1 && nIndexDec != -1 && nIndexTh < nIndexDec )
           || (nIndexTh != -1 && nIndexDec == -1)
           )
        {
            aBuf.append( pBuffer + nIndex, nIndexTh - nIndex );
            aBuf.append( rNewThSep );
            nIndex = nIndexTh + rOldThSep.getLength();
        }
        else if( nIndexDec != -1 )
        {
            aBuf.append( pBuffer + nIndex, nIndexDec - nIndex );
            aBuf.append( rNewDecSep );
            nIndex = nIndexDec + rOldDecSep.getLength();
        }
        else
        {
            aBuf.append( pBuffer + nIndex );
            nIndex = -1;
        }
    }

    io_rText = aBuf.makeStringAndClear();
}

void ImplUpdateSeparators( const OUString& rOldDecSep, const OUString& rNewDecSep,
                                  const OUString& rOldThSep, const OUString& rNewThSep,
                                  Edit* pEdit )
{
    bool bChangeDec = (rOldDecSep != rNewDecSep);
    bool bChangeTh = (rOldThSep != rNewThSep );

    if( bChangeDec || bChangeTh )
    {
        bool bUpdateMode = pEdit->IsUpdateMode();
        pEdit->SetUpdateMode( false );
        OUString aText = pEdit->GetText();
        ImplUpdateSeparatorString( aText, rOldDecSep, rNewDecSep, rOldThSep, rNewThSep );
        pEdit->SetText( aText );

        ComboBox* pCombo = dynamic_cast<ComboBox*>(pEdit);
        if( pCombo )
        {
            // update box entries
            sal_Int32 nEntryCount = pCombo->GetEntryCount();
            for ( sal_Int32 i=0; i < nEntryCount; i++ )
            {
                aText = pCombo->GetEntry( i );
                void* pEntryData = pCombo->GetEntryData( i );
                ImplUpdateSeparatorString( aText, rOldDecSep, rNewDecSep, rOldThSep, rNewThSep );
                pCombo->RemoveEntryAt(i);
                pCombo->InsertEntry( aText, i );
                pCombo->SetEntryData( i, pEntryData );
            }
        }
        if( bUpdateMode )
            pEdit->SetUpdateMode( bUpdateMode );
    }
}

} // namespace

FormatterBase::FormatterBase(Edit* pField)
{
    mpField                     = pField;
    mpLocaleDataWrapper         = nullptr;
    mbReformat                  = false;
    mbStrictFormat              = false;
    mbEmptyFieldValue           = false;
    mbEmptyFieldValueEnabled    = false;
}

FormatterBase::~FormatterBase()
{
}

LocaleDataWrapper& FormatterBase::ImplGetLocaleDataWrapper() const
{
    if ( !mpLocaleDataWrapper )
    {
        const_cast<FormatterBase*>(this)->mpLocaleDataWrapper.reset( new LocaleDataWrapper( GetLanguageTag() ) );
    }
    return *mpLocaleDataWrapper;
}

const LocaleDataWrapper& FormatterBase::GetLocaleDataWrapper() const
{
    return ImplGetLocaleDataWrapper();
}

void FormatterBase::Reformat()
{
}

void FormatterBase::ReformatAll()
{
    Reformat();
};

void FormatterBase::SetStrictFormat( bool bStrict )
{
    if ( bStrict != mbStrictFormat )
    {
        mbStrictFormat = bStrict;
        if ( mbStrictFormat )
            ReformatAll();
    }
}

const lang::Locale& FormatterBase::GetLocale() const
{
    if ( mpField )
        return mpField->GetSettings().GetLanguageTag().getLocale();
    else
        return Application::GetSettings().GetLanguageTag().getLocale();
}

const LanguageTag& FormatterBase::GetLanguageTag() const
{
    if ( mpField )
        return mpField->GetSettings().GetLanguageTag();
    else
        return Application::GetSettings().GetLanguageTag();
}

void FormatterBase::ImplSetText( const OUString& rText, Selection const * pNewSelection )
{
    if ( mpField )
    {
        if (pNewSelection)
            mpField->SetText(rText, *pNewSelection);
        else
        {
            Selection aSel = mpField->GetSelection();
            aSel.Min() = aSel.Max();
            mpField->SetText(rText, aSel);
        }
        MarkToBeReformatted( false );
    }
}

void FormatterBase::SetEmptyFieldValue()
{
    if ( mpField )
        mpField->SetText( OUString() );
    mbEmptyFieldValue = true;
}

bool FormatterBase::IsEmptyFieldValue() const
{
    return (!mpField || mpField->GetText().isEmpty());
}

void NumericFormatter::FormatValue(Selection const * pNewSelection)
{
    mbFormatting = true;
    ImplSetText(CreateFieldText(mnLastValue), pNewSelection);
    mbFormatting = false;
}

void NumericFormatter::ImplNumericReformat()
{
    mnLastValue = GetValue();
    FormatValue();
}

void NumericFormatter::ImplInit()
{
    mnFieldValue        = 0;
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = SAL_MAX_INT32;
        // a "large" value substantially smaller than SAL_MAX_INT64, to avoid
        // overflow in computations using this "dummy" value
    mnDecimalDigits     = 2;
    mbThousandSep       = true;
    mbShowTrailingZeros = true;
    mbWrapOnLimits      = false;
    mbFormatting       = false;

    // for fields
    mnSpinSize          = 1;
    mnFirst             = mnMin;
    mnLast              = mnMax;

    SetDecimalDigits( 0 );
}

NumericFormatter::NumericFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
{
    ImplInit();
}

NumericFormatter::~NumericFormatter()
{
}

void NumericFormatter::SetMin( sal_Int64 nNewMin )
{
    mnMin = nNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void NumericFormatter::SetMax( sal_Int64 nNewMax )
{
    mnMax = nNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void NumericFormatter::SetUseThousandSep( bool bValue )
{
    mbThousandSep = bValue;
    ReformatAll();
}

void NumericFormatter::SetDecimalDigits( sal_uInt16 nDigits )
{
    mnDecimalDigits = nDigits;
    ReformatAll();
}

void NumericFormatter::SetShowTrailingZeros( bool bShowTrailingZeros )
{
    if ( mbShowTrailingZeros != bShowTrailingZeros )
    {
        mbShowTrailingZeros = bShowTrailingZeros;
        ReformatAll();
    }
}


void NumericFormatter::SetValue( sal_Int64 nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( false );
}

OUString NumericFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return ImplGetLocaleDataWrapper().getNum( nValue, GetDecimalDigits(), IsUseThousandSep(), IsShowTrailingZeros() );
}

void NumericFormatter::ImplSetUserValue( sal_Int64 nNewValue, Selection const * pNewSelection )
{
    nNewValue = ClipAgainstMinMax(nNewValue);
    mnLastValue = nNewValue;

    if ( GetField() )
        FormatValue(pNewSelection);
}

void NumericFormatter::SetUserValue( sal_Int64 nNewValue )
{
    ImplSetUserValue( nNewValue );
}

sal_Int64 NumericFormatter::GetValueFromString(const OUString& rStr) const
{
    sal_Int64 nTempValue;

    if (ImplNumericGetValue(rStr, nTempValue,
        GetDecimalDigits(), ImplGetLocaleDataWrapper()))
    {
        return ClipAgainstMinMax(nTempValue);
    }
    else
        return mnLastValue;
}

sal_Int64 NumericFormatter::GetValue() const
{
    if (mbFormatting) //don't parse the entry if we're currently formatting what to put in it
        return mnLastValue;

    return GetField() ? GetValueFromString(GetField()->GetText()) : 0;
}

bool NumericFormatter::IsValueModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyFieldValue();
    else if ( GetValue() != mnFieldValue )
        return true;
    else
        return false;
}

sal_Int64 NumericFormatter::Normalize( sal_Int64 nValue ) const
{
    return (nValue * ImplPower10( GetDecimalDigits() ) );
}

sal_Int64 NumericFormatter::Denormalize( sal_Int64 nValue ) const
{
    sal_Int64 nFactor = ImplPower10( GetDecimalDigits() );

    if ((nValue < ( SAL_MIN_INT64 + nFactor )) ||
        (nValue > ( SAL_MAX_INT64 - nFactor )))
    {
        return ( nValue / nFactor );
    }

    if( nValue < 0 )
    {
        sal_Int64 nHalf = nFactor / 2;
        return ((nValue - nHalf) / nFactor );
    }
    else
    {
        sal_Int64 nHalf = nFactor / 2;
        return ((nValue + nHalf) / nFactor );
    }
}

void NumericFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( GetField()->GetText().isEmpty() && ImplGetEmptyFieldValue() )
        return;

    ImplNumericReformat();
}

void NumericFormatter::FieldUp()
{
    sal_Int64 nValue = GetValue();
    sal_Int64 nRemainder = nValue % mnSpinSize;
    if (nValue >= 0)
        nValue = (nRemainder == 0) ? nValue + mnSpinSize : nValue + mnSpinSize - nRemainder;
    else
        nValue = (nRemainder == 0) ? nValue + mnSpinSize : nValue - nRemainder;

    nValue = ClipAgainstMinMax(nValue);

    ImplNewFieldValue( nValue );
}

void NumericFormatter::FieldDown()
{
    sal_Int64 nValue = GetValue();
    sal_Int64 nRemainder = nValue % mnSpinSize;
    if (nValue >= 0)
        nValue = (nRemainder == 0) ? nValue - mnSpinSize : nValue - nRemainder;
    else
        nValue = (nRemainder == 0) ? nValue - mnSpinSize : nValue - mnSpinSize - nRemainder;

    nValue = ClipAgainstMinMax(nValue);

    ImplNewFieldValue( nValue );
}

void NumericFormatter::FieldFirst()
{
    ImplNewFieldValue( mnFirst );
}

void NumericFormatter::FieldLast()
{
    ImplNewFieldValue( mnLast );
}

void NumericFormatter::ImplNewFieldValue( sal_Int64 nNewValue )
{
    if ( GetField() )
    {
        // !!! We should check why we do not validate in ImplSetUserValue() if the value was
        // changed. This should be done there as well since otherwise the call to Modify would not
        // be allowed. Anyway, the paths from ImplNewFieldValue, ImplSetUserValue, and ImplSetText
        // should be checked and clearly traced (with comment) in order to find out what happens.

        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        OUString aText = GetField()->GetText();
        // leave it as is if selected until end
        if ( static_cast<sal_Int32>(aSelection.Max()) == aText.getLength() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        sal_Int64 nOldLastValue  = mnLastValue;
        ImplSetUserValue( nNewValue, &aSelection );
        mnLastValue = nOldLastValue;

        // Modify during Edit is only set during KeyInput
        if ( GetField()->GetText() != aText )
        {
            GetField()->SetModifyFlag();
            GetField()->Modify();
        }
    }
}

sal_Int64 NumericFormatter::ClipAgainstMinMax(sal_Int64 nValue) const
{
    if (nValue > mnMax)
        nValue = mbWrapOnLimits ? ((nValue - mnMin) % (mnMax + 1)) + mnMin
                                : mnMax;
    else if (nValue < mnMin)
        nValue = mbWrapOnLimits ? ((nValue + mnMax + 1 - mnMin) % (mnMax + 1)) + mnMin
                                : mnMin;
    return nValue;
}

NumericField::NumericField(vcl::Window* pParent, WinBits nWinStyle)
    : SpinField(pParent, nWinStyle)
    , NumericFormatter(this)
{
    Reformat();
}

void NumericField::dispose()
{
    ClearField();
    SpinField::dispose();
}

bool NumericField::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "digits")
        SetDecimalDigits(rValue.toInt32());
    else if (rKey == "spin-size")
        SetSpinSize(rValue.toInt32());
    else if (rKey == "wrap")
        mbWrapOnLimits = toBool(rValue);
    else
        return SpinField::set_property(rKey, rValue);
    return true;
}

bool NumericField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplNumericProcessKeyInput( *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool NumericField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::EventNotify( rNEvt );
}

void NumericField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void NumericField::Modify()
{
    MarkToBeReformatted( true );
    SpinField::Modify();
}

void NumericField::Up()
{
    FieldUp();
    SpinField::Up();
}

void NumericField::Down()
{
    FieldDown();
    SpinField::Down();
}

void NumericField::First()
{
    FieldFirst();
    SpinField::First();
}

void NumericField::Last()
{
    FieldLast();
    SpinField::Last();
}

namespace
{
    Size calcMinimumSize(const Edit &rSpinField, const NumericFormatter &rFormatter)
    {
        OUStringBuffer aBuf;
        sal_Int32 nTextLen;

        nTextLen = OUString::number(rFormatter.GetMin()).getLength();
        string::padToLength(aBuf, nTextLen, '9');
        Size aMinTextSize = rSpinField.CalcMinimumSizeForText(
            rFormatter.CreateFieldText(aBuf.makeStringAndClear().toInt64()));

        nTextLen = OUString::number(rFormatter.GetMax()).getLength();
        string::padToLength(aBuf, nTextLen, '9');
        Size aMaxTextSize = rSpinField.CalcMinimumSizeForText(
            rFormatter.CreateFieldText(aBuf.makeStringAndClear().toInt64()));

        Size aRet(std::max(aMinTextSize.Width(), aMaxTextSize.Width()),
                  std::max(aMinTextSize.Height(), aMaxTextSize.Height()));

        OUStringBuffer sBuf("999999999");
        sal_uInt16 nDigits = rFormatter.GetDecimalDigits();
        if (nDigits)
        {
            sBuf.append('.');
            string::padToLength(aBuf, aBuf.getLength() + nDigits, '9');
        }
        aMaxTextSize = rSpinField.CalcMinimumSizeForText(sBuf.makeStringAndClear());
        aRet.setWidth( std::min(aRet.Width(), aMaxTextSize.Width()) );

        return aRet;
    }
}

Size NumericField::CalcMinimumSize() const
{
    return calcMinimumSize(*this, *this);
}

NumericBox::NumericBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox(pParent, nWinStyle)
    , NumericFormatter(this)
{
    Reformat();
    if ( !(nWinStyle & WB_HIDE ) )
        Show();
}

void NumericBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

Size NumericBox::CalcMinimumSize() const
{
    Size aRet(calcMinimumSize(*this, *this));

    if (IsDropDownBox())
    {
        Size aComboSugg(ComboBox::CalcMinimumSize());
        aRet.setWidth( std::max(aRet.Width(), aComboSugg.Width()) );
        aRet.setHeight( std::max(aRet.Height(), aComboSugg.Height()) );
    }

    return aRet;
}

bool NumericBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplNumericProcessKeyInput( *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool NumericBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::EventNotify( rNEvt );
}

void NumericBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void NumericBox::Modify()
{
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void NumericBox::ImplNumericReformat( const OUString& rStr, sal_Int64& rValue,
                                                OUString& rOutStr )
{
    if (ImplNumericGetValue(rStr, rValue, GetDecimalDigits(), ImplGetLocaleDataWrapper()))
    {
        sal_Int64 nTempVal = ClipAgainstMinMax(rValue);
        rOutStr = CreateFieldText( nTempVal );
    }
}

void NumericBox::ReformatAll()
{
    sal_Int64 nValue;
    OUString aStr;
    SetUpdateMode( false );
    sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; i++ )
    {
        ImplNumericReformat( GetEntry( i ), nValue, aStr );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    NumericFormatter::Reformat();
    SetUpdateMode( true );
}

void NumericBox::InsertValue( sal_Int64 nValue, sal_Int32 nPos )
{
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

static bool ImplMetricProcessKeyInput( const KeyEvent& rKEvt,
                                       bool bUseThousandSep, const LocaleDataWrapper& rWrapper )
{
    // no meaningful strict format; therefore allow all characters
    return ImplNumericProcessKeyInput( rKEvt, false, bUseThousandSep, rWrapper );
}

static OUString ImplMetricGetUnitText(const OUString& rStr)
{
    // fetch unit text
    OUStringBuffer aStr;
    for (sal_Int32 i = rStr.getLength()-1; i >= 0; --i)
    {
        sal_Unicode c = rStr[i];
        if ( (c == '\'') || (c == '\"') || (c == '%' ) || unicode::isAlpha(c) || unicode::isControl(c) )
            aStr.insert(0, c);
        else
        {
            if (!aStr.isEmpty())
                break;
        }
    }
    return aStr.makeStringAndClear();
}

// #104355# support localized measurements

static const OUString ImplMetricToString( FieldUnit rUnit )
{
    // return unit's default string (ie, the first one )
    for (auto const& elem : ImplGetFieldUnits())
    {
        if (elem.second == rUnit)
            return elem.first;
    }

    return OUString();
}

FieldUnit MetricFormatter::StringToMetric(const OUString &rMetricString)
{
    // return FieldUnit
    OUString aStr = rMetricString.toAsciiLowerCase().replaceAll(" ", "");
    for (auto const& elem : ImplGetCleanedFieldUnits())
    {
        if ( elem.first == aStr )
            return elem.second;
    }

    return FieldUnit::NONE;
}

static FieldUnit ImplMetricGetUnit(const OUString& rStr)
{
    OUString aStr = ImplMetricGetUnitText(rStr);
    return MetricFormatter::StringToMetric(aStr);
}

#define K *1000L
#define M *1000000LL
#define X *5280L

// twip in km = 254 / 14 400 000 000
// expressions too big for default size 32 bit need LL to avoid overflow

static const sal_Int64 aImplFactor[sal_uInt16(FieldUnit::LINE) + 1]
                                  [sal_uInt16(FieldUnit::LINE) + 1] =
{ /*
mm/100    mm    cm       m     km  twip point  pica  inch    foot       mile     char     line  */
{    1,  100,  1 K,  100 K, 100 M, 2540, 2540, 2540, 2540,2540*12,2540*12 X ,   53340, 396240},
{    1,    1,   10,    1 K,   1 M, 2540, 2540, 2540, 2540,2540*12,2540*12 X ,    5334, 396240},
{    1,    1,    1,    100, 100 K,  254,  254,  254,  254, 254*12, 254*12 X ,    5334,  39624},
{    1,    1,    1,      1,   1 K,  254,  254,  254,  254, 254*12, 254*12 X ,  533400,  39624},
{    1,    1,    1,      1,     1,  254,  254,  254,  254, 254*12, 254*12 X ,533400 K,  39624},
{ 1440,144 K,144 K,14400 K,14400LL M, 1,   20,  240, 1440,1440*12,1440*12 X ,     210,   3120},
{   72, 7200, 7200,  720 K, 720 M,    1,    1,   12,   72,  72*12,  72*12 X ,     210,    156},
{    6,  600,  600,   60 K,  60 M,    1,    1,    1,    6,   6*12,   6*12 X ,     210,     10},
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,     12,     12 X ,     210,     45},
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,      1,      1 X ,     210,     45},
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,      1,        1 ,     210,     45},
{  144, 1440,14400,  14400, 14400,    1,   20,  240, 1440,1440*12, 1440*12 X,       1,   156 },
{  720,72000,72000, 7200 K,7200LL M, 20,   10,   13,   11,  11*12,   11*12 X,     105,     1 }
};
#undef X
#undef M
#undef K

static FieldUnit eDefaultUnit = FieldUnit::NONE;

FieldUnit MetricField::GetDefaultUnit() { return eDefaultUnit; }
void MetricField::SetDefaultUnit( FieldUnit meUnit ) { eDefaultUnit = meUnit; }

static FieldUnit ImplMap2FieldUnit( MapUnit meUnit, long& nDecDigits )
{
    switch( meUnit )
    {
        case MapUnit::Map100thMM :
            nDecDigits -= 2;
            return FieldUnit::MM;
        case MapUnit::Map10thMM :
            nDecDigits -= 1;
            return FieldUnit::MM;
        case MapUnit::MapMM :
            return FieldUnit::MM;
        case MapUnit::MapCM :
            return FieldUnit::CM;
        case MapUnit::Map1000thInch :
            nDecDigits -= 3;
            return FieldUnit::INCH;
        case MapUnit::Map100thInch :
            nDecDigits -= 2;
            return FieldUnit::INCH;
        case MapUnit::Map10thInch :
            nDecDigits -= 1;
            return FieldUnit::INCH;
        case MapUnit::MapInch :
            return FieldUnit::INCH;
        case MapUnit::MapPoint :
            return FieldUnit::POINT;
        case MapUnit::MapTwip :
            return FieldUnit::TWIP;
        default:
            OSL_FAIL( "default eInUnit" );
            break;
    }
    return FieldUnit::NONE;
}

static double nonValueDoubleToValueDouble( double nValue )
{
    return rtl::math::isFinite( nValue ) ? nValue : 0.0;
}

sal_Int64 MetricField::ConvertValue( sal_Int64 nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                     FieldUnit eInUnit, FieldUnit eOutUnit )
{
    double nDouble = nonValueDoubleToValueDouble( ConvertDoubleValue(
                static_cast<double>(nValue), mnBaseValue, nDecDigits, eInUnit, eOutUnit ) );
    sal_Int64 nLong ;

    // caution: precision loss in double cast
    if ( nDouble <= double(SAL_MIN_INT64) )
        nLong = SAL_MIN_INT64;
    else if ( nDouble >= double(SAL_MAX_INT64) )
        nLong = SAL_MAX_INT64;
    else
        nLong = static_cast<sal_Int64>( nDouble );

    return nLong;
}

sal_Int64 MetricField::ConvertValue( sal_Int64 nValue, sal_uInt16 nDigits,
                                     MapUnit eInUnit, FieldUnit eOutUnit )
{
    return static_cast<sal_Int64>(
        nonValueDoubleToValueDouble(
            ConvertDoubleValue( nValue, nDigits, eInUnit, eOutUnit ) ) );
}

double MetricField::ConvertDoubleValue( double nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                        FieldUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eInUnit != eOutUnit )
    {
        sal_Int64 nMult = 1, nDiv = 1;

        if (eInUnit == FieldUnit::PERCENT)
        {
            if ( (mnBaseValue <= 0) || (nValue <= 0) )
                return nValue;
            nDiv = 100 * ImplPower10(nDecDigits);

            nMult = mnBaseValue;
        }
        else if ( eOutUnit == FieldUnit::PERCENT ||
                  eOutUnit == FieldUnit::CUSTOM ||
                  eOutUnit == FieldUnit::NONE ||
                  eOutUnit == FieldUnit::DEGREE ||
                  eOutUnit == FieldUnit::SECOND ||
                  eOutUnit == FieldUnit::MILLISECOND ||
                  eOutUnit == FieldUnit::PIXEL ||
                  eInUnit  == FieldUnit::CUSTOM ||
                  eInUnit  == FieldUnit::NONE ||
                  eInUnit  == FieldUnit::DEGREE ||
                  eInUnit  == FieldUnit::MILLISECOND ||
                  eInUnit  == FieldUnit::PIXEL )
             return nValue;
        else
        {
            if (eOutUnit == FieldUnit::MM_100TH)
                eOutUnit = FieldUnit::NONE;
            if (eInUnit == FieldUnit::MM_100TH)
                eInUnit = FieldUnit::NONE;

            nDiv  = aImplFactor[sal_uInt16(eInUnit)][sal_uInt16(eOutUnit)];
            nMult = aImplFactor[sal_uInt16(eOutUnit)][sal_uInt16(eInUnit)];

            SAL_WARN_IF( nMult <= 0, "vcl", "illegal *" );
            SAL_WARN_IF( nDiv  <= 0, "vcl", "illegal /" );
        }

        if ( nMult != 1 && nMult > 0 )
            nValue *= nMult;
        if ( nDiv != 1 && nDiv > 0 )
        {
            nValue += ( nValue < 0 ) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }

    return nValue;
}

double MetricField::ConvertDoubleValue( double nValue, sal_uInt16 nDigits,
                                        MapUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eOutUnit == FieldUnit::PERCENT ||
         eOutUnit == FieldUnit::CUSTOM ||
         eOutUnit == FieldUnit::NONE ||
         eInUnit == MapUnit::MapPixel ||
         eInUnit == MapUnit::MapSysFont ||
         eInUnit == MapUnit::MapAppFont ||
         eInUnit == MapUnit::MapRelative )
    {
        OSL_FAIL( "invalid parameters" );
        return nValue;
    }

    long nDecDigits = nDigits;
    FieldUnit eFieldUnit = ImplMap2FieldUnit( eInUnit, nDecDigits );

    if ( nDecDigits < 0 )
    {
        while ( nDecDigits )
        {
            nValue += 5;
            nValue /= 10;
            nDecDigits++;
        }
    }
    else
    {
        nValue *= ImplPower10(nDecDigits);
    }

    if ( eFieldUnit != eOutUnit )
    {
        sal_Int64 nDiv  = aImplFactor[sal_uInt16(eFieldUnit)][sal_uInt16(eOutUnit)];
        sal_Int64 nMult = aImplFactor[sal_uInt16(eOutUnit)][sal_uInt16(eFieldUnit)];

        SAL_WARN_IF( nMult <= 0, "vcl", "illegal *" );
        SAL_WARN_IF( nDiv  <= 0, "vcl", "illegal /" );

        if ( nMult != 1 && nMult > 0)
            nValue *= nMult;
        if ( nDiv != 1 && nDiv > 0 )
        {
            nValue += (nValue < 0) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }
    return nValue;
}

double MetricField::ConvertDoubleValue( double nValue, sal_uInt16 nDigits,
                                        FieldUnit eInUnit, MapUnit eOutUnit )
{
    if ( eInUnit == FieldUnit::PERCENT ||
         eInUnit == FieldUnit::CUSTOM ||
         eInUnit == FieldUnit::NONE ||
         eInUnit == FieldUnit::DEGREE ||
         eInUnit == FieldUnit::SECOND ||
         eInUnit == FieldUnit::MILLISECOND ||
         eInUnit == FieldUnit::PIXEL ||
         eOutUnit == MapUnit::MapPixel ||
         eOutUnit == MapUnit::MapSysFont ||
         eOutUnit == MapUnit::MapAppFont ||
         eOutUnit == MapUnit::MapRelative )
    {
        OSL_FAIL( "invalid parameters" );
        return nValue;
    }

    long nDecDigits = nDigits;
    FieldUnit eFieldUnit = ImplMap2FieldUnit( eOutUnit, nDecDigits );

    if ( nDecDigits < 0 )
    {
        nValue *= ImplPower10(-nDecDigits);
    }
    else
    {
        nValue /= ImplPower10(nDecDigits);
    }

    if ( eFieldUnit != eInUnit )
    {
        sal_Int64 nDiv  = aImplFactor[sal_uInt16(eInUnit)][sal_uInt16(eFieldUnit)];
        sal_Int64 nMult = aImplFactor[sal_uInt16(eFieldUnit)][sal_uInt16(eInUnit)];

        SAL_WARN_IF( nMult <= 0, "vcl", "illegal *" );
        SAL_WARN_IF( nDiv  <= 0, "vcl", "illegal /" );

        if( nMult != 1 && nMult > 0 )
            nValue *= nMult;
        if( nDiv != 1 && nDiv > 0 )
        {
            nValue += (nValue < 0) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }
    return nValue;
}

bool MetricFormatter::TextToValue(const OUString& rStr, double& rValue, sal_Int64 nBaseValue,
                                  sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper, FieldUnit eUnit)
{
    // Get value
    sal_Int64 nValue;
    if ( !ImplNumericGetValue( rStr, nValue, nDecDigits, rLocaleDataWrapper ) )
        return false;

    // Determine unit
    FieldUnit eEntryUnit = ImplMetricGetUnit( rStr );

    // Recalculate unit
    // caution: conversion to double loses precision
    rValue = MetricField::ConvertDoubleValue( static_cast<double>(nValue), nBaseValue, nDecDigits, eEntryUnit, eUnit );

    return true;
}

void MetricFormatter::ImplMetricReformat( const OUString& rStr, double& rValue, OUString& rOutStr )
{
    if ( !TextToValue( rStr, rValue, mnBaseValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit ) )
        return;

    double nTempVal = rValue;
    // caution: precision loss in double cast
    if ( nTempVal > GetMax() )
        nTempVal = static_cast<double>(GetMax());
    else if ( nTempVal < GetMin())
        nTempVal = static_cast<double>(GetMin());
    rOutStr = CreateFieldText( static_cast<sal_Int64>(nTempVal) );
}

inline void MetricFormatter::ImplInit()
{
    mnBaseValue = 0;
    meUnit = MetricField::GetDefaultUnit();
}

MetricFormatter::MetricFormatter(Edit* pEdit)
    : NumericFormatter(pEdit)
{
    ImplInit();
}

MetricFormatter::~MetricFormatter()
{
}

void MetricFormatter::SetUnit( FieldUnit eNewUnit )
{
    if (eNewUnit == FieldUnit::MM_100TH)
    {
        SetDecimalDigits( GetDecimalDigits() + 2 );
        meUnit = FieldUnit::MM;
    }
    else
        meUnit = eNewUnit;
    ReformatAll();
}

void MetricFormatter::SetCustomUnitText( const OUString& rStr )
{
    maCustomUnitText = rStr;
    ReformatAll();
}

void MetricFormatter::SetValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    SetUserValue( nNewValue, eInUnit );
    mnFieldValue = mnLastValue;
}

OUString MetricFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    //whether percent is separated from its number is locale
    //specific, pawn it off to icu to decide
    if (meUnit == FieldUnit::PERCENT)
    {
        double dValue = nValue;
        dValue /= ImplPower10(GetDecimalDigits());
        return unicode::formatPercent(dValue, GetLanguageTag());
    }

    OUString aStr = NumericFormatter::CreateFieldText( nValue );

    if( meUnit == FieldUnit::CUSTOM )
        aStr += maCustomUnitText;
    else
    {
        if (meUnit != FieldUnit::NONE && meUnit != FieldUnit::DEGREE)
            aStr += " ";
        assert(meUnit != FieldUnit::PERCENT);
        aStr += ImplMetricToString( meUnit );
    }
    return aStr;
}

void MetricFormatter::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    // convert to previously configured units
    nNewValue = MetricField::ConvertValue( nNewValue, mnBaseValue, GetDecimalDigits(), eInUnit, meUnit );
    NumericFormatter::SetUserValue( nNewValue );
}

sal_Int64 MetricFormatter::GetValueFromStringUnit(const OUString& rStr, FieldUnit eOutUnit) const
{
    double nTempValue;
    // caution: precision loss in double cast
    if (!TextToValue(rStr, nTempValue, mnBaseValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit))
        nTempValue = static_cast<double>(mnLastValue);

    // caution: precision loss in double cast
    if (nTempValue > mnMax)
        nTempValue = static_cast<double>(mnMax);
    else if (nTempValue < mnMin)
        nTempValue = static_cast<double>(mnMin);

    // convert to requested units
    return MetricField::ConvertValue(static_cast<sal_Int64>(nTempValue), mnBaseValue, GetDecimalDigits(), meUnit, eOutUnit);
}

sal_Int64 MetricFormatter::GetValueFromString(const OUString& rStr) const
{
    return GetValueFromStringUnit(rStr, FieldUnit::NONE);
}

sal_Int64 MetricFormatter::GetValue( FieldUnit eOutUnit ) const
{
    return GetField() ? GetValueFromStringUnit(GetField()->GetText(), eOutUnit) : 0;
}

void MetricFormatter::SetValue( sal_Int64 nValue )
{
    // Implementation not inline, because it is a virtual Function
    SetValue( nValue, FieldUnit::NONE );
}

void MetricFormatter::SetMin( sal_Int64 nNewMin, FieldUnit eInUnit )
{
    // convert to requested units
    NumericFormatter::SetMin( MetricField::ConvertValue( nNewMin, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

sal_Int64 MetricFormatter::GetMin( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( NumericFormatter::GetMin(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

void MetricFormatter::SetMax( sal_Int64 nNewMax, FieldUnit eInUnit )
{
    // convert to requested units
    NumericFormatter::SetMax( MetricField::ConvertValue( nNewMax, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

sal_Int64 MetricFormatter::GetMax( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( NumericFormatter::GetMax(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

void MetricFormatter::SetBaseValue( sal_Int64 nNewBase, FieldUnit eInUnit )
{
    mnBaseValue = MetricField::ConvertValue( nNewBase, mnBaseValue, GetDecimalDigits(),
                                             eInUnit, meUnit );
}

sal_Int64 MetricFormatter::GetBaseValue() const
{
    // convert to requested units
    return MetricField::ConvertValue( mnBaseValue, mnBaseValue, GetDecimalDigits(),
                                      meUnit, FieldUnit::NONE );
}

void MetricFormatter::Reformat()
{
    if ( !GetField() )
        return;

    OUString aText = GetField()->GetText();
    if ( meUnit == FieldUnit::CUSTOM )
        maCurUnitText = ImplMetricGetUnitText( aText );

    OUString aStr;
    // caution: precision loss in double cast
    double nTemp = static_cast<double>(mnLastValue);
    ImplMetricReformat( aText, nTemp, aStr );
    mnLastValue = static_cast<sal_Int64>(nTemp);

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        if ( meUnit == FieldUnit::CUSTOM )
            CustomConvert();
    }
    else
        SetValue( mnLastValue );
    maCurUnitText.clear();
}

sal_Int64 MetricFormatter::GetCorrectedValue( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( 0/*nCorrectedValue*/, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

MetricField::MetricField(vcl::Window* pParent, WinBits nWinStyle)
    : SpinField(pParent, nWinStyle)
    , MetricFormatter(this)
{
    Reformat();
}

void MetricField::dispose()
{
    ClearField();
    SpinField::dispose();
}

Size MetricField::CalcMinimumSize() const
{
    return calcMinimumSize(*this, *this);
}

bool MetricField::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "digits")
        SetDecimalDigits(rValue.toInt32());
    else if (rKey == "spin-size")
        SetSpinSize(rValue.toInt32());
    else
        return SpinField::set_property(rKey, rValue);
    return true;
}

void MetricField::SetUnit( FieldUnit nNewUnit )
{
    sal_Int64 nRawMax = GetMax( nNewUnit );
    sal_Int64 nMax = Denormalize( nRawMax );
    sal_Int64 nMin = Denormalize( GetMin( nNewUnit ) );
    sal_Int64 nFirst = Denormalize( GetFirst( nNewUnit ) );
    sal_Int64 nLast = Denormalize( GetLast( nNewUnit ) );

    MetricFormatter::SetUnit( nNewUnit );

    SetMax( Normalize( nMax ), nNewUnit );
    SetMin( Normalize( nMin ), nNewUnit );
    SetFirst( Normalize( nFirst ), nNewUnit );
    SetLast( Normalize( nLast ), nNewUnit );
}

void MetricField::SetFirst( sal_Int64 nNewFirst, FieldUnit eInUnit )
{
    // convert
    nNewFirst = MetricField::ConvertValue( nNewFirst, mnBaseValue, GetDecimalDigits(),
                                           eInUnit, meUnit );
    mnFirst = nNewFirst;
}

sal_Int64 MetricField::GetFirst( FieldUnit eOutUnit ) const
{
    // convert
    return MetricField::ConvertValue( mnFirst, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

void MetricField::SetLast( sal_Int64 nNewLast, FieldUnit eInUnit )
{
    // convert
    nNewLast = MetricField::ConvertValue( nNewLast, mnBaseValue, GetDecimalDigits(),
                                          eInUnit, meUnit );
    mnLast = nNewLast;
}

sal_Int64 MetricField::GetLast( FieldUnit eOutUnit ) const
{
    // conver
    return MetricField::ConvertValue( mnLast, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

bool MetricField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplMetricProcessKeyInput( *rNEvt.GetKeyEvent(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool MetricField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::EventNotify( rNEvt );
}

void MetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void MetricField::Modify()
{
    MarkToBeReformatted( true );
    SpinField::Modify();
}

void MetricField::Up()
{
    FieldUp();
    SpinField::Up();
}

void MetricField::Down()
{
    FieldDown();
    SpinField::Down();
}

void MetricField::First()
{
    FieldFirst();
    SpinField::First();
}

void MetricField::Last()
{
    FieldLast();
    SpinField::Last();
}

void MetricField::CustomConvert()
{
    maCustomConvertLink.Call( *this );
}

MetricBox::MetricBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox(pParent, nWinStyle)
    , MetricFormatter(this)
{
    Reformat();
}

void MetricBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

Size MetricBox::CalcMinimumSize() const
{
    Size aRet(calcMinimumSize(*this, *this));

    if (IsDropDownBox())
    {
        Size aComboSugg(ComboBox::CalcMinimumSize());
        aRet.setWidth( std::max(aRet.Width(), aComboSugg.Width()) );
        aRet.setHeight( std::max(aRet.Height(), aComboSugg.Height()) );
    }

    return aRet;
}

bool MetricBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2()  )
    {
        if ( ImplMetricProcessKeyInput( *rNEvt.GetKeyEvent(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool MetricBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::EventNotify( rNEvt );
}

void MetricBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void MetricBox::Modify()
{
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void MetricBox::ReformatAll()
{
    double nValue;
    OUString aStr;
    SetUpdateMode( false );
    sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; i++ )
    {
        ImplMetricReformat( GetEntry( i ), nValue, aStr );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    MetricFormatter::Reformat();
    SetUpdateMode( true );
}

void MetricBox::CustomConvert()
{
    maCustomConvertLink.Call( *this );
}

void MetricBox::InsertValue( sal_Int64 nValue, FieldUnit eInUnit, sal_Int32 nPos )
{
    // convert to previously configured units
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

static bool ImplCurrencyProcessKeyInput( const KeyEvent& rKEvt,
                                         bool bUseThousandSep, const LocaleDataWrapper& rWrapper )
{
    // no strict format set; therefore allow all characters
    return ImplNumericProcessKeyInput( rKEvt, false, bUseThousandSep, rWrapper );
}

static bool ImplCurrencyGetValue( const OUString& rStr, sal_Int64& rValue,
                                  sal_uInt16 nDecDigits, const LocaleDataWrapper& rWrapper )
{
    // fetch number
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rWrapper, true );
}

void CurrencyFormatter::ImplCurrencyReformat( const OUString& rStr, OUString& rOutStr )
{
    sal_Int64 nValue;
    if ( !ImplNumericGetValue( rStr, nValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), true ) )
        return;

    sal_Int64 nTempVal = nValue;
    if ( nTempVal > GetMax() )
        nTempVal = GetMax();
    else if ( nTempVal < GetMin())
        nTempVal = GetMin();
    rOutStr = CreateFieldText( nTempVal );
}

CurrencyFormatter::CurrencyFormatter(Edit* pField)
    : NumericFormatter(pField)
{
}

CurrencyFormatter::~CurrencyFormatter()
{
}

void CurrencyFormatter::SetValue( sal_Int64 nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( false );
}

OUString CurrencyFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return ImplGetLocaleDataWrapper().getCurr( nValue, GetDecimalDigits(),
                                               ImplGetLocaleDataWrapper().getCurrSymbol(),
                                               IsUseThousandSep() );
}

sal_Int64 CurrencyFormatter::GetValueFromString(const OUString& rStr) const
{
    sal_Int64 nTempValue;
    if ( ImplCurrencyGetValue( rStr, nTempValue, GetDecimalDigits(), ImplGetLocaleDataWrapper() ) )
    {
        return ClipAgainstMinMax(nTempValue);
    }
    else
        return mnLastValue;
}

void CurrencyFormatter::Reformat()
{
    if ( !GetField() )
        return;

    OUString aStr;
    ImplCurrencyReformat( GetField()->GetText(), aStr );

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr  );
        sal_Int64 nTemp = mnLastValue;
        ImplCurrencyGetValue( aStr, nTemp, GetDecimalDigits(), ImplGetLocaleDataWrapper() );
        mnLastValue = nTemp;
    }
    else
        SetValue( mnLastValue );
}

CurrencyField::CurrencyField(vcl::Window* pParent, WinBits nWinStyle)
    : SpinField(pParent, nWinStyle)
    , CurrencyFormatter(this)
{
    Reformat();
}

void CurrencyField::dispose()
{
    ClearField();
    SpinField::dispose();
}

bool CurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplCurrencyProcessKeyInput( *rNEvt.GetKeyEvent(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool CurrencyField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::EventNotify( rNEvt );
}

void CurrencyField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void CurrencyField::Modify()
{
    MarkToBeReformatted( true );
    SpinField::Modify();
}

void CurrencyField::Up()
{
    FieldUp();
    SpinField::Up();
}

void CurrencyField::Down()
{
    FieldDown();
    SpinField::Down();
}

void CurrencyField::First()
{
    FieldFirst();
    SpinField::First();
}

void CurrencyField::Last()
{
    FieldLast();
    SpinField::Last();
}

CurrencyBox::CurrencyBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox(pParent, nWinStyle)
    , CurrencyFormatter(this)
{
    Reformat();
}

void CurrencyBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

bool CurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplCurrencyProcessKeyInput( *rNEvt.GetKeyEvent(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool CurrencyBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::EventNotify( rNEvt );
}

void CurrencyBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        OUString sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        OUString sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        OUString sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

void CurrencyBox::Modify()
{
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void CurrencyBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( false );
    sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; i++ )
    {
        ImplCurrencyReformat( GetEntry( i ), aStr );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    CurrencyFormatter::Reformat();
    SetUpdateMode( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
