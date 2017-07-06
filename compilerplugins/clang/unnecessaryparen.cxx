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

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"

/**
look for unnecessary parentheses
*/

namespace {

class UnnecessaryParen:
    public RecursiveASTVisitor<UnnecessaryParen>, public loplugin::Plugin
{
public:
    explicit UnnecessaryParen(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );
        // fixing this, makes the source in the .y files look horrible
        if (loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
             return;
        if (loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/idlc/source/parser.cxx"))
             return;
        if (loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/rsc/source/parser/rscyacc.cxx"))
             return;

        // TODO yuck, comma operator at work
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/writerfilter/source/rtftok/rtftokenizer.cxx"))
             return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/html/htmltab.cxx"))
             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitParenExpr(const ParenExpr *);
    bool VisitIfStmt(const IfStmt *);
    bool VisitDoStmt(const DoStmt *);
    bool VisitWhileStmt(const WhileStmt *);
    bool VisitSwitchStmt(const SwitchStmt *);
    bool VisitCallExpr(const CallExpr *);
    bool TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *);
    bool TraverseCaseStmt(CaseStmt *);
    bool TraverseConditionalOperator(ConditionalOperator *);
private:
    void VisitSomeStmt(const Stmt *parent, const Expr* cond, StringRef stmtName);
    Expr* insideCaseStmt = nullptr;
    Expr* insideConditionalOperator = nullptr;
};

bool UnnecessaryParen::TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *)
{
    // for some reason, the parentheses in an expression like "sizeof(x)" actually show up
    // in the AST, so just ignore that part of the AST
    return true;
}

bool UnnecessaryParen::TraverseCaseStmt(CaseStmt * caseStmt)
{
    auto old = insideCaseStmt;
    insideCaseStmt = caseStmt->getLHS()->IgnoreImpCasts();
    bool ret = RecursiveASTVisitor::TraverseCaseStmt(caseStmt);
    insideCaseStmt = old;
    return ret;
}

bool UnnecessaryParen::TraverseConditionalOperator(ConditionalOperator * conditionalOperator)
{
    auto old = insideConditionalOperator;
    insideConditionalOperator = conditionalOperator->getCond()->IgnoreImpCasts();
    bool ret = RecursiveASTVisitor::TraverseConditionalOperator(conditionalOperator);
    insideConditionalOperator = old;
    return ret;
}

bool UnnecessaryParen::VisitParenExpr(const ParenExpr* parenExpr)
{
    if (ignoreLocation(parenExpr))
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    if (insideCaseStmt && parenExpr == insideCaseStmt)
        return true;
    if (insideConditionalOperator && parenExpr == insideConditionalOperator)
        return true;

    auto subParenExpr = dyn_cast<ParenExpr>(parenExpr->getSubExpr()->IgnoreImpCasts());
    if (subParenExpr) {
        if (subParenExpr->getLocStart().isMacroID())
            return true;
        report(
            DiagnosticsEngine::Warning, "parentheses around parentheses",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
    }

    auto declRefExpr = dyn_cast<DeclRefExpr>(parenExpr->getSubExpr()->IgnoreImpCasts());
    if (declRefExpr) {
        if (declRefExpr->getLocStart().isMacroID())
            return true;

        // hack for BAD_CAST macro
        SourceManager& SM = compiler.getSourceManager();
        const char *p1 = SM.getCharacterData( declRefExpr->getLocStart().getLocWithOffset(-10) );
        const char *p2 = SM.getCharacterData( declRefExpr->getLocStart() );
        if ( std::string(p1, p2 - p1).find("BAD_CAST") != std::string::npos )
            return true;

        report(
            DiagnosticsEngine::Warning, "unnecessary parentheses around identifier",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
    }
    return true;
}

bool UnnecessaryParen::VisitIfStmt(const IfStmt* ifStmt)
{
    VisitSomeStmt(ifStmt, ifStmt->getCond(), "if");
    return true;
}

bool UnnecessaryParen::VisitDoStmt(const DoStmt* doStmt)
{
    VisitSomeStmt(doStmt, doStmt->getCond(), "do");
    return true;
}

bool UnnecessaryParen::VisitWhileStmt(const WhileStmt* whileStmt)
{
    VisitSomeStmt(whileStmt, whileStmt->getCond(), "while");
    return true;
}

bool UnnecessaryParen::VisitSwitchStmt(const SwitchStmt* switchStmt)
{
    VisitSomeStmt(switchStmt, switchStmt->getCond(), "switch");
    return true;
}

void UnnecessaryParen::VisitSomeStmt(const Stmt *parent, const Expr* cond, StringRef stmtName)
{
    if (ignoreLocation(parent))
        return;
    if (parent->getLocStart().isMacroID())
        return;

    auto parenExpr = dyn_cast<ParenExpr>(cond->IgnoreImpCasts());
    if (parenExpr) {
        if (parenExpr->getLocStart().isMacroID())
            return;
        // assignments need extra parentheses or they generate a compiler warning
        auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
        if (binaryOp && binaryOp->getOpcode() == BO_Assign)
            return;
        report(
            DiagnosticsEngine::Warning, "parentheses immediately inside %0 statement",
            parenExpr->getLocStart())
            << stmtName
            << parenExpr->getSourceRange();
    }
}

bool UnnecessaryParen::VisitCallExpr(const CallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (callExpr->getLocStart().isMacroID())
        return true;
    if (callExpr->getNumArgs() != 1 || isa<CXXOperatorCallExpr>(callExpr))
        return true;

    auto parenExpr = dyn_cast<ParenExpr>(callExpr->getArg(0)->IgnoreImpCasts());
    if (parenExpr) {
        if (parenExpr->getLocStart().isMacroID())
            return true;
        // assignments need extra parentheses or they generate a compiler warning
        auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
        if (binaryOp && binaryOp->getOpcode() == BO_Assign)
            return true;
        report(
            DiagnosticsEngine::Warning, "parentheses immediately inside single-arg call",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration< UnnecessaryParen > X("unnecessaryparen", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
