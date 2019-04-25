/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/tabdlg.hxx>
#include <vcl/lstbox.hxx>

class ScTpCompatOptions : public SfxTabPage
{
    friend class VclPtr<ScTpCompatOptions>;
public:
    using SfxTabPage::DeactivatePage;

    static  VclPtr<SfxTabPage> Create (TabPageParent pParent, const SfxItemSet* rCoreAttrs);

    virtual bool FillItemSet(SfxItemSet* rCoreAttrs) override;
    virtual void Reset(const SfxItemSet* rCoreAttrs) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet ) override;

    virtual ~ScTpCompatOptions() override;
    virtual void dispose() override;
private:
    explicit ScTpCompatOptions(vcl::Window* pParent, const SfxItemSet& rCoreAttrs);
private:
    VclPtr<ListBox>   m_pLbKeyBindings;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
