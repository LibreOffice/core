/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"

/**
  Look for places where we can flatten the control flow in a method.

 */

namespace {

class Flatten:
    public RecursiveASTVisitor<Flatten>, public loplugin::RewritePlugin
{
public:
    explicit Flatten(InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseCXXCatchStmt(CXXCatchStmt * );
    bool VisitIfStmt(const IfStmt * );
private:
    bool rewrite(const IfStmt * );
    SourceRange ignoreMacroExpansions(SourceRange range);
    std::string getSourceAsString(SourceRange range);
};

static const Stmt * containsSingleThrowExpr(const Stmt * stmt)
{
    if (auto compoundStmt = dyn_cast<CompoundStmt>(stmt)) {
        if (compoundStmt->size() != 1)
            return nullptr;
        stmt = *compoundStmt->body_begin();
    }
    if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(stmt)) {
        stmt = exprWithCleanups->getSubExpr();
    }
    return dyn_cast<CXXThrowExpr>(stmt);
}

bool Flatten::TraverseCXXCatchStmt(CXXCatchStmt* )
{
    // ignore stuff inside catch statements, where doing a "if...else..throw" is more natural
    return true;
}

bool Flatten::VisitIfStmt(const IfStmt* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    if (!ifStmt->getElse())
        return true;

    // ignore if/then/else/if chains for now
    if (isa<IfStmt>(ifStmt->getElse()))
        return true;

    // ignore if we are part of an if/then/else/if chain
    auto parentIfStmt = dyn_cast<IfStmt>(parentStmt(ifStmt));
    if (parentIfStmt && parentIfStmt->getElse() == ifStmt)
        return true;

    auto throwExpr = containsSingleThrowExpr(ifStmt->getElse());
    if (!throwExpr)
        return true;

    // if both the "if" and the "else" contain throws, no improvement
    if (containsSingleThrowExpr(ifStmt->getThen()))
        return true;

    if (!rewrite(ifStmt))
    {
        report(
            DiagnosticsEngine::Warning,
            "unconditional throw in else branch, rather invert the condition, throw early, and flatten the normal case",
            throwExpr->getLocStart())
            << throwExpr->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            "if condition here",
            ifStmt->getLocStart())
            << ifStmt->getSourceRange();
    }
    return true;
}

static std::string stripOpenAndCloseBrace(std::string s);
static std::string deindentThenStmt(std::string const & s);
static std::vector<std::string> split(std::string const & s);

bool Flatten::rewrite(const IfStmt* ifStmt)
{
    if (!rewriter)
        return false;

    auto conditionRange = ignoreMacroExpansions(ifStmt->getCond()->getSourceRange());
    if (!conditionRange.isValid()) {
        return false;
    }
    auto thenRange = ignoreMacroExpansions(ifStmt->getThen()->getSourceRange());
    if (!thenRange.isValid()) {
        return false;
    }
    auto elseRange = ignoreMacroExpansions(ifStmt->getElse()->getSourceRange());
    if (!elseRange.isValid()) {
        return false;
    }
    auto elseKeywordRange = ifStmt->getElseLoc();

    // in adjusting the formatting I assume that "{" starts on a new line

    std::string conditionString = getSourceAsString(conditionRange);
    conditionString = "!(" + conditionString + ")";

    std::string thenString = getSourceAsString(thenRange);
    bool thenIsCompound = false;
    if (auto compoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen())) {
        if (compoundStmt->getLBracLoc().isValid()) {
            thenIsCompound = true;
            thenString = stripOpenAndCloseBrace(thenString);
        }
    }
    thenString = deindentThenStmt(thenString);

    std::string elseString = getSourceAsString(elseRange);
    bool elseIsCompound = false;
    if (auto compoundStmt = dyn_cast<CompoundStmt>(ifStmt->getElse())) {
        if (compoundStmt->getLBracLoc().isValid()) {
            elseIsCompound = true;
        }
    }
    // indent else block if necessary
    if (thenIsCompound && !elseIsCompound)
        elseString = "    " + elseString;

    if (!replaceText(elseRange, thenString)) {
        return false;
    }
    if (!replaceText(elseKeywordRange, "")) {
        return false;
    }
    if (!replaceText(thenRange, elseString)) {
        return false;
    }
    if (!replaceText(conditionRange, conditionString)) {
        return false;
    }

    return true;
}

std::string stripOpenAndCloseBrace(std::string s)
{
    size_t openBrace = s.find_first_of("{");
    if (openBrace != std::string::npos) {
        size_t openLineEnd = s.find_first_of("\n", openBrace + 1);
        if (openLineEnd != std::string::npos)
            s = s.substr(openLineEnd + 1);
        else
            s = s.substr(openBrace + 1);
    }
    size_t closeBrace = s.find_last_of("}");
    if (closeBrace != std::string::npos) {
        size_t closeLineEnd = s.find_last_of("\n", closeBrace);
        if (closeLineEnd != std::string::npos)
            s = s.substr(0, closeLineEnd - 1);
        else
            s = s.substr(0, closeBrace - 1);
    }
    return s;
}

std::string deindentThenStmt(std::string const & s)
{
    std::vector<std::string> lines = split(s);
    std::string rv;
    for (auto s : lines) {
        rv += s.length() > 4 ? s.substr(4) : s;
        rv += "\n";
    }
    return rv;
}

std::vector<std::string> split(std::string const & s)
{
    size_t next = -1;
    std::vector<std::string> rv;
    do
    {
        size_t current = next + 1;
        next = s.find_first_of( "\n", current );
        rv.push_back(s.substr( current, next - current ));
    }
    while (next != std::string::npos);
    return rv;
}

SourceRange Flatten::ignoreMacroExpansions(SourceRange range) {
    while (compiler.getSourceManager().isMacroArgExpansion(range.getBegin())) {
        range.setBegin(
            compiler.getSourceManager().getImmediateMacroCallerLoc(
                range.getBegin()));
    }
    if (range.getBegin().isMacroID()) {
        SourceLocation loc;
        if (Lexer::isAtStartOfMacroExpansion(
                range.getBegin(), compiler.getSourceManager(),
                compiler.getLangOpts(), &loc))
        {
            range.setBegin(loc);
        }
    }
    while (compiler.getSourceManager().isMacroArgExpansion(range.getEnd())) {
        range.setEnd(
            compiler.getSourceManager().getImmediateMacroCallerLoc(
                range.getEnd()));
    }
    if (range.getEnd().isMacroID()) {
        SourceLocation loc;
        if (Lexer::isAtEndOfMacroExpansion(
                range.getEnd(), compiler.getSourceManager(),
                compiler.getLangOpts(), &loc))
        {
            range.setEnd(loc);
        }
    }
    return range.getBegin().isMacroID() || range.getEnd().isMacroID()
        ? SourceRange() : range;
}

std::string Flatten::getSourceAsString(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );
    unsigned n = Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    return std::string( p1, p2 - p1 + n);
}

loplugin::Plugin::Registration< Flatten > X("flatten", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
