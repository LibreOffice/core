/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clang/Lex/Lexer.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace {

class LiteralToBoolConversion:
    public RecursiveASTVisitor<LiteralToBoolConversion>,
    public loplugin::RewritePlugin
{
public:
    explicit LiteralToBoolConversion(InstantiationData const & data):
        RewritePlugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);

private:
    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    void handleImplicitCastSubExpr(
        ImplicitCastExpr const * castExpr, Expr const * subExpr);
};

bool LiteralToBoolConversion::VisitImplicitCastExpr(
    ImplicitCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!expr->getType()->isBooleanType()) {
        return true;
    }
    handleImplicitCastSubExpr(expr, expr->getSubExpr());
    return true;
}

bool LiteralToBoolConversion::isFromCIncludeFile(
    SourceLocation spellingLocation) const
{
    return !compat::isInMainFile(compiler.getSourceManager(), spellingLocation)
        && (StringRef(
                compiler.getSourceManager().getPresumedLoc(spellingLocation)
                .getFilename())
            .endswith(".h"));
}

void LiteralToBoolConversion::handleImplicitCastSubExpr(
    ImplicitCastExpr const * castExpr, Expr const * subExpr)
{
    Expr const * expr2 = subExpr;
        // track sub-expr with potential parens, to e.g. rewrite all of expanded
        //
        // #define sal_False ((sal_Bool)0)
        //
        // including the parens
    subExpr = expr2->IgnoreParenCasts();
    for (;;) {
        BinaryOperator const * op = dyn_cast<BinaryOperator>(subExpr);
        if (op == nullptr || op->getOpcode() != BO_Comma) {
            break;
        }
        expr2 = op->getRHS();
        subExpr = expr2->IgnoreParenCasts();
    }
    if (subExpr->getType()->isBooleanType()) {
        return;
    }
    ConditionalOperator const * op = dyn_cast<ConditionalOperator>(subExpr);
    if (op != nullptr) {
        handleImplicitCastSubExpr(castExpr, op->getTrueExpr());
        handleImplicitCastSubExpr(castExpr, op->getFalseExpr());
        return;
    }
    APSInt res;
    if (!subExpr->isValueDependent()
        && subExpr->isIntegerConstantExpr(res, compiler.getASTContext())
        && res.getLimitedValue() <= 1)
    {
        SourceLocation loc { subExpr->getLocStart() };
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (compat::isMacroBodyExpansion(compiler, loc)) {
            StringRef name { Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts()) };
            if (name == "sal_False" || name == "sal_True") {
                loc = compiler.getSourceManager().getImmediateExpansionRange(
                    loc).first;
            }
            if (isFromCIncludeFile(
                    compiler.getSourceManager().getSpellingLoc(loc)))
            {
                return;
            }
        }
    }
    if (isa<StringLiteral>(subExpr)) {
        SourceLocation loc { subExpr->getLocStart() };
        if (compiler.getSourceManager().isMacroArgExpansion(loc)
            && (Lexer::getImmediateMacroName(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "assert"))
        {
            return;
        }
    }
    if (isa<IntegerLiteral>(subExpr) || isa<CharacterLiteral>(subExpr)
        || isa<FloatingLiteral>(subExpr) || isa<ImaginaryLiteral>(subExpr)
        || isa<StringLiteral>(subExpr))
    {
        bool bRewritten = false;
        if (rewriter != nullptr) {
            SourceLocation loc { compiler.getSourceManager().getExpansionLoc(
                    expr2->getLocStart()) };
            if (compiler.getSourceManager().getExpansionLoc(expr2->getLocEnd())
                == loc)
            {
                char const * s = compiler.getSourceManager().getCharacterData(
                    loc);
                unsigned n = Lexer::MeasureTokenLength(
                    expr2->getLocEnd(), compiler.getSourceManager(),
                    compiler.getLangOpts());
                std::string tok { s, n };
                if (tok == "sal_False" || tok == "0") {
                    bRewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            expr2->getLocStart()),
                        n, "false");
                } else if (tok == "sal_True" || tok == "1") {
                    bRewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            expr2->getLocStart()),
                        n, "true");
                }
            }
        }
        if (!bRewritten) {
            report(
                DiagnosticsEngine::Warning,
                "implicit conversion (%0) of literal of type %1 to %2",
                expr2->getLocStart())
                << castExpr->getCastKindName() << subExpr->getType()
                << castExpr->getType() << expr2->getSourceRange();
        }
    } else if (subExpr->isNullPointerConstant(
                   compiler.getASTContext(), Expr::NPC_ValueDependentIsNull)
               > Expr::NPCK_ZeroExpression)
    {
        // The test above originally checked for != Expr::NPCK_NotNull, but in non-C++11
        // mode we can get also Expr::NPCK_ZeroExpression inside templates, even though
        // the expression is actually not a null pointer. Clang bug or C++98 misfeature?
        // See Clang's NPCK_ZeroExpression declaration and beginning of isNullPointerConstant().
        static_assert( Expr::NPCK_NotNull == 0 && Expr::NPCK_ZeroExpression == 1, "Clang API change" );
        report(
            DiagnosticsEngine::Warning,
            ("implicit conversion (%0) of null pointer constant of type %1 to"
             " %2"),
            expr2->getLocStart())
            << castExpr->getCastKindName() << subExpr->getType()
            << castExpr->getType() << expr2->getSourceRange();
    } else if (!subExpr->isValueDependent()
               && subExpr->isIntegerConstantExpr(res, compiler.getASTContext()))
    {
        report(
            DiagnosticsEngine::Warning,
            ("implicit conversion (%0) of integer constant expression of type"
             " %1 with value %2 to %3"),
            expr2->getLocStart())
            << castExpr->getCastKindName() << subExpr->getType()
            << res.toString(10) << castExpr->getType()
            << expr2->getSourceRange();
    }
}

loplugin::Plugin::Registration<LiteralToBoolConversion> X(
    "literaltoboolconversion", true);

}
