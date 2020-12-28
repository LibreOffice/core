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

#pragma once

#include <config_options.h>
#include <i18nlangtag/lang.h>
#include <tools/link.hxx>
#include <vcl/settings.hxx>
#include <map>
#include <memory>
#include <string_view>

class SvNumberFormatter;

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

    public:
        NumberValidator( const sal_Unicode _cThSep, const sal_Unicode _cDecSep );

        bool isValidNumericFragment( std::u16string_view _rText );

    private:
        bool implValidateNormalized( const OUString& _rText );
    };
}

enum class FORMAT_CHANGE_TYPE
{
    KEYONLY           = 0x00,        // only a new key was set
    FORMATTER         = 0x01,        // a new formatter was set, usually implies a change of the key, too
    PRECISION         = 0x02,        // a new precision was set
    THOUSANDSSEP      = 0x03,        // the thousands separator setting changed
    CURRENCY_SYMBOL   = 0x10,
    CURRSYM_POSITION  = 0x20,
};

class VCL_DLLPUBLIC Formatter
{
private:
    // A SvNumberFormatter is very expensive (regarding time and space), it is a Singleton
    class StaticFormatter
    {
        static SvNumberFormatter*   s_cFormatter;
        static sal_uLong                s_nReferences;
    public:
        StaticFormatter();
        ~StaticFormatter();

        operator SvNumberFormatter* () { return GetFormatter(); }
        UNLESS_MERGELIBS(VCL_DLLPUBLIC) static SvNumberFormatter* GetFormatter();
    };

protected:
    OUString      m_sLastValidText;
    // Has nothing to do with the current value. It is the last text, which was valid at input (checked by CheckText,
    // not yet through formatter)
    Selection   m_aLastSelection;

    double              m_dMinValue;
    double              m_dMaxValue;
    bool                m_bHasMin : 1;
    bool                m_bHasMax : 1;

    bool                m_bWrapOnLimits : 1;
    bool                m_bStrictFormat : 1;

    bool                m_bEnableEmptyField : 1;
    bool                m_bAutoColor : 1;
    bool                m_bEnableNaN : 1;
    bool                m_bDisableRemainderFactor : 1;
    enum valueState { valueDirty, valueString, valueDouble };
    valueState          m_ValueState;
    double              m_dCurrentValue;
    double              m_dDefaultValue;

    sal_uLong               m_nFormatKey;
    SvNumberFormatter*  m_pFormatter;
    StaticFormatter     m_aStaticFormatter;

    double              m_dSpinSize;
    double              m_dSpinFirst;
    double              m_dSpinLast;

    // There is a difference, when text formatting is enabled, if LostFocus formats the current String and displays it,
    // or if a double is created from the String and then
    bool                m_bTreatAsNumber;
    // And with the following members we can use it for formatted text output as well ...
    OUString            m_sCurrentTextValue;
    OUString            m_sDefaultText;

    // The last color from the Formatter at the last output operation (not we would use it, but you can get it)
    const Color*              m_pLastOutputColor;

    bool                m_bUseInputStringForFormatting;

    Link<sal_Int64*, TriState> m_aInputHdl;
    Link<LinkParamNone*, bool> m_aOutputHdl;

public:
    Formatter();
    virtual ~Formatter();

    void SetFieldText(const OUString& rText, const Selection& rNewSelection);

    virtual Selection GetEntrySelection() const = 0;
    virtual OUString GetEntryText() const = 0;
    virtual SelectionOptions GetEntrySelectionOptions() const = 0;
    virtual void SetEntryText(const OUString& rText, const Selection& rSel) = 0;
    virtual void SetEntryTextColor(const Color* pColor) = 0;
    virtual void FieldModified() = 0;

    // Min-/Max-management
    bool    HasMinValue() const         { return m_bHasMin; }
    virtual void ClearMinValue()        { m_bHasMin = false; }
    virtual void SetMinValue(double dMin);
    double  GetMinValue() const         { return m_dMinValue; }

    bool    HasMaxValue() const         { return m_bHasMax; }
    virtual void ClearMaxValue()             { m_bHasMax = false; }
    virtual void SetMaxValue(double dMax);
    double  GetMaxValue() const         { return m_dMaxValue; }

    // Current value
    void    SetValue(double dVal);
    double  GetValue();
    // The default implementation uses a formatter, if available

    void    SetTextValue(const OUString& rText);
    // The String is transformed to a double (with a formatter) and SetValue is called afterwards

    bool    IsEmptyFieldEnabled() const         { return m_bEnableEmptyField; }
    void    EnableEmptyField(bool bEnable);
    // If disabled, the value will be reset to the last valid value on leave

    void    SetDefaultValue(double dDefault)    { m_dDefaultValue = dDefault; m_ValueState = valueDirty; }
    // If the current String is invalid, GetValue() returns this value
    double  GetDefaultValue() const             { return m_dDefaultValue; }

    void SetLastSelection(const Selection& rSelection) { m_aLastSelection = rSelection; }

    // Settings for the format
    sal_uLong   GetFormatKey() const                { return m_nFormatKey; }
    void    SetFormatKey(sal_uLong nFormatKey);

    SvNumberFormatter*  GetOrCreateFormatter() const { return m_pFormatter ? m_pFormatter : const_cast<Formatter*>(this)->CreateFormatter(); }

