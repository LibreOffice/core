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

#include <sal/config.h>

#include <string_view>

#include <oox/vml/vmlshapecontext.hxx>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/helper.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/vml/vmltextboxcontext.hxx>

#include <osl/diagnose.h>
#include <filter/msfilter/escherex.hxx>
#include <o3tl/string_view.hxx>

namespace oox::vml {

using namespace ::com::sun::star;

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace {

/** Returns the boolean value from the specified VML attribute (if present).
 */
std::optional< bool > lclDecodeBool( const AttributeList& rAttribs, sal_Int32 nToken )
{
    std::optional< OUString > oValue = rAttribs.getString( nToken );
    if( oValue.has_value() ) return std::optional< bool >( ConversionHelper::decodeBool( oValue.value() ) );
    return std::optional< bool >();
}

/** Returns the percentage value from the specified VML attribute (if present).
    The value will be normalized (1.0 is returned for 100%).
 */
std::optional< double > lclDecodePercent( const AttributeList& rAttribs, sal_Int32 nToken, double fDefValue )
{
    std::optional< OUString > oValue = rAttribs.getString( nToken );
    if( oValue.has_value() ) return std::optional< double >( ConversionHelper::decodePercent( oValue.value(), fDefValue ) );
    return std::optional< double >();
}

/** #119750# Special method for opacity; it *should* be a percentage value, but there are cases
    where a value relative to 0xffff (65536) is used, ending with an 'f'
 */
std::optional< double > lclDecodeOpacity( const AttributeList& rAttribs, sal_Int32 nToken, double fDefValue )
{
    std::optional< OUString > oValue = rAttribs.getString( nToken );
    double fRetval(fDefValue);

    if( oValue.has_value() )
    {
        const OUString& aString(oValue.value());
        const sal_Int32 nLength(aString.getLength());

        if(nLength > 0)
        {
            if(aString.endsWith("f"))
            {
                fRetval = std::clamp(aString.toDouble() / 65536.0, 0.0, 1.0);
            }
            else
            {
                fRetval = ConversionHelper::decodePercent( aString, fDefValue );
            }
        }
    }

    return std::optional< double >(fRetval);
}

/** Returns the integer value pair from the specified VML attribute (if present).
 */
std::optional< Int32Pair > lclDecodeInt32Pair( const AttributeList& rAttribs, sal_Int32 nToken )
{
    std::optional< OUString > oValue = rAttribs.getString( nToken );
    std::optional< Int32Pair > oRetValue;
    if( oValue.has_value() )
    {
        std::u16string_view aValue1, aValue2;
        ConversionHelper::separatePair( aValue1, aValue2, oValue.value(), ',' );
        oRetValue = Int32Pair( o3tl::toInt32(aValue1), o3tl::toInt32(aValue2) );
    }
    return oRetValue;
}

/** Returns the percentage pair from the specified VML attribute (if present).
 */
std::optional< DoublePair > lclDecodePercentPair( const AttributeList& rAttribs, sal_Int32 nToken )
{
    std::optional< OUString > oValue = rAttribs.getString( nToken );
    std::optional< DoublePair > oRetValue;
    if( oValue.has_value() )
    {
        std::u16string_view aValue1, aValue2;
        ConversionHelper::separatePair( aValue1, aValue2, oValue.value(), ',' );
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
bool lclDecodeVmlxBool( std::u16string_view rValue, bool bDefaultForEmpty )
{
    if( rValue.empty() ) return bDefaultForEmpty;
    sal_Int32 nToken = AttributeConversion::decodeToken( rValue );
    // anything else than 't' or 'True' is considered to be false, as specified
    return (nToken == XML_t) || (nToken == XML_True);
}

} // namespace

ShapeLayoutContext::ShapeLayoutContext( ContextHandler2Helper const & rParent, Drawing& rDrawing ) :
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
            OUString aBlockIds = rAttribs.getStringDefaulted( XML_data);
            sal_Int32 nIndex = 0;
            while( nIndex >= 0 )
            {
                std::u16string_view aToken = o3tl::trim(o3tl::getToken(aBlockIds, 0, ' ', nIndex ));
                if( !aToken.empty() )
                    mrDrawing.registerBlockId( o3tl::toInt32(aToken) );
            }
        }
        break;
    }
    return nullptr;
}

