/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NetChartTypeTemplate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:22:43 $
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
#ifndef CHART_NETCHARTTYPETEMPLATE_HXX
#define CHART_NETCHARTTYPETEMPLATE_HXX

#include "ChartTypeTemplate.hxx"

#ifndef _COM_SUN_STAR_CHART2_STACKMODE_HPP_
#include <com/sun/star/chart2/StackMode.hpp>
#endif

namespace chart
{

class NetChartTypeTemplate : public ChartTypeTemplate
{
public:
    explicit NetChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & rServiceName,
        ::com::sun::star::chart2::StackMode eStackMode,
        bool bSymbols );
    virtual ~NetChartTypeTemplate();

    APPHELPER_XSERVICEINFO_DECL()

protected:
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
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType > getDefaultChartType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::chart2::StackMode getYStackMode() const;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XBoundedCoordinateSystem >
        createCoordinateSystem(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XBoundedCoordinateSystemContainer > & xCoordSysCnt );

private:
    ::com::sun::star::chart2::StackMode  m_eStackMode;
    bool                                         m_bHasSymbols;
};

} //  namespace chart

// CHART_NETCHARTTYPETEMPLATE_HXX
#endif
