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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedSceneProperty.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "BaseGFXHelper.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{

void WrappedSceneProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedD3DTransformMatrixProperty( spChart2ModelContact ) );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DTransformMatrixProperty::WrappedD3DTransformMatrixProperty(
            ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("D3DTransformMatrix"),C2U("D3DTransformMatrix"))
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedD3DTransformMatrixProperty::~WrappedD3DTransformMatrixProperty()
{
}

void WrappedD3DTransformMatrixProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( DiagramHelper::isPieOrDonutChart( m_spChart2ModelContact->getChart2Diagram() ) )
    {
        drawing::HomogenMatrix aHM;
        if( rOuterValue >>= aHM )
        {
            ::basegfx::B3DTuple aRotation( BaseGFXHelper::GetRotationFromMatrix(
                BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aHM ) ) );

            ::basegfx::B3DHomMatrix aMatrix;
            aMatrix.rotate( aRotation );
            ::basegfx::B3DHomMatrix aObjectMatrix;
            ::basegfx::B3DHomMatrix aNewMatrix = aMatrix*aObjectMatrix;

            aHM = BaseGFXHelper::B3DHomMatrixToHomogenMatrix(aNewMatrix);

            WrappedProperty::setPropertyValue( uno::makeAny(aHM), xInnerPropertySet );
            return;
        }
    }

    WrappedProperty::setPropertyValue( rOuterValue, xInnerPropertySet );
}

Any WrappedD3DTransformMatrixProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( DiagramHelper::isPieOrDonutChart( m_spChart2ModelContact->getChart2Diagram() ) )
    {
        uno::Any aAMatrix( WrappedProperty::getPropertyValue( xInnerPropertySet ) );
        drawing::HomogenMatrix aHM;
        if( aAMatrix >>= aHM )
        {
            ::basegfx::B3DTuple aRotation( BaseGFXHelper::GetRotationFromMatrix(
                BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aHM ) ) );

            ::basegfx::B3DHomMatrix aMatrix;
            aMatrix.rotate( aRotation );
            ::basegfx::B3DHomMatrix aObjectMatrix;
            ::basegfx::B3DHomMatrix aNewMatrix = aMatrix*aObjectMatrix;

            aHM = BaseGFXHelper::B3DHomMatrixToHomogenMatrix(aNewMatrix);

            return uno::makeAny(aHM);
        }
    }

    return WrappedProperty::getPropertyValue( xInnerPropertySet );
}

Any WrappedD3DTransformMatrixProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return WrappedProperty::getPropertyDefault( xInnerPropertyState );
}

} //namespace wrapper
} //namespace chart
//.............................................................................
