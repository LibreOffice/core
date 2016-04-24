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

#include "oox/vml/vmlshapecontext.hxx"

#include "oox/core/xmlfilterbase.hxx"
#include <oox/helper/attributelist.hxx>
#include <oox/helper/helper.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/vml/vmltextboxcontext.hxx"

#include <osl/diagnose.h>

namespace oox {
namespace vml {

using namespace ::com::sun::star;

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

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

/** #119750# Special method for opacity; it *should* be a percentage value, but there are cases
    where a value relative to 0xffff (65536) is used, ending with an 'f'
 */
OptValue< double > lclDecodeOpacity( const AttributeList& rAttribs, sal_Int32 nToken, double fDefValue )
{
    OptValue< OUString > oValue = rAttribs.getString( nToken );
    double fRetval(fDefValue);

    if( oValue.has() )
    {
        const OUString aString(oValue.get());
        const sal_Int32 nLength(aString.getLength());

        if(nLength > 0)
        {
            if(aString.endsWith("f"))
            {
                fRetval = std::max(0.0, std::min(1.0, aString.toDouble() / 65536.0));
            }
            else
            {
                fRetval = ConversionHelper::decodePercent( aString, fDefValue );
            }
        }
    }

    return OptValue< double >(fRetval);
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
    if( rValue.isEmpty() ) return bDefaultForEmpty;
    sal_Int32 nToken = AttributeConversion::decodeToken( rValue );
    // anything else than 't' or 'True' is considered to be false, as specified
    return (nToken == XML_t) || (nToken == XML_True);
}

} // namespace

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
                if( !aToken.isEmpty() )
                    mrDrawing.registerBlockId( aToken.toInt32() );
            }
        }
        break;
    }
    return nullptr;
}

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
        maElementText.clear();
        return this;
    }
    return nullptr;
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

ShapeContextBase::ShapeContextBase( ContextHandler2Helper& rParent ) :
    ContextHandler2( rParent )
{
}

ContextHandlerRef ShapeContextBase::createShapeContext( ContextHandler2Helper& rParent,
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
            if (rAttribs.hasAttribute(XML_path))
                return new ShapeContext( rParent, rShapes.createShape< BezierShape >(), rAttribs );
            else
                return new ShapeContext( rParent, rShapes.createShape< ComplexShape >(), rAttribs );
        case VML_TOKEN( rect ):
            return new RectangleShapeContext( rParent, rAttribs, rShapes.createShape< RectangleShape >() );
        case VML_TOKEN( roundrect ):
            return new ShapeContext( rParent, rShapes.createShape< RectangleShape >(), rAttribs );
        case VML_TOKEN( oval ):
            return new ShapeContext( rParent, rShapes.createShape< EllipseShape >(), rAttribs );
        case VML_TOKEN( polyline ):
            return new ShapeContext( rParent, rShapes.createShape< PolyLineShape >(), rAttribs );
        case VML_TOKEN( line ):
            return new ShapeContext( rParent, rShapes.createShape< LineShape >(), rAttribs );
        case VML_TOKEN( curve ):
            return new ShapeContext( rParent, rShapes.createShape< BezierShape >(), rAttribs );

        // TODO:
        case VML_TOKEN( arc ):
        case VML_TOKEN( diagram ):
        case VML_TOKEN( image ):
            return new ShapeContext( rParent, rShapes.createShape< ComplexShape >(), rAttribs );
    }
    return nullptr;
}

