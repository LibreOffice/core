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
    public loplugin::FilteringPlugin<ExpressionAlwaysZero>
{
public:
    explicit ExpressionAlwaysZero(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override
    {
        // don't use getMainFileID, it may return "<stdin>"
        std::string fn(handler.getMainFileName());

        loplugin::normalizeDotDotInFilePath(fn);
        // encoding of constant value for binary file format
        if (startswith(fn, SRCDIR "/package/source/zipapi/ZipFile.cxx"))
            return;
        // some auto-generated static data
        if (startswith(fn, SRCDIR "/sal/textenc/tables.cxx"))
            return;
        // nested conditional defines that are not worth cleaning up
        if (startswith(fn, SRCDIR "/opencl/source/openclwrapper.cxx"))
            return;
        // some kind of matrix calculation, the compiler will optimise it out anyway
        if (startswith(fn, SRCDIR "/vcl/source/gdi/bitmap4.cxx"))
            return;
        // code follows a pattern
        if (startswith(fn, SRCDIR "/svx/source/svdraw/svdhdl.cxx"))
            return;
        // looks like some kind of TODO marker
        if (startswith(fn, SRCDIR "/chart2/source/view/main/PropertyMapper.cxx")
            || startswith(fn, SRCDIR "/sc/source/core/data/formulacell.cxx"))
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
    if (compat::getBeginLoc(binaryOperator).isMacroID())
        return true;

    auto op = binaryOperator->getOpcode();
    if (!(op == BO_And || op == BO_AndAssign || op == BO_LAnd))
        return true;

    auto lhsValue = getExprValue(binaryOperator->getLHS());
    auto rhsValue = getExprValue(binaryOperator->getRHS());
    if (lhsValue && lhsValue->getExtValue() == 0)
        ; // ok
    else if (rhsValue && rhsValue->getExtValue() == 0)
        ; // ok
    else if (lhsValue && rhsValue && (lhsValue->getExtValue() & rhsValue->getExtValue()) == 0)
        ; // ok
    else
        return true;
    report(
        DiagnosticsEngine::Warning, "expression always evaluates to zero, lhs=%0 rhs=%1",
        compat::getBeginLoc(binaryOperator))
        << (lhsValue ? lhsValue->toString(10) : "unknown")
        << (rhsValue ? rhsValue->toString(10) : "unknown")
        << binaryOperator->getSourceRange();
    return true;
}

bool ExpressionAlwaysZero::VisitCXXOperatorCallExpr( CXXOperatorCallExpr const * cxxOperatorCallExpr )
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;
    if (compat::getBeginLoc(cxxOperatorCallExpr).isMacroID())
        return true;

    auto op = cxxOperatorCallExpr->getOperator();
    if ( !(op == OO_Amp || op == OO_AmpEqual || op == OO_AmpAmp))
        return true;

    if (cxxOperatorCallExpr->getNumArgs() != 2)
        return true;
    auto lhsValue = getExprValue(cxxOperatorCallExpr->getArg(0));
    auto rhsValue = getExprValue(cxxOperatorCallExpr->getArg(1));
    if (lhsValue && lhsValue->getExtValue() == 0)
        ; // ok
    else if (rhsValue && rhsValue->getExtValue() == 0)
        ; // ok
    else if (lhsValue && rhsValue && (lhsValue->getExtValue() & rhsValue->getExtValue()) == 0)
        ; // ok
    else
        return true;
    report(
        DiagnosticsEngine::Warning, "expression always evaluates to zero, lhs=%0 rhs=%1",
        compat::getBeginLoc(cxxOperatorCallExpr))
        << (lhsValue ? lhsValue->toString(10) : "unknown")
        << (rhsValue ? rhsValue->toString(10) : "unknown")
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

// on clang-3.8, this plugin can generate OOM
#if CLANG_VERSION >= 30900
loplugin::Plugin::Registration< ExpressionAlwaysZero > X("expressionalwayszero");
#else
loplugin::Plugin::Registration< ExpressionAlwaysZero > X("expressionalwayszero", false);
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
