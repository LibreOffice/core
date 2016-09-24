/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cmath>

#include "calcconfig.hxx"
#include "calcoptionsdlg.hxx"
#include "docfunc.hxx"
#include "docsh.hxx"
#include "interpre.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "scopetools.hxx"
#include "viewdata.hxx"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <comphelper/random.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>

#if HAVE_FEATURE_OPENCL
#include "formulagroup.hxx"
#include "globalnames.hxx"
#endif

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

ScCalcOptionsDialog::ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig, bool bWriteConfig)
    : ModalDialog(pParent, "FormulaCalculationOptions",
        "modules/scalc/ui/formulacalculationoptions.ui")
    , maConfig(rConfig)
    , mbSelectedEmptyStringAsZero(rConfig.mbEmptyStringAsZero)
    , mbWriteConfig(bWriteConfig)
{
    get(mpConversion,"comboConversion");
    mpConversion->SelectEntryPos(static_cast<sal_Int32>(rConfig.meStringConversion));
    mpConversion->SetSelectHdl(LINK(this, ScCalcOptionsDialog, ConversionModifiedHdl));

    get(mpEmptyAsZero,"checkEmptyAsZero");
    mpEmptyAsZero->Check(rConfig.mbEmptyStringAsZero);
    mpEmptyAsZero->SetClickHdl(LINK(this, ScCalcOptionsDialog, AsZeroModifiedHdl));
    CoupleEmptyAsZeroToStringConversion();

    get(mpSyntax,"comboSyntaxRef");
    mpSyntax->SelectEntryPos( toSelectedItem(rConfig.meStringRefAddressSyntax) );
    mpSyntax->SetSelectHdl(LINK(this, ScCalcOptionsDialog, SyntaxModifiedHdl));

    get(mpCurrentDocOnly,"current_doc");
    mpCurrentDocOnly->Check(!mbWriteConfig);
    mpCurrentDocOnly->SetClickHdl(LINK(this, ScCalcOptionsDialog, CurrentDocOnlyHdl));
}

ScCalcOptionsDialog::~ScCalcOptionsDialog()
{
    disposeOnce();
}

void ScCalcOptionsDialog::dispose()
{
    mpEmptyAsZero.clear();
    mpConversion.clear();
    mpSyntax.clear();
    mpCurrentDocOnly.clear();
    ModalDialog::dispose();
}

void ScCalcOptionsDialog::CoupleEmptyAsZeroToStringConversion()
{
    switch (maConfig.meStringConversion)
    {
        case ScCalcConfig::StringConversion::ILLEGAL:
            maConfig.mbEmptyStringAsZero = false;
            mpEmptyAsZero->Check(false);
            mpEmptyAsZero->Enable(false);
            break;
        case ScCalcConfig::StringConversion::ZERO:
            maConfig.mbEmptyStringAsZero = true;
            mpEmptyAsZero->Check();
            mpEmptyAsZero->Enable(false);
            break;
        case ScCalcConfig::StringConversion::UNAMBIGUOUS:
        case ScCalcConfig::StringConversion::LOCALE:
            // Reset to the value the user selected before.
            maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero;
            mpEmptyAsZero->Enable();
            mpEmptyAsZero->Check( mbSelectedEmptyStringAsZero);
            break;
    }
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, AsZeroModifiedHdl, Button*, pCheckBox, void )
{
    maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero = static_cast<CheckBox*>(pCheckBox)->IsChecked();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, ConversionModifiedHdl, ListBox&, rConv, void )
{
    maConfig.meStringConversion = (ScCalcConfig::StringConversion)rConv.GetSelectEntryPos();
    CoupleEmptyAsZeroToStringConversion();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, SyntaxModifiedHdl, ListBox&, rSyntax, void)
{
    maConfig.SetStringRefSyntax(toAddressConvention(rSyntax.GetSelectEntryPos()));
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, CurrentDocOnlyHdl, Button*, pCheckBox, void)
{
    mbWriteConfig = !(static_cast<CheckBox*>(pCheckBox)->IsChecked());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
