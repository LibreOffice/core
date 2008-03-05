/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textbodypropertiescontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:27:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/textbodypropertiescontext.hxx"

#include <com/sun/star/text/ControlCharacter.hpp>
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
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
    const Reference< XFastAttributeList >& xAttributes, Shape& rShape )
: ContextHandler( rParent )
, mrShape( rShape )
{
    AttributeList attribs(xAttributes);

    // ST_TextWrappingType
    sal_Int32 nWrappingType = xAttributes->getOptionalValueToken( XML_wrap, XML_square );
    const OUString sTextWordWrap( RTL_CONSTASCII_USTRINGPARAM( "TextWordWrap" ) );
    mrShape.getShapeProperties()[ sTextWordWrap ] <<= (nWrappingType == XML_square);

    // ST_Coordinate
    const OUString sTextLeftDistance( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) );
    const OUString sTextUpperDistance( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) );
    const OUString sTextRightDistance( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) );
    const OUString sTextLowerDistance( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) );
    OUString sValue;
    sValue = xAttributes->getOptionalValue( XML_lIns );
    sal_Int32 nLeftInset = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrShape.getShapeProperties()[ sTextLeftDistance ]  <<= static_cast< sal_Int32 >( nLeftInset );

    sValue = xAttributes->getOptionalValue( XML_tIns );
    sal_Int32 nTopInset  = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrShape.getShapeProperties()[ sTextUpperDistance ] <<= static_cast< sal_Int32 >( nTopInset );

    sValue = xAttributes->getOptionalValue( XML_rIns );
    sal_Int32 nRightInset  = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 91440 / 360 );
    mrShape.getShapeProperties()[ sTextRightDistance ] <<= static_cast< sal_Int32 >( nRightInset );

    sValue = xAttributes->getOptionalValue( XML_bIns );
    sal_Int32 nBottonInset = ( sValue.getLength() != 0 ? GetCoordinate(  sValue ) : 45720 / 360 );;
    mrShape.getShapeProperties()[ sTextLowerDistance ] <<= static_cast< sal_Int32 >( nBottonInset );


    // ST_TextAnchoringType
//   sal_Int32 nAnchoringType = xAttributes->getOptionalValueToken( XML_anchor, XML_t );

//   bool bAnchorCenter = attribs.getBool( XML_anchorCtr, false );

//   bool bCompatLineSpacing = attribs.getBool( XML_compatLnSpc, false );
//   bool bForceAA = attribs.getBool( XML_forceAA, false );
//   bool bFromWordArt = attribs.getBool( XML_fromWordArt, false );

  // ST_TextHorzOverflowType
//   sal_Int32 nHorzOverflow = xAttributes->getOptionalValueToken( XML_horzOverflow, XML_overflow );
    // ST_TextVertOverflowType
//   sal_Int32 nVertOverflow =  xAttributes->getOptionalValueToken( XML_vertOverflow, XML_overflow );

    // ST_TextColumnCount
//   sal_Int32 nNumCol = attribs.getInteger( XML_numCol, 1 );

    // ST_Angle
//   sal_Int32 nRot = attribs.getInteger( XML_rot, 0 );
//   bool bRtlCol = attribs.getBool( XML_rtlCol, false );
    // ST_PositiveCoordinate
//   sal_Int32 nSpcCol = attribs.getInteger( XML_spcCol, 0 );
//   bool bSpcFirstLastPara = attribs.getBool( XML_spcFirstLastPara, 0 );
//   bool bUpRight = attribs.getBool( XML_upright, 0 );
    // ST_TextVerticalType
//   sal_Int32 nVert = xAttributes->getOptionalValueToken( XML_vert, XML_horz );
}

// --------------------------------------------------------------------

void TextBodyPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextBodyPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& /*xAttributes*/) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    const OUString sTextAutoGrowHeight( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) );
    switch( aElementToken )
    {
            // Sequence
            case NMSP_DRAWINGML|XML_prstTxWarp:     // CT_PresetTextShape
            case NMSP_DRAWINGML|XML_prot:           // CT_TextProtectionProperty
                break;

            // EG_TextAutofit
            case NMSP_DRAWINGML|XML_noAutofit:
                mrShape.getShapeProperties()[ sTextAutoGrowHeight ] <<= sal_False;  // CT_TextNoAutofit
                break;
            case NMSP_DRAWINGML|XML_normAutofit:    // CT_TextNormalAutofit
            case NMSP_DRAWINGML|XML_spAutoFit:
                mrShape.getShapeProperties()[ sTextAutoGrowHeight ] <<= sal_True;
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

