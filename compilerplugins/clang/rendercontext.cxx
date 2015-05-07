/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

// Check for calls to OutputDevice methods that are not passing through RenderContext

namespace {

class RenderContext:
    public RecursiveASTVisitor<RenderContext>, public loplugin::Plugin
{
public:
    explicit RenderContext(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseFunctionDecl(const FunctionDecl * decl);

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *);

private:
    bool        mbChecking = false;
};

bool RenderContext::TraverseFunctionDecl(const FunctionDecl * pFunctionDecl) {
    if (ignoreLocation(pFunctionDecl)) {
        return true;
    }
    if (!pFunctionDecl->hasBody()) {
        return true;
    }
    if ( pFunctionDecl != pFunctionDecl->getCanonicalDecl() ) {
        return true;
    }
    const CXXMethodDecl *pCXXMethodDecl = dyn_cast<CXXMethodDecl>(pFunctionDecl);
    if (pCXXMethodDecl) {
        std::string aParentName = pCXXMethodDecl->getParent()->getQualifiedNameAsString();
        if (aParentName == "OutputDevice")
            return true;
    }
    mbChecking = true;
    TraverseStmt(pFunctionDecl->getBody());
    mbChecking = false;
    return true;
}

bool RenderContext::VisitCXXMemberCallExpr(const CXXMemberCallExpr* pCXXMemberCallExpr)
{
    if (!mbChecking)
        return true;
    if (ignoreLocation(pCXXMemberCallExpr)) {
        return true;
    }
    const CXXRecordDecl *pCXXRecordDecl = pCXXMemberCallExpr->getRecordDecl();
    if (pCXXRecordDecl->getQualifiedNameAsString() != "OutputDevice") {
        return true;
    }
    const ImplicitCastExpr *pImplicitCastExpr = dyn_cast<ImplicitCastExpr>(pCXXMemberCallExpr->getImplicitObjectArgument());
    std::string t2 = "0";
    if (pImplicitCastExpr) {
        t2 = "2";
        QualType aType = pImplicitCastExpr->getSubExpr()->getType();
        if (aType->isPointerType())
            aType = aType->getPointeeType();
        t2 = aType.getAsString();
        if (t2 == "vcl::RenderContext")
            return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "Should be calling OutputDevice method through RenderContext.",
        pCXXMemberCallExpr->getLocStart())
      << pCXXMemberCallExpr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< RenderContext > X("rendercontext", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
