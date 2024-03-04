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

#include "XMLChangeImportContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <sal/log.hxx>

using ::com::sun::star::uno::Reference;
using ::xmloff::token::XML_CHANGE_ID;


XMLChangeImportContext::XMLChangeImportContext(
    SvXMLImport& rImport,
    Element const eElement,
    bool bOutsideOfParagraph)
    :   SvXMLImportContext(rImport)
    ,   m_Element(eElement)
    ,   m_bIsOutsideOfParagraph(bOutsideOfParagraph)
{
}

XMLChangeImportContext::~XMLChangeImportContext()
{
}

void XMLChangeImportContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_CHANGE_ID):
            {
                // Id found! Now call RedlineImportHelper

                // prepare parameters
                rtl::Reference<XMLTextImportHelper> rHelper =
                    GetImport().GetTextImport();
                OUString sID = aIter.toString();

                // <text:change> is both start and end
                if (Element::START == m_Element || Element::POINT == m_Element)
                    rHelper->RedlineSetCursor(sID, true, m_bIsOutsideOfParagraph);
                if (Element::END == m_Element || Element::POINT == m_Element)
                    rHelper->RedlineSetCursor(sID, false, m_bIsOutsideOfParagraph);

                // outside of paragraph and still open? set open redline ID
                if (m_bIsOutsideOfParagraph)
                {
                    rHelper->SetOpenRedlineId(sID);
                }
                break;
            }
            // else: ignore
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
