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

#ifndef SFXENUMITEM_HXX
#define SFXENUMITEM_HXX

#include "svl/svldllapi.h"
#include <svl/cenumitm.hxx>

//============================================================================

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
    TYPEINFO();

    sal_uInt16 GetValue() const { return m_nValue; }

    void SetValue(sal_uInt16 nTheValue);

    // SfxPoolItem
    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE;

    virtual sal_uInt16 GetEnumValue() const SAL_OVERRIDE;

    virtual void SetEnumValue(sal_uInt16 nTheValue) SAL_OVERRIDE;

};

//============================================================================

class SVL_DLLPUBLIC SfxBoolItem
    : public SfxPoolItem
{
    bool m_bValue;

public:
    TYPEINFO();

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
    virtual int operator ==(const SfxPoolItem & rItem) const SAL_OVERRIDE;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const SAL_OVERRIDE;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const SAL_OVERRIDE;

    virtual bool QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8 = 0)
        const SAL_OVERRIDE;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 = 0)
        SAL_OVERRIDE;


    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
        SAL_OVERRIDE;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const SAL_OVERRIDE;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const SAL_OVERRIDE;

    virtual sal_uInt16 GetValueCount() const;

    virtual OUString GetValueTextByVal(sal_Bool bTheValue) const;

};

#endif // SFXENUMITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
