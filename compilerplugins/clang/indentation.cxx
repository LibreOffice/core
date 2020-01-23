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
    bool VisitNamespaceDecl(NamespaceDecl const*);

private:
    std::string GetFullNamespace(const NamespaceDecl* nsDecl);

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
#if CLANG_VERSION < 100000
        // Before
        // <https://github.com/llvm/llvm-project/commit/dc3957ec215dd17b8d293461f18696566637a6cd>
        // "Include leading attributes in DeclStmt's SourceRange", getBeginLoc of a VarDecl DeclStmt
        // with an UnusedAttr pointed after the attr (and getLocation of the attr pointed at
        // "maybe_unused", not at the leading "[["), so just ignore those in old compiler versions:
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
#endif

        auto stmtLoc = compat::getBeginLoc(stmt);

        StringRef macroName;
        bool partOfMacro = false;
        if (SM.isMacroArgExpansion(stmtLoc) || SM.isMacroBodyExpansion(stmtLoc))
        {
            partOfMacro = true;
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

        if (!partOfMacro)
            if (auto ifStmt = dyn_cast<IfStmt>(stmt))
            {
                auto bodyStmt = ifStmt->getThen();
                if (bodyStmt && !isa<CompoundStmt>(bodyStmt))
                {
                    stmtLoc = compat::getBeginLoc(bodyStmt);
                    invalid1 = false;
                    invalid2 = false;
                    unsigned bodyColumn = SM.getPresumedColumnNumber(stmtLoc, &invalid1);
                    unsigned bodyLine = SM.getPresumedLineNumber(stmtLoc, &invalid2);
                    if (invalid1 || invalid2)
                        return true;

                    if (bodyLine != tmpLine && bodyColumn <= tmpColumn)
                        report(DiagnosticsEngine::Warning, "if body should be indented", stmtLoc);
                }

                auto elseStmt = ifStmt->getElse();
                if (elseStmt && !isa<CompoundStmt>(elseStmt) && !isa<IfStmt>(elseStmt))
                {
                    stmtLoc = compat::getBeginLoc(elseStmt);
                    invalid1 = false;
                    invalid2 = false;
                    unsigned elseColumn = SM.getPresumedColumnNumber(stmtLoc, &invalid1);
                    unsigned elseLine = SM.getPresumedLineNumber(stmtLoc, &invalid2);
                    if (invalid1 || invalid2)
                        return true;
                    if (elseLine != tmpLine && elseColumn <= tmpColumn)
                        report(DiagnosticsEngine::Warning, "else body should be indented", stmtLoc);
                }
                if (elseStmt && !isa<CompoundStmt>(bodyStmt))
                {
                    stmtLoc = ifStmt->getElseLoc();
                    invalid1 = false;
                    invalid2 = false;
                    unsigned elseColumn = SM.getPresumedColumnNumber(stmtLoc, &invalid1);
                    unsigned elseLine = SM.getPresumedLineNumber(stmtLoc, &invalid2);
                    if (invalid1 || invalid2)
                        return true;
                    if (elseLine != tmpLine && elseColumn != tmpColumn)
                        report(DiagnosticsEngine::Warning, "if and else not aligned", stmtLoc);
                }
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

void trim(std::string& str)
{
    // right trim
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(it1.base(), str.end());
    // left trim
    auto it2 = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(str.begin(), it2);
}

bool Indentation::VisitNamespaceDecl(NamespaceDecl const* nsDecl)
{
    if (ignoreLocation(nsDecl))
        return true;
    if (nsDecl->isAnonymousNamespace())
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(nsDecl->getLocation())))
        return true;

    // right now, just fixing up the fallout from clang-tidy-modernize-namespaces, which
    // does not touch header files
    if (!compiler.getSourceManager().isInMainFile(nsDecl->getLocation()))
        return true;

    auto& SM = compiler.getSourceManager();

    // if we have a combined ns (.e.g namespace aaa::bbb), this appears in the AST
    // as two nested namespace sharing the same source locations, so ignore the outer decls
    if (!nsDecl->decls_empty())
    {
        auto child = dyn_cast_or_null<NamespaceDecl>(*nsDecl->decls_begin());
        if (child)
        {
            bool invalid1 = false;
            bool invalid2 = false;
            unsigned line1 = SM.getPresumedLineNumber(nsDecl->getBeginLoc(), &invalid1);
            unsigned line2 = SM.getPresumedLineNumber(child->getBeginLoc(), &invalid2);
            if (line1 == line2)
                return true;
        }
    }

    // Truly hacky way to find the actual beginning of an xxx::yyy namespace declaration
    // if we are inside the yyy NameSpaceDecl of
    //      namespace xxx::yyy
    // the beginLoc is just between the "xxx" and the "::"
    auto nsDeclBeginLoc = nsDecl->getBeginLoc();
    bool foundMultiple = false;
    {
        constexpr int BACKSCAN = 32;
        auto beginLoc = nsDecl->getBeginLoc().getLocWithOffset(-BACKSCAN);
        auto endLoc = nsDecl->getBeginLoc().getLocWithOffset(3);
        const char* p1 = SM.getCharacterData(beginLoc);
        const char* p2 = SM.getCharacterData(endLoc);
        unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
        if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
            return true;
        auto s = std::string(p1, p2 - p1);
        auto idx1 = s.rfind(" "); // find the space preceding the namespace token
        if (idx1 != std::string::npos)
        {
            auto namespaceToken = s.substr(idx1);
            if (namespaceToken.find("::") != std::string::npos)
            {
                auto idx = s.rfind("\n");
                nsDeclBeginLoc = nsDecl->getBeginLoc().getLocWithOffset(idx - BACKSCAN + 1);
                foundMultiple = true;
            }
        }
    }

    // for now, I am only interested in fixing the fallout from clang-tidy-modernize-namespace, not
    // anything else
    if (!foundMultiple)
        return true;

    bool invalid1 = false;
    bool invalid2 = false;
    unsigned col1 = SM.getPresumedColumnNumber(nsDeclBeginLoc, &invalid1);
    unsigned col2 = SM.getPresumedColumnNumber(nsDecl->getRBraceLoc(), &invalid2);
    unsigned line1 = SM.getPresumedLineNumber(nsDeclBeginLoc, &invalid1);
    unsigned line2 = SM.getPresumedLineNumber(nsDecl->getRBraceLoc(), &invalid2);
    if (invalid1 || invalid2)
        return true;
    if (line1 == line2) // single line declaration
        return true;
    if (col1 != col2)
        report(DiagnosticsEngine::Warning, "statement right brace mis-aligned",
               nsDecl->getRBraceLoc());

    // no easy way to get the position of the left brace
    auto endLoc = nsDecl->getBeginLoc().getLocWithOffset(256);
    const char* p1 = SM.getCharacterData(SM.getExpansionLoc(nsDecl->getBeginLoc()));
    const char* p2 = SM.getCharacterData(SM.getExpansionLoc(endLoc));
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
        return true;
    auto s = std::string(p1, p2 - p1 + n);
    auto idx1 = s.find("\n");
    auto idx2 = s.find("{");
    if (idx1 != std::string::npos && idx2 != std::string::npos)
        if (idx1 < idx2)
        {
            auto col3 = idx2 - idx1;
            if (col1 != col3)
                report(DiagnosticsEngine::Warning, "statement left brace mis-aligned",
                       nsDecl->getBeginLoc());
        }

    // extract the comment following the end brace
    auto beginLoc = nsDecl->getRBraceLoc();
    endLoc = beginLoc.getLocWithOffset(128);
    p1 = SM.getCharacterData(SM.getExpansionLoc(beginLoc));
    p2 = SM.getCharacterData(SM.getExpansionLoc(endLoc));
    n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
        return true;
    s = std::string(p1, p2 - p1 + n);
    idx1 = s.find("//");
    idx2 = s.find("\n");
    if (idx1 != std::string::npos && idx2 != std::string::npos && idx1 < idx2)
    {
        idx1 += 2;
        s = s.substr(idx1, idx2 - idx1);
        trim(s);
        std::string fullNamespace = GetFullNamespace(nsDecl);
        if (!(s == fullNamespace || s == (fullNamespace + " namespace") || s == "namespace"
              || s == ("namespace " + fullNamespace) || s == ("namespace ::" + fullNamespace)
              || s == ("end " + fullNamespace) || s == "end namespace"
              || s == ("end namespace " + fullNamespace)
              || s == ("end " + fullNamespace + " namespace") || s == "end of namespace"
              || s == ("end of namespace " + fullNamespace)
              || s == ("end of namespace ::" + fullNamespace)
              || s == ("eof of namespace " + fullNamespace)))
        {
            report(DiagnosticsEngine::Warning, "incorrect comment at end of namespace %0",
                   nsDecl->getRBraceLoc())
                << fullNamespace;
        }
    }

    return true;
}

std::string Indentation::GetFullNamespace(const NamespaceDecl* nsDecl)
{
    std::vector<llvm::StringRef> names;
    auto ns = nsDecl;
    while (ns)
    {
        names.push_back(ns->getName());
        ns = dyn_cast<NamespaceDecl>(ns->getParent());
    }
    std::string fullNamespace;
    for (auto it = names.rbegin(); it != names.rend(); ++it)
        fullNamespace += "::" + it->str();
    fullNamespace = fullNamespace.substr(2);
    return fullNamespace;
}

loplugin::Plugin::Registration<Indentation> indentation("indentation");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
