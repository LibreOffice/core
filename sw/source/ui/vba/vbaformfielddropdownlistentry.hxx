/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XListEntry.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XListEntry>
    SwVbaFormFieldDropDownListEntry_BASE;

class SwVbaFormFieldDropDownListEntry : public SwVbaFormFieldDropDownListEntry_BASE
{
private:
    sw::mark::DropDownFieldmark& m_rDropDown;
    // All LO and internal UNO functions are 0-based. Convert to 1-based when sending to VBA
    const sal_Int32 m_nZIndex;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFieldDropDownListEntry(
        const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
        const css::uno::Reference<css::uno::XComponentContext>& rContext,
        sw::mark::DropDownFieldmark& rFormField, sal_Int32 nZIndex);
    ~SwVbaFormFieldDropDownListEntry() override;

    // XListEntry
    sal_Int32 SAL_CALL getIndex() override;

    OUString SAL_CALL getName() override;
    void SAL_CALL setName(const OUString& sSet) override;

    void SAL_CALL Delete() override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
