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
#include <vcl/keycod.hxx>

#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <doc.hxx>
#include <unocontentcontrol.hxx>
#include <unoport.hxx>
#include <wrtsh.hxx>

using namespace com::sun::star;

namespace
{
inline constexpr OUString CURRENT_DATE_FORMAT = u"YYYY-MM-DD"_ustr;
}

SwFormatContentControl* SwFormatContentControl::CreatePoolDefault(sal_uInt16 nWhich)
{
    return new SwFormatContentControl(nWhich);
}

SwFormatContentControl::SwFormatContentControl(sal_uInt16 nWhich)
    : SfxPoolItem(nWhich)
    , m_pTextAttr(nullptr)
{
    setNonShareable();
}

SwFormatContentControl::SwFormatContentControl(
    const std::shared_ptr<SwContentControl>& pContentControl, sal_uInt16 nWhich)
    : SfxPoolItem(nWhich)
    , m_pContentControl(pContentControl)
    , m_pTextAttr(nullptr)
{
    setNonShareable();
    if (!pContentControl)
    {
        SAL_WARN("sw.core", "SwFormatContentControl ctor: no pContentControl?");
    }
    // Not calling m_pContentControl->SetFormatContentControl(this) here; only from SetTextAttr.
}

SwFormatContentControl::~SwFormatContentControl()
{
    if (m_pContentControl
        // SwFormatContentControl is not shareable, so ptr compare is OK
        && areSfxPoolItemPtrsEqual(m_pContentControl->GetFormatContentControl(), this))
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
        // SwFormatContentControl is not shareable, so ptr compare is OK
        else if (areSfxPoolItemPtrsEqual(m_pContentControl->GetFormatContentControl(), this))
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
    if (m_pContentControl
        // SwFormatContentControl is not shareable, so ptr compare is OK
        && areSfxPoolItemPtrsEqual(m_pContentControl->GetFormatContentControl(), this))
    {
        // Not calling Modify, that would call SwXContentControl::SwClientNotify.
        m_pContentControl->NotifyChangeTextNode(pTextNode);
    }
}

SwTextNode* SwFormatContentControl::GetTextNode() const
{
    if (!m_pContentControl)
    {
        return nullptr;
    }

    return m_pContentControl->GetTextNode();
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
    if (!pFormat)
    {
        return;
    }

    const std::shared_ptr<SwContentControl>& pOther = pFormat->GetContentControl();
    if (!pOther)
    {
        return;
    }

    SetShowingPlaceHolder(pOther->m_bShowingPlaceHolder);
    SetCheckbox(pOther->m_bCheckbox);
    SetChecked(pOther->m_bChecked);
    SetCheckedState(pOther->m_aCheckedState);
    SetUncheckedState(pOther->m_aUncheckedState);
    SetListItems(pOther->m_aListItems);
    SetPicture(pOther->m_bPicture);
    SetDate(pOther->m_bDate);
    SetDateFormat(pOther->m_aDateFormat);
    SetDateLanguage(pOther->m_aDateLanguage);
    SetCurrentDate(pOther->m_aCurrentDate);
    SetPlainText(pOther->m_bPlainText);
    SetComboBox(pOther->m_bComboBox);
    SetDropDown(pOther->m_bDropDown);
    SetPlaceholderDocPart(pOther->m_aPlaceholderDocPart);
    SetDataBindingPrefixMappings(pOther->m_aDataBindingPrefixMappings);
    SetDataBindingXpath(pOther->m_aDataBindingXpath);
    SetDataBindingStoreItemID(pOther->m_aDataBindingStoreItemID);
    SetColor(pOther->m_aColor);
    SetAppearance(pOther->m_aAppearance);
    SetAlias(pOther->m_aAlias);
    SetTag(pOther->m_aTag);
    SetId(pOther->m_nId);
    SetTabIndex(pOther->m_nTabIndex);
    SetLock(pOther->m_aLock);
    SetMultiLine(pOther->m_aMultiLine);
}

SwContentControl::~SwContentControl() {}

void SwContentControl::SetXContentControl(const rtl::Reference<SwXContentControl>& xContentControl)
{
    m_wXContentControl = xContentControl.get();
}

SwTextContentControl* SwContentControl::GetTextAttr() const
{
    return m_pFormat ? m_pFormat->GetTextAttr() : nullptr;
}

