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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <tools/date.hxx>
#include <tools/fldunit.hxx>
#include <tools/time.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/spinfld.hxx>

namespace com::sun::star::lang { struct Locale; }

class CalendarWrapper;
class LocaleDataWrapper;
class LanguageTag;

class VCL_DLLPUBLIC FormatterBase
{
private:
    VclPtr<Edit>            mpField;
    std::unique_ptr<LocaleDataWrapper>
                            mpLocaleDataWrapper;
    bool                    mbReformat;
    bool                    mbStrictFormat;
    bool                    mbEmptyFieldValue;
    bool                    mbEmptyFieldValueEnabled;

protected:
    SAL_DLLPRIVATE void     ImplSetText( const OUString& rText, Selection const * pNewSel = nullptr );
    SAL_DLLPRIVATE bool     ImplGetEmptyFieldValue() const  { return mbEmptyFieldValue; }

    void                    SetEmptyFieldValueData( bool bValue ) { mbEmptyFieldValue = bValue; }

    SAL_DLLPRIVATE LocaleDataWrapper& ImplGetLocaleDataWrapper() const;

    Edit*                   GetField() const            { return mpField; }
    void                    ClearField() { mpField.clear(); }

public:
    explicit                FormatterBase(Edit* pField);
    virtual                 ~FormatterBase();

    const LocaleDataWrapper& GetLocaleDataWrapper() const;

    bool                    MustBeReformatted() const   { return mbReformat; }
    void                    MarkToBeReformatted( bool b ) { mbReformat = b; }

    void                    SetStrictFormat( bool bStrict );
    bool                    IsStrictFormat() const { return mbStrictFormat; }

    virtual void            Reformat();
    virtual void            ReformatAll();

    const css::lang::Locale& GetLocale() const;
    const LanguageTag&      GetLanguageTag() const;

    void                    SetEmptyFieldValue();
    bool                    IsEmptyFieldValue() const;

    void                    EnableEmptyFieldValue( bool bEnable )   { mbEmptyFieldValueEnabled = bEnable; }
    bool                    IsEmptyFieldValueEnabled() const        { return mbEmptyFieldValueEnabled; }
};

class VCL_DLLPUBLIC NumericFormatter : public FormatterBase
{
public:
    virtual                 ~NumericFormatter() override;

    virtual void            Reformat() override;

    void                    SetMin( sal_Int64 nNewMin );
    sal_Int64               GetMin() const { return mnMin; }
    void                    SetMax( sal_Int64 nNewMax );
    sal_Int64               GetMax() const { return mnMax; }

    sal_Int64               ClipAgainstMinMax(sal_Int64 nValue) const;

    void                    SetFirst( sal_Int64 nNewFirst )   { mnFirst = nNewFirst; }
    sal_Int64               GetFirst() const                  { return mnFirst; }
    void                    SetLast( sal_Int64 nNewLast )     { mnLast = nNewLast; }
    sal_Int64               GetLast() const                   { return mnLast; }
    void                    SetSpinSize( sal_Int64 nNewSize ) { mnSpinSize = nNewSize; }
    sal_Int64               GetSpinSize() const               { return mnSpinSize; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16                  GetDecimalDigits() const { return mnDecimalDigits;}

    void                    SetUseThousandSep( bool b );
    bool                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetUserValue( sal_Int64 nNewValue );
    virtual void            SetValue( sal_Int64 nNewValue );
    sal_Int64               GetValue() const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const;

    sal_Int64               Normalize( sal_Int64 nValue ) const;
    sal_Int64               Denormalize( sal_Int64 nValue ) const;

    OUString                GetValueString() const;
    void                    SetValueFromString(const OUString& rStr);

protected:
    sal_Int64               mnLastValue;
    sal_Int64               mnMin;
    sal_Int64               mnMax;
    bool                    mbFormatting;

    // the members below are used in all derivatives of NumericFormatter
    // not in NumericFormatter itself.
    sal_Int64               mnSpinSize;
    sal_Int64               mnFirst;
    sal_Int64               mnLast;

                            NumericFormatter(Edit* pEdit);

    void                    FieldUp();
    void                    FieldDown();
    void                    FieldFirst();
    void                    FieldLast();
    void                    FormatValue(Selection const * pNewSelection = nullptr);

    SAL_DLLPRIVATE void     ImplNumericReformat();
    SAL_DLLPRIVATE void     ImplNewFieldValue( sal_Int64 nNewValue );
    SAL_DLLPRIVATE void     ImplSetUserValue( sal_Int64 nNewValue, Selection const * pNewSelection = nullptr );

    virtual sal_Int64       GetValueFromString(const OUString& rStr) const;

private:
    sal_uInt16              mnDecimalDigits;
    bool                    mbThousandSep;

};

class VCL_DLLPUBLIC MetricFormatter : public NumericFormatter
{
public:
    virtual                 ~MetricFormatter() override;

