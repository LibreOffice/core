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
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"

// Check for calls to OutputDevice methods that are not passing through RenderContext

namespace
{

class RenderContext:
    public RecursiveASTVisitor<RenderContext>, public loplugin::Plugin
{
public:
    explicit RenderContext(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseFunctionDecl(const FunctionDecl * decl);

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *);

private:
    bool        mbChecking = false;
};

// We use Traverse to set a flag so we can easily ignore certain method calls
bool RenderContext::TraverseFunctionDecl(const FunctionDecl * pFunctionDecl)
{
    if (ignoreLocation(pFunctionDecl)) {
        return true;
    }
    if (!pFunctionDecl->hasBody()) {
        return true;
    }
    if ( pFunctionDecl != pFunctionDecl->getCanonicalDecl() ) {
        return true;
    }
    // Ignore methods inside the OutputDevice class
    const CXXMethodDecl *pCXXMethodDecl = dyn_cast<CXXMethodDecl>(pFunctionDecl);
    if (pCXXMethodDecl) {
        if (loplugin::TypeCheck(pCXXMethodDecl->getParent()).Class("OutputDevice").GlobalNamespace())
            return true;
    }
    // we are only currently interested in methods where the first parameter is RenderContext
    if (pFunctionDecl->getNumParams() == 0)
        return true;
    if ( loplugin::TypeCheck(pFunctionDecl->getParamDecl( 0 )->getType()).Class("RenderContext").GlobalNamespace() ) {
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
    if (!loplugin::TypeCheck(pCXXRecordDecl).Class("OutputDevice").GlobalNamespace()) {
        return true;
    }
    // ignore a handful of methods. They will most probably still be present in Window for use during processing outside of the Paint()
    // method lifecycle
    const CXXMethodDecl *pCXXMethodDecl = pCXXMemberCallExpr->getMethodDecl();
    if (pCXXMethodDecl->isInstance()) {
        StringRef name = pCXXMethodDecl->getName();
        if (name == "LogicToPixel" || name == "GetMapMode" || name == "GetFontMetric" || name == "LogicToLogic"
            || name == "PixelToLogic" || name == "SetDigitLanguage")
        {
            return true;
        }
    }
    // for calling through a pointer
    const ImplicitCastExpr *pImplicitCastExpr = dyn_cast<ImplicitCastExpr>(pCXXMemberCallExpr->getImplicitObjectArgument());
    if (pImplicitCastExpr) {
        QualType aType = pImplicitCastExpr->getSubExpr()->getType();
        if (aType->isPointerType())
            aType = aType->getPointeeType();
        std::string t2 = aType.getAsString();
        if (t2 == "vcl::RenderContext" || t2 == "const vcl::RenderContext")
            return true;
    }
    // for calling through a reference
    const DeclRefExpr *pDeclRefExpr = dyn_cast<DeclRefExpr>(pCXXMemberCallExpr->getImplicitObjectArgument());
    if (pDeclRefExpr) {
        QualType aType = pDeclRefExpr->getType();
        std::string t2 = aType.getAsString();
        if (t2 == "vcl::RenderContext" || t2 == "const vcl::RenderContext")
            return true;
    }
    // for calling through a chain of methods
    const CXXMemberCallExpr *pMemberExpr = dyn_cast<CXXMemberCallExpr>(pCXXMemberCallExpr->getImplicitObjectArgument());
    if (pMemberExpr) {
        QualType aType = pMemberExpr->getType();
        if (aType->isPointerType())
            aType = aType->getPointeeType();
        std::string t2 = aType.getAsString();
        if (t2 == "vcl::RenderContext" || t2 == "const vcl::RenderContext")
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
