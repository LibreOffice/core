/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: themeelementscontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:32:24 $
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

#include "oox/drawingml/themeelementscontext.hxx"
#include "oox/drawingml/clrschemecontext.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

class fillStyleListContext : public ContextHandler
{
    std::vector< FillPropertiesPtr >& mrFillStyleList;

public:
    fillStyleListContext( ContextHandler& rParent, std::vector< FillPropertiesPtr >& rFillStyleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

fillStyleListContext::fillStyleListContext( ContextHandler& rParent, std::vector< FillPropertiesPtr >& rFillStyleList )
: ContextHandler( rParent )
, mrFillStyleList( rFillStyleList )
{
}

Reference< XFastContextHandler > fillStyleListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_noFill:
        case NMSP_DRAWINGML|XML_solidFill:
        case NMSP_DRAWINGML|XML_gradFill:
        case NMSP_DRAWINGML|XML_blipFill:
        case NMSP_DRAWINGML|XML_pattFill:
        case NMSP_DRAWINGML|XML_grpFill:
        {
            mrFillStyleList.push_back( FillPropertiesPtr( new FillProperties ) );
            xRet = FillPropertiesGroupContext::StaticCreateContext( *this, aElementToken, xAttribs, *mrFillStyleList.back() );
        }
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

class lineStyleListContext : public ContextHandler
{
    std::vector< LinePropertiesPtr >& mrLineStyleList;

public:
    lineStyleListContext( ContextHandler& rParent, std::vector< LinePropertiesPtr >& rLineStyleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

lineStyleListContext::lineStyleListContext( ContextHandler& rParent, std::vector< LinePropertiesPtr >& rLineStyleList )
: ContextHandler( rParent )
, mrLineStyleList( rLineStyleList )
{
}

Reference< XFastContextHandler > lineStyleListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_ln:
        {
            mrLineStyleList.push_back( LinePropertiesPtr( new LineProperties ) );
            xRet.set( new LinePropertiesContext( *this, xAttribs, *mrLineStyleList.back() ) );
        }
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

class effectStyleListContext : public ContextHandler
{
    std::vector< PropertyMap >& mrEffectStyleList;

public:
    effectStyleListContext( ContextHandler& rParent, std::vector< PropertyMap >& rEffectStyleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

effectStyleListContext::effectStyleListContext( ContextHandler& rParent, std::vector< PropertyMap >& rEffectStyleList )
: ContextHandler( rParent )
, mrEffectStyleList( rEffectStyleList )
{
}

Reference< XFastContextHandler > effectStyleListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& /* xAttribs */ ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_effectStyle:
        {
            mrEffectStyleList.push_back( PropertyMap() );
            // todo: last effect list entry needs to be filled/
            break;
        }
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

class bgFillStyleListContext : public ContextHandler
{
    std::vector< FillPropertiesPtr >& mrBgFillStyleList;

public:
    bgFillStyleListContext( ContextHandler& rParent, std::vector< FillPropertiesPtr >& rBgFillStyleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

bgFillStyleListContext::bgFillStyleListContext( ContextHandler& rParent, std::vector< FillPropertiesPtr >& rBgFillStyleList )
: ContextHandler( rParent )
, mrBgFillStyleList( rBgFillStyleList )
{
}

Reference< XFastContextHandler > bgFillStyleListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_noFill:
        case NMSP_DRAWINGML|XML_solidFill:
        case NMSP_DRAWINGML|XML_gradFill:
        case NMSP_DRAWINGML|XML_blipFill:
        case NMSP_DRAWINGML|XML_pattFill:
        case NMSP_DRAWINGML|XML_grpFill:
        {
            mrBgFillStyleList.push_back( FillPropertiesPtr( new FillProperties ) );
            xRet = FillPropertiesGroupContext::StaticCreateContext( *this, aElementToken, xAttribs, *mrBgFillStyleList.back() );
        }
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// ---------------------------------------------------------------------

themeElementsContext::themeElementsContext( ContextHandler& rParent, Theme& rTheme )
: ContextHandler( rParent )
, mrTheme( rTheme )
{
}

Reference< XFastContextHandler > themeElementsContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    // CT_BaseStyles
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_clrScheme:  // CT_ColorScheme
        {
            xRet.set( new clrSchemeContext( *this, mrTheme.getClrScheme() ) );
            break;
        }

        case NMSP_DRAWINGML|XML_fontScheme: // CT_FontScheme
            break;

        case NMSP_DRAWINGML|XML_fmtScheme:  // CT_StyleMatrix
            mrTheme.getStyleName() = xAttribs->getOptionalValue( XML_name );
        break;

            case NMSP_DRAWINGML|XML_fillStyleLst:   // CT_FillStyleList
                xRet.set( new fillStyleListContext( *this, mrTheme.getFillStyleList() ) );
            break;
            case NMSP_DRAWINGML|XML_lnStyleLst:    // CT_LineStyleList
                xRet.set( new lineStyleListContext( *this, mrTheme.getLineStyleList() ) );
            break;
            case NMSP_DRAWINGML|XML_effectStyleLst: // CT_EffectStyleList
                xRet.set( new effectStyleListContext( *this, mrTheme.getEffectStyleList() ) );
            break;
            case NMSP_DRAWINGML|XML_bgFillStyleLst: // CT_BackgroundFillStyleList
                xRet.set( new bgFillStyleListContext( *this, mrTheme.getBgFillStyleList() ) );
            break;

        case NMSP_DRAWINGML|XML_extLst:     // CT_OfficeArtExtensionList
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

} }