ClientDataContext::ClientDataContext( ContextHandler2Helper const & rParent,
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

ShapeContextBase::ShapeContextBase( ContextHandler2Helper const & rParent ) :
    ContextHandler2( rParent )
{
}

ContextHandlerRef ShapeContextBase::createShapeContext( ContextHandler2Helper const & rParent,
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
            if (rAttribs.hasAttribute(XML_path) &&
                    // tdf#122563 skip in the case of empty path
                    !rAttribs.getStringDefaulted(XML_path).isEmpty())
                return new ShapeContext( rParent, rShapes.createShape< BezierShape >(), rAttribs );
            else
                return new ShapeContext( rParent, rShapes.createShape< ComplexShape >(), rAttribs );
        case VML_TOKEN(background):
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

        case W_TOKEN(control):
            return new ControlShapeContext( rParent, rShapes, rAttribs );
    }
    return nullptr;
}

ShapeTypeContext::ShapeTypeContext(ContextHandler2Helper const & rParent,
        std::shared_ptr<ShapeType> const& pShapeType,
        const AttributeList& rAttribs)
    : ShapeContextBase(rParent)
    , m_pShapeType(pShapeType) // tdf#112311 keep it alive
    , mrTypeModel( pShapeType->getTypeModel() )
{
    // shape identifier and shape name
    bool bHasOspid = rAttribs.hasAttribute( O_TOKEN( spid ) );
    mrTypeModel.maShapeId = rAttribs.getXString( bHasOspid ? O_TOKEN( spid ) : XML_id, OUString() );
    mrTypeModel.maLegacyId = rAttribs.getStringDefaulted( XML_id);
    OSL_ENSURE( !mrTypeModel.maShapeId.isEmpty(), "ShapeTypeContext::ShapeTypeContext - missing shape identifier" );
    // builtin shape type identifier
    mrTypeModel.moShapeType = rAttribs.getInteger( O_TOKEN( spt ) );
    // if the o:spid attribute exists, the id attribute contains the user-defined shape name
    if( bHasOspid )
    {
        mrTypeModel.maShapeName = rAttribs.getXString( XML_id, OUString() );
        // get ShapeType and ShapeId from name for compatibility
        static constexpr OUString sShapeTypePrefix = u"shapetype_"_ustr;
        OUString tmp;
        if( mrTypeModel.maShapeName.startsWith( sShapeTypePrefix ) )
        {
            mrTypeModel.maShapeId = mrTypeModel.maShapeName;
            mrTypeModel.moShapeType = o3tl::toInt32(mrTypeModel.maShapeName.subView(sShapeTypePrefix.getLength()));
        }
        else if (mrTypeModel.maShapeName.startsWith("_x0000_t", &tmp))
        {
            mrTypeModel.maShapeId = mrTypeModel.maShapeName;
            mrTypeModel.moShapeType = tmp.toInt32();
        }
    }

    // coordinate system position/size, CSS style
    mrTypeModel.moCoordPos = lclDecodeInt32Pair( rAttribs, XML_coordorigin );
    mrTypeModel.moCoordSize = lclDecodeInt32Pair( rAttribs, XML_coordsize );
    setStyle( rAttribs.getStringDefaulted( XML_style) );
    if( lclDecodeBool( rAttribs, O_TOKEN( hr )).value_or( false ))
    {   // MSO's handling of o:hr width is nowhere near what the spec says:
        // - o:hrpct is not in % but in 0.1%
        // - if o:hrpct is not given, 100% width is assumed
        // - given width is used only if explicit o:hrpct="0" is given
        OUString hrpct = rAttribs.getString( O_TOKEN( hrpct ), u"1000"_ustr );
        if( hrpct != "0" )
            mrTypeModel.maWidthPercent = OUString::number( hrpct.toInt32() );
        mrTypeModel.maWrapDistanceLeft = "0";
        mrTypeModel.maWrapDistanceRight = "0";
        mrTypeModel.maPositionHorizontal = rAttribs.getString( O_TOKEN( hralign ), u"left"_ustr );
        mrTypeModel.moWrapType = "topAndBottom";
    }

    // stroke settings (may be overridden by v:stroke element later)
    mrTypeModel.maStrokeModel.moStroked = lclDecodeBool( rAttribs, XML_stroked );
    mrTypeModel.maStrokeModel.moColor = rAttribs.getString( XML_strokecolor );
    mrTypeModel.maStrokeModel.moWeight = rAttribs.getString( XML_strokeweight );

    // fill settings (may be overridden by v:fill element later)
    mrTypeModel.maFillModel.moFilled = lclDecodeBool( rAttribs, XML_filled );
    mrTypeModel.maFillModel.moColor = rAttribs.getString( XML_fillcolor );

    // For roundrect we may have an arcsize attribute to read
    mrTypeModel.maArcsize = rAttribs.getStringDefaulted(XML_arcsize);
    // editas
    mrTypeModel.maEditAs = rAttribs.getStringDefaulted(XML_editas);

    mrTypeModel.maAdjustments = rAttribs.getStringDefaulted(XML_adj);
}

