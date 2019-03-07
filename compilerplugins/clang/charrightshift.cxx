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
#include "plugin.hxx"

namespace {

class CharRightShift:
    public loplugin::FilteringPlugin<CharRightShift>
{
public:
    explicit CharRightShift(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitBinShr(BinaryOperator const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto t = expr->getLHS()->IgnoreParenImpCasts()->getType();
        if (!loplugin::TypeCheck(t).Char()) {
            return true;
        }
        if (!expr->getRHS()->getType()->isBuiltinType()) {
            //TODO: in which case the expression should be an
            // CXXOperatorCallExpr instead of a BinaryOperator? but at least
            // recent Clang trunk reports
            //
            //  '(' >> orExpression
            //
            // (connectivity/source/commontools/RowFunctionParser.cxx, the RHS
            // being of type boost::spirit::rule<ScannerT>) here
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            ("right shift of %0 is implementation-defined when 'char' is signed"
             " and value is negative"),
            expr->getLHS()->getExprLoc())
            << t << expr->getSourceRange();
        return true;
    }
};

loplugin::Plugin::Registration<CharRightShift> charrightshift("charrightshift");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
