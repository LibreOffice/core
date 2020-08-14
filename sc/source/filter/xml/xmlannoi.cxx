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
#include <xmloff/xmlnmspe.hxx>
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                      ScXMLAnnotationData& rAnnotationData) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    mrAnnotationData( rAnnotationData )
{
    uno::Reference<drawing::XShapes> xLocalShapes (GetScImport().GetTables().GetCurrentXShapes());
    if (xLocalShapes.is())
    {
        XMLTableShapeImportHelper* pTableShapeImport = static_cast<XMLTableShapeImportHelper*>(GetScImport().GetShapeImport().get());
        pTableShapeImport->SetAnnotation(this);
        pShapeContext.reset( GetScImport().GetShapeImport()->CreateGroupChildContext(
            GetScImport(), nPrfx, rLName, xAttrList, xLocalShapes, true) );
    }

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAnnotationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR:
            {
                maAuthorBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE:
            {
                maCreateDateBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING:
            {
                maCreateDateStringBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY:
            {
                mrAnnotationData.mbShown = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_X:
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_Y:
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
        }
    }
}

ScXMLAnnotationContext::~ScXMLAnnotationContext()
{
}

void ScXMLAnnotationContext::StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList)
{
    if (pShapeContext)
        pShapeContext->StartElement(xAttrList);
}

SvXMLImportContextRef ScXMLAnnotationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLName, XML_CREATOR ) )
            xContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, maAuthorBuffer);
        else if( IsXMLToken( rLName, XML_DATE ) )
            xContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, maCreateDateBuffer);
    }
    else if( XML_NAMESPACE_META == nPrefix )
    {
        if( IsXMLToken( rLName, XML_DATE_STRING ) )
            xContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, maCreateDateStringBuffer);
    }

    if( !xContext && pShapeContext )
        xContext = pShapeContext->CreateChildContext(nPrefix, rLName, xAttrList);

    return xContext;
}

void ScXMLAnnotationContext::Characters( const OUString& rChars )
{
    maTextBuffer.append(rChars);
}

void ScXMLAnnotationContext::EndElement()
{
    if (pShapeContext)
    {
        pShapeContext->EndElement();
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
