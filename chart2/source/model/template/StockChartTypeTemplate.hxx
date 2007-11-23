/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StockChartTypeTemplate.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:04:20 $
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
#ifndef CHART_STOCKCHARTTYPETEMPLATE_HXX
#define CHART_STOCKCHARTTYPETEMPLATE_HXX

#include "ChartTypeTemplate.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

namespace chart
{

class StockChartTypeTemplate :
        public MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    enum StockVariant
    {
        LOW_HI_CLOSE,
        OPEN_LOW_HI_CLOSE,
        VOL_LOW_HI_CLOSE,
        VOL_OPEN_LOW_HI_CLOSE
    };

    /** CTOR

        @param bJapaneseStyle
            If true, the candlesticks are drawn as solid white or black boxes
            depending on rising or falling stock-values.  Otherwise the
            open-value will be drawn as a small line at the left side of a
            straight vertical line, and the close-value on the right hand side.
     */
    explicit StockChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & rServiceName,
        StockVariant eVariant,
        bool bJapaneseStyle );
    virtual ~StockChartTypeTemplate();

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
    virtual sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram,
        sal_Bool bAdaptProperties )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > SAL_CALL
        getChartTypeForNewSeries( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType > >& aFormerlyUsedChartTypes )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataInterpreter > SAL_CALL getDataInterpreter()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL applyStyle(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetStyles(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

    // ChartTypeTemplate
    virtual sal_Int32 getAxisCountByDimension( sal_Int32 nDimension );

    // ____ ChartTypeTemplate ____
    virtual void createChartTypes(
            const ::com::sun::star::uno::Sequence<
                 ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries > > >& aSeriesSeq,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XChartType > > & aOldChartTypesSeq
            );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex );

private:
    // todo: deprecate this variable
    StockVariant  m_eStockVariant;
};

} //  namespace chart

// CHART_STOCKCHARTTYPETEMPLATE_HXX
#endif
