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
#include <stack>

#include "plugin.hxx"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/Signals.h>

/// Finds preprocessor usage which is redundant (only #ifndef for now).

namespace loplugin
{
struct Entry
{
    clang::SourceLocation m_aLoc;
    std::string m_aMacroName;
};

class RedundantPreprocessor : public clang::PPCallbacks, public Plugin
{
public:
    explicit RedundantPreprocessor(const InstantiationData& data);
    virtual void run() override;
    void Ifndef(clang::SourceLocation aLoc, const clang::Token& rMacroNameTok,
                const clang::MacroDefinition& rMacroDefinition) override;
    void Ifdef(clang::SourceLocation aLoc, const clang::Token& rMacroNameTok,
               const clang::MacroDefinition& rMacroDefinition) override;
    void Endif(clang::SourceLocation aLoc, clang::SourceLocation aIfLoc) override;
    enum
    {
        isPPCallback = true
    };

private:
    clang::Preprocessor& m_rPP;
    std::vector<Entry> m_aDefStack;
    std::vector<Entry> m_aNotDefStack;
};

RedundantPreprocessor::RedundantPreprocessor(const InstantiationData& data)
    : Plugin(data)
    , m_rPP(compiler.getPreprocessor())
{
    compiler.getPreprocessor().addPPCallbacks(std::unique_ptr<PPCallbacks>(this));
}

void RedundantPreprocessor::run()
{
    // nothing, only check preprocessor usage
}

void RedundantPreprocessor::Ifdef(clang::SourceLocation aLoc, const clang::Token& rMacroNameTok,
                                  const clang::MacroDefinition& /*rMacroDefinition*/)
{
    if (ignoreLocation(aLoc))
        return;

    if (m_rPP.getSourceManager().isInMainFile(aLoc))
    {
        std::string aMacroName = m_rPP.getSpelling(rMacroNameTok);
        for (const auto& rEntry : m_aDefStack)
        {
            if (rEntry.m_aMacroName == aMacroName)
            {
                report(DiagnosticsEngine::Warning, "nested ifdef", aLoc);
                report(DiagnosticsEngine::Note, "previous ifdef", rEntry.m_aLoc);
            }
        }
    }

    Entry aEntry;
    aEntry.m_aLoc = aLoc;
    aEntry.m_aMacroName = m_rPP.getSpelling(rMacroNameTok);
    m_aDefStack.push_back(aEntry);
}

void RedundantPreprocessor::Ifndef(clang::SourceLocation aLoc, const clang::Token& rMacroNameTok,
                                   const clang::MacroDefinition& /*rMacroDefinition*/)
{
    if (ignoreLocation(aLoc))
        return;

    if (m_rPP.getSourceManager().isInMainFile(aLoc))
    {
        std::string aMacroName = m_rPP.getSpelling(rMacroNameTok);
        for (const auto& rEntry : m_aNotDefStack)
        {
            if (rEntry.m_aMacroName == aMacroName)
            {
                report(DiagnosticsEngine::Warning, "nested ifndef", aLoc);
                report(DiagnosticsEngine::Note, "previous ifndef", rEntry.m_aLoc);
            }
        }
    }

    Entry aEntry;
    aEntry.m_aLoc = aLoc;
    aEntry.m_aMacroName = m_rPP.getSpelling(rMacroNameTok);
    m_aNotDefStack.push_back(aEntry);
}

void RedundantPreprocessor::Endif(clang::SourceLocation /*aLoc*/, clang::SourceLocation aIfLoc)
{
    if (!m_aDefStack.empty())
    {
        if (aIfLoc == m_aDefStack.back().m_aLoc)
            m_aDefStack.pop_back();
    }
    if (!m_aNotDefStack.empty())
    {
        if (aIfLoc == m_aNotDefStack.back().m_aLoc)
            m_aNotDefStack.pop_back();
    }
}

static Plugin::Registration<RedundantPreprocessor> X("redundantpreprocessor");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
