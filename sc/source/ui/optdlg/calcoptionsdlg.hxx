/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_OPTDLG_CALCOPTIONSDLG_HXX__
#define __SC_OPTDLG_CALCOPTIONSDLG_HXX__

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "svx/checklbx.hxx"

#include "calcconfig.hxx"

class ScCalcOptionsDialog : public ModalDialog
{
public:
    ScCalcOptionsDialog(Window* pParent, const ScCalcConfig& rConfig);
    virtual ~ScCalcOptionsDialog();

    DECL_LINK( SettingsSelHdl, Control* );
    DECL_LINK( BtnToggleHdl, void* );

    const ScCalcConfig& GetConfig() const;

private:
    void FillOptionsList();
    void SelectionChanged();
    void ListOptionValueChanged();
    void RadioValueChanged();

    OUString toString(formula::FormulaGrammar::AddressConvention eConv) const;
    OUString toString(bool bVal) const;
    SvTreeListEntry *createBoolItem(const OUString &rCaption, bool bValue) const;
    void     setValueAt(size_t nPos, const OUString &rString);

private:
    SvxCheckListBox* mpLbSettings;

    ListBox* mpLbOptionEdit;
    RadioButton* mpBtnTrue;
    RadioButton* mpBtnFalse;

    FixedText* mpFtAnnotation;

    OUString maTrue;
    OUString maFalse;

    OUString maCalcA1;
    OUString maExcelA1;
    OUString maExcelR1C1;

    OUString maCaptionStringRefSyntax;
    OUString maDescStringRefSyntax;
    OUString maUseFormulaSyntax;

    OUString maCaptionEmptyStringAsZero;
    OUString maDescEmptyStringAsZero;

    OUString maCaptionOpenCLEnabled;
    OUString maDescOpenCLEnabled;

    ScCalcConfig maConfig;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
