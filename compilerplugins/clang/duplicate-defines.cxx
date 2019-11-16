/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <iostream>
#include <unordered_map>

#include "plugin.hxx"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/Signals.h>

/// Finds duplicated preprocessor defines, which generally indicate that some definition
/// needs to be centralised somewhere.

namespace
{
struct Entry
{
    clang::SourceLocation m_aLoc;
};

class DuplicateDefines : public clang::PPCallbacks, public loplugin::Plugin
{
public:
    explicit DuplicateDefines(const loplugin::InstantiationData& data);
    virtual void run() override;
    void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD) override;
    void MacroUndefined(const Token& MacroNameTok, const MacroDefinition& MD,
                        const MacroDirective* Undef) override;
    enum
    {
        isPPCallback = true
    };

private:
    clang::Preprocessor& m_rPP;
    std::unordered_map<std::string, Entry> m_aDefMap;
};

DuplicateDefines::DuplicateDefines(const loplugin::InstantiationData& data)
    : Plugin(data)
    , m_rPP(compiler.getPreprocessor())
{
    compiler.getPreprocessor().addPPCallbacks(std::unique_ptr<PPCallbacks>(this));
}

void DuplicateDefines::run()
{
    // nothing, only check preprocessor usage
}

void DuplicateDefines::MacroDefined(const Token& rMacroNameTok, const MacroDirective*)
{
    auto aLoc = rMacroNameTok.getLocation();
    if (ignoreLocation(aLoc))
        return;

    std::string aMacroName = m_rPP.getSpelling(rMacroNameTok);

    // some testing macro
    if (aMacroName == "RTL_STRING_CONST_FUNCTION")
        return;
    if (aMacroName == "rtl")
        return;
    // we replicate this macro in all the .hrc files
    if (aMacroName == "NC_")
        return;
    // TODO no obvious fix for these
    if (aMacroName == "FID_SEARCH_NOW" || aMacroName == "FID_SVX_START" || aMacroName == "FN_PARAM")
        return;

    if (!m_aDefMap.emplace(aMacroName, Entry{ aLoc }).second)
    {
        report(DiagnosticsEngine::Warning, "duplicate defines", aLoc);
        report(DiagnosticsEngine::Note, "previous define", m_aDefMap[aMacroName].m_aLoc);
        return;
    }
}

void DuplicateDefines::MacroUndefined(const Token& rMacroNameTok, const MacroDefinition& /*MD*/,
                                      const MacroDirective* /*Undef*/)
{
    auto aLoc = rMacroNameTok.getLocation();
    if (ignoreLocation(aLoc))
        return;

    std::string aMacroName = m_rPP.getSpelling(rMacroNameTok);
    m_aDefMap.erase(aMacroName);
}

loplugin::Plugin::Registration<DuplicateDefines> X("duplicatedefines", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
