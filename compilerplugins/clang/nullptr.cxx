/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

namespace {

char const * kindName(Expr::NullPointerConstantKind kind) {
    switch (kind) {
    case Expr::NPCK_NotNull:
        assert(false); // cannot happen
        // fall through
    case Expr::NPCK_ZeroExpression:
        return "ZeroExpression";
    case Expr::NPCK_ZeroLiteral:
        return "ZeroLiteral";
    case Expr::NPCK_CXX11_nullptr:
        return "CXX11_nullptr";
    case Expr::NPCK_GNUNull:
        return "GNUNull";
    }
}

class Nullptr:
    public RecursiveASTVisitor<Nullptr>, public loplugin::RewritePlugin
{
public:
    explicit Nullptr(InstantiationData const & data): RewritePlugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitImplicitCastExpr(CastExpr const * expr);

    bool VisitGNUNullExpr(GNUNullExpr const * expr);

    bool VisitBinaryOperator(BinaryOperator const * expr);

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const * expr);

    // bool shouldVisitTemplateInstantiations() const { return true; }

private:
    bool isInLokIncludeFile(SourceLocation spellingLocation) const;

    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isMacroBodyExpansion(SourceLocation location) const;

    void handleNull(
        Expr const * expr, char const * castKind,
        Expr::NullPointerConstantKind nullPointerKind);

    void rewriteOrWarn(
        Expr const * expr, char const * castKind,
        Expr::NullPointerConstantKind nullPointerKind,
        char const * replacement);

    std::set<Expr const *> gnuNulls_;
};

bool Nullptr::VisitImplicitCastExpr(CastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    switch (expr->getCastKind()) {
    case CK_NullToPointer:
    case CK_NullToMemberPointer:
        break;
    default:
        return true;
    }
    Expr::NullPointerConstantKind k = expr->isNullPointerConstant(
        compiler.getASTContext(), Expr::NPC_ValueDependentIsNotNull);
    switch (k) {
    case Expr::NPCK_NotNull:
        k = expr->isNullPointerConstant(
            compiler.getASTContext(), Expr::NPC_ValueDependentIsNull);
        switch (k) {
        case Expr::NPCK_NotNull:
            break;
        case Expr::NPCK_ZeroExpression:
        case Expr::NPCK_ZeroLiteral:
            report(
                DiagnosticsEngine::Warning,
                "suspicious ValueDependentIsNull %0", expr->getLocStart())
                << kindName(k) << expr->getSourceRange();
            break;
        default:
            assert(false); // cannot happen
        }
        break;
    case Expr::NPCK_CXX11_nullptr:
        break;
    default:
        handleNull(expr->getSubExpr(), expr->getCastKindName(), k);
        break;
    }
    return true;
}

bool Nullptr::VisitGNUNullExpr(GNUNullExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    handleNull(expr, nullptr, Expr::NPCK_GNUNull);
    return true;
}

bool Nullptr::VisitBinaryOperator(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    Expr const * e;
    switch (expr->getOpcode()) {
    case BO_EQ:
    case BO_NE:
        if (expr->getRHS()->getType()->isPointerType()) {
            e = expr->getLHS();
            break;
        }
        // fall through
    case BO_Assign:
        if (expr->getLHS()->getType()->isPointerType()) {
            e = expr->getRHS();
            break;
        }
        // fall through
    default:
        return true;
    }
    //TODO: detect NPCK_ZeroExpression where appropriate
    auto const lit = dyn_cast<IntegerLiteral>(e->IgnoreParenImpCasts());
    if (lit == nullptr || lit->getValue().getBoolValue()) {
        return true;
    }
    handleNull(e, nullptr, Expr::NPCK_ZeroLiteral);
    return true;
}

bool Nullptr::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    Expr const * e;
    switch (expr->getOperator()) {
    case OO_EqualEqual:
    case OO_ExclaimEqual:
        if (expr->getArg(1)->getType()->isPointerType()) {
            e = expr->getArg(0);
            break;
        }
        // fall through
    case OO_Equal:
        if (expr->getArg(0)->getType()->isPointerType()) {
            e = expr->getArg(1);
            break;
        }
        // fall through
    default:
        return true;
    }
    //TODO: detect NPCK_ZeroExpression where appropriate
    auto const lit = dyn_cast<IntegerLiteral>(e->IgnoreParenImpCasts());
    if (lit == nullptr || lit->getValue().getBoolValue()) {
        return true;
    }
    handleNull(e, nullptr, Expr::NPCK_ZeroLiteral);
    return true;
}

bool Nullptr::isInLokIncludeFile(SourceLocation spellingLocation) const {
    return compiler.getSourceManager().getFilename(spellingLocation)
        .startswith(SRCDIR "/include/LibreOfficeKit/");
}

