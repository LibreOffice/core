/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include "plugin.hxx"
#include "check.hxx"

/*
*/

namespace
{
class HarmfulAuto : public loplugin::FilteringPlugin<HarmfulAuto>
{
public:
    explicit HarmfulAuto(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(VarDecl const*);
    bool hasTemplateArguments(const Expr*);
};

bool HarmfulAuto::VisitVarDecl(const VarDecl* pDecl)
{
    if (ignoreLocation(pDecl))
        return true;
    if (isa<DecompositionDecl>(pDecl))
        return true;
    QualType aType = pDecl->getType();

    if (!isa<AutoType>(aType.getTypePtr()))
        return true;

    auto canonicalType = aType.getCanonicalType();
    if (isa<AutoType>(canonicalType))
        // Can't suggest what to spell out, ignore.
        return true;

    if (auto recordType = dyn_cast<RecordType>(canonicalType.getTypePtr()))
    {
        auto recordDecl = recordType->getDecl();
        if (recordDecl->isLambda())
            return true;
        if (loplugin::DeclCheck(recordDecl).Class("function").StdNamespace())
            return true;
    }

    std::string aTypeName = aType.getAsString();
    if (aTypeName.find("iterator") != std::string::npos
        || aTypeName.find("Iterator") != std::string::npos)
        // Ignore iterators.
        return true;

    if (aTypeName.find("std::chrono::duration") != std::string::npos)
        // Unclear what to do here.
        return true;

    if (pDecl->hasInit())
    {
        if (isa<CXXStaticCastExpr>(pDecl->getInit()))
            return true;
        if (isa<CXXReinterpretCastExpr>(pDecl->getInit()))
            return true;

        auto pExprWithCleanups = dyn_cast<ExprWithCleanups>(pDecl->getInit());
        if (pExprWithCleanups)
        {
            if (isa<CXXStaticCastExpr>(pExprWithCleanups->getSubExpr()))
                return true;
        }

        auto expr2 = compat::IgnoreImplicit(pDecl->getInit());

        if (isa<CXXNewExpr>(expr2))
            return true;
        // ignore the lambda init stuff we do
        if (pDecl->isStaticLocal() && isa<CXXOperatorCallExpr>(expr2))
            return true;

        /*
         * Allow e.g.
         * auto pFoo = std::make_shared<Foo>();
         */
        if (hasTemplateArguments(expr2))
            return true;
    }

    report(DiagnosticsEngine::Warning, "harmful auto, consider spelling out %0 instead",
           compat::getBeginLoc(pDecl))
        << pDecl->getSourceRange() << aType;
    aType->dump();
    //pDecl->getInit()->dump();
    return true;
}

bool HarmfulAuto::hasTemplateArguments(const Expr* pExpr)
{
    if (!pExpr)
        return false;

    auto pCallExpr = dyn_cast<CallExpr>(pExpr);
    if (!pCallExpr)
    {
        auto pExprWithCleanups = dyn_cast<ExprWithCleanups>(pExpr);
        if (!pExprWithCleanups)
            return false;

        pCallExpr = dyn_cast<CallExpr>(pExprWithCleanups->getSubExpr());
        if (!pCallExpr)
        {
            auto pCXXConstructExpr = dyn_cast<CXXConstructExpr>(pExprWithCleanups->getSubExpr());
            if (!pCXXConstructExpr || pCXXConstructExpr->getNumArgs() < 1)
                return false;

            auto pMaterializeTemporaryExpr
                = dyn_cast<MaterializeTemporaryExpr>(pCXXConstructExpr->getArg(0));
            if (!pMaterializeTemporaryExpr)
                return false;

            auto pCXXBindTemporaryExpr
                = dyn_cast<CXXBindTemporaryExpr>(pMaterializeTemporaryExpr->getSubExpr());
            if (!pCXXBindTemporaryExpr)
                return false;

            pCallExpr = dyn_cast<CallExpr>(pCXXBindTemporaryExpr->getSubExpr());
        }
    }

    if (!pCallExpr || !pCallExpr->getCalleeDecl())
        return false;

    auto pFunctionDecl = dyn_cast<FunctionDecl>(pCallExpr->getCalleeDecl());
    if (!pFunctionDecl)
        return false;

    return pFunctionDecl->getTemplateSpecializationArgs() != nullptr;
}

loplugin::Plugin::Registration<HarmfulAuto> harmfulauto("harmfulauto");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
