/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <stack>

/**
  Look for places where we can flatten the control flow in a method by returning early.
 */
namespace {

class Flatten:
    public loplugin::FilteringRewritePlugin<Flatten>
{
public:
    explicit Flatten(loplugin::InstantiationData const & data):
        FilteringRewritePlugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseIfStmt(IfStmt *);
    bool TraverseCXXCatchStmt(CXXCatchStmt * );
    bool TraverseCompoundStmt(CompoundStmt *);
    bool TraverseFunctionDecl(FunctionDecl *);
    bool TraverseCXXMethodDecl(CXXMethodDecl *);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl *);
    bool TraverseCXXConversionDecl(CXXConversionDecl *);
    bool TraverseCXXDestructorDecl(CXXDestructorDecl *);
    bool VisitIfStmt(IfStmt const * );
private:
    bool rewrite1(IfStmt const * );
    bool rewrite2(IfStmt const * );
    bool rewriteLargeIf(IfStmt const * );
    SourceRange ignoreMacroExpansions(SourceRange range);
    SourceRange extendOverComments(SourceRange range);
    std::string getSourceAsString(SourceRange range);
    llvm::Optional<std::string> invertCondition(Expr const * condExpr, SourceRange conditionRange);
    bool isLargeCompoundStmt(Stmt const *);

    Stmt const * lastStmtInCompoundStmt = nullptr;
    FunctionDecl const * functionDecl = nullptr;
    CompoundStmt const * functionDeclBody = nullptr;
    Stmt const * mElseBranch = nullptr;
};

static Stmt const * containsSingleThrowExpr(Stmt const * stmt)
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

static bool containsVarDecl(Stmt const * stmt)
{
    if (auto compoundStmt = dyn_cast<CompoundStmt>(stmt)) {
        for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i) {
            auto declStmt = dyn_cast<DeclStmt>(*i);
            if (declStmt && isa<VarDecl>(*declStmt->decl_begin()))
                return true;
        }
        return false;
    }
    auto declStmt = dyn_cast<DeclStmt>(stmt);
    return declStmt && isa<VarDecl>(*declStmt->decl_begin());
}

bool Flatten::TraverseCXXCatchStmt(CXXCatchStmt* )
{
    // ignore stuff inside catch statements, where doing a "if...else..throw" is more natural
    return true;
}

bool Flatten::TraverseIfStmt(IfStmt * ifStmt)
{
    if (!WalkUpFromIfStmt(ifStmt)) {
        return false;
    }
    auto const saved = mElseBranch;
    mElseBranch = ifStmt->getElse();
    auto ret = true;
    for (auto const sub: ifStmt->children()) {
        if (!TraverseStmt(sub)) {
            ret = false;
            break;
        }
    }
    mElseBranch = saved;
    return ret;
}

bool Flatten::TraverseCompoundStmt(CompoundStmt * compoundStmt)
{
    auto copy = lastStmtInCompoundStmt;
    if (compoundStmt->size() > 0)
        lastStmtInCompoundStmt = compoundStmt->body_back();
    else
        lastStmtInCompoundStmt = nullptr;

    bool rv = RecursiveASTVisitor<Flatten>::TraverseCompoundStmt(compoundStmt);

    lastStmtInCompoundStmt = copy;
    return rv;
}

bool Flatten::TraverseFunctionDecl(FunctionDecl * fd)
{
    auto copy1 = functionDeclBody;
    auto copy2 = fd;
    functionDeclBody = dyn_cast_or_null<CompoundStmt>(fd->getBody());
    functionDecl = fd;
    bool rv = RecursiveASTVisitor<Flatten>::TraverseFunctionDecl(fd);
    functionDeclBody = copy1;
    functionDecl = copy2;
    return rv;
}

bool Flatten::TraverseCXXMethodDecl(CXXMethodDecl * fd)
{
    auto copy1 = functionDeclBody;
    auto copy2 = fd;
    functionDeclBody = dyn_cast_or_null<CompoundStmt>(fd->getBody());
    functionDecl = fd;
    bool rv = RecursiveASTVisitor<Flatten>::TraverseCXXMethodDecl(fd);
    functionDeclBody = copy1;
    functionDecl = copy2;
    return rv;
}

bool Flatten::TraverseCXXConstructorDecl(CXXConstructorDecl * fd)
{
    auto copy1 = functionDeclBody;
    auto copy2 = fd;
    functionDeclBody = dyn_cast_or_null<CompoundStmt>(fd->getBody());
    functionDecl = fd;
    bool rv = RecursiveASTVisitor<Flatten>::TraverseCXXConstructorDecl(fd);
    functionDeclBody = copy1;
    functionDecl = copy2;
    return rv;
}

bool Flatten::TraverseCXXConversionDecl(CXXConversionDecl * fd)
{
    auto copy1 = functionDeclBody;
    auto copy2 = fd;
    functionDeclBody = dyn_cast_or_null<CompoundStmt>(fd->getBody());
    functionDecl = fd;
    bool rv = RecursiveASTVisitor<Flatten>::TraverseCXXConversionDecl(fd);
    functionDeclBody = copy1;
    functionDecl = copy2;
    return rv;
}

bool Flatten::TraverseCXXDestructorDecl(CXXDestructorDecl * fd)
{
    auto copy1 = functionDeclBody;
    auto copy2 = fd;
    functionDeclBody = dyn_cast_or_null<CompoundStmt>(fd->getBody());
    functionDecl = fd;
    bool rv = RecursiveASTVisitor<Flatten>::TraverseCXXDestructorDecl(fd);
    functionDeclBody = copy1;
    functionDecl = copy2;
    return rv;
}


bool Flatten::VisitIfStmt(IfStmt const * ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    // ignore if we are part of an if/then/else/if chain
    if (ifStmt == mElseBranch || (ifStmt->getElse() && isa<IfStmt>(ifStmt->getElse())))
        return true;

    // look for a large if(){} block at the end of a function
    if (!ifStmt->getElse()
        && (functionDecl->getReturnType().isNull() || functionDecl->getReturnType()->isVoidType())
        && functionDeclBody && functionDeclBody->size()
        && functionDeclBody->body_back() == ifStmt
        && isLargeCompoundStmt(ifStmt->getThen()))
    {
        if (!rewriteLargeIf(ifStmt))
        {
            report(
                DiagnosticsEngine::Warning,
                "large if statement at end of function, rather invert the condition and exit early, and flatten the function",
                compat::getBeginLoc(ifStmt))
              << ifStmt->getSourceRange();
        }
        return true;
    }

    if (!ifStmt->getElse())
        return true;

    auto const thenThrowExpr = containsSingleThrowExpr(ifStmt->getThen());
    auto const elseThrowExpr = containsSingleThrowExpr(ifStmt->getElse());
    // If neither contains a throw, nothing to do; if both contain throws, no
    // improvement:
    if ((thenThrowExpr == nullptr) == (elseThrowExpr == nullptr)) {
        return true;
    }

    if (containsPreprocessingConditionalInclusion(ifStmt->getSourceRange())) {
        return true;
    }

    if (elseThrowExpr)
    {
        // if the "if" statement is not the last statement in its block, and it contains
        // var decls in its then block, we cannot de-indent the then block without
        // extending the lifetime of some variables, which may be problematic
        if (ifStmt != lastStmtInCompoundStmt && containsVarDecl(ifStmt->getThen()))
            return true;

        if (!rewrite1(ifStmt))
        {
            report(
                DiagnosticsEngine::Warning,
                "unconditional throw in else branch, rather invert the condition, throw early, and flatten the normal case",
                compat::getBeginLoc(elseThrowExpr))
                << elseThrowExpr->getSourceRange();
            report(
                DiagnosticsEngine::Note,
                "if condition here",
                compat::getBeginLoc(ifStmt))
                << ifStmt->getSourceRange();
        }
    }
    if (thenThrowExpr)
    {
        // if the "if" statement is not the last statement in its block, and it contains
        // var decls in its else block, we cannot de-indent the else block without
        // extending the lifetime of some variables, which may be problematic
        if (ifStmt != lastStmtInCompoundStmt && containsVarDecl(ifStmt->getElse()))
            return true;

        if (!rewrite2(ifStmt))
        {
            report(
                DiagnosticsEngine::Warning,
                "unconditional throw in then branch, just flatten the else",
                compat::getBeginLoc(thenThrowExpr))
                << thenThrowExpr->getSourceRange();
        }
    }
    return true;
}

