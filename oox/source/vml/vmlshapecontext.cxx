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

#include "oox/vml/vmlshapecontext.hxx"

#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/vml/vmltextboxcontext.hxx"

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star::awt;

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;
using ::rtl::OUString;

// ============================================================================

namespace {

/** Returns the boolean value from the specified VML attribute (if present).
 */
OptValue< bool > lclDecodeBool( const AttributeList& rAttribs, sal_Int32 nToken )
{
    OptValue< OUString > oValue = rAttribs.getString( nToken );
    if( oValue.has() ) return OptValue< bool >( ConversionHelper::decodeBool( oValue.get() ) );
    return OptValue< bool >();
}

/** Returns the percentage value from the specified VML attribute (if present).
    The value will be normalized (1.0 is returned for 100%).
 */
OptValue< double > lclDecodePercent( const AttributeList& rAttribs, sal_Int32 nToken, double fDefValue )
{
    OptValue< OUString > oValue = rAttribs.getString( nToken );
    if( oValue.has() ) return OptValue< double >( ConversionHelper::decodePercent( oValue.get(), fDefValue ) );
    return OptValue< double >();
}

/** Returns the integer value pair from the specified VML attribute (if present).
 */
OptValue< Int32Pair > lclDecodeInt32Pair( const AttributeList& rAttribs, sal_Int32 nToken )
{
    OptValue< OUString > oValue = rAttribs.getString( nToken );
    OptValue< Int32Pair > oRetValue;
    if( oValue.has() )
    {
        OUString aValue1, aValue2;
        ConversionHelper::separatePair( aValue1, aValue2, oValue.get(), ',' );
        oRetValue = Int32Pair( aValue1.toInt32(), aValue2.toInt32() );
    }
    return oRetValue;
}

/** Returns the percentage pair from the specified VML attribute (if present).
 */
OptValue< DoublePair > lclDecodePercentPair( const AttributeList& rAttribs, sal_Int32 nToken )
{
    OptValue< OUString > oValue = rAttribs.getString( nToken );
    OptValue< DoublePair > oRetValue;
    if( oValue.has() )
    {
        OUString aValue1, aValue2;
        ConversionHelper::separatePair( aValue1, aValue2, oValue.get(), ',' );
        oRetValue = DoublePair(
            ConversionHelper::decodePercent( aValue1, 0.0 ),
            ConversionHelper::decodePercent( aValue2, 0.0 ) );
    }
    return oRetValue;
}

/** Returns the boolean value from the passed string of an attribute in the x:
    namespace (VML for spreadsheets). Supported values: f, t, False, True.
    @param bDefaultForEmpty  Default value for the empty string.
 */
bool lclDecodeVmlxBool( const OUString& rValue, bool bDefaultForEmpty )
{
    if( rValue.getLength() == 0 ) return bDefaultForEmpty;
    sal_Int32 nToken = AttributeConversion::decodeToken( rValue );
    // anything else than 't' or 'True' is considered to be false, as specified
    return (nToken == XML_t) || (nToken == XML_True);
}

} // namespace

// ============================================================================

ShapeLayoutContext::ShapeLayoutContext( ContextHandler2Helper& rParent, Drawing& rDrawing ) :
    ContextHandler2( rParent ),
    mrDrawing( rDrawing )
{
}


ContextHandlerRef ShapeLayoutContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case O_TOKEN( idmap ):
        {
            OUString aBlockIds = rAttribs.getString( XML_data, OUString() );
            sal_Int32 nIndex = 0;
            while( nIndex >= 0 )
            {
                OUString aToken = aBlockIds.getToken( 0, ' ', nIndex ).trim();
                if( aToken.getLength() > 0 )
                    mrDrawing.registerBlockId( aToken.toInt32() );
            }
        }
        break;
    }
    return 0;
}

// ============================================================================

ClientDataContext::ClientDataContext( ContextHandler2Helper& rParent,
        ClientData& rClientData, const AttributeList& rAttribs ) :
    ContextHandler2( rParent ),
    mrClientData( rClientData )
{
    mrClientData.mnObjType = rAttribs.getToken( XML_ObjectType, XML_TOKEN_INVALID );
}

ContextHandlerRef ClientDataContext::onCreateContext( sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
    if( isRootElement() )
    {
        maElementText = OUString();
        return this;
    }
    return 0;
}

void ClientDataContext::onCharacters( const OUString& rChars )
{
    /*  Empty but existing elements have special meaning, e.g. 'true'. Collect
        existing text and convert it in onEndElement(). */
    maElementText = rChars;
}