    SvNumberFormatter*  GetFormatter() const    { return m_pFormatter; }
    void    SetFormatter(SvNumberFormatter* pFormatter, bool bResetFormat = true);
    // If bResetFormat is sal_False, the old format is tried to be kept. (expensive, if it is no default format, available in all formatters)
    // If sal_True, the new FormatKey is set to zero

    bool    GetThousandsSep() const;
    void    SetThousandsSep(bool _bUseSeparator);
        // the is no check if the current format is numeric, so be cautious when calling these functions

    void    DisableRemainderFactor();
    bool    GetDisableRemainderFactor() const { return m_bDisableRemainderFactor; }

    void    SetWrapOnLimits(bool bWrapOnLimits) { m_bWrapOnLimits = bWrapOnLimits; }

    sal_uInt16  GetDecimalDigits() const;
    void    SetDecimalDigits(sal_uInt16 _nPrecision);
        // There is no check if the current format is numeric, so be cautious when calling these functions

    SvNumberFormatter*  StandardFormatter() { return m_aStaticFormatter; }
    // If no new Formatter is created explicitly, this can be used in SetFormatter...

    OUString    GetFormat(LanguageType& eLang) const;
    bool        SetFormat(const OUString& rFormatString, LanguageType eLang);
    // sal_False, if the FormatString could not be set (and very probably is invalid)
    // This Object is shared via all instances, so be careful!

    bool    IsStrictFormat() const              { return m_bStrictFormat; }
    void    SetStrictFormat(bool bEnable)       { m_bStrictFormat = bEnable; }
    // Check format during input

    virtual void SetSpinSize(double dStep)   { m_dSpinSize = dStep; }
    double  GetSpinSize() const         { return m_dSpinSize; }

    void    SetSpinFirst(double dFirst) { m_dSpinFirst = dFirst; }
    double  GetSpinFirst() const        { return m_dSpinFirst; }

    void    SetSpinLast(double dLast)   { m_dSpinLast = dLast; }
    double  GetSpinLast() const         { return m_dSpinLast; }

    bool    TreatingAsNumber() const    { return m_bTreatAsNumber; }
    void    TreatAsNumber(bool bDoSo) { m_bTreatAsNumber = bDoSo; }

    void    SetInputHdl(const Link<sal_Int64*,TriState>& rLink) { m_aInputHdl = rLink; }
    void    SetOutputHdl(const Link<LinkParamNone*, bool>& rLink) { m_aOutputHdl = rLink; }
public:

    //The following methods are interesting, if m_bTreatAsNumber is set to sal_False
    //If someone does not care about all the double handling and just wants to print the text formatted.
    //(((The text will be formatted, using the Formatter, and then set)
    void SetTextFormatted(const OUString& rText);
    OUString const & GetTextValue() const;

    void      SetDefaultText(const OUString& rDefault) { m_sDefaultText = rDefault; }
    const OUString& GetDefaultText() const { return m_sDefaultText; }

    // The last colour from the Formatter's last output operation. Output operations get triggered by:
    // SetValue, SetTextValue, SetTextFormatted, also indirectly via SetMin - / -MaxValue
    const Color*  GetLastOutputColor() const { return m_pLastOutputColor; }

    /** reformats the current text. Interesting if the user entered some text in an "input format", and
        this should be formatted in the "output format" (which may differ, e.g. by additional numeric
        digits or such).
    */
    void    Commit();

    // enable automatic coloring. if set to sal_True, and the format the field is working with for any current value
    // says that it has to be painted in a special color (e.g. a format where negative numbers should be printed
    // red), the text is painted with that color automatically.
    // The color used is the same as returned by GetLastOutputColor()
    void    SetAutoColor(bool _bAutomatic);

    /** enables handling of not-a-number value.

        When this is set to <FALSE/> (the default), then invalid inputs (i.e. text which cannot be
        interpreted, according to the current formatting) will be handled as if the default value
        has been entered. GetValue the will return this default value.

        When set to <TRUE/>, then GetValue will return NaN (not a number, see <method scope="rtl::math">isNan</method>)
        when the current input is invalid.

        Note that setting this to <TRUE/> implies that upon leaving the control, the input
        will *not* be corrected to a valid value. For example, if the user enters "foo" in the
        control, and then tabs out of it, the text "foo" will persist, and GetValue will
        return NaN in subsequent calls.
    */
    void    EnableNotANumber( bool _bEnable );

    /** When being set to true, the strings in the field are formatted using the
        InputLine format.  That's also what you get in Calc when you edit a cell
        using F2
     */
    void    UseInputStringForFormatting();
    bool    IsUsingInputStringForFormatting() const { return m_bUseInputStringForFormatting;}

    void    Modify(bool makeValueDirty = true);

    void    EntryLostFocus();

    void    ReFormat();

    // any aspect of the current format has changed
    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat);

protected:

    // Override CheckText for input-time checks
    virtual bool CheckText(const OUString&) const { return true; }

    void ImplSetTextImpl(const OUString& rNew, Selection const * pNewSel);
    void ImplSetValue(double dValue, bool bForce);
    bool ImplGetValue(double& dNewVal);

    void ImplSetFormatKey(sal_uLong nFormatKey);
        // SetFormatKey without FormatChanged notification

    SvNumberFormatter*  CreateFormatter() { SetFormatter(StandardFormatter()); return m_pFormatter; }

    virtual void UpdateCurrentValue(double dCurrentValue) { m_dCurrentValue = dCurrentValue; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