static std::string stripOpenAndCloseBrace(std::string s);
static std::string stripTrailingEmptyLines(std::string s);
static std::string deindent(std::string const & s);
static std::vector<std::string> split(std::string s);
static bool startswith(std::string const & rStr, char const * pSubStr);
static int countLeadingSpaces(std::string const &);
static std::string padSpace(int iNoSpaces);
static bool replace(std::string & s, std::string const & from, std::string const & to);

bool Flatten::rewrite1(IfStmt const * ifStmt)
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

    thenRange = extendOverComments(thenRange);
    elseRange = extendOverComments(elseRange);
    elseKeywordRange = extendOverComments(elseKeywordRange);

    // in adjusting the formatting I assume that "{" starts on a new line

    llvm::Optional<std::string> conditionString = invertCondition(ifStmt->getCond(), conditionRange);
    if (!conditionString)
        return false;

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
    if (!replaceText(conditionRange, *conditionString)) {
        return false;
    }

    return true;
}

bool Flatten::rewrite2(IfStmt const * ifStmt)
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

    elseRange = extendOverComments(elseRange);
    elseKeywordRange = extendOverComments(elseKeywordRange);

    // in adjusting the formatting I assume that "{" starts on a new line

    std::string elseString = getSourceAsString(elseRange);
    if (auto compoundStmt = dyn_cast<CompoundStmt>(ifStmt->getElse())) {
        if (compoundStmt->getLBracLoc().isValid()) {
            elseString = stripOpenAndCloseBrace(elseString);
        }
    }
    elseString = deindent(elseString);

    if (!replaceText(elseRange, elseString)) {
        return false;
    }
    if (!removeText(elseKeywordRange)) {
        return false;
    }

    return true;
}

bool Flatten::rewriteLargeIf(IfStmt const * ifStmt)
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

    thenRange = extendOverComments(thenRange);

    // in adjusting the formatting I assume that "{" starts on a new line

    llvm::Optional<std::string> conditionString = invertCondition(ifStmt->getCond(), conditionRange);
    if (!conditionString)
        return false;

    std::string thenString = getSourceAsString(thenRange);
    if (auto compoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen())) {
        if (compoundStmt->getLBracLoc().isValid()) {
            thenString = stripOpenAndCloseBrace(thenString);
        }
    }
    int iNoSpaces = countLeadingSpaces(thenString);
    thenString = padSpace(iNoSpaces) + "return;\n\n" + deindent(thenString);
    thenString = stripTrailingEmptyLines(thenString);

    if (!replaceText(thenRange, thenString)) {
        return false;
    }
    if (!replaceText(conditionRange, *conditionString)) {
        return false;
    }

    return true;
}

