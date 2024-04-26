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
#include <unotools/weakref.hxx>

#include "calbck.hxx"
#include "swdllapi.h"

namespace vcl
{
class KeyCode;
}
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
    PLAIN_TEXT,
    COMBO_BOX,
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
    SwTextNode* GetTextNode() const;
    static SwFormatContentControl* CreatePoolDefault(sal_uInt16 nWhich);
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

    const OUString& ToString() const;

    bool operator==(const SwContentControlListItem& rOther) const;

    static void ItemsToAny(const std::vector<SwContentControlListItem>& rItems,
                           css::uno::Any& rVal);

    static std::vector<SwContentControlListItem> ItemsFromAny(const css::uno::Any& rVal);
};

/// Stores the properties of a content control.
class SW_DLLPUBLIC SwContentControl final : public sw::BroadcastingModify
{
    unotools::WeakReference<SwXContentControl> m_wXContentControl;

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

    /// Plain text, i.e. not rich text.
    bool m_bPlainText = false;

    /// Same as drop-down, but free-form input is also accepted.
    bool m_bComboBox = false;

    /// Same as combo box, but free-form input is not accepted.
    bool m_bDropDown = false;

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

    /// The appearance: just remembered.
    OUString m_aAppearance;

    /// The alias.
    OUString m_aAlias;

    /// The tag: just remembered.
    OUString m_aTag;

    /// The id: just remembered.
    sal_Int32 m_nId = 0;

    /// The tabIndex: just remembered.
    sal_uInt32 m_nTabIndex = 0;

    /// The control and content locks: mostly just remembered.
    OUString m_aLock;

    /// The multiline property: just remembered.
    OUString m_aMultiLine;

    /// Stores a list item index, in case the doc model is not yet updated.
    // i.e. temporarily store the selected item until the text is inserted by GotoContentControl.
    std::optional<size_t> m_oSelectedListItem;

    /// Stores a date timestamp, in case the doc model is not yet updated.
    // i.e. temporarily store the date until the text is inserted by GotoContentControl.
    std::optional<double> m_oSelectedDate;

    /**
     * E.g. checkbox is read-only by default, but we still update contents on interaction
     * internally. This flag is true for the duration of that interaction.
     */
    bool m_bReadWrite = false;

public:
    SwTextContentControl* GetTextAttr() const;

    SwTextNode* GetTextNode() const { return m_pTextNode; }

    SwFormatContentControl* GetFormatContentControl() const { return m_pFormat; }

    void SetFormatContentControl(SwFormatContentControl* pFormat) { m_pFormat = pFormat; };

    void NotifyChangeTextNode(SwTextNode* pTextNode);

    const unotools::WeakReference<SwXContentControl>& GetXContentControl() const
    {
        return m_wXContentControl;
    }

    void SetXContentControl(const rtl::Reference<SwXContentControl>& xContentControl);

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

    const OUString& GetCheckedState() const { return m_aCheckedState; }

    void SetUncheckedState(const OUString& rUncheckedState) { m_aUncheckedState = rUncheckedState; }

    const OUString& GetUncheckedState() const { return m_aUncheckedState; }

    const std::vector<SwContentControlListItem>& GetListItems() const { return m_aListItems; }

    void SetListItems(const std::vector<SwContentControlListItem>& rListItems)
    {
        m_aListItems = rListItems;
    }

    bool AddListItem(size_t nZIndex, const OUString& rDisplayText, const OUString& rValue);
    void DeleteListItem(size_t nZIndex);
    void ClearListItems();

    void SetPicture(bool bPicture) { m_bPicture = bPicture; }

    bool GetPicture() const { return m_bPicture; }

    void SetDate(bool bDate) { m_bDate = bDate; }

    bool GetDate() const { return m_bDate; }

    void SetDateFormat(const OUString& rDateFormat) { m_aDateFormat = rDateFormat; }

    const OUString& GetDateFormat() const { return m_aDateFormat; }

    void SetDateLanguage(const OUString& rDateLanguage) { m_aDateLanguage = rDateLanguage; }

    const OUString& GetDateLanguage() const { return m_aDateLanguage; }

    void SetCurrentDate(const OUString& rCurrentDate) { m_aCurrentDate = rCurrentDate; }

    const OUString& GetCurrentDate() const { return m_aCurrentDate; }

    /// Formats fCurrentDate and sets it.
    void SetCurrentDateValue(double fCurrentDate);

