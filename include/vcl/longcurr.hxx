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

#ifndef INCLUDED_VCL_LONGCURR_HXX
#define INCLUDED_VCL_LONGCURR_HXX

#include <vcl/dllapi.h>
#include <tools/bigint.hxx>
#include <vcl/field.hxx>

class LocaleDataWrapper;


class VCL_DLLPUBLIC LongCurrencyFormatter : public FormatterBase
{
public:
                            virtual ~LongCurrencyFormatter() override;

    virtual void            Reformat() override;
    virtual void            ReformatAll() override;

    void                    SetUseThousandSep( bool b );
    bool                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetCurrencySymbol( const OUString& rStr );
    OUString const &        GetCurrencySymbol() const;

    void                    SetMin(const BigInt& rNewMin);
    const BigInt&           GetMin() const { return mnMin; }
    void                    SetMax(const BigInt& rNewMax);
    const BigInt&           GetMax() const { return mnMax; }

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

    OUString                maCurrencySymbol;
    sal_uInt16              mnDecimalDigits;
    bool                    mbThousandSep;

};


class VCL_DLLPUBLIC LongCurrencyField : public SpinField, public LongCurrencyFormatter
{
    friend void ImplNewLongCurrencyFieldValue(LongCurrencyField*, const BigInt&);

private:
    BigInt          mnSpinSize;
    BigInt          mnFirst;
    BigInt          mnLast;

public:
                    LongCurrencyField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void            Modify() override;
    void            Up() override;
    void            Down() override;
    void            First() override;
    void            Last() override;

    void            SetFirst(const BigInt& rNewFirst ) { mnFirst = rNewFirst; }
    const BigInt&   GetFirst() const { return mnFirst; }
    void            SetLast(const BigInt& rNewLast ) { mnLast = rNewLast; }
    const BigInt&   GetLast() const { return mnLast; }
    void            SetSpinSize(const BigInt& rNewSize) { mnSpinSize = rNewSize; }
    const BigInt&   GetSpinSize() const { return mnSpinSize; }
};


class VCL_DLLPUBLIC LongCurrencyBox : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void            Modify() override;
    void            ReformatAll() override;
};

#endif // INCLUDED_VCL_LONGCURR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
