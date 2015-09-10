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

#ifndef INCLUDED_VCL_FIELD_HXX
#define INCLUDED_VCL_FIELD_HXX

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


// - FormatterBase -


class VCL_DLLPUBLIC FormatterBase
{
private:
    VclPtr<Edit>            mpField;
    LocaleDataWrapper*      mpLocaleDataWrapper;
    bool                    mbReformat;
    bool                    mbStrictFormat;
    bool                    mbEmptyFieldValue;
    bool                    mbEmptyFieldValueEnabled;
    bool                    mbDefaultLocale;

protected:
    SAL_DLLPRIVATE void     ImplSetText( const OUString& rText, Selection* pNewSel = NULL );
    SAL_DLLPRIVATE bool     ImplGetEmptyFieldValue() const  { return mbEmptyFieldValue; }

    void                    SetEmptyFieldValueData( bool bValue ) { mbEmptyFieldValue = bValue; }

    SAL_DLLPRIVATE LocaleDataWrapper& ImplGetLocaleDataWrapper() const;
    bool                    IsDefaultLocale() const { return mbDefaultLocale; }

public:
    explicit                FormatterBase( Edit* pField = NULL );
    virtual                 ~FormatterBase();

    const LocaleDataWrapper& GetLocaleDataWrapper() const;

    void                    SetField( Edit* pField )    { mpField = pField; }
    Edit*                   GetField() const            { return mpField; }

    bool                    MustBeReformatted() const   { return mbReformat; }
    void                    MarkToBeReformatted( bool b ) { mbReformat = b; }

    void                    SetStrictFormat( bool bStrict );
    bool                    IsStrictFormat() const { return mbStrictFormat; }

    virtual void            Reformat();
    virtual void            ReformatAll();

    virtual void            SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    const ::com::sun::star::lang::Locale&   GetLocale() const;
    const LanguageTag&      GetLanguageTag() const;

    const AllSettings&      GetFieldSettings() const;

    void                    SetEmptyFieldValue();
    bool                    IsEmptyFieldValue() const;

    void                    EnableEmptyFieldValue( bool bEnable )   { mbEmptyFieldValueEnabled = bEnable; }
    bool                    IsEmptyFieldValueEnabled() const        { return mbEmptyFieldValueEnabled; }
};



// - PatternFormatter -


#define PATTERN_FORMAT_EMPTYLITERALS    ((sal_uInt16)0x0001)

class VCL_DLLPUBLIC PatternFormatter : public FormatterBase
{
private:
    OString                m_aEditMask;
    OUString               maFieldString;
    OUString               maLiteralMask;
    sal_uInt16             mnFormatFlags;
    bool                   mbSameMask;
    bool               mbInPattKeyInput;

protected:
                            PatternFormatter();

    SAL_DLLPRIVATE void ImplSetMask(const OString& rEditMask,
        const OUString& rLiteralMask);
    SAL_DLLPRIVATE bool     ImplIsSameMask() const { return mbSameMask; }
    SAL_DLLPRIVATE bool&    ImplGetInPattKeyInput() { return mbInPattKeyInput; }

public:
    virtual                 ~PatternFormatter();

    virtual void            Reformat() SAL_OVERRIDE;

    void SetMask(const OString& rEditMask, const OUString& rLiteralMask );
    const OString& GetEditMask() const { return m_aEditMask; }
    const OUString&        GetLiteralMask() const  { return maLiteralMask; }

    sal_uInt16              GetFormatFlags() const { return mnFormatFlags; }

    void                    SetString( const OUString& rStr );
    OUString                GetString() const;
};


// - NumericFormatter -


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
    bool                    mbThousandSep;
    bool                    mbShowTrailingZeros;
    bool                    mbWrapOnLimits;

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
    SAL_DLLPRIVATE bool ImplNumericReformat( const OUString& rStr, sal_Int64& rValue, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplNewFieldValue( sal_Int64 nNewValue );
    SAL_DLLPRIVATE void     ImplSetUserValue( sal_Int64 nNewValue, Selection* pNewSelection = NULL );

public:
    virtual                 ~NumericFormatter();

    virtual void            Reformat() SAL_OVERRIDE;

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

    void                    SetShowTrailingZeros( bool bShowTrailingZeros );
    bool                    IsShowTrailingZeros() const { return mbShowTrailingZeros; }


    void                    SetUserValue( sal_Int64 nNewValue );
    virtual void            SetValue( sal_Int64 nNewValue );
    virtual sal_Int64       GetValue() const;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const;
    bool                    IsValueModified() const;

    sal_Int64               Normalize( sal_Int64 nValue ) const;
    sal_Int64               Denormalize( sal_Int64 nValue ) const;
};


