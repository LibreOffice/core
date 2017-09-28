/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ostream>
#include <set>

#include <formula/FormulaCompiler.hxx>
#include <formula/grammar.hxx>
#include <formula/opcode.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/configmgr.hxx>

#include "calcconfig.hxx"
#include "compiler.hxx"
#include "docsh.hxx"

#include <comphelper/configurationlistener.hxx>

using comphelper::ConfigurationListener;

static rtl::Reference<ConfigurationListener> const & getMiscListener()
{
    static rtl::Reference<ConfigurationListener> xListener;
    if (!xListener.is())
        xListener.set(new ConfigurationListener("/org.openoffice.Office.Common/Misc"));
    return xListener;
}

bool ScCalcConfig::isOpenCLEnabled()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return false;
    static comphelper::ConfigurationListenerProperty<bool> gOpenCLEnabled(getMiscListener(), "UseOpenCL");
    return gOpenCLEnabled.get();
}

bool ScCalcConfig::isSwInterpreterEnabled()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return false;
    static comphelper::ConfigurationListenerProperty<bool> gSwInterpreterEnabled(getMiscListener(), "UseSwInterpreter");
    return gSwInterpreterEnabled.get();
}

ScCalcConfig::ScCalcConfig() :
    meStringRefAddressSyntax(formula::FormulaGrammar::CONV_UNSPECIFIED),
    meStringConversion(StringConversion::LOCALE),     // old LibreOffice behavior
    mbEmptyStringAsZero(false),
    mbHasStringRefSyntax(false)
{
    setOpenCLConfigToDefault();

    // SAL _DEBUG(__FILE__ ":" << __LINE__ << ": ScCalcConfig::ScCalcConfig(): " << *this);
}

void ScCalcConfig::setOpenCLConfigToDefault()
{
    // Note that these defaults better be kept in sync with those in
    // officecfg/registry/schema/org/openoffice/Office/Calc.xcs.
    // Crazy.
    mbOpenCLAutoSelect = true;
    mnOpenCLMinimumFormulaGroupSize = 100;
}

void ScCalcConfig::reset()
{
    *this = ScCalcConfig();
}

void ScCalcConfig::MergeDocumentSpecific( const ScCalcConfig& r )
{
    // String conversion options are per document.
    meStringConversion       = r.meStringConversion;
    mbEmptyStringAsZero      = r.mbEmptyStringAsZero;
    // INDIRECT ref syntax is per document.
    meStringRefAddressSyntax = r.meStringRefAddressSyntax;
    mbHasStringRefSyntax      = r.mbHasStringRefSyntax;
}

void ScCalcConfig::SetStringRefSyntax( formula::FormulaGrammar::AddressConvention eConv )
{
    meStringRefAddressSyntax = eConv;
    mbHasStringRefSyntax = true;
}

bool ScCalcConfig::operator== (const ScCalcConfig& r) const
{
    return meStringRefAddressSyntax == r.meStringRefAddressSyntax &&
           meStringConversion == r.meStringConversion &&
           mbEmptyStringAsZero == r.mbEmptyStringAsZero &&
           mbHasStringRefSyntax == r.mbHasStringRefSyntax &&
           mbOpenCLAutoSelect == r.mbOpenCLAutoSelect &&
           maOpenCLDevice == r.maOpenCLDevice &&
           mnOpenCLMinimumFormulaGroupSize == r.mnOpenCLMinimumFormulaGroupSize;
}

bool ScCalcConfig::operator!= (const ScCalcConfig& r) const
{
    return !operator==(r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
