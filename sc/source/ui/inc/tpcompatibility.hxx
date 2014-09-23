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
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

class ScTpCompatOptions : public SfxTabPage
{
public:
    using SfxTabPage::DeactivatePage;

    static  SfxTabPage* Create (vcl::Window* pParent, const SfxItemSet* rCoreAttrs);

    virtual bool FillItemSet(SfxItemSet* rCoreAttrs) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rCoreAttrs) SAL_OVERRIDE;
    virtual int DeactivatePage(SfxItemSet* pSet = NULL) SAL_OVERRIDE;

private:
    explicit ScTpCompatOptions(vcl::Window* pParent, const SfxItemSet& rCoreAttrs);
    virtual ~ScTpCompatOptions();

private:
    ListBox*   m_pLbKeyBindings;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
