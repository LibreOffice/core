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

    void                    SetMin( BigInt nNewMin );
    BigInt                  GetMin() const { return mnMin; }
    void                    SetMax( BigInt nNewMax );
    BigInt                  GetMax() const { return mnMax; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16                  GetDecimalDigits() const;
    void                    SetValue( BigInt nNewValue );
    void                    SetUserValue( BigInt nNewValue );
    BigInt                  GetValue() const;
    bool                IsEmptyValue() const { return GetField()->GetText().isEmpty(); }

    BigInt                  GetCorrectedValue() const { return mnCorrectedValue; }
};


// - LongCurrencyField -


class VCL_DLLPUBLIC LongCurrencyField : public SpinField, public LongCurrencyFormatter
{
    friend void ImplNewLongCurrencyFieldValue( LongCurrencyField*, BigInt );

private:
    BigInt          mnSpinSize;
    BigInt          mnFirst;
    BigInt          mnLast;

public:
                    LongCurrencyField( Window* pParent, WinBits nWinStyle );
                    virtual ~LongCurrencyField();

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Modify() SAL_OVERRIDE;
    void            Up() SAL_OVERRIDE;
    void            Down() SAL_OVERRIDE;
    void            First() SAL_OVERRIDE;
    void            Last() SAL_OVERRIDE;

    void            SetFirst( BigInt nNewFirst ) { mnFirst = nNewFirst; }
    BigInt          GetFirst() const { return mnFirst; }
    void            SetLast( BigInt nNewLast ) { mnLast = nNewLast; }
    BigInt          GetLast() const { return mnLast; }
    void            SetSpinSize( BigInt nNewSize ) { mnSpinSize = nNewSize; }
    BigInt          GetSpinSize() const { return mnSpinSize; }
};


// - LongCurrencyBox -


class VCL_DLLPUBLIC LongCurrencyBox : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( Window* pParent, WinBits nWinStyle );
                    virtual ~LongCurrencyBox();

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Modify() SAL_OVERRIDE;
    void            ReformatAll() SAL_OVERRIDE;

    BigInt          GetValue() const
                        { return LongCurrencyFormatter::GetValue(); }
};

#endif // INCLUDED_VCL_LONGCURR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
