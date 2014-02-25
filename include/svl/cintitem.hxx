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

#ifndef INCLUDED_SVL_CINTITEM_HXX
#define INCLUDED_SVL_CINTITEM_HXX

#include <svl/svldllapi.h>
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>


DBG_NAMEEX_VISIBILITY(CntByteItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntByteItem: public SfxPoolItem
{
    sal_uInt8 m_nValue;

public:
    TYPEINFO();

    CntByteItem(sal_uInt16 which = 0, sal_uInt8 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue) { DBG_CTOR(CntByteItem, 0); }

    CntByteItem(const CntByteItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntByteItem, 0); }

    virtual ~CntByteItem() { DBG_DTOR(CntByteItem, 0); }

    virtual bool operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_uInt8 GetMin() const;

    virtual sal_uInt8 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_uInt8 GetValue() const { return m_nValue; }

    inline void SetValue(sal_uInt8 nTheValue);
};

inline void CntByteItem::SetValue(sal_uInt8 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntByteItem::SetValue(): Pooled item");
    m_nValue = nTheValue;
}


DBG_NAMEEX_VISIBILITY(CntUInt16Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUInt16Item: public SfxPoolItem
{
    sal_uInt16 m_nValue;

public:
    TYPEINFO();

    CntUInt16Item(sal_uInt16 which = 0, sal_uInt16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntUInt16Item, 0); }

    CntUInt16Item(sal_uInt16 which, SvStream & rStream);

    CntUInt16Item(const CntUInt16Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntUInt16Item, 0); }

    virtual ~CntUInt16Item() { DBG_DTOR(CntUInt16Item, 0); }

    virtual bool operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_uInt16 GetMin() const;

    virtual sal_uInt16 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_uInt16 GetValue() const { return m_nValue; }

    inline void SetValue(sal_uInt16 nTheValue);
};

inline void CntUInt16Item::SetValue(sal_uInt16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntUInt16Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}


DBG_NAMEEX_VISIBILITY(CntInt32Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntInt32Item: public SfxPoolItem
{
    sal_Int32 m_nValue;

public:
    TYPEINFO();

    CntInt32Item(sal_uInt16 which = 0, sal_Int32 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntInt32Item, 0); }

    CntInt32Item(sal_uInt16 which, SvStream & rStream);

    CntInt32Item(const CntInt32Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntInt32Item, 0); }

    virtual ~CntInt32Item() { DBG_DTOR(CntInt32Item, 0); }

    virtual bool operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream &, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_Int32 GetMin() const;

    virtual sal_Int32 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_Int32 GetValue() const { return m_nValue; }

    inline void SetValue(sal_Int32 nTheValue);
};

inline void CntInt32Item::SetValue(sal_Int32 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntInt32Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}


DBG_NAMEEX_VISIBILITY(CntUInt32Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUInt32Item: public SfxPoolItem
{
    sal_uInt32 m_nValue;

public:
    TYPEINFO();

    CntUInt32Item(sal_uInt16 which = 0, sal_uInt32 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntUInt32Item, 0); }

    CntUInt32Item(sal_uInt16 nWhich, SvStream & rStream);

    CntUInt32Item(const CntUInt32Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntUInt32Item, 0); }

    virtual ~CntUInt32Item() { DBG_DTOR(CntUInt32Item, 0); }

    virtual bool operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_uInt32 GetMin() const;

    virtual sal_uInt32 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_uInt32 GetValue() const { return m_nValue; }

    inline void SetValue(sal_uInt32 nTheValue);
};

inline void CntUInt32Item::SetValue(sal_uInt32 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntUInt32Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

#endif // INCLUDED_SVL_CINTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
