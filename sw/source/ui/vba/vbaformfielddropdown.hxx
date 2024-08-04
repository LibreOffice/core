/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XDropDown.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XDropDown> SwVbaFormFieldDropDown_BASE;

class SwVbaFormFieldDropDown : public SwVbaFormFieldDropDown_BASE
{
private:
    sw::mark::DropDownFieldmark* m_pDropDown;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFieldDropDown(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                           const css::uno::Reference<css::uno::XComponentContext>& rContext,
                           sw::mark::Fieldmark& rFormField);
    ~SwVbaFormFieldDropDown() override;

    // XDropDown
    OUString SAL_CALL getDefaultPropertyName() override;

    // Default member: True if the specified form field object is a valid listbox field
    sal_Bool SAL_CALL getValid() override;

    // Returns and sets the index for the default listbox entry
    sal_Int32 SAL_CALL getDefault() override;
    void SAL_CALL setDefault(sal_Int32 nSet) override;
    // Returns and sets the index of the selected listbox entry
    sal_Int32 SAL_CALL getValue() override;
    void SAL_CALL setValue(sal_Int32 nIndex) override;

    // Returns a ListEntries collection that represents all the available entries
    css::uno::Any SAL_CALL ListEntries(const css::uno::Any& rIndex) override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