// - MetricFormatter -


class VCL_DLLPUBLIC MetricFormatter : public NumericFormatter
{
private:
    SAL_DLLPRIVATE  void    ImplInit();

protected:
    OUString                maCustomUnitText;
    OUString                maCurUnitText;
    sal_Int64               mnBaseValue;
    FieldUnit               meUnit;
    Link<MetricFormatter&,void> maCustomConvertLink;

protected:
                            MetricFormatter();

    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE bool     ImplMetricReformat( const OUString& rStr, double& rValue, OUString& rOutStr );

public:
    virtual                 ~MetricFormatter();

    virtual void            CustomConvert() = 0;
    virtual void            Reformat() SAL_OVERRIDE;

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
    virtual void            SetValue( sal_Int64 nValue ) SAL_OVERRIDE;
    using NumericFormatter::SetUserValue;
    void                    SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual sal_Int64       GetValue( FieldUnit eOutUnit ) const;
    virtual sal_Int64       GetValue() const SAL_OVERRIDE;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const SAL_OVERRIDE;
    sal_Int64               GetCorrectedValue( FieldUnit eOutUnit ) const;

    void                    SetCustomConvertHdl( const Link<MetricFormatter&,void>& rLink ) { maCustomConvertLink = rLink; }
};



// - CurrencyFormatter -


class VCL_DLLPUBLIC CurrencyFormatter : public NumericFormatter
{
private:
    SAL_DLLPRIVATE void     ImplInit();

protected:
                            CurrencyFormatter();
    SAL_DLLPRIVATE bool     ImplCurrencyReformat( const OUString& rStr, OUString& rOutStr );

public:
    virtual                 ~CurrencyFormatter();

    virtual void            Reformat() SAL_OVERRIDE;

    OUString                GetCurrencySymbol() const;

    virtual void            SetValue( sal_Int64 nNewValue ) SAL_OVERRIDE;
    virtual sal_Int64       GetValue() const SAL_OVERRIDE;
    virtual OUString        CreateFieldText( sal_Int64 nValue ) const SAL_OVERRIDE;
};



// - DateFormatter -


class VCL_DLLPUBLIC DateFormatter : public FormatterBase
{
private:
    CalendarWrapper*        mpCalendarWrapper;
    Date                    maFieldDate;
    Date                    maLastDate;
    Date                    maMin;
    Date                    maMax;
    bool                    mbLongFormat;
    bool                    mbShowDateCentury;
    sal_uInt16                  mnDateFormat;
    sal_uLong                   mnExtDateFormat;
    bool                    mbEnforceValidValue;

    SAL_DLLPRIVATE void     ImplInit();

protected:
                            DateFormatter();

    SAL_DLLPRIVATE const Date& ImplGetFieldDate() const    { return maFieldDate; }
    SAL_DLLPRIVATE bool     ImplDateReformat( const OUString& rStr, OUString& rOutStr,
                                              const AllSettings& rSettings );
    SAL_DLLPRIVATE void     ImplSetUserDate( const Date& rNewDate,
                                             Selection* pNewSelection = NULL );
    SAL_DLLPRIVATE OUString ImplGetDateAsText( const Date& rDate,
                                               const AllSettings& rSettings ) const;
    SAL_DLLPRIVATE void     ImplNewFieldValue( const Date& rDate );
    CalendarWrapper&        GetCalendarWrapper() const;

    SAL_DLLPRIVATE bool     ImplAllowMalformedInput() const;

public:
    virtual                 ~DateFormatter();

    virtual void            Reformat() SAL_OVERRIDE;
    virtual void            ReformatAll() SAL_OVERRIDE;

    virtual void            SetLocale( const ::com::sun::star::lang::Locale& rLocale ) SAL_OVERRIDE;


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
    void                    SetUserDate( const Date& rNewDate );
    Date                    GetDate() const;
    void                    SetEmptyDate();
    bool                    IsEmptyDate() const;

    void                    ResetLastDate() { maLastDate = Date( 0, 0, 0 ); }

    static void             ExpandCentury( Date& rDate );
    static void             ExpandCentury( Date& rDate, sal_uInt16 nTwoDigitYearStart );

    static Date             GetInvalidDate() { return Date( 0, 0, 0 ); }

