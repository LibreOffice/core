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

#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <svl/zformat.hxx>
#include <svtools/fmtfield.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchResult.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <unotools/syslocale.hxx>
#include <map>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

// hmm. No support for regular expression. Well, I always (not really :) wanted to write a finite automat
// so here comes a finite automat ...

namespace validation
{
    // the states of our automat.
    enum State
    {
        START,              // at the very start of the string
        NUM_START,          // the very start of the number

        DIGIT_PRE_COMMA,    // some pre-comma digits are read, perhaps including some thousand separators

        DIGIT_POST_COMMA,   // reading digits after the comma
        EXPONENT_START,     // at the very start of the exponent value
                            //    (means: not including the "e" which denotes the exponent)
        EXPONENT_DIGIT,     // currently reading the digits of the exponent

        END                 // reached the end of the string
    };

    // a row in the transition table (means the set of states to be reached from a given state)
    typedef ::std::map< sal_Unicode, State >        StateTransitions;

    // a single transition
    typedef StateTransitions::value_type            Transition;

    // the complete transition table
    typedef ::std::map< State, StateTransitions >   TransitionTable;

    // the validator class
    class NumberValidator
    {
    private:
        TransitionTable     m_aTransitions;
        const sal_Unicode   m_cThSep;
        const sal_Unicode   m_cDecSep;

    public:
        NumberValidator( const sal_Unicode _cThSep, const sal_Unicode _cDecSep );

        bool isValidNumericFragment( const OUString& _rText );

    private:
        bool implValidateNormalized( const OUString& _rText );
    };

    static void lcl_insertStopTransition( StateTransitions& _rRow )
    {
        _rRow.insert( Transition( '_', END ) );
    }

    static void lcl_insertStartExponentTransition( StateTransitions& _rRow )
    {
        _rRow.insert( Transition( 'e', EXPONENT_START ) );
    }

    static void lcl_insertSignTransitions( StateTransitions& _rRow, const State eNextState )
    {
        _rRow.insert( Transition( '-', eNextState ) );
        _rRow.insert( Transition( '+', eNextState ) );
    }

    static void lcl_insertDigitTransitions( StateTransitions& _rRow, const State eNextState )
    {
        for ( sal_Unicode aChar = '0'; aChar <= '9'; ++aChar )
            _rRow.insert( Transition( aChar, eNextState ) );
    }

    static void lcl_insertCommonPreCommaTransitions( StateTransitions& _rRow, const sal_Unicode _cThSep, const sal_Unicode _cDecSep )
    {
        // digits are allowed
        lcl_insertDigitTransitions( _rRow, DIGIT_PRE_COMMA );

        // the thousand separator is allowed
        _rRow.insert( Transition( _cThSep, DIGIT_PRE_COMMA ) );

        // a comma is allowed
        _rRow.insert( Transition( _cDecSep, DIGIT_POST_COMMA ) );
    }

    NumberValidator::NumberValidator( const sal_Unicode _cThSep, const sal_Unicode _cDecSep )
        :m_cThSep( _cThSep )
        ,m_cDecSep( _cDecSep )
    {
        // build up our transition table

        // how to proceed from START
        {
            StateTransitions& rRow = m_aTransitions[ START ];
            rRow.insert( Transition( '_', NUM_START ) );
                // if we encounter the normalizing character, we want to proceed with the number
        }

        // how to proceed from NUM_START
        {
            StateTransitions& rRow = m_aTransitions[ NUM_START ];

            // a sign is allowed
            lcl_insertSignTransitions( rRow, DIGIT_PRE_COMMA );

            // common transitions for the two pre-comma states
            lcl_insertCommonPreCommaTransitions( rRow, m_cThSep, m_cDecSep );

            // the exponent may start here
            // (this would mean string like "_+e10_", but this is a valid fragment, though no valid number)
            lcl_insertStartExponentTransition( rRow );
        }

        // how to proceed from DIGIT_PRE_COMMA
        {
            StateTransitions& rRow = m_aTransitions[ DIGIT_PRE_COMMA ];

            // common transitions for the two pre-comma states
            lcl_insertCommonPreCommaTransitions( rRow, m_cThSep, m_cDecSep );

            // the exponent may start here
            lcl_insertStartExponentTransition( rRow );

            // the final transition indicating the end of the string
            // (if there is no comma and no post-comma, then the string may end here)
            lcl_insertStopTransition( rRow );
        }

        // how to proceed from DIGIT_POST_COMMA
        {
            StateTransitions& rRow = m_aTransitions[ DIGIT_POST_COMMA ];

            // there might be digits, which would keep the state at DIGIT_POST_COMMA
            lcl_insertDigitTransitions( rRow, DIGIT_POST_COMMA );

            // the exponent may start here
            lcl_insertStartExponentTransition( rRow );

            // the string may end here
            lcl_insertStopTransition( rRow );
        }

        // how to proceed from EXPONENT_START
        {
            StateTransitions& rRow = m_aTransitions[ EXPONENT_START ];

            // there may be a sign
            lcl_insertSignTransitions( rRow, EXPONENT_DIGIT );

            // there may be digits
            lcl_insertDigitTransitions( rRow, EXPONENT_DIGIT );

            // the string may end here
            lcl_insertStopTransition( rRow );
        }

        // how to proceed from EXPONENT_DIGIT
        {
            StateTransitions& rRow = m_aTransitions[ EXPONENT_DIGIT ];

            // there may be digits
            lcl_insertDigitTransitions( rRow, EXPONENT_DIGIT );

            // the string may end here
            lcl_insertStopTransition( rRow );
        }

        // how to proceed from END
        {
            /*StateTransitions& rRow =*/ m_aTransitions[ EXPONENT_DIGIT ];
            // no valid transition to leave this state
            // (note that we, for consistency, nevertheless want to have a row in the table)
        }
    }

