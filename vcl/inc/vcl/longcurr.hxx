/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: longcurr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:59:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _LONGCURR_HXX
#define _LONGCURR_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
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
    SAL_DLLPRIVATE friend BOOL ImplLongCurrencyReformat( const XubString&, BigInt, BigInt, USHORT, const LocaleDataWrapper&, XubString&, LongCurrencyFormatter& );
    SAL_DLLPRIVATE void        ImpInit();

protected:
    BigInt                  mnFieldValue;
    BigInt                  mnLastValue;
    BigInt                  mnMin;
    BigInt                  mnMax;
    BigInt                  mnCorrectedValue;
    String                  maCurrencySymbol;
    USHORT                  mnType;
    USHORT                  mnDecimalDigits;
    BOOL                    mbThousandSep;

                            LongCurrencyFormatter();
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );

public:
                            ~LongCurrencyFormatter();

    virtual void            Reformat();
    virtual void            ReformatAll();

    void                    SetUseThousandSep( BOOL b );
    BOOL                    IsUseThousandSep() const { return mbThousandSep; }

    void                    SetCurrencySymbol( const String& rStr );
    String                  GetCurrencySymbol() const;

    void                    SetMin( BigInt nNewMin );
    BigInt                  GetMin() const { return mnMin; }
    void                    SetMax( BigInt nNewMax );
    BigInt                  GetMax() const { return mnMax; }

    void                    SetDecimalDigits( USHORT nDigits );
    USHORT                  GetDecimalDigits() const;
    void                    SetValue( BigInt nNewValue );
    void                    SetUserValue( BigInt nNewValue );
    BigInt                  GetValue() const;
    BOOL                    IsValueModified() const;

    void                    SetEmptyValue();
    BOOL                    IsEmptyValue() const { return !GetField()->GetText().Len(); }

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
                                 USHORT nPos = COMBOBOX_APPEND );
    void            RemoveValue( BigInt nValue );
    BigInt          GetValue() const
                        { return LongCurrencyFormatter::GetValue(); }
    BigInt          GetValue( USHORT nPos ) const;
    USHORT          GetValuePos( BigInt nValue ) const;
};

#endif // _LONGCURR_HXX