void ClientDataContext::onEndElement()
{
    switch( getCurrentElement() )
    {
        case VMLX_TOKEN( Anchor ):      mrClientData.maAnchor = maElementText;                                          break;
        case VMLX_TOKEN( FmlaMacro ):   mrClientData.maFmlaMacro = maElementText;                                       break;
        case VMLX_TOKEN( FmlaPict ):    mrClientData.maFmlaPict = maElementText;                                        break;
        case VMLX_TOKEN( FmlaLink ):    mrClientData.maFmlaLink = maElementText;                                        break;
        case VMLX_TOKEN( FmlaRange ):   mrClientData.maFmlaRange = maElementText;                                       break;
        case VMLX_TOKEN( FmlaGroup ):   mrClientData.maFmlaGroup = maElementText;                                       break;
        case VMLX_TOKEN( TextHAlign ):  mrClientData.mnTextHAlign = AttributeConversion::decodeToken( maElementText );  break;
        case VMLX_TOKEN( TextVAlign ):  mrClientData.mnTextVAlign = AttributeConversion::decodeToken( maElementText );  break;
        case VMLX_TOKEN( Column ):      mrClientData.mnCol = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Row ):         mrClientData.mnRow = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Checked ):     mrClientData.mnChecked = maElementText.toInt32();                               break;
        case VMLX_TOKEN( DropStyle ):   mrClientData.mnDropStyle = AttributeConversion::decodeToken( maElementText );   break;
        case VMLX_TOKEN( DropLines ):   mrClientData.mnDropLines = maElementText.toInt32();                             break;
        case VMLX_TOKEN( Val ):         mrClientData.mnVal = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Min ):         mrClientData.mnMin = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Max ):         mrClientData.mnMax = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Inc ):         mrClientData.mnInc = maElementText.toInt32();                                   break;
        case VMLX_TOKEN( Page ):        mrClientData.mnPage = maElementText.toInt32();                                  break;
        case VMLX_TOKEN( SelType ):     mrClientData.mnSelType = AttributeConversion::decodeToken( maElementText );     break;
        case VMLX_TOKEN( VTEdit ):      mrClientData.mnVTEdit = maElementText.toInt32();                                break;
        case VMLX_TOKEN( PrintObject ): mrClientData.mbPrintObject = lclDecodeVmlxBool( maElementText, true );          break;
        case VMLX_TOKEN( Visible ):     mrClientData.mbVisible = lclDecodeVmlxBool( maElementText, true );              break;
        case VMLX_TOKEN( DDE ):         mrClientData.mbDde = lclDecodeVmlxBool( maElementText, true );                  break;
        case VMLX_TOKEN( NoThreeD ):    mrClientData.mbNo3D = lclDecodeVmlxBool( maElementText, true );                 break;
        case VMLX_TOKEN( NoThreeD2 ):   mrClientData.mbNo3D2 = lclDecodeVmlxBool( maElementText, true );                break;
        case VMLX_TOKEN( MultiLine ):   mrClientData.mbMultiLine = lclDecodeVmlxBool( maElementText, true );            break;
        case VMLX_TOKEN( VScroll ):     mrClientData.mbVScroll = lclDecodeVmlxBool( maElementText, true );              break;
        case VMLX_TOKEN( SecretEdit ):  mrClientData.mbSecretEdit = lclDecodeVmlxBool( maElementText, true );           break;
    }
}

// ============================================================================

ShapeContextBase::ShapeContextBase( ContextHandler2Helper& rParent ) :
    ContextHandler2( rParent )
{
}

/*static*/ ContextHandlerRef ShapeContextBase::createShapeContext( ContextHandler2Helper& rParent,
        ShapeContainer& rShapes, sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case O_TOKEN( shapelayout ):
            return new ShapeLayoutContext( rParent, rShapes.getDrawing() );

        case VML_TOKEN( shapetype ):
            return new ShapeTypeContext( rParent, rShapes.createShapeType(), rAttribs );
        case VML_TOKEN( group ):
            return new GroupShapeContext( rParent, rShapes.createShape< GroupShape >(), rAttribs );
        case VML_TOKEN( shape ):
            return new ShapeContext( rParent, rShapes.createShape< ComplexShape >(), rAttribs );
        case VML_TOKEN( rect ):
            return new RectangleShapeContext( rParent, rAttribs, rShapes.createShape< RectangleShape >() );
        case VML_TOKEN( roundrect ):
            return new ShapeContext( rParent, rShapes.createShape< RectangleShape >(), rAttribs );
        case VML_TOKEN( oval ):
            return new ShapeContext( rParent, rShapes.createShape< EllipseShape >(), rAttribs );
        case VML_TOKEN( polyline ):
            return new ShapeContext( rParent, rShapes.createShape< PolyLineShape >(), rAttribs );

        // TODO:
        case VML_TOKEN( arc ):
        case VML_TOKEN( curve ):
        case VML_TOKEN( line ):
        case VML_TOKEN( diagram ):
        case VML_TOKEN( image ):
            return new ShapeContext( rParent, rShapes.createShape< ComplexShape >(), rAttribs );
    }
    return 0;
}

