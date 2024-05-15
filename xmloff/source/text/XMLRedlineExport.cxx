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
#include <o3tl/any.hxx>
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/frame/XModel.hpp>
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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/securityoptions.hxx>


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


XMLRedlineExport::XMLRedlineExport(SvXMLExport& rExp)
:   sDeletion(GetXMLToken(XML_DELETION))
,   sFormatChange(GetXMLToken(XML_FORMAT_CHANGE))
,   sInsertion(GetXMLToken(XML_INSERTION))
,   rExport(rExp)
,   pCurrentChangesList(nullptr)
{
}


XMLRedlineExport::~XMLRedlineExport()
{
}


void XMLRedlineExport::ExportChange(
    const Reference<XPropertySet> & rPropSet,
    bool bAutoStyle)
{
    if (bAutoStyle)
    {
        // For the headers/footers, we have to collect the autostyles
        // here.  For the general case, however, it's better to collect
        // the autostyles by iterating over the global redline
        // list. So that's what we do: Here, we collect autostyles
        // only if we have no current list of changes. For the
        // main-document case, the autostyles are collected in
        // ExportChangesListAutoStyles().
        if (pCurrentChangesList != nullptr)
            ExportChangeAutoStyle(rPropSet);
    }
    else
    {
        ExportChangeInline(rPropSet);
    }
}


void XMLRedlineExport::ExportChangesList(bool bAutoStyles)
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
    bool bAutoStyles)
{
    // in the header/footer case, auto styles are collected from the
    // inline change elements.
    if (bAutoStyles)
        return;

    // look for changes list for this XText
    ChangesMapType::iterator aFind = aChangeMap.find(rText);
    if (aFind == aChangeMap.end())
        return;

    ChangesVectorType& rChangesList = aFind->second;

    // export only if changes are found
    if (rChangesList.empty())
        return;

    // changes container element
    SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                XML_TRACKED_CHANGES,
                                true, true);

    // iterate over changes list
    for (auto const& change : rChangesList)
    {
        ExportChangedRegion(change);
    }
    // else: changes list empty -> ignore
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
            auto rv = aChangeMap.emplace(std::piecewise_construct, std::forward_as_tuple(rText), std::forward_as_tuple());
            pCurrentChangesList = &rv.first->second;
        }
        else
            pCurrentChangesList = &aIter->second;
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


void XMLRedlineExport::ExportChangesListElements()
{
    // get redlines (aka tracked changes) from the model
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (!xSupplier.is())
        return;

    Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

    // redline protection key
    Reference<XPropertySet> aDocPropertySet( rExport.GetModel(),
                                             uno::UNO_QUERY );
    // redlining enabled?
    bool bEnabled = *o3tl::doAccess<bool>(aDocPropertySet->getPropertyValue(
                                            u"RecordChanges"_ustr ));

    // only export if we have redlines or attributes
    if ( !(aEnumAccess->hasElements() || bEnabled) )
        return;


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
                                true, true);

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
            aAny = xPropSet->getPropertyValue(u"IsInHeaderFooter"_ustr);
            if (! *o3tl::doAccess<bool>(aAny))
            {
                // and finally, export change
                ExportChangedRegion(xPropSet);
            }
        }
        // else: no XPropertySet -> no export
    }
    // else: no redlines -> no export
    // else: no XRedlineSupplier -> no export
}

void XMLRedlineExport::ExportChangeAutoStyle(
    const Reference<XPropertySet> & rPropSet)
{
    // record change (if changes should be recorded)
    if (nullptr != pCurrentChangesList)
    {
        // put redline in list if it's collapsed or the redline start
        Any aIsStart = rPropSet->getPropertyValue(u"IsStart"_ustr);
        Any aIsCollapsed = rPropSet->getPropertyValue(u"IsCollapsed"_ustr);

        if ( *o3tl::doAccess<bool>(aIsStart) ||
             *o3tl::doAccess<bool>(aIsCollapsed) )
            pCurrentChangesList->push_back(rPropSet);
    }

    // get XText for export of redline auto styles
    Any aAny = rPropSet->getPropertyValue(u"RedlineText"_ustr);
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
    if (!xSupplier.is())
        return;

    Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

    // only export if we actually have redlines
    if (!aEnumAccess->hasElements())
        return;

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
            aAny = xPropSet->getPropertyValue(u"IsInHeaderFooter"_ustr);
            if (! *o3tl::doAccess<bool>(aAny))
            {
                ExportChangeAutoStyle(xPropSet);
            }
        }
    }
}