ShapeTypeContext::ShapeTypeContext( ContextHandler2Helper& rParent, ShapeType& rShapeType, const AttributeList& rAttribs ) :
    ShapeContextBase( rParent ),
    mrTypeModel( rShapeType.getTypeModel() )
{
    // shape identifier and shape name
    bool bHasOspid = rAttribs.hasAttribute( O_TOKEN( spid ) );
    mrTypeModel.maShapeId = rAttribs.getXString( bHasOspid ? O_TOKEN( spid ) : XML_id, OUString() );
    mrTypeModel.maLegacyId = rAttribs.getString( XML_id, OUString() );
    OSL_ENSURE( !mrTypeModel.maShapeId.isEmpty(), "ShapeTypeContext::ShapeTypeContext - missing shape identifier" );
    // if the o:spid attribute exists, the id attribute contains the user-defined shape name
    if( bHasOspid )
        mrTypeModel.maShapeName = rAttribs.getXString( XML_id, OUString() );
    // builtin shape type identifier
    mrTypeModel.moShapeType = rAttribs.getInteger( O_TOKEN( spt ) );

    // coordinate system position/size, CSS style
    mrTypeModel.moCoordPos = lclDecodeInt32Pair( rAttribs, XML_coordorigin );
    mrTypeModel.moCoordSize = lclDecodeInt32Pair( rAttribs, XML_coordsize );
    setStyle( rAttribs.getString( XML_style, OUString() ) );
    if( lclDecodeBool( rAttribs, O_TOKEN( hr )).get( false ))
    {   // MSO's handling of o:hr width is nowhere near what the spec says:
        // - o:hrpct is not in % but in 0.1%
        // - if o:hrpct is not given, 100% width is assumed
        // - given width is used only if explicit o:hrpct="0" is given
        OUString hrpct = rAttribs.getString( O_TOKEN( hrpct ), "1000" );
        if( hrpct != "0" )
            mrTypeModel.maWidth = OUString::number( hrpct.toInt32() / 10 ) + "%";
    }

    // stroke settings (may be overridden by v:stroke element later)
    mrTypeModel.maStrokeModel.moStroked = lclDecodeBool( rAttribs, XML_stroked );
    mrTypeModel.maStrokeModel.moColor = rAttribs.getString( XML_strokecolor );
    mrTypeModel.maStrokeModel.moWeight = rAttribs.getString( XML_strokeweight );

    // fill settings (may be overridden by v:fill element later)
    mrTypeModel.maFillModel.moFilled = lclDecodeBool( rAttribs, XML_filled );
    mrTypeModel.maFillModel.moColor = rAttribs.getString( XML_fillcolor );

    // For roundrect we may have a arcsize attribute to read
    mrTypeModel.maArcsize = rAttribs.getString( XML_arcsize,OUString( ) );
    // editas
    mrTypeModel.maEditAs = rAttribs.getString(XML_editas, OUString());
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
            mrTypeModel.maStrokeModel.moOpacity = lclDecodeOpacity( rAttribs, XML_opacity, 1.0 );
            mrTypeModel.maStrokeModel.moWeight.assignIfUsed( rAttribs.getString( XML_weight ) );
            mrTypeModel.maStrokeModel.moDashStyle = rAttribs.getString( XML_dashstyle );
            mrTypeModel.maStrokeModel.moLineStyle = rAttribs.getToken( XML_linestyle );
            mrTypeModel.maStrokeModel.moEndCap = rAttribs.getToken( XML_endcap );
            mrTypeModel.maStrokeModel.moJoinStyle = rAttribs.getToken( XML_joinstyle );
        break;
        case VML_TOKEN( fill ):
            mrTypeModel.maFillModel.moFilled.assignIfUsed( lclDecodeBool( rAttribs, XML_on ) );
            mrTypeModel.maFillModel.moColor.assignIfUsed( rAttribs.getString( XML_color ) );
            mrTypeModel.maFillModel.moOpacity = lclDecodeOpacity( rAttribs, XML_opacity, 1.0 );
            mrTypeModel.maFillModel.moColor2 = rAttribs.getString( XML_color2 );
            mrTypeModel.maFillModel.moOpacity2 = lclDecodeOpacity( rAttribs, XML_opacity2, 1.0 );
            mrTypeModel.maFillModel.moType = rAttribs.getToken( XML_type );
            mrTypeModel.maFillModel.moAngle = rAttribs.getInteger( XML_angle );
            mrTypeModel.maFillModel.moFocus = lclDecodePercent( rAttribs, XML_focus, 0.0 );
            mrTypeModel.maFillModel.moFocusPos = lclDecodePercentPair( rAttribs, XML_focusposition );
            mrTypeModel.maFillModel.moFocusSize = lclDecodePercentPair( rAttribs, XML_focussize );
            mrTypeModel.maFillModel.moBitmapPath = decodeFragmentPath( rAttribs, O_TOKEN( relid ) );
            mrTypeModel.maFillModel.moRotate = lclDecodeBool( rAttribs, XML_rotate );
        break;
        case VML_TOKEN( imagedata ):
        {
            // shapes in docx use r:id for the relationship id
            // in xlsx it they use o:relid
            bool bHasORelId = rAttribs.hasAttribute( O_TOKEN( relid ) );
            mrTypeModel.moGraphicPath = decodeFragmentPath( rAttribs, bHasORelId ? O_TOKEN( relid ) : R_TOKEN( id ) );
            mrTypeModel.moGraphicTitle = rAttribs.getString( O_TOKEN( title ) );

            // Get crop attributes.
            mrTypeModel.moCropBottom = rAttribs.getString(XML_cropbottom);
            mrTypeModel.moCropLeft = rAttribs.getString(XML_cropleft);
            mrTypeModel.moCropRight = rAttribs.getString(XML_cropright);
            mrTypeModel.moCropTop = rAttribs.getString(XML_croptop);
        }
        break;
        case NMSP_vmlWord | XML_wrap:
            mrTypeModel.moWrapAnchorX = rAttribs.getString(XML_anchorx);
            mrTypeModel.moWrapAnchorY = rAttribs.getString(XML_anchory);
            mrTypeModel.moWrapType = rAttribs.getString(XML_type);
            mrTypeModel.moWrapSide = rAttribs.getString(XML_side);
        break;
        case VML_TOKEN( shadow ):
        {
            mrTypeModel.maShadowModel.mbHasShadow = true;
            mrTypeModel.maShadowModel.moShadowOn.assignIfUsed(lclDecodeBool(rAttribs, XML_on));
            mrTypeModel.maShadowModel.moColor.assignIfUsed(rAttribs.getString(XML_color));
            mrTypeModel.maShadowModel.moOffset.assignIfUsed(rAttribs.getString(XML_offset));
            mrTypeModel.maShadowModel.moOpacity = lclDecodePercent(rAttribs, XML_opacity, 1.0);
        }
        break;
        case VML_TOKEN( textpath ):
            mrTypeModel.maTextpathModel.moString.assignIfUsed(rAttribs.getString(XML_string));
        break;
    }
    return nullptr;
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
                 if( aName == "position" )      mrTypeModel.maPosition = aValue;
            else if( aName == "z-index" )        mrTypeModel.maZIndex = aValue;
            else if( aName == "left" )           mrTypeModel.maLeft = aValue;
            else if( aName == "top" )            mrTypeModel.maTop = aValue;
            else if( aName == "width" )          mrTypeModel.maWidth = aValue;
            else if( aName == "height" )         mrTypeModel.maHeight = aValue;
            else if( aName == "margin-left" )    mrTypeModel.maMarginLeft = aValue;
            else if( aName == "margin-top" )     mrTypeModel.maMarginTop = aValue;
            else if( aName == "mso-position-vertical-relative" )  mrTypeModel.maPositionVerticalRelative = aValue;
            else if( aName == "mso-position-horizontal-relative" )  mrTypeModel.maPositionHorizontalRelative = aValue;
            else if( aName == "mso-position-horizontal" ) mrTypeModel.maPositionHorizontal = aValue;
            else if( aName == "mso-position-vertical" ) mrTypeModel.maPositionVertical = aValue;
            else if( aName == "mso-width-percent" ) mrTypeModel.maWidthPercent = aValue;
            else if( aName == "mso-width-relative" ) mrTypeModel.maWidthRelative = aValue;
            else if( aName == "mso-height-percent" ) mrTypeModel.maHeightPercent = aValue;
            else if( aName == "mso-height-relative" ) mrTypeModel.maHeightRelative = aValue;
            else if( aName == "mso-fit-shape-to-text" )           mrTypeModel.mbAutoHeight = true;
            else if( aName == "rotation" )       mrTypeModel.maRotation = aValue;
            else if( aName == "flip" )       mrTypeModel.maFlip = aValue;
            else if( aName == "visibility" )
                mrTypeModel.mbVisible = aValue != "hidden";
            else if( aName == "mso-wrap-style" ) mrTypeModel.maWrapStyle = aValue;
            else if ( aName == "v-text-anchor" ) mrTypeModel.maVTextAnchor = aValue;
            else if ( aName == "mso-wrap-distance-left" ) mrTypeModel.maWrapDistanceLeft = aValue;
            else if ( aName == "mso-wrap-distance-right" ) mrTypeModel.maWrapDistanceRight = aValue;
            else if ( aName == "mso-wrap-distance-top" ) mrTypeModel.maWrapDistanceTop = aValue;
            else if ( aName == "mso-wrap-distance-bottom" ) mrTypeModel.maWrapDistanceBottom = aValue;
        }
    }
}