ContextHandlerRef ShapeTypeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( stroke ):
            assignIfUsed( mrTypeModel.maStrokeModel.moStroked, lclDecodeBool( rAttribs, XML_on ) );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowType = rAttribs.getToken( XML_startarrow );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowWidth = rAttribs.getToken( XML_startarrowwidth );
            mrTypeModel.maStrokeModel.maStartArrow.moArrowLength = rAttribs.getToken( XML_startarrowlength );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowType = rAttribs.getToken( XML_endarrow );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowWidth = rAttribs.getToken( XML_endarrowwidth );
            mrTypeModel.maStrokeModel.maEndArrow.moArrowLength = rAttribs.getToken( XML_endarrowlength );
            assignIfUsed( mrTypeModel.maStrokeModel.moColor, rAttribs.getString( XML_color ) );
            mrTypeModel.maStrokeModel.moOpacity = lclDecodeOpacity( rAttribs, XML_opacity, 1.0 );
            assignIfUsed( mrTypeModel.maStrokeModel.moWeight, rAttribs.getString( XML_weight ) );
            mrTypeModel.maStrokeModel.moDashStyle = rAttribs.getString( XML_dashstyle );
            mrTypeModel.maStrokeModel.moLineStyle = rAttribs.getToken( XML_linestyle );
            mrTypeModel.maStrokeModel.moEndCap = rAttribs.getToken( XML_endcap );
            mrTypeModel.maStrokeModel.moJoinStyle = rAttribs.getToken( XML_joinstyle );
        break;
        case VML_TOKEN( fill ):
        {
            // in DOCX shapes use r:id for the relationship id
            // in XLSX they use o:relid
            bool bHasORelId = rAttribs.hasAttribute( O_TOKEN(relid) );
            assignIfUsed( mrTypeModel.maFillModel.moFilled, lclDecodeBool( rAttribs, XML_on ) );
            assignIfUsed( mrTypeModel.maFillModel.moColor, rAttribs.getString( XML_color ) );
            mrTypeModel.maFillModel.moOpacity = lclDecodeOpacity( rAttribs, XML_opacity, 1.0 );
            mrTypeModel.maFillModel.moColor2 = rAttribs.getString( XML_color2 );
            mrTypeModel.maFillModel.moOpacity2 = lclDecodeOpacity( rAttribs, XML_opacity2, 1.0 );
            mrTypeModel.maFillModel.moType = rAttribs.getToken( XML_type );
            mrTypeModel.maFillModel.moAngle = rAttribs.getInteger( XML_angle );
            mrTypeModel.maFillModel.moFocus = lclDecodePercent( rAttribs, XML_focus, 0.0 );
            mrTypeModel.maFillModel.moFocusPos = lclDecodePercentPair( rAttribs, XML_focusposition );
            mrTypeModel.maFillModel.moFocusSize = lclDecodePercentPair( rAttribs, XML_focussize );
            mrTypeModel.maFillModel.moBitmapPath = decodeFragmentPath( rAttribs, bHasORelId ? O_TOKEN(relid) : R_TOKEN(id) );
            mrTypeModel.maFillModel.moRotate = lclDecodeBool( rAttribs, XML_rotate );
            break;
        }
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

            // Gain / contrast.
            std::optional<OUString> oGain = rAttribs.getString(XML_gain);
            sal_Int32 nGain = 0x10000;
            if (oGain.has_value() && oGain.value().endsWith("f"))
            {
                nGain = oGain.value().toInt32();
            }
            if (nGain < 0x10000)
            {
                nGain *= 101; // 100 + 1 to round
                nGain /= 0x10000;
                nGain -= 100;
            }
            mrTypeModel.mnGain = nGain;

            // Blacklevel / brightness.
            std::optional<OUString> oBlacklevel = rAttribs.getString(XML_blacklevel);
            sal_Int16 nBlacklevel = 0;
            if (oBlacklevel.has_value() && oBlacklevel.value().endsWith("f"))
            {
                nBlacklevel = oBlacklevel.value().toInt32();
            }
            if (nBlacklevel != 0)
            {
                nBlacklevel /= 327;
            }
            mrTypeModel.mnBlacklevel = nBlacklevel;
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
            mrTypeModel.maShadowModel.moShadowOn = lclDecodeBool(rAttribs, XML_on).value_or(false);
            assignIfUsed(mrTypeModel.maShadowModel.moColor, rAttribs.getString(XML_color));
            assignIfUsed(mrTypeModel.maShadowModel.moOffset, rAttribs.getString(XML_offset));
            mrTypeModel.maShadowModel.moOpacity = lclDecodePercent(rAttribs, XML_opacity, 1.0);
        }
        break;
        case VML_TOKEN( textpath ):
            assignIfUsed(mrTypeModel.maTextpathModel.moString, rAttribs.getString(XML_string));
            assignIfUsed(mrTypeModel.maTextpathModel.moStyle, rAttribs.getString(XML_style));
            assignIfUsed(mrTypeModel.maTextpathModel.moTrim, lclDecodeBool(rAttribs, XML_trim));
        break;
    }
    return nullptr;
}

