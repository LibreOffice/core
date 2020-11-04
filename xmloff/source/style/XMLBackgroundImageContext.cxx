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


#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <tools/debug.hxx>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <XMLBackgroundImageContext.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;

const SvXMLEnumMapEntry<GraphicLocation> psXML_BrushHoriPos[] =
{
    { XML_LEFT,         GraphicLocation_LEFT_MIDDLE },
    { XML_RIGHT,        GraphicLocation_RIGHT_MIDDLE    },
    { XML_TOKEN_INVALID,                    GraphicLocation(0)           }
};

const SvXMLEnumMapEntry<GraphicLocation> psXML_BrushVertPos[] =
{
    { XML_TOP,          GraphicLocation_MIDDLE_TOP  },
    { XML_BOTTOM,       GraphicLocation_MIDDLE_BOTTOM   },
    { XML_TOKEN_INVALID,                    GraphicLocation(0)           }
};

static void lcl_xmlbic_MergeHoriPos( GraphicLocation& ePos,
                                   GraphicLocation eHori )
{
    DBG_ASSERT( GraphicLocation_LEFT_MIDDLE==eHori ||
                GraphicLocation_MIDDLE_MIDDLE==eHori ||
                GraphicLocation_RIGHT_MIDDLE==eHori,
                "lcl_xmlbic_MergeHoriPos: vertical pos must be middle" );

    switch( ePos )
    {
    case GraphicLocation_LEFT_TOP:
    case GraphicLocation_MIDDLE_TOP:
    case GraphicLocation_RIGHT_TOP:
        ePos = GraphicLocation_LEFT_MIDDLE==eHori
                ? GraphicLocation_LEFT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eHori
                        ? GraphicLocation_MIDDLE_TOP
                        : GraphicLocation_RIGHT_TOP);
        break;

    case GraphicLocation_LEFT_MIDDLE:
    case GraphicLocation_MIDDLE_MIDDLE:
    case GraphicLocation_RIGHT_MIDDLE:
        ePos = eHori;
        break;

    case GraphicLocation_LEFT_BOTTOM:
    case GraphicLocation_MIDDLE_BOTTOM:
    case GraphicLocation_RIGHT_BOTTOM:
        ePos = GraphicLocation_LEFT_MIDDLE==eHori
                ? GraphicLocation_LEFT_BOTTOM
                : (GraphicLocation_MIDDLE_MIDDLE==eHori
                        ? GraphicLocation_MIDDLE_BOTTOM
                        : GraphicLocation_RIGHT_BOTTOM);
        break;
    default:
        break;
    }
}

static void lcl_xmlbic_MergeVertPos( GraphicLocation& ePos,
                                                   GraphicLocation eVert )
{
    DBG_ASSERT( GraphicLocation_MIDDLE_TOP==eVert ||
                GraphicLocation_MIDDLE_MIDDLE==eVert ||
                GraphicLocation_MIDDLE_BOTTOM==eVert,
                "lcl_xmlbic_MergeVertPos: horizontal pos must be middle" );

    switch( ePos )
    {
    case GraphicLocation_LEFT_TOP:
    case GraphicLocation_LEFT_MIDDLE:
    case GraphicLocation_LEFT_BOTTOM:
        ePos = GraphicLocation_MIDDLE_TOP==eVert
                ? GraphicLocation_LEFT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eVert
                        ? GraphicLocation_LEFT_MIDDLE
                        : GraphicLocation_LEFT_BOTTOM);
        break;

    case GraphicLocation_MIDDLE_TOP:
    case GraphicLocation_MIDDLE_MIDDLE:
    case GraphicLocation_MIDDLE_BOTTOM:
        ePos = eVert;
        break;

    case GraphicLocation_RIGHT_TOP:
    case GraphicLocation_RIGHT_MIDDLE:
    case GraphicLocation_RIGHT_BOTTOM:
        ePos = GraphicLocation_MIDDLE_TOP==eVert
                ? GraphicLocation_RIGHT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eVert
                        ? GraphicLocation_RIGHT_MIDDLE
                        : GraphicLocation_RIGHT_BOTTOM);
        break;
    default:
        break;
    }
}


