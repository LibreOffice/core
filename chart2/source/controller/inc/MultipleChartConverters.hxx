/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MultipleChartConverters.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:24:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_MULTIPLECHARTCONVERTERS_HXX
#define CHART2_MULTIPLECHARTCONVERTERS_HXX

#include "MultipleItemConverter.hxx"

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#include <memory>

class SdrModel;

namespace chart
{
namespace wrapper
{

class AllAxisItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllAxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllAxisItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

class AllGridItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllGridItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel  );
    virtual ~AllGridItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

class AllDataLabelItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllDataLabelItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllDataLabelItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

class AllTitleItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllTitleItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllTitleItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

class AllSeriesStatisticsConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllSeriesStatisticsConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool );
    virtual ~AllSeriesStatisticsConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

} //  namespace wrapper
} //  namespace chart

// CHART2_MULTIPLECHARTCONVERTERS_HXX
#endif
