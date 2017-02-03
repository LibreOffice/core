/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSeriesOrDiagramProperty.hxx"
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include <unonames.hxx>

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedDataCaptionProperty : public WrappedSeriesOrDiagramProperty< sal_Int32 >
{
public:
    virtual sal_Int32 getValueFromSeries( const css::uno::Reference< css::beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const css::uno::Reference< css::beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& aNewValue ) const override;

    explicit WrappedDataCaptionProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                        tSeriesOrDiagramPropertyType ePropertyType );
};

namespace
{
enum
{
    //data caption properties
    PROP_CHART_DATAPOINT_DATA_CAPTION = FAST_PROPERTY_ID_START_CHART_DATACAPTION_PROP
};

sal_Int32 lcl_LabelToCaption( const chart2::DataPointLabel& rLabel )
{
    sal_Int32 nCaption=0;

    if( rLabel.ShowNumber )
        nCaption |= css::chart::ChartDataCaption::VALUE;
    if( rLabel.ShowNumberInPercent )
        nCaption |= css::chart::ChartDataCaption::PERCENT;
    if( rLabel.ShowCategoryName )
        nCaption |= css::chart::ChartDataCaption::TEXT;
    if( rLabel.ShowLegendSymbol )
        nCaption |= css::chart::ChartDataCaption::SYMBOL;

    return nCaption;
}

chart2::DataPointLabel lcl_CaptionToLabel( sal_Int32 nCaption )
{
    chart2::DataPointLabel aLabel(false,false,false,false);

    if( nCaption & css::chart::ChartDataCaption::VALUE )
        aLabel.ShowNumber = true;
    if( nCaption & css::chart::ChartDataCaption::PERCENT )
        aLabel.ShowNumberInPercent = true;
    if( nCaption & css::chart::ChartDataCaption::TEXT )
        aLabel.ShowCategoryName = true;
    if( nCaption & css::chart::ChartDataCaption::SYMBOL )
        aLabel.ShowLegendSymbol = true;

    return aLabel;
}

void lcl_addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact
                                    , tSeriesOrDiagramPropertyType ePropertyType )
{
    //if !spChart2ModelContact.get() is then the created properties do belong to a single series or single datapoint
    //otherwise they do belong to the whole diagram

    rList.push_back( new WrappedDataCaptionProperty( spChart2ModelContact, ePropertyType ) );
}

}//anonymous namespace

void WrappedDataCaptionProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "DataCaption",
                  PROP_CHART_DATAPOINT_DATA_CAPTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void WrappedDataCaptionProperties::addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DATA_SERIES );
}

void WrappedDataCaptionProperties::addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DIAGRAM );
}

WrappedDataCaptionProperty::WrappedDataCaptionProperty(
      const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact
    , tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< sal_Int32 >( "DataCaption"
            , uno::Any( sal_Int32(0) ), spChart2ModelContact, ePropertyType )
{
}

sal_Int32 WrappedDataCaptionProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Int32 aRet = 0;
    m_aDefaultValue >>= aRet;
    chart2::DataPointLabel aLabel;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel ) )
        aRet = lcl_LabelToCaption( aLabel );
    return aRet;
}

void WrappedDataCaptionProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& nCaption ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::DataPointLabel aLabel = lcl_CaptionToLabel( nCaption );
    xSeriesPropertySet->setPropertyValue( CHART_UNONAME_LABEL, uno::Any( aLabel ) );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
