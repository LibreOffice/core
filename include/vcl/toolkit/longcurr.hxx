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
#include <vcl/dllapi.h>
#include <tools/bigint.hxx>
#include <vcl/toolkit/field.hxx>

class LocaleDataWrapper;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) LongCurrencyFormatter : public FormatterBase
{
public:
                            virtual ~LongCurrencyFormatter() override;

    virtual void            Reformat() override;
    virtual void            ReformatAll() override;

    OUString const &        GetCurrencySymbol() const;

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16              GetDecimalDigits() const { return mnDecimalDigits;}
    void                    SetValue(const BigInt& rNewValue);
    void                    SetUserValue( BigInt nNewValue );
    BigInt                  GetValue() const;

protected:
    BigInt                  mnLastValue;
    BigInt                  mnMin;
    BigInt                  mnMax;

                            LongCurrencyFormatter(Edit* pEdit);
private:
    friend bool ImplLongCurrencyReformat( const OUString&, BigInt const &, BigInt const &, sal_uInt16, const LocaleDataWrapper&, OUString&, LongCurrencyFormatter const & );
    SAL_DLLPRIVATE void        ImpInit();

    sal_uInt16              mnDecimalDigits;

};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) LongCurrencyBox final : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void            Modify() override;
    void            ReformatAll() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
