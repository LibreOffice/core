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
        /*
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );
        if (fn == SRCDIR "/sd/source/ui/framework/factories/ChildWindowPane.cxx")
             return;
        if (fn == SRCDIR "/forms/source/component/Date.cxx")
             return;
        if (fn == SRCDIR "/forms/source/component/Time.cxx")
            return;
*/
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator *);
};

bool ComparisonWithConstant::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (!(binaryOp->getOpcode() == BO_EQ || binaryOp->getOpcode() == BO_NE
          || binaryOp->getOpcode() == BO_LT || binaryOp->getOpcode() == BO_GT
          || binaryOp->getOpcode() == BO_LE || binaryOp->getOpcode() == BO_GE)) {
        return true;
    }
    // ignore logging macros
    if (compiler.getSourceManager().isMacroBodyExpansion(binaryOp->getSourceRange().getBegin())
        || compiler.getSourceManager().isMacroArgExpansion(binaryOp->getSourceRange().getBegin())) {
        return true;
    }
    // protect against clang assert
    if (binaryOp->getLHS()->isValueDependent() || binaryOp->getRHS()->isValueDependent()) {
        return true;
    }
    APValue result;
    if (!binaryOp->getLHS()->isIntegerConstantExpr(compiler.getASTContext())) {
        return true;
    }
    if (binaryOp->getRHS()->isIntegerConstantExpr(compiler.getASTContext())) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning, "Rather put constant on right when comparing",
        binaryOp->getSourceRange().getBegin())
        << binaryOp->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< ComparisonWithConstant > X("comparisonwithconstant", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
