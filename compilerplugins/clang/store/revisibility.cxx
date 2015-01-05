/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

// Find function declarations that redundantly re-specify a visibility attribute
// (via SAL_DLLPUBLIC_EXPORT etc.) that was already specified with a previous
// declaration of that function.  But MSVC wants consistency of __declspec
// across friend declarations, so just ignore those for now.

namespace {

bool hasExplicitVisibilityAttr(Decl const * decl) {
    VisibilityAttr const * attr = decl->getAttr<VisibilityAttr>();
    return attr != nullptr && !attr->isInherited();
}

bool isFriendDecl(Decl const * decl) {
    return decl->getFriendObjectKind() != Decl::FOK_None;
}

class ReVisibility:
    public RecursiveASTVisitor<ReVisibility>, public loplugin::Plugin
{
public:
    explicit ReVisibility(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const * decl);
};

bool ReVisibility::VisitFunctionDecl(FunctionDecl const * decl) {
    if (!ignoreLocation(decl) && hasExplicitVisibilityAttr(decl)
        && !isFriendDecl(decl))
    {
        for (Decl const * p = decl;;) {
            p = p->getPreviousDecl();
            if (p == nullptr) {
                break;
            }
            if (hasExplicitVisibilityAttr(p) && !isFriendDecl(p)) {
                report(
                    DiagnosticsEngine::Warning,
                    "Redundant visibility re-declaration",
                    decl->getAttr<VisibilityAttr>()->getLocation())
                    << decl->getAttr<VisibilityAttr>()->getRange();
                report(
                    DiagnosticsEngine::Note,
                    "Previous visibility declaration is here",
                    p->getAttr<VisibilityAttr>()->getLocation())
                    << p->getAttr<VisibilityAttr>()->getRange();
                break;
            }
        }
    }
    return true;
}

loplugin::Plugin::Registration<ReVisibility> X("revisibility");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