    /** enables or disables the enforcement of valid values

        If this is set to true (which is the default), then GetDate will always return a valid
        date, no matter whether the current text can really be interpreted as date. (Note: this
        is the compatible bahavior).

        If this is set to false, the GetDate will return GetInvalidDate, in case the current text
        cannot be interpreted as date.

        In addition, if this is set to false, the text in the field will \em not be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    inline bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};



// - TimeFormatter -


class VCL_DLLPUBLIC TimeFormatter : public FormatterBase
{
private:
    tools::Time             maLastTime;
    tools::Time             maMin;
    tools::Time             maMax;
    TimeFieldFormat         meFormat;
    sal_uInt16              mnTimeFormat;
    bool                    mbDuration;
    bool                    mbEnforceValidValue;

    SAL_DLLPRIVATE void     ImplInit();

protected:
    tools::Time             maFieldTime;

                            TimeFormatter();

    SAL_DLLPRIVATE bool     ImplTimeReformat( const OUString& rStr, OUString& rOutStr );
    SAL_DLLPRIVATE void     ImplNewFieldValue( const tools::Time& rTime );
    SAL_DLLPRIVATE void     ImplSetUserTime( const tools::Time& rNewTime, Selection* pNewSelection = NULL );
    SAL_DLLPRIVATE bool     ImplAllowMalformedInput() const;

public:

                            enum TimeFormat {
                                HOUR_12,
                                HOUR_24
                            };

    virtual                 ~TimeFormatter();

    virtual void            Reformat() SAL_OVERRIDE;
    virtual void            ReformatAll() SAL_OVERRIDE;

    void                    SetMin( const tools::Time& rNewMin );
    const tools::Time&             GetMin() const { return maMin; }
    void                    SetMax( const tools::Time& rNewMax );
    const tools::Time&             GetMax() const { return maMax; }

    void                    SetTimeFormat( TimeFormat eNewFormat );
    TimeFormat              GetTimeFormat() const { return (TimeFormat)mnTimeFormat;}

    void                    SetFormat( TimeFieldFormat eNewFormat );
    TimeFieldFormat         GetFormat() const { return meFormat; }

    void                    SetDuration( bool mbDuration );
    bool                    IsDuration() const { return mbDuration; }

    void                    SetTime( const tools::Time& rNewTime );
    void                    SetUserTime( const tools::Time& rNewTime );
    tools::Time             GetTime() const;
    void                    SetEmptyTime() { FormatterBase::SetEmptyFieldValue(); }
    bool                    IsEmptyTime() const { return FormatterBase::IsEmptyFieldValue(); }

    static tools::Time      GetInvalidTime() { return tools::Time( 99, 99, 99 ); }

    /** enables or disables the enforcement of valid values

        If this is set to true (which is the default), then GetTime will always return a valid
        time, no matter whether the current text can really be interpreted as time. (Note: this
        is the compatible bahavior).

        If this is set to false, the GetTime will return GetInvalidTime, in case the current text
        cannot be interpreted as time.

        In addition, if this is set to false, the text in the field will <em>not</em> be corrected
        when the control loses the focus - instead, the invalid input will be preserved.
    */
    void                    EnforceValidValue( bool _bEnforce ) { mbEnforceValidValue = _bEnforce; }
    inline bool             IsEnforceValidValue( ) const { return mbEnforceValidValue; }
};



// - PatternField -


class VCL_DLLPUBLIC PatternField : public SpinField, public PatternFormatter
{
public:
    explicit                PatternField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            Modify() SAL_OVERRIDE;
};



// - NumericField -


class VCL_DLLPUBLIC NumericField : public SpinField, public NumericFormatter
{
protected:
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                NumericField( vcl::Window* pParent, WinBits nWinStyle );
    explicit                NumericField( vcl::Window* pParent, const ResId& );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual Size            CalcMinimumSize() const SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            Up() SAL_OVERRIDE;
    virtual void            Down() SAL_OVERRIDE;
    virtual void            First() SAL_OVERRIDE;
    virtual void            Last() SAL_OVERRIDE;
    virtual bool            set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};



// - MetricField  -


class VCL_DLLPUBLIC MetricField : public SpinField, public MetricFormatter
{
protected:
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
    explicit                MetricField( vcl::Window* pParent, WinBits nWinStyle );
    explicit                MetricField( vcl::Window* pParent, const ResId& );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual Size            CalcMinimumSize() const SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            Up() SAL_OVERRIDE;
    virtual void            Down() SAL_OVERRIDE;
    virtual void            First() SAL_OVERRIDE;
    virtual void            Last() SAL_OVERRIDE;
    virtual void            CustomConvert() SAL_OVERRIDE;

