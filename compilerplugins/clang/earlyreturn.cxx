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

bool EarlyReturn::VisitIfStmt(const IfStmt* ifStatement)
{
    if (ignoreLocation(ifStatement))
        return true;
    if (isInUnoIncludeFile(ifStatement->getLocStart()))
        return true;

    if (!ifStatement->getElse())
        return true;

    auto compoundStmt = dyn_cast<CompoundStmt>(ifStatement->getThen());
    if (!compoundStmt)
        return true;
    if(compoundStmt->size() == 0)
        return true;

    auto returnStmt = dyn_cast<ReturnStmt>(compoundStmt->body_back());
    if (!returnStmt)
        return true;

    // ignore if/then/else/if for now
    if (isa<IfStmt>(ifStatement->getElse()))
        return true;

    // ignore if/then/else with return in both paths for now, not sure that it's an improvement to change it
    if (isa<ReturnStmt>(ifStatement->getElse()))
        return true;
    compoundStmt = dyn_cast<CompoundStmt>(ifStatement->getElse());
    if (compoundStmt && compoundStmt->size()>0 && isa<ReturnStmt>(compoundStmt->body_back()))
        return true;

    report(
        DiagnosticsEngine::Warning,
        "since this if statement unconditionally returns, no need for an else statement",
        ifStatement->getLocStart())
        << ifStatement->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< EarlyReturn > X("earlyreturn", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
