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
    Expr const * sub = expr->getSubExpr()->IgnoreParenCasts();
    Expr const * expr2 = expr;
    for (;;) {
        BinaryOperator const * op = dyn_cast<BinaryOperator>(sub);
        if (op == nullptr || op->getOpcode() != BO_Comma) {
            break;
        }
        expr2 = op->getRHS()->IgnoreParenCasts();
        sub = expr2;
    }
    if (sub->getType()->isBooleanType()) {
        return true;
    }
    APSInt res;
    if (!sub->isValueDependent()
        && sub->isIntegerConstantExpr(res, compiler.getASTContext())
        && res.getLimitedValue() <= 1)
    {
        SourceLocation loc { sub->getLocStart() };
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
                return true;
            }
        }
    }
    if (isa<StringLiteral>(sub)) {
        SourceLocation loc { sub->getLocStart() };
        if (compiler.getSourceManager().isMacroArgExpansion(loc)
            && (Lexer::getImmediateMacroName(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "assert"))
        {
            return true;
        }
    }
    if (isa<IntegerLiteral>(sub) || isa<CharacterLiteral>(sub)
        || isa<FloatingLiteral>(sub) || isa<ImaginaryLiteral>(sub)
        || isa<StringLiteral>(sub))
    {
        bool rewritten = false;
        if (rewriter != nullptr) {
            SourceLocation loc { compiler.getSourceManager().getExpansionLoc(
                    expr->getLocStart()) };
            if (compiler.getSourceManager().getExpansionLoc(expr->getLocEnd())
                == loc)
            {
                char const * s = compiler.getSourceManager().getCharacterData(
                    loc);
                unsigned n = Lexer::MeasureTokenLength(
                    expr->getLocEnd(), compiler.getSourceManager(),
                    compiler.getLangOpts());
                std::string tok { s, n };
                if (tok == "sal_False" || tok == "0") {
                    rewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            expr->getLocStart()),
                        n, "false");
                } else if (tok == "sal_True" || tok == "1") {
                    rewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            expr->getLocStart()),
                        n, "true");
                }
            }
        }
        if (!rewritten) {
            report(
                DiagnosticsEngine::Warning,
                "implicit conversion (%0) of literal of type %1 to %2",
                expr2->getLocStart())
                << expr->getCastKindName() << expr->getSubExpr()->getType()
                << expr->getType() << expr2->getSourceRange();
        }
    } else if (sub->isNullPointerConstant(
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
            << expr->getCastKindName() << expr->getSubExpr()->getType()
            << expr->getType() << expr2->getSourceRange();
    } else if (!sub->isValueDependent()
               && sub->isIntegerConstantExpr(res, compiler.getASTContext()))
    {
        report(
            DiagnosticsEngine::Warning,
            ("implicit conversion (%0) of integer constant expression of type"
             " %1 with value %2 to %3"),
            expr2->getLocStart())
            << expr->getCastKindName() << expr->getSubExpr()->getType()
            << res.toString(10) << expr->getType() << expr2->getSourceRange();
    }
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

loplugin::Plugin::Registration<LiteralToBoolConversion> X(
    "literaltoboolconversion", true);

}
