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

    bool preRun() override { return true; }

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
        auto paramRecordDecl = dyn_cast_or_null<RecordDecl>(
            paramDecl->getType()->getUnqualifiedDesugaredType()->getAsTagDecl());
        if (!paramRecordDecl || !paramRecordDecl->getIdentifier()
            || paramRecordDecl->getName() != "basic_string_view")
            continue;
        // unwrap the operator that converts to std::u16string_view
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(argExpr->IgnoreImpCasts());
        if (!memberCallExpr)
            continue;
        // unwrap the call to copy()
        auto memberCallExpr2 = dyn_cast<CXXMemberCallExpr>(
            compat::IgnoreImplicit(memberCallExpr->getImplicitObjectArgument()));
        if (!memberCallExpr2)
            continue;
        auto methodDecl = memberCallExpr2->getMethodDecl();
        if (!methodDecl->getIdentifier() || methodDecl->getName() != "copy")
            continue;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using subView()",
               compat::getBeginLoc(argExpr))
            << argExpr->getSourceRange();
    }

    return true;
}

bool StringView::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOperatorCallExpr)
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;

    auto check = [&](const Expr* expr) -> void {
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(compat::IgnoreImplicit(expr));
        if (!memberCallExpr)
            return;
        auto methodDecl = memberCallExpr->getMethodDecl();
        if (!methodDecl->getIdentifier() || methodDecl->getName() != "copy")
            return;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using subView()",
               compat::getBeginLoc(expr))
            << expr->getSourceRange();
    };
    auto op = cxxOperatorCallExpr->getOperator();
    if (op == OO_Plus && cxxOperatorCallExpr->getNumArgs() == 2)
    {
        check(cxxOperatorCallExpr->getArg(0));
        check(cxxOperatorCallExpr->getArg(1));
    }
    if (compat::isComparisonOp(cxxOperatorCallExpr))
    {
        check(cxxOperatorCallExpr->getArg(0));
        check(cxxOperatorCallExpr->getArg(1));
    }
    else if (op == OO_PlusEqual)
        check(cxxOperatorCallExpr->getArg(1));
    else if (op == OO_Subscript)
        check(cxxOperatorCallExpr->getArg(0));
    return true;
}

bool StringView::VisitFunctionDecl(FunctionDecl const* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    // debugging
    // if (functionDecl->getIdentifier() && functionDecl->getName() == "f1")
    //     functionDecl->dump();
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
        auto paramRecordDecl = dyn_cast_or_null<RecordDecl>(
            paramDecl->getType()->getUnqualifiedDesugaredType()->getAsTagDecl());
        if (!paramRecordDecl || !paramRecordDecl->getIdentifier()
            || paramRecordDecl->getName() != "basic_string_view")
            continue;
        // unwrap the operator that converts to std::u16string_view
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(argExpr->IgnoreImpCasts());
        if (!memberCallExpr)
            continue;
        // unwrap the call to copy()
        auto memberCallExpr2 = dyn_cast<CXXMemberCallExpr>(
            compat::IgnoreImplicit(memberCallExpr->getImplicitObjectArgument()));
        if (!memberCallExpr2)
            continue;
        auto methodDecl = memberCallExpr2->getMethodDecl();
        if (!methodDecl->getIdentifier() || methodDecl->getName() != "copy")
            continue;
        report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using subView()",
               compat::getBeginLoc(argExpr))
            << argExpr->getSourceRange();
    }

    return true;
}

loplugin::Plugin::Registration<StringView> stringview("stringview");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
