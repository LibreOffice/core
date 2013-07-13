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

#include "Tickmarks_Dates.hxx"
#include "DateScaling.hxx"
#include <rtl/math.hxx>
#include "DateHelper.hxx"

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;
using ::basegfx::B2DVector;
using ::com::sun::star::chart::TimeUnit::DAY;
using ::com::sun::star::chart::TimeUnit::MONTH;
using ::com::sun::star::chart::TimeUnit::YEAR;

DateTickFactory::DateTickFactory(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement )
            : m_aScale( rScale )
            , m_aIncrement( rIncrement )
            , m_xInverseScaling(NULL)
{
    //@todo: make sure that the scale is valid for the scaling

    if( m_aScale.Scaling.is() )
    {
        m_xInverseScaling = m_aScale.Scaling->getInverseScaling();
        OSL_ENSURE( m_xInverseScaling.is(), "each Scaling needs to return a inverse Scaling" );
    }

    m_fScaledVisibleMin = m_aScale.Minimum;
    if( m_xInverseScaling.is() )
        m_fScaledVisibleMin = m_aScale.Scaling->doScaling(m_fScaledVisibleMin);

    m_fScaledVisibleMax = m_aScale.Maximum;
    if( m_xInverseScaling.is() )
        m_fScaledVisibleMax = m_aScale.Scaling->doScaling(m_fScaledVisibleMax);
}

DateTickFactory::~DateTickFactory()
{
}

void DateTickFactory::getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos, bool bShifted ) const
{
    rAllTickInfos.resize(2);
    ::std::vector< TickInfo >& rMajorTicks = rAllTickInfos[0];
    ::std::vector< TickInfo >& rMinorTicks = rAllTickInfos[1];
    rMajorTicks.clear();
    rMinorTicks.clear();

    Date aNull(m_aScale.NullDate);

    Date aDate = aNull + static_cast<long>(::rtl::math::approxFloor(m_aScale.Minimum));
    Date aMaxDate = aNull + static_cast<long>(::rtl::math::approxFloor(m_aScale.Maximum));

    uno::Reference< chart2::XScaling > xScaling(m_aScale.Scaling);
    uno::Reference< chart2::XScaling > xInverseScaling(m_xInverseScaling);
    if( bShifted )
    {
        xScaling = new DateScaling(aNull,m_aScale.TimeResolution,true/*bShifted*/);
        xInverseScaling = xScaling->getInverseScaling();
    }

    //create major date tickinfos
    while( aDate<= aMaxDate )
    {
        if( bShifted && aDate==aMaxDate )
            break;

        TickInfo aNewTick(xInverseScaling); aNewTick.fScaledTickValue = aDate - aNull;

        if( xInverseScaling.is() )
            aNewTick.fScaledTickValue = xScaling->doScaling(aNewTick.fScaledTickValue);
        rMajorTicks.push_back( aNewTick );

        if(m_aIncrement.MajorTimeInterval.Number<=0)
            break;

        //find next major date
        switch( m_aIncrement.MajorTimeInterval.TimeUnit )
        {
        case DAY:
            aDate += m_aIncrement.MajorTimeInterval.Number;
            break;
        case YEAR:
            aDate = DateHelper::GetDateSomeYearsAway( aDate, m_aIncrement.MajorTimeInterval.Number );
            break;
        case MONTH:
        default:
            aDate = DateHelper::GetDateSomeMonthsAway( aDate, m_aIncrement.MajorTimeInterval.Number );
            break;
        }
    }

    //create minor date tickinfos
    aDate = aNull + static_cast<long>(::rtl::math::approxFloor(m_aScale.Minimum));
    while( aDate<= aMaxDate )
    {
        if( bShifted && aDate==aMaxDate )
            break;

        TickInfo aNewTick(xInverseScaling); aNewTick.fScaledTickValue = aDate - aNull;
        if( xInverseScaling.is() )
            aNewTick.fScaledTickValue = xScaling->doScaling(aNewTick.fScaledTickValue);
        rMinorTicks.push_back( aNewTick );

        if(m_aIncrement.MinorTimeInterval.Number<=0)
            break;

        //find next minor date
        switch( m_aIncrement.MinorTimeInterval.TimeUnit )
        {
        case DAY:
            aDate += m_aIncrement.MinorTimeInterval.Number;
            break;
        case YEAR:
            aDate = DateHelper::GetDateSomeYearsAway( aDate, m_aIncrement.MinorTimeInterval.Number );
            break;
        case MONTH:
        default:
            aDate = DateHelper::GetDateSomeMonthsAway( aDate, m_aIncrement.MinorTimeInterval.Number );
            break;
        }
    }
}

void DateTickFactory::getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    getAllTicks( rAllTickInfos, false );
}

void DateTickFactory::getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    getAllTicks( rAllTickInfos, true );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