void SwContentControl::NotifyChangeTextNode(SwTextNode* pTextNode)
{
    m_pTextNode = pTextNode;
    if (m_pTextNode && (GetRegisteredIn() != m_pTextNode))
    {
        m_pTextNode->Add(*this);
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
        SetXContentControl(nullptr);
        GetNotifier().Broadcast(SfxHint(SfxHintId::Deinitializing));
    }
}

std::optional<size_t> SwContentControl::GetSelectedListItem(bool bCheckDocModel) const
{
    if (!bCheckDocModel || m_oSelectedListItem)
        return m_oSelectedListItem;

    const size_t nLen = GetListItems().size();
    if (GetShowingPlaceHolder() || !nLen || !GetTextAttr())
        return std::nullopt;

    const OUString& rText = GetTextAttr()->ToString();
    for (size_t i = 0; i < nLen; ++i)
    {
        if (GetTextAttr()[i].ToString() == rText)
            return i;
    }
    assert(!GetDropDown() && "DropDowns must always have an associated list item");
    return std::nullopt;
}

bool SwContentControl::AddListItem(size_t nZIndex, const OUString& rDisplayText,
                                   const OUString& rValue)
{
    SwContentControlListItem aListItem;
    if (rValue.isEmpty())
    {
        if (rDisplayText.isEmpty())
            return false;
        aListItem.m_aValue = rDisplayText;
    }
    else
    {
        aListItem.m_aValue = rValue;
        aListItem.m_aDisplayText = rDisplayText;
    }

    // Avoid adding duplicates
    for (auto& rListItem : GetListItems())
    {
        if (rListItem == aListItem)
            return false;
    }

    const size_t nLen = GetListItems().size();
    nZIndex = std::min(nZIndex, nLen);
    const std::optional<size_t> oSelected = GetSelectedListItem();
    if (oSelected && *oSelected >= nZIndex)
    {
        if (*oSelected < nLen)
            SetSelectedListItem(*oSelected + 1);
    }
    std::vector<SwContentControlListItem> vListItems = GetListItems();
    vListItems.insert(vListItems.begin() + nZIndex, aListItem);
    SetListItems(vListItems);
    return true;
}

void SwContentControl::DeleteListItem(size_t nZIndex)
{
    if (nZIndex >= GetListItems().size())
        return;

    const std::optional<size_t> oSelected = GetSelectedListItem();
    if (oSelected)
    {
        if (*oSelected == nZIndex)
        {
            SetSelectedListItem(std::nullopt);
            if (m_bDropDown && GetTextAttr())
                GetTextAttr()->Invalidate();
        }
        else if (*oSelected < nZIndex)
            SetSelectedListItem(*oSelected - 1);
    }

    std::vector<SwContentControlListItem> vListItems = GetListItems();
    vListItems.erase(vListItems.begin() + nZIndex);
    SetListItems(vListItems);
    return;
}

void SwContentControl::ClearListItems()
{
    SetSelectedListItem(std::nullopt);
    SetListItems(std::vector<SwContentControlListItem>());
    if (m_bDropDown && GetTextAttr())
        GetTextAttr()->Invalidate();
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
    double fSelectedDate = 0;
    if (m_oSelectedDate)
    {
        fSelectedDate = *m_oSelectedDate;
    }
    else
    {
        fSelectedDate = GetCurrentDateValue();
    }

    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        return OUString();
    }

    pNumberFormatter->GetOutputString(fSelectedDate, nFormat, aFormatted, &pColor, false);
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

bool SwContentControl::IsInteractingCharacter(sal_Unicode cCh)
{
    if (GetCheckbox())
    {
        return cCh == ' ';
    }

    if (GetPicture())
    {
        return cCh == '\r';
    }

    return false;
}

bool SwContentControl::ShouldOpenPopup(const vcl::KeyCode& rKeyCode)
{
    switch (GetType())
    {
        case SwContentControlType::DROP_DOWN_LIST:
        case SwContentControlType::COMBO_BOX:
        case SwContentControlType::DATE:
        {
            // Alt-down opens the popup.
            return rKeyCode.IsMod2() && rKeyCode.GetCode() == KEY_DOWN;
        }
        default:
            break;
    }

    return false;
}