    bool NumberValidator::implValidateNormalized( const OUString& _rText )
    {
        const sal_Unicode* pCheckPos = _rText.getStr();
        State eCurrentState = START;

        while ( END != eCurrentState )
        {
            // look up the transition row for the current state
            TransitionTable::const_iterator aRow = m_aTransitions.find( eCurrentState );
            DBG_ASSERT( m_aTransitions.end() != aRow,
                "NumberValidator::implValidateNormalized: invalid transition table (row not found)!" );

            if ( m_aTransitions.end() != aRow )
            {
                // look up the current character in this row
                StateTransitions::const_iterator aTransition = aRow->second.find( *pCheckPos );
                if ( aRow->second.end() != aTransition )
                {
                    // there is a valid transition for this character
                    eCurrentState = aTransition->second;
                    ++pCheckPos;
                    continue;
                }
            }

            // if we're here, there is no valid transition
            break;
        }

        DBG_ASSERT( ( END != eCurrentState ) || ( 0 == *pCheckPos ),
            "NumberValidator::implValidateNormalized: inconsistency!" );
            // if we're at END, then the string should be done, too - the string should be normalized, means ending
            // a "_" and not containing any other "_" (except at the start), and "_" is the only possibility
            // to reach the END state

        // the string is valid if and only if we reached the final state
        return ( END == eCurrentState );
    }

    bool NumberValidator::isValidNumericFragment( const OUString& _rText )
    {
        if ( _rText.isEmpty() )
            // empty strings are always allowed
            return true;

        // normalize the string
        OUString sNormalized( "_" );
        sNormalized += _rText;
        sNormalized += "_";

        return implValidateNormalized( sNormalized );
    }
}

SvNumberFormatter* FormattedField::StaticFormatter::s_cFormatter = nullptr;
sal_uLong FormattedField::StaticFormatter::s_nReferences = 0;

SvNumberFormatter* FormattedField::StaticFormatter::GetFormatter()
{
    if (!s_cFormatter)
    {
        // get the Office's locale and translate
        LanguageType eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);
        s_cFormatter = new SvNumberFormatter(
            ::comphelper::getProcessComponentContext(),
            eSysLanguage);
    }
    return s_cFormatter;
}

FormattedField::StaticFormatter::StaticFormatter()
{
    ++s_nReferences;
}

FormattedField::StaticFormatter::~StaticFormatter()
{
    if (--s_nReferences == 0)
    {
        delete s_cFormatter;
        s_cFormatter = nullptr;
    }
}


FormattedField::FormattedField(vcl::Window* pParent, WinBits nStyle, SvNumberFormatter* pInitialFormatter)
    :SpinField(pParent, nStyle)
    ,m_aLastSelection(0,0)
    ,m_dMinValue(0)
    ,m_dMaxValue(0)
    ,m_bHasMin(false)
    ,m_bHasMax(false)
    ,m_bStrictFormat(true)
    ,m_bEnableEmptyField(true)
    ,m_bAutoColor(false)
    ,m_bEnableNaN(false)
    ,m_ValueState(valueDirty)
    ,m_dCurrentValue(0)
    ,m_dDefaultValue(0)
    ,m_nFormatKey(0)
    ,m_pFormatter(nullptr)
    ,m_dSpinSize(1)
    ,m_dSpinFirst(-1000000)
    ,m_dSpinLast(1000000)
    ,m_bTreatAsNumber(true)
    ,m_pLastOutputColor(nullptr)
    ,m_bUseInputStringForFormatting(false)
{

    if (pInitialFormatter)
    {
        m_pFormatter = pInitialFormatter;
        m_nFormatKey = 0;
    }
}

