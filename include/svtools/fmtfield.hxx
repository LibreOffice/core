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

#ifndef INCLUDED_SVTOOLS_FMTFIELD_HXX
#define INCLUDED_SVTOOLS_FMTFIELD_HXX

#include <svtools/svtdllapi.h>
#include <vcl/spinfld.hxx>
#include <svl/zforlist.hxx>

namespace validation { class NumberValidator; }

enum class FORMAT_CHANGE_TYPE
{
    KEYONLY           = 0x00,        // only a new key was set
    FORMATTER         = 0x01,        // a new formatter was set, usually implies a change of the key, too
    PRECISION         = 0x02,        // a new precision was set
    THOUSANDSSEP      = 0x03,        // the thousands separator setting changed
    CURRENCY_SYMBOL   = 0x10,
    CURRSYM_POSITION  = 0x20,
};


class SVT_DLLPUBLIC FormattedField : public SpinField
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
        SVT_DLLPUBLIC SvNumberFormatter* GetFormatter();
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

    bool                m_bStrictFormat : 1;

    bool                m_bEnableEmptyField : 1;
    bool                m_bAutoColor : 1;
    bool                m_bEnableNaN : 1;
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
    Color*              m_pLastOutputColor;

    bool                m_bUseInputStringForFormatting;

public:
    FormattedField(vcl::Window* pParent, WinBits nStyle = 0, SvNumberFormatter* pInitialFormatter = nullptr);

    // Min-/Max-management
    bool    HasMinValue() const         { return m_bHasMin; }
    void    ClearMinValue()             { m_bHasMin = false; }
    void    SetMinValue(double dMin);
    double  GetMinValue() const         { return m_dMinValue; }

    bool    HasMaxValue() const         { return m_bHasMax; }
    void    ClearMaxValue()             { m_bHasMax = false; }
    void    SetMaxValue(double dMax);
    double  GetMaxValue() const         { return m_dMaxValue; }

    // Current value
    void    SetValue(double dVal);
    double  GetValue();
    // The default implementation uses a formatter, if available

    void    SetTextValue(const OUString& rText);
    // The String is transformed to a double (with a formatter) and SetValue is called afterwards

    bool    IsEmptyFieldEnabled() const         { return m_bEnableEmptyField; }
    void    EnableEmptyField(bool bEnable);
    // If disabled, the value will be resetted to the last valid value on leave

    void    SetDefaultValue(double dDefault)    { m_dDefaultValue = dDefault; m_ValueState = valueDirty; }
    // If the current String is invalid, GetValue() returns this value
    double  GetDefaultValue() const             { return m_dDefaultValue; }

    // Settings for the format
    sal_uLong   GetFormatKey() const                { return m_nFormatKey; }
    void    SetFormatKey(sal_uLong nFormatKey);

    SvNumberFormatter*  GetFormatter() const    { return m_pFormatter; }
    void    SetFormatter(SvNumberFormatter* pFormatter, bool bResetFormat = true);
    // If bResetFormat is sal_False, the old format is tried to be kept. (expensive, if it is no default format, available in all formatters)
    // If sal_True, the new FormatKey is set to zero

    bool    GetThousandsSep() const;
    void    SetThousandsSep(bool _bUseSeparator);
        // the is no check if the current format is numeric, so be cautious when calling these functions

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

    // Spin-Handling
    virtual void Up() override;
    virtual void Down() override;
    // Default Implementation: +/- default spin size to the double value
    virtual void First() override;
    virtual void Last() override;
    // Default Implementation: Current double is set to the first or last value

    void    SetSpinSize(double dStep)   { m_dSpinSize = dStep; }
    double  GetSpinSize() const         { return m_dSpinSize; }

    void    SetSpinFirst(double dFirst) { m_dSpinFirst = dFirst; }
    double  GetSpinFirst() const        { return m_dSpinFirst; }

    void    SetSpinLast(double dLast)   { m_dSpinLast = dLast; }
    double  GetSpinLast() const         { return m_dSpinLast; }

    bool    TreatingAsNumber() const    { return m_bTreatAsNumber; }
    void    TreatAsNumber(bool bDoSo) { m_bTreatAsNumber = bDoSo; }

public:
    virtual void SetText( const OUString& rStr ) override;
    virtual void SetText( const OUString& rStr, const Selection& rNewSelection ) override;

    //The following methods are interesting, if m_bTreatAsNumber is set to sal_False
    //If someone does not care about all the double handling and just wants to print the text formatted.
    //(((The text will be formatted, using the Formatter, and then set)
    void SetTextFormatted(const OUString& rText);
    OUString const & GetTextValue() const;

    void      SetDefaultText(const OUString& rDefault) { m_sDefaultText = rDefault; }
    const OUString& GetDefaultText() const { return m_sDefaultText; }

    // The last colour from the Formatter's last output operation. Output operations get triggered by:
    // SetValue, SetTextValue, SetTextFormatted, also indirectly via SetMin - / -MaxValue
    Color*  GetLastOutputColor() const { return m_pLastOutputColor; }

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
        intepreted, according to the current formatting) will be handled as if the default value
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

protected:
    virtual bool Notify(NotifyEvent& rNEvt) override;
    void impl_Modify(bool makeValueDirty = true);
    virtual void Modify() override;

    // Override CheckTextfor input-time checks
    virtual bool CheckText(const OUString&) const { return true; }

    // any aspect of the current format has changed
    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat);

    void ImplSetTextImpl(const OUString& rNew, Selection* pNewSel);
    void ImplSetValue(double dValue, bool bForce);
    bool ImplGetValue(double& dNewVal);

    void ImplSetFormatKey(sal_uLong nFormatKey);
        // SetFormatKey without FormatChanged notification

    SvNumberFormatter*  CreateFormatter() { SetFormatter(StandardFormatter()); return m_pFormatter; }
    SvNumberFormatter*  ImplGetFormatter() const { return m_pFormatter ? m_pFormatter : const_cast<FormattedField*>(this)->CreateFormatter(); }

    bool PreNotify(NotifyEvent& rNEvt) override;

    void ReFormat();
};


class SVT_DLLPUBLIC DoubleNumericField : public FormattedField
{
protected:
    validation::NumberValidator*    m_pNumberValidator;

public:
    DoubleNumericField(vcl::Window* pParent, WinBits nStyle = 0)
        :FormattedField(pParent, nStyle)
        ,m_pNumberValidator( nullptr )
    {
        ResetConformanceTester();
    }

    virtual ~DoubleNumericField();
    virtual void dispose() override;

protected:
    virtual bool CheckText(const OUString& sText) const override;

    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat) override;
    void ResetConformanceTester();
};


class DoubleCurrencyField : public FormattedField
{
    OUString   m_sCurrencySymbol;
    bool       m_bPrependCurrSym;
    bool       m_bChangingFormat;

public:
    DoubleCurrencyField(vcl::Window* pParent, WinBits nStyle = 0);

    const OUString& getCurrencySymbol() const { return m_sCurrencySymbol; }
    void        setCurrencySymbol(const OUString& rSymbol);

    bool        getPrependCurrSym() const { return m_bPrependCurrSym; }
    void        setPrependCurrSym(bool _bPrepend);

protected:
    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat) override;

    void UpdateCurrencyFormat();
};

#endif // INCLUDED_SVTOOLS_FMTFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
