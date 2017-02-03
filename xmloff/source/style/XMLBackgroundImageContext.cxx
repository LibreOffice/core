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

#include <tools/debug.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include "XMLBackgroundImageContext.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_BGIMG_HREF,
    XML_TOK_BGIMG_TYPE,
    XML_TOK_BGIMG_ACTUATE,
    XML_TOK_BGIMG_SHOW,
    XML_TOK_BGIMG_POSITION,
    XML_TOK_BGIMG_REPEAT,
    XML_TOK_BGIMG_FILTER,
    XML_TOK_BGIMG_OPACITY
};
static const SvXMLTokenMapEntry* lcl_getBGImgAttributesAttrTokenMap()
{
    static const SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
    {
        { XML_NAMESPACE_XLINK, XML_HREF,        XML_TOK_BGIMG_HREF      },
        { XML_NAMESPACE_XLINK, XML_TYPE,        XML_TOK_BGIMG_TYPE      },
        { XML_NAMESPACE_XLINK, XML_ACTUATE,     XML_TOK_BGIMG_ACTUATE   },
        { XML_NAMESPACE_XLINK, XML_SHOW,        XML_TOK_BGIMG_SHOW      },
        { XML_NAMESPACE_STYLE, XML_POSITION,    XML_TOK_BGIMG_POSITION  },
        { XML_NAMESPACE_STYLE, XML_REPEAT,      XML_TOK_BGIMG_REPEAT    },
        { XML_NAMESPACE_STYLE, XML_FILTER_NAME, XML_TOK_BGIMG_FILTER    },
        { XML_NAMESPACE_DRAW,  XML_OPACITY,     XML_TOK_BGIMG_OPACITY   },
        XML_TOKEN_MAP_END
    };
    return aBGImgAttributesAttrTokenMap;
}


static const SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { XML_LEFT,         GraphicLocation_LEFT_MIDDLE },
    { XML_RIGHT,        GraphicLocation_RIGHT_MIDDLE    },
    { XML_TOKEN_INVALID,                    0           }
};

static const SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { XML_TOP,          GraphicLocation_MIDDLE_TOP  },
    { XML_BOTTOM,       GraphicLocation_MIDDLE_BOTTOM   },
    { XML_TOKEN_INVALID,                    0           }
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
        const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( lcl_getBGImgAttributesAttrTokenMap() );

    ePos = GraphicLocation_NONE;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_BGIMG_HREF:
            sURL = rValue;
            if( GraphicLocation_NONE == ePos )
                ePos = GraphicLocation_TILED;
            break;
        case XML_TOK_BGIMG_TYPE:
        case XML_TOK_BGIMG_ACTUATE:
        case XML_TOK_BGIMG_SHOW:
            break;
        case XML_TOK_BGIMG_POSITION:
            {
                GraphicLocation eNewPos = GraphicLocation_NONE, eTmp;
                sal_uInt16 nTmp;
                SvXMLTokenEnumerator aTokenEnum( rValue );
                OUString aToken;
                bool bHori = false, bVert = false;
                bool bOK = true;
                while( bOK && aTokenEnum.getNextToken( aToken ) )
                {
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
                    else if( SvXMLUnitConverter::convertEnum( nTmp, aToken,
                                                         psXML_BrushHoriPos ) )
                    {
                        if( bVert )
                            lcl_xmlbic_MergeHoriPos( eNewPos,
                                        (GraphicLocation)nTmp );
                        else if( !bHori )
                            eNewPos = (GraphicLocation)nTmp;
                        else
                            bOK = false;
                        bHori = true;
                    }
                    else if( SvXMLUnitConverter::convertEnum( nTmp, aToken,
                                                         psXML_BrushVertPos ) )
                    {
                        if( bHori )
                            lcl_xmlbic_MergeVertPos( eNewPos,
                                            (GraphicLocation)nTmp );
                        else if( !bVert )
                            eNewPos = (GraphicLocation)nTmp;
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
        case XML_TOK_BGIMG_REPEAT:
            {
                sal_uInt16 nPos = GraphicLocation_NONE;
                static const SvXMLEnumMapEntry psXML_BrushRepeat[] =
                {
                    { XML_BACKGROUND_REPEAT,        GraphicLocation_TILED   },
                    { XML_BACKGROUND_NO_REPEAT,     GraphicLocation_MIDDLE_MIDDLE       },
                    { XML_BACKGROUND_STRETCH,       GraphicLocation_AREA    },
                    { XML_TOKEN_INVALID,            0           }
                };
                if( SvXMLUnitConverter::convertEnum( nPos, rValue,
                                                psXML_BrushRepeat ) )
                {
                    if( GraphicLocation_MIDDLE_MIDDLE != nPos ||
                        GraphicLocation_NONE == ePos ||
                        GraphicLocation_AREA == ePos ||
                        GraphicLocation_TILED == ePos )
                        ePos = (GraphicLocation)nPos;
                }
            }
            break;
        case XML_TOK_BGIMG_FILTER:
            sFilter = rValue;
            break;
        case XML_TOK_BGIMG_OPACITY:
            {
                sal_Int32 nTmp;
                // convert from percent and clip
                if (::sax::Converter::convertPercent( nTmp, rValue ))
                {
                    if( (nTmp >= 0) && (nTmp <= 100) )
                        nTransparency = static_cast<sal_Int8>( 100-nTmp );
                }
            }
            break;
        }
    }

}

XMLBackgroundImageContext::XMLBackgroundImageContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        sal_Int32 nTransparencyIdx,
        sal_Int32 nBitmapModeIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
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

SvXMLImportContext *XMLBackgroundImageContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    if( (XML_NAMESPACE_OFFICE == nPrefix) &&
        xmloff::token::IsXMLToken( rLocalName,
                                        xmloff::token::XML_BINARY_DATA ) )
    {
        if( sURL.isEmpty() && !xBase64Stream.is() )
        {
            xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( xBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    xBase64Stream );
        }
    }
    if( !pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLBackgroundImageContext::EndElement()
{
    if( !sURL.isEmpty() )
    {
        sURL = GetImport().ResolveGraphicObjectURL( sURL, false );
    }
    else if( xBase64Stream.is() )
    {
        sURL = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
        xBase64Stream = nullptr;
    }

    if( sURL.isEmpty() )
        ePos = GraphicLocation_NONE;
    else if( GraphicLocation_NONE == ePos )
        ePos = GraphicLocation_TILED;

    if (!sURL.isEmpty())
        aProp.maValue <<= sURL;
    aPosProp.maValue <<= ePos;
    aFilterProp.maValue <<= sFilter;
    aTransparencyProp.maValue <<= nTransparency;

    SetInsert( true );
    XMLElementPropertyContext::EndElement();

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
