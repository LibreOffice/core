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

#ifndef INCLUDED_SVL_CUSTRITM_HXX
#define INCLUDED_SVL_CUSTRITM_HXX

#include <svl/svldllapi.h>
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>


DBG_NAMEEX_VISIBILITY(CntUnencodedStringItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUnencodedStringItem: public SfxPoolItem
{
    OUString m_aValue;

public:
    TYPEINFO();

    CntUnencodedStringItem(sal_uInt16 which = 0): SfxPoolItem(which)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(sal_uInt16 which, const OUString & rTheValue):
        SfxPoolItem(which), m_aValue(rTheValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(const CntUnencodedStringItem & rItem):
        SfxPoolItem(rItem), m_aValue(rItem.m_aValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    virtual ~CntUnencodedStringItem() { DBG_DTOR(CntUnencodedStringItem, 0); }

    virtual bool operator ==(const SfxPoolItem & rItem) const;

    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual int Compare(SfxPoolItem const & rWith,
                        IntlWrapper const & rIntlWrapper) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    const OUString & GetValue() const { return m_aValue; }

    inline void SetValue(const OUString & rTheValue);
};

inline void CntUnencodedStringItem::SetValue(const OUString & rTheValue)
{
    DBG_ASSERT(GetRefCount() == 0,
               "CntUnencodedStringItem::SetValue(): Pooled item");
    m_aValue = rTheValue;
}

#endif // INCLUDED_SVL_CUSTRITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
