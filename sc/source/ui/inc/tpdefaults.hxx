/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Albert Thuswaldner <albert.thuswaldner@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

    virtual sal_Bool FillItemSet(SfxItemSet& rCoreSet);
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
    FixedLine     aFLInitSpreadSheet;
    FixedText     aFtNSheets;
    NumericField  aEdNSheets;
    FixedText     aFtSheetPrefix;
    Edit          aEdSheetPrefix;

    // Stores old Sheet Prefix
    OUString maOldPrefixValue;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