// NOTE: call SetReadWrite separately to implement true (un)locking.
// This is mostly a theoretical function; the lock state is mainly kept for round-tripping purposes.
// It is implemented here primarily for pointless VBA control, but with the intention that it
// could be made functionally useful as well for checkboxes/dropdowns/pictures.
// Returns whether the content (bControl=false) cannot be modified,
// or if the control cannot be deleted.
std::optional<bool> SwContentControl::GetLock(bool bControl) const
{
    std::optional<bool> oLock;
    if (m_aLock.isEmpty())
        return oLock;
    else if (m_aLock.equalsIgnoreAsciiCase("sdtContentLocked"))
        oLock = true;
    else if (m_aLock.equalsIgnoreAsciiCase("unlocked"))
        oLock = false;
    else if (m_aLock.equalsIgnoreAsciiCase("sdtLocked"))
        oLock = bControl;
    else if (m_aLock.equalsIgnoreAsciiCase("contentLocked"))
        oLock = !bControl;

    assert(oLock.has_value() && "invalid or unknown lock state");
    return oLock;
}

void SwContentControl::SetLock(bool bLockContent, bool bLockControl)
{
    if (!bLockContent && !bLockControl)
        m_aLock = "unlocked";
    else if (bLockContent && bLockControl)
        m_aLock = "sdtContentLocked";
    else if (bLockContent)
        m_aLock = "contentLocked";
    else
        m_aLock = "sdtLocked";
}

