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

#include "XMLChangedRegionImportContext.hxx"
#include "XMLChangeElementImportContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextCursor;
using namespace ::com::sun::star;
using ::com::sun::star::xml::sax::XAttributeList;



TYPEINIT1(XMLChangedRegionImportContext, SvXMLImportContext);

XMLChangedRegionImportContext::XMLChangedRegionImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bMergeLastPara(sal_True)
{
}

XMLChangedRegionImportContext::~XMLChangedRegionImportContext()
{
}

void XMLChangedRegionImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes: id
    bool bHaveXmlId( false );
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        const OUString sValue = xAttrList->getValueByIndex(nAttr);
        if (XML_NAMESPACE_XML == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_ID))
            {
                sID = sValue;
                bHaveXmlId = true;
            }
        }
        else if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_ID))
            {
                if (!bHaveXmlId) { sID = sValue; }
            }
            else if( IsXMLToken( sLocalName, XML_MERGE_LAST_PARAGRAPH ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, sValue))
                {
                    bMergeLastPara = bTmp;
                }
            }
        }
    }
}

SvXMLImportContext* XMLChangedRegionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        // from the ODF 1.2 standard :
        // The <text:changed-region> element has the following child elements:
        // <text:deletion>, <text:format-change> and <text:insertion>.
        if ( IsXMLToken( rLocalName, XML_INSERTION ) ||
             IsXMLToken( rLocalName, XML_DELETION ) ||
             IsXMLToken( rLocalName, XML_FORMAT_CHANGE ) )
        {
            // create XMLChangeElementImportContext for all kinds of changes
            pContext = new XMLChangeElementImportContext(
               GetImport(), nPrefix, rLocalName,
               IsXMLToken( rLocalName, XML_DELETION ),
               *this);
        }
        // else: it may be a text element, see below
    }

    if (NULL == pContext)
    {
        // illegal element content! TODO: discard the redlines
        // for the moment -> use text

        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);

        // or default if text fail
        if (NULL == pContext)
        {
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);
        }
    }

    return pContext;
}

void XMLChangedRegionImportContext::EndElement()
{
    // restore old XCursor (if necessary)
    if (xOldCursor.is())
    {
        // delete last paragraph
        // (one extra paragraph was inserted in the beginning)
        UniReference<XMLTextImportHelper> rHelper =
            GetImport().GetTextImport();
        rHelper->DeleteParagraph();

        GetImport().GetTextImport()->SetCursor(xOldCursor);
        xOldCursor = NULL;
    }
}

void XMLChangedRegionImportContext::SetChangeInfo(
    const OUString& rType,
    const OUString& rAuthor,
    const OUString& rComment,
    const OUString& rDate)
{
    util::DateTime aDateTime;
    if (::sax::Converter::parseDateTime(aDateTime, 0, rDate))
    {
        GetImport().GetTextImport()->RedlineAdd(
            rType, sID, rAuthor, rComment, aDateTime, bMergeLastPara);
    }
}

void XMLChangedRegionImportContext::UseRedlineText()
{
    // if we haven't already installed the redline cursor, do it now
    if (! xOldCursor.is())
    {
        // get TextImportHelper and old Cursor
        UniReference<XMLTextImportHelper> rHelper(GetImport().GetTextImport());
        Reference<XTextCursor> xCursor( rHelper->GetCursor() );

        // create Redline and new Cursor
        Reference<XTextCursor> xNewCursor =
            rHelper->RedlineCreateText(xCursor, sID);

        if (xNewCursor.is())
        {
            // save old cursor and install new one
            xOldCursor = xCursor;
            rHelper->SetCursor( xNewCursor );
        }
        // else: leave as is
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