bool Nullptr::isFromCIncludeFile(SourceLocation spellingLocation) const {
    return !compat::isInMainFile(compiler.getSourceManager(), spellingLocation)
        && (StringRef(
                compiler.getSourceManager().getPresumedLoc(spellingLocation)
                .getFilename())
            .endswith(".h"));
}

bool Nullptr::isMacroBodyExpansion(SourceLocation location) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return compiler.getSourceManager().isMacroBodyExpansion(location);
#else
    return location.isMacroID()
        && !compiler.getSourceManager().isMacroArgExpansion(location);
#endif
}

void Nullptr::handleNull(
    Expr const * expr, char const * castKind,
    Expr::NullPointerConstantKind nullPointerKind)
{
    auto e = expr;
    SourceLocation loc;
    for (;;) {
        e = e->IgnoreImpCasts();
        loc = e->getLocStart();
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (isMacroBodyExpansion(loc)) {
            if (Lexer::getImmediateMacroName(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "NULL")
            {
                if (!compiler.getLangOpts().CPlusPlus) {
                    //TODO: if !castKind, warn if NULL is passed into fn call
                    // ellipsis, cast to void*
                    return;
                }
                loc = compiler.getSourceManager()
                    .getImmediateExpansionRange(loc).first;
                if (isInUnoIncludeFile(
                        compiler.getSourceManager().getSpellingLoc(loc))
                    || isInLokIncludeFile(
                        compiler.getSourceManager().getSpellingLoc(loc))
                    || isFromCIncludeFile(
                        compiler.getSourceManager().getSpellingLoc(loc)))
                {
                    //TODO: if !castKind, warn if NULL is passed into fn call
                    // ellipsis, cast to void*
                    return;
                }
            } else if (ignoreLocation(
                           compiler.getSourceManager().getSpellingLoc(loc)))
            {
                return;
            }
        }
        ParenExpr const * pe = dyn_cast<ParenExpr>(e);
        if (pe == nullptr) {
            break;
        }
        e = pe->getSubExpr();
    }
    if (nullPointerKind == Expr::NPCK_GNUNull) {
        if (castKind == nullptr) {
            if (gnuNulls_.erase(expr) == 1) {
                return;
            }
        } else {
            auto const ok = gnuNulls_.insert(expr).second;
            assert(ok); (void) ok;
        }
    }
    auto const asMacro = !compiler.getLangOpts().CPlusPlus
        || isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(loc))
        || isInLokIncludeFile(compiler.getSourceManager().getSpellingLoc(loc))
        || isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(loc));
    assert(!asMacro || nullPointerKind != Expr::NPCK_GNUNull);
    rewriteOrWarn(e, castKind, nullPointerKind, asMacro ? "NULL" : "nullptr");
}

void Nullptr::rewriteOrWarn(
    Expr const * expr, char const * castKind,
    Expr::NullPointerConstantKind nullPointerKind, char const * replacement)
{
    if (rewriter != nullptr) {
        SourceLocation locStart(expr->getLocStart());
        while (compiler.getSourceManager().isMacroArgExpansion(locStart)) {
            locStart = compiler.getSourceManager()
                .getImmediateMacroCallerLoc(locStart);
        }
        if (compiler.getLangOpts().CPlusPlus && isMacroBodyExpansion(locStart)
            && (Lexer::getImmediateMacroName(
                    locStart, compiler.getSourceManager(),
                    compiler.getLangOpts())
                == "NULL"))
        {
            locStart = compiler.getSourceManager().getImmediateExpansionRange(
                locStart).first;
        }
        SourceLocation locEnd(expr->getLocEnd());
        while (compiler.getSourceManager().isMacroArgExpansion(locEnd)) {
            locEnd = compiler.getSourceManager()
                .getImmediateMacroCallerLoc(locEnd);
        }
        if (compiler.getLangOpts().CPlusPlus && isMacroBodyExpansion(locEnd)
            && (Lexer::getImmediateMacroName(
                    locEnd, compiler.getSourceManager(),
                    compiler.getLangOpts())
                == "NULL"))
        {
            locEnd = compiler.getSourceManager().getImmediateExpansionRange(
                locEnd).first;
        }
        if (replaceText(SourceRange(compiler.getSourceManager().getSpellingLoc(locStart), compiler.getSourceManager().getSpellingLoc(locEnd)), replacement)) {
            return;
        }
    }
    if (castKind == nullptr) {
        report(DiagnosticsEngine::Warning, "%0 -> %1", expr->getLocStart())
            << kindName(nullPointerKind) << replacement
            << expr->getSourceRange();
    } else {
        report(
            DiagnosticsEngine::Warning, "%0 ValueDependentIsNotNull %1 -> %2",
            expr->getLocStart())
            << castKind << kindName(nullPointerKind) << replacement
            << expr->getSourceRange();
    }
}

loplugin::Plugin::Registration<Nullptr> X("nullptr", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
