#include<iostream>
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

    bool isMacroBodyExpansion(SourceLocation location) const;
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
    if (sub->getType()->isBooleanType()) {
        return true;
    }
    IntegerLiteral const * lit = dyn_cast<IntegerLiteral>(sub);
    if (lit != nullptr && lit->getValue().getLimitedValue() <= 1) {
        SourceLocation loc { sub->getLocStart() };
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (isMacroBodyExpansion(loc)) {
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
                expr->getLocStart())
                << expr->getCastKindName() << expr->getSubExpr()->getType()
                << expr->getType() << expr->getSourceRange();
        }
// At least Clang 3.2 would erroneously warn about Cache::add
// (binaryurp/source/cache.hxx:53)
//
//   if( !size_) {
//
// as "implicit conversion (IntegralToBoolean) of null pointer constant of type
// 'std::size_t' (aka 'unsigned long') to 'bool'":
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    } else if (sub->isNullPointerConstant(
                   compiler.getASTContext(), Expr::NPC_ValueDependentIsNull)
               != Expr::NPCK_NotNull)
    {
        report(
            DiagnosticsEngine::Warning,
            ("implicit conversion (%0) of null pointer constant of type %1 to"
             " %2"),
            expr->getLocStart())
            << expr->getCastKindName() << expr->getSubExpr()->getType()
            << expr->getType() << expr->getSourceRange();
#endif
    } else if (sub->isIntegerConstantExpr(compiler.getASTContext())) {
        CallExpr const * ce = dyn_cast<CallExpr>(sub);
        if (ce == nullptr
            || compat::getBuiltinCallee(*ce) != Builtin::BI__builtin_expect)
        {
            report(
                DiagnosticsEngine::Warning,
                ("implicit conversion (%0) of integer constant expression of"
                 " type %1 to %2"),
                expr->getLocStart())
                << expr->getCastKindName() << expr->getSubExpr()->getType()
                << expr->getType() << expr->getSourceRange();
        }
    }
    return true;
}

bool LiteralToBoolConversion::isFromCIncludeFile(
    SourceLocation spellingLocation) const
{
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    if (compiler.getSourceManager().isInMainFile(spellingLocation)) {
        return false;
    }
#else
    if (compiler.getSourceManager().isFromMainFile(spellingLocation)) {
        return false;
    }
#endif
    return compiler.getSourceManager().getFilename(spellingLocation).endswith(
        ".h");
}

bool LiteralToBoolConversion::isMacroBodyExpansion(SourceLocation location)
    const
{
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return compiler.getSourceManager().isMacroBodyExpansion(location);
#else
    return location.isMacroID()
        && !compiler.getSourceManager().isMacroArgExpansion(location);
#endif
}

loplugin::Plugin::Registration<LiteralToBoolConversion> X(
    "literaltoboolconversion", true);

}
