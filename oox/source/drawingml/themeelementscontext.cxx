/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: themeelementscontext.cxx,v $
 * $Revision: 1.5 $
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
    EffectStyleList& mrEffectStyleList;

public:
    effectStyleListContext( ContextHandler& rParent, EffectStyleList& rEffectStyleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

effectStyleListContext::effectStyleListContext( ContextHandler& rParent, EffectStyleList& rEffectStyleList )
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
            mrEffectStyleList.push_back( EffectStyleList::value_type( new PropertyMap ) );
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
            mrTheme.setStyleName( xAttribs->getOptionalValue( XML_name ) );
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
