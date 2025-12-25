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

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include "config_clang.h"
#include "plugin.hxx"
#include "check.hxx"
#include <unordered_set>
#include <unordered_map>

/*
Look for places where we are throwing away useful information by converting sal_UCS4 to sal_Unicode.
*/

namespace
{
class Narrow : public loplugin::FilteringPlugin<Narrow>
{
public:
    explicit Narrow(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator*);
    bool VisitVarDecl(const VarDecl*);
};

static const Expr* IgnoreImplicitAndConversionOperator(const Expr* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto memberCall = dyn_cast<CXXMemberCallExpr>(expr))
    {
        if (auto conversionDecl = dyn_cast_or_null<CXXConversionDecl>(memberCall->getMethodDecl()))
        {
            if (!conversionDecl->isExplicit())
                expr = memberCall->getImplicitObjectArgument()->IgnoreImplicit();
        }
    }
    return expr;
}

bool Narrow::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp))
        return true;
    if (binaryOp->getBeginLoc().isMacroID())
        return true;
    if (binaryOp->getOpcode() != BO_Assign)
        return true;
    auto rhsExpr = IgnoreImplicitAndConversionOperator(binaryOp->getRHS());
    if (!loplugin::TypeCheck(rhsExpr->getType()).Typedef("sal_UCS4").GlobalNamespace())
        return true;
    auto lhsType = binaryOp->getLHS()->getType();
    if (auto lhsTemplateType = dyn_cast<clang::SubstTemplateTypeParmType>(lhsType))
        lhsType = lhsTemplateType->getReplacementType();
    auto tc = loplugin::TypeCheck(lhsType);
    // std::unique_ptr<sal_UCS4[]> will decompose to UInt.
    if (tc.Typedef("sal_UCS4").GlobalNamespace()
        || lhsType->isSpecificBuiltinType(BuiltinType::UInt))
        return true;
    report(DiagnosticsEngine::Warning,
           "loosing information assigning a sal_UCS4 value to a %0 type", binaryOp->getBeginLoc())
        << lhsType << binaryOp->getSourceRange();
    return true;
}

bool Narrow::VisitVarDecl(const VarDecl* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (!decl->getInit())
        return true;
    auto expr = IgnoreImplicitAndConversionOperator(decl->getInit());
    if (!loplugin::TypeCheck(expr->getType()).Typedef("sal_UCS4").GlobalNamespace())
        return true;
    if (loplugin::TypeCheck(decl->getType()).Typedef("sal_UCS4").GlobalNamespace())
        return true;
    report(DiagnosticsEngine::Warning,
           "loosing information assigning a sal_UCS4 value to a %0 type", decl->getBeginLoc())
        << decl->getType() << decl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<Narrow> narrow("narrow");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
