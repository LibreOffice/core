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

#include <cassert>
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
    // we replicate these macros in all the .hrc files
    if (aMacroName == "NC_" || aMacroName == "NNC_")
        return;
    // We define this prior to including <windows.h>:
    if (aMacroName == "WIN32_LEAN_AND_MEAN")
    {
        return;
    }
    // TODO no obvious fix for these
    if (aMacroName == "FID_SEARCH_NOW" || aMacroName == "FID_SVX_START" || aMacroName == "FN_PARAM")
        return;
    // ignore for now, requires adding too many includes to sw/
    if (aMacroName == "MM50")
        return;

    // ignore for now, we have the same define in svx and sw, but I can't remove one of them because
    // they reference strings in different resource bundles
    if (aMacroName == "STR_UNDO_COL_DELETE" || aMacroName == "STR_UNDO_ROW_DELETE"
        || aMacroName == "STR_TABLE_NUMFORMAT" || aMacroName == "STR_DELETE")
        return;

    if (m_aDefMap.emplace(aMacroName, Entry{ aLoc }).second)
    {
        return;
    }

    // Happens e.g. with macros defined in include/premac.h, which is intended to be included
    // (without include guards) multiple times:
    auto const other = m_aDefMap[aMacroName].m_aLoc;
    assert(aLoc == compiler.getSourceManager().getSpellingLoc(aLoc));
    assert(other == compiler.getSourceManager().getSpellingLoc(other));
    if ((compiler.getSourceManager().getFilename(aLoc)
         == compiler.getSourceManager().getFilename(other))
        && (compiler.getSourceManager().getSpellingLineNumber(aLoc)
            == compiler.getSourceManager().getSpellingLineNumber(other))
        && (compiler.getSourceManager().getSpellingColumnNumber(aLoc)
            == compiler.getSourceManager().getSpellingColumnNumber(other)))
    {
        return;
    }

    report(DiagnosticsEngine::Warning, "duplicate defines", aLoc);
    report(DiagnosticsEngine::Note, "previous define", other);
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

loplugin::Plugin::Registration<DuplicateDefines> X("duplicatedefines", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