VCL_BUILDER_FACTORY_ARGS(FormattedField, WB_BORDER | WB_SPIN)

void FormattedField::SetText(const OUString& rStr)
{

    SpinField::SetText(rStr);
    m_ValueState = valueDirty;
}

void FormattedField::SetText( const OUString& rStr, const Selection& rNewSelection )
{

    SpinField::SetText( rStr, rNewSelection );
    m_ValueState = valueDirty;
}

void FormattedField::SetTextFormatted(const OUString& rStr)
{

#if defined DBG_UTIL
    if (ImplGetFormatter()->IsTextFormat(m_nFormatKey))
         SAL_INFO("svtools", "FormattedField::SetTextFormatted : valid only with text formats !");
#endif

    m_sCurrentTextValue = rStr;

    OUString sFormatted;
    double dNumber = 0.0;
    // IsNumberFormat changes the format key parameter
    sal_uInt32 nTempFormatKey = static_cast< sal_uInt32 >( m_nFormatKey );
    if( IsUsingInputStringForFormatting() &&
        ImplGetFormatter()->IsNumberFormat(m_sCurrentTextValue, nTempFormatKey, dNumber) )
    {
        ImplGetFormatter()->GetInputLineString(dNumber, m_nFormatKey, sFormatted);
    }
    else
    {
        ImplGetFormatter()->GetOutputString(m_sCurrentTextValue,
                                            m_nFormatKey,
                                            sFormatted,
                                            &m_pLastOutputColor);
    }

    // calculate the new selection
    Selection aSel(GetSelection());
    Selection aNewSel(aSel);
    aNewSel.Justify();
    sal_Int32 nNewLen = sFormatted.getLength();
    sal_Int32 nCurrentLen = GetText().getLength();
    if ((nNewLen > nCurrentLen) && (aNewSel.Max() == nCurrentLen))
    {   // the new text is longer and the cursor was behind the last char (of the old text)
        if (aNewSel.Min() == 0)
        {   // the whole text was selected -> select the new text on the whole, too
            aNewSel.Max() = nNewLen;
            if (!nCurrentLen)
            {   // there wasn't really a previous selection (as there was no previous text), we're setting a new one -> check the selection options
                SelectionOptions nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
                if (nSelOptions & SelectionOptions::ShowFirst)
                {   // selection should be from right to left -> swap min and max
                    aNewSel.Min() = aNewSel.Max();
                    aNewSel.Max() = 0;
                }
            }
        }
        else if (aNewSel.Max() == aNewSel.Min())
        {   // there was no selection -> set the cursor behind the new last char
            aNewSel.Max() = nNewLen;
            aNewSel.Min() = nNewLen;
        }
    }
    else if (aNewSel.Max() > nNewLen)
        aNewSel.Max() = nNewLen;
    else
        aNewSel = aSel; // don't use the justified version
    SpinField::SetText(sFormatted, aNewSel);
    m_ValueState = valueString;
}

OUString const & FormattedField::GetTextValue() const
{
    if (m_ValueState != valueString )
    {
        const_cast<FormattedField*>(this)->m_sCurrentTextValue = GetText();
        const_cast<FormattedField*>(this)->m_ValueState = valueString;
    }
    return m_sCurrentTextValue;
}

void FormattedField::EnableNotANumber( bool _bEnable )
{
    if ( m_bEnableNaN == _bEnable )
        return;

    m_bEnableNaN = _bEnable;
}

void FormattedField::SetAutoColor(bool _bAutomatic)
{
    if (_bAutomatic == m_bAutoColor)
        return;

    m_bAutoColor = _bAutomatic;
    if (m_bAutoColor)
    {   // if auto color is switched on, adjust the current text color, too
        if (m_pLastOutputColor)
            SetControlForeground(*m_pLastOutputColor);
        else
            SetControlForeground();
    }
}

void FormattedField::impl_Modify(bool makeValueDirty)
{

    if (!IsStrictFormat())
    {
        if(makeValueDirty)
            m_ValueState = valueDirty;
        SpinField::Modify();
        return;
    }

    OUString sCheck = GetText();
    if (CheckText(sCheck))
    {
        m_sLastValidText = sCheck;
        m_aLastSelection = GetSelection();
        if(makeValueDirty)
            m_ValueState = valueDirty;
    }
    else
    {
        ImplSetTextImpl(m_sLastValidText, &m_aLastSelection);
    }

    SpinField::Modify();
}

void FormattedField::Modify()
{

    impl_Modify();
}