    virtual void            Reformat() override;

    virtual void            SetUnit( FieldUnit meUnit );
    FieldUnit               GetUnit() const { return meUnit; }
    void                    SetCustomUnitText( const OUString& rStr );
    const OUString&         GetCustomUnitText() const { return maCustomUnitText; }

    using NumericFormatter::SetMax;
    void                    SetMax( sal_Int64 nNewMax, FieldUnit eInUnit );
    using NumericFormatter::GetMax;
    sal_Int64               GetMax( FieldUnit eOutUnit ) const;
    using NumericFormatter::SetMin;
    void                    SetMin( sal_Int64 nNewMin, FieldUnit eInUnit );
    using NumericFormatter::GetMin;
    sal_Int64               GetMin( FieldUnit eOutUnit ) const;

    void                    SetValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual void            SetValue( sal_Int64 nValue ) override;
    using NumericFormatter::SetUserValue;
    void                    SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    using NumericFormatter::GetValue;
    sal_Int64               GetValue( FieldUnit eOutUnit ) const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const override;
    sal_Int64               GetCorrectedValue( FieldUnit eOutUnit ) const;

protected:
    FieldUnit               meUnit;

                            MetricFormatter(Edit* pEdit);

    SAL_DLLPRIVATE void     ImplMetricReformat( const OUString& rStr, double& rValue, OUString& rOutStr );

    virtual sal_Int64       GetValueFromString(const OUString& rStr) const override;
    sal_Int64               GetValueFromStringUnit(const OUString& rStr, FieldUnit eOutUnit) const;

private:
    OUString                maCustomUnitText;
};

class VCL_DLLPUBLIC MetricField : public SpinField, public MetricFormatter
{
public:
    explicit                MetricField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual Size            CalcMinimumSize() const override;

    virtual void            Modify() override;

    virtual void            Up() override;
    virtual void            Down() override;
    virtual void            First() override;
    virtual void            Last() override;

    virtual void            SetUnit( FieldUnit meUnit ) override;

    void                    SetFirst( sal_Int64 nNewFirst, FieldUnit eInUnit );
    sal_Int64               GetFirst( FieldUnit eOutUnit ) const;
    void                    SetLast( sal_Int64 nNewLast, FieldUnit eInUnit );
    sal_Int64               GetLast( FieldUnit eOutUnit ) const;

    virtual bool            set_property(const OString &rKey, const OUString &rValue) override;
    virtual void            dispose() override;

    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
    virtual FactoryFunction GetUITestFactory() const override;
};

class VCL_DLLPUBLIC MetricBox : public ComboBox, public MetricFormatter
{
public:
    explicit                MetricBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual Size            CalcMinimumSize() const override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;

