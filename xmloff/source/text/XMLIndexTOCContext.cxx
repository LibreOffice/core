/*************************************************************************
 *
 *  $RCSfile: XMLIndexTOCContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-14 14:42:50 $
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


#ifndef _XMLOFF_XMLINDEXTOCCONTEXT_HXX_
#include "XMLIndexTOCContext.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLINDEXTOCSOURCECONTEXT_HXX_
#include "XMLIndexTOCSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXOBJECTSOURCECONTEXT_HXX_
#include "XMLIndexObjectSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXALPHABETICALSOURCECONTEXT_HXX_
#include "XMLIndexAlphabeticalSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXUSERSOURCECONTEXT_HXX_
#include "XMLIndexUserSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYSOURCECONTEXT_HXX_
#include "XMLIndexBibliographySourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTABLESOURCECONTEXT_HXX_
#include "XMLIndexTableSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXILLUSTRATIONSOURCECONTEXT_HXX_
#include "XMLIndexIllustrationSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;


TYPEINIT1(XMLIndexTOCContext, SvXMLImportContext);

static const sal_Char* aIndexServiceMap[] =
{
    "com.sun.star.text.ContentIndex",
    "com.sun.star.text.DocumentIndex",
    "com.sun.star.text.TableIndex",
    "com.sun.star.text.ObjectIndex",
    "com.sun.star.text.Bibliography",
    "com.sun.star.text.UserIndex",
    "com.sun.star.text.IllustrationsIndex"
};

static const sal_Char* aIndexSourceElementMap[] =
{
    sXML_table_of_content_source,
    sXML_alphabetical_index_source,
    sXML_table_index_source,
    sXML_object_index_source,
    sXML_bibliography_source,
    sXML_user_index_source,
    sXML_illustration_index_source
};

SvXMLEnumMapEntry __READONLY_DATA aIndexTypeMap[] =
{
    { sXML_table_of_content,    TEXT_INDEX_TOC },
    { sXML_alphabetical_index,  TEXT_INDEX_ALPHABETICAL },
    { sXML_table_index,         TEXT_INDEX_TABLE },
    { sXML_object_index,        TEXT_INDEX_OBJECT },
    { sXML_bibliography,        TEXT_INDEX_BIBLIOGRAPHY },
    { sXML_user_index,          TEXT_INDEX_USER },
    { sXML_illustration_index,  TEXT_INDEX_ILLUSTRATION },
    { NULL, NULL }
};


XMLIndexTOCContext::XMLIndexTOCContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        pSourceElementName(NULL),
        bValid(sal_False)
{
    if (XML_NAMESPACE_TEXT == nPrfx)
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, rLocalName, aIndexTypeMap))
        {
            // check for array index:
            DBG_ASSERT(nTmp >= 0, "index too low");
            DBG_ASSERT(nTmp < (sizeof(aIndexServiceMap)/sizeof(sal_Char*)),
                       "index too high");
            DBG_ASSERT(sizeof(aIndexServiceMap) ==
                       sizeof(aIndexSourceElementMap),
                       "service and source element maps must be same size");

            eIndexType = (enum IndexTypeEnum)nTmp;
            pSourceElementName = aIndexSourceElementMap[eIndexType];
            bValid = sal_True;
        }
    }
}

XMLIndexTOCContext::~XMLIndexTOCContext()
{
}

void XMLIndexTOCContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    if (bValid)
    {
        // create table of content (via MultiServiceFactory)
        Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                                 UNO_QUERY);
        if( xFactory.is() )
        {
            Reference<XInterface> xIfc =
                xFactory->createInstance(
                    OUString::createFromAscii(aIndexServiceMap[eIndexType]));
            if( xIfc.is() )
            {
                Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
                GetImport().GetTextImport()->InsertTextContent(xTextContent);

                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);
                xTOCPropertySet = xPropSet;
            }
        }

        // find text:style-name attribute and set section style
        sal_Int32 nCount = xAttrList->getLength();
        for(sal_Int32 nAttr = 0; nAttr < nCount; nAttr++)
        {
            OUString sLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                                  &sLocalName );
            if ( (XML_NAMESPACE_TEXT == nPrefix) &&
                 (sLocalName.equalsAsciiL(sXML_style_name,
                                          sizeof(sXML_style_name)-1)) )
            {
                XMLPropStyleContext* pStyle =
                    GetImport().GetTextImport()->FindSectionStyle(
                        xAttrList->getValueByIndex(nAttr));
                if (pStyle != NULL)
                {
                    pStyle->FillPropertySet( xTOCPropertySet );
                }
            }
        }
    }
}

void XMLIndexTOCContext::EndElement()
{
    // nothing left
}

SvXMLImportContext* XMLIndexTOCContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (bValid)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (rLocalName.equalsAsciiL(sXML_index_body,
                                        sizeof(sXML_index_body)-1))
            {
                // TODO: text content
            }
            else if (0 == rLocalName.compareToAscii(pSourceElementName))
            {
                // instantiate source context for this index type
                switch (eIndexType)
                {
                    case TEXT_INDEX_TOC:
                        pContext = new XMLIndexTOCSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_OBJECT:
                        pContext = new XMLIndexObjectSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ALPHABETICAL:
                        pContext = new XMLIndexAlphabeticalSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_USER:
                        pContext = new XMLIndexUserSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_BIBLIOGRAPHY:
                        pContext = new XMLIndexBibliographySourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_TABLE:
                        pContext = new XMLIndexTableSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ILLUSTRATION:
                        pContext = new XMLIndexIllustrationSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    default:
                        DBG_ERROR("index type not implemented");
                        break;
                }
            }
            // else: ignore
        }
        // else: no text: namespace -> ignore
    }
    // else: not valid -> ignore

    // default: ignore
    if (pContext == NULL)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}
