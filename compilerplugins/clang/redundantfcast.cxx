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
#include <unordered_set>
#include <vector>

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
            expr = compat::getSubExpr(materializeTemporaryExpr);
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
        if (m_Seen.insert(cxxFunctionalCastExpr->getExprLoc()).second)
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
                compat::getSubExpr(materializeTemporaryExpr)->IgnoreImpCasts());
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
            // Don't warn about (necessary) cast from braced-init-list in non-deduced contexts:
            if (lvalueType->getPointeeType()->getAs<SubstTemplateTypeParmType>() != nullptr
                && loplugin::TypeCheck(t1).ClassOrStruct("initializer_list").StdNamespace()
                && isa<CXXStdInitializerListExpr>(compat::getSubExprAsWritten(functionalCast)))
            {
                continue;
            }

            if (m_Seen.insert(arg->getExprLoc()).second)
            {
                report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
                       arg->getExprLoc())
                    << t2 << t1 << arg->getSourceRange();
                report(DiagnosticsEngine::Note, "in call to method here", param->getLocation())
                    << param->getSourceRange();
            }
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
            auto rvalueType = param->getType()->getAs<RValueReferenceType>();
            if (!rvalueType)
            {
                auto lvalueType = param->getType()->getAs<LValueReferenceType>();
                if (!lvalueType)
                    continue;
                if (!lvalueType->getPointeeType().isConstQualified())
                    continue;
            }
            auto valueType = param->getType()->getAs<ReferenceType>();
            auto paramClassOrStructType = valueType->getPointeeType()->getAs<RecordType>();
            if (!paramClassOrStructType)
                continue;
            // check for temporary and functional cast in argument expression
            auto arg = compat::IgnoreParenImplicit(callExpr->getArg(i));
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
            if (rvalueType)
            {
                // constructing a temporary to pass to a && argument is fine. But we will see that in the VisitFunctionalCast
                // method below and generate a warning. And we don't have enough context there to determine that we're
                // doing the wrong thing. So add the expression to the m_Seen list here to prevent that warning.
                m_Seen.insert(functionalCast->getExprLoc());
                continue;
            }

            if (m_Seen.insert(arg->getExprLoc()).second)
            {
                report(DiagnosticsEngine::Warning,
                       "redundant functional cast from %0 to %1 in construct expression",
                       arg->getExprLoc())
                    << t2 << t1 << arg->getSourceRange();
                report(DiagnosticsEngine::Note, "in call to method here", param->getLocation())
                    << param->getSourceRange();
            }
        }
        return true;
    }

    // Find redundant cast to std::function, where clang reports
    // two different types for the inner and outer
    bool isRedundantStdFunctionCast(CXXFunctionalCastExpr const* expr)
    {
        bool deduced = false;
        QualType target;
        auto const written = expr->getTypeAsWritten();
        if (auto const t1 = written->getAs<DeducedTemplateSpecializationType>())
        {
            auto const decl = t1->getTemplateName().getAsTemplateDecl();
            if (!decl)
            {
                return false;
            }
            if (!loplugin::DeclCheck(decl->getTemplatedDecl())
                     .ClassOrStruct("function")
                     .StdNamespace())
            {
                return false;
            }
            deduced = true;
        }
        else if (auto const t2 = written->getAs<TemplateSpecializationType>())
        {
            auto const decl = t2->getTemplateName().getAsTemplateDecl();
            if (!decl)
            {
                return false;
            }
            if (!loplugin::DeclCheck(decl->getTemplatedDecl())
                     .ClassOrStruct("function")
                     .StdNamespace())
            {
                return false;
            }
            if (t2->getNumArgs() != 1)
            {
                if (isDebugMode())
                {
                    report(DiagnosticsEngine::Fatal,
                           "TODO: unexpected std::function with %0 template arguments",
                           expr->getExprLoc())
                        << t2->getNumArgs() << expr->getSourceRange();
                }
                return false;
            }
            if (t2->getArg(0).getKind() != TemplateArgument::Type)
            {
                if (isDebugMode())
                {
                    report(DiagnosticsEngine::Fatal,
                           "TODO: unexpected std::function with non-type template argument",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                return false;
            }
            target = t2->getArg(0).getAsType();
        }
        else
        {
            return false;
        }
        auto cxxConstruct
            = dyn_cast<CXXConstructExpr>(compat::IgnoreParenImplicit(expr->getSubExpr()));
        if (!cxxConstruct)
            return false;
        auto const lambda
            = dyn_cast<LambdaExpr>(compat::IgnoreParenImplicit(cxxConstruct->getArg(0)));
        if (!lambda)
            return false;
        if (deduced)
            // std::function([...](Args)->Ret{...}) should always be redundant:
            return true;
        auto const decl = lambda->getCallOperator();
        std::vector<QualType> args;
        for (unsigned i = 0; i != decl->getNumParams(); ++i)
        {
            args.push_back(decl->getParamDecl(i)->getType());
        }
        auto const source
            = compiler.getASTContext().getFunctionType(decl->getReturnType(), args, {});
        // std::function<Ret1(Args1)>([...](Args2)->Ret2{...}) is redundant if target Ret1(Args1)
        // matches source Ret2(Args2):
        return target.getCanonicalType() == source.getCanonicalType();
    }

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        // specifying the name for an init-list is necessary sometimes
        if (isa<InitListExpr>(compat::IgnoreParenImplicit(expr->getSubExpr())))
            return true;
        if (isa<CXXStdInitializerListExpr>(compat::IgnoreParenImplicit(expr->getSubExpr())))
            return true;
        auto const t1 = expr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();
        if (!(t1.getCanonicalType().getTypePtr() == t2.getCanonicalType().getTypePtr()
              || isRedundantStdFunctionCast(expr)))
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

        if (m_Seen.insert(expr->getExprLoc()).second)
            report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
                   expr->getExprLoc())
                << t2 << t1 << expr->getSourceRange();
        return true;
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        std::string fn = handler.getMainFileName().str();
        loplugin::normalizeDotDotInFilePath(fn);
        // necessary on some other platforms
        if (fn == SRCDIR "/sal/osl/unx/socket.cxx")
            return false;
        // compile-time check of constant
        if (fn == SRCDIR "/bridges/source/jni_uno/jni_bridge.cxx")
            return false;
        // TODO constructing a temporary to pass to a && param
        if (fn == SRCDIR "/sc/source/ui/view/viewfunc.cxx")
            return false;
        // tdf#145203: FIREBIRD cannot create a table
        if (fn == SRCDIR "/connectivity/source/drivers/firebird/DatabaseMetaData.cxx")
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    std::unordered_set<SourceLocation> m_Seen;
};

static loplugin::Plugin::Registration<RedundantFCast> redundantfcast("redundantfcast");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
