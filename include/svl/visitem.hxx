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

#ifndef INCLUDED_SVL_VISITEM_HXX
#define INCLUDED_SVL_VISITEM_HXX

#include <svl/svldllapi.h>
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/frame/status/Visibility.hpp>


DBG_NAMEEX_VISIBILITY(SfxVisibilityItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxVisibilityItem: public SfxPoolItem
{
    ::com::sun::star::frame::status::Visibility m_nValue;

public:
    TYPEINFO_OVERRIDE();

    explicit SfxVisibilityItem(sal_uInt16 which = 0, bool bVisible = true):
        SfxPoolItem(which)
    {
        m_nValue.bVisible = bVisible;
        DBG_CTOR(SfxVisibilityItem, 0);
    }

    SfxVisibilityItem(sal_uInt16 which, SvStream & rStream);

    SfxVisibilityItem(const SfxVisibilityItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(SfxVisibilityItem, 0); }

    virtual ~SfxVisibilityItem() { DBG_DTOR(SfxVisibilityItem, 0); }

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

    virtual sal_uInt16 GetValueCount() const;

    virtual OUString GetValueTextByVal(sal_Bool bTheValue) const;

    bool GetValue() const { return m_nValue.bVisible; }

    void SetValue(bool bVisible) { m_nValue.bVisible = bVisible; }
};

#endif // INCLUDED_SVL_VISITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
