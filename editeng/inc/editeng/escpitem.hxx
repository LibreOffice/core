/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_ESCPITEM_HXX
#define _SVX_ESCPITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxEscapementItem -----------------------------------------------

#define DFLT_ESC_SUPER   33     // 1/3
#define DFLT_ESC_SUB    -33     // also 1/3 previously 8/100
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
#define DFLT_ESC_AUTO_SUB  -101

/*  [Description]

    This item describes the writing position.
*/

class EDITENG_DLLPUBLIC SvxEscapementItem : public SfxEnumItemInterface
{
    short nEsc;
    sal_uInt8  nProp;
public:
    TYPEINFO();

    SvxEscapementItem( const sal_uInt16 nId  );
    SvxEscapementItem( const SvxEscapement eEscape,
                   const sal_uInt16 nId  );
    SvxEscapementItem( const short nEsc, const sal_uInt8 nProp,
                   const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;

    inline void SetEscapement( const SvxEscapement eNew )
    {
        if( SVX_ESCAPEMENT_OFF == eNew )
            nEsc = 0, nProp = 100;
        else
            if( SVX_ESCAPEMENT_SUPERSCRIPT == eNew )
                nEsc = DFLT_ESC_SUPER, nProp = DFLT_ESC_PROP;
            else
                nEsc = DFLT_ESC_SUB, nProp = DFLT_ESC_PROP;
    }
    inline SvxEscapement GetEscapement() const { return static_cast< SvxEscapement >( GetEnumValue() ); }

    inline short &GetEsc() { return nEsc; }
    inline short GetEsc() const { return nEsc; }

    inline sal_uInt8 &GetProp() { return nProp; }
    inline sal_uInt8 GetProp() const { return nProp; }

    inline SvxEscapementItem& operator=(const SvxEscapementItem& rEsc)
        {
            nEsc  = rEsc.GetEsc();
            nProp = rEsc.GetProp();
            return *this;
        }

    virtual sal_uInt16      GetValueCount() const;
    virtual rtl::OUString   GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16      GetEnumValue() const;
    virtual void            SetEnumValue( sal_uInt16 nNewVal );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
