/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find places where a std::unique_ptr is release()'ed and returned as a raw
// pointer.  Some occurrences of that might better be rewritten to return the
// unique_ptr is returned directly.  (But other occurrences might be fine the
// way they are, hence place this plugin into store/).

#include "plugin.hxx"

namespace {

class ReturnUnique:
    public RecursiveASTVisitor<ReturnUnique>, public loplugin::Plugin
{
public:
    explicit ReturnUnique(InstantiationData const & data): Plugin(data) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitReturnStmt(ReturnStmt const * stmt);
};

bool ReturnUnique::VisitReturnStmt(ReturnStmt const * stmt) {
    if (ignoreLocation(stmt)) {
        return true;
    }
    auto const e1 = stmt->getRetValue();
    if (e1 == nullptr) {
        return true;
    }
    auto const e2 = dyn_cast<CXXMemberCallExpr>(e1->IgnoreParenImpCasts());
    if (e2 == nullptr) {
        return true;
    }
    auto const d1 = e2->getMethodDecl();
    if (d1 == nullptr) { // call via ptr to member
        return true;
    }
    auto const d2 = d1->getParent();
    assert(d2 != nullptr);
    assert(d2->getParent() != nullptr);
    auto const d3 = dyn_cast<NamespaceDecl>(d2->getParent());
    if (d3 == nullptr
        /* || dyn_cast<TranslationUnitDecl>(d3->getParent()) == nullptr */)
    {
        return true;
    }
    auto const id3 = d3->getIdentifier();
    if (id3 == nullptr /* || id3->getName() != "std" */) {
        return true;
    }
    auto const id2 = d2->getIdentifier();
    if (id2 == nullptr || id2->getName() != "unique_ptr") {
        return true;
    }
    auto const id1 = d1->getIdentifier();
    if (id1 == nullptr || id1->getName() != "release") {
        return true;
    }
    report(
        DiagnosticsEngine::Warning, "return std::unique_ptr::release",
        e2->getLocStart())
        << stmt->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<ReturnUnique> X("returnunique");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