ShapeContext::ShapeContext( ContextHandler2Helper& rParent, ShapeBase& rShape, const AttributeList& rAttribs ) :
    ShapeTypeContext( rParent, rShape, rAttribs ),
    mrShape( rShape ),
    mrShapeModel( rShape.getShapeModel() )
{
    // collect shape specific attributes
    mrShapeModel.maType = rAttribs.getXString( XML_type, OUString() );
    // polyline path
    setPoints( rAttribs.getString( XML_points, OUString() ) );
    // line start and end positions
    setFrom(rAttribs.getString(XML_from, OUString()));
    setTo(rAttribs.getString(XML_to, OUString()));
    setControl1(rAttribs.getString(XML_control1, OUString()));
    setControl2(rAttribs.getString(XML_control2, OUString()));
    setVmlPath(rAttribs.getString(XML_path, OUString()));
}

ContextHandlerRef ShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // Excel specific shape client data
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( textbox ):
            if (getParentElement() != VML_TOKEN( group ))
            {
                // Custom shape in Writer with a textbox are transformed into a frame
                dynamic_cast<SimpleShape&>( mrShape ).setService(
                        "com.sun.star.text.TextFrame");
            }
            else if (getCurrentElement() == VML_TOKEN(rect))
                // Transform only rectangles into a TextShape inside a groupshape.
                dynamic_cast<SimpleShape&>(mrShape).setService("com.sun.star.drawing.TextShape");
            return new TextBoxContext( *this, mrShapeModel.createTextBox(mrShape.getTypeModel()), rAttribs,
                mrShape.getDrawing().getFilter().getGraphicHelper());
        case VMLX_TOKEN( ClientData ):
            return new ClientDataContext( *this, mrShapeModel.createClientData(), rAttribs );
        case VMLPPT_TOKEN( textdata ):
            // Force RectangleShape, this is ugly :(
            // and is there because of the lines above which change it to TextFrame
            dynamic_cast< SimpleShape& >( mrShape ).setService(
                    "com.sun.star.drawing.RectangleShape");
            mrShapeModel.maLegacyDiagramPath = getFragmentPathFromRelId(rAttribs.getString(XML_id, OUString()));
            break;
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
        mrShapeModel.maPoints.push_back( awt::Point( nX, nY ) );
    }
}

void ShapeContext::setFrom( const OUString& rPoints )
{
    if (!rPoints.isEmpty())
        mrShapeModel.maFrom = rPoints;
}

void ShapeContext::setTo( const OUString& rPoints )
{
    if (!rPoints.isEmpty())
        mrShapeModel.maTo = rPoints;
}

void ShapeContext::setControl1( const OUString& rPoints )
{
    if (!rPoints.isEmpty())
        mrShapeModel.maControl1 = rPoints;
}

void ShapeContext::setControl2( const OUString& rPoints )
{
    if (!rPoints.isEmpty())
        mrShapeModel.maControl2 = rPoints;
}
void ShapeContext::setVmlPath( const OUString& rPath )
{
    if (!rPath.isEmpty())
        mrShapeModel.maVmlPath = rPath;
}

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

RectangleShapeContext::RectangleShapeContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, RectangleShape& rShape ) :
    ShapeContext( rParent, rShape, rAttribs )
{
}

ContextHandlerRef RectangleShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // The parent class's context is fine
    return ShapeContext::onCreateContext( nElement, rAttribs );
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
