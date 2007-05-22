/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LineChartType.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:49:19 $
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
#ifndef CHART_LINECHARTTYPE_HXX
#define CHART_LINECHARTTYPE_HXX

#include "ChartType.hxx"
#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_CHART2_CURVESTYLE_HPP_
#include <com/sun/star/chart2/CurveStyle.hpp>
#endif

namespace chart
{

class LineChartType : public ChartType
{
public:
    LineChartType(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~LineChartType();

    APPHELPER_XSERVICEINFO_DECL()

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LineChartType )

protected:
    explicit LineChartType( const LineChartType & rOther );

    // ____ XChartType ____
    virtual ::rtl::OUString SAL_CALL getChartType()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);
};

} //  namespace chart

// CHART_LINECHARTTYPE_HXX
#endif
