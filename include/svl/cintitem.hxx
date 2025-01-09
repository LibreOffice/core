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

class SVL_DLLPUBLIC CntByteItem: public SfxPoolItem
{
    sal_uInt8 m_nValue;

public:
    DECLARE_ITEM_TYPE_FUNCTION(CntByteItem)
    CntByteItem(sal_uInt16 which, sal_uInt8 nTheValue):
        SfxPoolItem(which), m_nValue(nTheValue) {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool supportsHashCode() const override;
    virtual size_t hashCode() const override final;

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal,
                          sal_uInt8 nMemberId) override;

    virtual CntByteItem* Clone(SfxItemPool * = nullptr) const override;

    sal_uInt8 GetValue() const { return m_nValue; }

    void SetValue(sal_uInt8 nTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nValue = nTheValue; }
};

class SVL_DLLPUBLIC CntUInt16Item: public SfxPoolItem
{
    sal_uInt16 m_nValue;

public:
    DECLARE_ITEM_TYPE_FUNCTION(CntUInt16Item)
    CntUInt16Item(sal_uInt16 which, sal_uInt16 nTheValue):
        SfxPoolItem(which), m_nValue(nTheValue)
    {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool supportsHashCode() const override final;
    virtual size_t hashCode() const override final;

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal,
                          sal_uInt8 nMemberId) override;

    virtual CntUInt16Item* Clone(SfxItemPool * = nullptr) const override;

    sal_uInt16 GetValue() const { return m_nValue; }

    void SetValue(sal_uInt16 nTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nValue = nTheValue; }
};


class SVL_DLLPUBLIC CntInt32Item: public SfxPoolItem
{
    sal_Int32 m_nValue;

public:
    DECLARE_ITEM_TYPE_FUNCTION(CntInt32Item)
    CntInt32Item(sal_uInt16 which, sal_Int32 nTheValue):
        SfxPoolItem(which), m_nValue(nTheValue)
    {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool supportsHashCode() const override final;
    virtual size_t hashCode() const override final;

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal,
                          sal_uInt8 nMemberId) override;

    virtual CntInt32Item* Clone(SfxItemPool * = nullptr) const override;

    sal_Int32 GetValue() const { return m_nValue; }

    void SetValue(sal_Int32 nTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nValue = nTheValue; }
};


class SVL_DLLPUBLIC CntUInt32Item: public SfxPoolItem
{
    sal_uInt32 m_nValue;

public:
    DECLARE_ITEM_TYPE_FUNCTION(CntUInt32Item)
    CntUInt32Item(sal_uInt16 which, sal_uInt32 nTheValue):
        SfxPoolItem(which), m_nValue(nTheValue)
    {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool supportsHashCode() const override final;
    virtual size_t hashCode() const override final;

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal,
                          sal_uInt8 nMemberId) override;

    virtual CntUInt32Item* Clone(SfxItemPool * = nullptr) const override;

    sal_uInt32 GetValue() const { return m_nValue; }

    void SetValue(sal_uInt32 nTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nValue = nTheValue; }
};

#endif // INCLUDED_SVL_CINTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
