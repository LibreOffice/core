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

#ifndef _SV_FIELD_HXX
#define _SV_FIELD_HXX

#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/combobox.hxx>
#include <tools/fldunit.hxx>

namespace com { namespace sun { namespace star { namespace lang { struct Locale; } } } }

class CalendarWrapper;
class LocaleDataWrapper;
class LanguageTag;

// -----------------
// - FormatterBase -
// -----------------

class VCL_DLLPUBLIC FormatterBase
{
private:
    Edit*                   mpField;
    LocaleDataWrapper*      mpLocaleDataWrapper;
    Link                    maErrorLink;
    sal_Bool                    mbReformat;
    sal_Bool                    mbStrictFormat;
    sal_Bool                    mbEmptyFieldValue;
    sal_Bool                    mbEmptyFieldValueEnabled;
    sal_Bool                    mbDefaultLocale;

protected:
    SAL_DLLPRIVATE void     ImplSetText( const OUString& rText, Selection* pNewSel = NULL );
    SAL_DLLPRIVATE sal_Bool     ImplGetEmptyFieldValue() const  { return mbEmptyFieldValue; }

    void                    SetEmptyFieldValueData( sal_Bool bValue ) { mbEmptyFieldValue = bValue; }

    SAL_DLLPRIVATE LocaleDataWrapper& ImplGetLocaleDataWrapper() const;
    sal_Bool                    IsDefaultLocale() const { return mbDefaultLocale; }

public:
    explicit                FormatterBase( Edit* pField = NULL );
    virtual                 ~FormatterBase();

    const LocaleDataWrapper& GetLocaleDataWrapper() const;

    void                    SetField( Edit* pField )    { mpField = pField; }
    Edit*                   GetField() const            { return mpField; }

    sal_Bool                    MustBeReformatted() const   { return mbReformat; }
    void                    MarkToBeReformatted( sal_Bool b ) { mbReformat = b; }

    void                    SetStrictFormat( sal_Bool bStrict );
    sal_Bool                    IsStrictFormat() const { return mbStrictFormat; }

    virtual void            Reformat();
    virtual void            ReformatAll();

    virtual void            SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    const ::com::sun::star::lang::Locale&   GetLocale() const;
    const LanguageTag&      GetLanguageTag() const;

    const AllSettings&      GetFieldSettings() const;

    void                    SetErrorHdl( const Link& rLink )    { maErrorLink = rLink; }
    const Link&             GetErrorHdl() const                 { return maErrorLink; }

    void                    SetEmptyFieldValue();
    sal_Bool                    IsEmptyFieldValue() const;

    void                    EnableEmptyFieldValue( sal_Bool bEnable )   { mbEmptyFieldValueEnabled = bEnable; }
    sal_Bool                    IsEmptyFieldValueEnabled() const        { return mbEmptyFieldValueEnabled; }
};


// --------------------
// - PatternFormatter -
// --------------------

#define PATTERN_FORMAT_EMPTYLITERALS    ((sal_uInt16)0x0001)

class VCL_DLLPUBLIC PatternFormatter : public FormatterBase
{
private:
    OString                m_aEditMask;
    OUString               maFieldString;
    OUString               maLiteralMask;
    sal_uInt16             mnFormatFlags;
    bool                   mbSameMask;
    sal_Bool               mbInPattKeyInput;

protected:
                            PatternFormatter();

    SAL_DLLPRIVATE void ImplSetMask(const OString& rEditMask,
        const OUString& rLiteralMask);
    SAL_DLLPRIVATE bool     ImplIsSameMask() const { return mbSameMask; }
    SAL_DLLPRIVATE sal_Bool&    ImplGetInPattKeyInput() { return mbInPattKeyInput; }

public:
    virtual                 ~PatternFormatter();

    virtual void            Reformat();

    void SetMask(const OString& rEditMask, const OUString& rLiteralMask );
    const OString& GetEditMask() const { return m_aEditMask; }
    const OUString&        GetLiteralMask() const  { return maLiteralMask; }

    void                    SetFormatFlags( sal_uInt16 nFlags ) { mnFormatFlags = nFlags; }
    sal_uInt16                  GetFormatFlags() const { return mnFormatFlags; }

    void                    SetString( const OUString& rStr );
    OUString                GetString() const;
    sal_Bool                IsStringModified() const { return !(GetString() == maFieldString ); }
};

