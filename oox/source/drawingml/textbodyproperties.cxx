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

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textbodyproperties.hxx"
#include "oox/token/tokens.hxx"

using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;

namespace oox {
namespace drawingml {

// ============================================================================

TextBodyProperties::TextBodyProperties():
    meVA( TextVerticalAdjust_TOP )
{
}

/* For Legacy purposes: TODO: Check if it is required at all! */
void TextBodyProperties::pushVertSimulation()
{
    sal_Int32 tVert = moVert.get( XML_horz );
    if( tVert == XML_vert || tVert == XML_eaVert || tVert == XML_vert270 || tVert == XML_mongolianVert ) {
        // #160799# fake different vertical text modes by top-bottom writing mode
        maPropertyMap[ PROP_TextWritingMode ] <<= WritingMode_TB_RL;

        // workaround for TB_LR as using WritingMode2 doesn't work
        if( meVA != TextVerticalAdjust_CENTER )
            maPropertyMap[ PROP_TextHorizontalAdjust ] <<=
                                (tVert == XML_vert270) ? TextHorizontalAdjust_RIGHT : TextHorizontalAdjust_LEFT;
        if( tVert == XML_vert270 )
            maPropertyMap[ PROP_TextVerticalAdjust ] <<= TextVerticalAdjust_BOTTOM;
        if( ( tVert == XML_vert && meVA == TextVerticalAdjust_TOP ) ||
            ( tVert == XML_vert270 && meVA == TextVerticalAdjust_BOTTOM ) )
            maPropertyMap[ PROP_TextHorizontalAdjust ] <<= TextHorizontalAdjust_RIGHT;
        else if( meVA == TextVerticalAdjust_CENTER )
            maPropertyMap[ PROP_TextHorizontalAdjust ] <<= TextHorizontalAdjust_CENTER;
    }
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