SwContentControlType SwContentControl::GetType() const
{
    if (m_bCheckbox)
    {
        return SwContentControlType::CHECKBOX;
    }

    if (m_bComboBox)
    {
        return SwContentControlType::COMBO_BOX;
    }

    if (m_bDropDown)
    {
        return SwContentControlType::DROP_DOWN_LIST;
    }

    if (m_bPicture)
    {
        return SwContentControlType::PICTURE;
    }

    if (m_bDate)
    {
        return SwContentControlType::DATE;
    }

    if (m_bPlainText)
    {
        return SwContentControlType::PLAIN_TEXT;
    }

    return SwContentControlType::RICH_TEXT;
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
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("plain-text"),
                                      BAD_CAST(OString::boolean(m_bPlainText).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("combo-box"),
                                      BAD_CAST(OString::boolean(m_bComboBox).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("drop-down"),
                                      BAD_CAST(OString::boolean(m_bDropDown).getStr()));
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
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("appearance"),
                                      BAD_CAST(m_aAppearance.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("alias"),
                                      BAD_CAST(m_aAlias.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("tag"), BAD_CAST(m_aTag.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"),
                                      BAD_CAST(OString::number(m_nId).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("tab-index"),
                                      BAD_CAST(OString::number(m_nTabIndex).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("lock"),
                                      BAD_CAST(m_aLock.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("multiline"),
                                      BAD_CAST(m_aMultiLine.toUtf8().getStr()));

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

const OUString& SwContentControlListItem::ToString() const
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
        pRet[i] = {
            comphelper::makePropertyValue(u"DisplayText"_ustr, rItem.m_aDisplayText),
            comphelper::makePropertyValue(u"Value"_ustr, rItem.m_aValue),
        };
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
        auto it = aMap.find(u"DisplayText"_ustr);
        if (it != aMap.end())
        {
            it->second >>= aItem.m_aDisplayText;
        }
        it = aMap.find(u"Value"_ustr);
        if (it != aMap.end())
        {
            it->second >>= aItem.m_aValue;
        }
        aRet.push_back(aItem);
    }

    return aRet;
}

SwTextContentControl*
SwTextContentControl::CreateTextContentControl(SwDoc& rDoc, SwTextNode* pTargetTextNode,
                                               const SfxPoolItemHolder& rHolder, sal_Int32 nStart,
                                               sal_Int32 nEnd, bool bIsCopy)
{
    if (bIsCopy)
    {
        // the item in rHolder is already cloned, now call DoCopy to copy the SwContentControl
        assert(pTargetTextNode
               && "SwTextContentControl ctor: cannot copy content control without target node");
        SwFormatContentControl* pSwFormatContentControl(
            static_cast<SwFormatContentControl*>(const_cast<SfxPoolItem*>(rHolder.getItem())));
        pSwFormatContentControl->DoCopy(*pTargetTextNode);
    }
    SwContentControlManager* pManager = &rDoc.GetContentControlManager();
    auto pTextContentControl(new SwTextContentControl(pManager, rHolder, nStart, nEnd));
    return pTextContentControl;
}

SwTextContentControl::SwTextContentControl(SwContentControlManager* pManager,
                                           const SfxPoolItemHolder& rAttr, sal_Int32 nStart,
                                           sal_Int32 nEnd)
    : SwTextAttr(rAttr, nStart)
    , SwTextAttrNesting(rAttr, nStart, nEnd)
    , m_pManager(pManager)
{
    SwFormatContentControl& rSwFormatContentControl(
        static_cast<SwFormatContentControl&>(GetAttr()));
    rSwFormatContentControl.SetTextAttr(this);
    SetHasDummyChar(true);
    m_pManager->Insert(this);
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

        if (pNode)
        {
            m_pManager = &pNode->GetDoc().GetContentControlManager();
        }
        else
        {
            if (m_pManager)
            {
                m_pManager->Erase(this);
            }
            m_pManager = nullptr;
        }
    }
}

void SwTextContentControl::Delete(bool bSaveContents)
{
    if (!GetTextNode())
        return;

    SwPaM aPaM(*GetTextNode(), GetStart(), *GetTextNode(), *End());
    if (bSaveContents)
        GetTextNode()->GetDoc().ResetAttrs(aPaM, /*bTextAttr=*/true, { RES_TXTATR_CONTENTCONTROL });
    else
        GetTextNode()->GetDoc().getIDocumentContentOperations().DeleteAndJoin(aPaM);
}

SwTextNode* SwTextContentControl::GetTextNode() const
{
    auto& rFormatContentControl = static_cast<const SwFormatContentControl&>(GetAttr());
    return rFormatContentControl.GetTextNode();
}

OUString SwTextContentControl::ToString() const
{
    if (!GetTextNode())
        return OUString();

    // Don't select the text attribute itself at the start.
    sal_Int32 nStart = GetStart() + 1;
    // Don't select the CH_TXTATR_BREAKWORD itself at the end.
    sal_Int32 nEnd = *End() - 1;

    SwPaM aPaM(*GetTextNode(), nStart, *GetTextNode(), nEnd);
    return aPaM.GetText();
}

void SwTextContentControl::Invalidate()
{
    SwDocShell* pDocShell = GetTextNode() ? GetTextNode()->GetDoc().GetDocShell() : nullptr;
    if (!pDocShell || !pDocShell->GetWrtShell())
        return;

    // save the cursor
    // NOTE: needs further testing to see if this is adequate (i.e. in auto-run macros...)
    pDocShell->GetWrtShell()->Push();

    // visit the control in the text (which makes any necessary visual changes)
    // NOTE: simply going to a checkbox causes a toggle, unless bOnlyRefresh
    auto& rFormatContentControl = static_cast<SwFormatContentControl&>(GetAttr());
    pDocShell->GetWrtShell()->GotoContentControl(rFormatContentControl, /*bOnlyRefresh=*/true);

    pDocShell->GetWrtShell()->Pop(SwCursorShell::PopMode::DeleteCurrent);
}

void SwTextContentControl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextContentControl"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    SwTextAttr::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SwContentControlManager::SwContentControlManager() {}

void SwContentControlManager::Insert(SwTextContentControl* pTextContentControl)
{
    m_aContentControls.push_back(pTextContentControl);
}

void SwContentControlManager::Erase(SwTextContentControl* pTextContentControl)
{
    std::erase(m_aContentControls, pTextContentControl);
}

SwTextContentControl* SwContentControlManager::Get(size_t nIndex)
{
    // Only sort now: the items may not have an associated text node by the time they are inserted
    // into the container.
    std::sort(m_aContentControls.begin(), m_aContentControls.end(),
              [](SwTextContentControl*& pLhs, SwTextContentControl*& pRhs) -> bool {
                  SwNodeOffset nIdxLHS = pLhs->GetTextNode()->GetIndex();
                  SwNodeOffset nIdxRHS = pRhs->GetTextNode()->GetIndex();
                  if (nIdxLHS == nIdxRHS)
                  {
                      return pLhs->GetStart() < pRhs->GetStart();
                  }

                  return nIdxLHS < nIdxRHS;
              });

    return m_aContentControls[nIndex];
}

SwTextContentControl* SwContentControlManager::UnsortedGet(size_t nIndex)
{
    assert(nIndex < m_aContentControls.size());
    return m_aContentControls[nIndex];
}

void SwContentControlManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwContentControlManager"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    for (const auto& pContentControl : m_aContentControls)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextContentControl"));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", pContentControl);
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
