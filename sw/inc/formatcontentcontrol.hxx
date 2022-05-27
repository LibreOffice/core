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

#pragma once

#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/weakref.hxx>
#include <sal/types.h>
#include <svl/poolitem.hxx>

#include "calbck.hxx"
#include "swdllapi.h"

class SwContentControl;
class SwTextContentControl;
class SwTextNode;
class SwXContentControl;

enum class SwContentControlType
{
    RICH_TEXT,
    CHECKBOX,
    DROP_DOWN_LIST,
    PICTURE,
    DATE,
};

/// SfxPoolItem subclass that wraps an SwContentControl.
class SW_DLLPUBLIC SwFormatContentControl final : public SfxPoolItem
{
    std::shared_ptr<SwContentControl> m_pContentControl;
    SwTextContentControl* m_pTextAttr;

public:
    SwTextContentControl* GetTextAttr() { return m_pTextAttr; }
    void SetTextAttr(SwTextContentControl* pTextAttr);

    /// This method must be called when the hint is actually copied.
    void DoCopy(SwTextNode& rTargetTextNode);

    explicit SwFormatContentControl(sal_uInt16 nWhich);

    explicit SwFormatContentControl(const std::shared_ptr<SwContentControl>& pContentControl,
                                    sal_uInt16 nWhich);
    ~SwFormatContentControl() override;

    /// SfxPoolItem
    bool operator==(const SfxPoolItem&) const override;
    SwFormatContentControl* Clone(SfxItemPool* pPool = nullptr) const override;

