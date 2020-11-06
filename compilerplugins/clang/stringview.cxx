/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    Look for places where we are making a substring copy of an OUString, and then passing it to a
    function that takes a u16string_view, in which case it is more efficient to pass a view
    of the OUString, rather than making a copy.

    TODO currently does not check if there is some other visible overload of the callee, that can take
    a string_view.
    TODO handle OUStringBuffer/OStringBuffer similarly
*/

namespace
{
class StringView : public loplugin::FilteringPlugin<StringView>
{
public:
    explicit StringView(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        //        std::string fn(handler.getMainFileName());
        //       loplugin::normalizeDotDotInFilePath(fn);
        //        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/OStringBuffer/"))
        //            return false;
        //        // there is an ifdef here, but my check is not working, not sure why
        //        if (fn == SRCDIR "/pyuno/source/module/pyuno_runtime.cxx")
        //            return false;
        //        // TODO the += depends on the result of the preceding assign, so can't merge
        //        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
        //            return false;
        return true;
    }

    virtual void run() override
    {
        if (!preRun())
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCallExpr(CallExpr const*);
    bool VisitCXXConstructExpr(CXXConstructExpr const*);
    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
};

bool StringView::VisitCallExpr(CallExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;

    const FunctionDecl* functionDecl;
    if (isa<CXXMemberCallExpr>(callExpr))
        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    else
        functionDecl = callExpr->getDirectCallee();
    if (!functionDecl)
        return true;

    unsigned len = std::min(callExpr->getNumArgs(), functionDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        const Expr* argExpr = callExpr->getArg(i);
        auto paramDecl = functionDecl->getParamDecl(i);
        auto tc = loplugin::TypeCheck(paramDecl->getType());
        if (!tc.Typedef("u16string_view").StdNamespace()
            && !tc.Typedef("string_view").StdNamespace())
            continue;
        // unwrap the operator that converts to std::u16string_view
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(argExpr->IgnoreImpCasts());
        if (!memberCallExpr)
            continue;
        // unwrap the call to copy()
        auto memberCallExpr2 = dyn_cast<CXXMemberCallExpr>(
            memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit());
        if (!memberCallExpr2)
            continue;
        if (memberCallExpr2->getMethodDecl()->getName() != "copy")
            continue;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using copyView()",
               compat::getBeginLoc(argExpr))
            << argExpr->getSourceRange();
    }

    return true;
}

bool StringView::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOperatorCallExpr)
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;

    auto op = cxxOperatorCallExpr->getOperator();
    if (op != OO_Plus)
        return true;

    auto check = [&](const Expr* expr) -> void {
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(expr->IgnoreImplicit());
        if (!memberCallExpr)
            return;
        if (memberCallExpr->getMethodDecl()->getName() != "copy")
            return;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using copyView()",
               compat::getBeginLoc(expr))
            << expr->getSourceRange();
    };
    check(cxxOperatorCallExpr->getArg(0));
    check(cxxOperatorCallExpr->getArg(1));
    return true;
}

bool StringView::VisitFunctionDecl(FunctionDecl const* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    if (functionDecl->getIdentifier() && functionDecl->getName() == "f2")
        functionDecl->dump();
    return true;
}

bool StringView::VisitCXXConstructExpr(CXXConstructExpr const* constructExpr)
{
    if (ignoreLocation(constructExpr))
        return true;

    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    if (!constructorDecl)
        return true;

    unsigned len = std::min(constructExpr->getNumArgs(), constructorDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        const Expr* argExpr = constructExpr->getArg(i);
        auto paramDecl = constructorDecl->getParamDecl(i);
        auto tc = loplugin::TypeCheck(paramDecl->getType());
        if (!tc.Typedef("u16string_view").StdNamespace()
            && !tc.Typedef("string_view").StdNamespace())
            continue;
        // unwrap the operator that converts to std::u16string_view
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(argExpr->IgnoreImpCasts());
        if (!memberCallExpr)
            continue;
        // unwrap the call to copy()
        auto memberCallExpr2 = dyn_cast<CXXMemberCallExpr>(
            memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit());
        if (!memberCallExpr2)
            continue;
        if (memberCallExpr2->getMethodDecl()->getName() != "copy")
            continue;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using copyView()",
               compat::getBeginLoc(argExpr))
            << argExpr->getSourceRange();
    }

    return true;
}

loplugin::Plugin::Registration<StringView> stringview("stringview");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
