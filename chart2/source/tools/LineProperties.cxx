/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LineProperties.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:08:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "LineProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void LineProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // Line Properties see service drawing::LineProperties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineStyle" ),
                  PROP_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

     rOutProperties.push_back(
         Property( C2U( "LineDash" ),
                   PROP_LINE_DASH,
                   ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));

//not in service description
    rOutProperties.push_back(
        Property( C2U( "LineDashName" ),
                  PROP_LINE_DASH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "LineColor" ),
                  PROP_LINE_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LineTransparence" ),
                  PROP_LINE_TRANSPARENCE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  PROP_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LineJoint" ),
                  PROP_LINE_JOINT,
                  ::getCppuType( reinterpret_cast< const drawing::LineJoint * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void LineProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LINE_STYLE, drawing::LineStyle_SOLID );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINE_WIDTH, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINE_COLOR, 0x000000 );  // black
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_LINE_TRANSPARENCE, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LINE_JOINT, drawing::LineJoint_NONE );
}

//static
bool LineProperties::IsLineVisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties )
{
    bool bRet = false;
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( C2U( "LineStyle" ) ) >>= aLineStyle;
            if( aLineStyle != drawing::LineStyle_NONE )
            {
                sal_Int16 nLineTransparence=0;
                xLineProperties->getPropertyValue( C2U( "LineTransparence" ) ) >>= nLineTransparence;
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

//static
void LineProperties::SetLineVisible( const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >& xLineProperties )
{
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( C2U( "LineStyle" ) ) >>= aLineStyle;
            if( aLineStyle == drawing::LineStyle_NONE )
                xLineProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) );

            sal_Int16 nLineTransparence=0;
            xLineProperties->getPropertyValue( C2U( "LineTransparence" ) ) >>= nLineTransparence;
            if(100==nLineTransparence)
                xLineProperties->setPropertyValue( C2U( "LineTransparence" ), uno::makeAny( sal_Int16(0) ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//static
void LineProperties::SetLineInvisible( const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >& xLineProperties )
{
    try
    {
        if( xLineProperties.is() )
        {
            drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
            xLineProperties->getPropertyValue( C2U( "LineStyle" ) ) >>= aLineStyle;
            if( aLineStyle != drawing::LineStyle_NONE )
                xLineProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} //  namespace chart
