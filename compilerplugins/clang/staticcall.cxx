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

class StaticCall:
    public RecursiveASTVisitor<StaticCall>, public loplugin::Plugin
{
public:
    explicit StaticCall(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr const * expr);
};

bool StaticCall::VisitCallExpr(CallExpr const * expr) {
    if (ignoreLocation(expr)
        || !isa<MemberExpr>(expr->getCallee()->IgnoreImpCasts()))
    {
        return true;
    }
    CXXMethodDecl const * decl = dyn_cast_or_null<CXXMethodDecl>(
        expr->getDirectCallee());
    if (decl != nullptr && decl->isStatic()) {
        report(
            DiagnosticsEngine::Warning,
            ("calling static member function through member call syntax, use"
             " '%0' instead"),
            expr->getLocStart())
            << decl->getQualifiedNameAsString() << expr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<StaticCall> X("staticcall");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
