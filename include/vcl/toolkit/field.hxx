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
#include <vcl/field.hxx>
#include <config_options.h>

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

#endif // INCLUDED_VCL_FIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
