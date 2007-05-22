/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedFillProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:02:22 $
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

#include "NamedFillProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_FILLBITMAP_HPP_
#include <com/sun/star/chart2/FillBitmap.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::rtl::OUString;

namespace chart
{

void NamedFillProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    const uno::Type tCppuTypeString = ::getCppuType( reinterpret_cast< const OUString * >(0));

    // Fill Properties
    // ---------------

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillTransparenceGradient" ),
                  PROP_FILL_TRANSPARENCE_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillGradient" ),
                  PROP_FILL_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillHatch" ),
                  PROP_FILL_HATCH,
                  ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillBitmapURL" ),
                  PROP_FILL_BITMAP,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void NamedFillProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
}

// static
OUString NamedFillProperties::GetPropertyNameForHandle( sal_Int32 nHandle )
{
    //will return e.g. "FillGradientName" for PROP_FILL_GRADIENT_NAME
    switch( nHandle )
    {
        case PROP_FILL_GRADIENT_NAME:
            return C2U( "FillGradientName" );
        case PROP_FILL_HATCH_NAME:
            return C2U( "FillHatchName" );
        case PROP_FILL_BITMAP_NAME:
            return C2U( "FillBitmapName" );
        case PROP_FILL_TRANSPARENCY_GRADIENT_NAME:
            return C2U( "FillTransparenceGradientName" );
    }
    return OUString();
}

} //  namespace chart
