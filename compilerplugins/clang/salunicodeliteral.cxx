/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"

namespace {

bool isAsciiCharacterLiteral(Expr const * expr) {
    if (auto const e = dyn_cast<CharacterLiteral>(expr)) {
        return e->getKind() == CharacterLiteral::Ascii;
    }
    return false;
}

class Visitor final:
    public RecursiveASTVisitor<Visitor>, public loplugin::Plugin
{
public:
    explicit Visitor(InstantiationData const & data): Plugin(data) {}

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr) {
        check(expr);
        return true;
    }

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr) {
        check(expr);
        return true;
    }

    bool VisitCStyleCastExpr(CStyleCastExpr const * expr) {
        check(expr);
        return true;
    }

private:
    void run() override {
        if (compiler.getLangOpts().CPlusPlus
            && compiler.getPreprocessor().getIdentifierInfo(
                "LIBO_INTERNAL_ONLY")->hasMacroDefinition())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    void check(ExplicitCastExpr const * expr) {
        if (ignoreLocation(expr)
            || isInUnoIncludeFile(expr->getExprLoc())
                //TODO: '#ifdef LIBO_INTERNAL_ONLY' within UNO include files
            || !(loplugin::TypeCheck(expr->getTypeAsWritten())
                 .Typedef("sal_Unicode").GlobalNamespace()))
        {
            return;
        }
        auto const e1 = expr->getSubExprAsWritten();
        auto const loc = e1->getLocStart();
        if (loc.isMacroID()
            && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(
                loc))
        {
            return;
        }
        auto const e2 = e1->IgnoreParenImpCasts();
        if (isAsciiCharacterLiteral(e2) || isa<IntegerLiteral>(e2)) {
            report(
                DiagnosticsEngine::Warning,
                ("in LIBO_INTERNAL_ONLY code, replace literal cast to %0 with a"
                 " u'...' char16_t character literal"),
                e2->getExprLoc())
                << expr->getTypeAsWritten() << expr->getSourceRange();
        }
    }
};

static loplugin::Plugin::Registration<Visitor> reg("salunicodeliteral");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
