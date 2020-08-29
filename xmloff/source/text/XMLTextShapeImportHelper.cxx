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

#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/XMLTextShapeImportHelper.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

const OUStringLiteral gsAnchorType(u"AnchorType");
const OUStringLiteral gsAnchorPageNo(u"AnchorPageNo");
const OUStringLiteral gsVertOrientPosition(u"VertOrientPosition");

XMLTextShapeImportHelper::XMLTextShapeImportHelper(
        SvXMLImport& rImp ) :
    XMLShapeImportHelper( rImp, rImp.GetModel(),
                          XMLTextImportHelper::CreateShapeExtPropMapper(rImp) ),
    rImport( rImp )
{
    Reference < XDrawPageSupplier > xDPS( rImp.GetModel(), UNO_QUERY );
    if( xDPS.is() )
    {
        Reference < XShapes > xShapes = xDPS->getDrawPage();
        pushGroupForPostProcessing( xShapes );
    }

}

XMLTextShapeImportHelper::~XMLTextShapeImportHelper()
{
    popGroupAndPostProcess();
}

void XMLTextShapeImportHelper::addShape(
    Reference< XShape >& rShape,
    const Reference< XFastAttributeList >& xAttrList,
    Reference< XShapes >& rShapes )
{
    if( rShapes.is() )
    {
        // It's a group shape or 3DScene , so we have to call the base class method.
        XMLShapeImportHelper::addShape( rShape, xAttrList, rShapes );
        return;
    }

    TextContentAnchorType eAnchorType = TextContentAnchorType_AT_PARAGRAPH;
    sal_Int16   nPage = 0;
    sal_Int32   nY = 0;

    rtl::Reference < XMLTextImportHelper > xTxtImport =
        rImport.GetTextImport();

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        const OUString sValue = aIter.toString();
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(TEXT, XML_ANCHOR_TYPE):
            {
                TextContentAnchorType eNew;
                // OD 2004-06-01 #i26791# - allow all anchor types
                if ( XMLAnchorTypePropHdl::convert( sValue, eNew ) )
                {
                    eAnchorType = eNew;
                }
            }
            break;
        case XML_ELEMENT(TEXT, XML_ANCHOR_PAGE_NUMBER):
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp, sValue, 1, SHRT_MAX))
                    nPage = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_ELEMENT(SVG, XML_Y):
            rImport.GetMM100UnitConverter().convertMeasureToCore( nY, sValue );
            break;
        }
    }

    Reference < XPropertySet > xPropSet( rShape, UNO_QUERY );

    // anchor type
    xPropSet->setPropertyValue( gsAnchorType, Any(eAnchorType) );

    Reference < XTextContent > xTxtCntnt( rShape, UNO_QUERY );
    xTxtImport->InsertTextContent( xTxtCntnt );

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    switch( eAnchorType )
    {
    case TextContentAnchorType_AT_PAGE:
        // only set positive page numbers
        if ( nPage > 0 )
        {
            xPropSet->setPropertyValue( gsAnchorPageNo, Any(nPage) );
        }
        break;
    case TextContentAnchorType_AS_CHARACTER:
        xPropSet->setPropertyValue( gsVertOrientPosition, Any(nY) );
        break;
    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
