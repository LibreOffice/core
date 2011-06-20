/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedGapwidthProperty.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
namespace wrapper
{

const sal_Int32 DEFAULT_GAPWIDTH = 100;
const sal_Int32 DEFAULT_OVERLAP = 0;

//-----------------------------------------------------------------------------

WrappedBarPositionProperty_Base::WrappedBarPositionProperty_Base(
                  const ::rtl::OUString& rOuterName
                , const ::rtl::OUString& rInnerSequencePropertyName
                , sal_Int32 nDefaultValue
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedDefaultProperty( rOuterName, rtl::OUString(), uno::makeAny( nDefaultValue ) )
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
        throw lang::IllegalArgumentException( C2U("GapWidth and Overlap property require value of type sal_Int32"), 0, 0 );

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

//-----------------------------------------------------------------------------

WrappedGapwidthProperty::WrappedGapwidthProperty(
        ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedBarPositionProperty_Base( C2U("GapWidth"), C2U("GapwidthSequence"), DEFAULT_GAPWIDTH, spChart2ModelContact )
{
}
WrappedGapwidthProperty::~WrappedGapwidthProperty()
{
}

//-----------------------------------------------------------------------------

WrappedBarOverlapProperty::WrappedBarOverlapProperty(
        ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedBarPositionProperty_Base( C2U("Overlap"), C2U("OverlapSequence"), DEFAULT_OVERLAP, spChart2ModelContact )
{
}
WrappedBarOverlapProperty::~WrappedBarOverlapProperty()
{
}

} //  namespace wrapper
} //  namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
