/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include <iostream>

/**

TODO multiplying/otherop on a combination of a float and int, and then truncating to int. like this:
    float getRotation() {}
    int moRotation = -F_PI180 * 90 * getRotation();
*/
namespace
{
class IntVsFloat : public loplugin::FilteringPlugin<IntVsFloat>
{
public:
    explicit IntVsFloat(loplugin::InstantiationData const& data)
        : loplugin::FilteringPlugin<IntVsFloat>(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitVarDecl(VarDecl const*);
    bool VisitBinEQ(BinaryOperator const*);

private:
    llvm::Optional<double> getExprValue(Expr const* expr);
};

bool IntVsFloat::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl->getLocation()))
        return true;
    auto init = varDecl->getInit();
    if (!init)
        return true;
    init = init->IgnoreImpCasts();
    if (varDecl->getType()->isFloatingType())
        return true;
    //    init->dump();
    llvm::Optional<double> d = getExprValue(init);
    if (!d)
        return true;
    if (static_cast<long>(*d) == *d)
        return true;
    report(DiagnosticsEngine::Warning, "assigning constant float value to int truncates data",
           compat::getBeginLoc(init))
        << init->getSourceRange();

    return true;
}

bool IntVsFloat::VisitBinEQ(BinaryOperator const* op)
{
    if (ignoreLocation(compat::getBeginLoc(op)))
        return true;
    auto lhs = op->getLHS()->IgnoreImpCasts();
    auto rhs = op->getRHS()->IgnoreImpCasts();
    if (!lhs->getType()->isFloatingType())
        std::swap(lhs, rhs);
    if (!lhs->getType()->isFloatingType())
        return true;
    if (rhs->getType()->isFloatingType())
        return true;
    llvm::Optional<double> d = getExprValue(lhs);
    if (!d)
        return true;
    if (static_cast<long>(*d) == *d)
        return true;
    report(DiagnosticsEngine::Warning, "comparing integer to float constant, can never be true",
           compat::getBeginLoc(op))
        << op->getSourceRange();
    return true;
}

llvm::Optional<double> IntVsFloat::getExprValue(Expr const* expr)
{
    // Of the available clang Evaluate* APIs, this is the __only__ one that produces useful output
    // (as of 17 Aug 2018 checkout of clang, ie. towards clang 7)

    if (expr->isValueDependent())
        return llvm::Optional<double>();
    Expr::EvalResult evalResult;
    if (!expr->EvaluateAsRValue(evalResult, compiler.getASTContext()))
        return llvm::Optional<double>();
    if (!evalResult.Val.isFloat())
        return llvm::Optional<double>();
    llvm::APFloat floatResult = evalResult.Val.getFloat();
    bool losesInfo;
    floatResult.convert(APFloat::IEEEdouble(), APFloat::rmNearestTiesToEven, &losesInfo);

    double d = floatResult.convertToDouble();
    return d;
}

loplugin::Plugin::Registration<IntVsFloat> intvsfloat("intvsfloat");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
