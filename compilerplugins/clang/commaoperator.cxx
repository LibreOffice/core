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
the comma operator is best used sparingly
*/

namespace {

Stmt const * lookThroughExprWithCleanups(Stmt const * stmt) {
    if (auto const e = dyn_cast_or_null<ExprWithCleanups>(stmt)) {
        return e->getSubExpr();
    }
    return stmt;
}

class CommaOperator:
    public RecursiveASTVisitor<CommaOperator>, public loplugin::Plugin
{
public:
    explicit CommaOperator(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseForStmt(ForStmt * stmt) {
        auto const saved1 = ignore1_;
        ignore1_ = lookThroughExprWithCleanups(stmt->getInit());
        auto const saved2 = ignore2_;
        ignore2_ = lookThroughExprWithCleanups(stmt->getInc());
        auto const ret = RecursiveASTVisitor::TraverseForStmt(stmt);
        ignore1_ = saved1;
        ignore2_ = saved2;
        return ret;
    }

    bool TraverseParenExpr(ParenExpr * expr) {
        auto const saved1 = ignore1_;
        ignore1_ = expr->getSubExpr();
        auto const ret = RecursiveASTVisitor::TraverseParenExpr(expr);
        ignore1_ = saved1;
        return ret;
    }

    bool TraverseBinComma(BinaryOperator * expr) {
        if (!WalkUpFromBinComma(expr)) {
            return false;
        }
        auto const saved1 = ignore1_;
        ignore1_ = expr->getLHS();
        auto const ret = TraverseStmt(expr->getLHS())
            && TraverseStmt(expr->getRHS());
        ignore1_ = saved1;
        return ret;
    }

    bool VisitBinComma(const BinaryOperator* );

private:
    Stmt const * ignore1_ = nullptr;
    Stmt const * ignore2_ = nullptr;
};

bool CommaOperator::VisitBinComma(const BinaryOperator* binaryOp)
{
    if (binaryOp == ignore1_ || binaryOp == ignore2_) {
        return true;
    }
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    // Ignore FD_SET expanding to "...} while(0, 0)" in some Microsoft
    // winsock2.h (TODO: improve heuristic of determining that the whole
    // binaryOp is part of a single macro body expansion):
    if (compiler.getSourceManager().isMacroBodyExpansion(
            binaryOp->getLocStart())
        && compiler.getSourceManager().isMacroBodyExpansion(
            binaryOp->getOperatorLoc())
        && compiler.getSourceManager().isMacroBodyExpansion(
            binaryOp->getLocEnd())
        && ignoreLocation(
            compiler.getSourceManager().getSpellingLoc(
                binaryOp->getOperatorLoc())))
    {
        return true;
    }
    report(
        DiagnosticsEngine::Warning, "comma operator hides code",
        binaryOp->getOperatorLoc())
      << binaryOp->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< CommaOperator > X("commaoperator", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
