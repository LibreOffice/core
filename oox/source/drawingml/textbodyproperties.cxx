/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* Push adjusted values, taking into consideration Shape Rotation */
void TextBodyProperties::pushRotationAdjustments( sal_Int32 nRotation )
{
    sal_Int32 nOff      = 0;
    sal_Int32 aProps[]  = { PROP_TextLeftDistance, PROP_TextUpperDistance, PROP_TextRightDistance, PROP_TextLowerDistance };
    sal_Int32 n         = ( sal_Int32 )( sizeof( aProps ) / sizeof( sal_Int32 ) );

    switch( nRotation ) // TODO: What happens for part rotations ?
    {
        case (90*1*60000): nOff = 1; break;
        case (90*2*60000): nOff = 2; break;
        case (90*3*60000): nOff = 3; break;
        default: break;
    }

    for( sal_Int32 i = 0; i < n; i++ )
    {
        sal_Int32 nVal = 0;

        // Hack for n#760986
        // TODO: Preferred method would be to have a textbox on top
        // of the shape and the place it according to the (off,ext)
        if( nOff == 0 && moTextOffX ) nVal = *moTextOffX;
        if( nOff == 1 && moTextOffY ) nVal = *moTextOffY;
        if( nVal < 0 ) nVal = 0;

        if( moInsets[i] )
            maPropertyMap[ aProps[ nOff ] ] <<= static_cast< sal_Int32 >( *moInsets[i] + nVal );
        else if( nVal )
            maPropertyMap[ aProps[ nOff ] ] <<= static_cast< sal_Int32 >( nVal );

        nOff = (nOff+1) % n;
    }
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