void FormattedField::ImplSetTextImpl(const OUString& rNew, Selection* pNewSel)
{

    if (m_bAutoColor)
    {
        if (m_pLastOutputColor)
            SetControlForeground(*m_pLastOutputColor);
        else
            SetControlForeground();
    }

    if (pNewSel)
        SpinField::SetText(rNew, *pNewSel);
    else
    {
        Selection aSel(GetSelection());
        aSel.Justify();

        sal_Int32 nNewLen = rNew.getLength();
        sal_Int32 nCurrentLen = GetText().getLength();

        if ((nNewLen > nCurrentLen) && (aSel.Max() == nCurrentLen))
        {   // new new text is longer and the cursor is behind the last char
            if (aSel.Min() == 0)
            {   // the whole text was selected -> select the new text on the whole, too
                aSel.Max() = nNewLen;
                if (!nCurrentLen)
                {   // there wasn't really a previous selection (as there was no previous text), we're setting a new one -> check the selection options
                    SelectionOptions nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
                    if (nSelOptions & SelectionOptions::ShowFirst)
                    {   // selection should be from right to left -> swap min and max
                        aSel.Min() = aSel.Max();
                        aSel.Max() = 0;
                    }
                }
            }
            else if (aSel.Max() == aSel.Min())
            {   // there was no selection -> set the cursor behind the new last char
                aSel.Max() = nNewLen;
                aSel.Min() = nNewLen;
            }
        }
        else if (aSel.Max() > nNewLen)
            aSel.Max() = nNewLen;
        SpinField::SetText(rNew, aSel);
    }

    m_ValueState = valueDirty; // not always necessary, but better re-evaluate for safety reasons
}

bool FormattedField::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
        m_aLastSelection = GetSelection();
    return SpinField::PreNotify(rNEvt);
}

void FormattedField::ImplSetFormatKey(sal_uLong nFormatKey)
{

    m_nFormatKey = nFormatKey;
    bool bNeedFormatter = (m_pFormatter == nullptr) && (nFormatKey != 0);
    if (bNeedFormatter)
    {
        ImplGetFormatter(); // this creates a standard formatter

        // It might happen that the standard formatter makes no sense here, but it takes a default
        // format. Thus, it is possible to set one of the other standard keys (which are spanning
        // across multiple formatters).
        m_nFormatKey = nFormatKey;
        // When calling SetFormatKey without a formatter, the key must be one of the standard values
        // that is available for all formatters (and, thus, also in this new one).
        DBG_ASSERT(m_pFormatter->GetEntry(nFormatKey) != nullptr, "FormattedField::ImplSetFormatKey : invalid format key !");
    }
}

void FormattedField::SetFormatKey(sal_uLong nFormatKey)
{
    bool bNoFormatter = (m_pFormatter == nullptr);
    ImplSetFormatKey(nFormatKey);
    FormatChanged((bNoFormatter && (m_pFormatter != nullptr)) ? FORMAT_CHANGE_TYPE::FORMATTER : FORMAT_CHANGE_TYPE::KEYONLY);
}

void FormattedField::SetFormatter(SvNumberFormatter* pFormatter, bool bResetFormat)
{

    if (bResetFormat)
    {
        m_pFormatter = pFormatter;

        // calc the default format key from the Office's UI locale
        if ( m_pFormatter )
        {
            // get the Office's locale and translate
            LanguageType eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);
            // get the standard numeric format for this language
            m_nFormatKey = m_pFormatter->GetStandardFormat( css::util::NumberFormat::NUMBER, eSysLanguage );
        }
        else
            m_nFormatKey = 0;
    }
    else
    {
        LanguageType aOldLang;
        OUString sOldFormat = GetFormat(aOldLang);

        sal_uInt32 nDestKey = pFormatter->TestNewString(sOldFormat);
        if (nDestKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            // language of the new formatter
            const SvNumberformat* pDefaultEntry = pFormatter->GetEntry(0);
            LanguageType aNewLang = pDefaultEntry ? pDefaultEntry->GetLanguage() : LANGUAGE_DONTKNOW;

            // convert the old format string into the new language
            sal_Int32 nCheckPos;
            short nType;
            pFormatter->PutandConvertEntry(sOldFormat, nCheckPos, nType, nDestKey, aOldLang, aNewLang);
            m_nFormatKey = nDestKey;
        }
        m_pFormatter = pFormatter;
    }

    FormatChanged(FORMAT_CHANGE_TYPE::FORMATTER);
}

OUString FormattedField::GetFormat(LanguageType& eLang) const
{
    const SvNumberformat* pFormatEntry = ImplGetFormatter()->GetEntry(m_nFormatKey);
    DBG_ASSERT(pFormatEntry != nullptr, "FormattedField::GetFormat: no number format for the given format key.");
    OUString sFormatString = pFormatEntry ? pFormatEntry->GetFormatstring() : OUString();
    eLang = pFormatEntry ? pFormatEntry->GetLanguage() : LANGUAGE_DONTKNOW;

    return sFormatString;
}