// --------------------
// - NumericFormatter -
// --------------------

class VCL_DLLPUBLIC NumericFormatter : public FormatterBase
{
private:
    SAL_DLLPRIVATE void     ImplInit();

protected:
    sal_Int64               mnFieldValue;
    sal_Int64               mnLastValue;
    sal_Int64               mnMin;
    sal_Int64               mnMax;
    sal_Int64               mnCorrectedValue;
    sal_uInt16                  mnType;
    sal_uInt16                  mnDecimalDigits;
    sal_Bool                    mbThousandSep;
    sal_Bool                    mbShowTrailingZeros;

    // the members below are used in all derivatives of NumericFormatter
    // not in NumericFormatter itself.
    sal_Int64               mnSpinSize;
    sal_Int64               mnFirst;
    sal_Int64               mnLast;

protected:
                            NumericFormatter();

    void                    FieldUp();
    void                    FieldDown();
    void                    FieldFirst();
    void                    FieldLast();

    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE sal_Bool ImplNumericReformat( const OUString& rStr, sal_Int64& rValue, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplNewFieldValue( sal_Int64 nNewValue );
    SAL_DLLPRIVATE void     ImplSetUserValue( sal_Int64 nNewValue, Selection* pNewSelection = NULL );

public:
    virtual                 ~NumericFormatter();

    virtual void            Reformat();

    void                    SetMin( sal_Int64 nNewMin );
    sal_Int64               GetMin() const { return mnMin; }
    void                    SetMax( sal_Int64 nNewMax );
    sal_Int64               GetMax() const { return mnMax; }

    void                    SetFirst( sal_Int64 nNewFirst )   { mnFirst = nNewFirst; }
    sal_Int64               GetFirst() const                  { return mnFirst; }
    void                    SetLast( sal_Int64 nNewLast )     { mnLast = nNewLast; }
    sal_Int64               GetLast() const                   { return mnLast; }
    void                    SetSpinSize( sal_Int64 nNewSize ) { mnSpinSize = nNewSize; }
    sal_Int64               GetSpinSize() const               { return mnSpinSize; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16                  GetDecimalDigits() const;

    void                    SetUseThousandSep( sal_Bool b );
    sal_Bool                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetShowTrailingZeros( sal_Bool bShowTrailingZeros );
    sal_Bool                    IsShowTrailingZeros() const { return mbShowTrailingZeros; }


    void                    SetUserValue( sal_Int64 nNewValue );
    virtual void            SetValue( sal_Int64 nNewValue );
    virtual sal_Int64       GetValue() const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const;
    sal_Bool                    IsValueModified() const;
    sal_Int64               GetCorrectedValue() const { return mnCorrectedValue; }

    sal_Int64               Normalize( sal_Int64 nValue ) const;
    sal_Int64               Denormalize( sal_Int64 nValue ) const;
};

// -------------------
// - MetricFormatter -
// -------------------

class VCL_DLLPUBLIC MetricFormatter : public NumericFormatter
{
private:
    SAL_DLLPRIVATE  void    ImplInit();

protected:
    OUString                maCustomUnitText;
    OUString                maCurUnitText;
    sal_Int64               mnBaseValue;
    FieldUnit               meUnit;
    Link                    maCustomConvertLink;

protected:
                            MetricFormatter();

    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE sal_Bool     ImplMetricReformat( const OUString& rStr, double& rValue, OUString& rOutStr );

public:
    virtual                 ~MetricFormatter();

    virtual void            CustomConvert() = 0;
    virtual void            Reformat();

    virtual void            SetUnit( FieldUnit meUnit );
    FieldUnit               GetUnit() const { return meUnit; }
    void                    SetCustomUnitText( const OUString& rStr );
    const OUString&         GetCustomUnitText() const { return maCustomUnitText; }
    const OUString&         GetCurUnitText() const { return maCurUnitText; }

    using NumericFormatter::SetMax;
    void                    SetMax( sal_Int64 nNewMax, FieldUnit eInUnit );
    using NumericFormatter::GetMax;
    sal_Int64               GetMax( FieldUnit eOutUnit ) const;
    using NumericFormatter::SetMin;
    void                    SetMin( sal_Int64 nNewMin, FieldUnit eInUnit );
    using NumericFormatter::GetMin;
    sal_Int64               GetMin( FieldUnit eOutUnit ) const;
    void                    SetBaseValue( sal_Int64 nNewBase, FieldUnit eInUnit = FUNIT_NONE );
    sal_Int64               GetBaseValue( FieldUnit eOutUnit = FUNIT_NONE ) const;

    virtual void            SetValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual void            SetValue( sal_Int64 nValue );
    using NumericFormatter::SetUserValue;
    void                    SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual sal_Int64       GetValue( FieldUnit eOutUnit ) const;
    virtual sal_Int64       GetValue() const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const;
    using NumericFormatter::GetCorrectedValue;
    sal_Int64               GetCorrectedValue( FieldUnit eOutUnit ) const;

    void                    SetCustomConvertHdl( const Link& rLink ) { maCustomConvertLink = rLink; }
    const Link&             GetCustomConvertHdl() const { return maCustomConvertLink; }
};


// ---------------------
// - CurrencyFormatter -
// ---------------------

class VCL_DLLPUBLIC CurrencyFormatter : public NumericFormatter
{
private:
    SAL_DLLPRIVATE void     ImplInit();

protected:
                            CurrencyFormatter();
    SAL_DLLPRIVATE sal_Bool     ImplCurrencyReformat( const OUString& rStr, OUString& rOutStr );

public:
    virtual                 ~CurrencyFormatter();

    virtual void            Reformat();

    OUString                GetCurrencySymbol() const;

    virtual void            SetValue( sal_Int64 nNewValue );
    virtual sal_Int64       GetValue() const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const;
};


// -----------------
// - DateFormatter -
// -----------------

class VCL_DLLPUBLIC DateFormatter : public FormatterBase
{
private:
    CalendarWrapper*        mpCalendarWrapper;
    Date                    maFieldDate;
    Date                    maLastDate;
    Date                    maMin;
    Date                    maMax;
    Date                    maCorrectedDate;
    sal_Bool                    mbLongFormat;
    sal_Bool                    mbShowDateCentury;
    sal_uInt16                  mnDateFormat;
    sal_uLong                   mnExtDateFormat;
    sal_Bool                    mbEnforceValidValue;

    SAL_DLLPRIVATE void     ImplInit();

protected:
                            DateFormatter();

    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE const Date& ImplGetFieldDate() const    { return maFieldDate; }
    SAL_DLLPRIVATE sal_Bool     ImplDateReformat( const OUString& rStr, OUString& rOutStr,
                                              const AllSettings& rSettings );
    SAL_DLLPRIVATE void     ImplSetUserDate( const Date& rNewDate,
                                             Selection* pNewSelection = NULL );
    SAL_DLLPRIVATE OUString ImplGetDateAsText( const Date& rDate,
                                               const AllSettings& rSettings ) const;
    SAL_DLLPRIVATE void     ImplNewFieldValue( const Date& rDate );
    CalendarWrapper&        GetCalendarWrapper() const;

    SAL_DLLPRIVATE sal_Bool     ImplAllowMalformedInput() const;

public:
    virtual                 ~DateFormatter();

    virtual void            Reformat();
    virtual void            ReformatAll();

    virtual void            SetLocale( const ::com::sun::star::lang::Locale& rLocale );


    void                    SetExtDateFormat( ExtDateFieldFormat eFormat );
    ExtDateFieldFormat      GetExtDateFormat( sal_Bool bResolveSystemFormat = sal_False ) const;

    void                    SetMin( const Date& rNewMin );
    const Date&             GetMin() const { return maMin; }

    void                    SetMax( const Date& rNewMax );
    const Date&             GetMax() const { return maMax; }


    // --------------------------------------------------------------
    // MT: Remove these methods too, ExtDateFormat should be enough!
    //     What should happen if using DDMMYYYY, but ShowCentury=sal_False?
    // --------------------------------------------------------------
    void                    SetLongFormat( sal_Bool bLong );
    sal_Bool                    IsLongFormat() const { return mbLongFormat; }
    void                    SetShowDateCentury( sal_Bool bShowCentury );
    sal_Bool                    IsShowDateCentury() const { return mbShowDateCentury; }
    // --------------------------------------------------------------

    void                    SetDate( const Date& rNewDate );
    void                    SetUserDate( const Date& rNewDate );
    Date                    GetDate() const;
    void                    SetEmptyDate();
    sal_Bool                    IsEmptyDate() const;
    Date                    GetCorrectedDate() const { return maCorrectedDate; }

    void                    ResetLastDate() { maLastDate = Date( 0, 0, 0 ); }

    static void             ExpandCentury( Date& rDate );
    static void             ExpandCentury( Date& rDate, sal_uInt16 nTwoDigitYearStart );

    static Date             GetInvalidDate() { return Date( 0, 0, 0 ); }

    /** enables or disables the enforcement of valid values

        If this is set to <TRUE/> (which is the default), then GetDate will always return a valid
        date, no matter whether the current text can really be interpreted as date. (Note: this
        is the compatible bahavior).

        If this is set to <FALSE/>, the GetDate will return GetInvalidDate, in case the current text
        cannot be interpreted as date.

        In addition, if this is set to <FALSE/>, the text in the field will <em>not</em> be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( sal_Bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    inline sal_Bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};


// -----------------
// - TimeFormatter -
// -----------------

class VCL_DLLPUBLIC TimeFormatter : public FormatterBase
{
private:
    Time                    maLastTime;
    Time                    maMin;
    Time                    maMax;
    Time                    maCorrectedTime;
    TimeFieldFormat         meFormat;
    sal_uInt16                  mnTimeFormat;
    sal_Bool                    mbDuration;
    sal_Bool                    mbEnforceValidValue;

    SAL_DLLPRIVATE void     ImplInit();

protected:
    Time                    maFieldTime;

                            TimeFormatter();

    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE sal_Bool     ImplTimeReformat( const OUString& rStr, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplNewFieldValue( const Time& rTime );
    SAL_DLLPRIVATE void     ImplSetUserTime( const Time& rNewTime, Selection* pNewSelection = NULL );
    SAL_DLLPRIVATE sal_Bool     ImplAllowMalformedInput() const;

public:

                            enum TimeFormat {
                                HOUR_12,
                                HOUR_24
                            };

    virtual                 ~TimeFormatter();

    virtual void            Reformat();
    virtual void            ReformatAll();

    void                    SetMin( const Time& rNewMin );
    const Time&             GetMin() const { return maMin; }
    void                    SetMax( const Time& rNewMax );
    const Time&             GetMax() const { return maMax; }

    void                    SetTimeFormat( TimeFormat eNewFormat );
    TimeFormat              GetTimeFormat() const;

    void                    SetFormat( TimeFieldFormat eNewFormat );
    TimeFieldFormat         GetFormat() const { return meFormat; }

    void                    SetDuration( sal_Bool mbDuration );
    sal_Bool                    IsDuration() const { return mbDuration; }

    void                    SetTime( const Time& rNewTime );
    void                    SetUserTime( const Time& rNewTime );
    Time                    GetTime() const;
    void                    SetEmptyTime() { FormatterBase::SetEmptyFieldValue(); }
    sal_Bool                    IsEmptyTime() const { return FormatterBase::IsEmptyFieldValue(); }
    Time                    GetCorrectedTime() const { return maCorrectedTime; }

    static Time             GetInvalidTime() { return Time( 99, 99, 99 ); }

    /** enables or disables the enforcement of valid values

        If this is set to <TRUE/> (which is the default), then GetTime will always return a valid
        time, no matter whether the current text can really be interpreted as time. (Note: this
        is the compatible bahavior).

        If this is set to <FALSE/>, the GetTime will return GetInvalidTime, in case the current text
        cannot be interpreted as time.

        In addition, if this is set to <FALSE/>, the text in the field will <em>not</em> be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( sal_Bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    inline sal_Bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};


// ----------------
// - PatternField -
// ----------------

class VCL_DLLPUBLIC PatternField : public SpinField, public PatternFormatter
{
public:
    explicit                PatternField( Window* pParent, WinBits nWinStyle );
    virtual                 ~PatternField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            Modify();
};


// ----------------
// - NumericField -
// ----------------

class VCL_DLLPUBLIC NumericField : public SpinField, public NumericFormatter
{
protected:
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                NumericField( Window* pParent, WinBits nWinStyle );
    explicit                NumericField( Window* pParent, const ResId& );
    virtual                 ~NumericField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size            CalcMinimumSize() const;

    virtual void            Modify();

    virtual void            Up();
    virtual void            Down();
    virtual void            First();
    virtual void            Last();
    virtual bool            set_property(const OString &rKey, const OString &rValue);
};


// ----------------
// - MetricField  -
// ----------------

class VCL_DLLPUBLIC MetricField : public SpinField, public MetricFormatter
{
protected:
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                MetricField( Window* pParent, WinBits nWinStyle );
    explicit                MetricField( Window* pParent, const ResId& );
    virtual                 ~MetricField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size            CalcMinimumSize() const;

    virtual void            Modify();

    virtual void            Up();
    virtual void            Down();
    virtual void            First();
    virtual void            Last();
    virtual void            CustomConvert();

    virtual void            SetUnit( FieldUnit meUnit );

    void                    SetFirst( sal_Int64 nNewFirst, FieldUnit eInUnit );
    inline void             SetFirst(sal_Int64 first) { SetFirst(first, FUNIT_NONE); }
    sal_Int64               GetFirst( FieldUnit eOutUnit ) const;
    inline sal_Int64        GetFirst() const { return GetFirst(FUNIT_NONE); }
    void                    SetLast( sal_Int64 nNewLast, FieldUnit eInUnit );
    inline void             SetLast(sal_Int64 last) { SetLast(last, FUNIT_NONE); }
    sal_Int64               GetLast( FieldUnit eOutUnit ) const;
    inline sal_Int64        GetLast() const { return GetLast(FUNIT_NONE); }

    static void             SetDefaultUnit( FieldUnit eDefaultUnit );
    static FieldUnit        GetDefaultUnit();
    static sal_Int64        ConvertValue( sal_Int64 nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                          FieldUnit eInUnit, FieldUnit eOutUnit );
    static sal_Int64        ConvertValue( sal_Int64 nValue, sal_uInt16 nDecDigits,
                                          MapUnit eInUnit, FieldUnit eOutUnit );

    // for backwards compatibility
    // caution: conversion to double loses precision
    static double           ConvertDoubleValue( double nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                                FieldUnit eInUnit, FieldUnit eOutUnit );
    static double           ConvertDoubleValue( double nValue, sal_uInt16 nDecDigits,
                                                FieldUnit eInUnit, MapUnit eOutUnit );
    static double           ConvertDoubleValue( double nValue, sal_uInt16 nDecDigits,
                                                MapUnit eInUnit, FieldUnit eOutUnit );

    // for backwards compatibility
    // caution: conversion to double loses precision
    static double           ConvertDoubleValue( sal_Int64 nValue, sal_Int64 nBaseValue, sal_uInt16 nDecDigits,
                                                FieldUnit eInUnit, FieldUnit eOutUnit )
    { return ConvertDoubleValue( static_cast<double>(nValue), nBaseValue, nDecDigits, eInUnit, eOutUnit ); }
    static double           ConvertDoubleValue( sal_Int64 nValue, sal_uInt16 nDecDigits,
                                                FieldUnit eInUnit, MapUnit eOutUnit )
    { return ConvertDoubleValue( static_cast<double>(nValue), nDecDigits, eInUnit, eOutUnit ); }
    static double           ConvertDoubleValue( sal_Int64 nValue, sal_uInt16 nDecDigits,
                                                MapUnit eInUnit, FieldUnit eOutUnit )
    { return ConvertDoubleValue( static_cast<double>(nValue), nDecDigits, eInUnit, eOutUnit ); }

    virtual bool            set_property(const OString &rKey, const OString &rValue);
};


// -----------------
// - CurrencyField -
// -----------------

class VCL_DLLPUBLIC CurrencyField : public SpinField, public CurrencyFormatter
{
public:
                            CurrencyField( Window* pParent, WinBits nWinStyle );

                            ~CurrencyField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            Up();
    virtual void            Down();
    virtual void            First();
    virtual void            Last();
};


// -------------
// - DateField -
// -------------

class VCL_DLLPUBLIC DateField : public SpinField, public DateFormatter
{
private:
    Date                    maFirst;
    Date                    maLast;

protected:
    SAL_DLLPRIVATE void     ImplDateSpinArea( sal_Bool bUp );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                DateField( Window* pParent, WinBits nWinStyle );
    explicit                DateField( Window* pParent, const ResId& );
    virtual                 ~DateField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            Up();
    virtual void            Down();
    virtual void            First();
    virtual void            Last();

    void                    SetFirst( const Date& rNewFirst )   { maFirst = rNewFirst; }
    Date                    GetFirst() const                    { return maFirst; }
    void                    SetLast( const Date& rNewLast )     { maLast = rNewLast; }
    Date                    GetLast() const                     { return maLast; }
};

// -------------
// - TimeField -
// -------------

class VCL_DLLPUBLIC TimeField : public SpinField, public TimeFormatter
{
private:
    Time                    maFirst;
    Time                    maLast;

protected:
    SAL_DLLPRIVATE void     ImplTimeSpinArea( sal_Bool bUp );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                TimeField( Window* pParent, WinBits nWinStyle );
    explicit                TimeField( Window* pParent, const ResId& );
    virtual                 ~TimeField();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            Up();
    virtual void            Down();
    virtual void            First();
    virtual void            Last();

    void                    SetFirst( const Time& rNewFirst )   { maFirst = rNewFirst; }
    Time                    GetFirst() const                    { return maFirst; }
    void                    SetLast( const Time& rNewLast )     { maLast = rNewLast; }
    Time                    GetLast() const                     { return maLast; }

    void                    SetExtFormat( ExtTimeFieldFormat eFormat );
};


// --------------
// - PatternBox -
// --------------

class VCL_DLLPUBLIC PatternBox : public ComboBox, public PatternFormatter
{
public:
                            PatternBox( Window* pParent, WinBits nWinStyle );
                            ~PatternBox();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );

    virtual void            Modify();

    virtual void            ReformatAll();
};


// --------------
// - NumericBox -
// --------------

class VCL_DLLPUBLIC NumericBox : public ComboBox, public NumericFormatter
{
public:
    explicit                NumericBox( Window* pParent, WinBits nWinStyle );
    explicit                NumericBox( Window* pParent, const ResId& );
    virtual                 ~NumericBox();

    virtual Size            CalcMinimumSize() const;

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            ReformatAll();

    void                    InsertValue( sal_Int64 nValue, sal_uInt16 nPos = COMBOBOX_APPEND );
};


// -------------
// - MetricBox -
// -------------

class VCL_DLLPUBLIC MetricBox : public ComboBox, public MetricFormatter
{
public:
    explicit                MetricBox( Window* pParent, WinBits nWinStyle );
    virtual                 ~MetricBox();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size            CalcMinimumSize() const;

    virtual void            Modify();

    virtual void            CustomConvert();
    virtual void            ReformatAll();

    void                    InsertValue( sal_Int64 nValue, FieldUnit eInUnit = FUNIT_NONE,
                                         sal_uInt16 nPos = COMBOBOX_APPEND );
    sal_Int64               GetValue( sal_uInt16 nPos, FieldUnit eOutUnit = FUNIT_NONE ) const;
    sal_uInt16                  GetValuePos( sal_Int64 nValue,
                                         FieldUnit eInUnit = FUNIT_NONE ) const;

    // Needed, because GetValue() with nPos hide these functions
    virtual sal_Int64       GetValue( FieldUnit eOutUnit ) const;
    virtual sal_Int64       GetValue() const;
};


// ---------------
// - CurrencyBox -
// ---------------

class VCL_DLLPUBLIC CurrencyBox : public ComboBox, public CurrencyFormatter
{
public:
    explicit                CurrencyBox( Window* pParent, WinBits nWinStyle );
    explicit                CurrencyBox( Window* pParent, const ResId& );
    virtual                 ~CurrencyBox();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            ReformatAll();

    virtual sal_Int64       GetValue() const;
};

// -----------
// - DateBox -
// -----------

class VCL_DLLPUBLIC DateBox : public ComboBox, public DateFormatter
{
public:
    explicit                DateBox( Window* pParent, WinBits nWinStyle );
    virtual                 ~DateBox();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            ReformatAll();
};


// -----------
// - TimeBox -
// -----------

class VCL_DLLPUBLIC TimeBox : public ComboBox, public TimeFormatter
{
public:
    explicit                TimeBox( Window* pParent, WinBits nWinStyle );
    virtual                 ~TimeBox();

    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual long            Notify( NotifyEvent& rNEvt );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            Modify();

    virtual void            ReformatAll();
};

#endif // _SV_FIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
