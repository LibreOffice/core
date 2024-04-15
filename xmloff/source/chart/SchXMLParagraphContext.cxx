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


#include "SchXMLParagraphContext.hxx"

#include <sal/log.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>

#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;

SchXMLParagraphContext::SchXMLParagraphContext( SvXMLImport& rImport,
                                                OUString& rText,
                                                OUString * pOutId /* = 0 */ ) :
        SvXMLImportContext( rImport ),
        mrText( rText ),
        mpId( pOutId )
{
}

SchXMLParagraphContext::~SchXMLParagraphContext()
{}

void SchXMLParagraphContext::startFastElement(
   sal_Int32 /*nElement*/,
   const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    // remember the id. It is used for storing the original cell range string in
    // a local table (cached data)
    if( !mpId )
        return;

    bool bHaveXmlId( false );

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(XML, XML_ID):
                (*mpId) = aIter.toString();
                bHaveXmlId = true;
                break;
            case XML_ELEMENT(TEXT, XML_ID):
            {   // text:id shall be ignored if xml:id exists
                if (!bHaveXmlId)
                {
                    (*mpId) = aIter.toString();
                }
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SchXMLParagraphContext::endFastElement(sal_Int32 )
{
    mrText = maBuffer.makeStringAndClear();
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLParagraphContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(TEXT, XML_TAB_STOP) )
    {
        maBuffer.append( u'\x0009');  // tabulator
    }
    else if( nElement == XML_ELEMENT(TEXT, XML_LINE_BREAK) )
    {
        maBuffer.append( u'\x000A');  // linefeed
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

void SchXMLParagraphContext::characters( const OUString& rChars )
{
    maBuffer.append( rChars );
}

SchXMLTitleParaContext::SchXMLTitleParaContext( SvXMLImport& rImport,
                                                std::vector<std::pair<OUString, OUString>>& rParaText,
                                                OUString * pOutId /* = 0 */ ) :
    SvXMLImportContext( rImport ),
    mrParaText( rParaText ),
    mpId( pOutId )
{
}

SchXMLTitleParaContext::~SchXMLTitleParaContext()
{}

void SchXMLTitleParaContext::startFastElement(
   sal_Int32 /*nElement*/,
   const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    // remember the id. It is used for storing the original cell range string in
    // a local table (cached data)
    if( !mpId )
        return;

    bool bHaveXmlId( false );

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                maStyleName = aIter.toString();
                break;
            case XML_ELEMENT(XML, XML_ID):
                (*mpId) = aIter.toString();
                bHaveXmlId = true;
                break;
            case XML_ELEMENT(TEXT, XML_ID):
            {   // text:id shall be ignored if xml:id exists
                if (!bHaveXmlId)
                {
                    (*mpId) = aIter.toString();
                }
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SchXMLTitleParaContext::endFastElement(sal_Int32 )
{
    if (!maBuffer.isEmpty())
        mrParaText.push_back(std::make_pair(maStyleName, maBuffer.makeStringAndClear()));
}

void SchXMLTitleParaContext::characters(const OUString& rChars)
{
    maBuffer.append(rChars);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTitleParaContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    if( nElement == XML_ELEMENT(TEXT, XML_SPAN) )
    {
        if (!maBuffer.isEmpty())
            mrParaText.push_back(std::make_pair(maStyleName, maBuffer.makeStringAndClear()));

        return new SchXMLTitleSpanContext(GetImport(), mrParaText, xAttrList);
    }
    else if( nElement == XML_ELEMENT(TEXT, XML_TAB_STOP) )
    {
        maBuffer.append( u'\x0009');  // tabulator
    }
    else if( nElement == XML_ELEMENT(TEXT, XML_LINE_BREAK) )
    {
        maBuffer.append( u'\x000A');  // linefeed
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

SchXMLTitleSpanContext::SchXMLTitleSpanContext(SvXMLImport& rImport, std::vector<std::pair<OUString, OUString>>& rSpanTexts,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList) :
    SvXMLImportContext(rImport),
    mrSpanTexts(rSpanTexts)
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME) )
        {
            maStyleName = aIter.toString();
            break;
        }
    }
}

SchXMLTitleSpanContext::~SchXMLTitleSpanContext()
{}

void SchXMLTitleSpanContext::characters(const OUString& rChars)
{
    maCharBuffer.append(rChars);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTitleSpanContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    if( nElement == XML_ELEMENT(TEXT, XML_TAB_STOP) )
    {
        maCharBuffer.append( u'\x0009');  // tabulator
    }
    else if( nElement == XML_ELEMENT(TEXT, XML_LINE_BREAK) )
    {
        maCharBuffer.append( u'\x000A');  // linefeed
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

void SchXMLTitleSpanContext::endFastElement(sal_Int32)
{
    if (!maCharBuffer.isEmpty())
        mrSpanTexts.push_back(std::make_pair(maStyleName, maCharBuffer.makeStringAndClear()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
