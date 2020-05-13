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

#ifndef INCLUDED_VCL_TOOLKIT_FIELD_HXX
#define INCLUDED_VCL_TOOLKIT_FIELD_HXX

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/longcurr.hxx>

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

    virtual boost::property_tree::ptree DumpAsPropertyTree() override;
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

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) LongCurrencyBox final : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void            Modify() override;
    void            ReformatAll() override;
};

#endif // INCLUDED_VCL_FIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
