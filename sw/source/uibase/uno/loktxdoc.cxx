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
#include <sfx2/lokhelper.hxx>

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
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <com/sun/star/text/XTextContent.hpp>

#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/document/XDocumentProperties2.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <sax/tools/converter.hxx>

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
        auto it = rArguments.find(u"type"_ustr);
        if (it != rArguments.end())
        {
            aType = it->second;
        }

        it = rArguments.find(u"commandPrefix"_ustr);
        if (it != rArguments.end())
        {
            aCommandPrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    auto aFields = rJsonWriter.startArray("fields");
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        sw::mark::Fieldmark* pFieldmark = *it;
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

        auto aField = rJsonWriter.startStruct();
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
    auto it = rArguments.find(u"type"_ustr);
    if (it != rArguments.end())
    {
        aType = it->second;
    }

    it = rArguments.find(u"commandPrefix"_ustr);
    if (it != rArguments.end())
    {
        aCommandPrefix = it->second;
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    sw::mark::Fieldmark* pFieldmark = rIDMA.getInnerFieldmarkFor(rCursor);
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
    auto it = rArguments.find(u"namePrefix"_ustr);
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
    auto aProperties = rJsonWriter.startArray("userDefinedProperties");
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

        auto aProperty = rJsonWriter.startStruct();
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
        auto it = rArguments.find(u"namePrefix"_ustr);
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    auto aBookmarks = rJsonWriter.startArray("bookmarks");
    for (auto it = rIDMA.getBookmarksBegin(); it != rIDMA.getBookmarksEnd(); ++it)
    {
        sw::mark::MarkBase* pMark = *it;
        if (!pMark->GetName().startsWith(aNamePrefix))
        {
            continue;
        }

        auto aProperty = rJsonWriter.startStruct();
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
        auto it = rArguments.find(u"namePrefix"_ustr);
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    IDocumentMarkAccess& rIDMA = *pDocShell->GetDoc()->getIDocumentMarkAccess();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    sw::mark::MarkBase* pBookmark = rIDMA.getOneInnermostBookmarkFor(rCursor);
    auto aBookmark = rJsonWriter.startNode("bookmark");
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
        auto it = rArguments.find(u"typeName"_ustr);
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
        auto it = rArguments.find(u"namePrefix"_ustr);
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    auto aBookmarks = rJsonWriter.startArray("setRefs");
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

        auto aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", pRefMark->GetRefName());
    }
}

/// Implements getCommandValues(".uno:Layout").
void GetLayout(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell)
{
    rJsonWriter.put("commandName", ".uno:Layout");
    auto aCommandValues = rJsonWriter.startNode("commandValues");
    auto aPages = rJsonWriter.startArray("pages");
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    for (SwFrame* pFrame = pLayout->GetLower(); pFrame; pFrame = pFrame->GetNext())
    {
        auto pPage = pFrame->DynCastPageFrame();
        auto aPage = rJsonWriter.startStruct();
        rJsonWriter.put("isInvalidContent", pPage->IsInvalidContent());
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
        auto it = rArguments.find(u"typeName"_ustr);
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
        auto it = rArguments.find(u"namePrefix"_ustr);
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
    auto aRefmark = rJsonWriter.startNode("setRef");
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
                auto aRowNode = rJsonWriter.startAnonArray();
                for (int k = 0; k < aData[j].getLength(); k++)
                {
                    rJsonWriter.putSimpleValue(OUString::number(aData[j][k]));
                }
            }
        }
    }
}

