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

#include "drawingml/textbodypropertiescontext.hxx"

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include "drawingml/textbodyproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_TextBodyProperties
TextBodyPropertiesContext::TextBodyPropertiesContext( ContextHandler2Helper& rParent,
    const AttributeList& rAttribs, TextBodyProperties& rTextBodyProp )
: ContextHandler2( rParent )
, mrTextBodyProp( rTextBodyProp )
{
    // ST_TextWrappingType
    sal_Int32 nWrappingType = rAttribs.getToken( XML_wrap, XML_square );
    mrTextBodyProp.maPropertyMap.setProperty( PROP_TextWordWrap, nWrappingType == XML_square );

    // ST_Coordinate
    OUString sValue;
    sal_Int32 aIns[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
    for( sal_Int32 i = 0; i < ( sal_Int32 )( sizeof( aIns ) / sizeof( sal_Int32 ) ); i++)
    {
        sValue = rAttribs.getString( aIns[i] ).get();
        if( !sValue.isEmpty() )
            mrTextBodyProp.moInsets[i] = GetCoordinate( sValue );
    }

    mrTextBodyProp.mbAnchorCtr = rAttribs.getBool( XML_anchorCtr, false );
    if( mrTextBodyProp.mbAnchorCtr )
        mrTextBodyProp.maPropertyMap.setProperty( PROP_TextHorizontalAdjust, TextHorizontalAdjust_CENTER );

//   bool bCompatLineSpacing = rAttribs.getBool( XML_compatLnSpc, false );
//   bool bForceAA = rAttribs.getBool( XML_forceAA, false );
//   bool bFromWordArt = rAttribs.getBool( XML_fromWordArt, false );

  // ST_TextHorzOverflowType
//   sal_Int32 nHorzOverflow = rAttribs.getToken( XML_horzOverflow, XML_overflow );
    // ST_TextVertOverflowType
//   sal_Int32 nVertOverflow =  rAttribs.getToken( XML_vertOverflow, XML_overflow );

    // ST_TextColumnCount
//   sal_Int32 nNumCol = rAttribs.getInteger( XML_numCol, 1 );

    // ST_Angle
    mrTextBodyProp.moRotation = rAttribs.getInteger( XML_rot );

//   bool bRtlCol = rAttribs.getBool( XML_rtlCol, false );
    // ST_PositiveCoordinate
//   sal_Int32 nSpcCol = rAttribs.getInteger( XML_spcCol, 0 );
//   bool bSpcFirstLastPara = rAttribs.getBool( XML_spcFirstLastPara, 0 );
//   bool bUpRight = rAttribs.getBool( XML_upright, 0 );

    // ST_TextVerticalType
    if( rAttribs.hasAttribute( XML_vert ) ) {
        mrTextBodyProp.moVert = rAttribs.getToken( XML_vert );
        sal_Int32 tVert = mrTextBodyProp.moVert.get( XML_horz );
        if (tVert == XML_vert || tVert == XML_eaVert || tVert == XML_mongolianVert)
            mrTextBodyProp.moRotation = 5400000;
        else if (tVert == XML_vert270)
            mrTextBodyProp.moRotation = 5400000 * 3;
        else {
            bool bRtl = rAttribs.getBool( XML_rtl, false );
            mrTextBodyProp.maPropertyMap.setProperty( PROP_TextWritingMode,
                ( bRtl ? WritingMode_RL_TB : WritingMode_LR_TB ));
        }
    }

    // ST_TextAnchoringType
    if( rAttribs.hasAttribute( XML_anchor ) )
    {
        mrTextBodyProp.meVA = GetTextVerticalAdjust( rAttribs.getToken( XML_anchor, XML_t ) );
        mrTextBodyProp.maPropertyMap.setProperty( PROP_TextVerticalAdjust, mrTextBodyProp.meVA);
    }

    // Push defaults
    mrTextBodyProp.maPropertyMap.setProperty( PROP_TextAutoGrowHeight, false);
    mrTextBodyProp.maPropertyMap.setProperty( PROP_TextFitToSize, drawing::TextFitToSizeType_NONE);
}

ContextHandlerRef TextBodyPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& /*rAttribs*/)
{
    switch( aElementToken )
    {
            // Sequence
            case A_TOKEN( prstTxWarp ):     // CT_PresetTextShape
            case A_TOKEN( prot ):           // CT_TextProtectionProperty
                break;

            // EG_TextAutofit
            case A_TOKEN( noAutofit ):
                mrTextBodyProp.maPropertyMap.setProperty( PROP_TextAutoGrowHeight, false);   // CT_TextNoAutofit
                break;
            case A_TOKEN( normAutofit ):    // CT_TextNormalAutofit
                mrTextBodyProp.maPropertyMap.setProperty( PROP_TextFitToSize, TextFitToSizeType_AUTOFIT);
                mrTextBodyProp.maPropertyMap.setProperty( PROP_TextAutoGrowHeight, false);
                break;
            case A_TOKEN( spAutoFit ):
                {
                    const sal_Int32 tVert = mrTextBodyProp.moVert.get( XML_horz );
                    if( tVert != XML_vert && tVert != XML_eaVert && tVert != XML_vert270 && tVert != XML_mongolianVert )
                        mrTextBodyProp.maPropertyMap.setProperty( PROP_TextAutoGrowHeight, true);
                }
                break;

            case A_TOKEN( scene3d ):        // CT_Scene3D

            // EG_Text3D
            case A_TOKEN( sp3d ):           // CT_Shape3D
            case A_TOKEN( flatTx ):         // CT_FlatText

                break;
    }

    return nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
