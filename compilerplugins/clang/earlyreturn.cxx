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

plugin that looks for code like:
    if (!pEntry || (nMaxLeading < 0 || nMaxLeading > nWordLen))
    {
        return nullptr;
    }
    else
where we can reduce indentation in the else branch. And make the rest of the method much easier to read.

 * */

namespace {

class EarlyReturn:
    public RecursiveASTVisitor<EarlyReturn>, public loplugin::Plugin
{
public:
    explicit EarlyReturn(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitIfStmt(const IfStmt * );
};

static bool endsWithReturnStmt(const Stmt * stmt)
{
    if (isa<ReturnStmt>(stmt))
        return true;

    auto compoundStmt = dyn_cast<CompoundStmt>(stmt);
    if (!compoundStmt)
        return false;
    if(compoundStmt->size() == 0)
        return false;

    return isa<ReturnStmt>(compoundStmt->body_back());
}

bool EarlyReturn::VisitIfStmt(const IfStmt* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    if (!ifStmt->getElse())
        return true;

    if (!endsWithReturnStmt(ifStmt->getThen()))
        return true;

    // ignore if/then/else/if chains for now
    if (isa<IfStmt>(ifStmt->getElse()))
        return true;

    // ignore if/then/else with return in both branches for now, not sure that it's an improvement to change it
    if (endsWithReturnStmt(ifStmt->getElse()))
        return true;

    // ignore if we are part of an if/then/else/if chain
    auto parentIfStmt = dyn_cast<IfStmt>(parentStmt(ifStmt));
    if (parentIfStmt && parentIfStmt->getElse() == ifStmt)
        return true;

    report(
        DiagnosticsEngine::Warning,
        "the then branch unconditionally returns, so no need for an else branch",
        ifStmt->getLocStart())
        << ifStmt->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< EarlyReturn > X("earlyreturn", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
