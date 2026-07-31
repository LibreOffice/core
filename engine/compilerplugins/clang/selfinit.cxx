/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <vector>

#include <clang/AST/ParentMapContext.h>

#include "config_clang.h"

#include "plugin.hxx"

// Warn when a variable is referenced from its own initializer.  This is not invalid in general (see
// C++17 [basic.life]), but is at least suspicious.

namespace
{
class SelfInit : public loplugin::FilteringPlugin<SelfInit>
{
public:
    explicit SelfInit(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool PreTraverseVarDecl(VarDecl* decl)
    {
        decls_.push_back({ decl, decl->getCanonicalDecl() });
        return true;
    }
    bool PostTraverseVarDecl(VarDecl*, bool)
    {
        decls_.pop_back();
        return true;
    }
    bool TraverseVarDecl(VarDecl* decl)
    {
        PreTraverseVarDecl(decl);
        auto const ret = FilteringPlugin::TraverseVarDecl(decl);
        PostTraverseVarDecl(decl, ret);
        return ret;
    }

    bool PreTraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* expr)
    {
        if (expr->getKind() == UETT_SizeOf)
            return false;
        return true;
    }
    bool TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* expr)
    {
        if (PreTraverseUnaryExprOrTypeTraitExpr(expr))
            return FilteringPlugin::TraverseUnaryExprOrTypeTraitExpr(expr);
        return true;
    }

    bool TraverseCXXTypeidExpr(CXXTypeidExpr*) { return true; }
    bool PreTraverseCXXTypeidExpr(CXXTypeidExpr*) { return false; }

    bool TraverseCXXNoexceptExpr(CXXNoexceptExpr*) { return true; }
    bool PreTraverseCXXNoexceptExpr(CXXNoexceptExpr*) { return false; }

    bool TraverseDecltypeTypeLoc(DecltypeTypeLoc
#if CLANG_VERSION >= 220000
                                 ,
                                 bool
#endif
    )
    {
        return true;
    }
    bool PreTraverseDecltypeTypeLoc(DecltypeTypeLoc
#if CLANG_VERSION >= 220000
                                    ,
                                    bool
#endif
    )
    {
        return false;
    }

    bool VisitDeclRefExpr(DeclRefExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        for (auto const& i : decls_)
        {
            if (expr->getDecl()->getCanonicalDecl() == i.canonical
                && !referencedThroughDeferredLambda(expr, i.current))
            {
                report(
                    DiagnosticsEngine::Warning,
                    ("referencing a variable during its own initialization is error-prone and thus"
                     " suspicious"),
                    expr->getLocation())
                    << expr->getSourceRange();
                report(DiagnosticsEngine::Note, "variable declared here", i.current->getLocation())
                    << i.current->getSourceRange();
            }
        }
        return true;
    }

private:
    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool referencedThroughDeferredLambda(DeclRefExpr const* ref, VarDecl const* decl)
    {
        auto node = DynTypedNode::create(*ref);
        for (;;)
        {
            auto const parents = compiler.getASTContext().getParents(node);
            if (parents.size() != 1)
            {
                return false;
            }
            node = parents[0];
            if (node.get<VarDecl>() == decl)
            {
                return false;
            }
            if (auto const lambda = node.get<LambdaExpr>())
            {
                if (!isImmediatelyInvoked(lambda))
                {
                    return true;
                }
            }
            if (node.get<TranslationUnitDecl>() != nullptr)
            {
                return false;
            }
        }
    }

    bool isImmediatelyInvoked(LambdaExpr const* lambda)
    {
        Stmt const* node = lambda;
        for (;;)
        {
            auto const parents = compiler.getASTContext().getParents(*node);
            if (parents.size() != 1)
            {
                return false;
            }
            auto const parent = parents[0].get<Stmt>();
            if (parent == nullptr)
            {
                return false;
            }
            if (auto const call = dyn_cast<CXXOperatorCallExpr>(parent))
            {
                return call->getOperator() == OO_Call && call->getNumArgs() != 0
                       && call->getArg(0)->IgnoreImplicit() == lambda;
            }
            if (isa<ParenExpr>(parent) || isa<ImplicitCastExpr>(parent)
                || isa<MaterializeTemporaryExpr>(parent) || isa<CXXBindTemporaryExpr>(parent))
            {
                node = parent;
                continue;
            }
            return false;
        }
    }

    struct Decl
    {
        VarDecl const* current;
        VarDecl const* canonical;
    };

    std::vector<Decl> decls_;
};

loplugin::Plugin::Registration<SelfInit> selfinit("selfinit");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
