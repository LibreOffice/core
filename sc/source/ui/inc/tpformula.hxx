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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPFORMULA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPFORMULA_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

#include <calcconfig.hxx>
#include <docoptio.hxx>

class Edit;

class ScTpFormulaOptions : public SfxTabPage
{
    friend class VclPtr<ScTpFormulaOptions>;
public:
    using SfxTabPage::DeactivatePage;

    static  VclPtr<SfxTabPage> Create (TabPageParent pParent, const SfxItemSet* rCoreSet);

    virtual bool FillItemSet(SfxItemSet* rCoreSet) override;
    virtual void Reset( const SfxItemSet* rCoreSet ) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet ) override;

private:
    explicit ScTpFormulaOptions(vcl::Window* pParent, const SfxItemSet& rCoreSet);
    virtual ~ScTpFormulaOptions() override;
    virtual void dispose() override;
    void ResetSeparators();
    void OnFocusSeparatorInput(Edit* pEdit);
    void UpdateCustomCalcRadioButtons(bool bDefault);
    void LaunchCustomCalcSettings();

    bool IsValidSeparator(const OUString& rSep) const;
    bool IsValidSeparatorSet() const;

    DECL_LINK( ButtonHdl, Button*, void );
    DECL_LINK( SepModifyHdl, Edit&, void );
    DECL_LINK( SepEditOnFocusHdl, Control&, void );

private:
    VclPtr<ListBox>     mpLbFormulaSyntax;
    VclPtr<CheckBox>    mpCbEnglishFuncName;

    VclPtr<RadioButton> mpBtnCustomCalcDefault;
    VclPtr<RadioButton> mpBtnCustomCalcCustom;
    VclPtr<PushButton>  mpBtnCustomCalcDetails;

    VclPtr<Edit>        mpEdSepFuncArg;
    VclPtr<Edit>        mpEdSepArrayCol;
    VclPtr<Edit>        mpEdSepArrayRow;
    VclPtr<PushButton>  mpBtnSepReset;

    VclPtr<ListBox>     mpLbOOXMLRecalcOptions;
    VclPtr<ListBox>     mpLbODFRecalcOptions;

    /** Stores old separator value of currently focused separator edit box.
        This value is used to revert undesired value change. */
    OUString maOldSepValue;

    ScCalcConfig maSavedConfig;
    ScCalcConfig maCurrentConfig;

    ScDocOptions maSavedDocOptions;
    ScDocOptions maCurrentDocOptions;

    sal_Unicode mnDecSep;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
