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
    public RecursiveASTVisitor<Flatten>, public loplugin::Plugin
{
public:
    explicit Flatten(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseCXXCatchStmt(CXXCatchStmt * );
    bool VisitIfStmt(const IfStmt * );
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
    return true;
}


loplugin::Plugin::Registration< Flatten > X("flatten", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
