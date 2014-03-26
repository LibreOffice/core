/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_TPFORMULA_HXX
#define SC_TPFORMULA_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>

#include "calcconfig.hxx"

class ScTpFormulaOptions : public SfxTabPage
{
public:
    using SfxTabPage::DeactivatePage;

    static  SfxTabPage* Create (Window* pParent, const SfxItemSet& rCoreSet);

    virtual bool FillItemSet(SfxItemSet& rCoreSet);
    virtual void Reset( const SfxItemSet& rCoreSet );
    virtual int DeactivatePage(SfxItemSet* pSet = NULL);

private:
    explicit ScTpFormulaOptions(Window* pParent, const SfxItemSet& rCoreSet);
    virtual ~ScTpFormulaOptions();

    void ResetSeparators();
    void OnFocusSeparatorInput(Edit* pEdit);
    void UpdateCustomCalcRadioButtons(bool bDefault);
    void LaunchCustomCalcSettings();

    bool IsValidSeparator(const OUString& rSep) const;
    bool IsValidSeparatorSet() const;

    DECL_LINK( ButtonHdl, Button* );
    DECL_LINK( SepModifyHdl, Edit* );
    DECL_LINK( SepEditOnFocusHdl, Edit* );

private:
    ListBox*     mpLbFormulaSyntax;
    CheckBox*    mpCbEnglishFuncName;

    RadioButton* mpBtnCustomCalcDefault;
    RadioButton* mpBtnCustomCalcCustom;
    PushButton*  mpBtnCustomCalcDetails;

    Edit*        mpEdSepFuncArg;
    Edit*        mpEdSepArrayCol;
    Edit*        mpEdSepArrayRow;
    PushButton*  mpBtnSepReset;

    ListBox*     mpLbOOXMLRecalcOptions;
    ListBox*     mpLbODFRecalcOptions;

    /** Stores old separator value of currently focused separator edit box.
        This value is used to revert undesired value change. */
    OUString maOldSepValue;

    ScCalcConfig maSavedConfig;
    ScCalcConfig maCurrentConfig;

    sal_Unicode mnDecSep;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
