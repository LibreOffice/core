/*************************************************************************
 *
 *  $RCSfile: ImageStyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-10 11:12:45 $
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

#ifndef _XMLOFF_IMAGESTYLE_HXX
#include "ImageStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include"attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include"xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include"xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include<rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include<rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

enum SvXMLTokenMapAttrs
{
    XML_TOK_IMAGE_NAME,
    XML_TOK_IMAGE_URL,
    XML_TOK_IMAGE_TYPE,
    XML_TOK_IMAGE_SHOW,
    XML_TOK_IMAGE_ACTUATE,
    /* XML_TOK_IMAGE_SIZEW,
       XML_TOK_IMAGE_SIZEH,*/
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aHatchAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, sXML_name, XML_TOK_IMAGE_NAME },
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_IMAGE_URL },
    { XML_NAMESPACE_XLINK, sXML_type, XML_TOK_IMAGE_TYPE },
    { XML_NAMESPACE_XLINK, sXML_show, XML_TOK_IMAGE_SHOW },
    { XML_NAMESPACE_XLINK, sXML_actuate, XML_TOK_IMAGE_ACTUATE },
    /*{ XML_NAMESPACE_XLINK, sXML_href, XML_TOK_IMAGE_URL },
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_IMAGE_URL },*/
    XML_TOKEN_MAP_END
};

XMLImageStyle::XMLImageStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & _rHandler,
                                        const SvXMLNamespaceMap& _rNamespaceMap, const SvXMLUnitConverter& _rUnitConverter )
: rHandler      ( _rHandler ),
  mrNamespaceMap ( _rNamespaceMap ),
  rUnitConverter( _rUnitConverter ),
  pAttrList     ( NULL )
{
}

XMLImageStyle::~XMLImageStyle()
{
}

void XMLImageStyle::AddAttribute( sal_uInt16 nPrefix, const sal_Char *pName, const OUString& aStrValue )
{
    const OUString aStrName( OUString::createFromAscii( pName ) );
    const OUString aStrCDATA( OUString::createFromAscii( sXML_CDATA ) );

    pAttrList->AddAttribute( mrNamespaceMap.GetQNameByKey( nPrefix, aStrName ), aStrCDATA, aStrValue );
}

sal_Bool XMLImageStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue )
{
    return ImpExportXML( rHandler, mrNamespaceMap, rUnitConverter, rStrName, rValue );
}

sal_Bool XMLImageStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName )
{
    return ImpImportXML( rUnitConverter, xAttrList, rValue, rStrName );
}

sal_Bool XMLImageStyle::ImpExportXML( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                                           const SvXMLNamespaceMap& rNamespaceMap, const SvXMLUnitConverter& rUnitConverter,
                                           const OUString& rStrName, const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;

    OUString aURL;

    if( rStrName.getLength() )
    {
        if( rValue >>= aURL )
        {
            pAttrList = new SvXMLAttributeList();   // Do NOT delete me !!
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList( pAttrList );

            OUString aStrValue;
            OUStringBuffer aOut;

            // Name
            AddAttribute( XML_NAMESPACE_DRAW, sXML_name, rStrName );

            // uri
            AddAttribute( XML_NAMESPACE_XLINK, sXML_href, aURL );
            AddAttribute( XML_NAMESPACE_XLINK, sXML_type, OUString::createFromAscii(sXML_simple) );
            AddAttribute( XML_NAMESPACE_XLINK, sXML_show, OUString::createFromAscii(sXML_embed) );
            AddAttribute( XML_NAMESPACE_XLINK, sXML_actuate, OUString::createFromAscii(sXML_onLoad) );

/*
            // size
            awt::Size aSize = xBitmap->getSize();

            rUnitConverter.convertNumber( aOut, aSize.Width );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, sXML_width, aStrValue );

            rUnitConverter.convertNumber( aOut, aSize.Height );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, sXML_height, aStrValue );
*/

            // Do Write
            rHandler->startElement( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_DRAW, OUString::createFromAscii(sXML_fill_image) ),
                                    xAttrList );
            rHandler->endElement( OUString::createFromAscii( sXML_fill_image ) );
        }
    }

    return bRet;
}

sal_Bool XMLImageStyle::ImpImportXML( const SvXMLUnitConverter& rUnitConverter,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                           uno::Any& rValue, OUString& rStrName )
{
    sal_Bool bRet     = sal_False;

    sal_Bool bHasHRef = sal_False;
    sal_Bool bHasName = sal_False;

    OUString aStrURL;

    SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = mrNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
            case XML_TOK_IMAGE_NAME:
                {
                    rStrName = rStrValue;
                    bHasName = sal_True;
                }
                break;
            case XML_TOK_IMAGE_URL:
                {
                    aStrURL = rStrValue;
                    bHasHRef = sal_True;
                }
                break;
            case XML_TOK_IMAGE_TYPE:
                // ignore
                break;
            case XML_TOK_IMAGE_SHOW:
                // ignore
                break;
            case XML_TOK_IMAGE_ACTUATE:
                // ignore
                break;
            default:
                DBG_WARNING( "Unknown token at import fill bitmap style" )
                ;
        }
    }

    rValue <<= aStrURL;

    bRet = bHasName && bHasHRef;

    return bRet;
}
