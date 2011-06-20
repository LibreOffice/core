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
using ::rtl::OUString;

//.............................................................................
namespace chart
{
//.............................................................................


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
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineDashName" ), uno::makeAny( rtl::OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineColor" ), uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineTransparence" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineWidth" ), uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineJoint" ), uno::makeAny( drawing::LineJoint_ROUND ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties( std::vector< WrappedProperty* >& rList )
{
    addIgnoreFillProperties_without_BitmapProperties( rList );
    addIgnoreFillProperties_only_BitmapProperties( rList );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_without_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_SOLID ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillColor" ), uno::makeAny( sal_Int32(-1) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparence" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparenceGradientName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparenceGradient" ), uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillGradientName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillGradient" ), uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillHatchName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillHatch" ), uno::makeAny( drawing::Hatch() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBackground" ), uno::makeAny( sal_Bool(sal_False) ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_only_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
//     rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmap" ), uno::makeAny( uno::Reference< awt::XBitmap > (0) ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapURL" ), uno::makeAny( ::rtl::OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapOffsetX" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapOffsetY" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapPositionOffsetX" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapPositionOffsetY" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapRectanglePoint" ), uno::makeAny( drawing::RectanglePoint_LEFT_TOP ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapLogicalSize" ), uno::makeAny( sal_Bool(sal_False) ) ) );//todo correct default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapSizeX" ), uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapSizeY" ), uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapMode" ), uno::makeAny( drawing::BitmapMode_REPEAT ) ) );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
