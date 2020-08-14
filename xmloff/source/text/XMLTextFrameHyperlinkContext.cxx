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

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

namespace drawing = com::sun::star::drawing;


XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    eDefaultAnchorType( eATyp ),
    bMap( false )
{
    OUString sShow;
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextHyperlinkAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_HYPERLINK_HREF:
            sHRef = GetImport().GetAbsoluteReference( rValue );
            break;
        case XML_TOK_TEXT_HYPERLINK_NAME:
            sName = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_TARGET_FRAME:
            sTargetFrameName = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_SHOW:
            sShow = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_SERVER_MAP:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool( bTmp, rValue ))
                {
                    bMap = bTmp;
                }
            }
            break;
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

void XMLTextFrameHyperlinkContext::EndElement()
{
}

SvXMLImportContextRef XMLTextFrameHyperlinkContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    XMLTextFrameContext *pTextFrameContext = nullptr;

    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_FRAME ) )
            pTextFrameContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                                rLocalName, xAttrList,
                                                eDefaultAnchorType );
    }

    if( pTextFrameContext )
    {
        pTextFrameContext->SetHyperlink( sHRef, sName, sTargetFrameName, bMap );
        pContext = pTextFrameContext;
        xFrameContext = pContext;
    }

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
