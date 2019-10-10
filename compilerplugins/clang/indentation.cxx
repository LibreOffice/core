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
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"

/*
Check for child statements inside a compound statement that do not share the same indentation.

TODO if an open-brace starts on a new line by itself, check that it lines up with it's closing-brace
TODO else should line up with if
*/

namespace
{
class Indentation : public loplugin::FilteringPlugin<Indentation>
{
public:
    explicit Indentation(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // include another file to build a table
        if (fn == SRCDIR "/sc/source/core/tool/cellkeytranslator.cxx")
            return false;
        // weird structure
        if (fn == SRCDIR "/sc/source/core/tool/compiler.cxx")
            return false;
        // looks like lex/yacc output
        if (fn == SRCDIR "/hwpfilter/source/grammar.cxx")
            return false;
        // TODO need to learn to handle attributes like "[[maybe_unused]]"
        if (fn == SRCDIR "/binaryurp/source/bridge.cxx")
            return false;
        // the QEMIT macros
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl/qt5/")
            || loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/gtk3_kde5/kde5_filepicker_ipc.cxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const*);
    bool PreTraverseSwitchStmt(SwitchStmt*);
    bool PostTraverseSwitchStmt(SwitchStmt*, bool);
    bool TraverseSwitchStmt(SwitchStmt*);
    bool VisitSwitchStmt(SwitchStmt const*);

private:
    std::vector<const Stmt*> switchStmtBodies;
};

bool Indentation::PreTraverseSwitchStmt(SwitchStmt* switchStmt)
{
    switchStmtBodies.push_back(switchStmt->getBody());
    return true;
}

bool Indentation::PostTraverseSwitchStmt(SwitchStmt*, bool)
{
    switchStmtBodies.pop_back();
    return true;
}

bool Indentation::TraverseSwitchStmt(SwitchStmt* switchStmt)
{
    PreTraverseSwitchStmt(switchStmt);
    auto ret = FilteringPlugin::TraverseSwitchStmt(switchStmt);
    PostTraverseSwitchStmt(switchStmt, ret);
    return ret;
}

bool Indentation::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    // these are kind of free form
    if (!switchStmtBodies.empty() && switchStmtBodies.back() == compoundStmt)
        return true;

    constexpr unsigned MAX = std::numeric_limits<unsigned>::max();
    unsigned column = MAX;
    Stmt const* firstStmt = nullptr;
    unsigned curLine = MAX;
    unsigned prevLine = MAX;
    SourceLocation prevEnd;
    auto& SM = compiler.getSourceManager();
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        auto stmt = *i;
        auto const actualPrevEnd = prevEnd;
        prevEnd = compat::getEndLoc(stmt); // compute early, before below `continue`s

        // these show up in macro expansions, not interesting
        if (isa<NullStmt>(stmt))
            continue;
        // these are always weirdly indented
        if (isa<LabelStmt>(stmt))
            continue;
        // At least until Clang 9.0.0, getBeginLoc of a VarDecl DeclStmt with an UnusedAttr points
        // after the attr (and getLocation of the attr would point at "maybe_unused", not at the
        // leading "[["), so just ignore those at least for now:
        if (auto const declStmt = dyn_cast<DeclStmt>(stmt))
        {
            if (declStmt->decl_begin() != declStmt->decl_end())
            {
                if (auto const decl = dyn_cast<VarDecl>(*declStmt->decl_begin()))
                {
                    if (decl->hasAttr<UnusedAttr>())
                    {
                        continue;
                    }
                }
            }
        }

        auto stmtLoc = compat::getBeginLoc(stmt);

        StringRef macroName;
        if (SM.isMacroArgExpansion(stmtLoc) || SM.isMacroBodyExpansion(stmtLoc))
        {
            macroName = Lexer::getImmediateMacroNameForDiagnostics(
                stmtLoc, compiler.getSourceManager(), compiler.getLangOpts());
            // CPPUNIT_TEST_SUITE/CPPUNIT_TEST/CPPUNIT_TEST_SUITE_END work together, so the one is indented inside the other
            if (macroName == "CPPUNIT_TEST_SUITE")
                continue;
            // similar thing in dbaccess/
            if (macroName == "DECL_PROP_IMPL")
                continue;
            // similar thing in forms/
            if (macroName == "DECL_IFACE_PROP_IMPL" || macroName == "DECL_BOOL_PROP_IMPL")
                continue;
#if CLANG_VERSION >= 70000
            stmtLoc = SM.getExpansionRange(stmtLoc).getBegin();
#else
            stmtLoc = SM.getExpansionRange(stmtLoc).first;
#endif
        }