void XMLRedlineExport::ExportChangeInline(
    const Reference<XPropertySet> & rPropSet)
{
    // determine element name (depending on collapsed, start/end)
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;
    Any aAny = rPropSet->getPropertyValue(u"IsCollapsed"_ustr);
    bool bCollapsed = *o3tl::doAccess<bool>(aAny);
    if (bCollapsed)
    {
        eElement = XML_CHANGE;
    }
    else
    {
        aAny = rPropSet->getPropertyValue(u"IsStart"_ustr);
        const bool bStart = *o3tl::doAccess<bool>(aAny);
        eElement = bStart ? XML_CHANGE_START : XML_CHANGE_END;
    }

    if (XML_TOKEN_INVALID != eElement)
    {
        // we always need the ID
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                             GetRedlineID(rPropSet));

        // export the element (no whitespace because we're in the text body)
        SvXMLElementExport aChangeElem(rExport, XML_NAMESPACE_TEXT,
                                       eElement, false, false);
    }
}


void XMLRedlineExport::ExportChangedRegion(
    const Reference<XPropertySet> & rPropSet)
{
    // Redline-ID
    rExport.AddAttributeIdLegacy(XML_NAMESPACE_TEXT, GetRedlineID(rPropSet));

    // merge-last-paragraph
    Any aAny = rPropSet->getPropertyValue(u"MergeLastPara"_ustr);
    if( ! *o3tl::doAccess<bool>(aAny) )
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_MERGE_LAST_PARAGRAPH,
                             XML_FALSE);

    // export change region element
    SvXMLElementExport aChangedRegion(rExport, XML_NAMESPACE_TEXT,
                                      XML_CHANGED_REGION, true, true);


    // scope for (first) change element
    {
        aAny = rPropSet->getPropertyValue(u"RedlineType"_ustr);
        OUString sType;
        aAny >>= sType;
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_TEXT,
                                   ConvertTypeName(sType), true, true);

        ExportChangeInfo(rPropSet);

        // get XText from the redline and export (if the XText exists)
        aAny = rPropSet->getPropertyValue(u"RedlineText"_ustr);
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

    // changed change? Hierarchical changes can only be two levels
    // deep. Here we check for the second level.
    aAny = rPropSet->getPropertyValue(u"RedlineSuccessorData"_ustr);
    Sequence<PropertyValue> aSuccessorData;
    aAny >>= aSuccessorData;

    // if we actually got a hierarchical change, make element and
    // process change info
    if (aSuccessorData.hasElements())
    {
        // The only change that can be "undone" is an insertion -
        // after all, you can't re-insert a deletion, but you can
        // delete an insertion. This assumption is asserted in
        // ExportChangeInfo(Sequence<PropertyValue>&).
        SvXMLElementExport aSecondChangeElem(
            rExport, XML_NAMESPACE_TEXT, XML_INSERTION,
            true, true);

        ExportChangeInfo(aSuccessorData);
    }
    // else: no hierarchical change
}


OUString const & XMLRedlineExport::ConvertTypeName(
    std::u16string_view sApiName)
{
    if (sApiName == u"Delete")
    {
        return sDeletion;
    }
    else if (sApiName == u"Insert")
    {
        return sInsertion;
    }
    else if (sApiName == u"Format")
    {
        return sFormatChange;
    }
    else
    {
        OSL_FAIL("unknown redline type");
        static constexpr OUString sUnknownChange(u"UnknownChange"_ustr);
        return sUnknownChange;
    }
}


/** Create a Redline-ID */
OUString XMLRedlineExport::GetRedlineID(
    const Reference<XPropertySet> & rPropSet)
{
    Any aAny = rPropSet->getPropertyValue(u"RedlineIdentifier"_ustr);
    OUString sTmp;
    aAny >>= sTmp;

    return "ct" + sTmp;
}


void XMLRedlineExport::ExportChangeInfo(
    const Reference<XPropertySet> & rPropSet)
{
    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
            SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo ) && !SvtSecurityOptions::IsOptionSet(
                SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo);

    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, true, true);

    Any aAny = rPropSet->getPropertyValue(u"RedlineAuthor"_ustr);
    OUString sTmp;
    aAny >>= sTmp;
    if (!sTmp.isEmpty())
    {
        SvXMLElementExport aCreatorElem( rExport, XML_NAMESPACE_DC,
                                          XML_CREATOR, true,
                                          false );
        rExport.Characters(bRemovePersonalInfo
                ? "Author" + OUString::number(rExport.GetInfoID(sTmp))
                : sTmp );
    }

    aAny = rPropSet->getPropertyValue(u"RedlineMovedID"_ustr);
    sal_uInt32 nTmp(0);
    aAny >>= nTmp;
    if (nTmp > 1)
    {
        SvXMLElementExport aCreatorElem(rExport, XML_NAMESPACE_LO_EXT, XML_MOVE_ID, true, false);
        rExport.Characters( OUString::number( nTmp ) );
    }

    aAny = rPropSet->getPropertyValue(u"RedlineDateTime"_ustr);
    util::DateTime aDateTime;
    aAny >>= aDateTime;
    {
        OUStringBuffer sBuf;
        ::sax::Converter::convertDateTime(sBuf, bRemovePersonalInfo
                ? util::DateTime(0, 0, 0, 0, 1, 1, 1970, true) // Epoch time
                : aDateTime, nullptr, true);
        SvXMLElementExport aDateElem( rExport, XML_NAMESPACE_DC,
                                          XML_DATE, true,
                                          false );
        rExport.Characters(sBuf.makeStringAndClear());
    }

    // comment as <text:p> sequence
    aAny = rPropSet->getPropertyValue(u"RedlineComment"_ustr);
    aAny >>= sTmp;
    WriteComment( sTmp );
}

