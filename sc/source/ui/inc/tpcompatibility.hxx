/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPCOMPATIBILITY_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPCOMPATIBILITY_HXX

#include <sfx2/tabdlg.hxx>

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
private:
    explicit ScTpCompatOptions(TabPageParent pParent, const SfxItemSet& rCoreAttrs);
private:
    std::unique_ptr<weld::ComboBox> m_xLbKeyBindings;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
