/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <calcconfig.hxx>
#include "calcoptionsdlg.hxx"

namespace {

formula::FormulaGrammar::AddressConvention toAddressConvention(sal_Int32 nPos)
{
    switch (nPos)
    {
        case 1:
            return formula::FormulaGrammar::CONV_OOO;
        case 2:
            return formula::FormulaGrammar::CONV_XL_A1;
        case 3:
            return formula::FormulaGrammar::CONV_XL_R1C1;
        case 4:
            return formula::FormulaGrammar::CONV_A1_XL_A1;
        case 0:
        default:
            ;
    }

    return formula::FormulaGrammar::CONV_UNSPECIFIED;
}

sal_Int32 toSelectedItem( formula::FormulaGrammar::AddressConvention eConv )
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return 1;
        case formula::FormulaGrammar::CONV_XL_A1:
            return 2;
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return 3;
        case formula::FormulaGrammar::CONV_A1_XL_A1:
            return 4;
        default:
            ;
    }
    return 0;
}

}

ScCalcOptionsDialog::ScCalcOptionsDialog(weld::Window* pParent, const ScCalcConfig& rConfig, bool bWriteConfig)
    : GenericDialogController(pParent, "modules/scalc/ui/formulacalculationoptions.ui", "FormulaCalculationOptions")
    , maConfig(rConfig)
    , mbSelectedEmptyStringAsZero(rConfig.mbEmptyStringAsZero)
    , mbWriteConfig(bWriteConfig)
    , mxEmptyAsZero(m_xBuilder->weld_check_button("checkEmptyAsZero"))
    , mxConversion(m_xBuilder->weld_combo_box("comboConversion"))
    , mxCurrentDocOnly(m_xBuilder->weld_check_button("current_doc"))
    , mxSyntax(m_xBuilder->weld_combo_box("comboSyntaxRef"))
{
    mxConversion->set_active(static_cast<int>(rConfig.meStringConversion));
    mxConversion->connect_changed(LINK(this, ScCalcOptionsDialog, ConversionModifiedHdl));

    mxEmptyAsZero->set_active(rConfig.mbEmptyStringAsZero);
    mxEmptyAsZero->connect_toggled(LINK(this, ScCalcOptionsDialog, AsZeroModifiedHdl));
    CoupleEmptyAsZeroToStringConversion();

    mxSyntax->set_active(toSelectedItem(rConfig.meStringRefAddressSyntax));
    mxSyntax->connect_changed(LINK(this, ScCalcOptionsDialog, SyntaxModifiedHdl));

    mxCurrentDocOnly->set_active(!mbWriteConfig);
    mxCurrentDocOnly->connect_toggled(LINK(this, ScCalcOptionsDialog, CurrentDocOnlyHdl));
}

ScCalcOptionsDialog::~ScCalcOptionsDialog()
{
}

void ScCalcOptionsDialog::CoupleEmptyAsZeroToStringConversion()
{
    switch (maConfig.meStringConversion)
    {
        case ScCalcConfig::StringConversion::ILLEGAL:
            maConfig.mbEmptyStringAsZero = false;
            mxEmptyAsZero->set_active(false);
            mxEmptyAsZero->set_sensitive(false);
            break;
        case ScCalcConfig::StringConversion::ZERO:
            maConfig.mbEmptyStringAsZero = true;
            mxEmptyAsZero->set_active(true);
            mxEmptyAsZero->set_sensitive(false);
            break;
        case ScCalcConfig::StringConversion::UNAMBIGUOUS:
        case ScCalcConfig::StringConversion::LOCALE:
            // Reset to the value the user selected before.
            maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero;
            mxEmptyAsZero->set_sensitive(true);
            mxEmptyAsZero->set_active(mbSelectedEmptyStringAsZero);
            break;
    }
}

IMPL_LINK(ScCalcOptionsDialog, AsZeroModifiedHdl, weld::ToggleButton&, rCheckBox, void )
{
    maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero = rCheckBox.get_active();
}

IMPL_LINK(ScCalcOptionsDialog, ConversionModifiedHdl, weld::ComboBox&, rConv, void)
{
    maConfig.meStringConversion = static_cast<ScCalcConfig::StringConversion>(rConv.get_active());
    CoupleEmptyAsZeroToStringConversion();
}

IMPL_LINK(ScCalcOptionsDialog, SyntaxModifiedHdl, weld::ComboBox&, rSyntax, void)
{
    maConfig.SetStringRefSyntax(toAddressConvention(rSyntax.get_active()));
}

IMPL_LINK(ScCalcOptionsDialog, CurrentDocOnlyHdl, weld::ToggleButton&, rCheckBox, void)
{
    mbWriteConfig = !rCheckBox.get_active();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
