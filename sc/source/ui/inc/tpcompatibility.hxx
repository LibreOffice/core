/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#if 1

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

class ScTpCompatOptions : public SfxTabPage
{
public:
    using SfxTabPage::DeactivatePage;

    static  SfxTabPage* Create (Window* pParent, const SfxItemSet& rCoreAttrs);

    virtual sal_Bool FillItemSet(SfxItemSet& rCoreAttrs);
    virtual void Reset(const SfxItemSet& rCoreAttrs);
    virtual int DeactivatePage(SfxItemSet* pSet = NULL);

private:
    explicit ScTpCompatOptions(Window* pParent, const SfxItemSet& rCoreAttrs);
    virtual ~ScTpCompatOptions();

private:
    FixedLine maFlKeyBindings;
    FixedText maFtKeyBindings;
    ListBox   maLbKeyBindings;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
