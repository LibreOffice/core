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

#include "compat.hxx"
#include "plugin.hxx"

/**
  Look for comparisons where the constant is on the left, it should be on the right.
 */

namespace {

class ComparisonWithConstant :
    public RecursiveASTVisitor<ComparisonWithConstant>, public loplugin::Plugin
{
public:
    explicit ComparisonWithConstant(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator *);
private:
    bool isConstantExpr(const Expr*);
};

bool ComparisonWithConstant::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (!(binaryOp->getOpcode() == BO_EQ || binaryOp->getOpcode() == BO_NE)) {
        return true;
    }
    // ignore logging macros
    if (compiler.getSourceManager().isMacroBodyExpansion(binaryOp->getSourceRange().getBegin())
        || compiler.getSourceManager().isMacroArgExpansion(binaryOp->getSourceRange().getBegin())) {
        return true;
    }
    if (!isConstantExpr(binaryOp->getLHS())) {
        return true;
    }
    if (isConstantExpr(binaryOp->getRHS())) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning, "Rather put constant on right when comparing",
        binaryOp->getSourceRange().getBegin())
        << binaryOp->getSourceRange();
    return true;
}

bool ComparisonWithConstant::isConstantExpr(const Expr* expr)
{
    if (expr->isValueDependent()) {
        return false;
    }
    ASTContext& ctx(compiler.getASTContext());
    APValue result;
    if (expr->isIntegerConstantExpr(ctx)) {
        return true;
    }
    if (ctx.getLangOpts().CPlusPlus && expr->isCXX11ConstantExpr(ctx, &result)) {
        return true;
    }
    return false;
}

loplugin::Plugin::Registration< ComparisonWithConstant > X("comparisonwithconstant", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
