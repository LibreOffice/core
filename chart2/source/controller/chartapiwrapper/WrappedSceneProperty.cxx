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
            aMatrix.rotate( aRotation.getX(), aRotation.getY(), aRotation.getZ() );
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
            aMatrix.rotate( aRotation.getX(), aRotation.getY(), aRotation.getZ() );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
