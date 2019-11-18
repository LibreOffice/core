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
#include <limits>
#include <stack>

#include "clang/Basic/Builtins.h"

#include "check.hxx"
#include "plugin.hxx"

// Find implicit conversions from non-'bool' constants (e.g., 'sal_False') to 'bool'.

namespace
{
class ConstToBool final : public loplugin::FilteringPlugin<ConstToBool>
{
public:
    explicit ConstToBool(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool PreTraverseLinkageSpecDecl(LinkageSpecDecl*)
    {
        assert(externCContexts_ != std::numeric_limits<unsigned int>::max()); //TODO
        ++externCContexts_;
        return true;
    }

    bool PostTraverseLinkageSpecDecl(LinkageSpecDecl*, bool)
    {
        assert(externCContexts_ != 0);
        --externCContexts_;
        return true;
    }

    bool TraverseLinkageSpecDecl(LinkageSpecDecl* decl)
    {
        bool ret = true;
        if (PreTraverseLinkageSpecDecl(decl))
        {
            ret = FilteringPlugin::TraverseLinkageSpecDecl(decl);
            PostTraverseLinkageSpecDecl(decl, ret);
        }
        return ret;
    }

    bool PreTraverseUnaryLNot(UnaryOperator* expr)
    {
        ignoredInAssert_.push(expr->getSubExpr());
        return true;
    }

    bool PostTraverseUnaryLNot(UnaryOperator*, bool)
    {
        assert(!ignoredInAssert_.empty());
        ignoredInAssert_.pop();
        return true;
    }

    bool TraverseUnaryLNot(UnaryOperator* expr)
    {
        bool ret = true;
        if (PreTraverseUnaryLNot(expr))
        {
            ret = FilteringPlugin::TraverseUnaryLNot(expr);
            PostTraverseUnaryLNot(expr, ret);
        }
        return ret;
    }

    bool PreTraverseBinLAnd(BinaryOperator* expr)
    {
        ignoredInAssert_.push(expr->getRHS());
        return true;
    }

    bool PostTraverseBinLAnd(BinaryOperator*, bool)
    {
        assert(!ignoredInAssert_.empty());
        ignoredInAssert_.pop();
        return true;
    }

    bool TraverseBinLAnd(BinaryOperator* expr)
    {
        bool ret = true;
        if (PreTraverseBinLAnd(expr))
        {
            ret = FilteringPlugin::TraverseBinLAnd(expr);
            PostTraverseBinLAnd(expr, ret);
        }
        return ret;
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (!expr->getType()->isBooleanType())
        {
            return true;
        }
        auto const sub = expr->getSubExpr();
        auto const t = sub->getType();
        if (t->isBooleanType())
        {
            return true;
        }
        if (sub->isValueDependent())
        {
            return true;
        }
        APValue res;
        if (!sub->isCXX11ConstantExpr(compiler.getASTContext(), &res))
        {
            return true;
        }
        auto const l = expr->getExprLoc();
        if (!ignoredInAssert_.empty() && expr == ignoredInAssert_.top())
        {
            if (auto const e = dyn_cast<clang::StringLiteral>(sub->IgnoreParenImpCasts()))
            {
                if (e->isAscii()) // somewhat randomly restrict to plain literals
                {
                    if (compiler.getSourceManager().isMacroArgExpansion(l)
                        && Lexer::getImmediateMacroName(l, compiler.getSourceManager(),
                                                        compiler.getLangOpts())
                               == "assert")
                    {
                        //TODO: only ignore outermost '!"..."' or '... && "..."'
                        return true;
                    }
                }
            }
        }
        auto l1 = l;
        if (compiler.getSourceManager().isMacroBodyExpansion(l1))
        {
            auto const n = Lexer::getImmediateMacroName(l1, compiler.getSourceManager(),
                                                        compiler.getLangOpts());
            if (n == "FALSE" || n == "TRUE" || n == "sal_False" || n == "sal_True")
            {
                l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
            }
            // For example, /usr/include/glib-2.0/glib/gmacros.h from
            // glib2-devel-2.62.1-1.fc31.x86_64 has
            //
            //   #define TRUE (!FALSE)
            //
            // so handle that wrapped macro body expansion, too:
            if (compiler.getSourceManager().isMacroBodyExpansion(l1)
                && Lexer::getImmediateMacroName(l1, compiler.getSourceManager(),
                                                compiler.getLangOpts())
                       == "TRUE")
            {
                l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
            }
        }
        if (isSharedCAndCppCode(l1))
        {
            // Cover just enough cases to handle things like `while (0)` or the use of `sal_True` in
            //
            //   #define OSL_FAIL(m) SAL_DETAIL_WARN_IF_FORMAT(sal_True, "legacy.osl", "%s", m)
            //
            // in include/osl/diagnose.h:
            if (auto const t1 = t->getAs<BuiltinType>())
            {
                if (t1->getKind() == BuiltinType::Int)
                {
                    auto const& v = res.getInt();
                    if (v == 0 || v == 1)
                    {
                        return true;
                    }
                }
            }
            if (loplugin::TypeCheck(t).Typedef("sal_Bool").GlobalNamespace())
            {
                return true;
            }
        }
        if (auto const e = dyn_cast<CallExpr>(sub->IgnoreParenImpCasts()))
        {
            // Ignore use of `long __builtin_expect(long, long)`, as found in the definition of
            // `assert` on macOS:
            if (e->getBuiltinCallee() == Builtin::BI__builtin_expect)
            {
                return true;
            }
        }
        bool suggestion;
        bool replacement = {};
        if (res.isInt())
        {
            suggestion = true;
            replacement = res.getInt() != 0;
        }
        else if (res.isFloat())
        {
            suggestion = true;
            replacement = !res.getFloat().isZero();
        }
        else if (res.isNullPointer())
        {
            suggestion = true;
            replacement = false;
        }
        else if (res.isLValue())
        {
            suggestion = true;
            replacement = true;
        }
        else
        {
            suggestion = false;
        }
        report(DiagnosticsEngine::Warning,
               "implicit conversion of constant %0 of type %1 to 'bool'%select{|; use "
               "'%select{false|true}3' instead}2",
               l)
            << res.getAsString(compiler.getASTContext(), t) << t << suggestion << replacement
            << expr->getSourceRange();
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    std::stack<Expr const*> ignoredInAssert_;
    unsigned int externCContexts_ = 0;

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool isFromCIncludeFile(SourceLocation spellingLocation) const
    {
        return !compiler.getSourceManager().isInMainFile(spellingLocation)
               && (StringRef(
                       compiler.getSourceManager().getPresumedLoc(spellingLocation).getFilename())
                       .endswith(".h"));
    }

    bool isSharedCAndCppCode(SourceLocation location) const
    {
        while (compiler.getSourceManager().isMacroArgExpansion(location))
        {
            location = compiler.getSourceManager().getImmediateMacroCallerLoc(location);
        }
        // Assume that code is intended to be shared between C and C++ if it comes from an include
        // file ending in .h, and is either in an extern "C" context  or the body of a macro
        // definition:
        return isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(location))
               && (externCContexts_ != 0
                   || compiler.getSourceManager().isMacroBodyExpansion(location));
    }
};

loplugin::Plugin::Registration<ConstToBool> consttobool("consttobool");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
