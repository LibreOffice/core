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
#include "plugin.hxx"
#include "compat.hxx"

/**
comparing floating point numbers using == or != is a bad idea.
*/

namespace {

class FpComparison:
    public RecursiveASTVisitor<FpComparison>, public loplugin::Plugin
{
public:
    explicit FpComparison(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator* );
    bool TraverseFunctionDecl(FunctionDecl* );
private:
    bool ignore(FunctionDecl* );
    enum class EState { None, TraverseProcess, TraverseIgnore };
    EState meState = EState::None;
};

static const std::set<std::string> whitelist {
    "rtl::math::approxEqual",
    "(anonymous namespace)::doubleToString",
    "(anonymous namespace)::stringToDouble",
    "rtl_math_round",
    "rtl_math_approxValue",
    "rtl_math_asinh",
    "rtl_math_acosh",
    "cppu::_equalSequence", // cppu/source/uno/eq.hxx
    "cppu::_equalData", // cppu/source/uno/eq.hxx
    "xmlscript::equalFont", // xmlscript/source/xmldlg_imexp/xmldlg_export.cxx

    // these might need fixing
    "basegfx::tools::getSmallestDistancePointToPolygon", // basegfx/source/polygon/b2dpolygontools.cxx
    "basegfx::tools::getSmallestDistancePointToPolyPolygon", // basegfx/source/polygon/b2dpolypolygontools.cxx
    "bridge_test::performTest", // testtools/source/bridgetest/bridgetest.cxx
    "bridge_test::equals",
    "(anonymous namespace)::lcl_getNANInsteadDBL_MIN", // chart2/source/controller/chartapiwrapper/ChartDataWrapper.cxx
};
bool FpComparison::TraverseFunctionDecl(FunctionDecl* function)
{
    bool bIgnore = ignore(function);
    meState = bIgnore ? EState::TraverseIgnore : EState::TraverseProcess;
    bool bRet = RecursiveASTVisitor::TraverseFunctionDecl(function);
    meState = EState::None;
    return bRet;
}

bool FpComparison::ignore(FunctionDecl* function)
{
    if (ignoreLocation(function)) {
        return true;
    }
    // we assume that these modules know what they are doing with FP stuff
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(function->getLocStart()));
    if (aFileName.startswith(SRCDIR "/sc/")) {
        return true;
    }
    if (!function->doesThisDeclarationHaveABody()) {
        return true;
    }
     // Ignore operator== and operator!=
    if (function->getOverloadedOperator() == OO_EqualEqual
        || function->getOverloadedOperator() == OO_ExclaimEqual) {
        return true;
    }
    // ignore known good functions
    std::string s = function->getQualifiedNameAsString();
    if (whitelist.find(s) != whitelist.end()) {
        return true;
    }
//    cout << "xxx " + function->getQualifiedNameAsString() << endl;
    return false;
}

static bool isZeroConstant(ASTContext& context, const Expr* expr)
{
    // calling isCXX11ConstantExpr with non-arithmetic types sometimes results in a crash
    if (!expr->getType()->isArithmeticType()) {
        return false;
    }
    // prevent clang crash
    if (!context.getLangOpts().CPlusPlus) {
        return false;
    }
    APValue result;
    if (expr->isCXX11ConstantExpr(context, &result)
        && result.isFloat() && result.getFloat().isZero())
    {
        return true;
    }
    return false;
}
bool FpComparison::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (meState != EState::TraverseProcess || ignoreLocation(binaryOp)) {
        return true;
    }
    if (binaryOp->getOpcode() != BO_EQ && binaryOp->getOpcode() != BO_NE) {
        return true;
    }
    // comparison with zero is valid
    if (isZeroConstant(compiler.getASTContext(), binaryOp->getLHS())
        || isZeroConstant(compiler.getASTContext(), binaryOp->getRHS()))
    {
        return true;
    }
    QualType LHSStrippedType = binaryOp->getLHS()->IgnoreParenImpCasts()->getType();
    QualType RHSStrippedType = binaryOp->getRHS()->IgnoreParenImpCasts()->getType();
    if (LHSStrippedType->isFloatingType() && RHSStrippedType->isFloatingType()) {
        report(
            DiagnosticsEngine::Warning, "floating-point comparison",
            binaryOp->getSourceRange().getBegin())
          << binaryOp->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< FpComparison > X("fpcomparison", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