void XMLBackgroundImageContext::ProcessAttrs(
        const Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    ePos = GraphicLocation_NONE;

    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        const OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
        case XML_ELEMENT(XLINK, XML_HREF):
            m_sURL = sValue;
            if( GraphicLocation_NONE == ePos )
                ePos = GraphicLocation_TILED;
            break;
        case XML_ELEMENT(XLINK, XML_TYPE):
        case XML_ELEMENT(XLINK, XML_ACTUATE):
        case XML_ELEMENT(XLINK, XML_SHOW):
            break;
        case XML_ELEMENT(STYLE, XML_POSITION):
            {
                GraphicLocation eNewPos = GraphicLocation_NONE, eTmp;
                SvXMLTokenEnumerator aTokenEnum( sValue );
                OUString aToken;
                bool bHori = false, bVert = false;
                bool bOK = true;
                while( bOK && aTokenEnum.getNextToken( aToken ) )
                {
                    GraphicLocation nTmpGraphicLocation;
                    if( bHori && bVert )
                    {
                        bOK = false;
                    }
                    else if( -1 != aToken.indexOf( '%' ) )
                    {
                        sal_Int32 nPrc = 50;
                        if (::sax::Converter::convertPercent( nPrc, aToken ))
                        {
                            if( !bHori )
                            {
                                eNewPos = nPrc < 25
                                    ? GraphicLocation_LEFT_TOP
                                    : (nPrc < 75 ? GraphicLocation_MIDDLE_MIDDLE
                                                : GraphicLocation_RIGHT_BOTTOM);
                                bHori = true;
                            }
                            else
                            {
                                eTmp = nPrc < 25
                                    ? GraphicLocation_LEFT_TOP
                                    : (nPrc < 75 ? GraphicLocation_LEFT_MIDDLE
                                                 : GraphicLocation_LEFT_BOTTOM);
                                lcl_xmlbic_MergeVertPos( eNewPos, eTmp );
                                bVert = true;
                            }
                        }
                        else
                        {
                            // wrong percentage
                            bOK = false;
                        }
                    }
                    else if( IsXMLToken( aToken, XML_CENTER ) )
                    {
                        if( bHori )
                            lcl_xmlbic_MergeVertPos( eNewPos,
                                          GraphicLocation_MIDDLE_MIDDLE );
                        else if ( bVert )
                            lcl_xmlbic_MergeHoriPos( eNewPos,
                                          GraphicLocation_MIDDLE_MIDDLE );
                        else
                            eNewPos = GraphicLocation_MIDDLE_MIDDLE;
                    }
                    else if( SvXMLUnitConverter::convertEnum( nTmpGraphicLocation, aToken,
                                                         psXML_BrushHoriPos ) )
                    {
                        if( bVert )
                            lcl_xmlbic_MergeHoriPos( eNewPos, nTmpGraphicLocation );
                        else if( !bHori )
                            eNewPos = nTmpGraphicLocation;
                        else
                            bOK = false;
                        bHori = true;
                    }
                    else if( SvXMLUnitConverter::convertEnum( nTmpGraphicLocation, aToken,
                                                         psXML_BrushVertPos ) )
                    {
                        if( bHori )
                            lcl_xmlbic_MergeVertPos( eNewPos, nTmpGraphicLocation );
                        else if( !bVert )
                            eNewPos = nTmpGraphicLocation;
                        else
                            bOK = false;
                        bVert = true;
                    }
                    else
                    {
                        bOK = false;
                    }
                }

                bOK &= GraphicLocation_NONE != eNewPos;
                if( bOK )
                    ePos = eNewPos;
            }
            break;
        case XML_ELEMENT(STYLE, XML_REPEAT):
            {
                GraphicLocation nPos = GraphicLocation_NONE;
                static const SvXMLEnumMapEntry<GraphicLocation> psXML_BrushRepeat[] =
                {
                    { XML_REPEAT,               GraphicLocation_TILED   },
                    { XML_BACKGROUND_NO_REPEAT, GraphicLocation_MIDDLE_MIDDLE       },
                    { XML_STRETCH,              GraphicLocation_AREA    },
                    { XML_TOKEN_INVALID,        GraphicLocation(0)      }
                };
                if( SvXMLUnitConverter::convertEnum( nPos, sValue,
                                                psXML_BrushRepeat ) )
                {
                    if( GraphicLocation_MIDDLE_MIDDLE != nPos ||
                        GraphicLocation_NONE == ePos ||
                        GraphicLocation_AREA == ePos ||
                        GraphicLocation_TILED == ePos )
                        ePos = nPos;
                }
            }
            break;
        case XML_ELEMENT(STYLE, XML_FILTER_NAME):
            sFilter = sValue;
            break;
        case XML_ELEMENT(DRAW, XML_OPACITY):
            {
                sal_Int32 nTmp;
                // convert from percent and clip
                if (::sax::Converter::convertPercent( nTmp, sValue ))
                {
                    if( (nTmp >= 0) && (nTmp <= 100) )
                        nTransparency = static_cast<sal_Int8>( 100-nTmp );
                }
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

}

XMLBackgroundImageContext::XMLBackgroundImageContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        sal_Int32 nTransparencyIdx,
        sal_Int32 nBitmapModeIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nElement, rProp, rProps ),
    aPosProp( nPosIdx ),
    m_nBitmapModeIdx(nBitmapModeIdx),
    aFilterProp( nFilterIdx ),
    aTransparencyProp( nTransparencyIdx ),
    nTransparency( 0 )
{
    ProcessAttrs( xAttrList );
}

