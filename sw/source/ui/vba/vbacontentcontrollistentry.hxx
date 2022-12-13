/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XContentControlListEntry.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <textcontentcontrol.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XContentControlListEntry>
    SwVbaContentControlListEntry_BASE;

class SwVbaContentControlListEntry : public SwVbaContentControlListEntry_BASE
{
private:
    std::shared_ptr<SwContentControl> m_pCC;
    // All LO and internal UNO functions are 0-based. Convert to 1-based when sending to VBA
    size_t m_nZIndex;

public:
    /// @throws css::uno::RuntimeException
    SwVbaContentControlListEntry(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                                 const css::uno::Reference<css::uno::XComponentContext>& rContext,
                                 std::shared_ptr<SwContentControl> pCC, size_t nZIndex);
    ~SwVbaContentControlListEntry() override;

    // XContentControlListEntry
    sal_Int32 SAL_CALL getIndex() override;
    void SAL_CALL setIndex(sal_Int32 nSet) override;

    OUString SAL_CALL getText() override;
    void SAL_CALL setText(const OUString& sSet) override;

    OUString SAL_CALL getValue() override;
    void SAL_CALL setValue(const OUString& sSet) override;

    void SAL_CALL Delete() override;
    void SAL_CALL MoveDown() override;
    void SAL_CALL MoveUp() override;
    void SAL_CALL Select() override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
