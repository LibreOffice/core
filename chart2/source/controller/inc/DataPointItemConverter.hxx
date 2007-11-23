/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataPointItemConverter.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:50:30 $
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
#ifndef CHART_DATAPOINTITEMCONVERTER_HXX
#define CHART_DATAPOINTITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "chartview/NumberFormatterWrapper.hxx"

#include <com/sun/star/chart2/XDataSeries.hpp>

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <memory>
#include <vector>

class SdrModel;

namespace chart
{
namespace wrapper
{

class DataPointItemConverter :
        public ::comphelper::ItemConverter
{
public:
    DataPointItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        NumberFormatterWrapper * pNumFormatter,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        GraphicPropertyItemConverter::eGraphicObjectType eMapTo =
            GraphicPropertyItemConverter::FILLED_DATA_POINT,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >(),
        bool bDataSeries = false,
        bool bUseSpecialFillColor = false,
        sal_Int32 nSpecialFillColor = 0,
        bool bOverwriteLabelsForAttributedDataPointsAlso=false,
        sal_Int32 nNumberFormat=0,
        sal_Int32 nPercentNumberFormat=0);

    virtual ~DataPointItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::std::vector< ItemConverter * >    m_aConverters;
    NumberFormatterWrapper *            m_pNumberFormatterWrapper;
    bool                                m_bDataSeries;
    bool                                m_bOverwriteLabelsForAttributedDataPointsAlso;
    bool                                m_bColorPerPoint;
    bool                                m_bUseSpecialFillColor;
    sal_Int32                           m_nSpecialFillColor;
    sal_Int32                           m_nNumberFormat;
    sal_Int32                           m_nPercentNumberFormat;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_aAvailableLabelPlacements;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DATAPOINTITEMCONVERTER_HXX
#endif
