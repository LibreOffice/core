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

#include <WrappedIgnoreProperty.hxx>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
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
{
    m_aCurrentValue = rOuterValue;
}

Any WrappedIgnoreProperty::getPropertyValue( const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
{
    return m_aCurrentValue;
}

void WrappedIgnoreProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
{
    m_aCurrentValue = m_aDefaultValue;
}

Any WrappedIgnoreProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
{
    return m_aDefaultValue;
}

beans::PropertyState WrappedIgnoreProperty::getPropertyState( const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
{
    return ( m_aCurrentValue == m_aDefaultValue
             ? beans::PropertyState_DEFAULT_VALUE
             : beans::PropertyState_DIRECT_VALUE );
}

void WrappedIgnoreProperties::addIgnoreLineProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList )
{
    rList.emplace_back( new WrappedIgnoreProperty( u"LineStyle"_ustr, uno::Any( drawing::LineStyle_SOLID ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"LineDashName"_ustr, uno::Any( OUString() ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"LineColor"_ustr, uno::Any( sal_Int32(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"LineTransparence"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"LineWidth"_ustr, uno::Any( sal_Int32(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"LineJoint"_ustr, uno::Any( drawing::LineJoint_ROUND ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList )
{
    addIgnoreFillProperties_without_BitmapProperties( rList );
    addIgnoreFillProperties_only_BitmapProperties( rList );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_without_BitmapProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList )
{
    rList.emplace_back( new WrappedIgnoreProperty( u"FillStyle"_ustr, uno::Any( drawing::FillStyle_SOLID ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillColor"_ustr, uno::Any( sal_Int32(-1) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillTransparence"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillTransparenceGradientName"_ustr, uno::Any( OUString() ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillGradientName"_ustr, uno::Any( OUString() ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillHatchName"_ustr, uno::Any( OUString() ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBackground"_ustr, uno::Any( false ) ) );
}

void WrappedIgnoreProperties::addIgnoreFillProperties_only_BitmapProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList )
{
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapOffsetX"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapOffsetY"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapPositionOffsetX"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapPositionOffsetY"_ustr, uno::Any( sal_Int16(0) ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapRectanglePoint"_ustr, uno::Any( drawing::RectanglePoint_LEFT_TOP ) ) );
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapLogicalSize"_ustr, uno::Any( false ) ) );//todo correct default?
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapSizeX"_ustr, uno::Any( sal_Int32(10) ) ) );//todo which default?
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapSizeY"_ustr, uno::Any( sal_Int32(10) ) ) );//todo which default?
    rList.emplace_back( new WrappedIgnoreProperty( u"FillBitmapMode"_ustr, uno::Any( drawing::BitmapMode_REPEAT ) ) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
