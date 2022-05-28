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

#include <formatcontentcontrol.hxx>

#include <libxml/xmlwriter.h>

#include <sal/log.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <svl/numformat.hxx>

#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <doc.hxx>

using namespace com::sun::star;

namespace
{
inline constexpr OUStringLiteral CURRENT_DATE_FORMAT = u"YYYY-MM-DD";
}

SwFormatContentControl* SwFormatContentControl::CreatePoolDefault(sal_uInt16 nWhich)
{
    return new SwFormatContentControl(nWhich);
}

SwFormatContentControl::SwFormatContentControl(sal_uInt16 nWhich)
    : SfxPoolItem(nWhich)
    , m_pTextAttr(nullptr)
{
}

SwFormatContentControl::SwFormatContentControl(
    const std::shared_ptr<SwContentControl>& pContentControl, sal_uInt16 nWhich)
    : SfxPoolItem(nWhich)
    , m_pContentControl(pContentControl)
    , m_pTextAttr(nullptr)
{
    if (!pContentControl)
    {
        SAL_WARN("sw.core", "SwFormatContentControl ctor: no pContentControl?");
    }
    // Not calling m_pContentControl->SetFormatContentControl(this) here; only from SetTextAttr.
}

SwFormatContentControl::~SwFormatContentControl()
{
    if (m_pContentControl && (m_pContentControl->GetFormatContentControl() == this))
    {
        NotifyChangeTextNode(nullptr);
        m_pContentControl->SetFormatContentControl(nullptr);
    }
}

bool SwFormatContentControl::operator==(const SfxPoolItem& rOther) const
{
    return SfxPoolItem::operator==(rOther)
           && m_pContentControl
                  == static_cast<const SwFormatContentControl&>(rOther).m_pContentControl;
}

SwFormatContentControl* SwFormatContentControl::Clone(SfxItemPool* /*pPool*/) const
{
    // If this is indeed a copy, then DoCopy will be called later.
    if (m_pContentControl)
    {
        return new SwFormatContentControl(m_pContentControl, Which());
    }
    else
    {
        return new SwFormatContentControl(Which());
    }
}

void SwFormatContentControl::SetTextAttr(SwTextContentControl* pTextAttr)
{
    if (m_pTextAttr && pTextAttr)
    {
        SAL_WARN("sw.core", "SwFormatContentControl::SetTextAttr: already has a text attribute");
    }
    if (!m_pTextAttr && !pTextAttr)
    {
        SAL_WARN("sw.core", "SwFormatContentControl::SetTextAttr: no attribute to remove");
    }
    m_pTextAttr = pTextAttr;
    if (!m_pContentControl)
    {
        SAL_WARN("sw.core", "inserted SwFormatContentControl has no SwContentControl");
    }
    // The SwContentControl should be able to find the current text attribute.
    if (m_pContentControl)
    {
        if (pTextAttr)
        {
            m_pContentControl->SetFormatContentControl(this);
        }
        else if (m_pContentControl->GetFormatContentControl() == this)
        {
            // The text attribute is gone, so de-register from text node.
            NotifyChangeTextNode(nullptr);
            m_pContentControl->SetFormatContentControl(nullptr);
        }
    }
}

void SwFormatContentControl::NotifyChangeTextNode(SwTextNode* pTextNode)
{
    // Not deleting m_pTextAttr here, SwNodes::ChgNode() doesn't do that, either.
    if (!m_pContentControl)
    {
        SAL_WARN("sw.core", "SwFormatContentControl::NotifyChangeTextNode: no content control?");
    }
    if (m_pContentControl && (m_pContentControl->GetFormatContentControl() == this))
    {
        // Not calling Modify, that would call SwXContentControl::SwClientNotify.
        m_pContentControl->NotifyChangeTextNode(pTextNode);
    }
}

// This SwFormatContentControl has been cloned and points at the same SwContentControl as the
// source: this function copies the SwContentControl.
void SwFormatContentControl::DoCopy(SwTextNode& rTargetTextNode)
{
    if (!m_pContentControl)
    {
        SAL_WARN("sw.core", "SwFormatContentControl::DoCopy: called for SwFormatContentControl "
                            "with no SwContentControl.");
        return;
    }

    m_pContentControl = std::make_shared<SwContentControl>(this);
    m_pContentControl->NotifyChangeTextNode(&rTargetTextNode);
}

