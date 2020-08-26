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

#include <editeng/memberids.h>
#include <vcl/graph.hxx>

#include <sal/log.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <editeng/brushitem.hxx>
#include <xmloff/xmluconv.hxx>

#include "xmlbrshi.hxx"
#include "xmlbrshe.hxx"
#include "xmlexp.hxx"
#include "xmlimpit.hxx"
#include "xmlexpit.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

namespace {

enum SvXMLTokenMapAttrs
{
    XML_TOK_BGIMG_HREF,
    XML_TOK_BGIMG_TYPE,
    XML_TOK_BGIMG_ACTUATE,
    XML_TOK_BGIMG_SHOW,
    XML_TOK_BGIMG_POSITION,
    XML_TOK_BGIMG_REPEAT,
    XML_TOK_BGIMG_FILTER,
};

}

const SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK, XML_HREF,        XML_TOK_BGIMG_HREF      },
    { XML_NAMESPACE_XLINK, XML_TYPE,        XML_TOK_BGIMG_TYPE      },
    { XML_NAMESPACE_XLINK, XML_ACTUATE,     XML_TOK_BGIMG_ACTUATE   },
    { XML_NAMESPACE_XLINK, XML_SHOW,        XML_TOK_BGIMG_SHOW      },
    { XML_NAMESPACE_STYLE, XML_POSITION,    XML_TOK_BGIMG_POSITION  },
    { XML_NAMESPACE_STYLE, XML_REPEAT,      XML_TOK_BGIMG_REPEAT    },
    { XML_NAMESPACE_STYLE, XML_FILTER_NAME, XML_TOK_BGIMG_FILTER    },
    XML_TOKEN_MAP_END
};


void SwXMLBrushItemImportContext::ProcessAttrs(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv )
{
    SvXMLTokenMap aTokenMap( aBGImgAttributesAttrTokenMap );

    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        const OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
        case XML_ELEMENT(XLINK, XML_HREF):
            m_xGraphic = GetImport().loadGraphicByURL(sValue);
            break;
        case XML_ELEMENT(XLINK, XML_TYPE):
        case XML_ELEMENT(XLINK, XML_ACTUATE):
        case XML_ELEMENT(XLINK, XML_SHOW):
            break;
        case XML_ELEMENT(STYLE, XML_POSITION):
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, sValue, MID_GRAPHIC_POSITION, rUnitConv );
            break;
        case XML_ELEMENT(STYLE, XML_REPEAT):
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, sValue, MID_GRAPHIC_REPEAT, rUnitConv );
            break;
        case XML_ELEMENT(STYLE, XML_FILTER_NAME):
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, sValue, MID_GRAPHIC_FILTER, rUnitConv );
            break;
        default:
            SAL_WARN("sw", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }

}

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLBrushItemImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if ((nElement & TOKEN_MASK) == xmloff::token::XML_BINARY_DATA)
    {
        if (!m_xBase64Stream.is())
        {
            m_xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if (m_xBase64Stream.is())
                return new XMLBase64ImportContext(GetImport(), m_xBase64Stream);
        }
    }
    SAL_WARN("sw", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

void SwXMLBrushItemImportContext::endFastElement(sal_Int32 )
{
    if (m_xBase64Stream.is())
    {
        m_xGraphic = GetImport().loadGraphicFromBase64(m_xBase64Stream);
        m_xBase64Stream = nullptr;
    }

    if (m_xGraphic.is())
    {
        Graphic aGraphic(m_xGraphic);
        SvxGraphicPosition eOldGraphicPos = pItem->GetGraphicPos();
        pItem->SetGraphic(aGraphic);
        if (GPOS_NONE == eOldGraphicPos && GPOS_NONE != pItem->GetGraphicPos())
            pItem->SetGraphicPos(GPOS_TILED);
    }

    if (!(pItem->GetGraphic()))
        pItem->SetGraphicPos(GPOS_NONE);
    else if (GPOS_NONE == pItem->GetGraphicPos())
        pItem->SetGraphicPos(GPOS_TILED);
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_Int32 /*nElement*/,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        const SvxBrushItem& rItem ) :
    SvXMLImportContext( rImport ),
    pItem( new SvxBrushItem( rItem ) )
{
    // delete any graphic that is existing
    pItem->SetGraphicPos( GPOS_NONE );

    ProcessAttrs( xAttrList, rUnitConv );
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_Int32 /*nElement*/,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        sal_uInt16 nWhich ) :
    SvXMLImportContext( rImport ),
    pItem( new SvxBrushItem( nWhich ) )
{
    ProcessAttrs( xAttrList, rUnitConv );
}

SwXMLBrushItemImportContext::~SwXMLBrushItemImportContext()
{
}

SwXMLBrushItemExport::SwXMLBrushItemExport( SwXMLExport& rExp ) :
    rExport( rExp )
{
}

SwXMLBrushItemExport::~SwXMLBrushItemExport()
{
}

void SwXMLBrushItemExport::exportXML( const SvxBrushItem& rItem )
{
    GetExport().CheckAttrList();

    uno::Reference<graphic::XGraphic> xGraphic;

    const Graphic* pGraphic = rItem.GetGraphic();

    if (pGraphic)
        xGraphic = pGraphic->GetXGraphic();

    if (xGraphic.is())
    {
        OUString sMimeType;
        OUString sValue = GetExport().AddEmbeddedXGraphic(xGraphic, sMimeType);
        if (!sValue.isEmpty())
        {
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sValue );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        }

        const SvXMLUnitConverter& rUnitConv = GetExport().GetTwipUnitConverter();
        if (SvXMLExportItemMapper::QueryXMLValue(rItem, sValue, MID_GRAPHIC_POSITION, rUnitConv))
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_POSITION, sValue );

        if (SvXMLExportItemMapper::QueryXMLValue(rItem, sValue, MID_GRAPHIC_REPEAT, rUnitConv))
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REPEAT, sValue );

        if (SvXMLExportItemMapper::QueryXMLValue(rItem, sValue, MID_GRAPHIC_FILTER, rUnitConv))
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FILTER_NAME, sValue );
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE,
                                  true, true );
        if (xGraphic.is())
        {
            // optional office:binary-data
            GetExport().AddEmbeddedXGraphicAsBase64(xGraphic);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
