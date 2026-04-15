/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLTableShapeImportHelper.hxx"

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

namespace {

/// Context handler for <loext:reply> inside <loext:threaded-comment>.
class ScXMLThreadedCommentReplyContext : public ScXMLImportContext
{
public:
    ScXMLThreadedCommentReplyContext(ScXMLImport& rImport,
                                    const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                                    ScXMLAnnotationData& rData)
        : ScXMLImportContext(rImport)
        , mrData(rData)
    {
        for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(LO_EXT, XML_ID):
                    maEntry.maGuid = aIter.toString();
                    break;
                case XML_ELEMENT(LO_EXT, XML_DATE_TIME):
                    maEntry.maDateTime = aIter.toString();
                    break;
                case XML_ELEMENT(LO_EXT, XML_PERSON_ID):
                    maEntry.maPersonId = aIter.toString();
                    break;
            }
        }
    }

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>&) override
    {
        if (nElement == XML_ELEMENT(LO_EXT, XML_TEXT))
            return new ScXMLContentContext(GetScImport(), maTextBuffer);
        return nullptr;
    }

    void SAL_CALL endFastElement(sal_Int32) override
    {
        maEntry.maText = maTextBuffer.makeStringAndClear();
        if (mrData.mpThreadedCommentData)
            mrData.mpThreadedCommentData->maReplies.push_back(std::move(maEntry));
    }

private:
    ScXMLAnnotationData& mrData;
    ScThreadedCommentEntry maEntry;
    OUStringBuffer maTextBuffer;
};

/// Context handler for <loext:threaded-comment> inside <office:annotation>.
class ScXMLThreadedCommentContext : public ScXMLImportContext
{
public:
    ScXMLThreadedCommentContext(ScXMLImport& rImport,
                               const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                               ScXMLAnnotationData& rData)
        : ScXMLImportContext(rImport)
        , mrData(rData)
    {
        mrData.mpThreadedCommentData = std::make_unique<ScThreadedCommentData>();

        for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(LO_EXT, XML_ID):
                    mrData.mpThreadedCommentData->maRoot.maGuid = aIter.toString();
                    break;
                case XML_ELEMENT(LO_EXT, XML_DATE_TIME):
                    mrData.mpThreadedCommentData->maRoot.maDateTime = aIter.toString();
                    break;
                case XML_ELEMENT(LO_EXT, XML_RESOLVED):
                    mrData.mpThreadedCommentData->mbDone = IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_ELEMENT(LO_EXT, XML_PERSON_ID):
                    mrData.mpThreadedCommentData->maRoot.maPersonId = aIter.toString();
                    break;
            }
        }
    }

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override
    {
        if (nElement == XML_ELEMENT(LO_EXT, XML_TEXT))
            return new ScXMLContentContext(GetScImport(), maTextBuffer);
        if (nElement == XML_ELEMENT(LO_EXT, XML_REPLY))
            return new ScXMLThreadedCommentReplyContext(GetScImport(), xAttrList, mrData);
        return nullptr;
    }

    void SAL_CALL endFastElement(sal_Int32) override
    {
        if (mrData.mpThreadedCommentData)
            mrData.mpThreadedCommentData->maRoot.maText = maTextBuffer.makeStringAndClear();
    }

private:
    ScXMLAnnotationData& mrData;
    OUStringBuffer maTextBuffer;
};

} // anonymous namespace

ScXMLAnnotationData::ScXMLAnnotationData() :
    mbUseShapePos( false ),
    mbShown( false )
{
}

ScXMLAnnotationData::~ScXMLAnnotationData()
{
}