std::optional< OUString > ShapeTypeContext::decodeFragmentPath( const AttributeList& rAttribs, sal_Int32 nToken ) const
{
    std::optional< OUString > oFragmentPath;
    std::optional< OUString > oRelId = rAttribs.getString( nToken );
    if( oRelId.has_value() )
        oFragmentPath = getFragmentPathFromRelId( oRelId.value() );
    return oFragmentPath;
}

void ShapeTypeContext::setStyle( std::u16string_view rStyle )
{
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        std::u16string_view aName, aValue;
        if( ConversionHelper::separatePair( aName, aValue, o3tl::getToken(rStyle, 0, ';', nIndex ), ':' ) )
        {
            if( aName == u"position" )      mrTypeModel.maPosition = aValue;
            else if( aName == u"z-index" )        mrTypeModel.maZIndex = aValue;
            else if( aName == u"left" )           mrTypeModel.maLeft = aValue;
            else if( aName == u"top" )            mrTypeModel.maTop = aValue;
            else if( aName == u"width" )          mrTypeModel.maWidth = aValue;
            else if( aName == u"height" )         mrTypeModel.maHeight = aValue;
            else if( aName == u"margin-left" )    mrTypeModel.maMarginLeft = aValue;
            else if( aName == u"margin-top" )     mrTypeModel.maMarginTop = aValue;
            else if( aName == u"mso-position-vertical-relative" )  mrTypeModel.maPositionVerticalRelative = aValue;
            else if( aName == u"mso-position-horizontal-relative" )  mrTypeModel.maPositionHorizontalRelative = aValue;
            else if( aName == u"mso-position-horizontal" ) mrTypeModel.maPositionHorizontal = aValue;
            else if( aName == u"mso-position-vertical" ) mrTypeModel.maPositionVertical = aValue;
            else if( aName == u"mso-width-percent" ) mrTypeModel.maWidthPercent = aValue;
            else if( aName == u"mso-width-relative" ) mrTypeModel.maWidthRelative = aValue;
            else if( aName == u"mso-height-percent" ) mrTypeModel.maHeightPercent = aValue;
            else if( aName == u"mso-height-relative" ) mrTypeModel.maHeightRelative = aValue;
            else if( aName == u"mso-fit-shape-to-text" )           mrTypeModel.mbAutoHeight = true;
            else if( aName == u"rotation" )       mrTypeModel.maRotation = aValue;
            else if( aName == u"flip" )       mrTypeModel.maFlip = aValue;
            else if( aName == u"visibility" )
                mrTypeModel.mbVisible = aValue != u"hidden";
            else if( aName == u"mso-wrap-style" ) mrTypeModel.maWrapStyle = aValue;
            else if ( aName == u"v-text-anchor" ) mrTypeModel.maVTextAnchor = aValue;
            else if ( aName == u"mso-wrap-distance-left" ) mrTypeModel.maWrapDistanceLeft = aValue;
            else if ( aName == u"mso-wrap-distance-right" ) mrTypeModel.maWrapDistanceRight = aValue;
            else if ( aName == u"mso-wrap-distance-top" ) mrTypeModel.maWrapDistanceTop = aValue;
            else if ( aName == u"mso-wrap-distance-bottom" ) mrTypeModel.maWrapDistanceBottom = aValue;
        }
    }
}

