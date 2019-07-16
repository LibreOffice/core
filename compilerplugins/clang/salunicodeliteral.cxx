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

bool isAsciiCharacterLiteral(Expr const * expr) {
    if (auto const e = dyn_cast<CharacterLiteral>(expr)) {
        return e->getKind() == CharacterLiteral::Ascii;
    }
    return false;
}

class SalUnicodeLiteral final:
    public loplugin::FilteringPlugin<SalUnicodeLiteral>
{
public:
    explicit SalUnicodeLiteral(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

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

    bool preRun() override {
        return compiler.getLangOpts().CPlusPlus
            && compiler.getPreprocessor().getIdentifierInfo(
                "LIBO_INTERNAL_ONLY")->hasMacroDefinition();
    }

    void run() override {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

private:
    void check(ExplicitCastExpr const * expr) {
        if (ignoreLocation(expr)
            || isInUnoIncludeFile(expr->getExprLoc()))
                //TODO: '#ifdef LIBO_INTERNAL_ONLY' within UNO include files
        {
            return;
        }
        for (auto t = expr->getTypeAsWritten();;) {
            auto const tt = t->getAs<TypedefType>();
            if (tt == nullptr) {
                return;
            }
            if (loplugin::TypeCheck(t).Typedef("sal_Unicode")
                .GlobalNamespace())
            {
                break;
            }
            t = tt->desugar();
        }
        auto const e1 = expr->getSubExprAsWritten();
        auto const loc = compat::getBeginLoc(e1);
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

static loplugin::Plugin::Registration<SalUnicodeLiteral> salunicodeliteral("salunicodeliteral");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
