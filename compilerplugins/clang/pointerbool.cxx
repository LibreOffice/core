/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>
#include "plugin.hxx"
#include "check.hxx"

/**
  Look for calls where the param is bool but the call-site-arg is pointer.
*/
#ifndef LO_CLANG_SHARED_PLUGINS

namespace
{
class PointerBool : public loplugin::FilteringPlugin<PointerBool>
{
public:
    explicit PointerBool(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool shouldVisitTemplateInstantiations() const { return true; }

    bool PreTraverseFunctionDecl(FunctionDecl* decl);
    bool PostTraverseFunctionDecl(FunctionDecl* decl, bool);
    bool TraverseFunctionDecl(FunctionDecl* decl);
    bool VisitCallExpr(CallExpr const*);

private:
    llvm::Optional<APSInt> getCallValue(const Expr* arg);
    std::vector<FunctionDecl*> functions_;
};

bool PointerBool::PreTraverseFunctionDecl(FunctionDecl* decl)
{
    functions_.push_back(decl);
    return true;
}

bool PointerBool::PostTraverseFunctionDecl(FunctionDecl*, bool)
{
    assert(!functions_.empty());
    functions_.pop_back();
    return true;
}

bool PointerBool::TraverseFunctionDecl(FunctionDecl* decl)
{
    bool ret = true;
    if (PreTraverseFunctionDecl(decl))
    {
        ret = FilteringPlugin::TraverseFunctionDecl(decl);
        PostTraverseFunctionDecl(decl, ret);
    }
    return ret;
}

bool PointerBool::VisitCallExpr(CallExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    // TODO this doesn't currently work, the args and the params don't seem to line up
    if (isa<CXXOperatorCallExpr>(callExpr))
        return true;
    const FunctionDecl* functionDecl;
    if (isa<CXXMemberCallExpr>(callExpr))
    {
        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    }
    else
    {
        functionDecl = callExpr->getDirectCallee();
    }
    if (!functionDecl)
        return true;

    unsigned len = std::min(callExpr->getNumArgs(), functionDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        auto param = functionDecl->getParamDecl(i);
        auto paramTC = loplugin::TypeCheck(param->getType());
        if (!paramTC.AnyBoolean())
            continue;
        auto arg = callExpr->getArg(i)->IgnoreImpCasts();
        auto argTC = loplugin::TypeCheck(arg->getType());
        if (argTC.AnyBoolean())
            continue;
        // sal_Bool is sometimes disguised
        if (isa<SubstTemplateTypeParmType>(arg->getType()))
            if (arg->getType()->getUnqualifiedDesugaredType()->isSpecificBuiltinType(
                    clang::BuiltinType::UChar))
                continue;
        if (arg->getType()->isDependentType())
            continue;
        if (arg->getType()->isIntegerType())
        {
            auto ret = getCallValue(arg);
            if (ret.hasValue() && (ret.getValue() == 1 || ret.getValue() == 0))
                continue;
            // something like: priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD
            if (isa<BinaryOperator>(arg->IgnoreParenImpCasts()))
                continue;
            // something like: pbEmbolden ? FcTrue : FcFalse
            if (isa<ConditionalOperator>(arg->IgnoreParenImpCasts()))
                continue;
        }
        report(DiagnosticsEngine::Warning,
               "possibly unwanted implicit conversion when calling bool param", arg->getExprLoc())
            << arg->getSourceRange();
        report(DiagnosticsEngine::Note, "method here", param->getLocation())
            << param->getSourceRange();
        if (!functions_.empty())
        {
            auto callerFD = functions_.back();
            if (callerFD->isTemplateInstantiation())
                report(DiagnosticsEngine::Note, "instantiated from here",
                       callerFD->getPointOfInstantiation());
        }
    }
    return true;
}

llvm::Optional<APSInt> PointerBool::getCallValue(const Expr* arg)
{
    arg = arg->IgnoreParenCasts();
    if (auto defArg = dyn_cast<CXXDefaultArgExpr>(arg))
    {
        arg = defArg->getExpr()->IgnoreParenCasts();
    }
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
    {
        return llvm::Optional<APSInt>();
    }
    APSInt x1;
    if (compat::EvaluateAsInt(arg, x1, compiler.getASTContext()))
    {
        return x1;
    }
    return llvm::Optional<APSInt>();
}

loplugin::Plugin::Registration<PointerBool> pointerbool("pointerbool");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
