/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillpropertiesgroupcontext.cxx,v $
 * $Revision: 1.8.6.1 $
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

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandler;
using ::oox::core::XmlFilterBase;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// ---------------------------------------------------------------------

class NoFillContext : public FillPropertiesGroupContext
{
public:
    NoFillContext( ContextHandler& rParent, FillProperties& rFillProperties ) throw();
};

// ---------------------------------------------------------------------

class SolidFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    SolidFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class GradFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    GradFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class PattFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    PattFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL startFastElement( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class GrpFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    GrpFillPropertiesContext( ContextHandler& rParent, const Reference< ::XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL startFastElement( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class clrChangeContext : public ContextHandler
{
public:
    clrChangeContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttributes,
        Color& rClrFrom, Color& rClrTo ) throw();
    ~clrChangeContext();
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);

private:
    Color&              mrClrFrom;
    Color&              mrClrTo;
    bool                mbUseAlpha;
};

// ---------------------------------------------------------------------

FillPropertiesContext::FillPropertiesContext( ContextHandler& rParent, FillProperties& rFillProperties ) throw()
: ContextHandler( rParent )
, mrFillProperties( rFillProperties )
{
}

Reference< XFastContextHandler > FillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw ( SAXException, RuntimeException )
{
    return FillPropertiesGroupContext::StaticCreateContext( *this, aElementToken, xAttribs, mrFillProperties );
}

// ---------------------------------------------------------------------

FillPropertiesGroupContext::FillPropertiesGroupContext( ContextHandler& rParent, FillProperties& rFillProperties, sal_Int32 nContext ) throw()
: ContextHandler( rParent )
, mrFillProperties( rFillProperties )
{
   mrFillProperties.moFillType = nContext;
}

// ---------------------------------------------------------------------

Reference< XFastContextHandler > FillPropertiesGroupContext::StaticCreateContext( ContextHandler& rParent, sal_Int32 aElementToken,
    const Reference< XFastAttributeList >& xAttribs, FillProperties& rFillProperties )
        throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_noFill:     // CT_NoFillProperties
        xRet.set( new NoFillContext( rParent, rFillProperties ) );
        break;
    case NMSP_DRAWINGML|XML_solidFill:  // CT_SolidFillProperties
        xRet.set( new SolidFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
        break;
    case NMSP_DRAWINGML|XML_gradFill:   // CT_GradientFillProperties
        xRet.set( new GradFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
        break;
    case NMSP_DRAWINGML|XML_blipFill:   // CT_BlipFillProperties
        xRet.set( new BlipFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
        break;
    case NMSP_DRAWINGML|XML_pattFill:   // CT_PatternFillProperties
        xRet.set( new PattFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
        break;
    case NMSP_DRAWINGML|XML_grpFill:    // CT_GroupFillProperties
        xRet.set( new GrpFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
        break;
    }
    return xRet;
}

// ---------------------------------------------------------------------

NoFillContext::NoFillContext( ContextHandler& rParent, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_noFill )
{
}

// ---------------------------------------------------------------------

SolidFillPropertiesContext::SolidFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >&, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_solidFill )
{
}

Reference< XFastContextHandler > SolidFillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_scrgbClr:   // CT_ScRgbColor
        case NMSP_DRAWINGML|XML_srgbClr:    // CT_SRgbColor
        case NMSP_DRAWINGML|XML_hslClr: // CT_HslColor
        case NMSP_DRAWINGML|XML_sysClr: // CT_SystemColor
        case NMSP_DRAWINGML|XML_schemeClr:  // CT_SchemeColor
        case NMSP_DRAWINGML|XML_prstClr:    // CT_PresetColor
            xRet.set( new colorChoiceContext( *this, mrFillProperties.maFillColor ) );
        break;
    }
    return xRet;
}

// ---------------------------------------------------------------------

GradFillPropertiesContext::GradFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
                                                        FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_gradFill )
{
    AttributeList aAttribs( xAttribs );
    rFillProperties.moFlipModeToken = aAttribs.getToken( XML_flip );
    rFillProperties.moRotateWithShape = aAttribs.getBool( XML_rotWithShape );
}

void GradFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > GradFillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_gsLst:          // CT_GradientStopList
        {
            xRet = this;
        }
    break;
    case NMSP_DRAWINGML|XML_gs:
        {
            if( aAttribs.hasAttribute( XML_pos ) )
            {
                double fPosition = GetPositiveFixedPercentage( xAttribs->getOptionalValue( XML_pos ) );
                xRet.set( new colorChoiceContext( *this, mrFillProperties.maGradientStops[ fPosition ] ) );
            }
        }
        break;

    // EG_ShadeProperties
        case NMSP_DRAWINGML|XML_lin:        // CT_LinearShadeProperties
            mrFillProperties.moShadeAngle = aAttribs.getInteger( XML_ang );
            mrFillProperties.moShadeScaled = aAttribs.getBool( XML_scaled );
        break;

        case NMSP_DRAWINGML|XML_path:           // CT_PathShadeProperties
            // always set a path type, this disables linear gradient in conversion
            mrFillProperties.moGradientPath = aAttribs.getToken( XML_path, XML_rect );
            xRet = this;
        break;

        case NMSP_DRAWINGML|XML_fillToRect:     // CT_RelativeRect
            mrFillProperties.moFillToRect = GetRelativeRect( xAttribs );
        break;

        case NMSP_DRAWINGML|XML_tileRect:           // CT_RelativeRect
            mrFillProperties.moTileRect = GetRelativeRect( xAttribs );
        break;

        default:;
    }
    return xRet;
}

// ---------------------------------------------------------------------
// CT_BlipFill
// ---------------------------------------------------------------------

BlipFillPropertiesContext::BlipFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
                                                        FillProperties& rFillProperties )
        throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_blipFill )
{
    /* todo
    if( xAttribs->hasAttribute( XML_dpi ) )
    {
         xsd:unsignedInt
        DPI (dots per inch) used to calculate the size of the blip. If not present or zero,
                    the DPI in the blip is used.

    }
    */
    AttributeList aAttribs( xAttribs );
    rFillProperties.moRotateWithShape = aAttribs.getBool( XML_rotWithShape );
}

void BlipFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
    if( msEmbed.getLength() )
    {
        const OUString aFragmentPath = getFragmentPathFromRelId( msEmbed );
        if( aFragmentPath.getLength() > 0 ) try
        {
            // get the input stream for the fill bitmap
            XmlFilterBase& rFilter = getFilter();
            Reference< io::XInputStream > xInputStream( rFilter.openInputStream( aFragmentPath ), UNO_QUERY_THROW );

            // load the fill bitmap into an XGraphic with the GraphicProvider
            static const OUString sGraphicProvider = CREATE_OUSTRING( "com.sun.star.graphic.GraphicProvider" );
            Reference< graphic::XGraphicProvider > xGraphicProvider( rFilter.getGlobalFactory()->createInstance( sGraphicProvider ), UNO_QUERY_THROW );

            static const OUString sInputStream = CREATE_OUSTRING( "InputStream" );
            beans::PropertyValues aMediaProperties(1);
            aMediaProperties[0].Name = sInputStream;
            aMediaProperties[0].Value <<= xInputStream;

            mrFillProperties.mxGraphic = xGraphicProvider->queryGraphic( aMediaProperties );
        }
        catch( Exception& )
        {
            OSL_ENSURE( false,
                (rtl::OString("oox::drawingml::BlipFillPropertiesContext::EndElement(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );

        }
    }
}

Reference< XFastContextHandler > BlipFillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_blip:           // CT_Blip
        msEmbed = xAttribs->getOptionalValue(NMSP_RELATIONSHIPS|XML_embed); // relationship identifer for embedded blobs
        msLink = xAttribs->getOptionalValue(NMSP_RELATIONSHIPS|XML_link);   // relationship identifer for linked blobs
        break;
        case NMSP_DRAWINGML|XML_alphaBiLevel:
        case NMSP_DRAWINGML|XML_alphaCeiling:
        case NMSP_DRAWINGML|XML_alphaFloor:
        case NMSP_DRAWINGML|XML_alphaInv:
        case NMSP_DRAWINGML|XML_alphaMod:
        case NMSP_DRAWINGML|XML_alphaModFix:
        case NMSP_DRAWINGML|XML_alphaRepl:
        case NMSP_DRAWINGML|XML_biLevel:
        case NMSP_DRAWINGML|XML_blur:
        break;
        case NMSP_DRAWINGML|XML_clrChange:
            {
                xRet = new clrChangeContext( *this, xAttribs, mrFillProperties.maColorChangeFrom, mrFillProperties.maColorChangeTo );
            }
            break;
        case NMSP_DRAWINGML|XML_clrRepl:
        case NMSP_DRAWINGML|XML_duotone:
        case NMSP_DRAWINGML|XML_extLst:
        case NMSP_DRAWINGML|XML_fillOverlay:
        case NMSP_DRAWINGML|XML_grayscl:
        case NMSP_DRAWINGML|XML_hsl:
        case NMSP_DRAWINGML|XML_lum:
        case NMSP_DRAWINGML|XML_tint:
        break;

    case NMSP_DRAWINGML|XML_srcRect:                    // CT_RelativeRect
// todo     maSrcRect = GetRelativeRect( xAttribs );
        break;
    case NMSP_DRAWINGML|XML_tile:                       // CT_TileInfo
        {
        mrFillProperties.moBitmapMode = getToken( aElementToken );

        AttributeList aAttribs( xAttribs );
        mrFillProperties.moFlipModeToken = aAttribs.getToken( XML_flip );
        mrFillProperties.moTileX = aAttribs.getInteger( XML_tx );
        mrFillProperties.moTileY = aAttribs.getInteger( XML_ty );
        mrFillProperties.moTileSX = aAttribs.getInteger( XML_sx );
        mrFillProperties.moTileSY = aAttribs.getInteger( XML_sy );
        mrFillProperties.moTileAlign = aAttribs.getToken( XML_algn );
        }
        break;
    case NMSP_DRAWINGML|XML_stretch:        // CT_StretchInfo
        mrFillProperties.moBitmapMode = getToken( aElementToken );
        break;
        case NMSP_DRAWINGML|XML_fillRect:
    // todo     maFillRect = GetRelativeRect( xAttribs );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

PattFillPropertiesContext::PattFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& rxAttribs, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_pattFill )
{
    AttributeList aAttribs( rxAttribs );
    mrFillProperties.moPattPreset = aAttribs.getToken( XML_prst );
}

void PattFillPropertiesContext::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
}

void PattFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > PattFillPropertiesContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case NMSP_DRAWINGML|XML_bgClr:
            return new colorChoiceContext( *this, mrFillProperties.maPattBgColor );
        case NMSP_DRAWINGML|XML_fgClr:
            return new colorChoiceContext( *this, mrFillProperties.maPattFgColor );
    }
    return 0;
}

// ---------------------------------------------------------------------

GrpFillPropertiesContext::GrpFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >&, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, rFillProperties, XML_grpFill )
{
}

void GrpFillPropertiesContext::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
}

void GrpFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > GrpFillPropertiesContext::createFastChildContext( sal_Int32, const Reference< XFastAttributeList >& )
    throw (SAXException, RuntimeException)
{
    return this;
}


// ---------------------------------------------------------------------

clrChangeContext::clrChangeContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttributes,
    Color& rClrFrom, Color& rClrTo ) throw()
: ContextHandler( rParent )
, mrClrFrom( rClrFrom )
, mrClrTo( rClrTo )
{
    mrClrFrom.setUnused();
    mrClrTo.setUnused();
    AttributeList aAttribs( xAttributes );
    mbUseAlpha = aAttribs.getBool( XML_useA, true );
}

clrChangeContext::~clrChangeContext()
{
    if ( !mbUseAlpha )
        mrClrTo.clearTransparence();
}

Reference< XFastContextHandler > clrChangeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_clrFrom:    // CT_Color
            xRet.set( new colorChoiceContext( *this, mrClrFrom ) );
            break;
        case NMSP_DRAWINGML|XML_clrTo:      // CT_Color
            xRet.set( new colorChoiceContext( *this, mrClrTo ) );
            break;
        default:
        break;
    }
    return xRet;
}

} }
