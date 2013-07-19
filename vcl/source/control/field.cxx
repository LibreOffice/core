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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include "tools/debug.hxx"

#include "tools/rc.h"

#include "vcl/dialog.hxx"
#include "vcl/field.hxx"
#include "vcl/event.hxx"
#include "vcl/svapp.hxx"

#include "svids.hrc"
#include "svdata.hxx"

#include "i18nutil/unicode.hxx"

#include "rtl/math.hxx"

#include <unotools/localedatawrapper.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

// -----------------------------------------------------------------------

#define FORMAT_NUMERIC       1
#define FORMAT_METRIC        2
#define FORMAT_CURRENCY      3

// -----------------------------------------------------------------------

static sal_Int64 ImplPower10( sal_uInt16 n )
{
    sal_uInt16  i;
    sal_Int64   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

// -----------------------------------------------------------------------

static bool ImplNumericProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                        bool bStrictFormat, bool bThousandSep,
                                        const LocaleDataWrapper& rLocaleDataWrappper )
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
             string::equals(rLocaleDataWrappper.getNumDecimalSep(), cChar) ||
             (bThousandSep && string::equals(rLocaleDataWrappper.getNumThousandSep(), cChar)) ||
             (cChar == '-') )
            return false;
        else
            return true;
    }
}

// -----------------------------------------------------------------------

