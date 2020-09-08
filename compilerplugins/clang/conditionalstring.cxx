/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Find uses of OUString in conditional expressions that could be rewritten as std::u16string_view,
// as in
//
//   s += (b ? OUString("xy") : OUString(z");

namespace
{
// Like Expr::IgnoreImplicit, but for an ImplicitCastExpr skips to getSubExprAsWritten (so skips a
// CXXConstructExpr where Expr::IgnoreImplicit would stop):
Expr const* ignoreImplicit(Expr const* expr)
{
    for (auto e = expr;;)
    {
        if (auto const e1 = dyn_cast<ImplicitCastExpr>(e))
        {
            e = e1->getSubExprAsWritten();
        }
#if CLANG_VERSION >= 80000
        else if (auto const e2 = dyn_cast<FullExpr>(e))
        {
            e = e2->getSubExpr();
        }
#endif
        else if (auto const e3 = dyn_cast<MaterializeTemporaryExpr>(e))
        {
            e = compat::getSubExpr(e3);
        }
        else if (auto const e4 = dyn_cast<CXXBindTemporaryExpr>(e))
        {
            e = e4->getSubExpr();
        }
        else
        {
            return e;
        }
    }
}

class ConditionalString final : public loplugin::FilteringPlugin<ConditionalString>
{
public:
    explicit ConditionalString(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitCallExpr(CallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const fn = expr->getDirectCallee();
        if (fn == nullptr)
        {
            return true;
        }
        //TODO: Instead of a hardcoded list of functions, check that `fn` has overloads taking
        // OUString and std::u16string_view, respectively (and operator + is even more complicated
        // than that, going via ToStringHelper<std::u16string_view> specialization; the getNumArgs
        // checks for the various functions are meant to guard against the unlikely case that the
        // affected parameters get defaulted in the future; overloaded operators cannot generally
        // have defaulted parameters):
        loplugin::DeclCheck const dc(fn);
        if (dc.Operator(OO_Equal).Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            assert(fn->getNumParams() == 1);
            if (isa<CXXOperatorCallExpr>(expr))
            {
                assert(expr->getNumArgs() == 2);
                check(expr->getArg(1));
            }
            else
            {
                assert(expr->getNumArgs() == 1);
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Operator(OO_PlusEqual).Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            assert(fn->getNumParams() == 1);
            if (isa<CXXOperatorCallExpr>(expr))
            {
                assert(expr->getNumArgs() == 2);
                check(expr->getArg(1));
            }
            else
            {
                assert(expr->getNumArgs() == 1);
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("reverseCompareTo").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 1)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("equalsIgnoreAsciiCase")
                .Class("OUString")
                .Namespace("rtl")
                .GlobalNamespace()
            && fn->getNumParams() == 1)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("match").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("matchIgnoreAsciiCase").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("startsWith").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("startsWithIgnoreAsciiCase")
                .Class("OUString")
                .Namespace("rtl")
                .GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("endsWith").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("endsWithIgnoreAsciiCase")
                .Class("OUString")
                .Namespace("rtl")
                .GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Operator(OO_EqualEqual)
                .Namespace("rtl")
                .GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_ExclaimEqual)
                .Namespace("rtl")
                .GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_Less).Namespace("rtl").GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_LessEqual)
                .Namespace("rtl")
                .GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_Greater).Namespace("rtl").GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_GreaterEqual)
                .Namespace("rtl")
                .GlobalNamespace()) //TODO: more precicse check
        {
            assert(fn->getNumParams() == 2);
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Function("indexOf").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("lastIndexOf").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 1)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("replaceFirst").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 3)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            if (expr->getNumArgs() >= 2)
            {
                check(expr->getArg(1));
            }
            return true;
        }
        if (dc.Function("replaceAll").Class("OUString").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            if (expr->getNumArgs() >= 2)
            {
                check(expr->getArg(1));
            }
            return true;
        }
        if (dc.Operator(OO_Plus).Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2) //TODO: more precicse check
        {
            assert(expr->getNumArgs() == 2);
            check(expr->getArg(0));
            check(expr->getArg(1));
            return true;
        }
        if (dc.Operator(OO_Equal).Class("OUStringBuffer").Namespace("rtl").GlobalNamespace())
        {
            assert(fn->getNumParams() == 1);
            if (isa<CXXOperatorCallExpr>(expr))
            {
                assert(expr->getNumArgs() == 2);
                check(expr->getArg(1));
            }
            else
            {
                assert(expr->getNumArgs() == 1);
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("append").Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 1)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("insert").Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 2)
            {
                check(expr->getArg(1));
            }
            return true;
        }
        if (dc.Function("indexOf").Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 2)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        if (dc.Function("lastIndexOf").Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            && fn->getNumParams() == 1)
        {
            if (expr->getNumArgs() >= 1)
            {
                check(expr->getArg(0));
            }
            return true;
        }
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    enum class Kind
    {
        OUStringFromLiteral,
        StringViewOrVoid,
        Other
    };

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    Kind getKind(Expr const* expr)
    {
        auto const tc = loplugin::TypeCheck(ignoreImplicit(expr)->getType());
        if (tc.ClassOrStruct("basic_string_view").StdNamespace() //TODO: check explicitly for
            // std::basic_string_view<char16_t>
            || tc.Void())
        {
            return Kind::StringViewOrVoid;
        }
        if (loplugin::TypeCheck(expr->getType())
                .Class("OUString")
                .Namespace("rtl")
                .GlobalNamespace())
        {
            // Check for both explicit
            //
            //   OUString("...")
            //
            // and implicit
            //
            //    "..."
            //
            // expressions:
            auto e = expr->IgnoreParens();
            if (auto const e1 = dyn_cast<CXXFunctionalCastExpr>(e))
            {
                e = e1->getSubExpr();
            }
            if (auto const e1
                = dyn_cast<CXXConstructExpr>(compat::IgnoreImplicit(e)->IgnoreParens()))
            {
                if (e1->getNumArgs() != 0 //TODO
                    && isa<clang::StringLiteral>(e1->getArg(0)->IgnoreParenImpCasts()))
                {
                    return Kind::OUStringFromLiteral;
                }
            }
        }
        return Kind::Other;
    }

    void check(Expr const* expr)
    {
        //TODO: skip `,`; handle ?: chains
        auto const cond = dyn_cast<ConditionalOperator>(expr->IgnoreParenImpCasts());
        if (cond == nullptr)
        {
            return;
        }
        auto const k1 = getKind(cond->getTrueExpr());
        if (k1 == Kind::Other)
        {
            return;
        }
        auto const k2 = getKind(cond->getFalseExpr());
        if (k2 == Kind::Other || (k1 == Kind::StringViewOrVoid && k2 == Kind::StringViewOrVoid))
        {
            return;
        }
        if (k1 == Kind::OUStringFromLiteral && k2 == Kind::OUStringFromLiteral)
        {
            report(DiagnosticsEngine::Warning,
                   ("replace both 2nd and 3rd operands of conditional expression with"
                    " `std::u16string_view`"),
                   cond->getExprLoc())
                << cond->getSourceRange();
        }
        else
        {
            assert((k1 == Kind::OUStringFromLiteral && k2 == Kind::StringViewOrVoid)
                   || (k1 == Kind::StringViewOrVoid && k2 == Kind::OUStringFromLiteral));
            auto const second = k1 == Kind::OUStringFromLiteral;
            auto const sub
                = (second ? cond->getTrueExpr() : cond->getFalseExpr())->IgnoreParenImpCasts();
            report(DiagnosticsEngine::Warning,
                   ("replace %select{2nd|3rd}0 operand of conditional expression with"
                    " `std::u16string_view`"),
                   sub->getExprLoc())
                << (second ? 0 : 1) << sub->getSourceRange();
            report(DiagnosticsEngine::Note, "conditional expression is here", cond->getExprLoc())
                << cond->getSourceRange();
        }
    }
};

loplugin::Plugin::Registration<ConditionalString> conditionalstring("conditionalstring");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
