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
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

using ::rtl::OUString;
using ::com::sun::star::awt::Point;
using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace vml {

// ============================================================================

namespace {

/** Returns the boolean value from the specified VML attribute (if present).
 */
OptValue< bool > lclDecodeBool( const AttributeList& rAttribs, sal_Int32 nElement )
{
    OptValue< OUString > oValue = rAttribs.getString( nElement );
    if( oValue.has() ) return OptValue< bool >( ConversionHelper::decodeBool( oValue.get() ) );
    return OptValue< bool >();
}

/** Returns the percentage value from the specified VML attribute (if present).
    The value will be normalized (1.0 is returned for 100%).
 */
OptValue< double > lclDecodePercent( const AttributeList& rAttribs, sal_Int32 nElement, double fDefValue )
{
    OptValue< OUString > oValue = rAttribs.getString( nElement );
    if( oValue.has() ) return OptValue< double >( ConversionHelper::decodePercent( oValue.get(), fDefValue ) );
    return OptValue< double >();
}

/** Returns the integer value pair from the specified VML attribute (if present).
 */
OptValue< Int32Pair > lclDecodeInt32Pair( const AttributeList& rAttribs, sal_Int32 nElement )
{
    OptValue< OUString > oValue = rAttribs.getString( nElement );
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
OptValue< DoublePair > lclDecodePercentPair( const AttributeList& rAttribs, sal_Int32 nElement )
{
    OptValue< OUString > oValue = rAttribs.getString( nElement );
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
    // anything else than 't' or 'True' is considered to be false, as specified
    return ((rValue.getLength() == 1) && (rValue[ 0 ] == 't')) || (rValue == CREATE_OUSTRING( "True" ));
}

} // namespace

// ============================================================================

ShapeClientDataContext::ShapeClientDataContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, ShapeClientData& rClientData ) :
    ContextHandler2( rParent ),
    mrClientData( rClientData )
{
    mrClientData.mnObjType = rAttribs.getToken( XML_ObjectType, XML_TOKEN_INVALID );
}

ContextHandlerRef ShapeClientDataContext::onCreateContext( sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
    return isRootElement() ? this : 0;
}

void ShapeClientDataContext::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case VMLX_TOKEN( Anchor ):      mrClientData.maAnchor = rChars;                                 break;
        case VMLX_TOKEN( FmlaPict ):    mrClientData.maPictureLink = rChars;                            break;
        case VMLX_TOKEN( FmlaLink ):    mrClientData.maLinkedCell = rChars;                             break;
        case VMLX_TOKEN( FmlaRange ):   mrClientData.maSourceRange = rChars;                            break;
        case VMLX_TOKEN( Column ):      mrClientData.mnCol = rChars.toInt32();                          break;
        case VMLX_TOKEN( Row ):         mrClientData.mnRow = rChars.toInt32();                          break;
        case VMLX_TOKEN( PrintObject ): mrClientData.mbPrintObject = lclDecodeVmlxBool( rChars, true ); break;
        case VMLX_TOKEN( Visible ):     mrClientData.mbVisible = true;                                  break;
    }
}

// ============================================================================

ShapeContextBase::ShapeContextBase( ContextHandler2Helper& rParent ) :
    ContextHandler2( rParent )
{
}

/*static*/ ContextHandlerRef ShapeContextBase::createShapeContext( ContextHandler2Helper& rParent,
        sal_Int32 nElement, const AttributeList& rAttribs, ShapeContainer& rShapes )
{
    switch( nElement )
    {
        case VML_TOKEN( shapetype ):
            return new ShapeTypeContext( rParent, rAttribs, rShapes.createShapeType() );
        case VML_TOKEN( group ):
            return new GroupShapeContext( rParent, rAttribs, rShapes.createShape< GroupShape >() );
        case VML_TOKEN( shape ):
            return new ShapeContext( rParent, rAttribs, rShapes.createShape< ComplexShape >() );
        case VML_TOKEN( rect ):
        case VML_TOKEN( roundrect ):
            return new ShapeContext( rParent, rAttribs, rShapes.createShape< RectangleShape >() );
        case VML_TOKEN( oval ):
            return new ShapeContext( rParent, rAttribs, rShapes.createShape< EllipseShape >() );
        case VML_TOKEN( polyline ):
            return new ShapeContext( rParent, rAttribs, rShapes.createShape< PolyLineShape >() );

        // TODO:
        case VML_TOKEN( arc ):
        case VML_TOKEN( curve ):
        case VML_TOKEN( line ):
        case VML_TOKEN( diagram ):
        case VML_TOKEN( image ):
            return new ShapeContext( rParent, rAttribs, rShapes.createShape< ComplexShape >() );
    }
    return false;
}

