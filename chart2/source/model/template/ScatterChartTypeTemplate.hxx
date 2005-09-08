/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScatterChartTypeTemplate.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:24:41 $
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
#ifndef CHART_SCATTERCHARTTYPETEMPLATE_HXX
#define CHART_SCATTERCHARTTYPETEMPLATE_HXX

#include "ChartTypeTemplate.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_STACKMODE_HPP_
#include <com/sun/star/chart2/StackMode.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_CURVESTYLE_HPP_
#include <com/sun/star/chart2/CurveStyle.hpp>
#endif

namespace chart
{

class ScatterChartTypeTemplate :
        public helper::MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    explicit ScatterChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & rServiceName,
        ::com::sun::star::chart2::CurveStyle eCurveStyle,
        bool bSymbols,
        bool bHasLines = true,
        sal_Int32 nDim = 2 );
    virtual ~ScatterChartTypeTemplate();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartTypeTemplate ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > SAL_CALL
        createDiagram( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XDataSeries > >& aSeriesSeq )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ChartTypeTemplate ____
    virtual sal_Int32 getDimension() const;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType > getDefaultChartType()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::chart2::CurveStyle
                       m_eCurveStyle;
    bool               m_bHasSymbols;
    bool               m_bHasLines;
    sal_Int32          m_nDim;
};

} //  namespace chart

// CHART_SCATTERCHARTTYPETEMPLATE_HXX
#endif
