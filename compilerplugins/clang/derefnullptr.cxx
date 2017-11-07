/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

class DerefNullPtr:
    public RecursiveASTVisitor<DerefNullPtr>, public loplugin::Plugin
{
public:
    explicit DerefNullPtr(loplugin::InstantiationData const & data):
        Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitUnaryDeref(UnaryOperator const * op);
};

bool DerefNullPtr::VisitUnaryDeref(UnaryOperator const * op) {
    if (!ignoreLocation(op)
        && (op->getSubExpr()->IgnoreParenCasts()->isNullPointerConstant(
                compiler.getASTContext(), Expr::NPC_ValueDependentIsNotNull/*TODO*/)
            != Expr::NPCK_NotNull))
    {
        report(
            DiagnosticsEngine::Warning, "null pointer dereference",
            op->getLocStart())
            << op->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<DerefNullPtr> X("derefnullptr");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
