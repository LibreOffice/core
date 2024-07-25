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

#include <unotxdoc.hxx>

#include <map>
#include <utility>
#include <vector>

#include <com/sun/star/beans/XPropertyAccess.hpp>

#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <tools/json_writer.hxx>
#include <tools/urlobj.hxx>
#include <xmloff/odffields.hxx>

#include <IDocumentMarkAccess.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <fmtrfmrk.hxx>
#include <wrtsh.hxx>
#include <txtrfmrk.hxx>
#include <ndtxt.hxx>

#include <unoport.hxx>
#include <unoprnms.hxx>
#include <unocontentcontrol.hxx>
#include <com/sun/star/text/XTextContent.hpp>

#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>

using namespace ::com::sun::star;

namespace
{
/// Implements getCommandValues(".uno:TextFormFields").
///
/// Parameters:
///
/// - type: e.g. ODF_UNHANDLED
/// - commandPrefix: field command prefix to not return all fieldmarks
void GetTextFormFields(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                       const std::map<OUString, OUString>& rArguments)
{
    OUString aType;
    OUString aCommandPrefix;
    {
        auto it = rArguments.find("type");
        if (it != rArguments.end())
        {
            aType = it->second;
        }

        it = rArguments.find("commandPrefix");
        if (it != rArguments.end())
        {
            aCommandPrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    tools::ScopedJsonWriterArray aFields = rJsonWriter.startArray("fields");
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        auto pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(*it);
        assert(pFieldmark);
        if (pFieldmark->GetFieldname() != aType)
        {
            continue;
        }

        auto itParam = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
        if (itParam == pFieldmark->GetParameters()->end())
        {
            continue;
        }

        OUString aCommand;
        itParam->second >>= aCommand;
        if (!aCommand.startsWith(aCommandPrefix))
        {
            continue;
        }

        tools::ScopedJsonWriterStruct aField = rJsonWriter.startStruct();
        rJsonWriter.put("type", aType);
        rJsonWriter.put("command", aCommand);
    }
}

/// Implements getCommandValues(".uno:TextFormField").
///
/// Parameters:
///
/// - type: e.g. ODF_UNHANDLED
/// - commandPrefix: field command prefix to not return all fieldmarks
void GetTextFormField(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                      const std::map<OUString, OUString>& rArguments)
{
    OUString aType;
    OUString aCommandPrefix;
    auto it = rArguments.find("type");
    if (it != rArguments.end())
    {
        aType = it->second;
    }

    it = rArguments.find("commandPrefix");
    if (it != rArguments.end())
    {
        aCommandPrefix = it->second;
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    sw::mark::IFieldmark* pFieldmark = rIDMA.getInnerFieldmarkFor(rCursor);
    auto typeNode = rJsonWriter.startNode("field");
    if (!pFieldmark)
    {
        return;
    }

    if (pFieldmark->GetFieldname() != aType)
    {
        return;
    }

    auto itParam = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
    if (itParam == pFieldmark->GetParameters()->end())
    {
        return;
    }

    OUString aCommand;
    itParam->second >>= aCommand;
    if (!aCommand.startsWith(aCommandPrefix))
    {
        return;
    }

    rJsonWriter.put("type", aType);
    rJsonWriter.put("command", aCommand);
}

/// Implements getCommandValues(".uno:SetDocumentProperties").
///
/// Parameters:
///
/// - namePrefix: field name prefix to not return all user-defined properties
void GetDocumentProperties(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                           const std::map<OUString, OUString>& rArguments)
{
    OUString aNamePrefix;
    auto it = rArguments.find("namePrefix");
    if (it != rArguments.end())
    {
        aNamePrefix = it->second;
    }

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(),
                                                               uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDP = xDPS->getDocumentProperties();
    uno::Reference<beans::XPropertyAccess> xUDP(xDP->getUserDefinedProperties(), uno::UNO_QUERY);
    auto aUDPs = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(
        xUDP->getPropertyValues());
    tools::ScopedJsonWriterArray aProperties = rJsonWriter.startArray("userDefinedProperties");
    for (const auto& rUDP : aUDPs)
    {
        if (!rUDP.Name.startsWith(aNamePrefix))
        {
            continue;
        }

        if (rUDP.Value.getValueTypeClass() != uno::TypeClass_STRING)
        {
            continue;
        }

        OUString aValue;
        rUDP.Value >>= aValue;

        tools::ScopedJsonWriterStruct aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", rUDP.Name);
        rJsonWriter.put("type", "string");
        rJsonWriter.put("value", aValue);
    }
}

/// Implements getCommandValues(".uno:Bookmarks").
///
/// Parameters:
///
/// - namePrefix: bookmark name prefix to not return all bookmarks
void GetBookmarks(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                  const std::map<OUString, OUString>& rArguments)
{
    OUString aNamePrefix;
    {
        auto it = rArguments.find("namePrefix");
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    tools::ScopedJsonWriterArray aBookmarks = rJsonWriter.startArray("bookmarks");
    for (auto it = rIDMA.getBookmarksBegin(); it != rIDMA.getBookmarksEnd(); ++it)
    {
        sw::mark::IMark* pMark = *it;
        if (!pMark->GetName().startsWith(aNamePrefix))
        {
            continue;
        }

        tools::ScopedJsonWriterStruct aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", pMark->GetName());
    }
}

/// Implements getCommandValues(".uno:Bookmark").
///
/// Parameters:
///
/// - namePrefix: bookmark name prefix to not return all bookmarks
void GetBookmark(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                 const std::map<OUString, OUString>& rArguments)
{
    OUString aNamePrefix;
    {
        auto it = rArguments.find("namePrefix");
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    sw::mark::IMark* pBookmark = rIDMA.getOneInnermostBookmarkFor(rCursor);
    tools::ScopedJsonWriterNode aBookmark = rJsonWriter.startNode("bookmark");
    if (!pBookmark)
    {
        return;
    }

    if (!pBookmark->GetName().startsWith(aNamePrefix))
    {
        return;
    }

    rJsonWriter.put("name", pBookmark->GetName());
}

/// Implements getCommandValues(".uno:Fields").
///
/// Parameters:
///
/// - typeName: field type condition to not return all fields
/// - namePrefix: field name prefix to not return all fields
void GetFields(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
               const std::map<OUString, OUString>& rArguments)
{
    OUString aTypeName;
    {
        auto it = rArguments.find("typeName");
        if (it != rArguments.end())
        {
            aTypeName = it->second;
        }
    }
    // See SwFieldTypeFromString().
    if (aTypeName != "SetRef")
    {
        return;
    }

    OUString aNamePrefix;
    {
        auto it = rArguments.find("namePrefix");
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    tools::ScopedJsonWriterArray aBookmarks = rJsonWriter.startArray("setRefs");
    std::vector<const SwFormatRefMark*> aRefMarks;
    for (sal_uInt16 i = 0; i < pDoc->GetRefMarks(); ++i)
    {
        aRefMarks.push_back(pDoc->GetRefMark(i));
    }
    // Sort the refmarks based on their start position.
    std::sort(aRefMarks.begin(), aRefMarks.end(),
              [](const SwFormatRefMark* pMark1, const SwFormatRefMark* pMark2) -> bool {
                  const SwTextRefMark* pTextRefMark1 = pMark1->GetTextRefMark();
                  const SwTextRefMark* pTextRefMark2 = pMark2->GetTextRefMark();
                  SwPosition aPos1(pTextRefMark1->GetTextNode(), pTextRefMark1->GetStart());
                  SwPosition aPos2(pTextRefMark2->GetTextNode(), pTextRefMark2->GetStart());
                  return aPos1 < aPos2;
              });

    for (const auto& pRefMark : aRefMarks)
    {
        if (!pRefMark->GetRefName().startsWith(aNamePrefix))
        {
            continue;
        }

        tools::ScopedJsonWriterStruct aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", pRefMark->GetRefName());
    }
}

/// Implements getCommandValues(".uno:Field").
///
/// Parameters:
///
/// - typeName: field type condition to not return all fields
/// - namePrefix: field name prefix to not return all fields
void GetField(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
              const std::map<OUString, OUString>& rArguments)
{
    OUString aTypeName;
    {
        auto it = rArguments.find("typeName");
        if (it != rArguments.end())
        {
            aTypeName = it->second;
        }
    }
    // See SwFieldTypeFromString().
    if (aTypeName != "SetRef")
    {
        return;
    }

    OUString aNamePrefix;
    {
        auto it = rArguments.find("namePrefix");
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    SwTextNode* pTextNode = rCursor.GetNode().GetTextNode();
    std::vector<SwTextAttr*> aAttrs
        = pTextNode->GetTextAttrsAt(rCursor.GetContentIndex(), RES_TXTATR_REFMARK);
    tools::ScopedJsonWriterNode aRefmark = rJsonWriter.startNode("setRef");
    if (aAttrs.empty())
    {
        return;
    }

    const SwFormatRefMark& rRefmark = aAttrs[0]->GetRefMark();
    if (!rRefmark.GetRefName().startsWith(aNamePrefix))
    {
        return;
    }

    rJsonWriter.put("name", rRefmark.GetRefName());
}

/// Implements getCommandValues(".uno:ExtractDocumentStructures").
///
/// Parameters:
///
/// - filter: To filter what document structure types to extract
///   now, only contentcontrol is supported.
void GetDocStructure(tools::JsonWriter& rJsonWriter, SwDocShell* /*pDocShell*/,
                     const std::map<OUString, OUString>& rArguments,
                     uno::Reference<container::XIndexAccess>& xContentControls)
{
    auto it = rArguments.find(u"filter"_ustr);
    if (it != rArguments.end())
    {
        // If filter is present but we are filtering not to contentcontrols
        if (!it->second.equals(u"contentcontrol"_ustr))
            return;
    }

    int iCCcount = xContentControls->getCount();

    for (int i = 0; i < iCCcount; ++i)
    {
        OString aNodeName("ContentControls.ByIndex."_ostr + OString::number(i));
        auto ContentControlNode = rJsonWriter.startNode(aNodeName);

        uno::Reference<text::XTextContent> xContentControl;

        xContentControls->getByIndex(i) >>= xContentControl;

        uno::Reference<text::XText> xContentControlText(xContentControl, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);

        sal_Int32 iID = -1;
        xContentControlProps->getPropertyValue(UNO_NAME_ID) >>= iID;
        rJsonWriter.put("id", iID);

        OUString aTag;
        xContentControlProps->getPropertyValue(UNO_NAME_TAG) >>= aTag;
        rJsonWriter.put("tag", aTag);

        OUString aAlias;
        xContentControlProps->getPropertyValue(UNO_NAME_ALIAS) >>= aAlias;
        rJsonWriter.put("alias", aAlias);

        sal_Int32 iType(0);
        xContentControlProps->getPropertyValue(UNO_NAME_CONTENT_CONTROL_TYPE) >>= iType;
        SwContentControlType aType = static_cast<SwContentControlType>(iType);

        bool bShowingPlaceHolder = false;
        xContentControlProps->getPropertyValue(UNO_NAME_SHOWING_PLACE_HOLDER)
            >>= bShowingPlaceHolder;
        OUString aContent;
        if (!bShowingPlaceHolder)
        {
            aContent = xContentControlText->getString();
        }
        rJsonWriter.put("content", aContent);

        switch (aType)
        {
            case SwContentControlType::RICH_TEXT:
            {
                rJsonWriter.put("type", "rich-text");
            }
            break;
            case SwContentControlType::CHECKBOX:
            {
                rJsonWriter.put("type", "checkbox");
                bool bchecked = false;
                xContentControlProps->getPropertyValue(UNO_NAME_CHECKED) >>= bchecked;
                rJsonWriter.put(UNO_NAME_CHECKED, OUString::boolean(bchecked));
            }
            break;
            case SwContentControlType::DROP_DOWN_LIST:
            {
                rJsonWriter.put("type", "drop-down-list");
                // we could list its elements if we want
            }
            break;
            case SwContentControlType::PICTURE:
            {
                rJsonWriter.put("type", "picture");
            }
            break;
            case SwContentControlType::DATE:
            {
                rJsonWriter.put("type", "date");
                OUString aDateFormat;
                xContentControlProps->getPropertyValue(UNO_NAME_DATE_FORMAT) >>= aDateFormat;
                rJsonWriter.put(UNO_NAME_DATE_FORMAT, aDateFormat);
                OUString aDateLanguage;
                xContentControlProps->getPropertyValue(UNO_NAME_DATE_LANGUAGE) >>= aDateLanguage;
                rJsonWriter.put(UNO_NAME_DATE_LANGUAGE, aDateLanguage);
                OUString aCurrentDate;
                xContentControlProps->getPropertyValue(UNO_NAME_CURRENT_DATE) >>= aCurrentDate;
                rJsonWriter.put(UNO_NAME_CURRENT_DATE, aCurrentDate);
            }
            break;
            case SwContentControlType::PLAIN_TEXT:
            {
                rJsonWriter.put("type", "plain-text");
            }
            break;
            case SwContentControlType::COMBO_BOX:
            {
                rJsonWriter.put("type", "combo-box");
                // we could list its elements if we want
            }
            break;
            default:
                //it should never happen
                rJsonWriter.put("type", "no type?");
        }
    }
}

void GetDocStructureCharts(tools::JsonWriter& rJsonWriter, SwDocShell* /*pDocShell*/,
                           const std::map<OUString, OUString>& rArguments,
                           uno::Reference<container::XIndexAccess>& xEmbeddeds)
{
    auto it = rArguments.find(u"filter"_ustr);
    if (it != rArguments.end())
    {
        // If filter is present but we are filtering not to charts
        if (!it->second.equals(u"charts"_ustr))
            return;
    }

    sal_Int32 nEOcount = xEmbeddeds->getCount();

    for (int i = 0; i < nEOcount; ++i)
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xEmbeddeds->getByIndex(i), uno::UNO_QUERY);
        if (!xShapeProps.is())
            continue;

        uno::Reference<frame::XModel> xDocModel;
        xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
        if (!xDocModel.is())
            continue;

        uno::Reference<chart2::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
        if (!xChartDoc.is())
            continue;

        uno::Reference<chart2::data::XDataProvider> xDataProvider(xChartDoc->getDataProvider());
        if (!xDataProvider.is())
            continue;

        uno::Reference<chart::XChartDataArray> xDataArray(xChartDoc->getDataProvider(),
                                                          uno::UNO_QUERY);
        if (!xDataArray.is())
            continue;

        uno::Reference<chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
        if (!xDiagram.is())
            continue;

        //we have the chart Data now, we can start to extract it
        OString aNodeName("Charts.ByEmbedIndex."_ostr + OString::number(i));
        auto aChartNode = rJsonWriter.startNode(aNodeName);

        //get the object name
        uno::Reference<container::XNamed> xNamedShape(xEmbeddeds->getByIndex(i), uno::UNO_QUERY);
        if (xNamedShape.is())
        {
            OUString aName;
            aName = xNamedShape->getName();
            rJsonWriter.put("name", aName);
        }

        //get the chart title, if there is one
        uno::Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
        if (xTitled.is())
        {
            uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
            if (xTitle.is())
            {
                OUString aTitle;
                const uno::Sequence<uno::Reference<chart2::XFormattedString>> aFSSeq
                    = xTitle->getText();
                for (auto const& fs : aFSSeq)
                    aTitle += fs->getString();
                rJsonWriter.put("title", aTitle);
            }
        }

        //get the chart subtitle, if there is one
        uno::Reference<chart2::XTitled> xSubTitled(xDiagram, uno::UNO_QUERY_THROW);
        if (xSubTitled.is())
        {
            uno::Reference<chart2::XTitle> xSubTitle = xSubTitled->getTitleObject();
            if (xSubTitle.is())
            {
                OUString aSubTitle;
                const uno::Sequence<uno::Reference<chart2::XFormattedString>> aFSSeq
                    = xSubTitle->getText();
                for (auto const& fs : aFSSeq)
                    aSubTitle += fs->getString();
                rJsonWriter.put("subtitle", aSubTitle);
            }
        }

        {
            uno::Sequence<OUString> aRowDesc = xDataArray->getRowDescriptions();
            auto aRowDescNode = rJsonWriter.startArray("RowDescriptions");
            for (int j = 0; j < aRowDesc.getLength(); j++)
            {
                rJsonWriter.putSimpleValue(aRowDesc[j]);
            }
        }
        {
            uno::Sequence<OUString> aColDesc = xDataArray->getColumnDescriptions();
            auto aColDescNode = rJsonWriter.startArray("ColumnDescriptions");
            for (int j = 0; j < aColDesc.getLength(); j++)
            {
                rJsonWriter.putSimpleValue(aColDesc[j]);
            }
        }
        {
            uno::Sequence<uno::Sequence<double>> aData = xDataArray->getData();
            auto aDataValuesNode = rJsonWriter.startArray("DataValues");
            for (int j = 0; j < aData.getLength(); j++)
            {
                OString aRowNodeName("Row."_ostr + OString::number(j));
                auto aRowNode = rJsonWriter.startArray(aRowNodeName);
                for (int k = 0; k < aData[j].getLength(); k++)
                {
                    rJsonWriter.putSimpleValue(OUString::number(aData[j][k]));
                }
            }
        }
    }
}

/// Implements getCommandValues(".uno:Sections").
///
/// Parameters:
///
/// - namePrefix: field name prefix to not return all sections
void GetSections(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                 const std::map<OUString, OUString>& rArguments)
{
    OUString aNamePrefix;
    {
        auto it = rArguments.find("namePrefix");
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    tools::ScopedJsonWriterArray aBookmarks = rJsonWriter.startArray("sections");
    for (const auto& pSection : pDoc->GetSections())
    {
        if (!pSection->GetName().startsWith(aNamePrefix))
        {
            continue;
        }

        tools::ScopedJsonWriterStruct aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", pSection->GetName());
    }
}
}

bool SwXTextDocument::supportsCommand(std::u16string_view rCommand)
{
    static const std::initializer_list<std::u16string_view> vForward
        = { u"TextFormFields", u"TextFormField", u"SetDocumentProperties",
            u"Bookmarks",      u"Fields",        u"Sections",
            u"Bookmark",       u"Field" };

    return std::find(vForward.begin(), vForward.end(), rCommand) != vForward.end();
}

void SwXTextDocument::getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand)
{
    std::map<OUString, OUString> aMap;

    static constexpr OStringLiteral aTextFormFields(".uno:TextFormFields");
    static constexpr OStringLiteral aTextFormField(".uno:TextFormField");
    static constexpr OStringLiteral aSetDocumentProperties(".uno:SetDocumentProperties");
    static constexpr OStringLiteral aBookmarks(".uno:Bookmarks");
    static constexpr OStringLiteral aFields(".uno:Fields");
    static constexpr OStringLiteral aSections(".uno:Sections");
    static constexpr OStringLiteral aBookmark(".uno:Bookmark");
    static constexpr OStringLiteral aField(".uno:Field");
    static constexpr OStringLiteral aExtractDocStructure(".uno:ExtractDocumentStructure");

    INetURLObject aParser(OUString::fromUtf8(rCommand));
    OUString aArguments = aParser.GetParam();
    sal_Int32 nParamIndex = 0;
    do
    {
        std::u16string_view aParam = o3tl::getToken(aArguments, 0, '&', nParamIndex);
        sal_Int32 nIndex = 0;
        OUString aKey;
        OUString aValue;
        do
        {
            std::u16string_view aToken = o3tl::getToken(aParam, 0, '=', nIndex);
            if (aKey.isEmpty())
                aKey = aToken;
            else
                aValue = aToken;
        } while (nIndex >= 0);
        OUString aDecodedValue
            = INetURLObject::decode(aValue, INetURLObject::DecodeMechanism::WithCharset);
        aMap[aKey] = aDecodedValue;
    } while (nParamIndex >= 0);

    if (o3tl::starts_with(rCommand, aTextFormFields))
    {
        GetTextFormFields(rJsonWriter, m_pDocShell, aMap);
    }
    if (o3tl::starts_with(rCommand, aTextFormField))
    {
        GetTextFormField(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aSetDocumentProperties))
    {
        GetDocumentProperties(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aBookmarks))
    {
        GetBookmarks(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aFields))
    {
        GetFields(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aSections))
    {
        GetSections(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aBookmark))
    {
        GetBookmark(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aField))
    {
        GetField(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aExtractDocStructure))
    {
        auto commentsNode = rJsonWriter.startNode("DocStructure");

        uno::Reference<container::XIndexAccess> xEmbeddeds(getEmbeddedObjects(), uno::UNO_QUERY);
        if (xEmbeddeds.is())
        {
            GetDocStructureCharts(rJsonWriter, m_pDocShell, aMap, xEmbeddeds);
        }

        uno::Reference<container::XIndexAccess> xContentControls = getContentControls();
        GetDocStructure(rJsonWriter, m_pDocShell, aMap, xContentControls);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
