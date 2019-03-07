/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

// Find occurrences of 'new T()' where the instance is zero-initialized upfront
// since C++11.  For one, in many cases this may be unnecessary and unintended,
// as the code was written before C++11.  For another, the zero-initialization
// would go away when T gets a user-provided default constructor, for example,
// so better make any necessary initialization more explicit in the code.

namespace {

class SubtleZeroInit final:
    public loplugin::FilteringPlugin<SubtleZeroInit>
{
public:
    explicit SubtleZeroInit(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    bool VisitCXXNewExpr(CXXNewExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto ce = expr->getConstructExpr();
        if (ce == nullptr) {
            return true;
        }
        if (!ce->requiresZeroInitialization()) {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            ("if zero-initialization of %0 is intentional here, better make"
             " that more explicit (e.g., assigning to members, default"
             " constructor, default member initializers, std::memset)"),
            expr->getExprLoc())
            << ce->getType() << expr->getSourceRange();
        return true;
    }

    virtual bool preRun() override {
        return compiler.getLangOpts().CPlusPlus;
    }

    virtual void run() override {
        if (preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<SubtleZeroInit> subtlezeroinit("subtlezeroinit");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
