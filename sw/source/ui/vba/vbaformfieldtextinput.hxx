/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XTextInput.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XTextInput> SwVbaFormFieldTextInput_BASE;

class SwVbaFormFieldTextInput : public SwVbaFormFieldTextInput_BASE
{
private:
    sw::mark::Fieldmark& m_rTextInput;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFieldTextInput(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                            const css::uno::Reference<css::uno::XComponentContext>& rContext,
                            sw::mark::Fieldmark& rFormField);
    ~SwVbaFormFieldTextInput() override;

    // XTextInput
    OUString SAL_CALL getDefaultPropertyName() override;

    // default member: True if the specified form field object is a valid text form field
    sal_Bool SAL_CALL getValid() override;

    // Returns and sets the default text string of the input box
    OUString SAL_CALL getDefault() override;
    void SAL_CALL setDefault(const OUString& bSet) override;
    // Returns the format string for the current text
    OUString SAL_CALL getFormat() override;
    /*
     * Returns the type of text form field.
     * Possible return values are:
     * wdCalculationText - Calculation text field,
     * wdCurrentDateText - Current date text field,
     * wdCurrentTimeText - Current time text field,
     * wdDateText - Date text field,
     * wdNumberText - Number text field,
     * wdRegularText - Regular text field.
     */
    sal_Int32 SAL_CALL getType() override;
    // Returns and sets the width, in points
    sal_Int32 SAL_CALL getWidth() override;
    void SAL_CALL setWidth(sal_Int32 nSet) override;

    // Deletes the text from the text form field.
    void SAL_CALL Clear() override;
    // Sets the type, default text string, format string, and enabled status
    void SAL_CALL EditType(sal_Int32 nType, const css::uno::Any& rDefault,
                           const css::uno::Any& rFormat, const css::uno::Any& rEnabled) override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
