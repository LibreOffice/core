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

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

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
    loplugin::DeclCheck dc(function);
    if ((dc.Function("approxEqual").Namespace("math").Namespace("rtl")
         .GlobalNamespace())
        || dc.Function("doubleToString").AnonymousNamespace().GlobalNamespace()
        || dc.Function("stringToDouble").AnonymousNamespace().GlobalNamespace()
        || dc.Function("rtl_math_round").GlobalNamespace()
        || dc.Function("rtl_math_approxValue").GlobalNamespace()
        || dc.Function("rtl_math_asinh").GlobalNamespace()
        || dc.Function("rtl_math_acosh").GlobalNamespace()
        || dc.Function("_equalSequence").Namespace("cppu").GlobalNamespace()
            // cppu/source/uno/eq.hxx
        || dc.Function("_equalData").Namespace("cppu").GlobalNamespace()
            // cppu/source/uno/eq.hxx
        || dc.Function("equalFont").Namespace("xmlscript").GlobalNamespace()
            // xmlscript/source/xmldlg_imexp/xmldlg_export.cxx
        // These might need fixing:
        || (dc.Function("getSmallestDistancePointToPolygon").Namespace("tools")
            .Namespace("basegfx").GlobalNamespace())
            // basegfx/source/polygon/b2dpolygontools.cxx
        || (dc.Function("getSmallestDistancePointToPolyPolygon")
            .Namespace("tools").Namespace("basegfx").GlobalNamespace())
            // basegfx/source/polygon/b2dpolypolygontools.cxx
        || dc.Function("performTest").Namespace("bridge_test").GlobalNamespace()
            // testtools/source/bridgetest/bridgetest.cxx
        || dc.Function("equals").Namespace("bridge_test").GlobalNamespace()
        || (dc.Function("lcl_getNANInsteadDBL_MIN").AnonymousNamespace()
            .GlobalNamespace()))
            // chart2/source/controller/chartapiwrapper/ChartDataWrapper.cxx
    {
        return true;
    }
//    cout << "xxx " + function->getQualifiedNameAsString() << endl;
    return false;
}

bool isZeroConstant(ASTContext& context, const Expr* expr)
{
    if (!expr->getType()->isFloatingType()) {
        return false;
    }
    // prevent clang crash
    if (!context.getLangOpts().CPlusPlus) {
        return false;
    }
    APValue result;
    if (!expr->isCXX11ConstantExpr(context, &result)) {
        return false;
    }
    assert(result.isFloat());
    return result.getFloat().isZero();
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
