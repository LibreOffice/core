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

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextCursor;
using namespace ::com::sun::star;
using ::com::sun::star::xml::sax::XAttributeList;


XMLChangedRegionImportContext::XMLChangedRegionImportContext(SvXMLImport& rImport) :
        SvXMLImportContext(rImport),
        bMergeLastPara(true)
{
}

XMLChangedRegionImportContext::~XMLChangedRegionImportContext()
{
}

void XMLChangedRegionImportContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // process attributes: id
    bool bHaveXmlId( false );
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(XML, XML_ID):
            {
                sID = aIter.toString();
                bHaveXmlId = true;
                break;
            }
            case XML_ELEMENT(TEXT, XML_ID):
            {
                if (!bHaveXmlId) { sID = aIter.toString(); }
                break;
            }
            case XML_ELEMENT(TEXT, XML_MERGE_LAST_PARAGRAPH):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                {
                    bMergeLastPara = bTmp;
                }
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLChangedRegionImportContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContextRef xContext;

    // from the ODF 1.2 standard :
    // The <text:changed-region> element has the following child elements:
    // <text:deletion>, <text:format-change> and <text:insertion>.
    if (nElement == XML_ELEMENT(TEXT, XML_INSERTION) ||
        nElement == XML_ELEMENT(TEXT, XML_DELETION) ||
        nElement == XML_ELEMENT(TEXT, XML_FORMAT_CHANGE) )
    {
        // create XMLChangeElementImportContext for all kinds of changes
        xContext = new XMLChangeElementImportContext(
            GetImport(),
            nElement == XML_ELEMENT(TEXT, XML_DELETION),
            *this,
            SvXMLImport::getNameFromToken(nElement));
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    // illegal element content! TODO: discard the redlines
    // for the moment -> use text
    // or default if text fail

    return xContext;
}

void XMLChangedRegionImportContext::endFastElement(sal_Int32 )
{
    // restore old XCursor (if necessary)
    if (xOldCursor.is())
    {
        // delete last paragraph
        // (one extra paragraph was inserted in the beginning)
        rtl::Reference<XMLTextImportHelper> rHelper =
            GetImport().GetTextImport();
        rHelper->DeleteParagraph();

        GetImport().GetTextImport()->SetCursor(xOldCursor);
        xOldCursor = nullptr;
    }
}

void XMLChangedRegionImportContext::SetChangeInfo(
    const OUString& rType,
    const OUString& rAuthor,
    const OUString& rComment,
    std::u16string_view rDate)
{
    util::DateTime aDateTime;
    if (::sax::Converter::parseDateTime(aDateTime, rDate))
    {
        GetImport().GetTextImport()->RedlineAdd(
            rType, sID, rAuthor, rComment, aDateTime, bMergeLastPara);
    }
}

void XMLChangedRegionImportContext::UseRedlineText()
{
    // if we haven't already installed the redline cursor, do it now
    if ( xOldCursor.is())
        return;

    // get TextImportHelper and old Cursor
    rtl::Reference<XMLTextImportHelper> rHelper(GetImport().GetTextImport());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
