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
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"

/*
Check for child statements inside a compound statement that do not share the same indentation.

TODO special case switch blocks and check that case statements all start on same column.
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

    virtual void run() override
    {
        //std::string fn(handler.getMainFileName());
        //loplugin::normalizeDotDotInFilePath(fn);
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const*);
    bool TraverseSwitchStmt(SwitchStmt*);

private:
    Stmt const* switchStmtBody = nullptr;
};

bool Indentation::TraverseSwitchStmt(SwitchStmt* switchStmt)
{
    auto prev = switchStmtBody;
    switchStmtBody = switchStmt->getBody();
    FilteringPlugin::TraverseSwitchStmt(switchStmt);
    switchStmtBody = prev;
    return true;
}

bool Indentation::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    // these are kind of free form
    if (switchStmtBody == compoundStmt)
        return true;

    constexpr unsigned MAX = std::numeric_limits<unsigned>::max();
    unsigned column = MAX;
    Stmt const* firstStmt = nullptr;
    unsigned curLine = MAX;
    unsigned prevLine = MAX;
    auto& SM = compiler.getSourceManager();
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        auto stmt = *i;

        // these show up in macro expansions, not interesting
        if (isa<NullStmt>(stmt))
            continue;
        // these are always weirdly indented
        if (isa<LabelStmt>(stmt))
            continue;

        auto stmtLoc = compat::getBeginLoc(stmt);

        if (SM.isMacroArgExpansion(stmtLoc) || SM.isMacroBodyExpansion(stmtLoc))
#if CLANG_VERSION >= 80000
            stmtLoc = SM.getExpansionRange(stmtLoc).getBegin();
#else
            stmtLoc = SM.getExpansionRange(stmtLoc).first;
#endif

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
            report(DiagnosticsEngine::Warning, "statement mis-aligned compared to neighbours",
                   stmtLoc);
            report(DiagnosticsEngine::Note, "measured against this one",
                   compat::getBeginLoc(firstStmt));
            //getParentStmt(compoundStmt)->dump();
            //stmt->dump();
        }
    }
    return true;
}

loplugin::Plugin::Registration<Indentation> X("indentation");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
