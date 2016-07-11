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

#include <xmloff/XMLTrackedChangesImportContext.hxx>
#include "XMLChangeElementImportContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/XTextCursor.hpp>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextCursor;
using namespace ::com::sun::star;
using ::com::sun::star::xml::sax::XAttributeList;
using namespace ::xmloff::token;


XMLTrackedChangesImportContext::XMLTrackedChangesImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bMergeLastPara(true)
{
}

XMLTrackedChangesImportContext::~XMLTrackedChangesImportContext()
{
}

void XMLTrackedChangesImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList )
{
    bool bTrackChanges = true;

    // scan for office:change and text:protection-key attributes
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        const OUString sValue = xAttrList->getValueByIndex(i);
        if ( XML_NAMESPACE_OFFICE == nPrefix )
        {
            if ( IsXMLToken( sLocalName, XML_CHANGE ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(
                    bTmp, sValue ) )
                {
                    bTrackChanges = bTmp;
                }
            }
        }
        if (XML_NAMESPACE_DC == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_CREATOR))
            {
                 sAuthor = sValue;
            }

            if (IsXMLToken(sLocalName, XML_DATE))
            {
                 sDate = sValue;
            }
        }
        sComment = "";
    }

    // set tracked changes
    GetImport().GetTextImport()->SetRecordChanges( bTrackChanges );
}


SvXMLImportContext* XMLTrackedChangesImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = nullptr;
    sal_uInt32 nStartParaPos, nEndParaPos, nStartTextPos, nEndTextPos;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        // from the ODF 1.2 standard :
        // The <text:changed-region> element has the following child elements:
        // <text:deletion>, <text:format-change> and <text:insertion>.
        if ( IsXMLToken( rLocalName, XML_INSERTION ) ||
             IsXMLToken( rLocalName, XML_DELETION ) ||
             IsXMLToken( rLocalName, XML_FORMAT_CHANGE ) )
        {
            sal_Int16 nLength = xAttrList->getLength();
            for( sal_Int16 i = 0; i < nLength; i++ )
            {
                OUString sLocalName;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                    GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );
                const OUString sValue = xAttrList->getValueByIndex(i);
                if (XML_NAMESPACE_C == nPrefix)
                {
                    if (IsXMLToken(sLocalName, xmloff::token::XML_START))
                    {
                        sStart = sValue.pData->buffer + 1;
                    }
                    if (IsXMLToken(sLocalName, XML_END))
                    {
                        sEnd = sValue;
                    }
                }
                if (XML_NAMESPACE_DC == nPrefix)
                {
                    if (IsXMLToken(sLocalName, XML_TYPE))
                    {
                        sType = sValue;
                    }
                }
            }
            if(sStart.indexOf('/') != -1)
            {
                sID = OUString(sStart.getStr(), sStart.indexOf('/'));
                nStartParaPos = sID.toUInt32();
            }
            else
            {
                nStartParaPos = sStart.toUInt32();
                sID = sStart;
            }
            SetChangeInfo( rLocalName, sAuthor, sComment, sDate, nStartParaPos );

            // create XMLChangeElementImportContext for all kinds of changes
            pContext = new XMLChangeElementImportContext(
                GetImport(), nPrefix, rLocalName,
                IsXMLToken( rLocalName, XML_DELETION ),
                *this);
        }
        // else: it may be a text element, see below
    }

    if (nullptr == pContext)
    {
        // illegal element content! TODO: discard the redlines
        // for the moment -> use text

        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);

        // or default if text fail
        if (nullptr == pContext)
        {
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);
        }
    }

    return pContext;
}

void XMLTrackedChangesImportContext::SetChangeInfo(const OUString& rType, const OUString& rAuthor, const OUString& rComment, const OUString& rDate, const sal_uInt32 nStartParaPos)
{
    util::DateTime aDateTime;
    if (::sax::Converter::parseDateTime(aDateTime, nullptr, rDate))
    {
        GetImport().GetTextImport()->RedlineAdd(
            rType, sID, rAuthor, rComment, aDateTime, bMergeLastPara, nStartParaPos);
    }
}

void XMLTrackedChangesImportContext::UseRedlineText()
{
    // if we haven't already installed the redline cursor, do it now
    if (! xOldCursor.is() )
    {
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
