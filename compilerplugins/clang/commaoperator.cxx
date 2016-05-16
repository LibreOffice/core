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
#include "compat.hxx"

/**
the comma operator is best used sparingly
*/

namespace {

class CommaOperator:
    public RecursiveASTVisitor<CommaOperator>, public loplugin::Plugin
{
public:
    explicit CommaOperator(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator* );
};

bool CommaOperator::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (binaryOp->getOpcode() != BO_Comma) {
        return true;
    }
    const Stmt* parent = parentStmt(binaryOp);
    if (parent != nullptr) {
        if (isa<ParenExpr>(parent)) {
            return true;
        }
        if (isa<BinaryOperator>(parent)) {
            return true;
        }
        if (isa<ForStmt>(parent)) {
            return true;
        }
        if (isa<ExprWithCleanups>(parent)) {
            const Stmt* parent2 = parentStmt(parent);
            if (isa<ForStmt>(parent2)) {
                return true;
            }
        }
    }
//    parent->dump();
    report(
        DiagnosticsEngine::Warning, "comma operator hides code",
        binaryOp->getSourceRange().getBegin())
      << binaryOp->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< CommaOperator > X("commaoperator", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