    /// Parses m_aCurrentDate and returns it.
    double GetCurrentDateValue() const;

    /// Formats m_oSelectedDate, taking m_aDateFormat and m_aDateLanguage into account.
    OUString GetDateString() const;

    void SetPlainText(bool bPlainText) { m_bPlainText = bPlainText; }

    bool GetPlainText() const { return m_bPlainText; }

    void SetComboBox(bool bComboBox) { m_bComboBox = bComboBox; }

    bool GetComboBox() const { return m_bComboBox; }

    void SetDropDown(bool bDropDown) { m_bDropDown = bDropDown; }

    bool GetDropDown() const { return m_bDropDown; }

    void SetPlaceholderDocPart(const OUString& rPlaceholderDocPart)
    {
        m_aPlaceholderDocPart = rPlaceholderDocPart;
    }

    const OUString& GetPlaceholderDocPart() const { return m_aPlaceholderDocPart; }

    void SetSelectedListItem(std::optional<size_t> oSelectedListItem)
    {
        m_oSelectedListItem = oSelectedListItem;
    }

    const std::optional<size_t>& GetSelectedListItem() const { return m_oSelectedListItem; }

    /// Get a copy of selected list item's index,
    /// potentially even if the selection is already written out to text (i.e. validated).
    std::optional<size_t> GetSelectedListItem(bool bCheckDocModel) const;

    void SetSelectedDate(std::optional<double> oSelectedDate) { m_oSelectedDate = oSelectedDate; }

    const std::optional<double>& GetSelectedDate() const { return m_oSelectedDate; }

    /// Should this character (during key input) interact with the content control?
    bool IsInteractingCharacter(sal_Unicode cCh);

    /// Given rKeyCode as a keyboard event, should a popup be opened for this content control?
    bool ShouldOpenPopup(const vcl::KeyCode& rKeyCode);

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void SetDataBindingPrefixMappings(const OUString& rDataBindingPrefixMappings)
    {
        m_aDataBindingPrefixMappings = rDataBindingPrefixMappings;
    }

    const OUString& GetDataBindingPrefixMappings() const { return m_aDataBindingPrefixMappings; }

    void SetDataBindingXpath(const OUString& rDataBindingXpath)
    {
        m_aDataBindingXpath = rDataBindingXpath;
    }

    const OUString& GetDataBindingXpath() const { return m_aDataBindingXpath; }

    void SetDataBindingStoreItemID(const OUString& rDataBindingStoreItemID)
    {
        m_aDataBindingStoreItemID = rDataBindingStoreItemID;
    }

    const OUString& GetDataBindingStoreItemID() const { return m_aDataBindingStoreItemID; }

    void SetColor(const OUString& rColor) { m_aColor = rColor; }

    const OUString& GetColor() const { return m_aColor; }

    void SetAppearance(const OUString& rAppearance) { m_aAppearance = rAppearance; }

    const OUString& GetAppearance() const { return m_aAppearance; }

    void SetAlias(const OUString& rAlias) { m_aAlias = rAlias; }

    const OUString& GetAlias() const { return m_aAlias; }

    void SetTag(const OUString& rTag) { m_aTag = rTag; }

    const OUString& GetTag() const { return m_aTag; }

    void SetId(sal_Int32 nId) { m_nId = nId; }

    sal_Int32 GetId() const { return m_nId; }

    void SetTabIndex(sal_uInt32 nTabIndex) { m_nTabIndex = nTabIndex; }

    sal_uInt32 GetTabIndex() const { return m_nTabIndex; }

    // At the design level, define how the control should be locked. No effect at implementation lvl
    void SetLock(bool bLockContent, bool bLockControl);
    void SetLock(const OUString& rLock) { m_aLock = rLock; }

    // At the design level, get how the control is locked. Does not reflect actual implementation.
    std::optional<bool> GetLock(bool bControl) const;
    const OUString& GetLock() const { return m_aLock; }

    void SetReadWrite(bool bReadWrite) { m_bReadWrite = bReadWrite; }

    // At the implementation level, define whether the user can directly modify the contents.
    bool GetReadWrite() const { return m_bReadWrite; }

    void SetMultiLine(const OUString& rMultiline) { m_aMultiLine = rMultiline; }

    const OUString& GetMultiLine() const { return m_aMultiLine; }

    SwContentControlType GetType() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
