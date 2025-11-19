/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"
#include <vector>

/** Look for OUString/OString being appended to using "foo = foo +"
 */
namespace
{
class StringPlusEqual : public clang::RecursiveASTVisitor<StringPlusEqual>, public loplugin::Plugin
{
public:
    explicit StringPlusEqual(loplugin::InstantiationData const& rData)
        : Plugin(rData)
    {
    }

    void run() override;
    bool VisitCallExpr(CallExpr const*);
};

void StringPlusEqual::run() { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

bool StringPlusEqual::VisitCallExpr(CallExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(callExpr);
    if (!operatorCallExpr)
        return true;
    if (operatorCallExpr->getOperator() != OO_Equal)
        return true;

    if (auto memberExpr = dyn_cast<MemberExpr>(callExpr->getArg(0)))
    {
        auto tc = loplugin::TypeCheck(memberExpr->getType());
        if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
            return true;
        auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
        if (isInUnoIncludeFile(
                compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
            return true;
        if (ignoreLocation(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
            return true;
        auto rhs = dyn_cast<CXXConstructExpr>(callExpr->getArg(1)->IgnoreImplicit());
        if (!rhs || rhs->getNumArgs() == 0)
            return true;
        auto rhs2 = dyn_cast<CXXOperatorCallExpr>(rhs->getArg(0)->IgnoreImplicit());
        if (!rhs2)
            return true;
        if (rhs2->getOperator() != OO_Plus)
            return true;
        auto rhsMemberExpr = dyn_cast<MemberExpr>(rhs2->getArg(0)->IgnoreImplicit());
        if (!rhsMemberExpr)
            return true;
        if (rhsMemberExpr->getMemberDecl() != memberExpr->getMemberDecl())
            return true;
        report(DiagnosticsEngine::Warning, "rather use +=", operatorCallExpr->getBeginLoc())
            << operatorCallExpr->getSourceRange();
    }
    else if (auto declRefExpr = dyn_cast<DeclRefExpr>(callExpr->getArg(0)))
    {
        auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
        if (!varDecl)
            return true;
        auto tc = loplugin::TypeCheck(varDecl->getType());
        if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
            return true;
        auto rhs = dyn_cast<CXXConstructExpr>(callExpr->getArg(1)->IgnoreImplicit());
        if (!rhs || rhs->getNumArgs() == 0)
            return true;
        auto rhs2 = dyn_cast<CXXOperatorCallExpr>(rhs->getArg(0)->IgnoreImplicit());
        if (!rhs2)
            return true;
        if (rhs2->getOperator() != OO_Plus)
            return true;
        auto rhsDeclRef = dyn_cast<DeclRefExpr>(rhs2->getArg(0)->IgnoreImplicit());
        if (!rhsDeclRef)
            return true;
        if (rhsDeclRef->getDecl() != declRefExpr->getDecl())
            return true;
        report(DiagnosticsEngine::Warning, "rather use +=", operatorCallExpr->getBeginLoc())
            << operatorCallExpr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<StringPlusEqual> X("stringplusequal", true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
