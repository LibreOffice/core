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

#include "oox/drawingml/textbodyproperties.hxx"
#include <com/sun/star/text/WritingMode.hpp>
#include "properties.hxx"
#include "tokens.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

TextBodyProperties::TextBodyProperties()
{
}

void TextBodyProperties::pushToPropMap( PropertyMap& rPropMap ) const
{
    rPropMap.insert( maPropertyMap.begin(), maPropertyMap.end() );

    // #160799# fake different vertical text modes by top-bottom writing mode
    if( moVert.get( XML_horz ) != XML_horz )
        rPropMap[ PROP_TextWritingMode ] <<= ::com::sun::star::text::WritingMode_TB_RL;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
