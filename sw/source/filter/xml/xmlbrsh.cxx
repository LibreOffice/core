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

#include "hintids.hxx"
#include <editeng/memberids.hrc>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/unomid.hxx>
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

static SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
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
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv )
{
    SvXMLTokenMap aTokenMap( aBGImgAttributesAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        const sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_BGIMG_HREF:
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, GetImport().ResolveGraphicObjectURL( rValue, false),
                MID_GRAPHIC_LINK, rUnitConv );
            break;
        case XML_TOK_BGIMG_TYPE:
        case XML_TOK_BGIMG_ACTUATE:
        case XML_TOK_BGIMG_SHOW:
            break;
        case XML_TOK_BGIMG_POSITION:
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, rValue, MID_GRAPHIC_POSITION, rUnitConv );
            break;
        case XML_TOK_BGIMG_REPEAT:
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, rValue, MID_GRAPHIC_REPEAT, rUnitConv );
            break;
        case XML_TOK_BGIMG_FILTER:
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, rValue, MID_GRAPHIC_FILTER, rUnitConv );
            break;
        }
    }

}

SvXMLImportContext *SwXMLBrushItemImportContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    if( xmloff::token::IsXMLToken( rLocalName,
                                        xmloff::token::XML_BINARY_DATA ) )
    {
        if( !xBase64Stream.is() && pItem->GetGraphicLink().isEmpty() )
        {
            const GraphicObject *pGrObj = pItem->GetGraphicObject();
            if( !pGrObj || GraphicType::NONE == pGrObj->GetType() )
            {
                xBase64Stream =
                    GetImport().GetStreamForGraphicObjectURLFromBase64();
                if( xBase64Stream.is() )
                    pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        xBase64Stream );
            }
        }
    }
    if( !pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SwXMLBrushItemImportContext::EndElement()
{
    if( xBase64Stream.is() )
    {
        const OUString sURL( GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream ) );
        xBase64Stream = nullptr;
        SvXMLImportItemMapper::PutXMLValue( *pItem, sURL, MID_GRAPHIC_LINK, GetImport().GetMM100UnitConverter() );
    }

    if( pItem->GetGraphicLink().isEmpty() && !(pItem->GetGraphic()) )
        pItem->SetGraphicPos( GPOS_NONE );
    else if( GPOS_NONE == pItem->GetGraphicPos() )
        pItem->SetGraphicPos( GPOS_TILED );
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        const SvxBrushItem& rItem ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pItem( new SvxBrushItem( rItem ) )
{
    // delete any graphic that is existing
    pItem->SetGraphicPos( GPOS_NONE );

    ProcessAttrs( xAttrList, rUnitConv );
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        sal_uInt16 nWhich ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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

    OUString sURL;
    const SvXMLUnitConverter& rUnitConv = GetExport().GetTwipUnitConverter();
    if( SvXMLExportItemMapper::QueryXMLValue(
            rItem, sURL, MID_GRAPHIC_LINK, rUnitConv ) )
    {
        OUString sValue = GetExport().AddEmbeddedGraphicObject( sURL );
        if( !sValue.isEmpty() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sValue );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        }

        if( SvXMLExportItemMapper::QueryXMLValue(
                rItem, sValue, MID_GRAPHIC_POSITION, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_POSITION, sValue );

        if( SvXMLExportItemMapper::QueryXMLValue(
                rItem, sValue, MID_GRAPHIC_REPEAT, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REPEAT, sValue );

        if( SvXMLExportItemMapper::QueryXMLValue(
                rItem, sValue, MID_GRAPHIC_FILTER, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FILTER_NAME, sValue );
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE,
                                  true, true );
        if( !sURL.isEmpty() )
        {
            // optional office:binary-data
            GetExport().AddEmbeddedGraphicObjectAsBase64( sURL );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