void SwFormatContentControl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatContentControl"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("m_pTextAttr"), "%p", m_pTextAttr);
    SfxPoolItem::dumpAsXml(pWriter);

    if (m_pContentControl)
    {
        m_pContentControl->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

SwContentControl::SwContentControl(SwFormatContentControl* pFormat)
    : sw::BroadcastingModify()
    , m_pFormat(pFormat)
    , m_pTextNode(nullptr)
{
}

SwContentControl::~SwContentControl() {}

SwTextContentControl* SwContentControl::GetTextAttr() const
{
    return m_pFormat ? m_pFormat->GetTextAttr() : nullptr;
}

void SwContentControl::NotifyChangeTextNode(SwTextNode* pTextNode)
{
    m_pTextNode = pTextNode;
    if (m_pTextNode && (GetRegisteredIn() != m_pTextNode))
    {
        m_pTextNode->Add(this);
    }
    else if (!m_pTextNode)
    {
        EndListeningAll();
    }
    if (!pTextNode)
    {
        // If the text node is gone, then invalidate clients (e.g. UNO object).
        GetNotifier().Broadcast(SfxHint(SfxHintId::Deinitializing));
    }
}

void SwContentControl::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;

    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CallSwClientNotify(rHint);
    GetNotifier().Broadcast(SfxHint(SfxHintId::DataChanged));

    if (pLegacy->GetWhich() == RES_REMOVE_UNO_OBJECT)
    {
        // Invalidate cached uno object.
        SetXContentControl(uno::Reference<text::XTextContent>());
        GetNotifier().Broadcast(SfxHint(SfxHintId::Deinitializing));
    }
}

OUString SwContentControl::GetDateString() const
{
    SwDoc& rDoc = m_pTextNode->GetDoc();
    SvNumberFormatter* pNumberFormatter = rDoc.GetNumberFormatter();
    sal_uInt32 nFormat = pNumberFormatter->GetEntryKey(
        m_aDateFormat, LanguageTag(m_aDateLanguage).getLanguageType());

    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        // If not found, then create it.
        sal_Int32 nCheckPos = 0;
        SvNumFormatType nType;
        OUString aFormat = m_aDateFormat;
        pNumberFormatter->PutEntry(aFormat, nCheckPos, nType, nFormat,
                                   LanguageTag(m_aDateLanguage).getLanguageType());
    }

    const Color* pColor = nullptr;
    OUString aFormatted;
    if (!m_oSelectedDate)
    {
        return OUString();
    }

    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        return OUString();
    }

    pNumberFormatter->GetOutputString(*m_oSelectedDate, nFormat, aFormatted, &pColor, false);
    return aFormatted;
}

void SwContentControl::SetCurrentDateValue(double fCurrentDate)
{
    SwDoc& rDoc = m_pTextNode->GetDoc();
    SvNumberFormatter* pNumberFormatter = rDoc.GetNumberFormatter();
    OUString aFormatted;
    sal_uInt32 nFormat = pNumberFormatter->GetEntryKey(CURRENT_DATE_FORMAT, LANGUAGE_ENGLISH_US);
    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        // If not found, then create it.
        sal_Int32 nCheckPos = 0;
        SvNumFormatType nType;
        OUString sFormat = CURRENT_DATE_FORMAT;
        pNumberFormatter->PutEntry(sFormat, nCheckPos, nType, nFormat, LANGUAGE_ENGLISH_US);
    }

    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        return;
    }

    const Color* pColor = nullptr;
    pNumberFormatter->GetOutputString(fCurrentDate, nFormat, aFormatted, &pColor, false);
    m_aCurrentDate = aFormatted + "T00:00:00Z";
}

double SwContentControl::GetCurrentDateValue() const
{
    if (m_aCurrentDate.isEmpty())
    {
        return 0;
    }

    SwDoc& rDoc = m_pTextNode->GetDoc();
    SvNumberFormatter* pNumberFormatter = rDoc.GetNumberFormatter();
    sal_uInt32 nFormat = pNumberFormatter->GetEntryKey(CURRENT_DATE_FORMAT, LANGUAGE_ENGLISH_US);
    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        sal_Int32 nCheckPos = 0;
        SvNumFormatType nType;
        OUString sFormat = CURRENT_DATE_FORMAT;
        pNumberFormatter->PutEntry(sFormat, nCheckPos, nType, nFormat, LANGUAGE_ENGLISH_US);
    }

    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        return 0;
    }

    double dCurrentDate = 0;
    OUString aCurrentDate = m_aCurrentDate.replaceAll("T00:00:00Z", "");
    (void)pNumberFormatter->IsNumberFormat(aCurrentDate, nFormat, dCurrentDate);
    return dCurrentDate;
}

