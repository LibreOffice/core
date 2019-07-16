/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"
#include <iostream>
#include <fstream>

namespace
{
class RedundantFCast final : public loplugin::FilteringPlugin<RedundantFCast>
{
public:
    explicit RedundantFCast(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitReturnStmt(ReturnStmt const* returnStmt)
    {
        if (ignoreLocation(returnStmt))
            return true;
        Expr const* expr = returnStmt->getRetValue();
        if (!expr)
            return true;
        if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(expr))
            expr = exprWithCleanups->getSubExpr();
        if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(expr))
        {
            if (cxxConstructExpr->getNumArgs() != 1)
                return true;
            expr = cxxConstructExpr->getArg(0);
        }
        if (auto materializeTemporaryExpr = dyn_cast<MaterializeTemporaryExpr>(expr))
            expr = materializeTemporaryExpr->GetTemporaryExpr();
        auto cxxFunctionalCastExpr = dyn_cast<CXXFunctionalCastExpr>(expr);
        if (!cxxFunctionalCastExpr)
            return true;
        auto const t1 = cxxFunctionalCastExpr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(cxxFunctionalCastExpr)->getType();
        if (t1.getCanonicalType().getTypePtr() != t2.getCanonicalType().getTypePtr())
            return true;
        if (!loplugin::isOkToRemoveArithmeticCast(compiler.getASTContext(), t1, t2,
                                                  cxxFunctionalCastExpr->getSubExpr()))
        {
            return true;
        }
        report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
               cxxFunctionalCastExpr->getExprLoc())
            << t2 << t1 << cxxFunctionalCastExpr->getSourceRange();
        return true;
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

    /* Check for the creation of unnecessary temporaries when calling a constructor that takes a param by const & */
    bool VisitCXXConstructExpr(CXXConstructExpr const* callExpr)
    {
        if (ignoreLocation(callExpr))
            return true;
        const CXXConstructorDecl* functionDecl = callExpr->getConstructor();

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
            auto arg = callExpr->getArg(i)->IgnoreImplicit();
            auto functionalCast = dyn_cast<CXXFunctionalCastExpr>(arg);
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
        // specifying the name for an init-list is necessary sometimes
        if (isa<InitListExpr>(expr->getSubExpr()->IgnoreImplicit()))
            return true;
        if (isa<CXXStdInitializerListExpr>(expr->getSubExpr()->IgnoreImplicit()))
            return true;
        auto const t1 = expr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();
        if (t1.getCanonicalType().getTypePtr() != t2.getCanonicalType().getTypePtr())
        {
            return true;
        }
        // (a) we do a lot of int/sal_Int32 kind of casts which might be platform necessary?
        // (b) we do bool/bool casts in unit tests to avoid one of the other plugins
        // so just ignore this kind of thing for now
        if (const auto* BT = dyn_cast<BuiltinType>(t1->getUnqualifiedDesugaredType()))
        {
            auto k = BT->getKind();
            if (k == BuiltinType::Double || k == BuiltinType::Float
                || (k >= BuiltinType::Bool && k <= BuiltinType::Int128))
                return true;
        }
        if (const auto* BT = dyn_cast<BuiltinType>(t2->getUnqualifiedDesugaredType()))
        {
            auto k = BT->getKind();
            if (k == BuiltinType::Double || k == BuiltinType::Float
                || (k >= BuiltinType::Bool && k <= BuiltinType::Int128))
                return true;
        }
        auto tc = loplugin::TypeCheck(t1);
        if (tc.Typedef("sal_Int32").GlobalNamespace())
            return true;

        report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
               expr->getExprLoc())
            << t2 << t1 << expr->getSourceRange();
        //getParentStmt(expr)->dump();
        return true;
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        std::string fn = handler.getMainFileName();
        loplugin::normalizeDotDotInFilePath(fn);
        // necessary on some other platforms
        if (fn == SRCDIR "/sal/osl/unx/socket.cxx")
            return false;
        // compile-time check of constant
        if (fn == SRCDIR "/bridges/source/jni_uno/jni_bridge.cxx")
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
};

static loplugin::Plugin::Registration<RedundantFCast> redundantfcast("redundantfcast");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
