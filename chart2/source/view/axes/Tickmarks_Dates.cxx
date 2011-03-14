/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "Tickmarks_Dates.hxx"
#include "DateScaling.hxx"
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include "DateHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
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
        DBG_ASSERT( m_xInverseScaling.is(), "each Scaling needs to return a inverse Scaling" );
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

//.............................................................................
} //namespace chart
//.............................................................................
