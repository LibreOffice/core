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

#include "LinePropertiesHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void LinePropertiesHelper::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // Line Properties see service drawing::LineProperties
    rOutProperties.push_back(
        Property( "LineStyle",
                  PROP_LINE_STYLE,
                  cppu::UnoType<drawing::LineStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

     rOutProperties.push_back(
         Property( "LineDash",
                   PROP_LINE_DASH,
                   cppu::UnoType<drawing::LineDash>::get(),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));

//not in service description
    rOutProperties.push_back(
        Property( "LineDashName",
                  PROP_LINE_DASH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "LineColor",
                  PROP_LINE_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LineTransparence",
                  PROP_LINE_TRANSPARENCE,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LineWidth",
                  PROP_LINE_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LineJoint",
                  PROP_LINE_JOINT,
                  cppu::UnoType<drawing::LineJoint>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void LinePropertiesHelper::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LINE_STYLE, drawing::LineStyle_SOLID );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINE_WIDTH, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINE_COLOR, 0x000000 );  // black
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_LINE_TRANSPARENCE, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LINE_JOINT, drawing::LineJoint_ROUND );
}

bool LinePropertiesHelper::IsLineVisible( const css::uno::Reference<
        css::beans::XPropertySet >& xLineProperties )
{
    bool bRet = false;
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( "LineStyle" ) >>= aLineStyle;
            if( aLineStyle != drawing::LineStyle_NONE )
            {
                sal_Int16 nLineTransparence=0;
                xLineProperties->getPropertyValue( "LineTransparence" ) >>= nLineTransparence;
                if(100!=nLineTransparence)
                {
                    bRet = true;
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return bRet;
}

void LinePropertiesHelper::SetLineVisible( const css::uno::Reference<
    css::beans::XPropertySet >& xLineProperties )
{
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( "LineStyle" ) >>= aLineStyle;
            if( aLineStyle == drawing::LineStyle_NONE )
                xLineProperties->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ) );

            sal_Int16 nLineTransparence=0;
            xLineProperties->getPropertyValue( "LineTransparence" ) >>= nLineTransparence;
            if(100==nLineTransparence)
                xLineProperties->setPropertyValue( "LineTransparence", uno::Any( sal_Int16(0) ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void LinePropertiesHelper::SetLineInvisible( const css::uno::Reference<
    css::beans::XPropertySet >& xLineProperties )
{
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( "LineStyle" ) >>= aLineStyle;
            if( aLineStyle != drawing::LineStyle_NONE )
                xLineProperties->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void LinePropertiesHelper::SetLineColor( const css::uno::Reference<
     css::beans::XPropertySet >& xLineProperties, sal_Int32 nColor  )
{
    try
    {
        if( xLineProperties.is() )
        {
            xLineProperties->setPropertyValue( "LineColor", uno::Any( nColor ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}


} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
