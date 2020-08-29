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

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLTableShapeImportHelper.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

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
        const OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(OFFICE, XML_AUTHOR):
            {
                maAuthorBuffer = sValue;
            }
            break;
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE):
            {
                maCreateDateBuffer = sValue;
            }
            break;
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE_STRING):
            {
                maCreateDateStringBuffer = sValue;
            }
            break;
            case XML_ELEMENT(OFFICE, XML_DISPLAY):
            {
                mrAnnotationData.mbShown = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_ELEMENT(SVG, XML_X):
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
            case XML_ELEMENT(SVG, XML_Y):
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
            default:
            SAL_WARN("sc", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
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
    }

    if( pShapeContext )
        return pShapeContext->createFastChildContext(nElement, xAttrList);

    SAL_WARN("sc", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
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
