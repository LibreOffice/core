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

#include <osl/diagnose.hxx>
#include <comphelper/string.hxx>
#include <tools/json_writer.hxx>
#include <tools/mapunit.hxx>
#include <unotools/localedatawrapper.hxx>
#include <o3tl/string_view.hxx>
#include <i18nutil/unicode.hxx>

#include <vcl/event.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/uitest/metricfielduiobject.hxx>

#include <svdata.hxx>

#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <string_view>

namespace
{

std::string FieldUnitToString(FieldUnit unit)
{
    switch(unit)
    {
        case FieldUnit::NONE:
            return "";

        case FieldUnit::MM:
            return "mm";

        case FieldUnit::CM:
            return "cm";

        case FieldUnit::M:
            return "m";

        case FieldUnit::KM:
            return "km";

        case FieldUnit::TWIP:
            return "twip";

        case FieldUnit::POINT:
            return "point";

        case FieldUnit::PICA:
            return "pica";

        case FieldUnit::INCH:
            return "inch";

        case FieldUnit::FOOT:
            return "foot";

        case FieldUnit::MILE:
            return "mile";

        case FieldUnit::CHAR:
            return "char";

        case FieldUnit::LINE:
            return "line";

        case FieldUnit::CUSTOM:
            return "custom";

        case FieldUnit::PERCENT:
            return "percent";

        case FieldUnit::MM_100TH:
            return "mm100th";

        case FieldUnit::PIXEL:
            return "pixel";

        case FieldUnit::DEGREE:
            return "degree";

        case FieldUnit::SECOND:
            return "second";

        case FieldUnit::MILLISECOND:
            return "millisecond";

        case FieldUnit::FONT_EM:
            return "em";

        case FieldUnit::FONT_CJK_ADVANCE:
            return "ic";
    }

    return "";
}

sal_Int64 ImplPower10( sal_uInt16 n )
{
    sal_Int64 nValue = 1;

    for (sal_uInt16 i = 0; i < n; i++)
    {
        nValue *= 10;
    }

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
                 rLocaleDataWrapper.getNumDecimalSep() == OUStringChar(cChar) ||
                 (bThousandSep && rLocaleDataWrapper.getNumThousandSep() == OUStringChar(cChar)) ||
                 rLocaleDataWrapper.getNumDecimalSepAlt() == OUStringChar(cChar) ||
                 (cChar == '-'));
    }
}

