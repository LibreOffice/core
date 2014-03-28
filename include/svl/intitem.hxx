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

#ifndef INCLUDED_SVL_INTITEM_HXX
#define INCLUDED_SVL_INTITEM_HXX

#include <svl/svldllapi.h>
#include <svl/cintitem.hxx>


class SVL_DLLPUBLIC SfxByteItem: public CntByteItem
{
public:
    TYPEINFO_OVERRIDE();

    explicit SfxByteItem(sal_uInt16 which = 0, sal_uInt8 nValue = 0):
        CntByteItem(which, nValue) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE
    { return new SfxByteItem(*this); }
};

class SVL_DLLPUBLIC SfxInt16Item: public SfxPoolItem
{
    sal_Int16 m_nValue;

public:
    TYPEINFO_OVERRIDE();

    explicit SfxInt16Item(sal_uInt16 which = 0, sal_Int16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    {}

    SfxInt16Item(sal_uInt16 nWhich, SvStream & rStream);

    SfxInt16Item(const SfxInt16Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    {}

    virtual ~SfxInt16Item() {}

    virtual bool operator ==(const SfxPoolItem & rItem) const SAL_OVERRIDE;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const SAL_OVERRIDE;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const SAL_OVERRIDE;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;

    virtual bool PutValue( const com::sun::star::uno::Any& rVal,
                           sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE;

    virtual sal_Int16 GetMin() const;

    virtual sal_Int16 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_Int16 GetValue() const { return m_nValue; }

    inline void SetValue(sal_Int16 nTheValue);
};

inline void SfxInt16Item::SetValue(sal_Int16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "SfxInt16Item::SetValue(); Pooled item");
    m_nValue = nTheValue;
}


class SVL_DLLPUBLIC SfxUInt16Item: public CntUInt16Item
{
public:
    TYPEINFO_OVERRIDE();

    explicit SfxUInt16Item(sal_uInt16 which = 0, sal_uInt16 nValue = 0):
        CntUInt16Item(which, nValue) {}

    SfxUInt16Item(sal_uInt16 which, SvStream & rStream):
        CntUInt16Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE
    { return new SfxUInt16Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE
    { return new SfxUInt16Item(*this); }
};


class SVL_DLLPUBLIC SfxInt32Item: public CntInt32Item
{
public:
    TYPEINFO_OVERRIDE();

    explicit SfxInt32Item(sal_uInt16 which = 0, sal_Int32 nValue = 0):
        CntInt32Item(which, nValue) {}

    SfxInt32Item(sal_uInt16 which, SvStream & rStream):
        CntInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE
    { return new SfxInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE
    { return new SfxInt32Item(*this); }

};


class SVL_DLLPUBLIC SfxUInt32Item: public CntUInt32Item
{
public:
    TYPEINFO_OVERRIDE();

    explicit SfxUInt32Item(sal_uInt16 which = 0, sal_uInt32 nValue = 0):
        CntUInt32Item(which, nValue) {}

    SfxUInt32Item(sal_uInt16 which, SvStream & rStream):
        CntUInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE
    { return new SfxUInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE
    { return new SfxUInt32Item(*this); }
};

#endif // INCLUDED_SVL_INTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
