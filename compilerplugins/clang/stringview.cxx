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
        auto const fn = handler.getMainFileName();
        return !(
            loplugin::isSamePathname(fn, SRCDIR "/sal/qa/OStringBuffer/rtl_OStringBuffer.cxx")
            || loplugin::isSamePathname(fn, SRCDIR "/sal/qa/rtl/strings/test_ostring_concat.cxx")
            || loplugin::isSamePathname(fn, SRCDIR "/sal/qa/rtl/strings/test_oustring_concat.cxx"));
    }

    virtual void run() override
    {
        if (!preRun())
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitImplicitCastExpr(ImplicitCastExpr const*);
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);
    bool VisitCXXConstructExpr(CXXConstructExpr const*);

private:
    void handleSubExprThatCouldBeView(Expr const* expr);
    void handleCXXConstructExpr(CXXConstructExpr const* expr);
    void handleCXXMemberCallExpr(CXXMemberCallExpr const* expr);
};

bool StringView::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOperatorCallExpr)
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;

    auto op = cxxOperatorCallExpr->getOperator();
    if (op == OO_Plus && cxxOperatorCallExpr->getNumArgs() == 2)
    {
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(0));
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(1));
    }
    if (compat::isComparisonOp(cxxOperatorCallExpr))
    {
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(0));
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(1));
    }
    else if (op == OO_PlusEqual)
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(1));
    else if (op == OO_Subscript)
        handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(0));
    else if (op == OO_Equal)
    {
        if (loplugin::TypeCheck(cxxOperatorCallExpr->getType())
                .Class("OUStringBuffer")
                .Namespace("rtl")
                .GlobalNamespace()
            || loplugin::TypeCheck(cxxOperatorCallExpr->getType())
                   .Class("OStringBuffer")
                   .Namespace("rtl")
                   .GlobalNamespace())
        {
            handleSubExprThatCouldBeView(cxxOperatorCallExpr->getArg(1));
        }
    }
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
    handleSubExprThatCouldBeView(expr->getSubExprAsWritten());
    return true;
}

void StringView::handleSubExprThatCouldBeView(Expr const* subExpr)
{
    auto const e0 = compat::IgnoreImplicit(subExpr);
    auto const e = e0->IgnoreParens();
    auto const tc = loplugin::TypeCheck(e->getType());
    if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
          || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
    {
        return;
    }
    if (auto const e1 = dyn_cast<CXXConstructExpr>(e))
    {
        if (e0 == subExpr)
        {
            handleCXXConstructExpr(e1);
        }
    }
    else if (auto const e2 = dyn_cast<CXXFunctionalCastExpr>(e))
    {
        auto e3 = e2->getSubExpr();
        if (auto const e4 = dyn_cast<CXXBindTemporaryExpr>(e3))
        {
            e3 = e4->getSubExpr();
        }
        if (auto const e4 = dyn_cast<CXXConstructExpr>(e3))
        {
            handleCXXConstructExpr(e4);
        }
    }
    else if (auto const e3 = dyn_cast<CXXMemberCallExpr>(e))
    {
        handleCXXMemberCallExpr(e3);
    }
}

