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

class SVL_DLLPUBLIC CntUnencodedStringItem: public SfxPoolItem
{
    OUString m_aValue;

public:
    TYPEINFO_OVERRIDE();

    CntUnencodedStringItem(sal_uInt16 which = 0): SfxPoolItem(which)
    {}

    CntUnencodedStringItem(sal_uInt16 which, const OUString & rTheValue):
        SfxPoolItem(which), m_aValue(rTheValue)
    {}

    CntUnencodedStringItem(const CntUnencodedStringItem & rItem):
        SfxPoolItem(rItem), m_aValue(rItem.m_aValue)
    {}

    virtual ~CntUnencodedStringItem() {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const override;

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