        // check for comment to the left of the statement
        {
            const char* p1 = SM.getCharacterData(stmtLoc);
            --p1;
            bool foundComment = false;
            while (*p1 && *p1 != '\n')
            {
                if (*p1 == '/')
                {
                    foundComment = true;
                    break;
                }
                --p1;
            }
            if (foundComment)
                continue;
        }

        bool invalid1 = false;
        bool invalid2 = false;
        unsigned tmpColumn = SM.getPresumedColumnNumber(stmtLoc, &invalid1);
        unsigned tmpLine = SM.getPresumedLineNumber(stmtLoc, &invalid2);
        if (invalid1 || invalid2)
            continue;
        prevLine = curLine;
        curLine = tmpLine;
        if (column == MAX)
        {
            column = tmpColumn;
            firstStmt = stmt;
        }
        else if (curLine == prevLine)
        {
            // ignore multiple statements on same line
        }
        else if (column != tmpColumn)
        {
            if (containsPreprocessingConditionalInclusion(SourceRange(
                    locationAfterToken(compiler.getSourceManager().getExpansionLoc(actualPrevEnd)),
                    compiler.getSourceManager().getExpansionLoc(compat::getBeginLoc(stmt)))))
                continue;
            report(DiagnosticsEngine::Warning, "statement mis-aligned compared to neighbours %0",
                   stmtLoc)
                << macroName;
            report(DiagnosticsEngine::Note, "measured against this one",
                   compat::getBeginLoc(firstStmt));
            //getParentStmt(compoundStmt)->dump();
            //stmt->dump();
        }
    }
    return true;
}

bool Indentation::VisitSwitchStmt(SwitchStmt const* switchStmt)
{
    if (ignoreLocation(switchStmt))
        return true;

    constexpr unsigned MAX = std::numeric_limits<unsigned>::max();
    unsigned column = MAX;
    Stmt const* firstStmt = nullptr;
    unsigned curLine = MAX;
    unsigned prevLine = MAX;
    auto& SM = compiler.getSourceManager();
    auto compoundStmt = dyn_cast<CompoundStmt>(switchStmt->getBody());
    if (!compoundStmt)
        return true;
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        Stmt const* caseStmt = dyn_cast<CaseStmt>(*i);
        if (!caseStmt)
            caseStmt = dyn_cast<DefaultStmt>(*i);
        if (!caseStmt)
            continue;

        auto stmtLoc = compat::getBeginLoc(caseStmt);

        bool invalid1 = false;
        bool invalid2 = false;
        unsigned tmpColumn = SM.getPresumedColumnNumber(stmtLoc, &invalid1);
        unsigned tmpLine = SM.getPresumedLineNumber(stmtLoc, &invalid2);
        if (invalid1 || invalid2)
            continue;
        prevLine = curLine;
        curLine = tmpLine;
        if (column == MAX)
        {
            column = tmpColumn;
            firstStmt = caseStmt;
        }
        else if (curLine == prevLine)
        {
            // ignore multiple statements on same line
        }
        else if (column != tmpColumn)
        {
            // disable this for now, ends up touching some very large switch statements in sw/ and sc/
            (void)firstStmt;
            //            report(DiagnosticsEngine::Warning, "statement mis-aligned compared to neighbours",
            //                   stmtLoc);
            //            report(DiagnosticsEngine::Note, "measured against this one",
            //                   compat::getBeginLoc(firstStmt));
            //getParentStmt(compoundStmt)->dump();
            //stmt->dump();
        }
    }
    return true;
}

loplugin::Plugin::Registration<Indentation> indentation("indentation");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