// ============================================================================

ShapeTypeContext::ShapeTypeContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, ShapeType& rShapeType ) :
    ShapeContextBase( rParent ),
    mrTypeModel( rShapeType.getTypeModel() )
{
    // shape identifier and shape name
    bool bHasOspid = rAttribs.hasAttribute( O_TOKEN( spid ) );
    mrTypeModel.maShapeId = rAttribs.getXString( bHasOspid ? O_TOKEN( spid ) : XML_id, OUString() );
    OSL_ENSURE( mrTypeModel.maShapeId.getLength() > 0, "ShapeTypeContext::ShapeTypeContext - missing shape identifier" );
    // if the o:spid attribute exists, the id attribute contains the user-defined shape name
    if( bHasOspid )
        mrTypeModel.maName = rAttribs.getXString( XML_id, OUString() );
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
            mrTypeModel.maFillModel.moRotate = lclDecodeBool( rAttribs, XML_rotate );
        break;
        case VML_TOKEN( imagedata ):
            OptValue< OUString > oGraphicRelId = rAttribs.getString( O_TOKEN( relid ) );
            if( oGraphicRelId.has() )
                mrTypeModel.moGraphicPath = getFragmentPathFromRelId( oGraphicRelId.get() );
            mrTypeModel.moGraphicTitle = rAttribs.getString( O_TOKEN( title ) );
        break;
    }
    return 0;
}

void ShapeTypeContext::setStyle( const OUString& rStyle )
{
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        OUString aName, aValue;
        if( ConversionHelper::separatePair( aName, aValue, rStyle.getToken( 0, ';', nIndex ), ':' ) )
        {
                 if( aName.equalsAscii( "position" ) )      mrTypeModel.maPosition = aValue;
            else if( aName.equalsAscii( "left" ) )          mrTypeModel.maLeft = aValue;
            else if( aName.equalsAscii( "top" ) )           mrTypeModel.maTop = aValue;
            else if( aName.equalsAscii( "width" ) )         mrTypeModel.maWidth = aValue;
            else if( aName.equalsAscii( "height" ) )        mrTypeModel.maHeight = aValue;
            else if( aName.equalsAscii( "margin-left" ) )   mrTypeModel.maMarginLeft = aValue;
            else if( aName.equalsAscii( "margin-top" ) )    mrTypeModel.maMarginTop = aValue;
        }
    }
}

// ============================================================================

ShapeContext::ShapeContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, ShapeBase& rShape ) :
    ShapeTypeContext( rParent, rAttribs, rShape ),
    mrShapeModel( rShape.getShapeModel() )
{
    // collect shape specific attributes
    mrShapeModel.maType = rAttribs.getXString( XML_type, OUString() );
    // polyline path
    setPoints( rAttribs.getString( XML_points, OUString() ) );
}

ContextHandlerRef ShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // Excel specific shape client data
    if( isRootElement() && (nElement == VMLX_TOKEN( ClientData )) )
        return new ShapeClientDataContext( *this, rAttribs, mrShapeModel.createClientData() );
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

GroupShapeContext::GroupShapeContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, GroupShape& rShape ) :
    ShapeContext( rParent, rAttribs, rShape ),
    mrShapes( rShape.getChildren() )
{
}

ContextHandlerRef GroupShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // try to create a context of an embedded shape
    ContextHandlerRef xContext = createShapeContext( *this, nElement, rAttribs, mrShapes );
    // handle remaining stuff of this shape in base class
    return xContext.get() ? xContext : ShapeContext::onCreateContext( nElement, rAttribs );
}

// ============================================================================

} // namespace vml
} // namespace oox

