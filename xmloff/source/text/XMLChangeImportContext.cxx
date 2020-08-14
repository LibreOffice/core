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
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CHANGE_ID;


XMLChangeImportContext::XMLChangeImportContext(
    SvXMLImport& rImport,
    sal_Int16 nPrefix,
    const OUString& rLocalName,
    Element const eElement,
    bool bOutsideOfParagraph)
    :   SvXMLImportContext(rImport, nPrefix, rLocalName)
    ,   m_Element(eElement)
    ,   m_bIsOutsideOfParagraph(bOutsideOfParagraph)
{
}

XMLChangeImportContext::~XMLChangeImportContext()
{
}

void XMLChangeImportContext::StartElement(
    const Reference<XAttributeList>& xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken( sLocalName, XML_CHANGE_ID ) )
        {
            // Id found! Now call RedlineImportHelper

            // prepare parameters
            rtl::Reference<XMLTextImportHelper> rHelper =
                GetImport().GetTextImport();
            OUString sID = xAttrList->getValueByIndex(nAttr);

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
        }
        // else: ignore
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
