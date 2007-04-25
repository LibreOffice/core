/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:02:31 $
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
#ifndef SC_VBA_CHART_HXX
#define SC_VBA_CHART_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <org/openoffice/excel/XChart.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::excel::XChart > ChartImpl_BASE;

class ScVbaChart : public ChartImpl_BASE
{

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::chart::XChartDocument > m_xChartDoc;
public:
    ScVbaChart( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::chart::XChartDocument >& xChartDoc ) : m_xContext(xContext), m_xChartDoc( xChartDoc ) {}
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SeriesCollection(const css::uno::Any&) throw (css::uno::RuntimeException);

};

#endif //SC_VBA_WINDOW_HXX
