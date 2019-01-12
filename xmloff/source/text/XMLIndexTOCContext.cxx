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

#include "XMLIndexTOCContext.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sax/tools/converter.hxx>
#include "XMLIndexTOCSourceContext.hxx"
#include "XMLIndexObjectSourceContext.hxx"
#include "XMLIndexAlphabeticalSourceContext.hxx"
#include "XMLIndexUserSourceContext.hxx"
#include "XMLIndexBibliographySourceContext.hxx"
#include "XMLIndexTableSourceContext.hxx"
#include "XMLIndexIllustrationSourceContext.hxx"
#include "XMLIndexBodyContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::IllegalArgumentException;


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

static const XMLTokenEnum aIndexSourceElementMap[] =
{
    XML_TABLE_OF_CONTENT_SOURCE,
    XML_ALPHABETICAL_INDEX_SOURCE,
    XML_TABLE_INDEX_SOURCE,
    XML_OBJECT_INDEX_SOURCE,
    XML_BIBLIOGRAPHY_SOURCE,
    XML_USER_INDEX_SOURCE,
    XML_ILLUSTRATION_INDEX_SOURCE
};

SvXMLEnumMapEntry<IndexTypeEnum> const aIndexTypeMap[] =
{
    { XML_TABLE_OF_CONTENT,     TEXT_INDEX_TOC },
    { XML_ALPHABETICAL_INDEX,   TEXT_INDEX_ALPHABETICAL },
    { XML_TABLE_INDEX,          TEXT_INDEX_TABLE },
    { XML_OBJECT_INDEX,         TEXT_INDEX_OBJECT },
    { XML_BIBLIOGRAPHY,         TEXT_INDEX_BIBLIOGRAPHY },
    { XML_USER_INDEX,           TEXT_INDEX_USER },
    { XML_ILLUSTRATION_INDEX,   TEXT_INDEX_ILLUSTRATION },
    { XML_TOKEN_INVALID,        IndexTypeEnum(0) }
};


XMLIndexTOCContext::XMLIndexTOCContext(SvXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
    , eIndexType(TEXT_INDEX_UNKNOWN)
    , bValid(false)
{
    if (XML_NAMESPACE_TEXT == nPrfx)
    {
        if (SvXMLUnitConverter::convertEnum(eIndexType, rLocalName, aIndexTypeMap))
        {
            // check for array index:
            OSL_ENSURE(unsigned(eIndexType) < (SAL_N_ELEMENTS(aIndexServiceMap)), "index out of range");
            OSL_ENSURE(SAL_N_ELEMENTS(aIndexServiceMap) ==
                       SAL_N_ELEMENTS(aIndexSourceElementMap),
                       "service and source element maps must be same size");
            bValid = true;
        }
    }
}

XMLIndexTOCContext::~XMLIndexTOCContext()
{
}

void XMLIndexTOCContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    if (!bValid)
        return;

    // find text:style-name attribute and set section style
    // find text:protected and set value
    // find text:name and set value (if not empty)
    sal_Int16 nCount = xAttrList->getLength();
    bool bProtected = false;
    OUString sIndexName;
    OUString sXmlId;
    XMLPropStyleContext* pStyle(nullptr);
    for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
            {
                pStyle = GetImport().GetTextImport()->FindSectionStyle(
                            xAttrList->getValueByIndex(nAttr));
            }
            else if ( IsXMLToken( sLocalName, XML_PROTECTED ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(
                     bTmp, xAttrList->getValueByIndex(nAttr)))
                {
                    bProtected = bTmp;
                }
            }
            else if ( IsXMLToken( sLocalName, XML_NAME ) )
            {
                sIndexName = xAttrList->getValueByIndex(nAttr);
            }
        }
        else if ( XML_NAMESPACE_XML == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_ID ) )
            {
                sXmlId = xAttrList->getValueByIndex(nAttr);
            }
        }
    }

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
            // get Property set
            Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);
            xTOCPropertySet = xPropSet;

            // insert section
            // a) insert section
            //    The inserted index consists of an empty paragraph
            //    only, as well as an empty paragraph *after* the index
            // b) insert marker after index, and put Cursor inside of the
            //    index

            // preliminaries
