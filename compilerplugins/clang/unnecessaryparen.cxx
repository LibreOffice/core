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
        // fixing this makes the source in the .y files look horrible
        if (loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
             return;
        if (loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/idlc/source/parser.cxx"))
             return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitParenExpr(const ParenExpr *);
    bool VisitIfStmt(const IfStmt *);
};

bool UnnecessaryParen::VisitParenExpr(const ParenExpr* parenExpr)
{
    if (ignoreLocation(parenExpr))
        return true;
    if (parenExpr->getLocStart().isMacroID())
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
    return true;
}

bool UnnecessaryParen::VisitIfStmt(const IfStmt* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    if (auto parenExpr = dyn_cast<ParenExpr>(ifStmt->getCond()->IgnoreImpCasts())) {
        if (parenExpr->getLocStart().isMacroID())
            return true;
        // assignments need extra parentheses or they generate a compiler warning
        auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
        if (binaryOp && binaryOp->getOpcode() == BO_Assign)
            return true;
        report(
            DiagnosticsEngine::Warning, "parentheses immediately inside if",
            ifStmt->getLocStart())
            << ifStmt->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration< UnnecessaryParen > X("unnecessaryparen", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