bool FormattedField::SetFormat(const OUString& rFormatString, LanguageType eLang)
{
    sal_uInt32 nNewKey = ImplGetFormatter()->TestNewString(rFormatString, eLang);
    if (nNewKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        sal_Int32 nCheckPos;
        short nType;
        OUString rFormat(rFormatString);
        if (!ImplGetFormatter()->PutEntry(rFormat, nCheckPos, nType, nNewKey, eLang))
            return false;
        DBG_ASSERT(nNewKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "FormattedField::SetFormatString : PutEntry returned an invalid key !");
    }

    if (nNewKey != m_nFormatKey)
        SetFormatKey(nNewKey);
    return true;
}

bool FormattedField::GetThousandsSep() const
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::GetThousandsSep : Are you sure what you are doing when setting the precision of a text format?");

    bool bThousand, IsRed;
    sal_uInt16 nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);

    return bThousand;
}

void FormattedField::SetThousandsSep(bool _bUseSeparator)
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::SetThousandsSep : Are you sure what you are doing when setting the precision of a text format?");

    // get the current settings
    bool bThousand, IsRed;
    sal_uInt16 nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);
    if (bThousand == (bool)_bUseSeparator)
        return;

    // we need the language for the following
    LanguageType eLang;
    GetFormat(eLang);

    // generate a new format ...
    OUString sFmtDescription = ImplGetFormatter()->GenerateFormat(m_nFormatKey, eLang, _bUseSeparator, IsRed, nPrecision, nAnzLeading);
    // ... and introduce it to the formatter
    sal_Int32 nCheckPos = 0;
    sal_uInt32 nNewKey;
    short nType;
    ImplGetFormatter()->PutEntry(sFmtDescription, nCheckPos, nType, nNewKey, eLang);

    // set the new key
    ImplSetFormatKey(nNewKey);
    FormatChanged(FORMAT_CHANGE_TYPE::THOUSANDSSEP);
}

sal_uInt16 FormattedField::GetDecimalDigits() const
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::GetDecimalDigits : Are you sure what you are doing when setting the precision of a text format?");

    bool bThousand, IsRed;
    sal_uInt16 nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);

    return nPrecision;
}

void FormattedField::SetDecimalDigits(sal_uInt16 _nPrecision)
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::SetDecimalDigits : Are you sure what you are doing when setting the precision of a text format?");

    // get the current settings
    bool bThousand, IsRed;
    sal_uInt16 nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);
    if (nPrecision == _nPrecision)
        return;

    // we need the language for the following
    LanguageType eLang;
    GetFormat(eLang);

    // generate a new format ...
    OUString sFmtDescription = ImplGetFormatter()->GenerateFormat(m_nFormatKey, eLang, bThousand, IsRed, _nPrecision, nAnzLeading);
    // ... and introduce it to the formatter
    sal_Int32 nCheckPos = 0;
    sal_uInt32 nNewKey;
    short nType;
    ImplGetFormatter()->PutEntry(sFmtDescription, nCheckPos, nType, nNewKey, eLang);

    // set the new key
    ImplSetFormatKey(nNewKey);
    FormatChanged(FORMAT_CHANGE_TYPE::PRECISION);
}

void FormattedField::FormatChanged( FORMAT_CHANGE_TYPE _nWhat )
{
    m_pLastOutputColor = nullptr;

    if ( (_nWhat == FORMAT_CHANGE_TYPE::FORMATTER) && m_pFormatter )
        m_pFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );

    ReFormat();
}

void FormattedField::Commit()
{
    // remember the old text
    OUString sOld( GetText() );

    // do the reformat
    ReFormat();

    // did the text change?
    if ( GetText() != sOld )
    {   // consider the field as modified,
        // but we already have the most recent value;
        // don't reparse it from the text
        // (can lead to data loss when the format is lossy,
        //  as is e.g. our default date format: 2-digit year!)
        impl_Modify(false);
    }
}

void FormattedField::ReFormat()
{
    if (!IsEmptyFieldEnabled() || !GetText().isEmpty())
    {
        if (TreatingAsNumber())
        {
            double dValue = GetValue();
            if ( m_bEnableNaN && ::rtl::math::isNan( dValue ) )
                return;
            ImplSetValue( dValue, true );
        }
        else
            SetTextFormatted(GetTextValue());
    }
}

