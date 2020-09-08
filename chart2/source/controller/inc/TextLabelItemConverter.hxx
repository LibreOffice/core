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

#include <com/sun/star/uno/Sequence.h>

#include <vector>

namespace com::sun::star::awt { struct Size; }
namespace com::sun::star::chart2 { class XDataSeries; }
namespace com::sun::star::frame { class XModel; }

namespace chart::wrapper {

class TextLabelItemConverter : public ItemConverter
{
public:
    TextLabelItemConverter(
        const css::uno::Reference<css::frame::XModel>& xChartModel,
        const css::uno::Reference<css::beans::XPropertySet>& rPropertySet,
        const css::uno::Reference<css::chart2::XDataSeries>& xSeries,
        SfxItemPool& rItemPool,
        const css::awt::Size* pRefSize,
        bool bDataSeries,
        sal_Int32 nNumberFormat,
        sal_Int32 nPercentNumberFormat );

    virtual ~TextLabelItemConverter() override;

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet ) override;

protected:
    virtual const sal_uInt16* GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet ) override;

private:
    std::vector<std::unique_ptr<ItemConverter>> maConverters;
    sal_Int32                           mnNumberFormat;
    sal_Int32                           mnPercentNumberFormat;
    css::uno::Sequence<sal_Int32>       maAvailableLabelPlacements;

    bool mbDataSeries:1;
    bool mbForbidPercentValue:1;

    css::uno::Reference<css::chart2::XDataSeries> m_xSeries;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
