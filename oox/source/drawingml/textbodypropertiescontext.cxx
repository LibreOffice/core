/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/drawingml/textbodypropertiescontext.hxx"

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include "oox/drawingml/textbodyproperties.hxx"
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


TextBodyPropertiesContext::TextBodyPropertiesContext( ContextHandler2Helper& rParent,
    const AttributeList& rAttribs, TextBodyProperties& rTextBodyProp )
: ContextHandler2( rParent )
, mrTextBodyProp( rTextBodyProp )
{
    
    sal_Int32 nWrappingType = rAttribs.getToken( XML_wrap, XML_square );
    mrTextBodyProp.maPropertyMap[ PROP_TextWordWrap ] <<= static_cast< sal_Bool >( nWrappingType == XML_square );

    
    OUString sValue;
    sal_Int32 aIns[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
    for( sal_Int32 i = 0; i < ( sal_Int32 )( sizeof( aIns ) / sizeof( sal_Int32 ) ); i++)
    {
        sValue = rAttribs.getString( aIns[i] ).get();
        if( !sValue.isEmpty() )
            mrTextBodyProp.moInsets[i] = GetCoordinate( sValue );
    }

    bool bAnchorCenter = rAttribs.getBool( XML_anchorCtr, false );
    if( rAttribs.hasAttribute( XML_anchorCtr ) ) {
        if( bAnchorCenter )
            mrTextBodyProp.maPropertyMap[ PROP_TextHorizontalAdjust ] <<=
                TextHorizontalAdjust_CENTER;
    }




  

    


    


    
    mrTextBodyProp.moRotation = rAttribs.getInteger( XML_rot );


    




    
    if( rAttribs.hasAttribute( XML_vert ) ) {
        mrTextBodyProp.moVert = rAttribs.getToken( XML_vert );
        bool bRtl = rAttribs.getBool( XML_rtl, false );
        sal_Int32 tVert = mrTextBodyProp.moVert.get( XML_horz );
        if( tVert == XML_vert || tVert == XML_eaVert || tVert == XML_vert270 || tVert == XML_mongolianVert )
            mrTextBodyProp.moRotation = -5400000*(tVert==XML_vert270?3:1);
        else
            mrTextBodyProp.maPropertyMap[ PROP_TextWritingMode ]
                <<= ( bRtl ? WritingMode_RL_TB : WritingMode_LR_TB );
    }

    
    if( rAttribs.hasAttribute( XML_anchor ) )
    {
        mrTextBodyProp.meVA = GetTextVerticalAdjust( rAttribs.getToken( XML_anchor, XML_t ) );
        mrTextBodyProp.maPropertyMap[ PROP_TextVerticalAdjust ] <<= mrTextBodyProp.meVA;
    }

    
    mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= false;
    mrTextBodyProp.maPropertyMap[ PROP_TextFitToSize ] <<= drawing::TextFitToSizeType_NONE;
}

ContextHandlerRef TextBodyPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& /*rAttribs*/)
{
    switch( aElementToken )
    {
            
            case A_TOKEN( prstTxWarp ):     
            case A_TOKEN( prot ):           
                break;

            
            case A_TOKEN( noAutofit ):
                mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= false;   
                break;
            case A_TOKEN( normAutofit ):    
                mrTextBodyProp.maPropertyMap[ PROP_TextFitToSize ] <<= TextFitToSizeType_AUTOFIT;
                mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= false;
                break;
            case A_TOKEN( spAutoFit ):
                mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= true;
                break;

            case A_TOKEN( scene3d ):        

            
            case A_TOKEN( sp3d ):           
            case A_TOKEN( flatTx ):         

                break;
    }

    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
