/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XContentControl.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <textcontentcontrol.hxx>
#include <rtl/ref.hxx>

class SwXTextDocument;

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XContentControl> SwVbaContentControl_BASE;

class SwVbaContentControl : public SwVbaContentControl_BASE
{
private:
    rtl::Reference<SwXTextDocument> mxTextDocument;
    std::shared_ptr<SwContentControl> m_pCC;

public:
    /// @throws css::uno::RuntimeException
    SwVbaContentControl(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                        const css::uno::Reference<css::uno::XComponentContext>& rContext,
                        const rtl::Reference<SwXTextDocument>& xTextDocument,
                        std::shared_ptr<SwContentControl> pContentControl);
    ~SwVbaContentControl() override;

    // XContentControl Properties
    sal_Bool SAL_CALL getAllowInsertDeleteSection() override;
    void SAL_CALL setAllowInsertDeleteSection(sal_Bool bSet) override;

    sal_Int32 SAL_CALL getAppearance() override;
    void SAL_CALL setAppearance(sal_Int32 nSet) override;

    OUString SAL_CALL getBuildingBlockCategory() override;
    void SAL_CALL setBuildingBlockCategory(const OUString& sSet) override;

    sal_Int32 SAL_CALL getBuildingBlockType() override;
    void SAL_CALL setBuildingBlockType(sal_Int32 nSet) override;

    sal_Bool SAL_CALL getChecked() override;
    void SAL_CALL setChecked(sal_Bool bSet) override;

    // returns or sets a WdColor (@since after 2010 I assume)
    sal_Int32 SAL_CALL getColor() override;
    void SAL_CALL setColor(sal_Int32 nSet) override;

    sal_Int32 SAL_CALL getDateCalendarType() override;
    void SAL_CALL setDateCalendarType(sal_Int32 nSet) override;

    OUString SAL_CALL getDateDisplayFormat() override;
    void SAL_CALL setDateDisplayFormat(const OUString& sSet) override;

    sal_Int32 SAL_CALL getDateDisplayLocale() override;

    sal_Int32 SAL_CALL getDateStorageFormat() override;
    void SAL_CALL setDateStorageFormat(sal_Int32 nSet) override;

    css::uno::Any SAL_CALL getDropdownListEntries() override;

    // This is an integer used as a unique identifier string
    OUString SAL_CALL getID() override;

    sal_Int32 SAL_CALL getLevel() override;

    // returns or sets if the user can delete the control
    sal_Bool SAL_CALL getLockContentControl() override;
    void SAL_CALL setLockContentControl(sal_Bool bSet) override;

    // returns or sets if the user can edit the contents (i.e. read-only flag)
    sal_Bool SAL_CALL getLockContents() override;
    void SAL_CALL setLockContents(sal_Bool bSet) override;

    sal_Bool SAL_CALL getMultiLine() override;
    void SAL_CALL setMultiLine(sal_Bool bSet) override;

    // WRONG- THIS SHOULD RETURN XBUILDINGBLOCK
    OUString SAL_CALL getPlaceholderText() override;

    sal_Bool SAL_CALL getShowingPlaceholderText() override;

    OUString SAL_CALL getRepeatingSectionItemTitle() override;
    void SAL_CALL setRepeatingSectionItemTitle(const OUString& rSet) override;

    css::uno::Reference<ooo::vba::word::XRange> SAL_CALL getRange() override;

    OUString SAL_CALL getTag() override;
    void SAL_CALL setTag(const OUString& rSet) override;

    // returns or sets if the control is removed after accepting user change (i.e. control -> text)
    sal_Bool SAL_CALL getTemporary() override;
    void SAL_CALL setTemporary(sal_Bool bSet) override;

    OUString SAL_CALL getTitle() override;
    void SAL_CALL setTitle(const OUString& rSet) override;

    // returns or sets a WdContentControlType that represents the type for a content control.
    sal_Int32 SAL_CALL getType() override;
    void SAL_CALL setType(sal_Int32 nSet) override;

    // XContentControl Methods

    // Copies the content control from the active document to the Clipboard.
    // Retrieve from the clipboard using the Paste method of the Selection object
    // or of the Range object, or use the Paste function from within Microsoft Word.
    void SAL_CALL Copy() override;

    // Removes the control from the active document and moves it to the Clipboard.
    void SAL_CALL Cut() override;

    // Specifies whether to delete the contents of the content control. The default value is False.
    // True removes both the content control and its contents.
    // False removes the control but leaves the contents of the content control in the document.
    void SAL_CALL Delete(const css::uno::Any& bDeleteContents) override;

    // Set the Unicode character used to display the checked state.
    void SAL_CALL SetCheckedSymbol(sal_Int32 Character, const css::uno::Any& sFont) override;

    // Set the Unicode character used to display the unchecked state.
    void SAL_CALL SetUnCheckedSymbol(sal_Int32 Character, const css::uno::Any& sFont) override;

    // Sets the placeholder text that displays until a user enters their own text.
    // Only one of the parameters is used when specifying placeholder text.
    // If more than one parameter is provided, use the text specified in the first parameter.
    // If all parameters are omitted, the placeholder text is blank.
    void SAL_CALL SetPlaceholderText(const css::uno::Any& BuildingBlock, const css::uno::Any& Range,
                                     const css::uno::Any& sText) override;

    void SAL_CALL Ungroup() override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
