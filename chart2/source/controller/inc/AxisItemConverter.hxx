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
#ifndef CHART_AXISITEMCONVERTER_HXX
#define CHART_AXISITEMCONVERTER_HXX

#include <chartview/ExplicitScaleValues.hxx>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/awt/Size.hpp>

#include "ItemConverter.hxx"

#include <vector>
#include <memory>

class SdrModel;

namespace chart
{
namespace wrapper
{

class AxisItemConverter : public ::comphelper::ItemConverter
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc,
        ExplicitScaleData * pScale = NULL,
        ExplicitIncrementData * pIncrement = NULL,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~AxisItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::std::vector< ItemConverter * >               m_aConverters;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XAxis >  m_xAxis;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument >      m_xChartDoc;

    ExplicitScaleData*      m_pExplicitScale;
    ExplicitIncrementData*  m_pExplicitIncrement;
};

} //  namespace wrapper
} //  namespace chart

// CHART_AXISITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
