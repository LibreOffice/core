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

#include "NamedFillProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/chart2/FillBitmap.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
