/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillpropertiesgroupcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:21:36 $
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

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using ::com::sun::star::beans::NamedValue;
using namespace ::oox::core;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// ---------------------------------------------------------------------

class NoFillContext : public FillPropertiesGroupContext
{
public:
    NoFillContext( ContextHandler& rParent, FillProperties& rFillProperties ) throw();
};

// ---------------------------------------------------------------------

class SolidColorFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    SolidColorFillPropertiesContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class GradFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    GradFillPropertiesContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class PattFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    PattFillPropertiesContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL startFastElement( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class GrpFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    GrpFillPropertiesContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL startFastElement( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

class clrChangeContext : public ContextHandler
{
    ColorPtr& mraClrFrom;
    ColorPtr& mraClrTo;
    sal_Bool mbUseAlpha;
public:
    clrChangeContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes,
        ColorPtr& raClrFrom, ColorPtr& raClrTo ) throw();
    ~clrChangeContext();
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};

// ---------------------------------------------------------------------

FillPropertiesGroupContext::FillPropertiesGroupContext( ContextHandler& rParent, ::com::sun::star::drawing::FillStyle eFillStyle, FillProperties& rFillProperties ) throw()
: ContextHandler( rParent )
, mrFillProperties( rFillProperties )
{
    rFillProperties.getFillStyle() = ::boost::optional< ::com::sun::star::drawing::FillStyle >( eFillStyle );
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
        xRet.set( new SolidColorFillPropertiesContext( rParent, xAttribs, rFillProperties ) );
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
: FillPropertiesGroupContext( rParent, FillStyle_NONE, rFillProperties )
{
}

// ---------------------------------------------------------------------

SolidColorFillPropertiesContext::SolidColorFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >&, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, FillStyle_SOLID, rFillProperties )
{
}
Reference< XFastContextHandler > SolidColorFillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    // colorTransformGroup

    // color should be available as rgb in member mnColor already, now modify it depending on
    // the transformation elements

    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {

    case NMSP_DRAWINGML|XML_scrgbClr:   // CT_ScRgbColor
    case NMSP_DRAWINGML|XML_srgbClr:    // CT_SRgbColor
    case NMSP_DRAWINGML|XML_hslClr: // CT_HslColor
    case NMSP_DRAWINGML|XML_sysClr: // CT_SystemColor
    case NMSP_DRAWINGML|XML_schemeClr:  // CT_SchemeColor
    case NMSP_DRAWINGML|XML_prstClr:    // CT_PresetColor
        {
            xRet.set( new colorChoiceContext( *this, *mrFillProperties.getFillColor() ) );
            break;
        }
    case NMSP_DRAWINGML|XML_tint:       // CT_PositiveFixedPercentage
    case NMSP_DRAWINGML|XML_shade:      // CT_PositiveFixedPercentage
    case NMSP_DRAWINGML|XML_comp:       // CT_ComplementTransform
    case NMSP_DRAWINGML|XML_inv:        // CT_InverseTransform
    case NMSP_DRAWINGML|XML_gray:       // CT_GrayscaleTransform
    case NMSP_DRAWINGML|XML_alpha:      // CT_PositiveFixedPercentage
    case NMSP_DRAWINGML|XML_alphaOff:   // CT_FixedPercentage
    case NMSP_DRAWINGML|XML_alphaMod:   // CT_PositivePercentage
    case NMSP_DRAWINGML|XML_hue:        // CT_PositiveFixedAngle
    case NMSP_DRAWINGML|XML_hueOff: // CT_Angle
    case NMSP_DRAWINGML|XML_hueMod: // CT_PositivePercentage
    case NMSP_DRAWINGML|XML_sat:        // CT_Percentage
    case NMSP_DRAWINGML|XML_satOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_satMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_lum:        // CT_Percentage
    case NMSP_DRAWINGML|XML_lumOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_lumMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_red:        // CT_Percentage
    case NMSP_DRAWINGML|XML_redOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_redMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_green:      // CT_Percentage
    case NMSP_DRAWINGML|XML_greenOff:   // CT_Percentage
    case NMSP_DRAWINGML|XML_greenMod:   // CT_Percentage
    case NMSP_DRAWINGML|XML_blue:       // CT_Percentage
    case NMSP_DRAWINGML|XML_blueOff:    // CT_Percentage
    case NMSP_DRAWINGML|XML_blueMod:    // CT_Percentage
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

class GradientStopContext : public ::oox::core::ContextHandler
{
    GradientStop& mrGradientStop;
public:
    GradientStopContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, GradientStop& rGradientStop ) throw();
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
};
GradientStopContext::GradientStopContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
                                                        GradientStop& rGradientStop ) throw()
