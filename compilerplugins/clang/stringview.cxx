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

    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitImplicitCastExpr(ImplicitCastExpr const*);

private:
    void handleCXXConstructExpr(CXXConstructExpr const* expr);
    void handleCXXMemberCallExpr(CXXMemberCallExpr const* expr);
};

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

bool StringView::VisitImplicitCastExpr(ImplicitCastExpr const* expr)
{
    if (ignoreLocation(expr))
    {
        return true;
    }
    if (!loplugin::TypeCheck(expr->getType()).ClassOrStruct("basic_string_view").StdNamespace())
    {
        return true;
    }
    auto const e = expr->getSubExprAsWritten()->IgnoreParens();
    auto const tc = loplugin::TypeCheck(e->getType());
    if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
          || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
    {
        return true;
    }
    if (auto const e1 = dyn_cast<CXXConstructExpr>(e))
    {
        handleCXXConstructExpr(e1);
    }
    else if (auto const e2 = dyn_cast<CXXMemberCallExpr>(e))
    {
        handleCXXMemberCallExpr(e2);
    }
    return true;
}

void StringView::handleCXXConstructExpr(CXXConstructExpr const* expr)
{
    if (expr->getNumArgs() != 0)
    {
        return;
    }
    report(DiagnosticsEngine::Warning,
           "instead of an empty %0, pass an empty '%select{std::string_view|std::u16string_view}1'",
           expr->getExprLoc())
        << expr->getType()
        << (loplugin::TypeCheck(expr->getType()).Class("OString").Namespace("rtl").GlobalNamespace()
                ? 0
                : 1)
        << expr->getSourceRange();
}

void StringView::handleCXXMemberCallExpr(CXXMemberCallExpr const* expr)
{
    auto const dc = loplugin::DeclCheck(expr->getMethodDecl()).Function("copy");
    if (!dc)
    {
        return;
    }
    if (!(dc.Class("OString").Namespace("rtl").GlobalNamespace()
          || dc.Class("OUString").Namespace("rtl").GlobalNamespace()))
    {
        return;
    }
    report(DiagnosticsEngine::Warning, "rather than copy, pass with a view using subView()",
           expr->getExprLoc())
        << expr->getSourceRange();
}

loplugin::Plugin::Registration<StringView> stringview("stringview");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