ScXMLAnnotationContext::ScXMLAnnotationContext( ScXMLImport& rImport,
                                      sal_Int32 nElement,
                                      const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                      ScXMLAnnotationData& rAnnotationData) :
    ScXMLImportContext( rImport ),
    mrAnnotationData( rAnnotationData )
{
    uno::Reference<drawing::XShapes> xLocalShapes (GetScImport().GetTables().GetCurrentXShapes());
    if (xLocalShapes.is())
    {
        XMLTableShapeImportHelper* pTableShapeImport = static_cast<XMLTableShapeImportHelper*>(GetScImport().GetShapeImport().get());
        pTableShapeImport->SetAnnotation(this);
        pShapeContext.reset( XMLShapeImportHelper::CreateGroupChildContext(
            GetScImport(), nElement, xAttrList, xLocalShapes, true) );
    }

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(OFFICE, XML_AUTHOR):
            {
                maAuthorBuffer = aIter.toString();
            }
            break;
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE):
            {
                maCreateDateBuffer = aIter.toString();
            }
            break;
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE_STRING):
            {
                maCreateDateStringBuffer = aIter.toString();
            }
            break;
            case XML_ELEMENT(OFFICE, XML_DISPLAY):
            {
                mrAnnotationData.mbShown = IsXMLToken(aIter, XML_TRUE);
            }
            break;
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
            default:
                XMLOFF_INFO_UNKNOWN("xmloff", aIter);
        }
    }
}

ScXMLAnnotationContext::~ScXMLAnnotationContext()
{
}

void ScXMLAnnotationContext::startFastElement(sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList)
{
    if (pShapeContext)
        pShapeContext->startFastElement(nElement, xAttrList);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > ScXMLAnnotationContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch (nElement)
    {
        case XML_ELEMENT(DC, XML_CREATOR):
            return new ScXMLContentContext(GetScImport(), maAuthorBuffer);
        case XML_ELEMENT(DC, XML_DATE):
            return new ScXMLContentContext(GetScImport(), maCreateDateBuffer);
        case XML_ELEMENT(META, XML_DATE_STRING):
            return new ScXMLContentContext(GetScImport(), maCreateDateStringBuffer);
        case XML_ELEMENT(LO_EXT, XML_THREADED_COMMENT):
            return new ScXMLThreadedCommentContext(GetScImport(), xAttrList, mrAnnotationData);
    }

    if( pShapeContext )
    {
        auto p = pShapeContext->createFastChildContext(nElement, xAttrList);
        if (p)
            return p;
    }

    XMLOFF_WARN_UNKNOWN_ELEMENT("sc", nElement);
    return nullptr;
}

void ScXMLAnnotationContext::characters( const OUString& rChars )
{
    maTextBuffer.append(rChars);
}

void ScXMLAnnotationContext::endFastElement(sal_Int32 nElement)
{
    if (pShapeContext)
    {
        pShapeContext->endFastElement(nElement);
        pShapeContext.reset();
    }

    mrAnnotationData.maAuthor = maAuthorBuffer.makeStringAndClear();
    mrAnnotationData.maCreateDate = maCreateDateBuffer.makeStringAndClear();
    if (mrAnnotationData.maCreateDate.isEmpty())
        mrAnnotationData.maCreateDate = maCreateDateStringBuffer.makeStringAndClear();
    mrAnnotationData.maSimpleText = maTextBuffer.makeStringAndClear();

    XMLTableShapeImportHelper* pTableShapeImport = static_cast<XMLTableShapeImportHelper*>(GetScImport().GetShapeImport().get());
    pTableShapeImport->SetAnnotation(nullptr);
}

void ScXMLAnnotationContext::SetShape( const uno::Reference< drawing::XShape >& rxShape, const uno::Reference< drawing::XShapes >& rxShapes,
                                       const OUString& rStyleName, const OUString& rTextStyle )
{
    mrAnnotationData.mxShape = rxShape;
    mrAnnotationData.mxShapes = rxShapes;
    mrAnnotationData.maStyleName = rStyleName;
    mrAnnotationData.maTextStyle = rTextStyle;
}

void ScXMLAnnotationContext::AddContentStyle( XmlStyleFamily nFamily, const OUString& rName, const ESelection& rSelection )
{
    mrAnnotationData.maContentStyles.emplace_back( nFamily, rName, rSelection );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
