/*************************************************************************
 *
 *  $RCSfile: xmlbrsh.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-15 17:16:59 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"
#include <tools/debug.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlkywd.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>

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
            pItem->importXML( GetImport().ResolveGraphicObjectURL( rValue,sal_False),
                                   MID_GRAPHIC_LINK, rUnitConv );
            break;
        case XML_TOK_BGIMG_TYPE:
        case XML_TOK_BGIMG_ACTUATE:
        case XML_TOK_BGIMG_SHOW:
            break;
        case XML_TOK_BGIMG_POSITION:
            pItem->importXML( rValue, MID_GRAPHIC_POSITION, rUnitConv );
            break;
        case XML_TOK_BGIMG_REPEAT:
            pItem->importXML( rValue, MID_GRAPHIC_REPEAT, rUnitConv );
            break;
        case XML_TOK_BGIMG_FILTER:
            pItem->importXML( rValue, MID_GRAPHIC_FILTER, rUnitConv );
            break;
        }
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

    OUString sValue;
    const SvXMLUnitConverter& rUnitConv = GetExport().GetTwipUnitConverter();
    if( rItem.exportXML( sValue, MID_GRAPHIC_LINK, rUnitConv ) )
    {
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                        GetExport().AddEmbeddedGraphicObject( sValue ) );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
//      AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, ACP2WS(sXML_embed) );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        if( rItem.exportXML( sValue, MID_GRAPHIC_POSITION, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_POSITION, sValue );

        if( rItem.exportXML( sValue, MID_GRAPHIC_REPEAT, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REPEAT, sValue );

        if( rItem.exportXML( sValue, MID_GRAPHIC_FILTER, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FILTER_NAME, sValue );
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE,
                              sal_True, sal_True );
}

/*************************************************************************

      Source Code Control ::com::sun::star::chaos::System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlbrsh.cxx,v 1.3 2001-06-15 17:16:59 dvo Exp $

      Source Code Control ::com::sun::star::chaos::System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/12/02 10:57:15  mib
      #80795#: use packages

      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.2  2000/09/18 16:05:05  willem.vandorp
      OpenOffice header added.

      Revision 1.1  2000/08/02 14:52:38  mib
      text export continued

      Revision 1.2  2000/06/26 08:29:55  mib
      no SVX dependency any longer

      Revision 1.1  2000/06/08 09:14:25  aw
      new export classes from svx

      Revision 1.5  2000/05/02 10:04:19  mib
      unicode

      Revision 1.4  2000/03/13 14:38:56  cl
      uno3

      Revision 1.3  2000/02/10 20:09:32  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.2  2000/01/06 14:59:19  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.1  1999/12/13 08:11:18  mib
      #70271#: XML brush item import/export

      Revision 1.1  1999/12/04 16:47:25  cl
      #70271# added support for exporting tab-stop elements in xml


*************************************************************************/