: ContextHandler( rParent )
, mrGradientStop( rGradientStop )
{
    if ( xAttribs->hasAttribute( XML_pos ) )
        mrGradientStop.mnPosition = GetPositiveFixedPercentage( xAttribs->getOptionalValue( XML_pos ) );
}
Reference< XFastContextHandler > GradientStopContext::createFastChildContext( sal_Int32 /* aElementToken */, const Reference< XFastAttributeList >& /* xAttribs */ ) throw (SAXException, RuntimeException)
{
    return new colorChoiceContext( *this, *(mrGradientStop.maColor) );
}

GradFillPropertiesContext::GradFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
                                                        FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, FillStyle_GRADIENT, rFillProperties )
{
    if( xAttribs->hasAttribute( XML_flip ) )            // ST_TileFlipMode
        rFillProperties.getFlipModeToken() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_flip, XML_none ) );

    if( xAttribs->hasAttribute( XML_rotWithShape ) )    // xsd:boolean
    {
        AttributeList aAttributeList( xAttribs );
        rFillProperties.getRotateWithShape() = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_rotWithShape, sal_False ) );
    }
}

void GradFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > GradFillPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_gsLst:          // CT_GradientStopList
        {
            xRet = this;
        }
    break;
    case NMSP_DRAWINGML|XML_gs:
        {
            std::vector< GradientStop >& rGradientStops = mrFillProperties.getGradientStops();
            GradientStop aGradientStop;
            rGradientStops.push_back( aGradientStop );
            xRet.set( new GradientStopContext( *this, xAttribs, rGradientStops.back() ) );
        }
        break;

    // EG_ShadeProperties
        case NMSP_DRAWINGML|XML_lin:        // CT_LinearShadeProperties
        {
            if ( xAttribs->hasAttribute( XML_ang ) )
            {
                AttributeList aAttributeList( xAttribs );
                mrFillProperties.getShadeAngle() = ::boost::optional< sal_Int32 >( aAttributeList.getInteger( XML_ang, 0 ) );
            }
            if ( xAttribs->hasAttribute( XML_scaled ) )
            {
                AttributeList aAttributeList( xAttribs );
                mrFillProperties.getShadeScaled() = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_scaled, sal_False ) );
            }
        }
        break;

        case NMSP_DRAWINGML|XML_path:           // CT_PathShadeProperties
        {
            if ( xAttribs->hasAttribute( XML_path ) )
                mrFillProperties.getShadeTypeToken() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_path, XML_rect ) );
            xRet = this;
        }
        break;
        case NMSP_DRAWINGML|XML_fillToRect:     // CT_RelativeRect
        {
            mrFillProperties.getFillToRect() = boost::optional< ::com::sun::star::geometry::IntegerRectangle2D >( GetRelativeRect( xAttribs ) );
        }
        break;

    case NMSP_DRAWINGML|XML_tileRect:           // CT_RelativeRect
    {
        mrFillProperties.getTileRect() = boost::optional< ::com::sun::star::geometry::IntegerRectangle2D >( GetRelativeRect( xAttribs ) );
    }
    break;
    default:
    break;
    }
    return xRet;
}

// ---------------------------------------------------------------------
// CT_BlipFill
// ---------------------------------------------------------------------

BlipFillPropertiesContext::BlipFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
                                                        FillProperties& rFillProperties )
        throw()