void GetDocStructureDocProps(tools::JsonWriter& rJsonWriter, SwDocShell* pDocShell,
                             const std::map<OUString, OUString>& rArguments)
{
    auto it = rArguments.find(u"filter"_ustr);
    if (it != rArguments.end())
    {
        // If filter is present but we are filtering not to document properties
        if (!it->second.equals(u"docprops"_ustr))
            return;
    }

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropsSupplier(
        pDocShell->GetModel(), uno::UNO_QUERY);
    if (!xDocumentPropsSupplier.is())
        return;

    //uno::Reference<document::XDocumentProperties> xDocProps();
    uno::Reference<document::XDocumentProperties2> xDocProps(
        xDocumentPropsSupplier->getDocumentProperties(), uno::UNO_QUERY);
    if (!xDocProps.is())
        return;

    auto aDocPropsNode = rJsonWriter.startNode("DocumentProperties");

    // StringBuffer for converting DateTimes to String
    OUStringBuffer aDateBuf(32);

    //Properties from XDocumentProperties
    OUString aAuthor = xDocProps->getAuthor();
    rJsonWriter.put("Author", aAuthor);

    OUString aGenerator = xDocProps->getGenerator();
    rJsonWriter.put("Generator", aGenerator);

    util::DateTime aCreationDate = xDocProps->getCreationDate();
    sax::Converter::convertDateTime(aDateBuf, aCreationDate, nullptr, true);
    rJsonWriter.put("CreationDate", aDateBuf.makeStringAndClear());

    OUString aTitle = xDocProps->getTitle();
    rJsonWriter.put("Title", aTitle);

    OUString aSubject = xDocProps->getSubject();
    rJsonWriter.put("Subject", aSubject);

    OUString aDescription = xDocProps->getDescription();
    rJsonWriter.put("Description", aDescription);

    uno::Sequence<OUString> aKeywords = xDocProps->getKeywords();
    {
        auto aKeywordsNode = rJsonWriter.startArray("Keywords");
        for (int i = 0; i < aKeywords.getLength(); i++)
        {
            rJsonWriter.putSimpleValue(aKeywords[i]);
        }
    }

    lang::Locale aLanguage = xDocProps->getLanguage();
    OUString aLanguageStr(LanguageTag::convertToBcp47(aLanguage, false));
    rJsonWriter.put("Language", aLanguageStr);

    OUString aModifiedBy = xDocProps->getModifiedBy();
    rJsonWriter.put("ModifiedBy", aModifiedBy);

    util::DateTime aModificationDate = xDocProps->getModificationDate();
    sax::Converter::convertDateTime(aDateBuf, aModificationDate, nullptr, true);
    rJsonWriter.put("ModificationDate", aDateBuf.makeStringAndClear());

    OUString aPrintedBy = xDocProps->getPrintedBy();
    rJsonWriter.put("PrintedBy", aPrintedBy);

    util::DateTime aPrintDate = xDocProps->getPrintDate();
    sax::Converter::convertDateTime(aDateBuf, aPrintDate, nullptr, true);
    rJsonWriter.put("PrintDate", aDateBuf.makeStringAndClear());

    OUString aTemplateName = xDocProps->getTemplateName();
    rJsonWriter.put("TemplateName", aTemplateName);

    OUString aTemplateURL = xDocProps->getTemplateURL();
    rJsonWriter.put("TemplateURL", aTemplateURL);

    util::DateTime aTemplateDate = xDocProps->getTemplateDate();
    sax::Converter::convertDateTime(aDateBuf, aTemplateDate, nullptr, true);
    rJsonWriter.put("TemplateDate", aDateBuf.makeStringAndClear());

    OUString aAutoloadURL = xDocProps->getAutoloadURL();
    rJsonWriter.put("AutoloadURL", aAutoloadURL);

    sal_Int32 aAutoloadSecs = xDocProps->getAutoloadSecs();
    rJsonWriter.put("AutoloadSecs", aAutoloadSecs);

    OUString aDefaultTarget = xDocProps->getDefaultTarget();
    rJsonWriter.put("DefaultTarget", aDefaultTarget);

    uno::Sequence<beans::NamedValue> aDocumentStatistics = xDocProps->getDocumentStatistics();
    {
        auto aDocumentStatisticsNode = rJsonWriter.startNode("DocumentStatistics");
        for (int i = 0; i < aDocumentStatistics.getLength(); i++)
        {
            // Todo check: do all stast are integer numbers?
            sal_Int32 nValue = 0;
            aDocumentStatistics[i].Value >>= nValue;
            std::string aStr(OUStringToOString(aDocumentStatistics[i].Name, RTL_TEXTENCODING_UTF8));
            rJsonWriter.put(aStr, nValue);
        }
    }

    sal_Int16 aEditingCycles = xDocProps->getEditingCycles();
    rJsonWriter.put("EditingCycles", aEditingCycles);

    sal_Int32 aEditingDuration = xDocProps->getEditingDuration();
    rJsonWriter.put("EditingDuration", aEditingDuration);

    //Properties from XDocumentProperties2
    uno::Sequence<OUString> aContributor = xDocProps->getContributor();
    {
        auto aContributorNode = rJsonWriter.startArray("Contributor");
        for (int i = 0; i < aContributor.getLength(); i++)
        {
            rJsonWriter.putSimpleValue(aContributor[i]);
        }
    }

    OUString aCoverage = xDocProps->getCoverage();
    rJsonWriter.put("Coverage", aCoverage);

    OUString aIdentifier = xDocProps->getIdentifier();
    rJsonWriter.put("Identifier", aIdentifier);

    uno::Sequence<OUString> aPublisher = xDocProps->getPublisher();
    {
        auto aPublisherNode = rJsonWriter.startArray("Publisher");
        for (int i = 0; i < aPublisher.getLength(); i++)
        {
            rJsonWriter.putSimpleValue(aPublisher[i]);
        }
    }

    uno::Sequence<OUString> aRelation = xDocProps->getRelation();
    {
        auto aRelationNode = rJsonWriter.startArray("Relation");
        for (int i = 0; i < aRelation.getLength(); i++)
        {
            rJsonWriter.putSimpleValue(aRelation[i]);
        }
    }

    OUString aRights = xDocProps->getRights();
    rJsonWriter.put("Rights", aRights);

    OUString aSource = xDocProps->getSource();
    rJsonWriter.put("Source", aSource);

    OUString aType = xDocProps->getType();
    rJsonWriter.put("Type", aType);

    // PropertySet -> JSON
    css::uno::Reference<css::beans::XPropertyContainer> aUserDefinedProperties
        = xDocProps->getUserDefinedProperties();
    uno::Reference<beans::XPropertySet> aUserDefinedPropertySet(aUserDefinedProperties,
                                                                uno::UNO_QUERY);
    if (aUserDefinedPropertySet.is())
    {
        auto aRelationNode = rJsonWriter.startNode("UserDefinedProperties");
        const uno::Sequence<beans::Property> aProperties
            = aUserDefinedPropertySet->getPropertySetInfo()->getProperties();
        for (const beans::Property& rProperty : aProperties)
        {
            const OUString& rKey = rProperty.Name;
            auto aNode = rJsonWriter.startNode(OUStringToOString(rKey, RTL_TEXTENCODING_UTF8));
            uno::Any aValue = aUserDefinedPropertySet->getPropertyValue(rKey);

            OUString aAnyType = aValue.getValueTypeName();
            rJsonWriter.put("type", aAnyType);

            if (aAnyType == "boolean")
                rJsonWriter.put("value", aValue.get<bool>());
            else if (aAnyType == "double")
                rJsonWriter.put("value", aValue.get<double>());
            else if (aAnyType == "float")
                rJsonWriter.put("value", aValue.get<float>());
            else if (aAnyType == "long")
                rJsonWriter.put("value", aValue.get<sal_Int32>());
            else if (aAnyType == "short")
                rJsonWriter.put("value", aValue.get<sal_Int16>());
            else if (aValue.has<OUString>())
                rJsonWriter.put("value", aValue.get<OUString>());
            else if (aValue.has<sal_uInt64>())
                rJsonWriter.put("value", aValue.get<sal_Int64>());
            else
            {
                // Todo: some more types should be supported..
                // AddProperty allow these 13 types:
                // "com.sun.star.util.Date", "DateTime", "DateTimeWithTimezone",
                // "DateWithTimezone", "Duration", "Time"
                // typelib_TypeClass_BOOLEAN, typelib_TypeClass_DOUBLE, typelib_TypeClass_FLOAT
                // typelib_TypeClass_HYPER, typelib_TypeClass_LONG, typelib_TypeClass_SHORT,
                // typelib_TypeClass_STRING
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
        auto it = rArguments.find(u"namePrefix"_ustr);
        if (it != rArguments.end())
        {
            aNamePrefix = it->second;
        }
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    auto aBookmarks = rJsonWriter.startArray("sections");
    for (const auto& pSection : pDoc->GetSections())
    {
        if (!pSection->GetName().startsWith(aNamePrefix))
        {
            continue;
        }

        auto aProperty = rJsonWriter.startStruct();
        rJsonWriter.put("name", pSection->GetName());
    }
}
}

bool SwXTextDocument::supportsCommand(std::u16string_view rCommand)
{
    static const std::initializer_list<std::u16string_view> vForward
        = { u"TextFormFields", u"TextFormField", u"SetDocumentProperties",
            u"Bookmarks",      u"Fields",        u"Sections",
            u"Bookmark",       u"Field",         u"Layout" };

    return std::find(vForward.begin(), vForward.end(), rCommand) != vForward.end();
}

void SwXTextDocument::getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand)
{
    static constexpr OStringLiteral aTextFormFields(".uno:TextFormFields");
    static constexpr OStringLiteral aTextFormField(".uno:TextFormField");
    static constexpr OStringLiteral aSetDocumentProperties(".uno:SetDocumentProperties");
    static constexpr OStringLiteral aBookmarks(".uno:Bookmarks");
    static constexpr OStringLiteral aFields(".uno:Fields");
    static constexpr OStringLiteral aSections(".uno:Sections");
    static constexpr OStringLiteral aBookmark(".uno:Bookmark");
    static constexpr OStringLiteral aField(".uno:Field");
    static constexpr OStringLiteral aExtractDocStructure(".uno:ExtractDocumentStructure");
    static constexpr OStringLiteral aLayout(".uno:Layout");

    std::map<OUString, OUString> aMap
        = SfxLokHelper::parseCommandParameters(OUString::fromUtf8(rCommand));

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
        GetDocStructureDocProps(rJsonWriter, m_pDocShell, aMap);
    }
    else if (o3tl::starts_with(rCommand, aLayout))
    {
        GetLayout(rJsonWriter, m_pDocShell);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
