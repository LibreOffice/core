/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
class RedundantFCast final : public RecursiveASTVisitor<RedundantFCast>, public loplugin::Plugin
{
public:
    explicit RedundantFCast(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    /* Check for the creation of unnecessary temporaries when calling a method that takes a param by const & */
    bool VisitCallExpr(CallExpr const* callExpr)
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
            // check if param is const&
            auto param = functionDecl->getParamDecl(i);
            auto lvalueType = param->getType()->getAs<LValueReferenceType>();
            if (!lvalueType)
                continue;
            if (!lvalueType->getPointeeType().isConstQualified())
                continue;
            auto paramClassOrStructType = lvalueType->getPointeeType()->getAs<RecordType>();
            if (!paramClassOrStructType)
                continue;
            // check for temporary and functional cast in argument expression
            auto arg = callExpr->getArg(i)->IgnoreImpCasts();
            auto materializeTemporaryExpr = dyn_cast<MaterializeTemporaryExpr>(arg);
            if (!materializeTemporaryExpr)
                continue;
            auto functionalCast = dyn_cast<CXXFunctionalCastExpr>(
                materializeTemporaryExpr->GetTemporaryExpr()->IgnoreImpCasts());
            if (!functionalCast)
                continue;
            auto const t1 = functionalCast->getTypeAsWritten();
            auto const t2 = compat::getSubExprAsWritten(functionalCast)->getType();
            if (t1.getCanonicalType().getTypePtr() != t2.getCanonicalType().getTypePtr())
                continue;
            // Check that the underlying expression is of the same class/struct type as the param i.e. that we are not instantiating
            // something useful
            if (t1.getCanonicalType().getTypePtr() != paramClassOrStructType)
                continue;

            report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
                   arg->getExprLoc())
                << t2 << t1 << arg->getSourceRange();
            report(DiagnosticsEngine::Note, "in call to method here", param->getLocation())
                << param->getSourceRange();
        }
        return true;
    }

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        auto const t1 = expr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();
        if (t1.getCanonicalType().getTypePtr() != t2.getCanonicalType().getTypePtr())
        {
            return true;
        }
        auto tc = loplugin::TypeCheck(t1);
        if (!(tc.Class("OUString").Namespace("rtl").GlobalNamespace()
              || tc.Class("Color").GlobalNamespace() || tc.Class("unique_ptr").StdNamespace()))
        {
            return true;
        }
        report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
               expr->getExprLoc())
            << t2 << t1 << expr->getSourceRange();
        return true;
    }

private:
    void run() override
    {
        if (compiler.getLangOpts().CPlusPlus)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<RedundantFCast> reg("redundantfcast");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
