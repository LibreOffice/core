/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <vector>

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

    bool TraverseDecltypeTypeLoc(DecltypeTypeLoc) { return true; }
    bool PreTraverseDecltypeTypeLoc(DecltypeTypeLoc) { return false; }

    bool VisitDeclRefExpr(DeclRefExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        for (auto const& i : decls_)
        {
            if (expr->getDecl()->getCanonicalDecl() == i.canonical)
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