static bool ImplNumericGetValue( const OUString& rStr, sal_Int64& rValue,
                                 sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrappper,
                                 bool bCurrency = false )
{
    OUString            aStr = rStr;
    OUStringBuffer aStr1, aStr2;
    bool                bNegative = false;
    sal_Int32           nDecPos;

    // react on empty string
    if ( rStr.isEmpty() )
        return false;

    // remove leading and trailing spaces
    aStr = aStr.trim();

    // find position of decimal point
    nDecPos = aStr.indexOf( rLocaleDataWrappper.getNumDecimalSep() );
    if ( nDecPos >= 0)
    {
        aStr1.append(aStr.getStr(), nDecPos);
        aStr2.append(aStr.getStr()+nDecPos+1);
    }
    else
        aStr1 = aStr;

    // negative?
    if ( bCurrency )
    {
        if ( aStr[0] == '(' && aStr[aStr.getLength()-1] == ')' )
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
        if ( !bNegative && bCurrency && !aStr.isEmpty() )
        {
            sal_uInt16 nFormat = rLocaleDataWrappper.getCurrNegativeFormat();
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
    }

    // remove all unwanted charaters
    for (sal_Int32 i=0; i < aStr1.getLength(); )
    {
        if ( (aStr1[i] >= '0') && (aStr1[i] <= '9') )
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

    if ( aStr1.isEmpty() && aStr2.isEmpty() )
        return false;

    if ( aStr1.isEmpty() )
        aStr1 = "0";
    if ( bNegative )
        aStr1 = "-" + aStr1;

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
    sal_Int64 nValue = aStr.toInt64();
    if( nValue == 0 )
    {
        // check if string is equivalent to zero
        sal_Int16 nIndex = bNegative ? 1 : 0;
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

static void ImplUpdateSeparatorString( String& io_rText,
                                       const String& rOldDecSep, const String& rNewDecSep,
                                       const String& rOldThSep, const String& rNewThSep )
{
    OUStringBuffer aBuf( io_rText.Len() );
    xub_StrLen nIndexDec = 0, nIndexTh = 0, nIndex = 0;

    const sal_Unicode* pBuffer = io_rText.GetBuffer();
    while( nIndex != STRING_NOTFOUND )
    {
        nIndexDec = io_rText.Search( rOldDecSep, nIndex );
        nIndexTh = io_rText.Search( rOldThSep, nIndex );
        if(   (nIndexTh != STRING_NOTFOUND && nIndexDec != STRING_NOTFOUND && nIndexTh < nIndexDec )
           || (nIndexTh != STRING_NOTFOUND && nIndexDec == STRING_NOTFOUND)
           )
        {
            aBuf.append( pBuffer + nIndex, nIndexTh - nIndex );
            aBuf.append( rNewThSep );
            nIndex = nIndexTh + rOldThSep.Len();
        }
        else if( nIndexDec != STRING_NOTFOUND )
        {
            aBuf.append( pBuffer + nIndex, nIndexDec - nIndex );
            aBuf.append( rNewDecSep );
            nIndex = nIndexDec + rOldDecSep.Len();
        }
        else
        {
            aBuf.append( pBuffer + nIndex );
            nIndex = STRING_NOTFOUND;
        }
    }

    io_rText = aBuf.makeStringAndClear();
}

static void ImplUpdateSeparators( const String& rOldDecSep, const String& rNewDecSep,
                                  const String& rOldThSep, const String& rNewThSep,
                                  Edit* pEdit )
{
    bool bChangeDec = (rOldDecSep != rNewDecSep);
    bool bChangeTh = (rOldThSep != rNewThSep );

    if( bChangeDec || bChangeTh )
    {
        sal_Bool bUpdateMode = pEdit->IsUpdateMode();
        pEdit->SetUpdateMode( sal_False );
        String aText = pEdit->GetText();
        ImplUpdateSeparatorString( aText, rOldDecSep, rNewDecSep, rOldThSep, rNewThSep );
        pEdit->SetText( aText );

        ComboBox* pCombo = dynamic_cast<ComboBox*>(pEdit);
        if( pCombo )
        {
            // update box entries
            sal_uInt16 nEntryCount = pCombo->GetEntryCount();
            for ( sal_uInt16 i=0; i < nEntryCount; i++ )
            {
                aText = pCombo->GetEntry( i );
                void* pEntryData = pCombo->GetEntryData( i );
                ImplUpdateSeparatorString( aText, rOldDecSep, rNewDecSep, rOldThSep, rNewThSep );
                pCombo->RemoveEntry( i );
                pCombo->InsertEntry( aText, i );
                pCombo->SetEntryData( i, pEntryData );
            }
        }
        if( bUpdateMode )
            pEdit->SetUpdateMode( bUpdateMode );
    }
}

// -----------------------------------------------------------------------

FormatterBase::FormatterBase( Edit* pField )
{
    mpField                     = pField;
    mpLocaleDataWrapper         = NULL;
    mbReformat                  = sal_False;
    mbStrictFormat              = sal_False;
    mbEmptyFieldValue           = sal_False;
    mbEmptyFieldValueEnabled    = sal_False;
    mbDefaultLocale             = sal_True;
}

// -----------------------------------------------------------------------

FormatterBase::~FormatterBase()
{
    delete mpLocaleDataWrapper;
}

// -----------------------------------------------------------------------

LocaleDataWrapper& FormatterBase::ImplGetLocaleDataWrapper() const
{
    if ( !mpLocaleDataWrapper )
    {
        ((FormatterBase*)this)->mpLocaleDataWrapper = new LocaleDataWrapper( GetLanguageTag() );
    }
    return *mpLocaleDataWrapper;
}

const LocaleDataWrapper& FormatterBase::GetLocaleDataWrapper() const
{
    return ImplGetLocaleDataWrapper();
}

// -----------------------------------------------------------------------

void FormatterBase::Reformat()
{
}

// -----------------------------------------------------------------------

void FormatterBase::ReformatAll()
{
    Reformat();
};

// -----------------------------------------------------------------------

void FormatterBase::SetStrictFormat( sal_Bool bStrict )
{
    if ( bStrict != mbStrictFormat )
    {
        mbStrictFormat = bStrict;
        if ( mbStrictFormat )
            ReformatAll();
    }
}

// -----------------------------------------------------------------------

void FormatterBase::SetLocale( const lang::Locale& rLocale )
{
    ImplGetLocaleDataWrapper().setLanguageTag( LanguageTag( rLocale) );
    mbDefaultLocale = sal_False;
    ReformatAll();
}

// -----------------------------------------------------------------------

const lang::Locale& FormatterBase::GetLocale() const
{
    if ( !mpLocaleDataWrapper || mbDefaultLocale )
    {
        if ( mpField )
            return mpField->GetSettings().GetLanguageTag().getLocale();
        else
            return Application::GetSettings().GetLanguageTag().getLocale();
    }

    return mpLocaleDataWrapper->getLanguageTag().getLocale();
}

// -----------------------------------------------------------------------

const LanguageTag& FormatterBase::GetLanguageTag() const
{
    if ( !mpLocaleDataWrapper || mbDefaultLocale )
    {
        if ( mpField )
            return mpField->GetSettings().GetLanguageTag();
        else
            return Application::GetSettings().GetLanguageTag();
    }

    return mpLocaleDataWrapper->getLanguageTag();
}

// -----------------------------------------------------------------------

const AllSettings& FormatterBase::GetFieldSettings() const
{
    if ( mpField )
        return mpField->GetSettings();
    else
        return Application::GetSettings();
}

// -----------------------------------------------------------------------

void FormatterBase::ImplSetText( const OUString& rText, Selection* pNewSelection )
{
    if ( mpField )
    {
        if ( pNewSelection )
            mpField->SetText( rText, *pNewSelection );
        else
        {
            Selection aSel = mpField->GetSelection();
            aSel.Min() = aSel.Max();
            mpField->SetText( rText, aSel );
        }

        MarkToBeReformatted( sal_False );
    }
}

// -----------------------------------------------------------------------

void FormatterBase::SetEmptyFieldValue()
{
    if ( mpField )
        mpField->SetText( ImplGetSVEmptyStr() );
    mbEmptyFieldValue = sal_True;
}

// -----------------------------------------------------------------------

sal_Bool FormatterBase::IsEmptyFieldValue() const
{
    return (!mpField || mpField->GetText().isEmpty());
}

// -----------------------------------------------------------------------

sal_Bool NumericFormatter::ImplNumericReformat( const OUString& rStr, sal_Int64& rValue,
                                                OUString& rOutStr )
{
    if ( !ImplNumericGetValue( rStr, rValue, GetDecimalDigits(), ImplGetLocaleDataWrapper() ) )
        return sal_True;
    else
    {
        sal_Int64 nTempVal = rValue;
        if ( nTempVal > mnMax )
            nTempVal = mnMax;
        else if ( nTempVal < mnMin )
            nTempVal = mnMin;

        if ( GetErrorHdl().IsSet() && (rValue != nTempVal) )
        {
            mnCorrectedValue = nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return sal_False;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = CreateFieldText( nTempVal );
        return sal_True;
    }
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplInit()
{
    mnFieldValue        = 0;
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = 0x7FFFFFFFFFFFFFFFLL;
    mnCorrectedValue    = 0;
    mnDecimalDigits     = 2;
    mnType              = FORMAT_NUMERIC;
    mbThousandSep       = sal_True;
    mbShowTrailingZeros = sal_True;

    // for fields
    mnSpinSize          = 1;
    mnFirst             = mnMin;
    mnLast              = mnMax;

    SetDecimalDigits( 0 );
}

// -----------------------------------------------------------------------

NumericFormatter::NumericFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplLoadRes( const ResId& rResId )
{
    ResMgr*     pMgr = rResId.GetResMgr();

    if( pMgr )
    {
        sal_uLong nMask = pMgr->ReadLong();

        if ( NUMERICFORMATTER_MIN & nMask )
            mnMin = pMgr->ReadLong();

        if ( NUMERICFORMATTER_MAX & nMask )
            mnMax = pMgr->ReadLong();

        if ( NUMERICFORMATTER_STRICTFORMAT & nMask )
            SetStrictFormat( (sal_Bool)pMgr->ReadShort() );

        if ( NUMERICFORMATTER_DECIMALDIGITS & nMask )
            SetDecimalDigits( pMgr->ReadShort() );

        if ( NUMERICFORMATTER_VALUE & nMask )
        {
            mnFieldValue = pMgr->ReadLong();
            if ( mnFieldValue > mnMax )
                mnFieldValue = mnMax;
            else if ( mnFieldValue < mnMin )
                mnFieldValue = mnMin;
            mnLastValue = mnFieldValue;
        }

        if ( NUMERICFORMATTER_NOTHOUSANDSEP & nMask )
            SetUseThousandSep( !(sal_Bool)pMgr->ReadShort() );
    }
}

// -----------------------------------------------------------------------

NumericFormatter::~NumericFormatter()
{
}

// -----------------------------------------------------------------------

void NumericFormatter::SetMin( sal_Int64 nNewMin )
{
    mnMin = nNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetMax( sal_Int64 nNewMax )
{
    mnMax = nNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetUseThousandSep( sal_Bool b )
{
    mbThousandSep = b;
    ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetDecimalDigits( sal_uInt16 nDigits )
{
    mnDecimalDigits = nDigits;
    ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetShowTrailingZeros( sal_Bool bShowTrailingZeros )
{
    if ( mbShowTrailingZeros != bShowTrailingZeros )
    {
        mbShowTrailingZeros = bShowTrailingZeros;
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 NumericFormatter::GetDecimalDigits() const
{
    return mnDecimalDigits;
}

// -----------------------------------------------------------------------

void NumericFormatter::SetValue( sal_Int64 nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( sal_False );
}

// -----------------------------------------------------------------------

OUString NumericFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return OUString(ImplGetLocaleDataWrapper().getNum( nValue, GetDecimalDigits(), IsUseThousandSep(), IsShowTrailingZeros() ));
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplSetUserValue( sal_Int64 nNewValue, Selection* pNewSelection )
{
    if ( nNewValue > mnMax )
        nNewValue = mnMax;
    else if ( nNewValue < mnMin )
        nNewValue = mnMin;
    mnLastValue = nNewValue;

    if ( GetField() )
        ImplSetText( CreateFieldText( nNewValue ), pNewSelection );
}

// -----------------------------------------------------------------------

void NumericFormatter::SetUserValue( sal_Int64 nNewValue )
{
    ImplSetUserValue( nNewValue );
}

// -----------------------------------------------------------------------

sal_Int64 NumericFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    sal_Int64 nTempValue;

    if ( ImplNumericGetValue( GetField()->GetText(), nTempValue,
                              GetDecimalDigits(), ImplGetLocaleDataWrapper() ) )
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

// -----------------------------------------------------------------------

sal_Bool NumericFormatter::IsValueModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyFieldValue();
    else if ( GetValue() != mnFieldValue )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Int64 NumericFormatter::Normalize( sal_Int64 nValue ) const
{
    return (nValue * ImplPower10( GetDecimalDigits() ) );
}

// -----------------------------------------------------------------------

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
        sal_Int64 nHalf = nFactor/2;
        return ((nValue-nHalf) / nFactor );
    }
    else
    {
        sal_Int64 nHalf = nFactor/2;
        return ((nValue+nHalf) / nFactor );
    }
}

// -----------------------------------------------------------------------

void NumericFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( GetField()->GetText().isEmpty() && ImplGetEmptyFieldValue() )
        return;

    OUString aStr;
    sal_Int64 nTemp = mnLastValue;
    sal_Bool bOK = ImplNumericReformat( GetField()->GetText(), nTemp, aStr );
    mnLastValue = nTemp;
    if ( !bOK )
        return;

    if ( !aStr.isEmpty() )
        ImplSetText( aStr );
    else
        SetValue( mnLastValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldUp()
{
    sal_Int64 nValue = GetValue();
    nValue += mnSpinSize;
    if ( nValue > mnMax )
        nValue = mnMax;

    ImplNewFieldValue( nValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldDown()
{
    sal_Int64 nValue = GetValue();
    nValue -= mnSpinSize;
    if ( nValue < mnMin )
        nValue = mnMin;

    ImplNewFieldValue( nValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldFirst()
{
    ImplNewFieldValue( mnFirst );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldLast()
{
    ImplNewFieldValue( mnLast );
}

// -----------------------------------------------------------------------

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
        if ( (sal_Int32)aSelection.Max() == aText.getLength() )
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

// -----------------------------------------------------------------------

NumericField::NumericField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

NumericField::NumericField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_NUMERICFIELD )
{
    rResId.SetRT( RSC_NUMERICFIELD );
    WinBits nStyle = ImplInitRes( rResId ) ;
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    ImplLoadRes( rResId );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

bool NumericField::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "digits")
        SetDecimalDigits(rValue.toInt32());
    else
        return SpinField::set_property(rKey, rValue);
    return true;
}

// -----------------------------------------------------------------------

void NumericField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    NumericFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );

    sal_uLong      nMask = ReadLongRes();

    if ( NUMERICFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( NUMERICFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( NUMERICFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();
}

// -----------------------------------------------------------------------

NumericField::~NumericField()
{
}

// -----------------------------------------------------------------------

long NumericField::PreNotify( NotifyEvent& rNEvt )
{
        if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplNumericProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long NumericField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void NumericField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void NumericField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void NumericField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void NumericField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void NumericField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

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

        nTextLen = OUString::valueOf(rFormatter.GetMin()).getLength();
        string::padToLength(aBuf, nTextLen, '9');
        Size aMinTextSize = rSpinField.CalcMinimumSizeForText(
            rFormatter.CreateFieldText(aBuf.makeStringAndClear().toInt64()));

        nTextLen = OUString::valueOf(rFormatter.GetMax()).getLength();
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
        aRet.Width() = std::min(aRet.Width(), aMaxTextSize.Width());

        return aRet;
    }
}

Size NumericField::CalcMinimumSize() const
{
    return calcMinimumSize(*this, *this);
}

// -----------------------------------------------------------------------

NumericBox::NumericBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

NumericBox::NumericBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_NUMERICBOX )
{
    rResId.SetRT( RSC_NUMERICBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    SetField( this );
    ComboBox::ImplLoadRes( rResId );
    NumericFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

Size NumericBox::CalcMinimumSize() const
{
    Size aRet(calcMinimumSize(*this, *this));

    if (IsDropDownBox())
    {
        Size aComboSugg(ComboBox::CalcMinimumSize());
        aRet.Width() = std::max(aRet.Width(), aComboSugg.Width());
        aRet.Height() = std::max(aRet.Height(), aComboSugg.Height());
    }

    return aRet;
}

// -----------------------------------------------------------------------

NumericBox::~NumericBox()
{
}

// -----------------------------------------------------------------------

long NumericBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplNumericProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long NumericBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void NumericBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void NumericBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void NumericBox::ReformatAll()
{
    sal_Int64 nValue;
    OUString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplNumericReformat( GetEntry( i ), nValue, aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    NumericFormatter::Reformat();
    SetUpdateMode( sal_True );
}

// -----------------------------------------------------------------------

void NumericBox::InsertValue( sal_Int64 nValue, sal_uInt16 nPos )
{
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

// -----------------------------------------------------------------------

static bool ImplMetricProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                       bool, bool bUseThousandSep, const LocaleDataWrapper& rWrapper )
{
    // no meaningfull strict format; therefore allow all characters
    return ImplNumericProcessKeyInput( pEdit, rKEvt, false, bUseThousandSep, rWrapper );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

// #104355# support localized mesaurements

static const OUString ImplMetricToString( FieldUnit rUnit )
{
    FieldUnitStringList* pList = ImplGetFieldUnits();
    if( pList )
    {
        // return unit's default string (ie, the first one )
        for( FieldUnitStringList::const_iterator it = pList->begin(); it != pList->end(); ++it )
        {
            if ( it->second == rUnit )
                return it->first;
        }
    }

    return OUString();
}

static FieldUnit ImplStringToMetric(const OUString &rMetricString)
{
    FieldUnitStringList* pList = ImplGetCleanedFieldUnits();
    if( pList )
    {
        // return FieldUnit
        OUString aStr(rMetricString.toAsciiLowerCase());
        aStr = string::remove(aStr, ' ');
        for( FieldUnitStringList::const_iterator it = pList->begin(); it != pList->end(); ++it )
        {
            if ( it->first == aStr )
                return it->second;
        }
    }

    return FUNIT_NONE;
}

// -----------------------------------------------------------------------

static FieldUnit ImplMetricGetUnit(const OUString& rStr)
{
    OUString aStr = ImplMetricGetUnitText( rStr );
    return ImplStringToMetric( aStr );
}

#define K *1000L
#define M *1000000LL
#define X *5280L

// twip in km = 254 / 14 400 000 000
// expressions too big for default size 32 bit need LL to avoid overflow

static const sal_Int64 aImplFactor[FUNIT_LINE+1][FUNIT_LINE+1] =
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

static FieldUnit eDefaultUnit = FUNIT_NONE;

FieldUnit MetricField::GetDefaultUnit() { return eDefaultUnit; }
void MetricField::SetDefaultUnit( FieldUnit meUnit ) { eDefaultUnit = meUnit; }

static FieldUnit ImplMap2FieldUnit( MapUnit meUnit, long& nDecDigits )
{
    switch( meUnit )
    {
        case MAP_100TH_MM :
            nDecDigits -= 2;
            return FUNIT_MM;
        case MAP_10TH_MM :
            nDecDigits -= 1;
            return FUNIT_MM;
        case MAP_MM :
            return FUNIT_MM;
        case MAP_CM :
            return FUNIT_CM;
        case MAP_1000TH_INCH :
            nDecDigits -= 3;
            return FUNIT_INCH;
        case MAP_100TH_INCH :
            nDecDigits -= 2;
            return FUNIT_INCH;
        case MAP_10TH_INCH :
            nDecDigits -= 1;
            return FUNIT_INCH;
        case MAP_INCH :
            return FUNIT_INCH;
        case MAP_POINT :
            return FUNIT_POINT;
        case MAP_TWIP :
            return FUNIT_TWIP;
        default:
            OSL_FAIL( "default eInUnit" );
            break;
    }
    return FUNIT_NONE;
}

// -----------------------------------------------------------------------

static double nonValueDoubleToValueDouble( double nValue )
{
    return rtl::math::isFinite( nValue ) ? nValue : 0.0;
}

sal_Int64 MetricField::ConvertValue( sal_Int64 nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                     FieldUnit eInUnit, FieldUnit eOutUnit )
{
    double nDouble = nonValueDoubleToValueDouble( ConvertDoubleValue(
                (double)nValue, mnBaseValue, nDecDigits, eInUnit, eOutUnit ) );
    sal_Int64 nLong ;

    // caution: precision loss in double cast
    if ( nDouble <= (double)SAL_MIN_INT64 )
        nLong = SAL_MIN_INT64;
    else if ( nDouble >= (double)SAL_MAX_INT64 )
        nLong = SAL_MAX_INT64;
    else
        nLong = static_cast<sal_Int64>( nDouble );

    return nLong;
}

// -----------------------------------------------------------------------

sal_Int64 MetricField::ConvertValue( sal_Int64 nValue, sal_uInt16 nDigits,
                                     MapUnit eInUnit, FieldUnit eOutUnit )
{
    return static_cast<sal_Int64>(
        nonValueDoubleToValueDouble(
            ConvertDoubleValue( nValue, nDigits, eInUnit, eOutUnit ) ) );
}

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                        FieldUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eInUnit != eOutUnit )
    {
        sal_Int64 nMult = 1, nDiv = 1;

        if ( eInUnit == FUNIT_PERCENT )
        {
            if ( (mnBaseValue <= 0) || (nValue <= 0) )
                return nValue;
            nDiv = 100 * ImplPower10(nDecDigits);

            nMult = mnBaseValue;
        }
        else if ( eOutUnit == FUNIT_PERCENT ||
                  eOutUnit == FUNIT_CUSTOM ||
                  eOutUnit == FUNIT_NONE ||
                  eOutUnit == FUNIT_DEGREE ||
                  eOutUnit == FUNIT_PIXEL ||
                  eInUnit  == FUNIT_CUSTOM ||
                  eInUnit  == FUNIT_NONE ||
                  eInUnit  == FUNIT_DEGREE ||
                  eInUnit  == FUNIT_PIXEL )
             return nValue;
        else
        {
            if ( eOutUnit == FUNIT_100TH_MM )
                eOutUnit = FUNIT_NONE;
            if ( eInUnit == FUNIT_100TH_MM )
                eInUnit = FUNIT_NONE;

            nDiv  = aImplFactor[eInUnit][eOutUnit];
            nMult = aImplFactor[eOutUnit][eInUnit];

            DBG_ASSERT( nMult > 0, "illegal *" );
            DBG_ASSERT( nDiv  > 0, "illegal /" );
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

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, sal_uInt16 nDigits,
                                        MapUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eOutUnit == FUNIT_PERCENT ||
         eOutUnit == FUNIT_CUSTOM ||
         eOutUnit == FUNIT_NONE ||
         eInUnit == MAP_PIXEL ||
         eInUnit == MAP_SYSFONT ||
         eInUnit == MAP_APPFONT ||
         eInUnit == MAP_RELATIVE )
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
        sal_Int64 nDiv  = aImplFactor[eFieldUnit][eOutUnit];
        sal_Int64 nMult = aImplFactor[eOutUnit][eFieldUnit];

        DBG_ASSERT( nMult > 0, "illegal *" );
        DBG_ASSERT( nDiv  > 0, "illegal /" );

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

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, sal_uInt16 nDigits,
                                        FieldUnit eInUnit, MapUnit eOutUnit )
{
    if ( eInUnit == FUNIT_PERCENT ||
         eInUnit == FUNIT_CUSTOM ||
         eInUnit == FUNIT_NONE ||
         eInUnit == FUNIT_DEGREE ||
         eInUnit == FUNIT_PIXEL ||
         eOutUnit == MAP_PIXEL ||
         eOutUnit == MAP_SYSFONT ||
         eOutUnit == MAP_APPFONT ||
         eOutUnit == MAP_RELATIVE )
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
        sal_Int64 nDiv  = aImplFactor[eInUnit][eFieldUnit];
        sal_Int64 nMult = aImplFactor[eFieldUnit][eInUnit];

        DBG_ASSERT( nMult > 0, "illegal *" );
        DBG_ASSERT( nDiv  > 0, "illegal /" );

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

// -----------------------------------------------------------------------

static bool ImplMetricGetValue( const OUString& rStr, double& rValue, sal_Int64 nBaseValue,
                                sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper, FieldUnit eUnit )
{
    // Zahlenwert holen
    sal_Int64 nValue;
    if ( !ImplNumericGetValue( rStr, nValue, nDecDigits, rLocaleDataWrapper ) )
        return false;

    // Einheit rausfinden
    FieldUnit eEntryUnit = ImplMetricGetUnit( rStr );

    // Einheiten umrechnen
    // caution: conversion to double loses precision
    rValue = MetricField::ConvertDoubleValue( (double)nValue, nBaseValue, nDecDigits, eEntryUnit, eUnit );

    return true;
}

// -----------------------------------------------------------------------

sal_Bool MetricFormatter::ImplMetricReformat( const OUString& rStr, double& rValue, OUString& rOutStr )
{
    if ( !ImplMetricGetValue( rStr, rValue, mnBaseValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit ) )
        return sal_True;
    else
    {
        double nTempVal = rValue;
        // caution: precision loss in double cast
        if ( nTempVal > GetMax() )
            nTempVal = (double)GetMax();
        else if ( nTempVal < GetMin())
            nTempVal = (double)GetMin();

        if ( GetErrorHdl().IsSet() && (rValue != nTempVal) )
        {
            mnCorrectedValue = (sal_Int64)nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return sal_False;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = CreateFieldText( (sal_Int64)nTempVal );
        return sal_True;
    }
}

// -----------------------------------------------------------------------

inline void MetricFormatter::ImplInit()
{
    mnBaseValue = 0;
    meUnit = MetricField::GetDefaultUnit();
    mnType = FORMAT_METRIC;
}

// -----------------------------------------------------------------------

MetricFormatter::MetricFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

void MetricFormatter::ImplLoadRes( const ResId& rResId )
{
    NumericFormatter::ImplLoadRes( rResId );

    ResMgr*     pMgr = rResId.GetResMgr();
    if( pMgr )
    {
        sal_uLong       nMask = pMgr->ReadLong();

        if ( METRICFORMATTER_UNIT & nMask )
            meUnit = (FieldUnit)pMgr->ReadLong();

        if ( METRICFORMATTER_CUSTOMUNITTEXT & nMask )
            maCustomUnitText = pMgr->ReadString();
    }
}

// -----------------------------------------------------------------------

MetricFormatter::~MetricFormatter()
{
}

// -----------------------------------------------------------------------

void MetricFormatter::SetUnit( FieldUnit eNewUnit )
{
    if ( eNewUnit == FUNIT_100TH_MM )
    {
        SetDecimalDigits( GetDecimalDigits() + 2 );
        meUnit = FUNIT_MM;
    }
    else
        meUnit = eNewUnit;
    ReformatAll();
}

// -----------------------------------------------------------------------

void MetricFormatter::SetCustomUnitText( const OUString& rStr )
{
    maCustomUnitText = rStr;
    ReformatAll();
}

// -----------------------------------------------------------------------

void MetricFormatter::SetValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    SetUserValue( nNewValue, eInUnit );
    mnFieldValue = mnLastValue;
}

// -----------------------------------------------------------------------

OUString MetricFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    OUString aStr = NumericFormatter::CreateFieldText( nValue );

    if( meUnit == FUNIT_CUSTOM )
        aStr += maCustomUnitText;
    else
        aStr += ImplMetricToString( meUnit );

    return aStr;
}

// -----------------------------------------------------------------------

void MetricFormatter::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    // convert to previously configured units
    nNewValue = MetricField::ConvertValue( nNewValue, mnBaseValue, GetDecimalDigits(), eInUnit, meUnit );
    NumericFormatter::SetUserValue( nNewValue );
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetValue( FieldUnit eOutUnit ) const
{
    if ( !GetField() )
        return 0;

    double nTempValue;
    // caution: precision loss in double cast
    if ( !ImplMetricGetValue( GetField()->GetText(), nTempValue, mnBaseValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit ) )
        nTempValue = (double)mnLastValue;

    // caution: precision loss in double cast
    if ( nTempValue > mnMax )
        nTempValue = (double)mnMax;
    else if ( nTempValue < mnMin )
        nTempValue = (double)mnMin;

    // convert to requested units
    return MetricField::ConvertValue( (sal_Int64)nTempValue, mnBaseValue, GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetValue( sal_Int64 nValue )
{
    // Implementation not inline, because it is a virtual Function
    SetValue( nValue, FUNIT_NONE );
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetMin( sal_Int64 nNewMin, FieldUnit eInUnit )
{
    // convert to requested units
    NumericFormatter::SetMin( MetricField::ConvertValue( nNewMin, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetMin( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( NumericFormatter::GetMin(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetMax( sal_Int64 nNewMax, FieldUnit eInUnit )
{
    // convert to requested units
    NumericFormatter::SetMax( MetricField::ConvertValue( nNewMax, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetMax( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( NumericFormatter::GetMax(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetBaseValue( sal_Int64 nNewBase, FieldUnit eInUnit )
{
    mnBaseValue = MetricField::ConvertValue( nNewBase, mnBaseValue, GetDecimalDigits(),
                                             eInUnit, meUnit );
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetBaseValue( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( mnBaseValue, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::Reformat()
{
    if ( !GetField() )
        return;

    OUString aText = GetField()->GetText();
    if ( meUnit == FUNIT_CUSTOM )
        maCurUnitText = ImplMetricGetUnitText( aText );

    OUString aStr;
    // caution: precision loss in double cast
    double nTemp = (double)mnLastValue;
    sal_Bool bOK = ImplMetricReformat( aText, nTemp, aStr );
    mnLastValue = (sal_Int64)nTemp;

    if ( !bOK )
        return;

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        if ( meUnit == FUNIT_CUSTOM )
            CustomConvert();
    }
    else
        SetValue( mnLastValue );
    maCurUnitText = OUString();
}

// -----------------------------------------------------------------------

sal_Int64 MetricFormatter::GetCorrectedValue( FieldUnit eOutUnit ) const
{
    // convert to requested units
    return MetricField::ConvertValue( mnCorrectedValue, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

MetricField::MetricField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

MetricField::MetricField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_METRICFIELD )
{
    rResId.SetRT( RSC_METRICFIELD );
    WinBits nStyle = ImplInitRes( rResId ) ;
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

Size MetricField::CalcMinimumSize() const
{
    return calcMinimumSize(*this, *this);
}

bool MetricField::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "format")
    {
        maCustomUnitText = OStringToOUString(rValue, RTL_TEXTENCODING_UTF8);
        meUnit = FUNIT_CUSTOM;
    }
    else if (rKey == "digits")
        SetDecimalDigits(rValue.toInt32());
    else
        return SpinField::set_property(rKey, rValue);
    return true;
}

void MetricField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    MetricFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );

    sal_uLong      nMask = ReadLongRes();

    if ( METRICFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( METRICFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( METRICFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();

    Reformat();
}

// -----------------------------------------------------------------------

MetricField::~MetricField()
{
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

// -----------------------------------------------------------------------

void MetricField::SetFirst( sal_Int64 nNewFirst, FieldUnit eInUnit )
{
    // convert
    nNewFirst = MetricField::ConvertValue( nNewFirst, mnBaseValue, GetDecimalDigits(),
                                           eInUnit, meUnit );
    mnFirst = nNewFirst;
}

// -----------------------------------------------------------------------

sal_Int64 MetricField::GetFirst( FieldUnit eOutUnit ) const
{
    // convert
    return MetricField::ConvertValue( mnFirst, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricField::SetLast( sal_Int64 nNewLast, FieldUnit eInUnit )
{
    // convert
    nNewLast = MetricField::ConvertValue( nNewLast, mnBaseValue, GetDecimalDigits(),
                                          eInUnit, meUnit );
    mnLast = nNewLast;
}

// -----------------------------------------------------------------------

sal_Int64 MetricField::GetLast( FieldUnit eOutUnit ) const
{
    // conver
    return MetricField::ConvertValue( mnLast, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

long MetricField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplMetricProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long MetricField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void MetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void MetricField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void MetricField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void MetricField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void MetricField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

void MetricField::Last()
{
    FieldLast();
    SpinField::Last();
}

// -----------------------------------------------------------------------

void MetricField::CustomConvert()
{
    maCustomConvertLink.Call( this );
}

// -----------------------------------------------------------------------

MetricBox::MetricBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

Size MetricBox::CalcMinimumSize() const
{
    Size aRet(calcMinimumSize(*this, *this));

    if (IsDropDownBox())
    {
        Size aComboSugg(ComboBox::CalcMinimumSize());
        aRet.Width() = std::max(aRet.Width(), aComboSugg.Width());
        aRet.Height() = std::max(aRet.Height(), aComboSugg.Height());
    }

    return aRet;
}

// -----------------------------------------------------------------------

MetricBox::~MetricBox()
{
}

// -----------------------------------------------------------------------

long MetricBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2()  )
    {
        if ( ImplMetricProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long MetricBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void MetricBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void MetricBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void MetricBox::ReformatAll()
{
    double nValue;
    OUString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplMetricReformat( GetEntry( i ), nValue, aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    MetricFormatter::Reformat();
    SetUpdateMode( sal_True );
}

// -----------------------------------------------------------------------

void MetricBox::CustomConvert()
{
    maCustomConvertLink.Call( this );
}

// -----------------------------------------------------------------------

void MetricBox::InsertValue( sal_Int64 nValue, FieldUnit eInUnit, sal_uInt16 nPos )
{
    // convert to previously configured units
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

// -----------------------------------------------------------------------

sal_Int64 MetricBox::GetValue( sal_uInt16 nPos, FieldUnit eOutUnit ) const
{
    double nValue = 0;
    ImplMetricGetValue( ComboBox::GetEntry( nPos ), nValue, mnBaseValue,
                        GetDecimalDigits(), ImplGetLocaleDataWrapper(), meUnit );

    // convert to previously configured units
    sal_Int64 nRetValue = MetricField::ConvertValue( (sal_Int64)nValue, mnBaseValue, GetDecimalDigits(),
                                                     meUnit, eOutUnit );

    return nRetValue;
}

// -----------------------------------------------------------------------

sal_uInt16 MetricBox::GetValuePos( sal_Int64 nValue, FieldUnit eInUnit ) const
{
    // convert to previously configured units
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    return ComboBox::GetEntryPos( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

sal_Int64 MetricBox::GetValue( FieldUnit eOutUnit ) const
{
    // Implementation not inline, because it is a virtual Function
    return MetricFormatter::GetValue( eOutUnit );
}

// -----------------------------------------------------------------------

sal_Int64 MetricBox::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

static bool ImplCurrencyProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                         bool, bool bUseThousandSep, const LocaleDataWrapper& rWrapper )
{
    // no strict format set; therefore allow all characters
    return ImplNumericProcessKeyInput( pEdit, rKEvt, false, bUseThousandSep, rWrapper );
}

// -----------------------------------------------------------------------

inline bool ImplCurrencyGetValue( const OUString& rStr, sal_Int64& rValue,
                                  sal_uInt16 nDecDigits, const LocaleDataWrapper& rWrapper )
{
    // fetch number
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rWrapper, true );
}

// -----------------------------------------------------------------------

sal_Bool CurrencyFormatter::ImplCurrencyReformat( const OUString& rStr, OUString& rOutStr )
{
    sal_Int64 nValue;
    if ( !ImplNumericGetValue( rStr, nValue, GetDecimalDigits(), ImplGetLocaleDataWrapper(), true ) )
        return sal_True;
    else
    {
        sal_Int64 nTempVal = nValue;
        if ( nTempVal > GetMax() )
            nTempVal = GetMax();
        else if ( nTempVal < GetMin())
            nTempVal = GetMin();

        if ( GetErrorHdl().IsSet() && (nValue != nTempVal) )
        {
            mnCorrectedValue = nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return sal_False;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = CreateFieldText( nTempVal );
        return sal_True;
    }
}

// -----------------------------------------------------------------------

inline void CurrencyFormatter::ImplInit()
{
    mnType = FORMAT_CURRENCY;
}

// -----------------------------------------------------------------------

CurrencyFormatter::CurrencyFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

CurrencyFormatter::~CurrencyFormatter()
{
}

// -----------------------------------------------------------------------

String CurrencyFormatter::GetCurrencySymbol() const
{
    return ImplGetLocaleDataWrapper().getCurrSymbol();
}

// -----------------------------------------------------------------------

void CurrencyFormatter::SetValue( sal_Int64 nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( sal_False );
}

// -----------------------------------------------------------------------

OUString CurrencyFormatter::CreateFieldText( sal_Int64 nValue ) const
{
    return ImplGetLocaleDataWrapper().getCurr( nValue, GetDecimalDigits(), GetCurrencySymbol(), IsUseThousandSep() );
}

// -----------------------------------------------------------------------

sal_Int64 CurrencyFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    sal_Int64 nTempValue;
    if ( ImplCurrencyGetValue( GetField()->GetText(), nTempValue, GetDecimalDigits(), ImplGetLocaleDataWrapper() ) )
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

// -----------------------------------------------------------------------

void CurrencyFormatter::Reformat()
{
    if ( !GetField() )
        return;

    OUString aStr;
    sal_Bool bOK = ImplCurrencyReformat( GetField()->GetText(), aStr );
    if ( !bOK )
        return;

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

// -----------------------------------------------------------------------

CurrencyField::CurrencyField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

CurrencyField::~CurrencyField()
{
}

// -----------------------------------------------------------------------

long CurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long CurrencyField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void CurrencyField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void CurrencyField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void CurrencyField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void CurrencyField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void CurrencyField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

void CurrencyField::Last()
{
    FieldLast();
    SpinField::Last();
}

// -----------------------------------------------------------------------

CurrencyBox::CurrencyBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

CurrencyBox::~CurrencyBox()
{
}

// -----------------------------------------------------------------------

long CurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsUseThousandSep(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long CurrencyBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void CurrencyBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        String sOldDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sOldThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        String sNewDecSep = ImplGetLocaleDataWrapper().getNumDecimalSep();
        String sNewThSep = ImplGetLocaleDataWrapper().getNumThousandSep();
        ImplUpdateSeparators( sOldDecSep, sNewDecSep, sOldThSep, sNewThSep, this );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void CurrencyBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void CurrencyBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplCurrencyReformat( GetEntry( i ), aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    CurrencyFormatter::Reformat();
    SetUpdateMode( sal_True );
}

// -----------------------------------------------------------------------

sal_Int64 CurrencyBox::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return CurrencyFormatter::GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