// write RedlineSuccessorData
void XMLRedlineExport::ExportChangeInfo(
    const Sequence<PropertyValue> & rPropertyValues)
{
    OUString sComment;
    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
            SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo ) && !SvtSecurityOptions::IsOptionSet(
                SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo);

    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, true, true);

    for(const PropertyValue& rVal : rPropertyValues)
    {
        if( rVal.Name == "RedlineAuthor" )
        {
            OUString sTmp;
            rVal.Value >>= sTmp;
            if (!sTmp.isEmpty())
            {
                SvXMLElementExport aCreatorElem( rExport, XML_NAMESPACE_DC,
                                        XML_CREATOR, true,
                                        false );
                rExport.Characters(bRemovePersonalInfo
                                        ? "Author" + OUString::number(rExport.GetInfoID(sTmp))
                                        : sTmp );
            }
        }
        else if( rVal.Name == "RedlineComment" )
        {
            rVal.Value >>= sComment;
        }
        else if( rVal.Name == "RedlineDateTime" )
        {
            util::DateTime aDateTime;
            rVal.Value >>= aDateTime;
            OUStringBuffer sBuf;
            ::sax::Converter::convertDateTime(sBuf, bRemovePersonalInfo
                                        ? util::DateTime(0, 0, 0, 0, 1, 1, 1970, true) // Epoch time
                                        : aDateTime, nullptr, true);
            SvXMLElementExport aDateElem( rExport, XML_NAMESPACE_DC,
                                          XML_DATE, true,
                                          false );
            rExport.Characters(sBuf.makeStringAndClear());
        }
        else if( rVal.Name == "RedlineType" )
        {
            // check if this is an insertion; cf. comment at calling location
            OUString sTmp;
            rVal.Value >>= sTmp;
            DBG_ASSERT(sTmp == "Insert",
                       "hierarchical change must be insertion");
        }
        // else: unknown value -> ignore
    }

    // finally write comment paragraphs
    WriteComment( sComment );
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
        aAny = rPropSet->getPropertyValue(bStart ? u"StartRedline"_ustr : u"EndRedline"_ustr);
    }
    catch(const UnknownPropertyException&)
    {
        // If we don't have the property, there's nothing to do.
        return;
    }

    Sequence<PropertyValue> aValues;
    aAny >>= aValues;

    // seek for redline properties
    bool bIsCollapsed = false;
    bool bIsStart = true;
    OUString sId;
    bool bIdOK = false; // have we seen an ID?
    for (const auto& rValue : aValues)
    {
        if (rValue.Name == "RedlineIdentifier")
        {
            rValue.Value >>= sId;
            bIdOK = true;
        }
        else if (rValue.Name == "IsCollapsed")
        {
            bIsCollapsed = *o3tl::doAccess<bool>(rValue.Value);
        }
        else if (rValue.Name == "IsStart")
        {
            bIsStart = *o3tl::doAccess<bool>(rValue.Value);
        }
    }

    if( !bIdOK )
        return;

    SAL_WARN_IF( sId.isEmpty(), "xmloff", "Redlines must have IDs" );

    // TODO: use GetRedlineID or eliminate that function
    rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                         "ct" + sId);

    // export the element
    // (whitespace because we're not inside paragraphs)
    SvXMLElementExport aChangeElem(
        rExport, XML_NAMESPACE_TEXT,
        bIsCollapsed ? XML_CHANGE :
            ( bIsStart ? XML_CHANGE_START : XML_CHANGE_END ),
        true, true);
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

void XMLRedlineExport::WriteComment(std::u16string_view rComment)
{
    if (rComment.empty())
        return;

    // iterate over all string-pieces separated by return (0x0a) and
    // put each inside a paragraph element.
    SvXMLTokenEnumerator aEnumerator(rComment, char(0x0a));
    std::u16string_view aSubString;
    while (aEnumerator.getNextToken(aSubString))
    {
        SvXMLElementExport aParagraph(
            rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
        rExport.Characters(OUString(aSubString));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
