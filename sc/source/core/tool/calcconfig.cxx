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

#include "calcconfig.hxx"
#include "compiler.hxx"
#include "docsh.hxx"

ScCalcConfig::ScCalcConfig() :
    meStringRefAddressSyntax(formula::FormulaGrammar::CONV_UNSPECIFIED),
    meStringConversion(STRING_CONVERSION_LOCALE_DEPENDENT),     // old LibreOffice behavior
    mbEmptyStringAsZero(false)
{
    setOpenCLConfigToDefault();

    // SAL _DEBUG(__FILE__ ":" << __LINE__ << ": ScCalcConfig::ScCalcConfig(): " << *this);
}

void ScCalcConfig::setOpenCLConfigToDefault()
{
    // Note that these defaults better be kept in sync with those in
    // officecfg/registry/schema/org/openoffice/Office/Calc.xcs.
    // Crazy.
    mbOpenCLSubsetOnly = true;
    mbOpenCLAutoSelect = true;
    mnOpenCLMinimumFormulaGroupSize = 100;

    // Keep in order of opcode value, is that clearest? (Random order,
    // at least, would make no sense at all.)
    maOpenCLSubsetOpCodes.insert(ocAdd);
    maOpenCLSubsetOpCodes.insert(ocSub);
    maOpenCLSubsetOpCodes.insert(ocMul);
    maOpenCLSubsetOpCodes.insert(ocDiv);
    maOpenCLSubsetOpCodes.insert(ocSin);
    maOpenCLSubsetOpCodes.insert(ocCos);
    maOpenCLSubsetOpCodes.insert(ocTan);
    maOpenCLSubsetOpCodes.insert(ocArcTan);
    maOpenCLSubsetOpCodes.insert(ocExp);
    maOpenCLSubsetOpCodes.insert(ocLn);
    maOpenCLSubsetOpCodes.insert(ocSqrt);
    maOpenCLSubsetOpCodes.insert(ocSNormInv);
    maOpenCLSubsetOpCodes.insert(ocRound);
    maOpenCLSubsetOpCodes.insert(ocPower);
    maOpenCLSubsetOpCodes.insert(ocSumProduct);
    maOpenCLSubsetOpCodes.insert(ocMin);
    maOpenCLSubsetOpCodes.insert(ocMax);
    maOpenCLSubsetOpCodes.insert(ocSum);
    maOpenCLSubsetOpCodes.insert(ocProduct);
    maOpenCLSubsetOpCodes.insert(ocAverage);
    maOpenCLSubsetOpCodes.insert(ocCount);
    maOpenCLSubsetOpCodes.insert(ocNormDist);
    maOpenCLSubsetOpCodes.insert(ocSumIfs);
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
}

bool ScCalcConfig::operator== (const ScCalcConfig& r) const
{
    return meStringRefAddressSyntax == r.meStringRefAddressSyntax &&
           meStringConversion == r.meStringConversion &&
           mbEmptyStringAsZero == r.mbEmptyStringAsZero &&
           mbOpenCLSubsetOnly == r.mbOpenCLSubsetOnly &&
           mbOpenCLAutoSelect == r.mbOpenCLAutoSelect &&
           maOpenCLDevice == r.maOpenCLDevice &&
           mnOpenCLMinimumFormulaGroupSize == r.mnOpenCLMinimumFormulaGroupSize &&
           maOpenCLSubsetOpCodes == r.maOpenCLSubsetOpCodes &&
           true;
}

bool ScCalcConfig::operator!= (const ScCalcConfig& r) const
{
    return !operator==(r);
}

std::ostream& operator<<(std::ostream& rStream, const ScCalcConfig& rConfig)
{
    rStream << "{"
        "StringRefAddressSyntax=" << rConfig.meStringRefAddressSyntax << ","
        "StringConversion=" << rConfig.meStringConversion << ","
        "EmptyStringAsZero=" << (rConfig.mbEmptyStringAsZero?"Y":"N") << ","
        "OpenCLSubsetOnly=" << (rConfig.mbOpenCLSubsetOnly?"Y":"N") << ","
        "OpenCLAutoSelect=" << (rConfig.mbOpenCLAutoSelect?"Y":"N") << ","
        "OpenCLDevice='" << rConfig.maOpenCLDevice << "',"
        "OpenCLMinimumFormulaGroupSize=" << rConfig.mnOpenCLMinimumFormulaGroupSize << ","
        "OpenCLSubsetOpCodes={" << ScOpCodeSetToSymbolicString(rConfig.maOpenCLSubsetOpCodes) << "},"
        "}";
    return rStream;
}

namespace {

formula::FormulaCompiler::OpCodeMapPtr setup()
{
    SfxObjectShell* pObjShell = SfxObjectShell::Current();
    ScDocShell* pScDocShell = PTR_CAST(ScDocShell, pObjShell);

    if (pScDocShell)
    {
        ScDocument& rDoc(pScDocShell->GetDocument());
        ScCompiler* pComp(new ScCompiler(&rDoc, ScAddress()));
        return pComp->GetOpCodeMap(css::sheet::FormulaLanguage::NATIVE);
    }

    return nullptr;
}

} // anonymous namespace

OUString ScOpCodeSetToNumberString(const ScCalcConfig::OpCodeSet& rOpCodes)
{
    OUStringBuffer result;

    for (auto i = rOpCodes.cbegin(); i != rOpCodes.cend(); ++i)
    {
        if (i != rOpCodes.cbegin())
            result.append(';');
        result.append(static_cast<sal_Int32>(*i));
    }

    return result.toString();
}

OUString ScOpCodeSetToSymbolicString(const ScCalcConfig::OpCodeSet& rOpCodes)
{
    OUStringBuffer result;
    formula::FormulaCompiler::OpCodeMapPtr pOpCodeMap(setup());

    if (!pOpCodeMap)
        return ScOpCodeSetToNumberString(rOpCodes);

    for (auto i = rOpCodes.cbegin(); i != rOpCodes.cend(); ++i)
    {
        if (i != rOpCodes.cbegin())
            result.append(';');
        result.append(pOpCodeMap->getSymbol(*i));
    }

    return result.toString();
}

ScCalcConfig::OpCodeSet ScStringToOpCodeSet(const OUString& rOpCodes)
{
    ScCalcConfig::OpCodeSet result;
    formula::FormulaCompiler::OpCodeMapPtr pOpCodeMap(setup());

    OUString s(rOpCodes + ";");

    const formula::OpCodeHashMap *pHashMap(nullptr);
    if (pOpCodeMap)
        pHashMap = pOpCodeMap->getHashMap();

    sal_Int32 fromIndex(0);
    sal_Int32 semicolon;
    while ((semicolon = s.indexOf(';', fromIndex)) >= 0)
    {
        if (semicolon > fromIndex)
        {
            OUString element(s.copy(fromIndex, semicolon - fromIndex));
            sal_Int32 n = element.toInt32();
            if (n > 0 || (n == 0 && element == "0"))
                result.insert(static_cast<OpCode>(n));
            else if (pHashMap)
            {
                auto opcode(pHashMap->find(element));
                if (opcode != pHashMap->end())
                    result.insert(static_cast<OpCode>(opcode->second));
                else
                    SAL_WARN("sc.opencl", "Unrecognized OpCode " << element << " in OpCode set string");
            }
            else
            {
                SAL_WARN("sc.opencl", "No current doc, can't convert from OpCode name to value");
            }
        }
        fromIndex = semicolon+1;
    }
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