llvm::Optional<std::string> Flatten::invertCondition(Expr const * condExpr, SourceRange conditionRange)
{
    std::string s = getSourceAsString(conditionRange);

    condExpr = condExpr->IgnoreImpCasts();

    if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(condExpr))
        condExpr = exprWithCleanups->getSubExpr()->IgnoreImpCasts();

    // an if statement will automatically invoke a bool-conversion method
    if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(condExpr))
    {
        if (memberCallExpr->getMethodDecl() && isa<CXXConversionDecl>(memberCallExpr->getMethodDecl()))
            condExpr = memberCallExpr->getImplicitObjectArgument()->IgnoreImpCasts();
    }

    if (auto unaryOp = dyn_cast<UnaryOperator>(condExpr))
    {
        if (unaryOp->getOpcode() != UO_LNot)
            return "!(" + s + ")";
        auto i = s.find("!");
        assert (i != std::string::npos);
        s = s.substr(i+1);
    }
    else if (auto binaryOp = dyn_cast<BinaryOperator>(condExpr))
    {
        bool ok = true;
        switch (binaryOp->getOpcode())
        {
            case BO_LT: ok = replace(s, "<", ">="); break;
            case BO_GT: ok = replace(s, ">", "<="); break;
            case BO_LE: ok = replace(s, "<=", ">"); break;
            case BO_GE: ok = replace(s, ">=", "<"); break;
            case BO_EQ: ok = replace(s, "==", "!="); break;
            case BO_NE: ok = replace(s, "!=", "=="); break;
            default:
                s = "!(" + s + ")";
        }
        if (!ok)
           return llvm::Optional<std::string>();
    }
    else if (auto opCallExpr = dyn_cast<CXXOperatorCallExpr>(condExpr))
    {
        bool ok = true;
        switch (opCallExpr->getOperator())
        {
            case OO_Less: ok = replace(s, "<", ">="); break;
            case OO_Greater: ok = replace(s, ">", "<="); break;
            case OO_LessEqual: ok = replace(s, "<=", ">"); break;
            case OO_GreaterEqual: ok = replace(s, ">=", "<"); break;
            case OO_EqualEqual: ok = replace(s, "==", "!="); break;
            case OO_ExclaimEqual: ok = replace(s, "!=", "=="); break;
            default:
                s = "!(" + s + ")";
        }
        if (!ok)
            return llvm::Optional<std::string>();
    }
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
    {
        assert( !"did not find {" );
        abort();
    }

    ++i;
    // strip to line end
    while (s[i] == ' ')
        ++i;
    if (s[i] == '\n')
         ++i;
    s = s.substr(i);

    i = s.rfind("}");
    if (i == std::string::npos)
    {
        assert( !"did not find }" );
        abort();
    }
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

bool startswith(std::string const & rStr, char const * pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

int countLeadingSpaces(std::string const & s)
{
    int i = 0;
    while (i < (int)s.length() && s[i] == ' ')
        i++;
    return i;
}

std::string padSpace(int iNoSpaces)
{
    std::string s;
    for (int i = 0; i < iNoSpaces; ++i)
        s += " ";
    return s;
}

std::string stripTrailingEmptyLines(std::string s)
{
    while (s.back() == '\n')
        s.resize(s.length() - 1);
    return s;
}

bool replace(std::string & s, std::string const & from, std::string const & to)
{
    auto i = s.find(from);
    assert (i != std::string::npos);
    s.replace(i, from.length(), to);
    // just in case we have something really weird, like the operator token is also present in the rest of the condition somehow
   return s.find(from) == std::string::npos;
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
    char const *p1 = SM.getCharacterData( startLoc );
    char const *p2 = SM.getCharacterData( endLoc );

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
    char const *p1 = SM.getCharacterData( startLoc );
    char const *p2 = SM.getCharacterData( endLoc );
    p2 += Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    if (p2 < p1) {
        // workaround clang weirdness, but don't return empty string
        // in case it happens during code replacement
        return "clang returned bad pointers";
    }
    if (p2 - p1 > 64 * 1024) {
        // workaround clang weirdness, but don't return empty string
        // in case it happens during code replacement
        return "clang returned overly large source range";
    }
    return std::string( p1, p2 - p1);
}

bool Flatten::isLargeCompoundStmt(Stmt const * stmt)
{
    auto stmtRange = stmt->getSourceRange();
    std::string s = getSourceAsString(stmtRange);
    return std::count(s.begin(), s.end(), '\n') > 10;
}

loplugin::Plugin::Registration< Flatten > X("flatten", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
