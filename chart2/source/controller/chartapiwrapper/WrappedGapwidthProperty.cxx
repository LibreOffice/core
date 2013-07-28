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

#include "WrappedGapwidthProperty.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace chart
{
namespace wrapper
{

const sal_Int32 DEFAULT_GAPWIDTH = 100;
const sal_Int32 DEFAULT_OVERLAP = 0;

WrappedBarPositionProperty_Base::WrappedBarPositionProperty_Base(
                  const OUString& rOuterName
                , const OUString& rInnerSequencePropertyName
                , sal_Int32 nDefaultValue
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedDefaultProperty( rOuterName, OUString(), uno::makeAny( nDefaultValue ) )
            , m_nDimensionIndex(0)
            , m_nAxisIndex(0)
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_nDefaultValue( nDefaultValue )
            , m_InnerSequencePropertyName( rInnerSequencePropertyName )
{
}

void WrappedBarPositionProperty_Base::setDimensionAndAxisIndex( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    m_nDimensionIndex = nDimensionIndex;
    m_nAxisIndex = nAxisIndex;
}

WrappedBarPositionProperty_Base::~WrappedBarPositionProperty_Base()
{
}

void WrappedBarPositionProperty_Base::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nNewValue = 0;
    if( ! (rOuterValue >>= nNewValue) )
        throw lang::IllegalArgumentException( "GapWidth and Overlap property require value of type sal_Int32", 0, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    if( m_nDimensionIndex==1 )
    {
        Sequence< Reference< chart2::XChartType > > aChartTypeList( DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
        for( sal_Int32 nN = 0; nN < aChartTypeList.getLength(); nN++ )
        {
            try
            {
                Reference< beans::XPropertySet > xProp( aChartTypeList[nN], uno::UNO_QUERY );
                if( xProp.is() )
                {
                    Sequence< sal_Int32 > aBarPositionSequence;
                    xProp->getPropertyValue( m_InnerSequencePropertyName ) >>= aBarPositionSequence;

                    long nOldLength = aBarPositionSequence.getLength();
                    if( nOldLength <= m_nAxisIndex  )
                    {
                        aBarPositionSequence.realloc( m_nAxisIndex+1 );
                        for( sal_Int32 i=nOldLength; i<m_nAxisIndex; i++ )
                        {
                            aBarPositionSequence[i] = m_nDefaultValue;
                        }
                    }
                    aBarPositionSequence[m_nAxisIndex] = nNewValue;

                    xProp->setPropertyValue( m_InnerSequencePropertyName, uno::makeAny( aBarPositionSequence ) );
                }
            }
            catch( uno::Exception& e )
            {
                //the above properties are not supported by all charttypes (only by column and bar)
                //in that cases this exception is ok
                e.Context.is();//to have debug information without compilation warnings
            }
        }
    }
}

Any WrappedBarPositionProperty_Base::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        bool bInnerValueDetected = false;
        sal_Int32 nInnerValue = m_nDefaultValue;

        if( m_nDimensionIndex==1 )
        {
            Sequence< Reference< chart2::XChartType > > aChartTypeList( DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
            for( sal_Int32 nN = 0; nN < aChartTypeList.getLength() && !bInnerValueDetected; nN++ )
            {
                try
                {
                    Reference< beans::XPropertySet > xProp( aChartTypeList[nN], uno::UNO_QUERY );
                    if( xProp.is() )
                    {
                        Sequence< sal_Int32 > aBarPositionSequence;
                        xProp->getPropertyValue( m_InnerSequencePropertyName ) >>= aBarPositionSequence;
                        if( m_nAxisIndex < aBarPositionSequence.getLength() )
                        {
                            nInnerValue = aBarPositionSequence[m_nAxisIndex];
                            bInnerValueDetected = true;
                        }
                    }
                }
                catch( uno::Exception& e )
                {
                    //the above properties are not supported by all charttypes (only by column and bar)
                    //in that cases this exception is ok
                    e.Context.is();//to have debug information without compilation warnings
                }
            }
        }
        if( bInnerValueDetected )
        {
            m_aOuterValue <<= nInnerValue;
        }
    }
    return m_aOuterValue;
}

WrappedGapwidthProperty::WrappedGapwidthProperty(
        ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedBarPositionProperty_Base( "GapWidth", "GapwidthSequence", DEFAULT_GAPWIDTH, spChart2ModelContact )
{
}
WrappedGapwidthProperty::~WrappedGapwidthProperty()
{
}

WrappedBarOverlapProperty::WrappedBarOverlapProperty(
        ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedBarPositionProperty_Base( "Overlap", "OverlapSequence", DEFAULT_OVERLAP, spChart2ModelContact )
{
}
WrappedBarOverlapProperty::~WrappedBarOverlapProperty()
{
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
