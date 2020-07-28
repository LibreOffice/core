/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <limits>

#include "clang/Lex/Lexer.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace {

class LiteralToBoolConversion:
    public loplugin::FilteringRewritePlugin<LiteralToBoolConversion>
{
public:
    explicit LiteralToBoolConversion(loplugin::InstantiationData const & data):
        FilteringRewritePlugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);

    bool PreTraverseLinkageSpecDecl(LinkageSpecDecl * decl);
    bool PostTraverseLinkageSpecDecl(LinkageSpecDecl * decl, bool);
    bool TraverseLinkageSpecDecl(LinkageSpecDecl * decl);

private:
    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isSharedCAndCppCode(SourceLocation location) const;

    void handleImplicitCastSubExpr(
        ImplicitCastExpr const * castExpr, Expr const * subExpr);

    unsigned int externCContexts_ = 0;
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

bool LiteralToBoolConversion::PreTraverseLinkageSpecDecl(LinkageSpecDecl *) {
    assert(externCContexts_ != std::numeric_limits<unsigned int>::max()); //TODO
    ++externCContexts_;
    return true;
}

bool LiteralToBoolConversion::PostTraverseLinkageSpecDecl(LinkageSpecDecl *, bool) {
    assert(externCContexts_ != 0);
    --externCContexts_;
    return true;
}

bool LiteralToBoolConversion::TraverseLinkageSpecDecl(LinkageSpecDecl * decl) {
    PreTraverseLinkageSpecDecl(decl);
    bool ret = RecursiveASTVisitor::TraverseLinkageSpecDecl(decl);
    PostTraverseLinkageSpecDecl(decl, ret);
    return ret;
}

bool LiteralToBoolConversion::isFromCIncludeFile(
    SourceLocation spellingLocation) const
{
    return !compiler.getSourceManager().isInMainFile(spellingLocation)
        && (StringRef(
                compiler.getSourceManager().getPresumedLoc(spellingLocation)
                .getFilename())
            .endswith(".h"));
}

bool LiteralToBoolConversion::isSharedCAndCppCode(SourceLocation location) const
{
    // Assume that code is intended to be shared between C and C++ if it comes
    // from an include file ending in .h, and is either in an extern "C" context
    // or the body of a macro definition:
    return
        isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(location))
        && (externCContexts_ != 0
            || compiler.getSourceManager().isMacroBodyExpansion(location));
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
    if (!subExpr->isValueDependent()) {
        if (auto const res = compat::getIntegerConstantExpr(subExpr, compiler.getASTContext())) {
            if (res->getLimitedValue() <= 1)
            {
                SourceLocation loc { compat::getBeginLoc(subExpr) };
                while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                    loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
                }
                if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
                    StringRef name { Lexer::getImmediateMacroName(
                            loc, compiler.getSourceManager(), compiler.getLangOpts()) };
                    if (name == "sal_False" || name == "sal_True") {
                        loc = compat::getImmediateExpansionRange(compiler.getSourceManager(), loc)
                            .first;
                    }
                    if (isSharedCAndCppCode(loc)) {
                        return;
                    }
                }
            }
        }
    }
    if (isa<clang::StringLiteral>(subExpr)) {
        SourceLocation loc { compat::getBeginLoc(subExpr) };
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
        || isa<clang::StringLiteral>(subExpr))
    {
        bool bRewritten = false;
        if (rewriter != nullptr) {
            SourceLocation loc { compiler.getSourceManager().getExpansionLoc(
                    compat::getBeginLoc(expr2)) };
            if (compiler.getSourceManager().getExpansionLoc(compat::getEndLoc(expr2))
                == loc)
            {
                char const * s = compiler.getSourceManager().getCharacterData(
                    loc);
                unsigned n = Lexer::MeasureTokenLength(
                    compat::getEndLoc(expr2), compiler.getSourceManager(),
                    compiler.getLangOpts());
                std::string tok { s, n };
                if (tok == "sal_False" || tok == "0") {
                    bRewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            compat::getBeginLoc(expr2)),
                        n, "false");
                } else if (tok == "sal_True" || tok == "1") {
                    bRewritten = replaceText(
                        compiler.getSourceManager().getExpansionLoc(
                            compat::getBeginLoc(expr2)),
                        n, "true");
                }
            }
        }
        if (!bRewritten) {
            report(
                DiagnosticsEngine::Warning,
                "implicit conversion (%0) of literal of type %1 to %2",
                compat::getBeginLoc(expr2))
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
            compat::getBeginLoc(expr2))
            << castExpr->getCastKindName() << subExpr->getType()
            << castExpr->getType() << expr2->getSourceRange();
    } else if (!subExpr->isValueDependent()) {
        if (auto const res = compat::getIntegerConstantExpr(subExpr, compiler.getASTContext())) {
            report(
                DiagnosticsEngine::Warning,
                ("implicit conversion (%0) of integer constant expression of type"
                 " %1 with value %2 to %3"),
                compat::getBeginLoc(expr2))
                << castExpr->getCastKindName() << subExpr->getType()
                << res->toString(10) << castExpr->getType()
                << expr2->getSourceRange();
        }
    }
}

loplugin::Plugin::Registration<LiteralToBoolConversion> literaltoboolconversion("literaltoboolconversion");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS
