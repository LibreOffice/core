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
#include "xmloff/xmlnmspe.hxx"
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
,   sStyle("Style")
,   sTextTable("TextTable")
,   sUnknownChange("UnknownChange")
,   sStartRedline("StartRedline")
,   sEndRedline("EndRedline")
,   sRedlineIdentifier("RedlineIdentifier")
,   sIsInHeaderFooter("IsInHeaderFooter")
,   sRedlineProtectionKey("RedlineProtectionKey")
,   sRecordChanges("RecordChanges")
,   sMergeLastPara("MergeLastPara")
,   sChangePrefix("ct")
,   rExport(rExp)
,   pCurrentChangesList(NULL)
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


void XMLRedlineExport::ExportChange(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyle)
{
    if (bAutoStyle)
    {
        // For the headers/footers, we have to collect the autostyles
        // here.  For the general case, however, it's better to collet
        // the autostyles by iterating over the global redline
        // list. So that's what we do: Here, we collect autostyles
        // only if we have no current list of changes. For the
        // main-document case, the autostyles are collected in
        // ExportChangesListAutoStyles().
        if (pCurrentChangesList != NULL)
            ExportChangeAutoStyle(rPropSet);
    }
    else
    {
        ExportChangeInline(rPropSet);
    }
}


void XMLRedlineExport::ExportChangesList(sal_Bool bAutoStyles)
{
    if (bAutoStyles)
    {
        ExportChangesListAutoStyles();
    }
    else
    {
        ExportChangesListElements();
    }
}


void XMLRedlineExport::ExportChangesList(
    const Reference<XText> & rText,
    sal_Bool bAutoStyles)
{
    // in the header/footer case, auto styles are collected from the
    // inline change elements.
    if (bAutoStyles)
        return;

    // look for changes list for this XText
    ChangesMapType::iterator aFind = aChangeMap.find(rText);
    if (aFind != aChangeMap.end())
    {
        ChangesListType* pChangesList = aFind->second;

        // export only if changes are found
        if (pChangesList->size() > 0)
        {
            // changes container element
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        XML_TRACKED_CHANGES,
                                        sal_True, sal_True);

            // iterate over changes list
            for( ChangesListType::iterator aIter = pChangesList->begin();
                 aIter != pChangesList->end();
                 ++aIter )
            {
                ExportChangedRegion( *aIter );
            }
        }
        // else: changes list empty -> ignore
    }
    // else: no changes list found -> empty
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
    pCurrentChangesList = NULL;
}


void XMLRedlineExport::ExportChangesListElements()
{
    // get redlines (aka tracked changes) from the model
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        // redline protection key
        Reference<XPropertySet> aDocPropertySet( rExport.GetModel(),
                                                 uno::UNO_QUERY );
        // redlining enabled?
        sal_Bool bEnabled = *(sal_Bool*)aDocPropertySet->getPropertyValue(
                                                sRecordChanges ).getValue();

        // only export if we have redlines or attributes
        if ( aEnumAccess->hasElements() || bEnabled )
        {

            // export only if we have changes, but tracking is not enabled
            if ( !bEnabled != !aEnumAccess->hasElements() )
            {
                rExport.AddAttribute(
                    XML_NAMESPACE_TEXT, XML_TRACK_CHANGES,
                    bEnabled ? XML_TRUE : XML_FALSE );
            }

            // changes container element
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        XML_TRACKED_CHANGES,
                                        sal_True, sal_True);

            // get enumeration and iterate over elements
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {
                    // export only if not in header or footer
                    // (those must be exported with their XText)
                    aAny = xPropSet->getPropertyValue(sIsInHeaderFooter);
                    if (! *(sal_Bool*)aAny.getValue())
                    {
                        // and finally, export change
                        ExportChangedRegion(xPropSet);
                    }
                }
                // else: no XPropertySet -> no export
            }
        }
        // else: no redlines -> no export
    }
    // else: no XRedlineSupplier -> no export
}

void XMLRedlineExport::ExportChangeAutoStyle(
    const Reference<XPropertySet> & rPropSet)
{
    // record change (if changes should be recorded)
    if (NULL != pCurrentChangesList)
    {
        // put redline in list if it's collapsed or the redline start
        Any aIsStart = rPropSet->getPropertyValue(sIsStart);
        Any aIsCollapsed = rPropSet->getPropertyValue(sIsCollapsed);

        if ( *(sal_Bool*)aIsStart.getValue() ||
             *(sal_Bool*)aIsCollapsed.getValue() )
            pCurrentChangesList->push_back(rPropSet);
    }

    // get XText for export of redline auto styles
    Any aAny = rPropSet->getPropertyValue(sRedlineText);
    Reference<XText> xText;
    aAny >>= xText;
    if (xText.is())
    {
        // export the auto styles
        rExport.GetTextParagraphExport()->collectTextAutoStyles(xText);
    }
}

void XMLRedlineExport::ExportChangesListAutoStyles()
{
    // get redlines (aka tracked changes) from the model
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        // only export if we actually have redlines
        if (aEnumAccess->hasElements())
        {
            // get enumeration and iterate over elements
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {

                    // export only if not in header or footer
                    // (those must be exported with their XText)
                    aAny = xPropSet->getPropertyValue(sIsInHeaderFooter);
                    if (! *(sal_Bool*)aAny.getValue())
                    {
                        ExportChangeAutoStyle(xPropSet);
                    }
                }
            }
        }
    }
}

void XMLRedlineExport::ExportChangeInline(
    const Reference<XPropertySet> & rPropSet)
{
    // determine element name (depending on collapsed, start/end)
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;
    Any aAny = rPropSet->getPropertyValue(sIsCollapsed);
    sal_Bool bCollapsed = *(sal_Bool *)aAny.getValue();
    sal_Bool bStart = sal_True; // ignored if bCollapsed = sal_True
    if (bCollapsed)
    {
        eElement = XML_CHANGE;
    }
    else
    {
        aAny = rPropSet->getPropertyValue(sIsStart);
        bStart = *(sal_Bool *)aAny.getValue();
        eElement = bStart ? XML_CHANGE_START : XML_CHANGE_END;
    }

    if (XML_TOKEN_INVALID != eElement)
    {
        // we always need the ID
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                             GetRedlineID(rPropSet));

        // export the element (no whitespace because we're in the text body)
        SvXMLElementExport aChangeElem(rExport, XML_NAMESPACE_TEXT,
                                       eElement, sal_False, sal_False);
    }
}


void XMLRedlineExport::ExportChangedRegion(
    const Reference<XPropertySet> & rPropSet)
{
    // Redline-ID
    rExport.AddAttributeIdLegacy(XML_NAMESPACE_TEXT, GetRedlineID(rPropSet));

    // merge-last-paragraph
    Any aAny = rPropSet->getPropertyValue(sMergeLastPara);
    if( ! *(sal_Bool*)aAny.getValue() )
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_MERGE_LAST_PARAGRAPH,
                             XML_FALSE);

    // export change region element
    SvXMLElementExport aChangedRegion(rExport, XML_NAMESPACE_TEXT,
                                      XML_CHANGED_REGION, sal_True, sal_True);


    // scope for (first) change element
    {
        aAny = rPropSet->getPropertyValue(sRedlineType);
        OUString sType;
        aAny >>= sType;
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_TEXT,
                                   ConvertTypeName(sType), sal_True, sal_True);

        ExportChangeInfo(rPropSet);

        // get XText from the redline and export (if the XText exists)
        aAny = rPropSet->getPropertyValue(sRedlineText);
        Reference<XText> xText;
        aAny >>= xText;
        if (xText.is())
        {
            rExport.GetTextParagraphExport()->exportText(xText);
            // default parameters: bProgress, bExportParagraph ???
        }
        // else: no text interface -> content is inline and will
        //       be exported there
    }

    // changed change? Hierarchical changes can onl be two levels
    // deep. Here we check for the second level.
    aAny = rPropSet->getPropertyValue(sRedlineSuccessorData);
    Sequence<PropertyValue> aSuccessorData;
    aAny >>= aSuccessorData;

    // if we actually got a hierarchical change, make element and
    // process change info
    if (aSuccessorData.getLength() > 0)
    {
        // The only change that can be "undone" is an insertion -
        // after all, you can't re-insert an deletion, but you can
        // delete an insertion. This assumption is asserted in
        // ExportChangeInfo(Sequence<PropertyValue>&).
        SvXMLElementExport aSecondChangeElem(
            rExport, XML_NAMESPACE_TEXT, XML_INSERTION,
            sal_True, sal_True);

        ExportChangeInfo(aSuccessorData);
    }
    // else: no hierarchical change
}