ShapeContext::ShapeContext(ContextHandler2Helper const& rParent,
                           const std::shared_ptr<ShapeBase>& pShape, const AttributeList& rAttribs)
    : ShapeTypeContext(rParent, pShape, rAttribs)
    , mrShape(*pShape)
    , mrShapeModel(pShape->getShapeModel())
{
    // collect shape specific attributes
    mrShapeModel.maType = rAttribs.getXString( XML_type, OUString() );
    // polyline path
    setPoints( rAttribs.getStringDefaulted( XML_points) );
    // line start and end positions
    setFrom(rAttribs.getStringDefaulted(XML_from));
    setTo(rAttribs.getStringDefaulted(XML_to));
    setControl1(rAttribs.getStringDefaulted(XML_control1));
    setControl2(rAttribs.getStringDefaulted(XML_control2));
    setVmlPath(rAttribs.getStringDefaulted(XML_path));
    setHyperlink(rAttribs.getStringDefaulted(XML_href));
}

ContextHandlerRef ShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // Excel specific shape client data
    if( isRootElement() ) switch( nElement )
    {
        case VML_TOKEN( textbox ):
        {
            // Calculate the shape type: map both <rect> and <v:shape> with a textbox shape type to
            // a TextShape.
            sal_Int32 nShapeType = 0;
            if (ShapeContainer* pShapeContainer = mrShape.getContainer())
            {
                OUString aType = mrShapeModel.maType;
                if (!aType.isEmpty() && aType[0] == '#')
                {
                    aType = aType.copy(1);
                }
                if (const ShapeType* pShapeType = pShapeContainer->getShapeTypeById(aType))
                {
                    nShapeType = pShapeType->getTypeModel().moShapeType.value();
                }
            }
            mrShapeModel.mbInGroup = (getParentElement() == VML_TOKEN(group));

            // FIXME: the shape with textbox should be used for the next cases
            if (getCurrentElement() == VML_TOKEN(rect) || nShapeType == ESCHER_ShpInst_TextBox)
            {
                if (mrShapeModel.mbInGroup)
                    // FIXME: without this a text will be added into the group-shape instead of its
                    // parent shape
                    dynamic_cast<SimpleShape&>(mrShape).setService(u"com.sun.star.drawing.TextShape"_ustr);
                else
                    // FIXME: without this we does not handle some properties like shadow
                    dynamic_cast<SimpleShape&>(mrShape).setService(u"com.sun.star.text.TextFrame"_ustr);
            }
            return new TextBoxContext( *this, mrShapeModel.createTextBox(mrShape.getTypeModel()), rAttribs,
                mrShape.getDrawing().getFilter().getGraphicHelper());
        }
        case VMLX_TOKEN( ClientData ):
            // tdf#41466 ActiveX control shapes with a textbox are transformed into a frame
            // (see unit test testActiveXOptionButtonGroup)
            dynamic_cast<SimpleShape&>(mrShape).setService(u"com.sun.star.text.TextFrame"_ustr);
            return new ClientDataContext( *this, mrShapeModel.createClientData(), rAttribs );
        case VMLPPT_TOKEN( textdata ):
            // Force RectangleShape, this is ugly :(
            // and is there because of the lines above which change it to TextFrame
            dynamic_cast< SimpleShape& >( mrShape ).setService(
                    u"com.sun.star.drawing.RectangleShape"_ustr);
            mrShapeModel.maLegacyDiagramPath = getFragmentPathFromRelId(rAttribs.getStringDefaulted(XML_id));
            break;
        case O_TOKEN( signatureline ):
            mrShapeModel.mbIsSignatureLine = true;
            mrShapeModel.maSignatureId = rAttribs.getStringDefaulted(XML_id);
            mrShapeModel.maSignatureLineSuggestedSignerName
                = rAttribs.getStringDefaulted(O_TOKEN(suggestedsigner));
            mrShapeModel.maSignatureLineSuggestedSignerTitle
                = rAttribs.getStringDefaulted(O_TOKEN(suggestedsigner2));
            mrShapeModel.maSignatureLineSuggestedSignerEmail
                = rAttribs.getStringDefaulted(O_TOKEN(suggestedsigneremail));
            mrShapeModel.maSignatureLineSigningInstructions
                = rAttribs.getStringDefaulted(O_TOKEN(signinginstructions));
            mrShapeModel.mbSignatureLineShowSignDate = ConversionHelper::decodeBool(
                rAttribs.getString(XML_showsigndate, u"t"_ustr)); // default is true
            mrShapeModel.mbSignatureLineCanAddComment = ConversionHelper::decodeBool(
                rAttribs.getString(XML_allowcomments, u"f"_ustr)); // default is false
            break;
        case O_TOKEN( lock ):
            // TODO
            break;
    }
    // handle remaining stuff in base class
    return ShapeTypeContext::onCreateContext( nElement, rAttribs );
}

