/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tokenmap.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/token/propertylist.hxx"
#include "properties.hxx"

namespace oox {

namespace {

// include auto-generated property name lists
#include "propertywords.inc"

} // namespace

// ============================================================================

PropertyList::PropertyList()
{
    reserve( static_cast< size_t >( PROP_COUNT ) );
    for( sal_Int32 nIdx = 0; nIdx < PROP_COUNT; ++nIdx )
        push_back( ::rtl::OUString::createFromAscii( propertywordlist[ nIdx ] ) );
}

PropertyList::~PropertyList()
{
}

// ============================================================================

} // namespace oox

