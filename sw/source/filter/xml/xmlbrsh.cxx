/*************************************************************************
 *
 *  $RCSfile: xmlbrsh.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:06:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include "hintids.hxx"
#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include <xmloff/XMLBase64ImportContext.hxx>
#endif

#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#include "xmlbrshi.hxx"
#include "xmlbrshe.hxx"
#include "xmlexp.hxx"
#include "xmlimpit.hxx"
#include "xmlexpit.hxx"

using namespace ::rtl;
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
    XML_TOK_NGIMG_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
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

TYPEINIT1( SwXMLBrushItemImportContext, SvXMLImportContext );

void SwXMLBrushItemImportContext::ProcessAttrs(
        const Reference< xml::sax::XAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv )
{
    SvXMLTokenMap aTokenMap( aBGImgAttributesAttrTokenMap );

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
            SvXMLImportItemMapper::PutXMLValue(
                *pItem, GetImport().ResolveGraphicObjectURL( rValue,sal_False),
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
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext;
    if( xmloff::token::IsXMLToken( rLocalName,
                                        xmloff::token::XML_BINARY_DATA ) )
    {
        if( !(pItem->GetGraphicLink() || pItem->GetGraphic() ) && !xBase64Stream.is() )
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

void SwXMLBrushItemImportContext::EndElement()
{
    if( xBase64Stream.is() )
    {
        OUString sURL( GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream ) );
        xBase64Stream = 0;
        SvXMLImportItemMapper::PutXMLValue( *pItem, sURL, MID_GRAPHIC_LINK, GetImport().GetMM100UnitConverter() );
    }

    if( !(pItem->GetGraphicLink() || pItem->GetGraphic() ) )
        pItem->SetGraphicPos( GPOS_NONE );
    else if( GPOS_NONE == pItem->GetGraphicPos() )
        pItem->SetGraphicPos( GPOS_TILED );
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList >& xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        const SvxBrushItem& rItem ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pItem( new SvxBrushItem( rItem ) )
{
    // delete any grephic that is existing
    pItem->SetGraphicPos( GPOS_NONE );

    ProcessAttrs( xAttrList, rUnitConv );
}

SwXMLBrushItemImportContext::SwXMLBrushItemImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const SvXMLUnitConverter& rUnitConv,
        sal_uInt16 nWhich ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pItem( new SvxBrushItem( nWhich ) )
{
    ProcessAttrs( xAttrList, rUnitConv );
}

SwXMLBrushItemImportContext::~SwXMLBrushItemImportContext()
{
    delete pItem;
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

    OUString sValue, sURL;
    const SvXMLUnitConverter& rUnitConv = GetExport().GetTwipUnitConverter();
    if( SvXMLExportItemMapper::QueryXMLValue(
            rItem, sURL, MID_GRAPHIC_LINK, rUnitConv ) )
    {
        sValue = GetExport().AddEmbeddedGraphicObject( sURL );
        if( sValue.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sValue );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    //      AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, ACP2WS(sXML_embed) );
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
                                  sal_True, sal_True );
        if( sURL.getLength() )
        {
            // optional office:binary-data
            GetExport().AddEmbeddedGraphicObjectAsBase64( sURL );
        }
    }
}


