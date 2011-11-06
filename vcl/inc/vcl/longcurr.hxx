/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _LONGCURR_HXX
#define _LONGCURR_HXX

#include <vcl/dllapi.h>
#include <tools/bigint.hxx>
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif

class LocaleDataWrapper;

// -------------------------
// - LongCurrencyFormatter -
// -------------------------

class VCL_DLLPUBLIC LongCurrencyFormatter : public FormatterBase
{
private:
    SAL_DLLPRIVATE friend sal_Bool ImplLongCurrencyReformat( const XubString&, BigInt, BigInt, sal_uInt16, const LocaleDataWrapper&, XubString&, LongCurrencyFormatter& );
    SAL_DLLPRIVATE void        ImpInit();

protected:
    BigInt                  mnFieldValue;
    BigInt                  mnLastValue;
    BigInt                  mnMin;
    BigInt                  mnMax;
    BigInt                  mnCorrectedValue;
    String                  maCurrencySymbol;
    sal_uInt16                  mnType;
    sal_uInt16                  mnDecimalDigits;
    sal_Bool                    mbThousandSep;

                            LongCurrencyFormatter();
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
                            ~LongCurrencyFormatter();

    virtual void            Reformat();
    virtual void            ReformatAll();

    void                    SetUseThousandSep( sal_Bool b );
    sal_Bool                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetCurrencySymbol( const String& rStr );
    String                  GetCurrencySymbol() const;

    void                    SetMin( BigInt nNewMin );
    BigInt                  GetMin() const { return mnMin; }
    void                    SetMax( BigInt nNewMax );
    BigInt                  GetMax() const { return mnMax; }

    void                    SetDecimalDigits( sal_uInt16 nDigits );
    sal_uInt16                  GetDecimalDigits() const;
    void                    SetValue( BigInt nNewValue );
    void                    SetUserValue( BigInt nNewValue );
    BigInt                  GetValue() const;
    sal_Bool                    IsValueModified() const;

    void                    SetEmptyValue();
    sal_Bool                    IsEmptyValue() const { return !GetField()->GetText().Len(); }

    BigInt                  GetCorrectedValue() const { return mnCorrectedValue; }

    BigInt                  Normalize( BigInt nValue ) const;
    BigInt                  Denormalize( BigInt nValue ) const;
};

// ---------------------
// - LongCurrencyField -
// ---------------------

class VCL_DLLPUBLIC LongCurrencyField : public SpinField, public LongCurrencyFormatter
{
//#if 0 // _SOLAR__PRIVATE
    friend void ImplNewLongCurrencyFieldValue( LongCurrencyField*, BigInt );
//#endif

private:
    BigInt          mnSpinSize;
    BigInt          mnFirst;
    BigInt          mnLast;

protected:
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );

public:
                    LongCurrencyField( Window* pParent, WinBits nWinStyle );
                    LongCurrencyField( Window* pParent, const ResId& rResId );
                    ~LongCurrencyField();

    long            PreNotify( NotifyEvent& rNEvt );
    long            Notify( NotifyEvent& rNEvt );

    void            Modify();
    void            Up();
    void            Down();
    void            First();
    void            Last();

    void            SetFirst( BigInt nNewFirst ) { mnFirst = nNewFirst; }
    BigInt          GetFirst() const { return mnFirst; }
    void            SetLast( BigInt nNewLast ) { mnLast = nNewLast; }
    BigInt          GetLast() const { return mnLast; }
    void            SetSpinSize( BigInt nNewSize ) { mnSpinSize = nNewSize; }
    BigInt          GetSpinSize() const { return mnSpinSize; }
};

// -------------------
// - LongCurrencyBox -
// -------------------

class VCL_DLLPUBLIC LongCurrencyBox : public ComboBox, public LongCurrencyFormatter
{
public:
                    LongCurrencyBox( Window* pParent, WinBits nWinStyle );
                    LongCurrencyBox( Window* pParent, const ResId& rResId );
                    ~LongCurrencyBox();

    long            PreNotify( NotifyEvent& rNEvt );
    long            Notify( NotifyEvent& rNEvt );

    void            Modify();
    void            ReformatAll();

    void            InsertValue( BigInt nValue,
                                 sal_uInt16 nPos = COMBOBOX_APPEND );
    void            RemoveValue( BigInt nValue );
    BigInt          GetValue() const
                        { return LongCurrencyFormatter::GetValue(); }
    BigInt          GetValue( sal_uInt16 nPos ) const;
    sal_uInt16          GetValuePos( BigInt nValue ) const;
};

#endif // _LONGCURR_HXX