const OUString XMLRedlineExport::ConvertTypeName(
    const OUString& sApiName)
{
    if (sApiName == sDelete)
    {
        return sDeletion;
    }
    else if (sApiName == sInsert)
    {
        return sInsertion;
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


/** Create a Redline-ID */
const OUString XMLRedlineExport::GetRedlineID(
    const Reference<XPropertySet> & rPropSet)
{
    Any aAny = rPropSet->getPropertyValue(sRedlineIdentifier);
    OUString sTmp;
    aAny >>= sTmp;

    OUStringBuffer sBuf(sChangePrefix);
    sBuf.append(sTmp);
    return sBuf.makeStringAndClear();
}


void XMLRedlineExport::ExportChangeInfo(
    const Reference<XPropertySet> & rPropSet)
{

    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, sal_True, sal_True);

    Any aAny = rPropSet->getPropertyValue(sRedlineAuthor);
    OUString sTmp;
    aAny >>= sTmp;
    if (!sTmp.isEmpty())
    {
        SvXMLElementExport aCreatorElem( rExport, XML_NAMESPACE_DC,
                                          XML_CREATOR, sal_True,
                                          sal_False );
        rExport.Characters(sTmp);
    }

    aAny = rPropSet->getPropertyValue(sRedlineDateTime);
    util::DateTime aDateTime;
    aAny >>= aDateTime;
    {
        OUStringBuffer sBuf;
        ::sax::Converter::convertDateTime(sBuf, aDateTime, 0);
        SvXMLElementExport aDateElem( rExport, XML_NAMESPACE_DC,
                                          XML_DATE, sal_True,
                                          sal_False );
        rExport.Characters(sBuf.makeStringAndClear());
    }

    // comment as <text:p> sequence
    aAny = rPropSet->getPropertyValue(sRedlineComment);
    aAny >>= sTmp;
    WriteComment( sTmp );
}

void XMLRedlineExport::ExportChangeInfo(
    const Sequence<PropertyValue> & rPropertyValues)
{
    OUString sComment;

    sal_Int32 nCount = rPropertyValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        const PropertyValue& rVal = rPropertyValues[i];

        if( rVal.Name.equals(sRedlineAuthor) )
        {
            OUString sTmp;
            rVal.Value >>= sTmp;
            if (!sTmp.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_AUTHOR, sTmp);
            }
        }
        else if( rVal.Name.equals(sRedlineComment) )
        {
            rVal.Value >>= sComment;
        }
        else if( rVal.Name.equals(sRedlineDateTime) )
        {
            util::DateTime aDateTime;
            rVal.Value >>= aDateTime;
            OUStringBuffer sBuf;
            ::sax::Converter::convertDateTime(sBuf, aDateTime, 0);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_DATE_TIME,
                                 sBuf.makeStringAndClear());
        }
        else if( rVal.Name.equals(sRedlineType) )
        {
            // check if this is an insertion; cf. comment at calling location
            OUString sTmp;
            rVal.Value >>= sTmp;
            DBG_ASSERT(sTmp.equals(sInsert),
                       "hierarchical change must be insertion");
        }
        // else: unknown value -> ignore
    }

    // finally write element
    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, sal_True, sal_True);

    WriteComment( sComment );
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bStart)
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
    sal_Bool bIsCollapsed = sal_False;
    sal_Bool bIsStart = sal_True;
    OUString sId;
    sal_Bool bIdOK = sal_False; // have we seen an ID?
    sal_Int32 nLength = aValues.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        if (sRedlineIdentifier.equals(pValues[i].Name))
        {
            pValues[i].Value >>= sId;
            bIdOK = sal_True;
        }
        else if (sIsCollapsed.equals(pValues[i].Name))
        {
            bIsCollapsed = *(sal_Bool*)pValues[i].Value.getValue();
        }
        else if (sIsStart.equals(pValues[i].Name))
        {
            bIsStart = *(sal_Bool*)pValues[i].Value.getValue();
        }
    }

    if( bIdOK )
    {
        DBG_ASSERT( !sId.isEmpty(), "Redlines must have IDs" );

        // TODO: use GetRedlineID or elimiate that function
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
            sal_True, sal_True);
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextContent> & rContent,
    sal_Bool bStart)
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
    sal_Bool bStart)
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
                rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
            rExport.Characters(aSubString);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
