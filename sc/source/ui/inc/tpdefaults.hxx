/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_TPDEFAULTS_HXX__
#define __SC_TPDEFAULTS_HXX__

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

class ScTpDefaultsOptions : public SfxTabPage
{
public:
    using SfxTabPage::DeactivatePage;

    static  SfxTabPage* Create (Window* pParent, const SfxItemSet& rCoreSet);

    virtual bool FillItemSet(SfxItemSet& rCoreSet);
    virtual void Reset(const SfxItemSet& rCoreSet);
    virtual int DeactivatePage(SfxItemSet* pSet = NULL);

private:
    explicit ScTpDefaultsOptions(Window* pParent, const SfxItemSet& rCoreSet);
    virtual ~ScTpDefaultsOptions();

    void CheckNumSheets();
    void CheckPrefix(Edit* pEdit);
    void OnFocusPrefixInput(Edit* pEdit);

    DECL_LINK(NumModifiedHdl, void *);
    DECL_LINK( PrefixModifiedHdl, Edit* );
    DECL_LINK( PrefixEditOnFocusHdl, Edit* );

private:
    NumericField* m_pEdNSheets;
    Edit*         m_pEdSheetPrefix;

    // Stores old Sheet Prefix
    OUString maOldPrefixValue;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
