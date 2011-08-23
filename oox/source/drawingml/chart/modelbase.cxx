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

#include "oox/drawingml/chart/modelbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

NumberFormat::NumberFormat() :
    mbSourceLinked( true )
{
}

void NumberFormat::setAttributes( const AttributeList& rAttribs )
{
    maFormatCode = rAttribs.getString( XML_formatCode, OUString() );
    // default is 'false', not 'true' as specified
    mbSourceLinked = rAttribs.getBool( XML_sourceLinked, false );
}

// ============================================================================

LayoutModel::LayoutModel() :
    mfX( 0.0 ),
    mfY( 0.0 ),
    mfW( 0.0 ),
    mfH( 0.0 ),
    mnXMode( XML_factor ),
    mnYMode( XML_factor ),
    mnWMode( XML_factor ),
    mnHMode( XML_factor ),
    mnTarget( XML_outer ),
    mbAutoLayout( true )
{
}

LayoutModel::~LayoutModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