XMLBackgroundImageContext::~XMLBackgroundImageContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLBackgroundImageContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if( nElement == XML_ELEMENT(OFFICE, xmloff::token::XML_BINARY_DATA) )
    {
        if( m_sURL.isEmpty() && !m_xBase64Stream.is() )
        {
            m_xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( m_xBase64Stream.is() )
                return new XMLBase64ImportContext( GetImport(), m_xBase64Stream );
        }
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void XMLBackgroundImageContext::endFastElement(sal_Int32 nElement)
{
    uno::Reference<graphic::XGraphic> xGraphic;
    if (!m_sURL.isEmpty())
    {
        xGraphic = GetImport().loadGraphicByURL(m_sURL);
    }
    else if (m_xBase64Stream.is())
    {
        xGraphic = GetImport().loadGraphicFromBase64(m_xBase64Stream);
        m_xBase64Stream = nullptr;
    }

    if (!xGraphic.is())
        ePos = GraphicLocation_NONE;
    else if (GraphicLocation_NONE == ePos)
        ePos = GraphicLocation_TILED;

    if (xGraphic.is())
        aProp.maValue <<= xGraphic;
    aPosProp.maValue <<= ePos;
    aFilterProp.maValue <<= sFilter;
    aTransparencyProp.maValue <<= nTransparency;

    SetInsert( true );
    XMLElementPropertyContext::endFastElement(nElement);

    if( -1 != aPosProp.mnIndex )
    {
        // See if a FillBitmapMode property is already set, in that case
        // BackGraphicLocation will be ignored.
        bool bFound = false;
        if (m_nBitmapModeIdx != -1)
        {
            for (XMLPropertyState& rProperty : rProperties)
            {
                if (rProperty.mnIndex == m_nBitmapModeIdx)
                {
                    bFound = true;

                    // Found, so map the old property to the new one.
                    switch (ePos)
                    {
                    case GraphicLocation_TILED:
                        rProperty.maValue <<= drawing::BitmapMode_REPEAT;
                        break;
                    case GraphicLocation_AREA:
                        rProperty.maValue <<= drawing::BitmapMode_STRETCH;
                        break;
                    case GraphicLocation_MIDDLE_MIDDLE:
                        rProperty.maValue <<= drawing::BitmapMode_NO_REPEAT;
                        break;
                    default:
                        break;
                    }
                    break;
                }
            }
        }
        if (!bFound)
            rProperties.push_back( aPosProp );
    }
    if( -1 != aFilterProp.mnIndex )
        rProperties.push_back( aFilterProp );
    if( -1 != aTransparencyProp.mnIndex )
        rProperties.push_back( aTransparencyProp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
