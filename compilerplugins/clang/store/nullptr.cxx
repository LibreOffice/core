/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

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

private:
    bool isInLokIncludeFile(SourceLocation spellingLocation) const;

    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isMacroBodyExpansion(SourceLocation location) const;

    void rewriteOrWarn(
        Expr const & expr, char const * castKind,
        Expr::NullPointerConstantKind nullPointerkind,
        char const * replacement);
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
                "suspicious ValueDependendIsNull %0", expr->getLocStart())
                << kindName(k) << expr->getSourceRange();
            break;
        default:
            assert(false); // cannot happen
        }
        break;
    case Expr::NPCK_CXX11_nullptr:
        break;
    default:
        {
            Expr const * e = expr->getSubExpr();
            SourceLocation loc;
            for (;;) {
                e = e->IgnoreImpCasts();
                loc = e->getLocStart();
                while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                    loc = compiler.getSourceManager()
                        .getImmediateMacroCallerLoc(
                            loc);
                }
                if (isMacroBodyExpansion(loc)) {
                    if (Lexer::getImmediateMacroName(
                            loc, compiler.getSourceManager(),
                            compiler.getLangOpts())
                        == "NULL")
                    {
                        if (!compiler.getLangOpts().CPlusPlus) {
                            return true;
                        }
                        loc = compiler.getSourceManager()
                            .getImmediateExpansionRange(loc).first;
                        if (isInUnoIncludeFile(
                                compiler.getSourceManager().getSpellingLoc(loc))
                            || isInLokIncludeFile(
                                compiler.getSourceManager().getSpellingLoc(loc))
                            || isFromCIncludeFile(
                                compiler.getSourceManager().getSpellingLoc(
                                    loc)))
                        {
                            return true;
                        }
                    } else if (ignoreLocation(
                                   compiler.getSourceManager().getSpellingLoc(
                                       loc)))
                    {
                        return true;
                    }
                }
                ParenExpr const * pe = dyn_cast<ParenExpr>(e);
                if (pe == nullptr) {
                    break;
                }
                e = pe->getSubExpr();
            }
            rewriteOrWarn(
                *e, expr->getCastKindName(), k,
                ((!compiler.getLangOpts().CPlusPlus
                  || isInUnoIncludeFile(
                      compiler.getSourceManager().getSpellingLoc(loc))
                  || isInLokIncludeFile(
                      compiler.getSourceManager().getSpellingLoc(loc))
                  || isFromCIncludeFile(
                      compiler.getSourceManager().getSpellingLoc(loc)))
                 ? "NULL" : "nullptr"));
        }
        break;
    }
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

void Nullptr::rewriteOrWarn(
    Expr const & expr, char const * castKind,
    Expr::NullPointerConstantKind nullPointerKind, char const * replacement)
{
    if (rewriter != nullptr) {
        SourceLocation locStart(expr.getLocStart());
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
        SourceLocation locEnd(expr.getLocEnd());
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
    report(
        DiagnosticsEngine::Warning, "%0 ValueDependendIsNotNull %1 -> %2",
        expr.getLocStart())
        << castKind << kindName(nullPointerKind) << replacement
        << expr.getSourceRange();
}

loplugin::Plugin::Registration<Nullptr> X("nullptr", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
