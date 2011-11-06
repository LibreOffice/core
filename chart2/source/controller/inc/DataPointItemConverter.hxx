/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART_DATAPOINTITEMCONVERTER_HXX
#define CHART_DATAPOINTITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "NumberFormatterWrapper.hxx"

#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

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
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
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
    bool                                m_bForbidPercentValue;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DATAPOINTITEMCONVERTER_HXX
#endif
