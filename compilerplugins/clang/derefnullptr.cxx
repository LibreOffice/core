/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

namespace {

class DerefNullPtr:
    public loplugin::FilteringPlugin<DerefNullPtr>
{
public:
    explicit DerefNullPtr(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

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
            compat::getBeginLoc(op))
            << op->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<DerefNullPtr> derefnullptr("derefnullptr");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
