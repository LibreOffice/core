/*************************************************************************
 *
 *  $RCSfile: ImageStyle.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:19:27 $
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_IMAGE_NAME,
    XML_TOK_IMAGE_DISPLAY_NAME,
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
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_IMAGE_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_IMAGE_DISPLAY_NAME },
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },
    { XML_NAMESPACE_XLINK, XML_TYPE, XML_TOK_IMAGE_TYPE },
    { XML_NAMESPACE_XLINK, XML_SHOW, XML_TOK_IMAGE_SHOW },
    { XML_NAMESPACE_XLINK, XML_ACTUATE, XML_TOK_IMAGE_ACTUATE },
    /*{ XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },*/
    XML_TOKEN_MAP_END
};

XMLImageStyle::XMLImageStyle()
{
}

XMLImageStyle::~XMLImageStyle()
{
}

#ifndef SVX_LIGHT

sal_Bool XMLImageStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue, SvXMLExport& rExport )
{
    return ImpExportXML( rStrName, rValue, rExport );
}

sal_Bool XMLImageStyle::ImpExportXML( const OUString& rStrName, const uno::Any& rValue, SvXMLExport& rExport )
{
    sal_Bool bRet = sal_False;

    OUString sImageURL;

    if( rStrName.getLength() )
    {
        if( rValue >>= sImageURL )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            // Name
            sal_Bool bEncoded = sal_False;
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                  rExport.EncodeStyleName( rStrName,
                                                           &bEncoded ) );
            if( bEncoded )
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                      rStrName );

            // uri
            const OUString aStr( rExport.AddEmbeddedGraphicObject( sImageURL ) );
            if( aStr.getLength() )
            {
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStr );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }
/*
            // size
            awt::Size aSize = xBitmap->getSize();

            rUnitConverter.convertNumber( aOut, aSize.Width );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH, aStrValue );

            rUnitConverter.convertNumber( aOut, aSize.Height );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT, aStrValue );
*/
            // Do Write
            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_FILL_IMAGE, sal_True, sal_True );

            if( sImageURL.getLength() )
            {
                // optional office:binary-data
                rExport.AddEmbeddedGraphicObjectAsBase64( sImageURL );
            }
        }
    }

    return bRet;
}

#endif // #ifndef SVX_LIGHT

sal_Bool XMLImageStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName, SvXMLImport& rImport )
{
    return ImpImportXML( xAttrList, rValue, rStrName, rImport );
}

sal_Bool XMLImageStyle::ImpImportXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                      uno::Any& rValue, OUString& rStrName,
                                      SvXMLImport& rImport )
{
    sal_Bool bRet     = sal_False;
    sal_Bool bHasHRef = sal_False;
    sal_Bool bHasName = sal_False;
    OUString aStrURL;
    OUString aDisplayName;

    SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
            case XML_TOK_IMAGE_NAME:
                {
                    rStrName = rStrValue;
                    bHasName = sal_True;
                }
                break;
            case XML_TOK_IMAGE_DISPLAY_NAME:
                {
                    aDisplayName = rStrValue;
                }
                break;
            case XML_TOK_IMAGE_URL:
                {
                    aStrURL = rImport.ResolveGraphicObjectURL( rStrValue, sal_False );
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

    if( aDisplayName.getLength() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_FILL_IMAGE_ID,
                                     rStrName, aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasHRef;

    return bRet;
}
