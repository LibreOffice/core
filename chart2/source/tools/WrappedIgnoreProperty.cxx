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

#include "WrappedIgnoreProperty.hxx"
#include "macros.hxx"
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;

namespace chart
{

WrappedIgnoreProperty::WrappedIgnoreProperty( const OUString& rOuterName, const Any& rDefaultValue )
                         : WrappedProperty( rOuterName, OUString() )
                         , m_aDefaultValue( rDefaultValue )
                         , m_aCurrentValue( rDefaultValue )
{
}
WrappedIgnoreProperty::~WrappedIgnoreProperty()
{
}

void WrappedIgnoreProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aCurrentValue = rOuterValue;
}

Any WrappedIgnoreProperty::getPropertyValue( const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aCurrentValue;
}

void WrappedIgnoreProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    m_aCurrentValue = m_aDefaultValue;
}

Any WrappedIgnoreProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aDefaultValue;
}

beans::PropertyState WrappedIgnoreProperty::getPropertyState( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    return ( m_aCurrentValue == m_aDefaultValue
             ? beans::PropertyState_DEFAULT_VALUE
             : beans::PropertyState_DIRECT_VALUE );
}

void WrappedIgnoreProperties::addIgnoreLineProperties( std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedIgnoreProperty( "LineStyle", uno::makeAny( drawing::LineStyle_SOLID ) ) );
    rList.push_back( new WrappedIgnoreProperty( "LineDashName", uno::makeAny( OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( "LineColor", uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "LineTransparence", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "LineWidth", uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "LineJoint", uno::makeAny( drawing::LineJoint_ROUND ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties( std::vector< WrappedProperty* >& rList )
{
    addIgnoreFillProperties_without_BitmapProperties( rList );
    addIgnoreFillProperties_only_BitmapProperties( rList );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_without_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedIgnoreProperty( "FillStyle", uno::makeAny( drawing::FillStyle_SOLID ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillColor", uno::makeAny( sal_Int32(-1) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillTransparence", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillTransparenceGradientName", uno::makeAny( OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( "FillTransparenceGradient", uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillGradientName", uno::makeAny( OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( "FillGradient", uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillHatchName", uno::makeAny( OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( "FillHatch", uno::makeAny( drawing::Hatch() ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBackground", uno::makeAny( sal_Bool(sal_False) ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_only_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
//     rList.push_back( new WrappedIgnoreProperty( "FillBitmapName", uno::makeAny( OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( "FillBitmap", uno::makeAny( uno::Reference< awt::XBitmap > (0) ) ) );
//    rList.push_back( new WrappedIgnoreProperty( "FillBitmapURL", uno::makeAny( OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapOffsetX", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapOffsetY", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapPositionOffsetX", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapPositionOffsetY", uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapRectanglePoint", uno::makeAny( drawing::RectanglePoint_LEFT_TOP ) ) );
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapLogicalSize", uno::makeAny( sal_Bool(sal_False) ) ) );//todo correct default?
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapSizeX", uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapSizeY", uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( "FillBitmapMode", uno::makeAny( drawing::BitmapMode_REPEAT ) ) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
