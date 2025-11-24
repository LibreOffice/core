/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unotools/configitem.hxx>

#include <com/sun/star/uno/Sequence.h>

class SwMasterUsrPref;

/// Configuration item that manages baseline grid related options (e.g. visibility of the grid).
class SwBaselineGridConfig final : public utl::ConfigItem
{
private:
    SwMasterUsrPref& m_rParent;

    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwBaselineGridConfig(SwMasterUsrPref& rParent);

    virtual void Notify(const css::uno::Sequence<OUString>& rPropertyNames) override;
    void Load();
    using ConfigItem::SetModified;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