bool FormattedField::Notify(NotifyEvent& rNEvt)
{

    if ((rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !IsReadOnly())
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        sal_uInt16 nMod = rKEvt.GetKeyCode().GetModifier();
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
                if (!nMod && ImplGetFormatter()->IsTextFormat(m_nFormatKey))
                {
                    // the base class would translate this into calls to Up/Down/First/Last,
                    // but we don't want this if we are text-formatted
                    return true;
                }
        }
    }

    if ((rNEvt.GetType() == MouseNotifyEvent::COMMAND) && !IsReadOnly())
    {
        const CommandEvent* pCommand = rNEvt.GetCommandEvent();
        if (pCommand->GetCommand() == CommandEventId::Wheel)
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ((pData->GetMode() == CommandWheelMode::SCROLL) && ImplGetFormatter()->IsTextFormat(m_nFormatKey))
            {
                // same as above : prevent the base class from doing Up/Down-calls
                // (normally I should put this test into the Up/Down methods itself, shouldn't I ?)
                // FS - 71553 - 19.01.00
                return true;
            }
        }
    }

    if (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS)
    {
        // Sonderbehandlung fuer leere Texte
        if (GetText().isEmpty())
        {
            if (!IsEmptyFieldEnabled())
            {
                if (TreatingAsNumber())
                {
                    ImplSetValue(m_dCurrentValue, true);
                    Modify();
                    m_ValueState = valueDouble;
                }
                else
                {
                    OUString sNew = GetTextValue();
                    if (!sNew.isEmpty())
                        SetTextFormatted(sNew);
                    else
                        SetTextFormatted(m_sDefaultText);
                    m_ValueState = valueString;
                }
            }
        }
        else
        {
            Commit();
        }
    }

    return SpinField::Notify( rNEvt );
}

void FormattedField::SetMinValue(double dMin)
{
    DBG_ASSERT(m_bTreatAsNumber, "FormattedField::SetMinValue : only to be used in numeric mode !");

    m_dMinValue = dMin;
    m_bHasMin = true;
    // for checking the current value at the new border -> ImplSetValue
    ReFormat();
}

void FormattedField::SetMaxValue(double dMax)
{
    DBG_ASSERT(m_bTreatAsNumber, "FormattedField::SetMaxValue : only to be used in numeric mode !");

    m_dMaxValue = dMax;
    m_bHasMax = true;
    // for checking the current value at the new border -> ImplSetValue
    ReFormat();
}

void FormattedField::SetTextValue(const OUString& rText)
{
    SetText(rText);
    ReFormat();
}

void FormattedField::EnableEmptyField(bool bEnable)
{
    if (bEnable == m_bEnableEmptyField)
        return;

    m_bEnableEmptyField = bEnable;
    if (!m_bEnableEmptyField && GetText().isEmpty())
        ImplSetValue(m_dCurrentValue, true);
}

void FormattedField::ImplSetValue(double dVal, bool bForce)
{

    if (m_bHasMin && (dVal<m_dMinValue))
        dVal = m_dMinValue;
    if (m_bHasMax && (dVal>m_dMaxValue))
        dVal = m_dMaxValue;
    if (!bForce && (dVal == GetValue()))
        return;

    DBG_ASSERT(ImplGetFormatter() != nullptr, "FormattedField::ImplSetValue : can't set a value without a formatter !");

    m_ValueState = valueDouble;
    m_dCurrentValue = dVal;

    OUString sNewText;
    if (ImplGetFormatter()->IsTextFormat(m_nFormatKey))
    {
        // first convert the number as string in standard format
        OUString sTemp;
        ImplGetFormatter()->GetOutputString(dVal, 0, sTemp, &m_pLastOutputColor);
        // then encode the string in the corresponding text format
        ImplGetFormatter()->GetOutputString(sTemp, m_nFormatKey, sNewText, &m_pLastOutputColor);
    }
    else
    {
        if( IsUsingInputStringForFormatting())
        {
            ImplGetFormatter()->GetInputLineString(dVal, m_nFormatKey, sNewText);
        }
        else
        {
            ImplGetFormatter()->GetOutputString(dVal, m_nFormatKey, sNewText, &m_pLastOutputColor);
        }
    }

    ImplSetTextImpl(sNewText, nullptr);
    m_ValueState = valueDouble;
    DBG_ASSERT(CheckText(sNewText), "FormattedField::ImplSetValue : formatted string doesn't match the criteria !");
}

