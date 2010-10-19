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

#include <osl/diagnose.h>

#include "oox/core/namespaces.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "diagramdefinitioncontext.hxx"
#include "diagramfragmenthandler.hxx"
#include "datamodelcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace oox { namespace drawingml {

DiagramDataFragmentHandler::DiagramDataFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramDataPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramDataFragmentHandler::~DiagramDataFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramDataFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramDataFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                    const Reference< XFastAttributeList >& )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case NMSP_DIAGRAM|XML_dataModel:
        xRet.set( new DataModelContext( *this, mpDataPtr ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

///////////////////

DiagramLayoutFragmentHandler::DiagramLayoutFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramLayoutPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramLayoutFragmentHandler::~DiagramLayoutFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramLayoutFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramLayoutFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                      const Reference< XFastAttributeList >& xAttribs )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case NMSP_DIAGRAM|XML_layoutDef:
        xRet.set( new DiagramDefinitionContext( *this, xAttribs, mpDataPtr ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

///////////////////////

DiagramQStylesFragmentHandler::DiagramQStylesFragmentHandler( XmlFilterBase& rFilter,
                                                              const OUString& rFragmentPath,
                                                              DiagramQStyleMap& rStylesMap ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    maStyleName(),
    maStyleEntry(),
    mrStylesMap( rStylesMap )
{}

::oox::core::ContextHandlerRef DiagramQStylesFragmentHandler::createStyleMatrixContext(
    sal_Int32 nElement,
    const AttributeList& rAttribs,
    ShapeStyleRef& o_rStyle )
{
    o_rStyle.mnThemedIdx = (nElement == (NMSP_DRAWINGML|XML_fontRef)) ?
        rAttribs.getToken( XML_idx, XML_none ) : rAttribs.getInteger( XML_idx, 0 );
    return new ColorContext( *this, o_rStyle.maPhClr );
}

::oox::core::ContextHandlerRef DiagramQStylesFragmentHandler::onCreateContext( sal_Int32 nElement,
                                                                               const AttributeList& rAttribs )
{
    // state-table like way of navigating the color fragment. we
    // currently ignore everything except styleLbl in the colorsDef
    // element
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return nElement == (NMSP_DIAGRAM|XML_styleDef) ? this : NULL;
        case NMSP_DIAGRAM|XML_styleDef:
            return nElement == (NMSP_DIAGRAM|XML_styleLbl) ? this : NULL;
        case NMSP_DIAGRAM|XML_styleLbl:
            return nElement == (NMSP_DIAGRAM|XML_style) ? this : NULL;
        case NMSP_DIAGRAM|XML_style:
        {
            switch( nElement )
            {
                case NMSP_DRAWINGML|XML_lnRef :     // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maLineStyle);
                case NMSP_DRAWINGML|XML_fillRef :   // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maFillStyle);
                case NMSP_DRAWINGML|XML_effectRef : // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maEffectStyle);
                case NMSP_DRAWINGML|XML_fontRef :   // CT_FontRe    ference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maTextStyle);
            }
            return 0;
        }
    }

    return 0;
}

void DiagramQStylesFragmentHandler::onStartElement( const AttributeList& rAttribs )
{
    if( getCurrentElement() == (NMSP_DIAGRAM|XML_styleLbl) )
    {
        maStyleName = rAttribs.getString( XML_name, OUString() );
        maStyleEntry = mrStylesMap[maStyleName];
    }
}

void DiagramQStylesFragmentHandler::onEndElement( const ::rtl::OUString& )
{
    if( getCurrentElement() == (NMSP_DIAGRAM|XML_styleLbl) )
        mrStylesMap[maStyleName] = maStyleEntry;
}

///////////////////////

ColorFragmentHandler::ColorFragmentHandler( ::oox::core::XmlFilterBase& rFilter,
                                            const ::rtl::OUString& rFragmentPath,
                                            DiagramColorMap& rColorsMap ) :
    FragmentHandler2(rFilter,rFragmentPath),
    maColorName(),
    maColorEntry(),
    mrColorsMap(rColorsMap)
{}

::oox::core::ContextHandlerRef ColorFragmentHandler::onCreateContext( sal_Int32 nElement,
                                                                      const AttributeList& /*rAttribs*/ )
{
    // state-table like way of navigating the color fragment. we
    // currently ignore everything except styleLbl in the colorsDef
    // element
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return nElement == (NMSP_DIAGRAM|XML_colorsDef) ? this : NULL;;
        case NMSP_DIAGRAM|XML_colorsDef:
            return nElement == (NMSP_DIAGRAM|XML_styleLbl) ? this : NULL;;
        case NMSP_DIAGRAM|XML_styleLbl:
            return ((nElement == (NMSP_DIAGRAM|XML_fillClrLst)) ||
                    (nElement == (NMSP_DIAGRAM|XML_linClrLst)) ||
                    (nElement == (NMSP_DIAGRAM|XML_effectClrLst)) ||
                    (nElement == (NMSP_DIAGRAM|XML_txLinClrLst)) ||
                    (nElement == (NMSP_DIAGRAM|XML_txFillClrLst)) ||
                    (nElement == (NMSP_DIAGRAM|XML_txEffectClrLst))) ? this : NULL;;

        // the actual colors - defer to color fragment handlers.

        // TODO(F1): well, actually, there might be *several* color
        // definitions in it, after all it's called list. but
        // apparently colorChoiceContext doesn't handle that anyway...
        case NMSP_DIAGRAM|XML_fillClrLst:
            return new ColorContext( *this, maColorEntry.maFillColor );
        case NMSP_DIAGRAM|XML_linClrLst:
            return new ColorContext( *this, maColorEntry.maLineColor );
        case NMSP_DIAGRAM|XML_effectClrLst:
            return new ColorContext( *this, maColorEntry.maEffectColor );
        case NMSP_DIAGRAM|XML_txFillClrLst:
            return new ColorContext( *this, maColorEntry.maTextFillColor );
        case NMSP_DIAGRAM|XML_txLinClrLst:
            return new ColorContext( *this, maColorEntry.maTextLineColor );
        case NMSP_DIAGRAM|XML_txEffectClrLst:
            return new ColorContext( *this, maColorEntry.maTextEffectColor );
    }

    return 0;
}

void ColorFragmentHandler::onStartElement( const AttributeList& rAttribs )
{
    if( getCurrentElement() == (NMSP_DIAGRAM|XML_styleLbl) )
    {
        maColorName = rAttribs.getString( XML_name, OUString() );
        maColorEntry = mrColorsMap[maColorName];
    }
}

void ColorFragmentHandler::onEndElement( const ::rtl::OUString& )
{
    if( getCurrentElement() == (NMSP_DIAGRAM|XML_styleLbl) )
        mrColorsMap[maColorName] = maColorEntry;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