void SwContentControl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwContentControl"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(
        pWriter, BAD_CAST("showing-place-holder"), "%s",
        BAD_CAST(OString::boolean(m_bShowingPlaceHolder).getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("checkbox"), "%s",
                                            BAD_CAST(OString::boolean(m_bCheckbox).getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("checked"), "%s",
                                            BAD_CAST(OString::boolean(m_bChecked).getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("checked-state"), "%s",
                                            BAD_CAST(m_aCheckedState.toUtf8().getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("unchecked-state"), "%s",
                                            BAD_CAST(m_aUncheckedState.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("picture"),
                                      BAD_CAST(OString::boolean(m_bPicture).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date"),
                                      BAD_CAST(OString::boolean(m_bDate).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date-format"),
                                      BAD_CAST(m_aDateFormat.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date-language"),
                                      BAD_CAST(m_aDateLanguage.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("current-date"),
                                      BAD_CAST(m_aCurrentDate.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("placeholder-doc-part"),
                                      BAD_CAST(m_aPlaceholderDocPart.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("data-binding-prefix-mappings"),
                                      BAD_CAST(m_aDataBindingPrefixMappings.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("data-binding-xpath"),
                                      BAD_CAST(m_aDataBindingXpath.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("data-binding-store-item-id"),
                                      BAD_CAST(m_aDataBindingStoreItemID.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("color"),
                                      BAD_CAST(m_aColor.toUtf8().getStr()));

    if (!m_aListItems.empty())
    {
        for (const auto& rListItem : m_aListItems)
        {
            rListItem.dumpAsXml(pWriter);
        }
    }

    (void)xmlTextWriterEndElement(pWriter);
}

void SwContentControlListItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwContentControlListItem"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("display-text"),
                                      BAD_CAST(m_aDisplayText.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(m_aValue.toUtf8().getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

OUString SwContentControlListItem::ToString() const
{
    if (!m_aDisplayText.isEmpty())
    {
        return m_aDisplayText;
    }

    return m_aValue;
}

bool SwContentControlListItem::operator==(const SwContentControlListItem& rOther) const
{
    return m_aDisplayText == rOther.m_aDisplayText && m_aValue == rOther.m_aValue;
}

void SwContentControlListItem::ItemsToAny(const std::vector<SwContentControlListItem>& rItems,
                                          uno::Any& rVal)
{
    uno::Sequence<uno::Sequence<beans::PropertyValue>> aRet(rItems.size());

    uno::Sequence<beans::PropertyValue>* pRet = aRet.getArray();
    for (size_t i = 0; i < rItems.size(); ++i)
    {
        const SwContentControlListItem& rItem = rItems[i];
        uno::Sequence<beans::PropertyValue> aItem = {
            comphelper::makePropertyValue("DisplayText", rItem.m_aDisplayText),
            comphelper::makePropertyValue("Value", rItem.m_aValue),
        };
        pRet[i] = aItem;
    }

    rVal <<= aRet;
}

std::vector<SwContentControlListItem>
SwContentControlListItem::ItemsFromAny(const css::uno::Any& rVal)
{
    std::vector<SwContentControlListItem> aRet;

    uno::Sequence<uno::Sequence<beans::PropertyValue>> aSequence;
    rVal >>= aSequence;
    for (const auto& rItem : aSequence)
    {
        comphelper::SequenceAsHashMap aMap(rItem);
        SwContentControlListItem aItem;
        auto it = aMap.find("DisplayText");
        if (it != aMap.end())
        {
            it->second >>= aItem.m_aDisplayText;
        }
        it = aMap.find("Value");
        if (it != aMap.end())
        {
            it->second >>= aItem.m_aValue;
        }
        aRet.push_back(aItem);
    }

    return aRet;
}

SwTextContentControl* SwTextContentControl::CreateTextContentControl(SwTextNode* pTargetTextNode,
                                                                     SwFormatContentControl& rAttr,
                                                                     sal_Int32 nStart,
                                                                     sal_Int32 nEnd, bool bIsCopy)
{
    if (bIsCopy)
    {
        // rAttr is already cloned, now call DoCopy to copy the SwContentControl
        if (!pTargetTextNode)
        {
            SAL_WARN("sw.core",
                     "SwTextContentControl ctor: cannot copy content control without target node");
        }
        rAttr.DoCopy(*pTargetTextNode);
    }
    auto pTextContentControl(new SwTextContentControl(rAttr, nStart, nEnd));
    return pTextContentControl;
}

SwTextContentControl::SwTextContentControl(SwFormatContentControl& rAttr, sal_Int32 nStart,
                                           sal_Int32 nEnd)
    : SwTextAttr(rAttr, nStart)
    , SwTextAttrNesting(rAttr, nStart, nEnd)
{
    rAttr.SetTextAttr(this);
    SetHasDummyChar(true);
}

SwTextContentControl::~SwTextContentControl()
{
    auto& rFormatContentControl = static_cast<SwFormatContentControl&>(GetAttr());
    if (rFormatContentControl.GetTextAttr() == this)
    {
        rFormatContentControl.SetTextAttr(nullptr);
    }
}

void SwTextContentControl::ChgTextNode(SwTextNode* pNode)
{
    auto& rFormatContentControl = static_cast<SwFormatContentControl&>(GetAttr());
    if (rFormatContentControl.GetTextAttr() == this)
    {
        rFormatContentControl.NotifyChangeTextNode(pNode);
    }
}

void SwTextContentControl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextContentControl"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    SwTextAttr::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
