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

#include "XMLRedlineExport.hxx"
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::document::XRedlinesSupplier;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextSection;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::std::list;


XMLRedlineExport::XMLRedlineExport(SvXMLExport& rExp)
:   sDelete("Delete")
,   sDeletion(GetXMLToken(XML_DELETION))
,   sFormat("Format")
,   sFormatChange(GetXMLToken(XML_FORMAT_CHANGE))
,   sInsert("Insert")
,   sInsertion(GetXMLToken(XML_INSERTION))
,   sIsCollapsed("IsCollapsed")
,   sIsStart("IsStart")
,   sRedlineAuthor("RedlineAuthor")
,   sRedlineComment("RedlineComment")
,   sRedlineDateTime("RedlineDateTime")
,   sRedlineSuccessorData("RedlineSuccessorData")
,   sRedlineText("RedlineText")
,   sRedlineType("RedlineType")
,   sRedlineUndoType("RedlineUndoType")
,   sRedlineUndoStart("RedlineUndoStart")
,   sRedlineUndoEnd("RedlineUndoEnd")
,   sUnknownChange("UnknownChange")
,   sStartRedline("StartRedline")
,   sEndRedline("EndRedline")
,   sRedlineIdentifier("RedlineIdentifier")
,   sIsInHeaderFooter("IsInHeaderFooter")
,   sRecordChanges("RecordChanges")
,   sMergeLastPara("MergeLastPara")
,   sChangePrefix("ct")
,   rExport(rExp)
,   pCurrentChangesList(nullptr)
{
}


XMLRedlineExport::~XMLRedlineExport()
{
    // delete changes lists
    for( ChangesMapType::iterator aIter = aChangeMap.begin();
         aIter != aChangeMap.end();
         ++aIter )
    {
        delete aIter->second;
    }
    aChangeMap.clear();
}

void XMLRedlineExport::ExportUndoChange(
    const Reference<XPropertySet> & rPropSet,
    sal_uInt32 nParaIdx,
    bool bAutoStyle)
{
    if (!bAutoStyle)
    {
        ExportUndoChangeInfo(rPropSet, nParaIdx);
    }
}

void XMLRedlineExport::SetCurrentXText(
    const Reference<XText> & rText)
{
    if (rText.is())
    {
        // look for appropriate list in map; use the found one, or create new
        ChangesMapType::iterator aIter = aChangeMap.find(rText);
        if (aIter == aChangeMap.end())
        {
            ChangesListType* pList = new ChangesListType;
            aChangeMap[rText] = pList;
            pCurrentChangesList = pList;
        }
        else
            pCurrentChangesList = aIter->second;
    }
    else
    {
        // don't record changes
        SetCurrentXText();
    }
}

void XMLRedlineExport::SetCurrentXText()
{
    pCurrentChangesList = nullptr;
}

void XMLRedlineExport::ExportUndoChangedRegion(
    const Reference<XPropertySet> & rPropSet, sal_uInt32 nParaIdx)
{
    {
        Any aAny = rPropSet->getPropertyValue(sRedlineType);
        OUString sType;
        aAny >>= sType;

        sal_uInt32 nCharStart=0, nCharEnd=0;
        sal_uInt32 nTextDelCount = rExport.GetTextParagraphExport()->getTextDelCount();
        rPropSet->getPropertyValue(sRedlineUndoStart) >>= nCharStart;
        rPropSet->getPropertyValue(sRedlineUndoEnd) >>= nCharEnd;
        nCharStart -= nTextDelCount;
        nCharEnd -= nTextDelCount;

        OUString sUndoType;
        aAny = rPropSet->getPropertyValue(sRedlineUndoType);
        aAny >>= sUndoType;

        if( sUndoType == "paragraph" )
        {
            nParaIdx++;
            rExport.AddAttribute(XML_NAMESPACE_C_EXT, XML_START, "/" + rtl::OUString::number(nParaIdx));
            rExport.AddAttribute(XML_NAMESPACE_DC, XML_TYPE, XML_PARAGRAPH);
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_C_EXT, XML_START, "/" + rtl::OUString::number(nParaIdx) + "/" + rtl::OUString::number(nCharStart));
            if( sType == sInsert || sType == sFormat )
                rExport.AddAttribute(XML_NAMESPACE_C_EXT, XML_END, "/" + rtl::OUString::number(nParaIdx) + "/" + rtl::OUString::number(nCharEnd));
            else
                rExport.GetTextParagraphExport()->setTextDelCount( rExport.GetTextParagraphExport()->getTextDelCount() + nCharEnd - nCharStart + 1 );
            if( sType == sFormat )
                rExport.AddAttribute(XML_NAMESPACE_DC, XML_TYPE, XML_FORMAT_CHANGE);
            else
                rExport.AddAttribute(XML_NAMESPACE_DC, XML_TYPE, XML_TEXT);
        }
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_TEXT,
                                ConvertTypeName(sType), true, true);

        // get XText from the redline and export (if the XText exists)
        aAny = rPropSet->getPropertyValue(sRedlineText);
        Reference<XText> xText;
        aAny >>= xText;
        if (xText.is())
        {
            rExport.GetTextParagraphExport()->exportText(xText);
            // default parameters: bProgress, bExportParagraph ???
        }
    }
}

