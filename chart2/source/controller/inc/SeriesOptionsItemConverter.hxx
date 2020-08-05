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

namespace com::sun::star::chart2 { class XCoordinateSystem; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart::wrapper
{

class SeriesOptionsItemConverter : public ItemConverter
{
public:
    SeriesOptionsItemConverter(
        const css::uno::Reference< css::frame::XModel > & xChartModel,
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        const css::uno::Reference< css::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool );
    virtual ~SeriesOptionsItemConverter() override;

protected:
    virtual const sal_uInt16 * GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet ) override;

private:
    css::uno::Reference< css::frame::XModel >  m_xChartModel;
    css::uno::Reference< css::uno::XComponentContext>   m_xCC;

    bool m_bAttachToMainAxis;
    bool m_bSupportingOverlapAndGapWidthProperties;
    bool m_bSupportingBarConnectors;

    sal_Int32 m_nBarOverlap;
    sal_Int32 m_nGapWidth;
    bool  m_bConnectBars;

    bool m_bSupportingAxisSideBySide;
    bool m_bGroupBarsPerAxis;

    bool m_bSupportingStartingAngle;
    sal_Int32 m_nStartingAngle;

    bool m_bClockwise;
    css::uno::Reference< css::chart2::XCoordinateSystem > m_xCooSys;

    css::uno::Sequence< sal_Int32 > m_aSupportedMissingValueTreatments;
    sal_Int32 m_nMissingValueTreatment;

    bool m_bSupportingPlottingOfHiddenCells;
    bool m_bIncludeHiddenCells;

    bool m_bHideLegendEntry;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
