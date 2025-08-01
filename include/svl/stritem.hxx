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
#pragma once

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <cassert>
#include <utility>


class SVL_DLLPUBLIC SfxStringItem: public SfxPoolItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxStringItem)
    SfxStringItem(sal_uInt16 which = 0)
        : SfxPoolItem(which) {}

    SfxStringItem(sal_uInt16 which, const OUString & rValue):
        SfxPoolItem(which), m_aValue(rValue) {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;
    // Note that all the subclasses are currently marked as false since we haven't check them to
    // be safe under hashing
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override final { return m_aValue.hashCode(); }

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&) const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual SfxStringItem* Clone(SfxItemPool * = nullptr) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    const OUString & GetValue() const { return m_aValue; }

    void SetValue(const OUString & rTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_aValue = rTheValue; }

private:
    OUString m_aValue;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