const OUString XMLRedlineExport::ConvertTypeName(
    const OUString& sApiName)
{
    if (sApiName == sDelete)
    {
        return sInsertion;
    }
    else if (sApiName == sInsert)
    {
        return sDeletion;
    }
    else if (sApiName == sFormat)
    {
        return sFormatChange;
    }
    else
    {
        OSL_FAIL("unknown redline type");
        return sUnknownChange;
    }
}

void XMLRedlineExport::ExportUndoChangeInfo(
    const Reference<XPropertySet> & rPropSet, sal_uInt32 nParaIdx)
{
    Any aAny = rPropSet->getPropertyValue(sIsCollapsed);
    bool bCollapsed = *static_cast<sal_Bool const *>(aAny.getValue());
    aAny = rPropSet->getPropertyValue(sIsStart);
    bool bStart = *static_cast<sal_Bool const *>(aAny.getValue());
    if( bCollapsed || bStart )
    {
        aAny = rPropSet->getPropertyValue(sRedlineAuthor);
        OUString sAuthor;
        aAny >>= sAuthor;
        if (!sAuthor.isEmpty())
        {
            rExport.AddAttribute(XML_NAMESPACE_DC, XML_CREATOR, sAuthor);
        }

        aAny = rPropSet->getPropertyValue(sRedlineDateTime);
        util::DateTime aDateTime;
        aAny >>= aDateTime;
        OUStringBuffer sBuf;
        ::sax::Converter::convertDateTime(sBuf, aDateTime, nullptr);
        rExport.AddAttribute(XML_NAMESPACE_DC, XML_DATE, sBuf.makeStringAndClear());
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_OFFICE,
                                    XML_CHANGE, true, true);
        ExportUndoChangedRegion(rPropSet, nParaIdx);
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XPropertySet> & rPropSet,
    bool bStart)
{
    if( ! rPropSet.is() )
        return;

    // get appropriate (start or end) property
    Any aAny;
    try
    {
        aAny = rPropSet->getPropertyValue(bStart ? sStartRedline : sEndRedline);
    }
    catch(const UnknownPropertyException&)
    {
        // If we don't have the property, there's nothing to do.
        return;
    }

    Sequence<PropertyValue> aValues;
    aAny >>= aValues;
    const PropertyValue* pValues = aValues.getConstArray();

    // seek for redline properties
    bool bIsCollapsed = false;
    bool bIsStart = true;
    OUString sId;
    bool bIdOK = false; // have we seen an ID?
    sal_Int32 nLength = aValues.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        if (sRedlineIdentifier.equals(pValues[i].Name))
        {
            pValues[i].Value >>= sId;
            bIdOK = true;
        }
        else if (sIsCollapsed.equals(pValues[i].Name))
        {
            bIsCollapsed = *static_cast<sal_Bool const *>(pValues[i].Value.getValue());
        }
        else if (sIsStart.equals(pValues[i].Name))
        {
            bIsStart = *static_cast<sal_Bool const *>(pValues[i].Value.getValue());
        }
    }

    if( bIdOK )
    {
        DBG_ASSERT( !sId.isEmpty(), "Redlines must have IDs" );

        // TODO: use GetRedlineID or eliminate that function
        OUStringBuffer sBuffer(sChangePrefix);
        sBuffer.append(sId);

        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                             sBuffer.makeStringAndClear());

        // export the element
        // (whitespace because we're not inside paragraphs)
        SvXMLElementExport aChangeElem(
            rExport, XML_NAMESPACE_TEXT,
            bIsCollapsed ? XML_CHANGE :
                ( bIsStart ? XML_CHANGE_START : XML_CHANGE_END ),
            true, true);
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextContent> & rContent,
    bool bStart)
{
    Reference<XPropertySet> xPropSet(rContent, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        OSL_FAIL("XPropertySet expected");
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextSection> & rSection,
    bool bStart)
{
    Reference<XPropertySet> xPropSet(rSection, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        OSL_FAIL("XPropertySet expected");
    }
}

void XMLRedlineExport::WriteComment(const OUString& rComment)
{
    if (!rComment.isEmpty())
    {
        // iterate over all string-pieces separated by return (0x0a) and
        // put each inside a paragraph element.
        SvXMLTokenEnumerator aEnumerator(rComment, sal_Char(0x0a));
        OUString aSubString;
        while (aEnumerator.getNextToken(aSubString))
        {
            SvXMLElementExport aParagraph(
                rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
            rExport.Characters(aSubString);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
