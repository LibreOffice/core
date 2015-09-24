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
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

class ScTpDefaultsOptions : public SfxTabPage
{
    friend class VclPtr<ScTpDefaultsOptions>;
public:
    using SfxTabPage::DeactivatePage;

    static  VclPtr<SfxTabPage> Create (vcl::Window* pParent, const SfxItemSet* rCoreSet);

    virtual bool FillItemSet(SfxItemSet* rCoreSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rCoreSet) SAL_OVERRIDE;
    virtual sfxpg DeactivatePage(SfxItemSet* pSet = NULL) SAL_OVERRIDE;

private:
    explicit ScTpDefaultsOptions(vcl::Window* pParent, const SfxItemSet& rCoreSet);
    virtual ~ScTpDefaultsOptions();
    virtual void dispose() SAL_OVERRIDE;

    void CheckNumSheets();
    void CheckPrefix(Edit* pEdit);
    void OnFocusPrefixInput(Edit* pEdit);

    DECL_LINK(NumModifiedHdl, void *);
    DECL_LINK( PrefixModifiedHdl, Edit* );
    DECL_LINK_TYPED( PrefixEditOnFocusHdl, Control&, void );

private:
    VclPtr<NumericField> m_pEdNSheets;
    VclPtr<Edit>         m_pEdSheetPrefix;

    // Stores old Sheet Prefix
    OUString maOldPrefixValue;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
