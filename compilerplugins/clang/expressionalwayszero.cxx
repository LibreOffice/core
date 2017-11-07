/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>
#include <iostream>
#include <fstream>

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

/**
    Look for & and operator& expressions where the result is always zero.
    Generally a mistake when people meant to use | or operator|
*/

namespace {

static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class ExpressionAlwaysZero:
    public RecursiveASTVisitor<ExpressionAlwaysZero>, public loplugin::Plugin
{
public:
    explicit ExpressionAlwaysZero(loplugin::InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        loplugin::normalizeDotDotInFilePath(fn);
        // encoding of constant value for binary file format
        if (startswith(fn, SRCDIR "/package/source/zipapi/ZipFile.cxx"))
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(BinaryOperator const *);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const *);
    bool TraverseStaticAssertDecl(StaticAssertDecl *);
private:
    // note, abusing std::unique_ptr as a std::optional lookalike
    std::unique_ptr<APSInt> getExprValue(const Expr* arg);
};

bool ExpressionAlwaysZero::VisitBinaryOperator( BinaryOperator const * binaryOperator )
{
    if (ignoreLocation(binaryOperator))
        return true;
    if (binaryOperator->getLocStart().isMacroID())
        return true;
    if (binaryOperator->getOpcode() != BO_And)
        return true;
    auto lhsValue = getExprValue(binaryOperator->getLHS());
    auto rhsValue = getExprValue(binaryOperator->getRHS());
    if (!lhsValue || !rhsValue || (lhsValue->getExtValue() & rhsValue->getExtValue()) != 0)
        return true;
    report(
        DiagnosticsEngine::Warning, "expression always evaluates to zero, lhs=%0 rhs=%1",
        binaryOperator->getLocStart())
        << lhsValue->toString(10)
        << rhsValue->toString(10)
        << binaryOperator->getSourceRange();
    return true;
}

bool ExpressionAlwaysZero::VisitCXXOperatorCallExpr( CXXOperatorCallExpr const * cxxOperatorCallExpr )
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;
    if (cxxOperatorCallExpr->getLocStart().isMacroID())
        return true;
    if (cxxOperatorCallExpr->getOperator() != clang::OverloadedOperatorKind::OO_Amp)
        return true;
    if (cxxOperatorCallExpr->getNumArgs() != 2)
        return true;
    auto lhsValue = getExprValue(cxxOperatorCallExpr->getArg(0));
    auto rhsValue = getExprValue(cxxOperatorCallExpr->getArg(1));
    if (!lhsValue || !rhsValue || (lhsValue->getExtValue() & rhsValue->getExtValue()) != 0)
        return true;
    report(
        DiagnosticsEngine::Warning, "expression always evaluates to zero, lhs=%0 rhs=%1",
        cxxOperatorCallExpr->getLocStart())
        << lhsValue->toString(10)
        << rhsValue->toString(10)
        << cxxOperatorCallExpr->getSourceRange();
    return true;
}

std::unique_ptr<APSInt> ExpressionAlwaysZero::getExprValue(Expr const * expr)
{
    expr = expr->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(expr)) {
        return std::unique_ptr<APSInt>();
    }
    APSInt x1;
    if (expr->EvaluateAsInt(x1, compiler.getASTContext()))
        return std::unique_ptr<APSInt>(new APSInt(x1));
    return std::unique_ptr<APSInt>();
}

// these will often evaluate to zero harmlessly
bool ExpressionAlwaysZero::TraverseStaticAssertDecl( StaticAssertDecl * )
{
    return true;
}

loplugin::Plugin::Registration< ExpressionAlwaysZero > X("expressionalwayszero");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
