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
#include <stack>

#include "check.hxx"
#include "plugin.hxx"

// Find matches of
//
//   ... << s.getStr()
//
// (for the rtl string classes) that can be written as just
//
//   ... << s

namespace
{
class GetStr final : public loplugin::FilteringPlugin<GetStr>
{
public:
    explicit GetStr(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool PreTraverseFunctionDecl(FunctionDecl* decl)
    {
        functions_.push(decl);
        return true;
    }

    bool PostTraverseFunctionDecl(FunctionDecl*, bool)
    {
        assert(!functions_.empty());
        functions_.pop();
        return true;
    }

    bool TraverseFunctionDecl(FunctionDecl* decl)
    {
        bool ret = true;
        if (PreTraverseFunctionDecl(decl))
        {
            ret = FilteringPlugin::TraverseFunctionDecl(decl);
            PostTraverseFunctionDecl(decl, ret);
        }
        return ret;
    }

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getOperator() != OO_LessLess)
        {
            return true;
        }
        assert(expr->getNumArgs() == 2);
        if (!loplugin::TypeCheck(expr->getArg(0)->getType())
                 .ClassOrStruct("basic_ostream")
                 .StdNamespace()) //TODO: check template args
        {
            return true;
        }
        auto const arg1 = expr->getArg(1);
        auto const e = dyn_cast<CXXMemberCallExpr>(arg1->IgnoreParenImpCasts());
        if (e == nullptr)
        {
            return true;
        }
        bool castToVoid = false;
        if (isa<ImplicitCastExpr>(arg1))
        {
            if (loplugin::TypeCheck(arg1->getType()).Pointer().Void())
            {
                castToVoid = true;
            }
        }
        auto const t = e->getObjectType();
        auto const tc = loplugin::TypeCheck(t);
        if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUString").Namespace("rtl").GlobalNamespace()
              || (castToVoid
                  && (tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
                      || tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()))))
        {
            return true;
        }
        if (!loplugin::DeclCheck(e->getMethodDecl()).Function("getStr"))
        {
            return true;
        }
        if (castToVoid)
        {
            report(DiagnosticsEngine::Warning,
                   ("suspicious use of 'getStr' on an object of type %0; the result is implicitly"
                    " cast to a void pointer in a call of 'operator <<'"),
                   e->getExprLoc())
                << t.getLocalUnqualifiedType() << expr->getSourceRange();
            return true;
        }
        if (!functions_.empty())
        {
            // Filter out occurrences of `s << t.getStr()` in the implementation of
            // `operator <<(std::basic_ostream<...> & s, T const & t)`:
            auto const fd = functions_.top();
            if (fd->getOverloadedOperator() == OO_LessLess)
            {
                assert(fd->getNumParams() == 2);
                if (loplugin::TypeCheck(fd->getParamDecl(0)->getType())
                        .LvalueReference()
                        .NonConstVolatile()
                        .TemplateSpecializationClass()
                        .ClassOrStruct("basic_ostream")
                        .StdNamespace()) //TODO: check template args
                {
                    if (auto const t2
                        = fd->getParamDecl(1)->getType()->getAs<LValueReferenceType>())
                    {
                        auto const t3 = t2->getPointeeType();
                        if (t3.isConstQualified() && !t3.isVolatileQualified()
                            && (t3.getCanonicalType().getTypePtr()
                                == t.getCanonicalType().getTypePtr()))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        report(DiagnosticsEngine::Warning,
               ("directly use object of type %0 in a call of 'operator <<', instead of calling"
                " 'getStr' first"),
               e->getExprLoc())
            << t.getLocalUnqualifiedType() << expr->getSourceRange();
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    std::stack<FunctionDecl*> functions_;

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

loplugin::Plugin::Registration<GetStr> getstr("getstr");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