void StringView::handleCXXConstructExpr(CXXConstructExpr const* expr)
{
    QualType argType;
    enum
    {
        None,
        OrChar,
        ViaConcatenation
    } extra
        = None;
    auto const d = expr->getConstructor();
    switch (d->getNumParams())
    {
        case 0:
            break;
        case 1:
        {
            auto const t = d->getParamDecl(0)->getType();
            if (t->isAnyCharacterType())
            {
                argType = expr->getArg(0)->IgnoreImplicit()->getType();
                extra = OrChar;
                break;
            }
            loplugin::TypeCheck tc(t);
            if (tc.LvalueReference()
                    .Const()
                    .Class("OStringLiteral")
                    .Namespace("rtl")
                    .GlobalNamespace()
                || tc.LvalueReference()
                       .Const()
                       .Class("OUStringLiteral")
                       .Namespace("rtl")
                       .GlobalNamespace()
                || tc.RvalueReference().Struct("OStringNumber").Namespace("rtl").GlobalNamespace()
                || tc.RvalueReference().Struct("OUStringNumber").Namespace("rtl").GlobalNamespace()
                || tc.ClassOrStruct("basic_string_view").StdNamespace())
            {
                argType = expr->getArg(0)->IgnoreImplicit()->getType();
                break;
            }
            if (tc.RvalueReference().Struct("OStringConcat").Namespace("rtl").GlobalNamespace()
                || tc.RvalueReference().Struct("OUStringConcat").Namespace("rtl").GlobalNamespace())
            {
                argType = expr->getArg(0)->IgnoreImplicit()->getType();
                extra = ViaConcatenation;
                break;
            }
            return;
        }
        case 2:
        {
            auto const t0 = d->getParamDecl(0)->getType();
            if (t0->isPointerType() && t0->getPointeeType()->isAnyCharacterType())
            {
                auto const t = d->getParamDecl(1)->getType();
                if (t->isIntegralType(compiler.getASTContext())
                    && !(t->isBooleanType() || t->isAnyCharacterType()))
                {
                    auto const arg = expr->getArg(1);
                    if (!arg->isValueDependent())
                    {
                        if (auto const val
                            = compat::getIntegerConstantExpr(arg, compiler.getASTContext()))
                        {
                            if (val->getExtValue() == 1)
                            {
                                extra = OrChar;
                            }
                        }
                    }
                    argType = expr->getArg(0)->IgnoreImplicit()->getType();
                    break;
                }
            }
            if (loplugin::TypeCheck(d->getParamDecl(1)->getType())
                    .Struct("Dummy")
                    .Namespace("libreoffice_internal")
                    .Namespace("rtl")
                    .GlobalNamespace())
            {
                argType = expr->getArg(0)->IgnoreImplicit()->getType();
                break;
            }
            return;
        }
        default:
            return;
    }
    report(DiagnosticsEngine::Warning,
           "instead of an %0%select{| constructed from a %2}1, pass a"
           " '%select{std::string_view|std::u16string_view}3'"
           "%select{| (or an '%select{rtl::OStringChar|rtl::OUStringChar}3')|"
           " via '%select{rtl::OStringConcatenation|rtl::OUStringConcatenation}3'}4",
           expr->getExprLoc())
        << expr->getType() << (argType.isNull() ? 0 : 1) << argType
        << (loplugin::TypeCheck(expr->getType()).Class("OString").Namespace("rtl").GlobalNamespace()
                ? 0
                : 1)
        << extra << expr->getSourceRange();
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

/** check for calls to O[U]StringBuffer::append that could be passed as a
    std::u16string_view */
bool StringView::VisitCXXMemberCallExpr(CXXMemberCallExpr const* expr)
{
    if (ignoreLocation(expr))
    {
        return true;
    }
    if (!loplugin::TypeCheck(expr->getType())
             .Class("OUStringBuffer")
             .Namespace("rtl")
             .GlobalNamespace()
        && !loplugin::TypeCheck(expr->getType())
                .Class("OStringBuffer")
                .Namespace("rtl")
                .GlobalNamespace())
    {
        return true;
    }
    auto const dc = loplugin::DeclCheck(expr->getMethodDecl());
    if (dc.Function("append") || dc.Function("indexOf") || dc.Function("lastIndexOf"))
    {
        handleSubExprThatCouldBeView(expr->getArg(0));
    }
    else if (dc.Function("insert"))
    {
        handleSubExprThatCouldBeView(expr->getArg(1));
    }
    return true;
}

/** check for calls to O[U]StringBuffer constructor that could be passed as a
    std::u16string_view */
bool StringView::VisitCXXConstructExpr(CXXConstructExpr const* expr)
{
    if (ignoreLocation(expr))
    {
        return true;
    }
    if (!loplugin::TypeCheck(expr->getType())
             .Class("OUStringBuffer")
             .Namespace("rtl")
             .GlobalNamespace()
        && !loplugin::TypeCheck(expr->getType())
                .Class("OStringBuffer")
                .Namespace("rtl")
                .GlobalNamespace())
    {
        return true;
    }
    if (expr->getNumArgs() > 0)
        handleSubExprThatCouldBeView(expr->getArg(0));
    return true;
}

loplugin::Plugin::Registration<StringView> stringview("stringview");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