bool FormattedField::ImplGetValue(double& dNewVal)
{

    dNewVal = m_dCurrentValue;
    if (m_ValueState == valueDouble)
        return true;

    dNewVal = m_dDefaultValue;
    OUString sText(GetText());
    if (sText.isEmpty())
        return true;

    DBG_ASSERT(ImplGetFormatter() != nullptr, "FormattedField::ImplGetValue : can't give you a current value without a formatter !");

    sal_uInt32 nFormatKey = m_nFormatKey; // IsNumberFormat changes the FormatKey!

    if (ImplGetFormatter()->IsTextFormat(nFormatKey) && m_bTreatAsNumber)
        // for detection of values like "1,1" in fields that are formatted as text
        nFormatKey = 0;

    // special treatment for percentage formatting
    if (ImplGetFormatter()->GetType(m_nFormatKey) == css::util::NumberFormat::PERCENT)
    {
        // the language of our format
        LanguageType eLanguage = m_pFormatter->GetEntry(m_nFormatKey)->GetLanguage();
        // the default number format for this language
        sal_uLong nStandardNumericFormat = m_pFormatter->GetStandardFormat(css::util::NumberFormat::NUMBER, eLanguage);

        sal_uInt32 nTempFormat = nStandardNumericFormat;
        double dTemp;
        if (m_pFormatter->IsNumberFormat(sText, nTempFormat, dTemp) &&
            css::util::NumberFormat::NUMBER == m_pFormatter->GetType(nTempFormat))
            // the string is equivalent to a number formatted one (has no % sign) -> append it
            sText += "%";
        // (with this, a input of '3' becomes '3%', which then by the formatter is translated
        // into 0.03. Without this, the formatter would give us the double 3 for an input '3',
        // which equals 300 percent.
    }
    if (!ImplGetFormatter()->IsNumberFormat(sText, nFormatKey, dNewVal))
        return false;

    if (m_bHasMin && (dNewVal<m_dMinValue))
        dNewVal = m_dMinValue;
    if (m_bHasMax && (dNewVal>m_dMaxValue))
        dNewVal = m_dMaxValue;
    return true;
}

void FormattedField::SetValue(double dVal)
{
    ImplSetValue(dVal, m_ValueState != valueDouble);
}

double FormattedField::GetValue()
{

    if ( !ImplGetValue( m_dCurrentValue ) )
    {
        if ( m_bEnableNaN )
            ::rtl::math::setNan( &m_dCurrentValue );
        else
            m_dCurrentValue = m_dDefaultValue;
    }

    m_ValueState = valueDouble;
    return m_dCurrentValue;
}

void FormattedField::Up()
{
    // setValue handles under- and overflows (min/max) automatically
    SetValue(GetValue() + m_dSpinSize);
    SetModifyFlag();
    Modify();

    SpinField::Up();
}

void FormattedField::Down()
{
    SetValue(GetValue() - m_dSpinSize);
    SetModifyFlag();
    Modify();

    SpinField::Down();
}

void FormattedField::First()
{
    if (m_bHasMin)
    {
        SetValue(m_dMinValue);
        SetModifyFlag();
        Modify();
    }

    SpinField::First();
}

void FormattedField::Last()
{
    if (m_bHasMax)
    {
        SetValue(m_dMaxValue);
        SetModifyFlag();
        Modify();
    }

    SpinField::Last();
}

void FormattedField::UseInputStringForFormatting()
{
    m_bUseInputStringForFormatting = true;
}


DoubleNumericField::~DoubleNumericField()
{
    disposeOnce();
}

void DoubleNumericField::dispose()
{
    delete m_pNumberValidator;
    FormattedField::dispose();
}

void DoubleNumericField::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    ResetConformanceTester();
    FormattedField::FormatChanged(nWhat);
}

bool DoubleNumericField::CheckText(const OUString& sText) const
{
    // We'd like to implement this using the NumberFormatter::IsNumberFormat, but unfortunately, this doesn't
    // recognize fragments of numbers (like, for instance "1e", which happens during entering e.g. "1e10")
    // Thus, the roundabout way via a regular expression
    return m_pNumberValidator->isValidNumericFragment( sText );
}

void DoubleNumericField::ResetConformanceTester()
{
    // the thousands and the decimal separator are language dependent
    const SvNumberformat* pFormatEntry = ImplGetFormatter()->GetEntry(m_nFormatKey);

    sal_Unicode cSeparatorThousand = ',';
    sal_Unicode cSeparatorDecimal = '.';
    if (pFormatEntry)
    {
        LocaleDataWrapper aLocaleInfo( LanguageTag( pFormatEntry->GetLanguage()) );

        OUString sSeparator = aLocaleInfo.getNumThousandSep();
        if (!sSeparator.isEmpty())
            cSeparatorThousand = sSeparator[0];

        sSeparator = aLocaleInfo.getNumDecimalSep();
        if (!sSeparator.isEmpty())
            cSeparatorDecimal = sSeparator[0];
    }

    delete m_pNumberValidator;
    m_pNumberValidator = new validation::NumberValidator( cSeparatorThousand, cSeparatorDecimal );
}