// Takes a string with a number, which may be an integer, a floating-point with locale-specified
// decimal separator, or a fraction (and if allowed, where negatives can be represented in currency
// format - in parentheses); pre-processes the string to be a floating-point scaled by nDecDigits;
// returns a pair { scaled_whole_part_string, decimal_part_string }.
std::pair<OUString, OUString> ToScaledWholeAndDec(std::u16string_view aStr, sal_uInt16 nDecDigits,
                                                  const LocaleDataWrapper& rLocaleDataWrapper,
                                                  bool bCurrency)
{
    // remove leading and trailing spaces
    aStr = o3tl::trim(aStr);
    OUStringBuffer      aStr1, aStr2, aStrNum, aStrDenom;
    bool                bNegative = false;
    bool                bFrac = false;

    // react on empty string
    if (aStr.empty())
        return {};

    // find position of decimal point
    auto nDecPos = aStr.find(rLocaleDataWrapper.getNumDecimalSep());
    if (nDecPos == std::u16string_view::npos && !rLocaleDataWrapper.getNumDecimalSepAlt().isEmpty())
        nDecPos = aStr.find( rLocaleDataWrapper.getNumDecimalSepAlt() );

    // parse fractional strings
    if (auto nFracDivPos = aStr.find('/');
        nFracDivPos > 0 && nFracDivPos != std::u16string_view::npos)
    {
        bFrac = true;
        auto nFracNumPos = aStr.rfind(' ', nFracDivPos);

        // If in "a b/c" format.
        if (nFracNumPos != std::u16string_view::npos)
        {
            aStr1.append(aStr.substr(0, nFracNumPos));
            aStrNum.append(aStr.substr(nFracNumPos+1, nFracDivPos-nFracNumPos-1));
            aStrDenom.append(aStr.substr(nFracDivPos+1));
        }
        // "a/b" format, or not a fraction at all
        else
        {
            aStrNum.append(aStr.substr(0, nFracDivPos));
            aStrDenom.append(aStr.substr(nFracDivPos+1));
        }

    }
    // parse decimal strings
    else if (nDecPos != std::u16string_view::npos)
    {
        aStr1.append(aStr.substr(0, nDecPos));
        aStr2.append(aStr.substr(nDecPos+1));
    }
    else
        aStr1 = aStr;

    // negative?
    if ( bCurrency )
    {
        if ( aStr.starts_with('(') && aStr.ends_with(')') )
            bNegative = true;
        if ( !bNegative )
        {
            for (size_t i = 0; i < aStr.size(); i++)
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
        if (!bNegative && !aStr.empty())
        {
            sal_uInt16 nFormat = rLocaleDataWrapper.getCurrNegativeFormat();
            if ( (nFormat == 3) || (nFormat == 6)  || // $1- || 1-$
                 (nFormat == 7) || (nFormat == 10) )  // 1$- || 1 $-
            {
                for (size_t i = aStr.size() - 1; i > 0; --i)
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
        return {};
    else if ( bFrac && aStr1.isEmpty() && (aStrNum.isEmpty() || aStrDenom.isEmpty()) )
        return {};

    if ( aStr1.isEmpty() )
        aStr1 = "0";
    if ( bNegative )
        aStr1.insert(0, "-");

    // Convert fractional strings
    if (bFrac) {
        // Convert to fraction
        sal_Int64 nWholeNum = o3tl::toInt64(aStr1);
        aStr1.setLength(0);
        sal_Int64 nNum = o3tl::toInt64(aStrNum);
        sal_Int64 nDenom = o3tl::toInt64(aStrDenom);
        if (nDenom == 0) return {}; // Division by zero
        double nFrac2Dec = nWholeNum + static_cast<double>(nNum)/nDenom; // Convert to double for floating point precision
        OUStringBuffer aStrFrac(OUString::number(nFrac2Dec));
        // Reconvert division result to string and parse
        nDecPos = aStrFrac.indexOf('.');
        if (nDecPos != std::u16string_view::npos)
        {
            aStr1.append(aStrFrac.getStr(), nDecPos);
            aStr2.append(aStrFrac.getStr()+nDecPos+1);
        }
        else
            aStr1 = std::move(aStrFrac);
    }

    if (nDecDigits)
    {
        const sal_Int32 moveTo1 = std::min(static_cast<sal_Int32>(nDecDigits), aStr2.getLength());
        aStr1.append(aStr2.subView(0, moveTo1) + RepeatedUChar('0', nDecDigits - moveTo1));
        aStr2.remove(0, moveTo1);
    }

    return { aStr1.makeStringAndClear(), aStr2.makeStringAndClear() };
}

bool ImplNumericGetValue(std::u16string_view rStr, sal_Int64& rValue, sal_uInt16 nDecDigits,
                         const LocaleDataWrapper& rLocaleDataWrapper, bool bCurrency = false)
{
    const auto [whole, dec] = ToScaledWholeAndDec(rStr, nDecDigits, rLocaleDataWrapper, bCurrency);
    if (whole.isEmpty() && dec.isEmpty())
        return false;

    // prune and round fraction
    const bool bRound = !dec.isEmpty() && dec[0] >= '5';

    // check range
    sal_Int64 nValue = whole.toInt64();
    const bool bNegative = whole.startsWith("-");
    if( nValue == 0 )
    {
        // check if string is equivalent to zero
        sal_Int32 nIndex = bNegative ? 1 : 0;
        while (nIndex < whole.getLength() && whole[nIndex] == '0')
            ++nIndex;
        if (nIndex < whole.getLength())
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

// The returned double is scaled according to nDecDigits, same way as ImplNumericGetValue
bool ImplNumericGetDoubleValue(std::u16string_view rStr, double& rValue, sal_uInt16 nDecDigits,
                               const LocaleDataWrapper& rLocaleDataWrapper)
{
    const auto [whole, dec] = ToScaledWholeAndDec(rStr, nDecDigits, rLocaleDataWrapper, false);
    if (whole.isEmpty() && dec.isEmpty())
        return false;

    rValue = o3tl::toDouble(Concat2View(whole + "." + dec));
    return true;
}

void ImplUpdateSeparatorString( OUString& io_rText,
                                       std::u16string_view rOldDecSep, std::u16string_view rNewDecSep,
                                       std::u16string_view rOldThSep, std::u16string_view rNewThSep )
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
            aBuf.append( OUString::Concat(std::u16string_view(pBuffer + nIndex, nIndexTh - nIndex )) + rNewThSep );
            nIndex = nIndexTh + rOldThSep.size();
        }
        else if( nIndexDec != -1 )
        {
            aBuf.append( OUString::Concat(std::u16string_view(pBuffer + nIndex, nIndexDec - nIndex )) + rNewDecSep );
            nIndex = nIndexDec + rOldDecSep.size();
        }
        else
        {
            aBuf.append( pBuffer + nIndex );
            nIndex = -1;
        }
    }

    io_rText = aBuf.makeStringAndClear();
}

void ImplUpdateSeparators( std::u16string_view rOldDecSep, std::u16string_view rNewDecSep,
                                  std::u16string_view rOldThSep, std::u16string_view rNewThSep,
                                  Edit* pEdit )
{
    bool bChangeDec = (rOldDecSep != rNewDecSep);
    bool bChangeTh = (rOldThSep != rNewThSep );

    if( !(bChangeDec || bChangeTh) )
        return;

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

sal_Int64 clipDoubleAgainstMinMax(double n, sal_Int64 nMin, sal_Int64 nMax)
{
    // caution: precision loss in double cast
    if (n <= static_cast<double>(nMin))
        return nMin;
    else if (n >= static_cast<double>(nMax))
        return nMax;
    else
        return static_cast<sal_Int64>(std::round(n));
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
        mpLocaleDataWrapper.reset( new LocaleDataWrapper( GetLanguageTag() ) );
    }
    return *mpLocaleDataWrapper;
}

/** reset the LocaleDataWrapper when the language tag changes */
void FormatterBase::ImplResetLocaleDataWrapper() const
{
    // just get rid of, the next time it is requested, it will get loaded with the right
    // language tag
    mpLocaleDataWrapper.reset();
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

const css::lang::Locale& FormatterBase::GetLocale() const
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

NumericFormatter::NumericFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
    , mnLastValue(0)
    , mnMin(0)
    // a "large" value substantially smaller than SAL_MAX_INT64, to avoid
    // overflow in computations using this "dummy" value
    , mnMax(SAL_MAX_INT32)
    , mbFormatting(false)
    , mnSpinSize(1)
    // for fields
    , mnFirst(mnMin)
    , mnLast(mnMax)
    , mnDecimalDigits(0)
    , mbThousandSep(true)
{
    ReformatAll();
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

void NumericFormatter::SetValue( sal_Int64 nNewValue )
{
    SetUserValue( nNewValue );
    SetEmptyFieldValueData( false );
}

OUString NumericFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return ImplGetLocaleDataWrapper().getNum( nValue, GetDecimalDigits(), IsUseThousandSep(), /*ShowTrailingZeros*/true );
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

sal_Int64 NumericFormatter::GetValueFromString(std::u16string_view rStr) const
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

OUString NumericFormatter::GetValueString() const
{
    return Application::GetSettings().GetNeutralLocaleDataWrapper().
        getNum(GetValue(), GetDecimalDigits(), false, false);
}

// currently used by online
void NumericFormatter::SetValueFromString(const OUString& rStr)
{
    sal_Int64 nValue;

    if (ImplNumericGetValue(rStr, nValue, GetDecimalDigits(),
        Application::GetSettings().GetNeutralLocaleDataWrapper()))
    {
        ImplNewFieldValue(nValue);
    }
    else
    {
        SAL_WARN("vcl", "fail to convert the value: " << rStr );
    }
}

sal_Int64 NumericFormatter::GetValue() const
{
    if (mbFormatting) //don't parse the entry if we're currently formatting what to put in it
        return mnLastValue;

    return GetField() ? GetValueFromString(GetField()->GetText()) : 0;
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
    if ( !GetField() )
        return;

    // !!! We should check why we do not validate in ImplSetUserValue() if the value was
    // changed. This should be done there as well since otherwise the call to Modify would not
    // be allowed. Anyway, the paths from ImplNewFieldValue, ImplSetUserValue, and ImplSetText
    // should be checked and clearly traced (with comment) in order to find out what happens.

    Selection aSelection = GetField()->GetSelection();
    aSelection.Normalize();
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

sal_Int64 NumericFormatter::ClipAgainstMinMax(sal_Int64 nValue) const
{
    return std::clamp(nValue, GetMin(), GetMax());
}

sal_Int64 NumericFormatter::ClipDoubleAgainstMinMax(double nValue) const
{
    return clipDoubleAgainstMinMax(nValue, GetMin(), GetMax());
}

namespace
{
    Size calcMinimumSize(const Edit &rSpinField, const NumericFormatter &rFormatter)
    {
        OUStringBuffer aBuf;
        sal_Int32 nTextLen;

        nTextLen = OUString::number(rFormatter.GetMin()).length;
        comphelper::string::padToLength(aBuf, nTextLen, '9');
        Size aMinTextSize = rSpinField.CalcMinimumSizeForText(
            rFormatter.CreateFieldText(OUString::unacquired(aBuf).toInt64()));
        aBuf.setLength(0);

        nTextLen = OUString::number(rFormatter.GetMax()).length;
        comphelper::string::padToLength(aBuf, nTextLen, '9');
        Size aMaxTextSize = rSpinField.CalcMinimumSizeForText(
            rFormatter.CreateFieldText(OUString::unacquired(aBuf).toInt64()));
        aBuf.setLength(0);

        Size aRet(std::max(aMinTextSize.Width(), aMaxTextSize.Width()),
                  std::max(aMinTextSize.Height(), aMaxTextSize.Height()));

        aBuf = "999999999";
        sal_uInt16 nDigits = rFormatter.GetDecimalDigits();
        if (nDigits)
        {
            aBuf.append("." + RepeatedUChar('9', nDigits));
        }
        aMaxTextSize = rSpinField.CalcMinimumSizeForText(aBuf.makeStringAndClear());
        aRet.setWidth( std::min(aRet.Width(), aMaxTextSize.Width()) );

        return aRet;
    }
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
    if ( (rNEvt.GetType() == NotifyEventType::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplNumericProcessKeyInput( *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool NumericBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == NotifyEventType::LOSEFOCUS )
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
        ImplResetLocaleDataWrapper();
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

void NumericBox::ImplNumericReformat( std::u16string_view rStr, sal_Int64& rValue,
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

static OUString ImplMetricGetUnitText(std::u16string_view rStr)
{
    // fetch unit text
    OUStringBuffer aStr;
    for (sal_Int32 i = static_cast<sal_Int32>(rStr.size())-1; i >= 0; --i)
    {
        sal_Unicode c = rStr[i];
        if ( (c == '\'') || (c == '\"') || (c == '%') || (c == 0x2032) || (c == 0x2033) || unicode::isAlpha(c) || unicode::isControl(c) )
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

static OUString ImplMetricToString( FieldUnit rUnit )
{
    // return unit's default string (ie, the first one )
    for (auto const& elem : ImplGetFieldUnits())
    {
        if (elem.second == rUnit)
            return elem.first;
    }

    return OUString();
}

namespace
{
    FieldUnit StringToMetric(const OUString &rMetricString)
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
}

static FieldUnit ImplMetricGetUnit(std::u16string_view rStr)
{
    OUString aStr = ImplMetricGetUnitText(rStr);
    return StringToMetric(aStr);
}

static FieldUnit ImplMap2FieldUnit( MapUnit meUnit, tools::Long& nDecDigits )
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
    return std::isfinite( nValue ) ? nValue : 0.0;
}

static double ConvertDoubleValue(double nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                 FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if ( eInUnit != eOutUnit )
    {
        if (eInUnit == FieldUnit::PERCENT && mnBaseValue > 0 && nValue > 0)
        {
            sal_Int64 nDiv = 100 * ImplPower10(nDecDigits);

            if (mnBaseValue != 1)
                nValue *= mnBaseValue;

            nValue += nDiv / 2;
            nValue /= nDiv;
        }
        else
        {
            const o3tl::Length eFrom = FieldToO3tlLength(eInUnit, o3tl::Length::invalid);
            const o3tl::Length eTo = FieldToO3tlLength(eOutUnit, o3tl::Length::invalid);
            if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
                nValue = o3tl::convert(nValue, eFrom, eTo);
        }
    }

    return nValue;
}

namespace vcl
{
    sal_Int64 ConvertValue(sal_Int64 nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                           FieldUnit eInUnit, FieldUnit eOutUnit)
    {
        double nDouble = nonValueDoubleToValueDouble(ConvertDoubleValue(
                    static_cast<double>(nValue), mnBaseValue, nDecDigits, eInUnit, eOutUnit));

        return clipDoubleAgainstMinMax(nDouble, SAL_MIN_INT64, SAL_MAX_INT64);
    }
}

namespace {

bool checkConversionUnits(FieldUnit eInUnit, FieldUnit eOutUnit)
{
    return FieldToO3tlLength(eInUnit, o3tl::Length::invalid) != o3tl::Length::invalid
        && FieldToO3tlLength(eOutUnit, o3tl::Length::invalid) != o3tl::Length::invalid;
}

double convertValue( double nValue, tools::Long nDigits, FieldUnit eInUnit, FieldUnit eOutUnit )
{
    if ( nDigits < 0 )
    {
        nValue /= ImplPower10(-nDigits);
    }
    else
    {
        nValue *= ImplPower10(nDigits);
    }

    if ( eInUnit != eOutUnit )
    {
        const o3tl::Length eFrom = FieldToO3tlLength(eInUnit), eTo = FieldToO3tlLength(eOutUnit);
        if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
            nValue = o3tl::convert(nValue, eFrom, eTo);
    }

    return nValue;
}

}

namespace vcl
{
    // nValue is not scaled to nDecDigits; the result is scaled
    sal_Int64 ConvertAndScaleValue( sal_Int64 nValue, sal_uInt16 nDigits,
                                         MapUnit eInUnit, FieldUnit eOutUnit )
    {
        tools::Long nDecDigits = nDigits;
        FieldUnit eFieldUnit = ImplMap2FieldUnit( eInUnit, nDecDigits );

        if ( !checkConversionUnits(eFieldUnit, eOutUnit) )
        {
            OSL_FAIL( "invalid parameters" );
            return nValue;
        }

        // Avoid sal_Int64 <-> double conversion issues if possible:
        if (eFieldUnit == eOutUnit && nDecDigits == 0)
        {
            return nValue;
        }

        return static_cast<sal_Int64>(
            nonValueDoubleToValueDouble(
                convertValue( nValue, nDecDigits, eFieldUnit, eOutUnit ) ) );
    }

    // nValue is already scaled to nDecDigits; the result is unscaled
    sal_Int64 ConvertAndUnscaleValue(sal_Int64 nValue, sal_uInt16 nDigits, FieldUnit eInUnit,
                                     FieldUnit eOutUnit)
    {
        if (!checkConversionUnits(eInUnit, eOutUnit))
        {
            SAL_WARN("vcl", "invalid parameters");
            return nValue;
        }

        // Avoid sal_Int64 <-> double conversion issues if possible:
        if (eInUnit == eOutUnit && nDigits == 0)
        {
            return nValue;
        }

        return static_cast<sal_Int64>(
            nonValueDoubleToValueDouble(
                convertValue( nValue, -static_cast<tools::Long>(nDigits), eInUnit, eOutUnit ) ) );
    }
}

namespace vcl
{
    bool TextToValue(std::u16string_view rStr, double& rValue, sal_Int64 nBaseValue,
                     sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper, FieldUnit eUnit)
    {
        // Get value
        double nValue;
        if (!ImplNumericGetDoubleValue(rStr, nValue, nDecDigits, rLocaleDataWrapper))
            return false;

        // Determine unit
        FieldUnit eEntryUnit = ImplMetricGetUnit( rStr );

        // Recalculate unit
        rValue = ConvertDoubleValue(nValue, nBaseValue, nDecDigits, eEntryUnit, eUnit);

        return true;
    }

    FieldUnit GetTextMetricUnit(std::u16string_view aStr) { return ImplMetricGetUnit(aStr); }
}

void MetricFormatter::ImplMetricReformat( std::u16string_view rStr, double& rValue, OUString& rOutStr )
{
    if (!vcl::TextToValue(rStr, rValue, 0, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit))
        return;

    rOutStr = CreateFieldText(ClipDoubleAgainstMinMax(rValue));
}

MetricFormatter::MetricFormatter(Edit* pEdit)
    : NumericFormatter(pEdit)
    , meUnit(FieldUnit::NONE)
{
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
        OUString aSuffix = ImplMetricToString( meUnit );
        if (meUnit != FieldUnit::NONE && meUnit != FieldUnit::DEGREE && meUnit != FieldUnit::INCH && meUnit != FieldUnit::FOOT)
            aStr += " ";
        if (meUnit == FieldUnit::INCH)
        {
            OUString sDoublePrime = u"\u2033"_ustr;
            if (aSuffix != "\"" && aSuffix != sDoublePrime)
                aStr += " ";
            else
                aSuffix = sDoublePrime;
        }
        else if (meUnit == FieldUnit::FOOT)
        {
            OUString sPrime = u"\u2032"_ustr;
            if (aSuffix != "'" && aSuffix != sPrime)
                aStr += " ";
            else
                aSuffix = sPrime;
        }

        assert(meUnit != FieldUnit::PERCENT);
        aStr += aSuffix;
    }
    return aStr;
}

void MetricFormatter::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    // convert to previously configured units
    nNewValue = vcl::ConvertValue( nNewValue, 0, GetDecimalDigits(), eInUnit, meUnit );
    NumericFormatter::SetUserValue( nNewValue );
}

sal_Int64 MetricFormatter::GetValueFromStringUnit(std::u16string_view rStr, FieldUnit eOutUnit) const
{
    double nTempValue;
    // caution: precision loss in double cast
    if (!vcl::TextToValue(rStr, nTempValue, 0, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit))
        nTempValue = static_cast<double>(mnLastValue);

    // convert to requested units
    return vcl::ConvertValue(ClipDoubleAgainstMinMax(nTempValue), 0, GetDecimalDigits(), meUnit, eOutUnit);
}

sal_Int64 MetricFormatter::GetValueFromString(std::u16string_view rStr) const
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
    NumericFormatter::SetMin(vcl::ConvertValue(nNewMin, 0, GetDecimalDigits(), eInUnit, meUnit));
}

sal_Int64 MetricFormatter::GetMin( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return vcl::ConvertValue(NumericFormatter::GetMin(), 0, GetDecimalDigits(), meUnit, eOutUnit);
}

void MetricFormatter::SetMax( sal_Int64 nNewMax, FieldUnit eInUnit )
{
    // convert to requested units
    NumericFormatter::SetMax(vcl::ConvertValue(nNewMax, 0, GetDecimalDigits(), eInUnit, meUnit));
}

sal_Int64 MetricFormatter::GetMax( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return vcl::ConvertValue(NumericFormatter::GetMax(), 0, GetDecimalDigits(), meUnit, eOutUnit);
}

void MetricFormatter::Reformat()
{
    if ( !GetField() )
        return;

    OUString aText = GetField()->GetText();

    OUString aStr;
    // caution: precision loss in double cast
    double nTemp = static_cast<double>(mnLastValue);
    ImplMetricReformat( aText, nTemp, aStr );
    mnLastValue = static_cast<sal_Int64>(nTemp);

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
    }
    else
        SetValue( mnLastValue );
}

sal_Int64 MetricFormatter::GetCorrectedValue( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return vcl::ConvertValue(0/*nCorrectedValue*/, 0, GetDecimalDigits(),
                             meUnit, eOutUnit);
}

MetricField::MetricField(vcl::Window* pParent, WinBits nWinStyle)
    : SpinField(pParent, nWinStyle, WindowType::METRICFIELD)
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

bool MetricField::set_property(const OUString &rKey, const OUString &rValue)
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
    nNewFirst = vcl::ConvertValue(nNewFirst, 0, GetDecimalDigits(), eInUnit, meUnit);
    mnFirst = nNewFirst;
}

sal_Int64 MetricField::GetFirst( FieldUnit eOutUnit ) const
{
    // convert
    return vcl::ConvertValue(mnFirst, 0, GetDecimalDigits(), meUnit, eOutUnit);
}

void MetricField::SetLast( sal_Int64 nNewLast, FieldUnit eInUnit )
{
    // convert
    nNewLast = vcl::ConvertValue(nNewLast, 0, GetDecimalDigits(), eInUnit, meUnit);
    mnLast = nNewLast;
}

sal_Int64 MetricField::GetLast( FieldUnit eOutUnit ) const
{
    // convert
    return vcl::ConvertValue(mnLast, 0, GetDecimalDigits(), meUnit, eOutUnit);
}

bool MetricField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == NotifyEventType::LOSEFOCUS )
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
        ImplResetLocaleDataWrapper();
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

void MetricField::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    SpinField::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("min", GetMin());
    rJsonWriter.put("max", GetMax());
    rJsonWriter.put("unit", FieldUnitToString(GetUnit()));
    OUString sValue = Application::GetSettings().GetNeutralLocaleDataWrapper().
        getNum(GetValue(), GetDecimalDigits(), false, false);
    rJsonWriter.put("value", sValue);
}

FactoryFunction MetricField::GetUITestFactory() const
{
    return MetricFieldUIObject::create;
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

bool MetricBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == NotifyEventType::LOSEFOCUS )
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
        ImplResetLocaleDataWrapper();
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

static bool ImplCurrencyGetValue( std::u16string_view rStr, sal_Int64& rValue,
                                  sal_uInt16 nDecDigits, const LocaleDataWrapper& rWrapper )
{
    // fetch number
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rWrapper, true );
}

void CurrencyFormatter::ImplCurrencyReformat(std::u16string_view rStr, OUString& rOutStr)
{
    sal_Int64 nValue;
    if ( !ImplNumericGetValue( rStr, nValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), true ) )
        return;
    rOutStr = CreateFieldText(ClipAgainstMinMax(nValue));
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
    SetEmptyFieldValueData( false );
}

OUString CurrencyFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return ImplGetLocaleDataWrapper().getCurr( nValue, GetDecimalDigits(),
                                               ImplGetLocaleDataWrapper().getCurrSymbol(),
                                               IsUseThousandSep() );
}

sal_Int64 CurrencyFormatter::GetValueFromString(std::u16string_view rStr) const
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

bool CurrencyField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == NotifyEventType::LOSEFOCUS )
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
        ImplResetLocaleDataWrapper();
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

bool CurrencyBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == NotifyEventType::LOSEFOCUS )
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
        ImplResetLocaleDataWrapper();
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
