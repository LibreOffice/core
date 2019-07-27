/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPDEFAULTS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPDEFAULTS_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/field.hxx>

class ScTpDefaultsOptions : public SfxTabPage
{
    friend class VclPtr<ScTpDefaultsOptions>;
public:
    using SfxTabPage::DeactivatePage;

    static  VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rCoreSet);

    virtual bool FillItemSet(SfxItemSet* rCoreSet) override;
    virtual void Reset(const SfxItemSet* rCoreSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;

private:
    explicit ScTpDefaultsOptions(TabPageParent pParent, const SfxItemSet& rCoreSet);
    virtual ~ScTpDefaultsOptions() override;

    void CheckNumSheets();
    void CheckPrefix();
    void OnFocusPrefixInput();

    DECL_LINK( NumModifiedHdl, weld::Entry&, void );
    DECL_LINK( PrefixModifiedHdl, weld::Entry&, void );
    DECL_LINK( PrefixEditOnFocusHdl, weld::Widget&, void );

private:
    // Stores old Sheet Prefix
    OUString maOldPrefixValue;

    std::unique_ptr<weld::SpinButton> m_xEdNSheets;
    std::unique_ptr<weld::Entry> m_xEdSheetPrefix;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
