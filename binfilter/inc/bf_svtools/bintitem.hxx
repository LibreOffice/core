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

#ifndef _BINTITEM_HXX
#define _BINTITEM_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter {

class SfxArguments;

DBG_NAMEEX(SfxBigIntItem)

class SfxBigIntItem: public SfxPoolItem
{
    BigInt aVal;

public:
    TYPEINFO();
    SfxBigIntItem();
    SfxBigIntItem(USHORT nWhich, SvStream &);
    SfxBigIntItem(const SfxBigIntItem&);
    ~SfxBigIntItem() { DBG_DTOR(SfxBigIntItem, 0); }

    virtual SfxItemPresentation GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric,
        SfxMapUnit ePresMetric,
        XubString &rText,
        const ::IntlWrapper * = 0 ) const;

    virtual int operator==(const SfxPoolItem&) const;
    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem &rWith) const;
    virtual SfxPoolItem* Clone(SfxItemPool *pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream &, USHORT nVersion) const;
    virtual SvStream& Store(SvStream &, USHORT nItemVersion) const;

    virtual	BOOL PutValue  ( const com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 );
    virtual	BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;

    virtual SfxFieldUnit GetUnit() const;	 // FUNIT_NONE

    const BigInt& GetValue() const { return aVal; }
    void SetValue(const BigInt& rNewVal)
    {
        DBG_ASSERT(GetRefCount() == 0, "SetValue() with pooled item");
        aVal = rNewVal;
    }
};

}

#endif /* _BINTITEM_HXX */

