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
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>

#include "check.hxx"
#include "plugin.hxx"

/**
 * Look for a mistake I made (a lot) at one point where we assign a reference to a reference var, which
 * does not do at all what I thought.
 */

namespace
{
class RefAssign : public loplugin::FilteringPlugin<RefAssign>
{
public:
    explicit RefAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/comphelper/source/misc/syntaxhighlight.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/numbers/zformat.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/tools/source/memtools/multisel.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/tools/source/generic/point.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/edit.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/fmtfield.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/field.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/field2.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/edit/textview.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/edit/vclmedit.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/editeng/editdoc.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/editeng/impedit2.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/svxruler.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/ribbar/inputwin.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/text/txtftn.cxx"))
            return false;

        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(BinaryOperator const*);
    // causes crashes in CallExpr::getReturnType
    bool TraverseFunctionTemplateDecl(FunctionTemplateDecl*) { return true; }
    bool TraverseClassTemplateDecl(ClassTemplateDecl*) { return true; }

private:
    clang::QualType ExtractType(Expr const*);
};

bool RefAssign::VisitBinaryOperator(BinaryOperator const* binaryOp)
{
    if (ignoreLocation(binaryOp))
        return true;
    if (binaryOp->getOpcode() != BO_Assign)
        return true;

    // ignore assigning to/from an element of a collection
    if (isa<CXXOperatorCallExpr>(binaryOp->getLHS()->IgnoreParenImpCasts()))
        return true;
    if (isa<CXXOperatorCallExpr>(binaryOp->getRHS()->IgnoreParenImpCasts()))
        return true;

    // if we are assigning to a parameter we probably mean it
    if (auto declRefExpr = dyn_cast<DeclRefExpr>(binaryOp->getLHS()->IgnoreParenImpCasts()))
        if (declRefExpr->getDecl() && isa<ParmVarDecl>(declRefExpr->getDecl()))
            return true;

    if (auto callExpr = dyn_cast<CallExpr>(binaryOp->getRHS()->IgnoreParenImpCasts()))
        if (auto functionDecl = dyn_cast_or_null<FunctionDecl>(callExpr->getCalleeDecl()))
            if (functionDecl->getIdentifier()
                && (functionDecl->getName() == "min" || functionDecl->getName() == "max"))
                return true;

    auto lhsType = ExtractType(binaryOp->getLHS());
    auto rhsType = ExtractType(binaryOp->getRHS());
    if (!loplugin::TypeCheck(lhsType).LvalueReference())
        return true;
    if (!loplugin::TypeCheck(rhsType).LvalueReference())
        return true;
    binaryOp->dump();
    report(DiagnosticsEngine::Warning,
           "assigning a %0 to a var of type %1 probably does not do what you think",
           binaryOp->getBeginLoc())
        << rhsType << lhsType << binaryOp->getSourceRange();
    return true;
}

clang::QualType RefAssign::ExtractType(Expr const* expr)
{
    expr = expr->IgnoreParenImpCasts();
    if (auto declReflExpr = dyn_cast<DeclRefExpr>(expr))
    {
        if (auto varDecl = dyn_cast<VarDecl>(declReflExpr->getDecl()))
            return varDecl->getType();
    }
    else if (auto callExpr = dyn_cast<CallExpr>(expr))
    {
        if (callExpr->isTypeDependent())
            return {};
        //        callExpr->dump();
        return callExpr->getCallReturnType(compiler.getASTContext());
    }
    return expr->getType();
}

loplugin::Plugin::Registration<RefAssign> refassign("refassign");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