    /**
     * Notify clients registered at m_pContentControl that this content control is being
     * (re-)moved.
     */
    void NotifyChangeTextNode(SwTextNode* pTextNode);
    static SwFormatContentControl* CreatePoolDefault(sal_uInt16 nWhich);
    std::shared_ptr<SwContentControl> GetContentControl() { return m_pContentControl; }
    const std::shared_ptr<SwContentControl>& GetContentControl() const { return m_pContentControl; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

/// Represents one list item in a content control dropdown list.
class SW_DLLPUBLIC SwContentControlListItem
{
public:
    /// This may be empty, ToString() falls back to m_aValue.
    OUString m_aDisplayText;
    /// This must not be empty.
    OUString m_aValue;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    OUString ToString() const;

    bool operator==(const SwContentControlListItem& rOther) const;

    static void ItemsToAny(const std::vector<SwContentControlListItem>& rItems,
                           css::uno::Any& rVal);

    static std::vector<SwContentControlListItem> ItemsFromAny(const css::uno::Any& rVal);
};

/// Stores the properties of a content control.
class SW_DLLPUBLIC SwContentControl : public sw::BroadcastingModify
{
    css::uno::WeakReference<css::text::XTextContent> m_wXContentControl;

    SwFormatContentControl* m_pFormat;

    /// Can be nullptr if not in a document for undo purposes.
    SwTextNode* m_pTextNode;

    /// Current content is placeholder text.
    bool m_bShowingPlaceHolder = false;

    /// Display the content control as a checkbox.
    bool m_bCheckbox = false;

    /// If m_bCheckbox is true, is the checkbox checked?
    bool m_bChecked = false;

    /// If m_bCheckbox is true, the value of a checked checkbox.
    OUString m_aCheckedState;

    /// If m_bCheckbox is true, the value of an unchecked checkbox.
    OUString m_aUncheckedState;

    std::vector<SwContentControlListItem> m_aListItems;

    bool m_bPicture = false;

    bool m_bDate = false;

    /// If m_bDate is true, the date format in a syntax accepted by SvNumberFormatter::PutEntry().
    OUString m_aDateFormat;

    /// If m_bDate is true, the date's BCP 47 language tag.
    OUString m_aDateLanguage;

    /// Date in YYYY-MM-DDT00:00:00Z format.
    OUString m_aCurrentDate;

    /// The placeholder's doc part: just remembered.
    OUString m_aPlaceholderDocPart;

    /// The data bindings's prefix mappings: just remembered.
    OUString m_aDataBindingPrefixMappings;

    /// The data bindings's XPath: just remembered.
    OUString m_aDataBindingXpath;

    /// The data bindings's store item ID: just remembered.
    OUString m_aDataBindingStoreItemID;

    /// The color: just remembered.
    OUString m_aColor;

    /// Stores a list item index, in case the doc model is not yet updated.
    std::optional<size_t> m_oSelectedListItem;

    /// Stores a date timestamp, in case the doc model is not yet updated.
    std::optional<double> m_oSelectedDate;

public:
    SwTextContentControl* GetTextAttr() const;

    SwTextNode* GetTextNode() const { return m_pTextNode; }

    SwFormatContentControl* GetFormatContentControl() const { return m_pFormat; }

    void SetFormatContentControl(SwFormatContentControl* pFormat) { m_pFormat = pFormat; };

    void NotifyChangeTextNode(SwTextNode* pTextNode);

    const css::uno::WeakReference<css::text::XTextContent>& GetXContentControl() const
    {
        return m_wXContentControl;
    }

    void SetXContentControl(const css::uno::Reference<css::text::XTextContent>& xContentCnotrol)
    {
        m_wXContentControl = xContentCnotrol;
    }

    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

    explicit SwContentControl(SwFormatContentControl* pFormat);

    virtual ~SwContentControl() override;

    void SetShowingPlaceHolder(bool bShowingPlaceHolder)
    {
        m_bShowingPlaceHolder = bShowingPlaceHolder;
    }

    bool GetShowingPlaceHolder() const { return m_bShowingPlaceHolder; }

    void SetCheckbox(bool bCheckbox) { m_bCheckbox = bCheckbox; }

    bool GetCheckbox() const { return m_bCheckbox; }

    void SetChecked(bool bChecked) { m_bChecked = bChecked; }

    bool GetChecked() const { return m_bChecked; }

    void SetCheckedState(const OUString& rCheckedState) { m_aCheckedState = rCheckedState; }

    OUString GetCheckedState() const { return m_aCheckedState; }

    void SetUncheckedState(const OUString& rUncheckedState) { m_aUncheckedState = rUncheckedState; }

    OUString GetUncheckedState() const { return m_aUncheckedState; }

    std::vector<SwContentControlListItem> GetListItems() const { return m_aListItems; }

    bool HasListItems() const { return !m_aListItems.empty(); }

    void SetListItems(const std::vector<SwContentControlListItem>& rListItems)
    {
        m_aListItems = rListItems;
    }

    void SetPicture(bool bPicture) { m_bPicture = bPicture; }

    bool GetPicture() const { return m_bPicture; }

    void SetDate(bool bDate) { m_bDate = bDate; }

    bool GetDate() const { return m_bDate; }

    void SetDateFormat(const OUString& rDateFormat) { m_aDateFormat = rDateFormat; }

    OUString GetDateFormat() const { return m_aDateFormat; }

    void SetDateLanguage(const OUString& rDateLanguage) { m_aDateLanguage = rDateLanguage; }

    OUString GetDateLanguage() const { return m_aDateLanguage; }

    void SetCurrentDate(const OUString& rCurrentDate) { m_aCurrentDate = rCurrentDate; }

    OUString GetCurrentDate() const { return m_aCurrentDate; }

    /// Formats fCurrentDate and sets it.
    void SetCurrentDateValue(double fCurrentDate);

    /// Parses m_aCurrentDate and returns it.
    double GetCurrentDateValue() const;

    /// Formats m_oSelectedDate, taking m_aDateFormat and m_aDateLanguage into account.
    OUString GetDateString() const;

    void SetPlaceholderDocPart(const OUString& rPlaceholderDocPart)
    {
        m_aPlaceholderDocPart = rPlaceholderDocPart;
    }

    OUString GetPlaceholderDocPart() const { return m_aPlaceholderDocPart; }

    void SetSelectedListItem(std::optional<size_t> oSelectedListItem)
    {
        m_oSelectedListItem = oSelectedListItem;
    }

    std::optional<size_t> GetSelectedListItem() const { return m_oSelectedListItem; }

    void SetSelectedDate(std::optional<double> oSelectedDate) { m_oSelectedDate = oSelectedDate; }

    std::optional<double> GetSelectedDate() const { return m_oSelectedDate; }

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void SetDataBindingPrefixMappings(const OUString& rDataBindingPrefixMappings)
    {
        m_aDataBindingPrefixMappings = rDataBindingPrefixMappings;
    }

    OUString GetDataBindingPrefixMappings() const { return m_aDataBindingPrefixMappings; }

    void SetDataBindingXpath(const OUString& rDataBindingXpath)
    {
        m_aDataBindingXpath = rDataBindingXpath;
    }

    OUString GetDataBindingXpath() const { return m_aDataBindingXpath; }

    void SetDataBindingStoreItemID(const OUString& rDataBindingStoreItemID)
    {
        m_aDataBindingStoreItemID = rDataBindingStoreItemID;
    }

    OUString GetDataBindingStoreItemID() const { return m_aDataBindingStoreItemID; }

    void SetColor(const OUString& rColor) { m_aColor = rColor; }

    OUString GetColor() const { return m_aColor; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
