/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LineProperties.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:18:46 $
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
#ifndef CHART_LINEPROPERTIES_HXX
#define CHART_LINEPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements service LineProperties
class LineProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.LineProperties
        PROP_LINE_STYLE = FAST_PROPERTY_ID_START_LINE_PROP,
        PROP_LINE_DASH,
        PROP_LINE_DASH_NAME, //not in service description
        PROP_LINE_COLOR,
        PROP_LINE_TRANSPARENCE,
        PROP_LINE_WIDTH,
        PROP_LINE_JOINT
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    static bool IsLineVisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );
    static void SetLineVisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );
    static void SetLineInvisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );

private:
    // not implemented
    LineProperties();
};

} //  namespace chart

// CHART_LINEPROPERTIES_HXX
#endif
