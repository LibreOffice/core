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
  Look for places where we can flatten the control flow in a method by returning early.
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
    SourceRange extendOverComments(SourceRange range);
    std::string getSourceAsString(SourceRange range);
    std::string invertCondition(Expr const * condExpr, SourceRange conditionRange);
    std::vector<std::pair<const char *, const char*>> mvModifiedRanges;
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
static std::string deindent(std::string const & s);
static std::vector<std::string> split(std::string s);
static bool startswith(const std::string& rStr, const char* pSubStr);

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
    SourceRange elseKeywordRange = ifStmt->getElseLoc();

    // If we overlap with a previous area we modified, we cannot perform this change
    // without corrupting the source
    SourceManager& SM = compiler.getSourceManager();
    const char *p1 = SM.getCharacterData( ifStmt->getSourceRange().getBegin() );
    const char *p2 = SM.getCharacterData( ifStmt->getSourceRange().getEnd() );
    for (std::pair<const char*, const char *> const & rPair : mvModifiedRanges)
    {
        if (rPair.first <= p1 && p1 <= rPair.second)
            return false;
        if (p1 <= rPair.second && rPair.first <= p2)
            return false;
    }
    mvModifiedRanges.emplace_back(p1, p2);

    thenRange = extendOverComments(thenRange);
    elseRange = extendOverComments(elseRange);
    elseKeywordRange = extendOverComments(elseKeywordRange);

    // in adjusting the formatting I assume that "{" starts on a new line

    std::string conditionString = invertCondition(ifStmt->getCond(), conditionRange);

    std::string thenString = getSourceAsString(thenRange);
    if (auto compoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen())) {
        if (compoundStmt->getLBracLoc().isValid()) {
            thenString = stripOpenAndCloseBrace(thenString);
        }
    }
    thenString = deindent(thenString);

    std::string elseString = getSourceAsString(elseRange);

    if (!replaceText(elseRange, thenString)) {
        return false;
    }
    if (!removeText(elseKeywordRange)) {
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

std::string Flatten::invertCondition(Expr const * condExpr, SourceRange conditionRange)
{
    std::string s = getSourceAsString(conditionRange);

    condExpr = condExpr->IgnoreImpCasts();

    if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(condExpr))
        condExpr = exprWithCleanups->getSubExpr()->IgnoreImpCasts();

    if (auto unaryOp = dyn_cast<UnaryOperator>(condExpr))
    {
        if (unaryOp->getOpcode() != UO_LNot)
            return "!(" + s + ")";
        auto i = s.find("!");
        assert (i != std::string::npos);
        s = s.substr(i+1);
    }
    else if (isa<CXXOperatorCallExpr>(condExpr))
        s = "!(" + s + ")";
    else if (isa<DeclRefExpr>(condExpr) || isa<CallExpr>(condExpr) || isa<MemberExpr>(condExpr))
        s = "!" + s;
    else
        s = "!(" + s + ")";
    return s;
}

std::string stripOpenAndCloseBrace(std::string s)
{
    size_t i = s.find("{");
    if (i == std::string::npos)
        throw "did not find {";

    ++i;
    // strip to line end
    while (s[i] == ' ')
        ++i;
    if (s[i] == '\n')
         ++i;
    s = s.substr(i);

    i = s.rfind("}");
    if (i == std::string::npos)
        throw "did not find }";
    --i;
    while (s[i] == ' ')
        --i;
    s = s.substr(0,i);
    return s;
}

std::string deindent(std::string const & s)
{
    std::vector<std::string> lines = split(s);
    std::string rv;
    for (auto s : lines) {
        if (startswith(s, "    "))
            rv += s.substr(4);
        else
            rv += s;
        rv += "\n";
    }
    return rv;
}

std::vector<std::string> split(std::string s)
{
    if (s.back() == '\n')
        s = s.substr(0, s.size()-1);
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

static bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
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

/**
 * Extend the SourceRange to include any leading and trailing whitespace, and any comments.
 */
SourceRange Flatten::extendOverComments(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );

    // scan backwards from the beginning to include any spaces on that line
    while (*(p1-1) == ' ')
        --p1;
    startLoc = startLoc.getLocWithOffset(p1 - SM.getCharacterData( startLoc ));

    // look for trailing ";"
    while (*(p2+1) == ';')
        ++p2;
    // look for trailing " "
    while (*(p2+1) == ' ')
        ++p2;
    // look for single line comments attached to the end of the statement
    if (*(p2+1) == '/' && *(p2+2) == '/')
    {
        p2 += 2;
        while (*(p2+1) && *(p2+1) != '\n')
            ++p2;
        if (*(p2+1) == '\n')
            ++p2;
    }
    else
    {
        // make the source code we extract include any trailing "\n"
        if (*(p2+1) == '\n')
            ++p2;
    }
    endLoc = endLoc.getLocWithOffset(p2 - SM.getCharacterData( endLoc ));

    return SourceRange(startLoc, endLoc);
}

std::string Flatten::getSourceAsString(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );
    p2 += Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    return std::string( p1, p2 - p1);
}

loplugin::Plugin::Registration< Flatten > X("flatten", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
