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

class Visitor final:
    public RecursiveASTVisitor<Visitor>, public loplugin::RewritePlugin
{
public:
    explicit Visitor(InstantiationData const & data): RewritePlugin(data) {}

    bool VisitCXXConstructExpr(CXXConstructExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto const ctor = expr->getConstructor();
        if (!loplugin::DeclCheck(ctor->getParent()).Class("SfxItemSet")
            .GlobalNamespace())
        {
            return true;
        }
        auto const numParams = ctor->getNumParams();
        auto const variadic = ctor->isVariadic();
        if (!(((numParams == 3 && !variadic) || (numParams == 4 && variadic))
              && (loplugin::TypeCheck(ctor->getParamDecl(0)->getType())
                  .LvalueReference().Class("SfxItemPool").GlobalNamespace())))
        {
            return true;
        }
        auto const numArgs = expr->getNumArgs();
        if (numArgs < 3) {
            report(
                DiagnosticsEngine::Warning,
                ("unexpected SfxItemPool constructor call with less than three"
                 " arguments"),
                expr->getExprLoc())
                << expr->getSourceRange();
            return true;
        }
        rewrite(expr, variadic, false, numArgs, expr->getArgs());
        return true;
    }

    bool VisitCallExpr(CallExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto const dre = dyn_cast<DeclRefExpr>(
            expr->getCallee()->IgnoreParenImpCasts());
        if (dre == nullptr
            || !(loplugin::DeclCheck(dre->getDecl()).Function("make_unique")
                 .Namespace("o3tl").GlobalNamespace()))
        {
            return true;
        }
        auto const numTArgs = dre->getNumTemplateArgs();
        if (numTArgs == 0) {
            report(
                DiagnosticsEngine::Warning,
                "unexpected o3tl::make_unique call without template arguments",
                expr->getExprLoc())
                << expr->getSourceRange();
            return true;
        }
        auto const tArg0 = dre->getTemplateArgs()[0].getArgument();
        if (tArg0.getKind() != TemplateArgument::Type) {
            report(
                DiagnosticsEngine::Warning,
                ("unexpected o3tl::make_unique call with non-type first"
                 " template argument"),
                expr->getExprLoc())
                << expr->getSourceRange();
            return true;
        }
        if (!loplugin::TypeCheck(tArg0.getAsType()).Class("SfxItemSet")
            .GlobalNamespace())
        {
            return true;
        }
        auto const numArgs = expr->getNumArgs();
        if (numArgs < 3) {
            return true;
        }
        rewrite(expr, numArgs > 3, true, numArgs, expr->getArgs());
        return true;
    }

private:
    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    SourceLocation nextToken(SourceLocation loc) {
        return loc.getLocWithOffset(
            Lexer::MeasureTokenLength(
                loc, compiler.getSourceManager(), compiler.getLangOpts()));
    }

    SourceLocation atMacroExpansionStart(SourceLocation loc) {
        while (loc.isMacroID()
               && (compiler.getSourceManager()
                   .isAtStartOfImmediateMacroExpansion(loc, &loc)))
        {}
        return loc;
    }

    SourceLocation atMacroExpansionEnd(SourceLocation loc) {
        while (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateExpansionRange(loc)
                .second;
        }
        return loc;
    }

    void rewrite(
        Expr const * expr, bool variadic, bool forward, unsigned numArgs,
        Expr const * const * args)
    {
        bool constant = true;
        unsigned firstZero = 0;
        for (unsigned i = 1; i != numArgs; ++i) {
            auto const arg = args[i];
            constant = constant
                && arg->isCXX11ConstantExpr(compiler.getASTContext());
            APSInt v;
            auto const zero
                = ((arg->EvaluateAsInt(v, compiler.getASTContext())
                    && v == 0)
                   || (variadic && i > 4
                       && arg->isNullPointerConstant(
                           compiler.getASTContext(),
                           Expr::NPC_ValueDependentIsNotNull)));
            if (variadic) {
                if (zero) {
                    if (firstZero == 0) {
                        if (i == 1) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("unexpected missing non-zero arguments before"
                                 " first zero argument in SfxItemPool"
                                 " constructor call"),
                                arg->getExprLoc())
                                << expr->getSourceRange();
                            return;
                        }
                        if (i % 2 == 0) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("unexpected odd number of potentially non-zero"
                                 " arguments before first definitely zero"
                                 " argument in SfxItemPool constructor call"),
                                arg->getExprLoc())
                                << expr->getSourceRange();
                            return;
                        }
                        firstZero = i;
                    }
                } else if (firstZero != 0) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("unexpected potentially non-zero argument in"
                         " SfxItemPool constructor call, following zero"
                         " argument"),
                        arg->getExprLoc())
                        << expr->getSourceRange();
                    return;
                }
            } else if (zero) {
                report(
                    DiagnosticsEngine::Warning,
                    "unexpected zero argument in SfxItemPool constructor call",
                    arg->getExprLoc())
                    << expr->getSourceRange();
                return;
            }
        }
        if (variadic && firstZero == 0) {
            report(
                DiagnosticsEngine::Warning,
                ("unexpected SfxItemPool constructor call with no detectable"
                 " zero arguments"),
                expr->getExprLoc())
                << expr->getSourceRange();
            return;
        }
        if (rewriter != nullptr) {
            if (!insertTextBefore(
                    atMacroExpansionStart(args[1]->getLocStart()),
                    (constant
                     ? StringRef("svl::Items<")
                     : (forward
                        ? StringRef("std::initializer_list<SfxItemSet::Pair>{{")
                        : StringRef("{{")))))
            {
                goto failed;
            }
            auto const postLoc = atMacroExpansionEnd(
                args[numArgs - 1]->getLocEnd());
            auto const postStr = constant ? StringRef(">{}") : StringRef("}}");
            if (variadic) {
                //TODO: the replaced range can contain relevant comments:
                if (!replaceText(
                        SourceRange(
                            nextToken(
                                atMacroExpansionEnd(
                                    args[firstZero - 1]->getLocEnd())),
                            postLoc),
                        postStr))
                {
                    goto failed;
                }
            } else {
                if (!insertTextAfterToken(postLoc, postStr)) {
                    goto failed;
                }
            }
            if (!constant && variadic) {
                for (unsigned i = 2; i != firstZero - 1; ++i) {
                    auto const arg = args[i];
                    if (!(i % 2 == 0
                          ? insertTextAfterToken(
                              atMacroExpansionEnd(arg->getLocEnd()), "}")
                          : insertTextBefore(
                              atMacroExpansionStart(arg->getLocStart()), "{")))
                    {
                        goto failed;
                    }
                }
            }
            return;
        }
    failed: //TODO: undo partial changes
        report(
            DiagnosticsEngine::Warning,
            ("rewrite SfxItemPool constructor call with"
             " %select{%select{|std::initializer_list<SfxItemSet::Pair>}1"
             "%{%{..., ...%}, ..., %{..., ...%}%}|svl::Items<...>%{%}}0 around"
             " the %select{|leading }2%3 WID arguments%select{| and remove the"
             " remaining %4 zero %plural{1:argument|:arguments}4}2"),
            expr->getExprLoc())
            << constant << forward << variadic
            << (variadic ? firstZero - 1 : numArgs - 1) << (numArgs - firstZero)
            << expr->getSourceRange();
    }
};

static loplugin::Plugin::Registration<Visitor> reg("sfxitemsetrewrite",true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