#ifndef DBG_UTIL
            OUString const sMarker(" ");
#else
            OUString const sMarker("Y");
#endif
            rtl::Reference<XMLTextImportHelper> rImport =
                GetImport().GetTextImport();

            // a) insert index
            Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
            try
            {
                GetImport().GetTextImport()->InsertTextContent(
                    xTextContent);
            }
            catch(const IllegalArgumentException& e)
            {
                // illegal argument? Then we can't accept indices here!
                Sequence<OUString> aSeq { GetLocalName() };
                GetImport().SetError(
                    XMLERROR_FLAG_ERROR | XMLERROR_NO_INDEX_ALLOWED_HERE,
                    aSeq, e.Message, nullptr );

                // set bValid to false, and return prematurely
                bValid = false;
                return;
            }

            // xml:id for RDF metadata
            GetImport().SetXmlId(xIfc, sXmlId);

            // b) insert marker and move cursor
            rImport->InsertString(sMarker);
            rImport->GetCursor()->goLeft(2, false);
        }
    }

    // finally, check for redlines that should start at
    // the section start node
    if( bValid )
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(true);

    if (pStyle != nullptr)
    {
        pStyle->FillPropertySet( xTOCPropertySet );
    }

    xTOCPropertySet->setPropertyValue( "IsProtected", Any(bProtected) );

    if (!sIndexName.isEmpty())
    {
        xTOCPropertySet->setPropertyValue( "Name", Any(sIndexName) );
    }

}

void XMLIndexTOCContext::EndElement()
{
    // complete import of index by removing the markers (if the index
    // was actually inserted, that is)
    if( bValid )
    {
        // preliminaries
        rtl::Reference<XMLTextImportHelper> rHelper= GetImport().GetTextImport();

        // get rid of last paragraph (unless it's the only paragraph)
        rHelper->GetCursor()->goRight(1, false);
        if( xBodyContextRef.is() &&
            static_cast<XMLIndexBodyContext*>(xBodyContextRef.get())->HasContent() )
        {
            rHelper->GetCursor()->goLeft(1, true);
            rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                             "", true);
        }

        // and delete second marker
        rHelper->GetCursor()->goRight(1, true);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                         "", true);

        // check for Redlines on our end node
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(false);
    }
}

SvXMLImportContextRef XMLIndexTOCContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContextRef xContext;

    if (bValid)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName, XML_INDEX_BODY ) )
            {
                xContext = new XMLIndexBodyContext(GetImport(), nPrefix,
                                                   rLocalName);
                if ( !xBodyContextRef.is() ||
                     !static_cast<XMLIndexBodyContext*>(xBodyContextRef.get())->HasContent() )
                {
                    xBodyContextRef = xContext;
                }
            }
            else if (IsXMLToken(rLocalName, aIndexSourceElementMap[eIndexType]))
            {
                // instantiate source context for the appropriate index type
                switch (eIndexType)
                {
                    case TEXT_INDEX_TOC:
                        xContext = new XMLIndexTOCSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_OBJECT:
                        xContext = new XMLIndexObjectSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ALPHABETICAL:
                        xContext = new XMLIndexAlphabeticalSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_USER:
                        xContext = new XMLIndexUserSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_BIBLIOGRAPHY:
                        xContext = new XMLIndexBibliographySourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_TABLE:
                        xContext = new XMLIndexTableSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ILLUSTRATION:
                        xContext = new XMLIndexIllustrationSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    default:
                        OSL_FAIL("index type not implemented");
                        break;
                }
            }
            // else: ignore
        }
        // else: no text: namespace -> ignore
    }
    // else: not valid -> ignore

    // default: ignore
    if (!xContext)
    {
        xContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
