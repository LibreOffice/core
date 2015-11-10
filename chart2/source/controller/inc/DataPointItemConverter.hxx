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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DATAPOINTITEMCONVERTER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DATAPOINTITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"

#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>

class SdrModel;

namespace chart { namespace wrapper {

class DataPointItemConverter : public ItemConverter
{
public:
    DataPointItemConverter(
        const css::uno::Reference<css::frame::XModel>& xChartModel,
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        const css::uno::Reference<css::beans::XPropertySet>& rPropertySet,
        const css::uno::Reference<css::chart2::XDataSeries>& xSeries,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const css::uno::Reference<css::lang::XMultiServiceFactory>& xNamedPropertyContainerFactory,
        GraphicPropertyItemConverter::eGraphicObjectType eMapTo =
            GraphicPropertyItemConverter::FILLED_DATA_POINT,
        const css::awt::Size* pRefSize = nullptr,
        bool bDataSeries = false,
        bool bUseSpecialFillColor = false,
        sal_Int32 nSpecialFillColor = 0,
        bool bOverwriteLabelsForAttributedDataPointsAlso = false,
        sal_Int32 nNumberFormat = 0,
        sal_Int32 nPercentNumberFormat = 0 );

    virtual ~DataPointItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet ) override;

protected:
    virtual const sal_uInt16 * GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw (css::uno::Exception) override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw (css::uno::Exception) override;

private:
    ::std::vector< ItemConverter * >    m_aConverters;
    bool                                m_bDataSeries;
    bool                                m_bOverwriteLabelsForAttributedDataPointsAlso;
    bool                                m_bUseSpecialFillColor;
    sal_Int32                           m_nSpecialFillColor;
    sal_Int32                           m_nNumberFormat;
    sal_Int32                           m_nPercentNumberFormat;
    css::uno::Sequence<sal_Int32>       m_aAvailableLabelPlacements;
    bool                                m_bForbidPercentValue;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
