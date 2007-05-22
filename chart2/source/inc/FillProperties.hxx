/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FillProperties.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:16:44 $
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
#ifndef CHART_FILLPROPERTIES_HXX
#define CHART_FILLPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements service FillProperties
class FillProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.FillProperties
        // com.sun.star.drawing.FillProperties
        PROP_FILL_STYLE = FAST_PROPERTY_ID_START_FILL_PROP
        , PROP_FILL_COLOR
        , PROP_FILL_TRANSPARENCE
        , PROP_FILL_TRANSPARENCE_GRADIENT_NAME
//        , PROP_FILL_TRANSPARENCE_GRADIENT //optional
        , PROP_FILL_GRADIENT_NAME
        , PROP_FILL_GRADIENT_STEPCOUNT
//        , PROP_FILL_GRADIENT   //optional
        , PROP_FILL_HATCH_NAME
//        , PROP_FILL_HATCH  //optional
        //------------------------
        // bitmap properties start
        , PROP_FILL_BITMAP_NAME
//        , PROP_FILL_BITMAP //optional
//        , PROP_FILL_BITMAP_URL //optional
        , PROP_FILL_BITMAP_OFFSETX
        , PROP_FILL_BITMAP_OFFSETY
        , PROP_FILL_BITMAP_POSITION_OFFSETX
        , PROP_FILL_BITMAP_POSITION_OFFSETY
        , PROP_FILL_BITMAP_RECTANGLEPOINT
        , PROP_FILL_BITMAP_LOGICALSIZE
        , PROP_FILL_BITMAP_SIZEX
        , PROP_FILL_BITMAP_SIZEY
        , PROP_FILL_BITMAP_MODE
        // bitmap properties end
        //------------------------
        , PROP_FILL_BACKGROUND
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

private:
    // not implemented
    FillProperties();
};

} //  namespace chart

// CHART_FILLPROPERTIES_HXX
#endif
