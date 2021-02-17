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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sax/tools/converter.hxx>
#include <sal/log.hxx>
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
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
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


static const char* aIndexServiceMap[] =
{
    "com.sun.star.text.ContentIndex",
    "com.sun.star.text.DocumentIndex",
    "com.sun.star.text.TableIndex",
    "com.sun.star.text.ObjectIndex",
    "com.sun.star.text.Bibliography",
    "com.sun.star.text.UserIndex",
    "com.sun.star.text.IllustrationsIndex"
};

const XMLTokenEnum aIndexSourceElementMap[] =
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
    sal_Int32 nElement)
    : SvXMLImportContext(rImport)
    , eIndexType(TEXT_INDEX_UNKNOWN)
    , bValid(false)
{
    if (IsTokenInNamespace(nElement, XML_NAMESPACE_TEXT))
    {
        if (SvXMLUnitConverter::convertEnum(eIndexType, SvXMLImport::getNameFromToken(nElement), aIndexTypeMap))
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

void XMLIndexTOCContext::startFastElement(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if (!bValid)
        return;

    // find text:style-name attribute and set section style
    // find text:protected and set value
    // find text:name and set value (if not empty)
    bool bProtected = false;
    OUString sIndexName;
    OUString sXmlId;
    XMLPropStyleContext* pStyle(nullptr);
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            {
                pStyle = GetImport().GetTextImport()->FindSectionStyle(
                            aIter.toString());
                break;
            }
            case XML_ELEMENT(TEXT, XML_PROTECTED):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                {
                    bProtected = bTmp;
                }
                break;
            }
            case XML_ELEMENT(TEXT, XML_NAME):
            {
                sIndexName = aIter.toString();
                break;
            }
            case XML_ELEMENT(XML, XML_ID):
            {
                sXmlId = aIter.toString();
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
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
                Sequence<OUString> aSeq { SvXMLImport::getNameFromToken(nElement) };
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
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor();

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

void XMLIndexTOCContext::endFastElement(sal_Int32 )
{
    // complete import of index by removing the markers (if the index
    // was actually inserted, that is)
    if( bValid )
    {
        // preliminaries
        rtl::Reference<XMLTextImportHelper> rHelper= GetImport().GetTextImport();

        // get rid of last paragraph (unless it's the only paragraph)
        rHelper->GetCursor()->goRight(1, false);
        if( xBodyContextRef.is() && xBodyContextRef->HasContent() )
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
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor();
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexTOCContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContextRef xContext;

    // not valid -> ignore
    if (!bValid)
        return nullptr;

    if (nElement == XML_ELEMENT(TEXT, XML_INDEX_BODY) )
    {
        rtl::Reference<XMLIndexBodyContext> xNewBodyContext = new XMLIndexBodyContext(GetImport());
        xContext = xNewBodyContext;
        if ( !xBodyContextRef.is() || !xBodyContextRef->HasContent() )
        {
            xBodyContextRef = xNewBodyContext;
        }
    }
    else if (nElement == XML_ELEMENT(TEXT, aIndexSourceElementMap[eIndexType]))
    {
        // instantiate source context for the appropriate index type
        switch (eIndexType)
        {
            case TEXT_INDEX_TOC:
                xContext = new XMLIndexTOCSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_OBJECT:
                xContext = new XMLIndexObjectSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_ALPHABETICAL:
                xContext = new XMLIndexAlphabeticalSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_USER:
                xContext = new XMLIndexUserSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_BIBLIOGRAPHY:
                xContext = new XMLIndexBibliographySourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_TABLE:
                xContext = new XMLIndexTableSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            case TEXT_INDEX_ILLUSTRATION:
                xContext = new XMLIndexIllustrationSourceContext(
                    GetImport(), xTOCPropertySet);
                break;

            default:
                OSL_FAIL("index type not implemented");
                break;
        }
    }
    // else: ignore

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
