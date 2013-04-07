/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

private:
    SvxCheckListBox maLbSettings;

    FixedText maFtOptionEditCaption;
    ListBox maLbOptionEdit;
    RadioButton maBtnTrue;
    RadioButton maBtnFalse;

    FixedLine maFlAnnotation;
    FixedText maFtAnnotation;

    OKButton maBtnOK;
    CancelButton maBtnCancel;

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

    ScCalcConfig maConfig;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
