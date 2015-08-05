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


// - LongCurrencyFormatter -


class VCL_DLLPUBLIC LongCurrencyFormatter : public FormatterBase
{
private:
    SAL_DLLPRIVATE friend bool ImplLongCurrencyReformat( const OUString&, BigInt, BigInt, sal_uInt16, const LocaleDataWrapper&, OUString&, LongCurrencyFormatter& );
    SAL_DLLPRIVATE void        ImpInit();

protected:
    BigInt                  mnFieldValue;
    BigInt                  mnLastValue;
    BigInt                  mnMin;
    BigInt                  mnMax;
    BigInt                  mnCorrectedValue;
    OUString                maCurrencySymbol;
    sal_uInt16              mnType;
    sal_uInt16              mnDecimalDigits;
    bool                mbThousandSep;

                            LongCurrencyFormatter();
public:
                            virtual ~LongCurrencyFormatter();

    virtual void            Reformat() SAL_OVERRIDE;
    virtual void            ReformatAll() SAL_OVERRIDE;

    void                    SetUseThousandSep( bool b );
    bool                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetCurrencySymbol( const OUString& rStr );
    OUString                GetCurrencySymbol() const;

    void                    SetMin(const BigInt& rNewMin);
    BigInt                  GetMin() const { return mnMin; }
    void                    SetMax(const BigInt& rNewMax);
    BigInt                  GetMax() const { return mnMax; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16                  GetDecimalDigits() const { return mnDecimalDigits;}
    void                    SetValue(const BigInt& rNewValue);
    void                    SetUserValue( BigInt nNewValue );
    BigInt                  GetValue() const;
};


// - LongCurrencyField -


class VCL_DLLPUBLIC LongCurrencyField : public SpinField, public LongCurrencyFormatter
{
    friend void ImplNewLongCurrencyFieldValue(LongCurrencyField*, const BigInt&);

private:
    BigInt          mnSpinSize;
    BigInt          mnFirst;
    BigInt          mnLast;

public:
                    LongCurrencyField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Modify() SAL_OVERRIDE;
    void            Up() SAL_OVERRIDE;
    void            Down() SAL_OVERRIDE;
    void            First() SAL_OVERRIDE;
    void            Last() SAL_OVERRIDE;

    void            SetFirst(const BigInt& rNewFirst ) { mnFirst = rNewFirst; }
    BigInt          GetFirst() const { return mnFirst; }
    void            SetLast(const BigInt& rNewLast ) { mnLast = rNewLast; }
    BigInt          GetLast() const { return mnLast; }
    void            SetSpinSize(const BigInt& rNewSize) { mnSpinSize = rNewSize; }
    BigInt          GetSpinSize() const { return mnSpinSize; }
};


// - LongCurrencyBox -


class VCL_DLLPUBLIC LongCurrencyBox : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Modify() SAL_OVERRIDE;
    void            ReformatAll() SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_LONGCURR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