    // Needed, because GetValue() with nPos hide these functions
    using MetricFormatter::GetValue;

    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) CurrencyFormatter : public NumericFormatter
{
protected:
                            CurrencyFormatter(Edit* pEdit);
    SAL_DLLPRIVATE void     ImplCurrencyReformat( const OUString& rStr, OUString& rOutStr );
    virtual sal_Int64       GetValueFromString(const OUString& rStr) const override;

public:
    virtual                 ~CurrencyFormatter() override;

    virtual void            Reformat() override;

    virtual void            SetValue( sal_Int64 nNewValue ) override;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) CurrencyField final : public SpinField, public CurrencyFormatter
{
public:
    CurrencyField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            Up() override;
    virtual void            Down() override;
    virtual void            First() override;
    virtual void            Last() override;
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) TimeFormatter : public FormatterBase
{
private:
    tools::Time             maLastTime;
    tools::Time             maMin;
    tools::Time             maMax;
    TimeFieldFormat         meFormat;
    TimeFormat              mnTimeFormat;
    bool                    mbDuration;
    bool                    mbEnforceValidValue;

protected:
    tools::Time             maFieldTime;

                            TimeFormatter(Edit* pEdit);

    SAL_DLLPRIVATE void     ImplTimeReformat( const OUString& rStr, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplNewFieldValue( const tools::Time& rTime );
    SAL_DLLPRIVATE void     ImplSetUserTime( const tools::Time& rNewTime, Selection const * pNewSelection = nullptr );
    SAL_DLLPRIVATE bool     ImplAllowMalformedInput() const;

public:
    static OUString         FormatTime(const tools::Time& rNewTime, TimeFieldFormat eFormat, TimeFormat eHourFormat, bool bDuration, const LocaleDataWrapper& rLocaleData);
    static bool             TextToTime(const OUString& rStr, tools::Time& rTime, TimeFieldFormat eFormat, bool bDuration, const LocaleDataWrapper& rLocaleDataWrapper, bool _bSkipInvalidCharacters = true);
    static int              GetTimeArea(TimeFieldFormat eFormat, const OUString& rText, int nCursor,
                                        const LocaleDataWrapper& rLocaleDataWrapper);
    static tools::Time      SpinTime(bool bUp, const tools::Time& rTime, TimeFieldFormat eFormat,
                                     bool bDuration, const OUString& rText, int nCursor,
                                     const LocaleDataWrapper& rLocaleDataWrapper);

    virtual                 ~TimeFormatter() override;

    virtual void            Reformat() override;
    virtual void            ReformatAll() override;

    void                    SetMin( const tools::Time& rNewMin );
    const tools::Time&             GetMin() const { return maMin; }
    void                    SetMax( const tools::Time& rNewMax );
    const tools::Time&             GetMax() const { return maMax; }

    void                    SetTimeFormat( TimeFormat eNewFormat );
    TimeFormat              GetTimeFormat() const { return mnTimeFormat;}

    void                    SetFormat( TimeFieldFormat eNewFormat );
    TimeFieldFormat         GetFormat() const { return meFormat; }

    void                    SetDuration( bool mbDuration );
    bool                    IsDuration() const { return mbDuration; }

    void                    SetTime( const tools::Time& rNewTime );
    void                    SetUserTime( const tools::Time& rNewTime );
    tools::Time             GetTime() const;
    void                    SetEmptyTime() { FormatterBase::SetEmptyFieldValue(); }
    bool                    IsEmptyTime() const { return FormatterBase::IsEmptyFieldValue(); }

    /** enables or disables the enforcement of valid values

        If this is set to true (which is the default), then GetTime will always return a valid
        time, no matter whether the current text can really be interpreted as time. (Note: this
        is the compatible behavior).

        If this is set to false, the GetTime will return GetInvalidTime, in case the current text
        cannot be interpreted as time.

        In addition, if this is set to false, the text in the field will <em>not</em> be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) TimeField final : public SpinField, public TimeFormatter
{
private:
    tools::Time                    maFirst;
    tools::Time                    maLast;

    SAL_DLLPRIVATE void     ImplTimeSpinArea( bool bUp );

public:
    explicit                TimeField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            Up() override;
    virtual void            Down() override;
    virtual void            First() override;
    virtual void            Last() override;

    void                    SetFirst( const tools::Time& rNewFirst )   { maFirst = rNewFirst; }
    const tools::Time&      GetFirst() const                    { return maFirst; }
    void                    SetLast( const tools::Time& rNewLast )     { maLast = rNewLast; }
    const tools::Time&      GetLast() const                     { return maLast; }

    void                    SetExtFormat( ExtTimeFieldFormat eFormat );
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DateFormatter : public FormatterBase
{
private:
    std::unique_ptr<CalendarWrapper> mxCalendarWrapper;
    Date                    maFieldDate;
    Date                    maLastDate;
    Date                    maMin;
    Date                    maMax;
    bool                    mbLongFormat;
    bool                    mbShowDateCentury;
    ExtDateFieldFormat      mnExtDateFormat;
    bool                    mbEnforceValidValue;

protected:
                            DateFormatter(Edit* pEdit);

    SAL_DLLPRIVATE const Date& ImplGetFieldDate() const    { return maFieldDate; }
    SAL_DLLPRIVATE void     ImplDateReformat( const OUString& rStr, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplSetUserDate( const Date& rNewDate,
                                             Selection const * pNewSelection = nullptr );
    SAL_DLLPRIVATE OUString ImplGetDateAsText( const Date& rDate ) const;
    SAL_DLLPRIVATE void     ImplNewFieldValue( const Date& rDate );
    CalendarWrapper&        GetCalendarWrapper() const;

    SAL_DLLPRIVATE bool     ImplAllowMalformedInput() const;

public:
    static OUString         FormatDate(const Date& rNewDate, ExtDateFieldFormat eFormat, const LocaleDataWrapper& rLocaleData, CalendarWrapper& rCalendarWrapper);
    static bool             TextToDate(const OUString& rStr, Date& rTime, ExtDateFieldFormat eFormat, const LocaleDataWrapper& rLocaleDataWrapper, const CalendarWrapper& rCalendarWrapper);
    static int              GetDateArea(ExtDateFieldFormat eFormat, const OUString& rText, int nCursor, const LocaleDataWrapper& rLocaleDataWrapper);

    virtual                 ~DateFormatter() override;

    virtual void            Reformat() override;
    virtual void            ReformatAll() override;

    void                    SetExtDateFormat( ExtDateFieldFormat eFormat );
    ExtDateFieldFormat      GetExtDateFormat( bool bResolveSystemFormat = false ) const;

    void                    SetMin( const Date& rNewMin );
    const Date&             GetMin() const { return maMin; }

    void                    SetMax( const Date& rNewMax );
    const Date&             GetMax() const { return maMax; }


    // MT: Remove these methods too, ExtDateFormat should be enough!
    //     What should happen if using DDMMYYYY, but ShowCentury=false?

    void                    SetLongFormat( bool bLong );
    bool                    IsLongFormat() const { return mbLongFormat; }
    void                    SetShowDateCentury( bool bShowCentury );
    bool                    IsShowDateCentury() const { return mbShowDateCentury; }


    void                    SetDate( const Date& rNewDate );
    Date                    GetDate() const;
    void                    SetEmptyDate();
    bool                    IsEmptyDate() const;

    void                    ResetLastDate() { maLastDate = Date( Date::EMPTY ); }

    static void             ExpandCentury( Date& rDate );
    static void             ExpandCentury( Date& rDate, sal_uInt16 nTwoDigitYearStart );

    /** enables or disables the enforcement of valid values

        If this is set to true (which is the default), then GetDate will always return a valid
        date, no matter whether the current text can really be interpreted as date. (Note: this
        is the compatible behavior).

        If this is set to false, the GetDate will return GetInvalidDate, in case the current text
        cannot be interpreted as date.

        In addition, if this is set to false, the text in the field will \em not be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};

class SAL_DLLPUBLIC_RTTI DateField : public SpinField, public DateFormatter
{
private:
    Date                    maFirst;
    Date                    maLast;

protected:
    void                    ImplDateSpinArea( bool bUp );

public:
    explicit                DateField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            Up() override;
    virtual void            Down() override;
    virtual void            First() override;
    virtual void            Last() override;

    void                    SetFirst( const Date& rNewFirst )   { maFirst = rNewFirst; }
    const Date&             GetFirst() const                    { return maFirst; }
    void                    SetLast( const Date& rNewLast )     { maLast = rNewLast; }
    const Date&             GetLast() const                     { return maLast; }
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) NumericBox : public ComboBox, public NumericFormatter
{
    SAL_DLLPRIVATE void     ImplNumericReformat( const OUString& rStr, sal_Int64& rValue, OUString& rOutStr );
public:
    explicit                NumericBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual Size            CalcMinimumSize() const override;

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;

    virtual void            dispose() override;
};

#define PATTERN_FORMAT_EMPTYLITERALS    (sal_uInt16(0x0001))

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) PatternFormatter : public FormatterBase
{
private:
    OString                m_aEditMask;
    OUString               maLiteralMask;
    bool                   mbSameMask;
    bool                   mbInPattKeyInput;

protected:
                            PatternFormatter(Edit* pEdit);

    SAL_DLLPRIVATE bool     ImplIsSameMask() const { return mbSameMask; }
    SAL_DLLPRIVATE bool&    ImplGetInPattKeyInput() { return mbInPattKeyInput; }

public:
    virtual                 ~PatternFormatter() override;

    virtual void            Reformat() override;

    void SetMask(const OString& rEditMask, const OUString& rLiteralMask );
    const OString& GetEditMask() const { return m_aEditMask; }
    const OUString&        GetLiteralMask() const  { return maLiteralMask; }

    void                    SetString( const OUString& rStr );
    OUString                GetString() const;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) PatternField final : public SpinField, public PatternFormatter
{
public:
    explicit                PatternField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            Modify() override;
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) PatternBox final : public ComboBox, public PatternFormatter
{
public:
                            PatternBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) CurrencyBox final : public ComboBox, public CurrencyFormatter
{
public:
    explicit                CurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;

    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DateBox final : public ComboBox, public DateFormatter
{
public:
    explicit                DateBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;
    virtual void            dispose() override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) TimeBox final : public ComboBox, public TimeFormatter
{
public:
    explicit                TimeBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            Modify() override;

    virtual void            ReformatAll() override;
    virtual void            dispose() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
