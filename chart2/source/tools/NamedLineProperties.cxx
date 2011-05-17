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

#include "NamedLineProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::rtl::OUString;

namespace chart
{

// @deprecated !!
void NamedLineProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    const uno::Type tCppuTypeString = ::getCppuType( reinterpret_cast< const OUString * >(0));

    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    if( bIncludeLineEnds )
    {
        rOutProperties.push_back(
            Property( C2U( "LineStartName" ),
                      PROP_LINE_START_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));

        rOutProperties.push_back(
            Property( C2U( "LineEndName" ),
                      PROP_LINE_END_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));
    }
}

void NamedLineProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap,
    bool bIncludeLineEnds /* = false */ )
{
}

OUString NamedLineProperties::GetPropertyNameForHandle( sal_Int32 nHandle )
{
    //will return e.g. "LineDashName" for PROP_LINE_DASH_NAME
    switch( nHandle )
    {
        case PROP_LINE_DASH_NAME:
            return C2U( "LineDashName" );
        case PROP_LINE_START_NAME:
        case PROP_LINE_END_NAME:
            break;
    }
    return OUString();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