void ShapeContext::setPoints(std::u16string_view rPoints)
{
    mrShapeModel.maPoints.clear();
    sal_Int32 nIndex = 0;

    while (nIndex >= 0)
    {
        sal_Int32 nX = ConversionHelper::decodeMeasureToTwip(
            mrShape.getDrawing().getFilter().getGraphicHelper(), o3tl::getToken(rPoints, 0, ',', nIndex),
            0, true, true);
        sal_Int32 nY = ConversionHelper::decodeMeasureToTwip(
            mrShape.getDrawing().getFilter().getGraphicHelper(), o3tl::getToken(rPoints, 0, ',', nIndex),
            0, false, true);
        mrShapeModel.maPoints.emplace_back(nX, nY);
    }
    // VML polyline has no size in its style attribute. Word writes the size to attribute
    // coordsize with values in twip but without unit. For others we get size from points.
    if (!mrShape.getTypeModel().maWidth.isEmpty() || !mrShape.getTypeModel().maHeight.isEmpty())
        return;

    if (mrShape.getTypeModel().moCoordSize.has_value())
    {
        double fWidth = mrShape.getTypeModel().moCoordSize.value().first;
        fWidth = o3tl::convert(fWidth, o3tl::Length::twip, o3tl::Length::pt);
        double fHeight = mrShape.getTypeModel().moCoordSize.value().second;
        fHeight = o3tl::convert(fHeight, o3tl::Length::twip, o3tl::Length::pt);
        mrShape.getTypeModel().maWidth = OUString::number(fWidth) + "pt";
        mrShape.getTypeModel().maHeight = OUString::number(fHeight) + "pt";
    }
    else if (mrShapeModel.maPoints.size())
    {
        double fMinX = mrShapeModel.maPoints[0].X;
        double fMaxX = mrShapeModel.maPoints[0].X;
        double fMinY = mrShapeModel.maPoints[0].Y;
        double fMaxY = mrShapeModel.maPoints[0].Y;
        for (const auto& rPoint : mrShapeModel.maPoints)
        {
            if (rPoint.X < fMinX)
                fMinX = rPoint.X;
            else if (rPoint.X > fMaxX)
                fMaxX = rPoint.X;
            if (rPoint.Y < fMinY)
                fMinY = rPoint.Y;
            else if (rPoint.Y > fMaxY)
                fMaxY = rPoint.Y;
        }
        mrShape.getTypeModel().maWidth
            = OUString::number(
                  o3tl::convert(fMaxX - fMinX, o3tl::Length::twip, o3tl::Length::pt))
              + "pt";
        mrShape.getTypeModel().maHeight
            = OUString::number(
                  o3tl::convert(fMaxY - fMinY, o3tl::Length::twip, o3tl::Length::pt))
              + "pt";
        // Set moCoordSize, otherwise default (1000,1000) is used.
        mrShape.getTypeModel().moCoordSize =
            Int32Pair(basegfx::fround(fMaxX - fMinX), basegfx::fround(fMaxY - fMinY));
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

void ShapeContext::setHyperlink( const OUString& rHyperlink )
{
    if (!rHyperlink.isEmpty())
        mrShapeModel.maHyperlink = rHyperlink;
}

GroupShapeContext::GroupShapeContext(ContextHandler2Helper const& rParent,
                                     const std::shared_ptr<GroupShape>& pShape,
                                     const AttributeList& rAttribs)
    : ShapeContext(rParent, pShape, rAttribs)
    , mrShapes(pShape->getChildren())
{
}

ContextHandlerRef GroupShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // try to create a context of an embedded shape
    ContextHandlerRef xContext = createShapeContext( *this, mrShapes, nElement, rAttribs );
    // handle remaining stuff of this shape in base class
    return xContext ? xContext : ShapeContext::onCreateContext( nElement, rAttribs );
}

RectangleShapeContext::RectangleShapeContext(ContextHandler2Helper const& rParent,
                                             const AttributeList& rAttribs,
                                             const std::shared_ptr<RectangleShape>& pShape)
    : ShapeContext(rParent, pShape, rAttribs)
{
}

ContextHandlerRef RectangleShapeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // The parent class's context is fine
    return ShapeContext::onCreateContext( nElement, rAttribs );
}

ControlShapeContext::ControlShapeContext( ::oox::core::ContextHandler2Helper const & rParent, ShapeContainer& rShapes, const AttributeList& rAttribs )
    : ShapeContextBase (rParent)
{
    ::oox::vml::ControlInfo aInfo;
    aInfo.maShapeId = rAttribs.getXString( W_TOKEN( shapeid ), OUString() );
    aInfo.maFragmentPath = getFragmentPathFromRelId(rAttribs.getStringDefaulted( R_TOKEN(id)));
    aInfo.maName = rAttribs.getStringDefaulted( W_TOKEN( name ));
    aInfo.mbTextContentShape = true;
    rShapes.getDrawing().registerControl(aInfo);
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