: FillPropertiesGroupContext( rParent, FillStyle_BITMAP, rFillProperties )
, meBitmapMode( BitmapMode_NO_REPEAT )
{
    /* todo
    if( xAttribs->hasAttribute( XML_dpi ) )
    {
         xsd:unsignedInt
        DPI (dots per inch) used to calculate the size of the blip. If not present or zero,
                    the DPI in the blip is used.

    }
    */
    if( xAttribs->hasAttribute( XML_rotWithShape ) )    // xsd:boolean
    {
        AttributeList aAttributeList( xAttribs );
        rFillProperties.getRotateWithShape() = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_rotWithShape, sal_False ) );
    }
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
            Reference< XInputStream > xInputStream( rFilter.openInputStream( aFragmentPath ), UNO_QUERY_THROW );

            // load the fill bitmap into an XGraphic with the GraphicProvider
            static const OUString sGraphicProvider = CREATE_OUSTRING( "com.sun.star.graphic.GraphicProvider" );
            Reference< XMultiServiceFactory > xMSFT( rFilter.getServiceFactory(), UNO_QUERY_THROW );
            Reference< XGraphicProvider > xGraphicProvider( xMSFT->createInstance( sGraphicProvider ), UNO_QUERY_THROW );

            static const OUString sInputStream = CREATE_OUSTRING( "InputStream" );
            PropertyValues aMediaProperties(1);
            aMediaProperties[0].Name = sInputStream;
            aMediaProperties[0].Value <<= xInputStream;

            Reference< XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties ) );
            mrFillProperties.mxGraphic = xGraphic;
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
        mrFillProperties.getBitmapMode() = ::boost::optional< ::com::sun::star::drawing::BitmapMode >( meBitmapMode );
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
                xRet = new clrChangeContext( *this, xAttribs, mrFillProperties.getColorChangeFrom(), mrFillProperties.getColorChangeTo() );
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
        meBitmapMode = BitmapMode_REPEAT;

        if( xAttribs->hasAttribute( XML_flip ) )        // ST_TileFlipMode
            mrFillProperties.getFlipModeToken() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_flip, XML_none ) );
        if ( xAttribs->hasAttribute( XML_tx ) )
            mrFillProperties.getTileX() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValue( XML_tx ).toInt32() );
        if ( xAttribs->hasAttribute( XML_ty ) )
            mrFillProperties.getTileY() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValue( XML_ty ).toInt32() );
        if ( xAttribs->hasAttribute( XML_sx ) )
            mrFillProperties.getTileSX() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValue( XML_sx ).toInt32() );
        if ( xAttribs->hasAttribute( XML_sx ) )
            mrFillProperties.getTileSY() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValue( XML_sy ).toInt32() );
        if ( xAttribs->hasAttribute( XML_algn ) )       // ST_RectAlignment
            mrFillProperties.getTileAlign() = ::boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_algn, XML_l ) );
        }
        break;
    case NMSP_DRAWINGML|XML_stretch:        // CT_StretchInfo
        meBitmapMode = BitmapMode_STRETCH;
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

PattFillPropertiesContext::PattFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >&, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, FillStyle_HATCH, rFillProperties )
{
}

void PattFillPropertiesContext::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
}

void PattFillPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > PattFillPropertiesContext::createFastChildContext( sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    return this;
}

// ---------------------------------------------------------------------

GrpFillPropertiesContext::GrpFillPropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >&, FillProperties& rFillProperties ) throw()
: FillPropertiesGroupContext( rParent, FillStyle_NONE, rFillProperties )
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
    ColorPtr& raClrFrom, ColorPtr& raClrTo ) throw()
: ContextHandler( rParent )
, mraClrFrom( raClrFrom )
, mraClrTo( raClrTo )
, mbUseAlpha( xAttributes->getOptionalValueToken( XML_useA, XML_true ) == XML_true ? sal_True : sal_False )
{
    mraClrFrom = ColorPtr( new Color() );
    mraClrTo = ColorPtr( new Color() );
}
clrChangeContext::~clrChangeContext()
{
    if ( !mbUseAlpha )
        mraClrTo->mbAlphaColor = sal_False;
}
Reference< XFastContextHandler > clrChangeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_clrFrom:    // CT_Color
            xRet.set( new colorChoiceContext( *this, *mraClrFrom ) );
            break;
        case NMSP_DRAWINGML|XML_clrTo:      // CT_Color
            xRet.set( new colorChoiceContext( *this, *mraClrTo ) );
            break;
        default:
        break;
    }
    return xRet;
}

} }
