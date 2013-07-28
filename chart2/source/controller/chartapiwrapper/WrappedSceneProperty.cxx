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

#include "WrappedSceneProperty.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "BaseGFXHelper.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace wrapper
{

void WrappedSceneProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedD3DTransformMatrixProperty( spChart2ModelContact ) );
}

WrappedD3DTransformMatrixProperty::WrappedD3DTransformMatrixProperty(
            ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty("D3DTransformMatrix","D3DTransformMatrix")
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
