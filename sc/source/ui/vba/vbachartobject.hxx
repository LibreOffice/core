/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachartobject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:02:54 $
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
#ifndef SC_VBA_CHARTOBJECT_HXX
#define SC_VBA_CHARTOBJECT_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <org/openoffice/excel/XChartObject.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::excel::XChartObject > ChartObjectImpl_BASE;

class ScVbaChartObject : public ChartObjectImpl_BASE
{

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::table::XTableChart  > m_xTableChart;
public:
    ScVbaChartObject( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XTableChart >& xTableChart ) : m_xContext(xContext), m_xTableChart( xTableChart ) {}
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XChart > SAL_CALL getChart() throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL test(  ) throw (css::uno::RuntimeException);
};

#endif //SC_VBA_WINDOW_HXX
