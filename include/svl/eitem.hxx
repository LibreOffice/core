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

#ifndef INCLUDED_SVL_EITEM_HXX
#define INCLUDED_SVL_EITEM_HXX

#include <svl/svldllapi.h>
#include <svl/cenumitm.hxx>



class SVL_DLLPUBLIC SfxEnumItem
    : public SfxEnumItemInterface
{
    sal_uInt16 m_nValue;

protected:
    explicit SfxEnumItem(sal_uInt16 const nWhich =0, sal_uInt16 const nValue =0)
        : SfxEnumItemInterface(nWhich)
        , m_nValue(nValue)
    { }

    SfxEnumItem(const SfxEnumItem & rItem)
        : SfxEnumItemInterface(rItem)
        , m_nValue(rItem.m_nValue)
    { }

    SfxEnumItem(sal_uInt16 const nWhich, SvStream & rStream);

public:
    TYPEINFO_OVERRIDE();

    sal_uInt16 GetValue() const { return m_nValue; }

    void SetValue(sal_uInt16 nTheValue);

    // SfxPoolItem
    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const override;

    virtual sal_uInt16 GetEnumValue() const override;

    virtual void SetEnumValue(sal_uInt16 nTheValue) override;

};



class SVL_DLLPUBLIC SfxBoolItem
    : public SfxPoolItem
{
    bool m_bValue;

public:
    TYPEINFO_OVERRIDE();
    static SfxPoolItem* CreateDefault();

    explicit SfxBoolItem(sal_uInt16 const nWhich = 0, bool const bValue = false)
        : SfxPoolItem(nWhich)
        , m_bValue(bValue)
    { }

    SfxBoolItem(SfxBoolItem const& rItem)
        : SfxPoolItem(rItem)
        , m_bValue(rItem.m_bValue)
    { }

    SfxBoolItem(sal_uInt16 nWhich, SvStream & rStream);

    bool GetValue() const { return m_bValue; }

    void SetValue(bool const bTheValue) { m_bValue = bTheValue; }

    // SfxPoolItem
    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8) override;


    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
        override;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const override;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const override;

    virtual OUString GetValueTextByVal(bool bTheValue) const;
};

#endif // INCLUDED_SVL_EITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
