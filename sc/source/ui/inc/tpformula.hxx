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

#pragma once

#include <sfx2/tabdlg.hxx>

#include <calcconfig.hxx>
#include <docoptio.hxx>

class ScTpFormulaOptions : public SfxTabPage
{
public:
    explicit ScTpFormulaOptions(weld::Container* pPage, weld::DialogController* pController,
                                const SfxItemSet& rCoreSet);
    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet);
    virtual ~ScTpFormulaOptions() override;

    virtual bool FillItemSet(SfxItemSet* rCoreSet) override;
    virtual void Reset(const SfxItemSet* rCoreSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;

private:
    void ResetSeparators();
    void OnFocusSeparatorInput(weld::Entry* pEdit);
    void UpdateCustomCalcRadioButtons(bool bDefault);
    void LaunchCustomCalcSettings();

    bool IsValidSeparator(const OUString& rSep) const;

    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(SepInsertTextHdl, OUString&, bool);
    DECL_LINK(ColSepInsertTextHdl, OUString&, bool);
    DECL_LINK(RowSepInsertTextHdl, OUString&, bool);
    DECL_LINK(SepModifyHdl, weld::Entry&, void);
    DECL_LINK(SepEditOnFocusHdl, weld::Widget&, void);

private:
    /** Stores old separator value of currently focused separator edit box.
        This value is used to revert undesired value change. */
    OUString maOldSepValue;

    ScCalcConfig maSavedConfig;
    ScCalcConfig maCurrentConfig;

    ScDocOptions maSavedDocOptions;
    ScDocOptions maCurrentDocOptions;

    sal_Unicode mnDecSep;

    std::unique_ptr<weld::ComboBox> mxLbFormulaSyntax;
    std::unique_ptr<weld::CheckButton> mxCbEnglishFuncName;

    std::unique_ptr<weld::RadioButton> mxBtnCustomCalcDefault;
    std::unique_ptr<weld::RadioButton> mxBtnCustomCalcCustom;
    std::unique_ptr<weld::Button> mxBtnCustomCalcDetails;

    std::unique_ptr<weld::Entry> mxEdSepFuncArg;
    std::unique_ptr<weld::Entry> mxEdSepArrayCol;
    std::unique_ptr<weld::Entry> mxEdSepArrayRow;
    std::unique_ptr<weld::Button> mxBtnSepReset;

    std::unique_ptr<weld::ComboBox> mxLbOOXMLRecalcOptions;
    std::unique_ptr<weld::ComboBox> mxLbODFRecalcOptions;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
