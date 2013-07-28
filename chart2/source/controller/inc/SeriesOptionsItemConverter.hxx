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
#ifndef CHART_SERIESOPTIONSITEMCONVERTER_HXX
#define CHART_SERIESOPTIONSITEMCONVERTER_HXX

#include "ItemConverter.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>

namespace chart
{
namespace wrapper
{

class SeriesOptionsItemConverter :
        public ::comphelper::ItemConverter
{
public:
    SeriesOptionsItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool );
    virtual ~SeriesOptionsItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >  m_xChartModel;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext>   m_xCC;

    bool m_bAttachToMainAxis;
    bool m_bSupportingOverlapAndGapWidthProperties;
    bool m_bSupportingBarConnectors;

    sal_Int32 m_nBarOverlap;
    sal_Int32 m_nGapWidth;
    sal_Bool  m_bConnectBars;

    bool m_bSupportingAxisSideBySide;
    bool m_bGroupBarsPerAxis;
    bool m_bAllSeriesAttachedToSameAxis;
    sal_Int32 m_nAllSeriesAxisIndex;

    bool m_bSupportingStartingAngle;
    sal_Int32 m_nStartingAngle;

    bool m_bClockwise;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > m_xCooSys;

    ::com::sun::star::uno::Sequence< sal_Int32 > m_aSupportedMissingValueTreatments;
    sal_Int32 m_nMissingValueTreatment;

    bool m_bSupportingPlottingOfHiddenCells;
    bool m_bIncludeHiddenCells;
};

} //  namespace wrapper
} //  namespace chart

// CHART_SERIESOPTIONSITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
