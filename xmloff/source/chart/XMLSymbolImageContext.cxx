/*************************************************************************
 *
 *  $RCSfile: XMLSymbolImageContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2001-08-14 13:49:56 $
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
#include "XMLSymbolImageContext.hxx"

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

TYPEINIT1( XMLSymbolImageContext, XMLElementPropertyContext );

using namespace ::com::sun::star;

enum SvXMLTokenMapAttrs
{
    XML_TOK_SYMBOL_IMAGE_HREF,
    XML_TOK_SYMBOL_IMAGE_TYPE,
    XML_TOK_SYMBOL_IMAGE_ACTUATE,
    XML_TOK_SYMBOL_IMAGE_SHOW,
    XML_TOK_SYMBOL_IMAGE_END = XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aSymbolImageAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_HREF,     XML_TOK_SYMBOL_IMAGE_HREF    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_TYPE,     XML_TOK_SYMBOL_IMAGE_TYPE    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_ACTUATE,  XML_TOK_SYMBOL_IMAGE_ACTUATE },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_SHOW,     XML_TOK_SYMBOL_IMAGE_SHOW },
    XML_TOKEN_MAP_END
};

XMLSymbolImageContext::XMLSymbolImageContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nPrfx, rLName, rProp, rProps )
{
}

XMLSymbolImageContext::~XMLSymbolImageContext()
{}

void XMLSymbolImageContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( aSymbolImageAttrTokenMap );
    ::rtl::OUString aLocalName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        const ::rtl::OUString& rAttrName = xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const ::rtl::OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SYMBOL_IMAGE_HREF:
                msURL = rValue;
                break;
            case XML_TOK_SYMBOL_IMAGE_ACTUATE:
            case XML_TOK_SYMBOL_IMAGE_TYPE:
            case XML_TOK_SYMBOL_IMAGE_SHOW:
                // these values are currently not interpreted
                // it is always assumed 'actuate=onLoad', 'type=simple', 'show=embed'
                break;
        }
    }
}

SvXMLImportContext* XMLSymbolImageContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = NULL;
    if( xmloff::token::IsXMLToken( rLocalName,
                                   xmloff::token::XML_BINARY_DATA ) )
    {
        if( ! msURL.getLength() && ! mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                       rLocalName, xAttrList,
                                                       mxBase64Stream );
        }
    }
    if( ! pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLSymbolImageContext::EndElement()
{
    ::rtl::OUString sResolvedURL;

    if( msURL.getLength() )
    {
        sResolvedURL = GetImport().ResolveGraphicObjectURL( msURL, sal_False );
    }
    else if( mxBase64Stream.is() )
    {
        sResolvedURL = GetImport().ResolveGraphicObjectURLFromBase64( mxBase64Stream );
        mxBase64Stream = 0;
    }

    if( sResolvedURL.getLength())
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= sResolvedURL;
        SetInsert( sal_True );
    }

    XMLElementPropertyContext::EndElement();
}