// ============================================================================

ShapeTypeContext::ShapeTypeContext( ContextHandler2Helper& rParent, ShapeType& rShapeType, const AttributeList& rAttribs ) :
    ShapeContextBase( rParent ),
    mrTypeModel( rShapeType.getTypeModel() )
{
    // shape identifier and shape name
    bool bHasOspid = rAttribs.hasAttribute( O_TOKEN( spid ) );
    mrTypeModel.maShapeId = rAttribs.getXString( bHasOspid ? O_TOKEN( spid ) : XML_id, OUString() );
    OSL_ENSURE( mrTypeModel.maShapeId.getLength() > 0, "ShapeTypeContext::ShapeTypeContext - missing shape identifier" );
    // if the o:spid attribute exists, the id attribute contains the user-defined shape name
    if( bHasOspid )
        mrTypeModel.maShapeName = rAttribs.getXString( XML_id, OUString() );
    // builtin shape type identifier
    mrTypeModel.moShapeType = rAttribs.getInteger( O_TOKEN( spt ) );

    // coordinate system position/size, CSS style
    mrTypeModel.moCoordPos = lclDecodeInt32Pair( rAttribs, XML_coordorigin );
    mrTypeModel.moCoordSize = lclDecodeInt32Pair( rAttribs, XML_coordsize );
    setStyle( rAttribs.getString( XML_style, OUString() ) );

    // stroke settings (may be overridden by v:stroke element later)
    mrTypeModel.maStrokeModel.moStroked = lclDecodeBool( rAttribs, XML_stroked );
    mrTypeModel.maStrokeModel.moColor = rAttribs.getString( XML_strokecolor );
    mrTypeModel.maStrokeModel.moWeight = rAttribs.getString( XML_strokeweight );

    // fill settings (may be overridden by v:fill element later)
    mrTypeModel.maFillModel.moFilled = lclDecodeBool( rAttribs, XML_filled );
    mrTypeModel.maFillModel.moColor = rAttribs.getString( XML_fillcolor );
}

ContextHandlerRef ShapeTypeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( stroke ):
            mrTypeModel.maStrokeModel.moStroked.assignIfUsed( lclDecodeBool( rAttribs, XML_on ) );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowType = rAttribs.getToken( XML_startarrow );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowWidth = rAttribs.getToken( XML_startarrowwidth );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowLength = rAttribs.getToken( XML_startarrowlength );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowType = rAttribs.getToken( XML_endarrow );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowWidth = rAttribs.getToken( XML_endarrowwidth );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowLength = rAttribs.getToken( XML_endarrowlength );
            mrTypeModel.maStrokeModel.moColor.assignIfUsed( rAttribs.getString( XML_color ) );
            mrTypeModel.maStrokeModel.moOpacity = lclDecodePercent( rAttribs, XML_opacity, 1.0 );
            mrTypeModel.maStrokeModel.moWeight.assignIfUsed( rAttribs.getString( XML_weight ) );
            mrTypeModel.maStrokeModel.moDashStyle = rAttribs.getString( XML_dashstyle );
            mrTypeModel.maStrokeModel.moLineStyle = rAttribs.getToken( XML_linestyle );
            mrTypeModel.maStrokeModel.moEndCap = rAttribs.getToken( XML_endcap );
            mrTypeModel.maStrokeModel.moJoinStyle = rAttribs.getToken( XML_joinstyle );
        break;
        case VML_TOKEN( fill ):
            mrTypeModel.maFillModel.moFilled.assignIfUsed( lclDecodeBool( rAttribs, XML_on ) );
            mrTypeModel.maFillModel.moColor.assignIfUsed( rAttribs.getString( XML_color ) );
            mrTypeModel.maFillModel.moOpacity = lclDecodePercent( rAttribs, XML_opacity, 1.0 );
            mrTypeModel.maFillModel.moColor2 = rAttribs.getString( XML_color2 );
            mrTypeModel.maFillModel.moOpacity2 = lclDecodePercent( rAttribs, XML_opacity2, 1.0 );
            mrTypeModel.maFillModel.moType = rAttribs.getToken( XML_type );
            mrTypeModel.maFillModel.moAngle = rAttribs.getInteger( XML_angle );
            mrTypeModel.maFillModel.moFocus = lclDecodePercent( rAttribs, XML_focus, 0.0 );
            mrTypeModel.maFillModel.moFocusPos = lclDecodePercentPair( rAttribs, XML_focusposition );
            mrTypeModel.maFillModel.moFocusSize = lclDecodePercentPair( rAttribs, XML_focussize );
            mrTypeModel.maFillModel.moBitmapPath = decodeFragmentPath( rAttribs, O_TOKEN( relid ) );
            mrTypeModel.maFillModel.moRotate = lclDecodeBool( rAttribs, XML_rotate );
        break;
        case VML_TOKEN( imagedata ):
            // shapes in docx use r:id for the relationship id
            // in xlsx it they use o:relid
            bool bHasORelId = rAttribs.hasAttribute( O_TOKEN( relid ) );
            mrTypeModel.moGraphicPath = decodeFragmentPath( rAttribs, bHasORelId ? O_TOKEN( relid ) : R_TOKEN( id ) );
            mrTypeModel.moGraphicTitle = rAttribs.getString( O_TOKEN( title ) );
        break;
    }
    return 0;
}