DoubleCurrencyField::DoubleCurrencyField(vcl::Window* pParent, WinBits nStyle)
    :FormattedField(pParent, nStyle)
    ,m_bChangingFormat(false)
{
    m_bPrependCurrSym = false;

    // initialize with a system currency format
    m_sCurrencySymbol = SvtSysLocale().GetLocaleData().getCurrSymbol();
    UpdateCurrencyFormat();
}

void DoubleCurrencyField::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    if (m_bChangingFormat)
    {
        FormattedField::FormatChanged(nWhat);
        return;
    }

    switch (nWhat)
    {
        case FORMAT_CHANGE_TYPE::FORMATTER:
        case FORMAT_CHANGE_TYPE::PRECISION:
        case FORMAT_CHANGE_TYPE::THOUSANDSSEP:
            // the aspects which changed don't take our currency settings into account (in fact, they most probably
            // destroyed them)
            UpdateCurrencyFormat();
            break;
        case FORMAT_CHANGE_TYPE::KEYONLY:
            OSL_FAIL("DoubleCurrencyField::FormatChanged : somebody modified my key !");
            // We always build our own format from the settings we get via special methods (setCurrencySymbol etc.).
            // Nobody but ourself should modifiy the format key directly !
            break;
        default: break;
    }

    FormattedField::FormatChanged(nWhat);
}

void DoubleCurrencyField::setCurrencySymbol(const OUString& rSymbol)
{
    if (m_sCurrencySymbol == rSymbol)
        return;

    m_sCurrencySymbol = rSymbol;
    UpdateCurrencyFormat();
    FormatChanged(FORMAT_CHANGE_TYPE::CURRENCY_SYMBOL);
}

void DoubleCurrencyField::setPrependCurrSym(bool _bPrepend)
{
    if (m_bPrependCurrSym == _bPrepend)
         return;

    m_bPrependCurrSym = _bPrepend;
    UpdateCurrencyFormat();
    FormatChanged(FORMAT_CHANGE_TYPE::CURRSYM_POSITION);
}

void DoubleCurrencyField::UpdateCurrencyFormat()
{
    // the old settings
    LanguageType eLanguage;
    GetFormat(eLanguage);
    bool bThSep = GetThousandsSep();
    sal_uInt16 nDigits = GetDecimalDigits();

    // build a new format string with the base class' and my own settings

    /* Strangely with gcc 4.6.3 this needs a temporary LanguageTag, otherwise
     * there's
     * error: request for member 'getNumThousandSep' in 'aLocaleInfo', which is
     * of non-class type 'LocaleDataWrapper(LanguageTag)' */
    LanguageTag aLanguageTag( eLanguage);
    LocaleDataWrapper aLocaleInfo( aLanguageTag );

    OUStringBuffer sNewFormat;
    if (bThSep)
    {
        sNewFormat.append('#');
        sNewFormat.append(aLocaleInfo.getNumThousandSep());
        sNewFormat.append("##0");
    }
    else
        sNewFormat.append('0');

    if (nDigits)
    {
        sNewFormat.append(aLocaleInfo.getNumDecimalSep());

        OUStringBuffer sTemp;
        comphelper::string::padToLength(sTemp, nDigits, '0');
        sNewFormat.append(sTemp);
    }

    if (getPrependCurrSym())
    {
        OUString sSymbol = getCurrencySymbol();
        sSymbol = comphelper::string::stripStart(sSymbol, ' ');
        sSymbol = comphelper::string::stripEnd(sSymbol, ' ');

        OUStringBuffer sTemp("[$");
        sTemp.append(sSymbol);
        sTemp.append("] ");
        sTemp.append(sNewFormat);

        // for negative values : $ -0.00, not -$ 0.00 ...
        // (the real solution would be a possibility to choose a "positive currency format" and a "negative currency format" ...
        // But not now ... (and hey, you could take a formatted field for this ....))
        // FS - 31.03.00 74642
        sTemp.append(";[$");
        sTemp.append(sSymbol);
        sTemp.append("] -");
        sTemp.append(sNewFormat);

        sNewFormat = sTemp;
    }
    else
    {
        OUString sTemp = getCurrencySymbol();
        sTemp = comphelper::string::stripStart(sTemp, ' ');
        sTemp = comphelper::string::stripEnd(sTemp, ' ');

        sNewFormat.append(" [$");
        sNewFormat.append(sTemp);
        sNewFormat.append(']');
    }

    // set this new basic format
    m_bChangingFormat = true;
    SetFormat(sNewFormat.makeStringAndClear(), eLanguage);
    m_bChangingFormat = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
