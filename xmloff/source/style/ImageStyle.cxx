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

#include <xmloff/ImageStyle.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <xmloff/xmltkmap.hxx>

using namespace css;
using namespace xmloff::token;

namespace {

enum SvXMLTokenMapAttrs
{
    XML_TOK_IMAGE_NAME,
    XML_TOK_IMAGE_DISPLAY_NAME,
    XML_TOK_IMAGE_URL,
    XML_TOK_IMAGE_TYPE,
    XML_TOK_IMAGE_SHOW,
    XML_TOK_IMAGE_ACTUATE
};

}

void XMLImageStyle::exportXML(OUString const & rStrName, uno::Any const & rValue, SvXMLExport& rExport)
{
    if (rStrName.isEmpty())
        return;

    if (!rValue.has<uno::Reference<awt::XBitmap>>())
        return;

    // Name
    bool bEncoded = false;
    rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,
                         rExport.EncodeStyleName(rStrName, &bEncoded));
    if (bEncoded)
    {
        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, rStrName);
    }

    auto xBitmap = rValue.get<uno::Reference<awt::XBitmap>>();
    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);

    OUString aMimeType;
    const OUString aStr = rExport.AddEmbeddedXGraphic(xGraphic, aMimeType);

    // uri
    if (!aStr.isEmpty())
    {
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStr );
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
    }

    // Do Write
    SvXMLElementExport aElem(rExport, XML_NAMESPACE_DRAW, XML_FILL_IMAGE, true, true);

    if (xBitmap.is() && xGraphic.is())
    {
        // optional office:binary-data
        rExport.AddEmbeddedXGraphicAsBase64(xGraphic);
    }
}

bool XMLImageStyle::importXML(uno::Reference<xml::sax::XFastAttributeList> const & xAttrList,
                              uno::Any& rValue, OUString& rStrName, SvXMLImport& rImport)
{
    bool bHasHRef = false;
    bool bHasName = false;
    OUString aDisplayName;
    uno::Reference<graphic::XGraphic> xGraphic;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString aStrValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(DRAW, XML_NAME):
                {
                    rStrName = aStrValue;
                    bHasName = true;
                }
                break;
            case XML_ELEMENT(DRAW, XML_DISPLAY_NAME):
                {
                    aDisplayName = aStrValue;
                }
                break;
            case XML_ELEMENT(XLINK, XML_HREF):
                {
                    xGraphic = rImport.loadGraphicByURL(aStrValue);
                    bHasHRef = true;
                }
                break;
            case XML_ELEMENT(XLINK, XML_TYPE):
                // ignore
                break;
            case XML_ELEMENT(XLINK, XML_SHOW):
                // ignore
                break;
            case XML_ELEMENT(XLINK, XML_ACTUATE):
                // ignore
                break;
            default:
                SAL_WARN("xmloff.style", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aStrValue);
        }
    }

    if (xGraphic.is())
        rValue <<= xGraphic;

    if( !aDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XmlStyleFamily::SD_FILL_IMAGE_ID,
                                     rStrName, aDisplayName );
        rStrName = aDisplayName;
    }

    return bHasName && bHasHRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