    virtual void            SetUnit( FieldUnit meUnit ) SAL_OVERRIDE;

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

    virtual bool            set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};



// - CurrencyField -


class VCL_DLLPUBLIC CurrencyField : public SpinField, public CurrencyFormatter
{
public:
    CurrencyField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            Up() SAL_OVERRIDE;
    virtual void            Down() SAL_OVERRIDE;
    virtual void            First() SAL_OVERRIDE;
    virtual void            Last() SAL_OVERRIDE;
};



// - DateField -


class VCL_DLLPUBLIC DateField : public SpinField, public DateFormatter
{
private:
    Date                    maFirst;
    Date                    maLast;

protected:
    SAL_DLLPRIVATE void     ImplDateSpinArea( bool bUp );

public:
    explicit                DateField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            Up() SAL_OVERRIDE;
    virtual void            Down() SAL_OVERRIDE;
    virtual void            First() SAL_OVERRIDE;
    virtual void            Last() SAL_OVERRIDE;

    void                    SetFirst( const Date& rNewFirst )   { maFirst = rNewFirst; }
    Date                    GetFirst() const                    { return maFirst; }
    void                    SetLast( const Date& rNewLast )     { maLast = rNewLast; }
    Date                    GetLast() const                     { return maLast; }
};


// - TimeField -


class VCL_DLLPUBLIC TimeField : public SpinField, public TimeFormatter
{
private:
    tools::Time                    maFirst;
    tools::Time                    maLast;

protected:
    SAL_DLLPRIVATE void     ImplTimeSpinArea( bool bUp );

public:
    explicit                TimeField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            Up() SAL_OVERRIDE;
    virtual void            Down() SAL_OVERRIDE;
    virtual void            First() SAL_OVERRIDE;
    virtual void            Last() SAL_OVERRIDE;

    void                    SetFirst( const tools::Time& rNewFirst )   { maFirst = rNewFirst; }
    tools::Time             GetFirst() const                    { return maFirst; }
    void                    SetLast( const tools::Time& rNewLast )     { maLast = rNewLast; }
    tools::Time             GetLast() const                     { return maLast; }

    void                    SetExtFormat( ExtTimeFieldFormat eFormat );
};



// - PatternBox -


class VCL_DLLPUBLIC PatternBox : public ComboBox, public PatternFormatter
{
public:
                            PatternBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            ReformatAll() SAL_OVERRIDE;
};



// - NumericBox -


class VCL_DLLPUBLIC NumericBox : public ComboBox, public NumericFormatter
{
public:
    explicit                NumericBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual Size            CalcMinimumSize() const SAL_OVERRIDE;

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            ReformatAll() SAL_OVERRIDE;

    void                    InsertValue( sal_Int64 nValue, sal_Int32  nPos = COMBOBOX_APPEND );
};



// - MetricBox -


class VCL_DLLPUBLIC MetricBox : public ComboBox, public MetricFormatter
{
public:
    explicit                MetricBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual Size            CalcMinimumSize() const SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            CustomConvert() SAL_OVERRIDE;
    virtual void            ReformatAll() SAL_OVERRIDE;

    void                    InsertValue( sal_Int64 nValue, FieldUnit eInUnit = FUNIT_NONE,
                                         sal_Int32  nPos = COMBOBOX_APPEND );
    sal_Int64               GetValue( sal_Int32  nPos, FieldUnit eOutUnit = FUNIT_NONE ) const;
    sal_Int32               GetValuePos( sal_Int64 nValue,
                                         FieldUnit eInUnit = FUNIT_NONE ) const;

    // Needed, because GetValue() with nPos hide these functions
    virtual sal_Int64       GetValue( FieldUnit eOutUnit ) const SAL_OVERRIDE;
    virtual sal_Int64       GetValue() const SAL_OVERRIDE;
};



// - CurrencyBox -


class VCL_DLLPUBLIC CurrencyBox : public ComboBox, public CurrencyFormatter
{
public:
    explicit                CurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            ReformatAll() SAL_OVERRIDE;

    virtual sal_Int64       GetValue() const SAL_OVERRIDE;
};


// - DateBox -


class VCL_DLLPUBLIC DateBox : public ComboBox, public DateFormatter
{
public:
    explicit                DateBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            ReformatAll() SAL_OVERRIDE;
};



// - TimeBox -


class VCL_DLLPUBLIC TimeBox : public ComboBox, public TimeFormatter
{
public:
    explicit                TimeBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            Modify() SAL_OVERRIDE;

    virtual void            ReformatAll() SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_FIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
