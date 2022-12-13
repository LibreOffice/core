/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XContentControlListEntries.hpp>
#include <ooo/vba/word/XContentControlListEntry.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

#include <textcontentcontrol.hxx>

#include "vbacontentcontrollistentries.hxx"
#include "vbacontentcontrollistentry.hxx"

typedef CollTestImplHelper<ooo::vba::word::XContentControlListEntries>
    SwVbaContentControlListEntries_BASE;

class SwVbaContentControlListEntries : public SwVbaContentControlListEntries_BASE
{
private:
    std::shared_ptr<SwContentControl> m_pCC;

public:
    /// @throws css::uno::RuntimeException
    SwVbaContentControlListEntries(const css::uno::Reference<ov::XHelperInterface>& xParent,
                                   const css::uno::Reference<css::uno::XComponentContext>& xContext,
                                   std::shared_ptr<SwContentControl> pCC);

    // XContentControlListEntries
    css::uno::Reference<ooo::vba::word::XContentControlListEntry> SAL_CALL
    Add(const OUString& rName, const css::uno::Any& rValue, const css::uno::Any& rIndex) override;
    void SAL_CALL Clear() override;
    sal_Int32 SAL_CALL getCount() override;

    // XEnumerationAccess
    css::uno::Type SAL_CALL getElementType() override;
    css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override;

    // SwVbaContentControlListEntries_BASE
    css::uno::Any createCollectionObject(const css::uno::Any& aSource) override;
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