OptValue< OUString > ShapeTypeContext::decodeFragmentPath( const AttributeList& rAttribs, sal_Int32 nToken ) const
{
    OptValue< OUString > oFragmentPath;
    OptValue< OUString > oRelId = rAttribs.getString( nToken );
    if( oRelId.has() )
        oFragmentPath = getFragmentPathFromRelId( oRelId.get() );
    return oFragmentPath;
}

void ShapeTypeContext::setStyle( const OUString& rStyle )
{
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        OUString aName, aValue;
        if( ConversionHelper::separatePair( aName, aValue, rStyle.getToken( 0, ';', nIndex ), ':' ) )
        {
                 if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "position" ) ) )      mrTypeModel.maPosition = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "left" ) ) )          mrTypeModel.maLeft = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "top" ) ) )           mrTypeModel.maTop = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "width" ) ) )         mrTypeModel.maWidth = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "height" ) ) )        mrTypeModel.maHeight = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "margin-left" ) ) )   mrTypeModel.maMarginLeft = aValue;
            else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "margin-top" ) ) )    mrTypeModel.maMarginTop = aValue;
        }
    }
}

// ============================================================================

ShapeContext::ShapeContext( ContextHandler2Helper& rParent, ShapeBase& rShape, const AttributeList& rAttribs ) :
    ShapeTypeContext( rParent, rShape, rAttribs ),
    mrShape( rShape ),
    mrShapeModel( rShape.getShapeModel() )
{
    // collect shape specific attributes
    mrShapeModel.maType = rAttribs.getXString( XML_type, OUString() );
    // polyline path
    setPoints( rAttribs.getString( XML_points, OUString() ) );
}

ContextHandlerRef ShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // Custom shape in Writer with a textbox are transformed into a frame
    if ( nElement == ( NMSP_VML + XML_textbox ) )
        dynamic_cast<SimpleShape&>( mrShape ).setService(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")) );

    // Excel specific shape client data
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( textbox ):
            return new TextBoxContext( *this, mrShapeModel.createTextBox(), rAttribs );
        case VMLX_TOKEN( ClientData ):
            return new ClientDataContext( *this, mrShapeModel.createClientData(), rAttribs );
    }
    // handle remaining stuff in base class
    return ShapeTypeContext::onCreateContext( nElement, rAttribs );
}

void ShapeContext::setPoints( const OUString& rPoints )
{
    mrShapeModel.maPoints.clear();
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        sal_Int32 nX = rPoints.getToken( 0, ',', nIndex ).toInt32();
        sal_Int32 nY = rPoints.getToken( 0, ',', nIndex ).toInt32();
        mrShapeModel.maPoints.push_back( Point( nX, nY ) );
    }
}

// ============================================================================

GroupShapeContext::GroupShapeContext( ContextHandler2Helper& rParent, GroupShape& rShape, const AttributeList& rAttribs ) :
    ShapeContext( rParent, rShape, rAttribs ),
    mrShapes( rShape.getChildren() )
{
}

ContextHandlerRef GroupShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // try to create a context of an embedded shape
    ContextHandlerRef xContext = createShapeContext( *this, mrShapes, nElement, rAttribs );
    // handle remaining stuff of this shape in base class
    return xContext.get() ? xContext : ShapeContext::onCreateContext( nElement, rAttribs );
}

// ============================================================================

RectangleShapeContext::RectangleShapeContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, RectangleShape& rShape ) :
    ShapeContext( rParent, rAttribs, rShape )
{
}

ContextHandlerRef RectangleShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if ( nElement == ( NMSP_VML + XML_textbox ) )
        dynamic_cast< SimpleShape &>( mrShape ).setService(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")) );

    // The parent class's context is fine
    return ShapeContext::onCreateContext( nElement, rAttribs );
}
// ============================================================================

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
