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
  Simplify boolean expressions involving smart pointers e.g.
    if (x.get())
  can be
    if (x)
*/
#ifndef LO_CLANG_SHARED_PLUGINS

namespace
{
class SimplifyPointerToBool : public loplugin::FilteringPlugin<SimplifyPointerToBool>
{
public:
    explicit SimplifyPointerToBool(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const*);
};

bool SimplifyPointerToBool::VisitImplicitCastExpr(ImplicitCastExpr const* castExpr)
{
    if (ignoreLocation(castExpr))
        return true;
    if (castExpr->getCastKind() != CK_PointerToBoolean)
        return true;
    auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(castExpr->getSubExpr());
    if (!memberCallExpr)
        return true;
    auto methodDecl = memberCallExpr->getMethodDecl();
    if (!methodDecl || !methodDecl->getIdentifier() || methodDecl->getName() != "get")
        return true;
    //    castExpr->dump();
    //    methodDecl->getParent()->getTypeForDecl()->dump();
    if (!loplugin::isSmartPointerType(methodDecl->getParent()->getTypeForDecl()))
        return true;
    //    if (isa<CXXOperatorCallExpr>(callExpr))
    //        return true;
    //    const FunctionDecl* functionDecl;
    //    if (isa<CXXMemberCallExpr>(callExpr))
    //    {
    //        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    //    }
    //    else
    //    {
    //        functionDecl = callExpr->getDirectCallee();
    //    }
    //    if (!functionDecl)
    //        return true;
    //
    //    unsigned len = std::min(callExpr->getNumArgs(), functionDecl->getNumParams());
    //    for (unsigned i = 0; i < len; ++i)
    //    {
    //        auto param = functionDecl->getParamDecl(i);
    //        auto paramTC = loplugin::TypeCheck(param->getType());
    //        if (!paramTC.AnyBoolean())
    //            continue;
    //        auto arg = callExpr->getArg(i)->IgnoreImpCasts();
    //        auto argTC = loplugin::TypeCheck(arg->getType());
    //        if (argTC.AnyBoolean())
    //            continue;
    //        // sal_Bool is sometimes disguised
    //        if (isa<SubstTemplateTypeParmType>(arg->getType()))
    //            if (arg->getType()->getUnqualifiedDesugaredType()->isSpecificBuiltinType(
    //                    clang::BuiltinType::UChar))
    //                continue;
    //        if (arg->getType()->isDependentType())
    //            continue;
    //        if (arg->getType()->isIntegerType())
    //        {
    //            auto ret = getCallValue(arg);
    //            if (ret.hasValue() && (ret.getValue() == 1 || ret.getValue() == 0))
    //                continue;
    //            // something like: priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD
    //            if (isa<BinaryOperator>(arg->IgnoreParenImpCasts()))
    //                continue;
    //            // something like: pbEmbolden ? FcTrue : FcFalse
    //            if (isa<ConditionalOperator>(arg->IgnoreParenImpCasts()))
    //                continue;
    //        }
    report(DiagnosticsEngine::Warning, "simplify, drop the get()", castExpr->getExprLoc())
        << castExpr->getSourceRange();
    //        report(DiagnosticsEngine::Note, "method here", param->getLocation())
    //            << param->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<SimplifyPointerToBool>
    simplifypointertobool("simplifypointertobool");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
