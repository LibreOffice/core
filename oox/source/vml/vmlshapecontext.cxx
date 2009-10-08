/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmldrawingfragmenthandler.cxx,v $
 * $Revision: 1.6 $
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

bool lclSeparateValue( OUString& orName, OUString& orValue, const OUString& rAttrib, sal_Unicode cSep = ':' )
{
    sal_Int32 nSepPos = rAttrib.indexOf( cSep );
    if( nSepPos <= 0 ) return false;
    orName = rAttrib.copy( 0, nSepPos ).trim();
    orValue = rAttrib.copy( nSepPos + 1 ).trim();
    return (orName.getLength() > 0) && (orValue.getLength() > 0);
}

/** Returns the boolean value from the passed string (supported: f, t, False, True).
    @param bDefaultForEmpty  Default value for the empty string.
 */
bool lclDecodeBool( const OUString& rValue, bool bDefaultForEmpty )
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
        case VMLX_TOKEN( Anchor ):      mrClientData.maAnchor = rChars;                             break;
        case VMLX_TOKEN( FmlaLink ):    mrClientData.maLinkedCell = rChars;                         break;
        case VMLX_TOKEN( FmlaPict ):    mrClientData.maPictureLink = rChars;                        break;
        case VMLX_TOKEN( FmlaRange ):   mrClientData.maSourceRange = rChars;                        break;
        case VMLX_TOKEN( PrintObject ): mrClientData.mbPrintObject = lclDecodeBool( rChars, true ); break;
    }
}

// ============================================================================

ShapeContextBase::ShapeContextBase( ContextHandler2Helper& rParent ) :
    ContextHandler2( rParent )
{
}

/*static*/ ContextHandlerRef ShapeContextBase::createContext( ContextHandler2Helper& rParent,
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
    mrTypeModel.monShapeType = rAttribs.getInteger( O_TOKEN( spt ) );
    // coordinate system position/size
    setCoordOrigin( rAttribs.getString( XML_coordorigin, OUString() ) );
    setCoordSize( rAttribs.getString( XML_coordsize, OUString() ) );
    // CSS style
    setStyle( rAttribs.getString( XML_style, OUString() ) );
    // border line
    mrTypeModel.mobStroked = rAttribs.getBool( XML_stroked );
    mrTypeModel.moStrokeColor = rAttribs.getString( XML_strokecolor );
    // shape fill
    mrTypeModel.mobFilled = rAttribs.getBool( XML_filled );
    mrTypeModel.moFillColor = rAttribs.getString( XML_fillcolor );
}

ContextHandlerRef ShapeTypeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( imagedata ):
            OptValue< OUString > oGraphicRelId = rAttribs.getString( O_TOKEN( relid ) );
            if( oGraphicRelId.has() )
                mrTypeModel.moGraphicPath = getFragmentPathFromRelId( oGraphicRelId.get() );
            mrTypeModel.moGraphicTitle = rAttribs.getString( O_TOKEN( title ) );
        break;
    }
    return 0;
}

void ShapeTypeContext::setCoordOrigin( const OUString& rCoordOrigin )
{
    OUString aCoordL, aCoordT;
    if( lclSeparateValue( aCoordL, aCoordT, rCoordOrigin, ',' ) )
    {
        mrTypeModel.monCoordLeft = aCoordL.toInt32();
        mrTypeModel.monCoordTop = aCoordT.toInt32();
    }
}

void ShapeTypeContext::setCoordSize( const OUString& rCoordSize )
{
    OUString aCoordW, aCoordH;
    if( lclSeparateValue( aCoordW, aCoordH, rCoordSize, ',' ) )
    {
        mrTypeModel.monCoordWidth = aCoordW.toInt32();
        mrTypeModel.monCoordHeight = aCoordH.toInt32();
    }
}

void ShapeTypeContext::setStyle( const OUString& rStyle )
{
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        OUString aName, aValue;
        if( lclSeparateValue( aName, aValue, rStyle.getToken( 0, ';', nIndex ) ) )
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
    ContextHandlerRef xContext = ShapeContextBase::createContext( *this, nElement, rAttribs, mrShapes );
    // handle remaining stuff of this shape in base class
    return xContext.get() ? xContext : ShapeContext::onCreateContext( nElement, rAttribs );
}

// ============================================================================

} // namespace vml
} // namespace oox

