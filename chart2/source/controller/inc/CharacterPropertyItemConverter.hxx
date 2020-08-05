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

#include "ItemConverter.hxx"
#include <com/sun/star/awt/Size.hpp>

#include <optional>

namespace chart::wrapper {

class CharacterPropertyItemConverter final : public ItemConverter
{
public:
    CharacterPropertyItemConverter(
        const css::uno::Reference<css::beans::XPropertySet>& rPropertySet,
        SfxItemPool& rItemPool );

    CharacterPropertyItemConverter(
        const css::uno::Reference<css::beans::XPropertySet>& rPropertySet,
        SfxItemPool& rItemPool,
        const css::awt::Size* pRefSize,
        const OUString & rRefSizePropertyName,
        const css::uno::Reference<css::beans::XPropertySet>& rRefSizePropSet = css::uno::Reference<css::beans::XPropertySet>() );

    virtual ~CharacterPropertyItemConverter() override;

private:
    virtual const sal_uInt16* GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;
    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet ) override;

    const css::uno::Reference<css::beans::XPropertySet>& GetRefSizePropertySet() const;

    OUString m_aRefSizePropertyName;
    css::uno::Reference<css::beans::XPropertySet> m_xRefSizePropSet;
    std::optional<css::awt::Size> m_pRefSize;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
