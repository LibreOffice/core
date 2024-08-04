/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XListEntries.hpp>
#include <ooo/vba/word/XListEntry.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

#include "vbaformfielddropdownlistentries.hxx"
#include "vbaformfielddropdownlistentry.hxx"

typedef CollTestImplHelper<ooo::vba::word::XListEntries> SwVbaFormFieldDropDownListEntries_BASE;

class SwVbaFormFieldDropDownListEntries : public SwVbaFormFieldDropDownListEntries_BASE
{
private:
    sw::mark::DropDownFieldmark& m_rDropDown;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFieldDropDownListEntries(
        const css::uno::Reference<ov::XHelperInterface>& xParent,
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        sw::mark::DropDownFieldmark& m_rDropDown);

    // XListEntries
    css::uno::Reference<ooo::vba::word::XListEntry>
        SAL_CALL Add(const OUString& rName, const css::uno::Any& rIndex) override;
    void SAL_CALL Clear() override;
    sal_Int32 SAL_CALL getCount() override;

    // XEnumerationAccess
    css::uno::Type SAL_CALL getElementType() override;
    css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override;

    // SwVbaFormFieldDropDownListEntries_BASE
    css::uno::Any createCollectionObject(const css::uno::Any& aSource) override;
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
