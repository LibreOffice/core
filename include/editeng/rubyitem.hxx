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

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>

class EDITENG_DLLPUBLIC SvxRubyItem : public SfxPoolItem
{
    OUString m_aText;
    css::text::RubyAdjust m_eAdjustment = css::text::RubyAdjust::RubyAdjust_LEFT;
    sal_Int16 m_ePosition = css::text::RubyPosition::ABOVE;

public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxRubyItem)
    explicit SvxRubyItem(const sal_uInt16 nId);

    const OUString& GetText() const;
    void SetText(OUString aValue);

    css::text::RubyAdjust GetAdjustment() const;
    void SetAdjustment(css::text::RubyAdjust eValue);

    sal_Int16 GetPosition() const;
    void SetPosition(sal_Int16 eValue);

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    virtual SvxRubyItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
