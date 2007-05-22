/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StockDataInterpreter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:53:00 $
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
#ifndef CHART_STOCKDATAINTERPRETER_HXX
#define CHART_STOCKDATAINTERPRETER_HXX

#ifndef CHART_DATAINTERPRETER_HXX
#include "DataInterpreter.hxx"
#endif

#include "StockChartTypeTemplate.hxx"

namespace chart
{

class StockDataInterpreter : public DataInterpreter
{
public:
    explicit StockDataInterpreter(
        StockChartTypeTemplate::StockVariant eVariant,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~StockDataInterpreter();

protected:
    // ____ XDataInterpreter ____
    virtual ::com::sun::star::chart2::InterpretedData SAL_CALL interpretDataSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >& aSeriesToReUse )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataCompatible(
        const ::com::sun::star::chart2::InterpretedData& aInterpretedData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::chart2::InterpretedData SAL_CALL reinterpretDataSeries(
        const ::com::sun::star::chart2::InterpretedData& aInterpretedData )
        throw (::com::sun::star::uno::RuntimeException);

private:
    StockChartTypeTemplate::StockVariant m_eStockVariant;

    StockChartTypeTemplate::StockVariant GetStockVariant() const;
};

} // namespace chart

// CHART_STOCKDATAINTERPRETER_HXX
#endif
