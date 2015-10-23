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
#include <svl/poolitem.hxx>
#include <com/sun/star/frame/status/Visibility.hpp>

class SVL_DLLPUBLIC SfxVisibilityItem: public SfxPoolItem
{
    css::frame::status::Visibility m_nValue;

public:
    TYPEINFO_OVERRIDE();

    explicit SfxVisibilityItem(sal_uInt16 which = 0, bool bVisible = true):
        SfxPoolItem(which)
    {
        m_nValue.bVisible = bVisible;
    }

    SfxVisibilityItem(sal_uInt16 which, SvStream & rStream);

    SfxVisibilityItem(const SfxVisibilityItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    {}

    virtual ~SfxVisibilityItem() {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const override;

    virtual bool QueryValue( css::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const override;

    virtual bool PutValue( const css::uno::Any& rVal,
                           sal_uInt8 nMemberId ) override;

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const override;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const override;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const override;

    OUString GetValueTextByVal(bool bTheValue) const;

    bool GetValue() const { return m_nValue.bVisible; }
};

#endif // INCLUDED_SVL_VISITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
