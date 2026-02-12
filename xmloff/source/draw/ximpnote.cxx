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

#include "ximpnote.hxx"
#include "ximpshap.hxx"
#include <xmloff/xmlnamespace.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SdXMLNotesContext::SdXMLNotesContext(
    SdXMLImport& rImport, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference<drawing::XShapes> const& rShapes, bool ignorePageNumberInThumbnail)
    : SdXMLGenericPageContext(rImport, xAttrList, rShapes)
    , mbIgnorePageNumberInThumbnail(ignorePageNumberInThumbnail)
{
    OUString sStyleName, sPageMasterName;

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_PAGE_LAYOUT_NAME):
            {
                sPageMasterName = sValue;
                break;
            }
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            {
                sStyleName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_HEADER_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_HEADER_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_HEADER_NAME):
            {
                maUseHeaderDeclName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_FOOTER_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_FOOTER_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_FOOTER_NAME):
            {
                maUseFooterDeclName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_DATE_TIME_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_DATE_TIME_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_DATE_TIME_NAME):
            {
                maUseDateTimeDeclName = sValue;
                break;
            }
        }
    }

    SetStyle(sStyleName);

    // now delete all up-to-now contained shapes from this notes page
    uno::Reference<drawing::XShape> xShape;
    while (rShapes->getCount())
    {
        rShapes->getByIndex(0) >>= xShape;
        if (xShape.is())
            rShapes->remove(xShape);
    }

    // set page-master?
    if (!sPageMasterName.isEmpty())
    {
        SetPageMaster(sPageMasterName);
    }
}

SdXMLNotesContext::~SdXMLNotesContext() {}

uno::Reference<xml::sax::XFastContextHandler> SdXMLNotesContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    auto pContext = SdXMLGenericPageContext::createFastChildContext(nElement, xAttrList);
    if (nElement == XML_ELEMENT(DRAW, XML_PAGE_THUMBNAIL) && mbIgnorePageNumberInThumbnail)
    {
        assert(dynamic_cast<SdXMLPageShapeContext*>(pContext.get()));
        auto pPageShapeContext = static_cast<SdXMLPageShapeContext*>(pContext.get());
        pPageShapeContext->ignorePageNumber();
    }
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
