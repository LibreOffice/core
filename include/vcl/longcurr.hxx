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

    void                    SetMin(const sal_Int64& rNewMin);
    const sal_Int64&        GetMin() const { return mnMin; }
    void                    SetMax(const sal_Int64& rNewMax);
    const sal_Int64&        GetMax() const { return mnMax; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16              GetDecimalDigits() const { return mnDecimalDigits;}
    void                    SetValue(const sal_Int64& rNewValue);
    void                    SetUserValue( sal_Int64 nNewValue );
    sal_Int64               GetValue() const;

protected:
    sal_Int64               mnLastValue;
    sal_Int64               mnMin;
    sal_Int64               mnMax;

                            LongCurrencyFormatter();
private:
    friend bool ImplLongCurrencyReformat( const OUString&, sal_Int64 const &, sal_Int64 const &, sal_uInt16, const LocaleDataWrapper&, OUString&, LongCurrencyFormatter& );
    SAL_DLLPRIVATE void     ImpInit();

    sal_Int64               mnFieldValue;
    sal_Int64               mnCorrectedValue;
    OUString                maCurrencySymbol;
    sal_uInt16              mnDecimalDigits;
    bool                    mbThousandSep;

};


class VCL_DLLPUBLIC LongCurrencyField : public SpinField, public LongCurrencyFormatter
{
    friend void ImplNewLongCurrencyFieldValue(LongCurrencyField*, const sal_Int64&);

private:
    sal_Int64               mnSpinSize;
    sal_Int64               mnFirst;
    sal_Int64               mnLast;

public:
                            LongCurrencyField( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;

    void                    Modify() override;
    void                    Up() override;
    void                    Down() override;
    void                    First() override;
    void                    Last() override;

    void                    SetFirst(const sal_Int64& rNewFirst ) { mnFirst = rNewFirst; }
    const sal_Int64&        GetFirst() const { return mnFirst; }
    void                    SetLast(const sal_Int64& rNewLast ) { mnLast = rNewLast; }
    const sal_Int64&        GetLast() const { return mnLast; }
    void                    SetSpinSize(const sal_Int64& rNewSize) { mnSpinSize = rNewSize; }
    const sal_Int64&        GetSpinSize() const { return mnSpinSize; }
};


class VCL_DLLPUBLIC LongCurrencyBox : public ComboBox, public LongCurrencyFormatter
{
public:
                            LongCurrencyBox( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;

    void                    Modify() override;
    void                    ReformatAll() override;
};

#endif // INCLUDED_VCL_LONGCURR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
