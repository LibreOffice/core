/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace {

class RedundantCopy final:
    public RecursiveASTVisitor<RedundantCopy>, public loplugin::Plugin
{
public:
    explicit RedundantCopy(loplugin::InstantiationData const & data):
        Plugin(data) {}

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto const t1 = expr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();
        if (t1.getCanonicalType().getTypePtr()
            != t2.getCanonicalType().getTypePtr())
        {
            return true;
        }
        auto tc = loplugin::TypeCheck(t1);
        if (!(tc.Class("OUString").Namespace("rtl").GlobalNamespace()
              || tc.Class("unique_ptr").StdNamespace()))
        {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            "redundant copy construction from %0 to %1", expr->getExprLoc())
            << t2 << t1 << expr->getSourceRange();
        return true;
    }

private:
    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<RedundantCopy> reg("redundantcopy");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
