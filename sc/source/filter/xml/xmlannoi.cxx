/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLTableShapeImportHelper.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLAnnotationData::ScXMLAnnotationData() :
    mbUseShapePos( false ),
    mbShown( false )
{
}

ScXMLAnnotationData::~ScXMLAnnotationData()
{
}

//------------------------------------------------------------------

ScXMLAnnotationContext::ScXMLAnnotationContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                      ScXMLAnnotationData& rAnnotationData,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrAnnotationData( rAnnotationData ),
    pCellContext(pTempCellContext),
    pShapeContext(NULL)
{
    uno::Reference<drawing::XShapes> xLocalShapes (GetScImport().GetTables().GetCurrentXShapes());
    if (xLocalShapes.is())
    {
        XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
        pTableShapeImport->SetAnnotation(this);
        pShapeContext = GetScImport().GetShapeImport()->CreateGroupChildContext(
            GetScImport(), nPrfx, rLName, xAttrList, xLocalShapes, true);
    }

    pCellContext = pTempCellContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAnnotationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

void ScXMLAnnotationContext::StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList)
{
    if (pShapeContext)
        pShapeContext->StartElement(xAttrList);
}

SvXMLImportContext *ScXMLAnnotationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLName, XML_CREATOR ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maAuthorBuffer);
        else if( IsXMLToken( rLName, XML_DATE ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maCreateDateBuffer);
    }
    else if( XML_NAMESPACE_META == nPrefix )
    {
        if( IsXMLToken( rLName, XML_DATE_STRING ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maCreateDateStringBuffer);
    }

    if( !pContext && pShapeContext )
        pContext = pShapeContext->CreateChildContext(nPrefix, rLName, xAttrList);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLAnnotationContext::Characters( const ::rtl::OUString& rChars )
{
    maTextBuffer.append(rChars);
}

void ScXMLAnnotationContext::EndElement()
{
    if (pShapeContext)
    {
        pShapeContext->EndElement();
        delete pShapeContext;
        pShapeContext = NULL;
    }

    mrAnnotationData.maAuthor = maAuthorBuffer.makeStringAndClear();
    mrAnnotationData.maCreateDate = maCreateDateBuffer.makeStringAndClear();
    if (mrAnnotationData.maCreateDate.isEmpty())
        mrAnnotationData.maCreateDate = maCreateDateStringBuffer.makeStringAndClear();
    mrAnnotationData.maSimpleText = maTextBuffer.makeStringAndClear();

    XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
    pTableShapeImport->SetAnnotation(NULL);
}

void ScXMLAnnotationContext::SetShape( const uno::Reference< drawing::XShape >& rxShape, const uno::Reference< drawing::XShapes >& rxShapes,
                                       const rtl::OUString& rStyleName, const rtl::OUString& rTextStyle )
{
    mrAnnotationData.mxShape = rxShape;
    mrAnnotationData.mxShapes = rxShapes;
    mrAnnotationData.maStyleName = rStyleName;
    mrAnnotationData.maTextStyle = rTextStyle;
}

void ScXMLAnnotationContext::AddContentStyle( sal_uInt16 nFamily, const rtl::OUString& rName, const ESelection& rSelection )
{
    mrAnnotationData.maContentStyles.push_back( ScXMLAnnotationStyleEntry( nFamily, rName, rSelection ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
