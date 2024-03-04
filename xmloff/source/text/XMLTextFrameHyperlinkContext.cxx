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

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/shapeimport.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"

#include <com/sun/star/drawing/XShapes.hpp>

using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

namespace drawing = com::sun::star::drawing;


XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< XFastAttributeList > & xAttrList,
        TextContentAnchorType eATyp ) :
    SvXMLImportContext( rImport ),
    eDefaultAnchorType( eATyp ),
    bMap( false )
{
    OUString sShow;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(XLINK, XML_HREF):
            sHRef = GetImport().GetAbsoluteReference( aIter.toString() );
            break;
        case XML_ELEMENT(OFFICE, XML_NAME):
            sName = aIter.toString();
            break;
        case XML_ELEMENT(OFFICE, XML_TARGET_FRAME_NAME):
            sTargetFrameName = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_SHOW):
            sShow = aIter.toString();
            break;
        case XML_ELEMENT(OFFICE, XML_SERVER_MAP):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool( bTmp, aIter.toView() ))
                {
                    bMap = bTmp;
                }
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !sShow.isEmpty() && sTargetFrameName.isEmpty() )
    {
        if( IsXMLToken( sShow, XML_NEW ) )
            sTargetFrameName = "_blank";
        else if( IsXMLToken( sShow, XML_REPLACE ) )
            sTargetFrameName = "_self";
    }
}

XMLTextFrameHyperlinkContext::~XMLTextFrameHyperlinkContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextFrameHyperlinkContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    XMLTextFrameContext *pTextFrameContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT(DRAW, XML_FRAME):
        {
            pTextFrameContext = new XMLTextFrameContext(GetImport(), xAttrList, eDefaultAnchorType);
            pTextFrameContext->SetHyperlink(sHRef, sName, sTargetFrameName, bMap);
            pContext = pTextFrameContext;
            xFrameContext = pContext;
        }
        break;
        case XML_ELEMENT(DRAW, XML_CUSTOM_SHAPE):
        case XML_ELEMENT(DRAW, XML_PATH):
        case XML_ELEMENT(DRAW, XML_ELLIPSE):
        case XML_ELEMENT(DRAW, XML_LINE):
        case XML_ELEMENT(DRAW, XML_RECT):
        case XML_ELEMENT(DRAW, XML_CAPTION):
        case XML_ELEMENT(DRAW, XML_POLYGON):
        case XML_ELEMENT(DRAW, XML_POLYLINE):
        case XML_ELEMENT(DRAW, XML_MEASURE):
        case XML_ELEMENT(DRAW, XML_CIRCLE):
        case XML_ELEMENT(DRAW, XML_CONNECTOR):
        case XML_ELEMENT(DRAW, XML_CONTROL):
        case XML_ELEMENT(DRAW, XML_PAGE_THUMBNAIL):
        case XML_ELEMENT(DRAW, XML_G):
        case XML_ELEMENT(DR3D, XML_SCENE):
        {
            Reference<XShapes> xShapes;
            SvXMLShapeContext* pShapeContext = XMLShapeImportHelper::CreateGroupChildContext(
                GetImport(), nElement, xAttrList, xShapes);
            pShapeContext->setHyperlink(sHRef);
            pContext = pShapeContext;
        }
        break;
    }

    if (!pContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return pContext;
}


TextContentAnchorType XMLTextFrameHyperlinkContext::GetAnchorType() const
{
    if( xFrameContext.is() )
    {
        SvXMLImportContext *pContext = xFrameContext.get();
        return dynamic_cast<XMLTextFrameContext&>(*pContext).GetAnchorType();
    }
    else
        return eDefaultAnchorType;

}

Reference < XTextContent > XMLTextFrameHyperlinkContext::GetTextContent() const
{
    Reference <XTextContent > xTxt;
    if( xFrameContext.is() )
    {
        SvXMLImportContext *pContext = xFrameContext.get();
        xTxt = dynamic_cast<XMLTextFrameContext&>(*pContext).GetTextContent();
    }

    return xTxt;
}

// Frame "to character": anchor moves from first to last char after saving (#i33242#)
Reference < drawing::XShape > XMLTextFrameHyperlinkContext::GetShape() const
{
    Reference < drawing::XShape > xShape;
    if( xFrameContext.is() )
    {
        SvXMLImportContext *pContext = xFrameContext.get();
        xShape = dynamic_cast<XMLTextFrameContext&>(*pContext).GetShape();
    }

    return xShape;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
