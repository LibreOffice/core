/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textbodypropertiescontext.cxx,v $
 * $Revision: 1.8 $
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

#include "oox/drawingml/textbodypropertiescontext.hxx"

#include <com/sun/star/text/ControlCharacter.hpp>
#include "oox/drawingml/textbodyproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextBodyProperties
TextBodyPropertiesContext::TextBodyPropertiesContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& xAttributes, TextBodyProperties& rTextBodyProp )
: ContextHandler( rParent )
, mrTextBodyProp( rTextBodyProp )
{
    AttributeList aAttribs( xAttributes );

    // ST_TextWrappingType
    sal_Int32 nWrappingType = aAttribs.getToken( XML_wrap, XML_square );
    mrTextBodyProp.maPropertyMap[ PROP_TextWordWrap ] <<= (nWrappingType == XML_square);

    // ST_Coordinate
    OUString sValue;
    sValue = xAttributes->getOptionalValue( XML_lIns );
    sal_Int32 nLeftInset = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrTextBodyProp.maPropertyMap[ PROP_TextLeftDistance ]  <<= static_cast< sal_Int32 >( nLeftInset );

    sValue = xAttributes->getOptionalValue( XML_tIns );
    sal_Int32 nTopInset  = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrTextBodyProp.maPropertyMap[ PROP_TextUpperDistance ] <<= static_cast< sal_Int32 >( nTopInset );

    sValue = xAttributes->getOptionalValue( XML_rIns );
    sal_Int32 nRightInset  = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrTextBodyProp.maPropertyMap[ PROP_TextRightDistance ] <<= static_cast< sal_Int32 >( nRightInset );

    sValue = xAttributes->getOptionalValue( XML_bIns );
    sal_Int32 nBottonInset = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 45720 / 360 );
    mrTextBodyProp.maPropertyMap[ PROP_TextLowerDistance ] <<= static_cast< sal_Int32 >( nBottonInset );


    // ST_TextAnchoringType
//   sal_Int32 nAnchoringType = xAttributes->getOptionalValueToken( XML_anchor, XML_t );

//   bool bAnchorCenter = aAttribs.getBool( XML_anchorCtr, false );

//   bool bCompatLineSpacing = aAttribs.getBool( XML_compatLnSpc, false );
//   bool bForceAA = aAttribs.getBool( XML_forceAA, false );
//   bool bFromWordArt = aAttribs.getBool( XML_fromWordArt, false );

  // ST_TextHorzOverflowType
//   sal_Int32 nHorzOverflow = xAttributes->getOptionalValueToken( XML_horzOverflow, XML_overflow );
    // ST_TextVertOverflowType
//   sal_Int32 nVertOverflow =  xAttributes->getOptionalValueToken( XML_vertOverflow, XML_overflow );

    // ST_TextColumnCount
//   sal_Int32 nNumCol = aAttribs.getInteger( XML_numCol, 1 );

    // ST_Angle
    mrTextBodyProp.moRotation = aAttribs.getInteger( XML_rot );

//   bool bRtlCol = aAttribs.getBool( XML_rtlCol, false );
    // ST_PositiveCoordinate
//   sal_Int32 nSpcCol = aAttribs.getInteger( XML_spcCol, 0 );
//   bool bSpcFirstLastPara = aAttribs.getBool( XML_spcFirstLastPara, 0 );
//   bool bUpRight = aAttribs.getBool( XML_upright, 0 );

    // ST_TextVerticalType
    mrTextBodyProp.moVert = aAttribs.getToken( XML_vert );
}

// --------------------------------------------------------------------

void TextBodyPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextBodyPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& /*xAttributes*/) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
            // Sequence
            case NMSP_DRAWINGML|XML_prstTxWarp:     // CT_PresetTextShape
            case NMSP_DRAWINGML|XML_prot:           // CT_TextProtectionProperty
                break;

            // EG_TextAutofit
            case NMSP_DRAWINGML|XML_noAutofit:
                mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= false;   // CT_TextNoAutofit
                break;
            case NMSP_DRAWINGML|XML_normAutofit:    // CT_TextNormalAutofit
            case NMSP_DRAWINGML|XML_spAutoFit:
                mrTextBodyProp.maPropertyMap[ PROP_TextAutoGrowHeight ] <<= true;
                break;

            case NMSP_DRAWINGML|XML_scene3d:        // CT_Scene3D

            // EG_Text3D
            case NMSP_DRAWINGML|XML_sp3d:           // CT_Shape3D
            case NMSP_DRAWINGML|XML_flatTx:         // CT_FlatText

                break;
    }

    return xRet;
}

// --------------------------------------------------------------------

} }

