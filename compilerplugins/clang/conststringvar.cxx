/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <set>
#include <stack>

#include "check.hxx"
#include "plugin.hxx"

// Find non-const vars of 'char const *' type initialized with a const expr,
// that could likely be const (and will then probably trigger further
// loplugin:stringconstant findings).

namespace {

// It looks like Clang wrongly implements DR 4
// (<http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#4>) and treats
// a variable declared in an 'extern "..." {...}'-style linkage-specification as
// if it contained the 'extern' specifier:
bool hasExternalLinkage(VarDecl const * decl) {
    if (decl->getLinkageAndVisibility().getLinkage() != ExternalLinkage) {
        return false;
    }
    for (auto ctx = decl->getLexicalDeclContext();
         ctx->getDeclKind() != Decl::TranslationUnit;
         ctx = ctx->getLexicalParent())
    {
        if (auto ls = dyn_cast<LinkageSpecDecl>(ctx)) {
            if (!ls->hasBraces()) {
                return true;
            }
            if (auto prev = decl->getPreviousDecl()) {
                return hasExternalLinkage(prev);
            }
            return !decl->isInAnonymousNamespace();
        }
    }
    return true;
}

class ConstStringVar:
    public RecursiveASTVisitor<ConstStringVar>, public loplugin::Plugin
{
public:
    explicit ConstStringVar(loplugin::InstantiationData const & data):
        Plugin(data) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
                // clang::Expr::isCXX11ConstantExpr only works for C++
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
            for (auto v: vars_) {
                report(
                    DiagnosticsEngine::Warning,
                    "variable is only used as rvalue, should be const",
                    v->getLocation())
                    << v->getSourceRange();
            }
        }
    }

    bool TraverseImplicitCastExpr(ImplicitCastExpr * expr) {
        bool match;
        switch (expr->getCastKind()) {
        case CK_NoOp:
            // OString CharPtrDetector ctor:
            match = bool(
                loplugin::TypeCheck(expr->getType()).Const().Pointer().Const()
                .Char());
            break;
        case CK_LValueToRValue:
            match = true;
            break;
        default:
            match = false;
            break;
        }
        bool pushed = false;
        if (match) {
            if (auto dr = dyn_cast<DeclRefExpr>(
                    expr->getSubExpr()->IgnoreParenImpCasts()))
            {
                if (auto vd = dyn_cast<VarDecl>(dr->getDecl())) {
                    if (vars_.find(vd->getCanonicalDecl()) != vars_.end()) {
                        casted_.push(dr);
                        pushed = true;
                    }
                }
            }
        }
        bool b = RecursiveASTVisitor::TraverseImplicitCastExpr(expr);
        if (pushed) {
            casted_.pop();
        }
        return b;
    }

    bool VisitVarDecl(VarDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        if (decl != decl->getCanonicalDecl()) {
            return true;
        }
        if (isa<ParmVarDecl>(decl) || hasExternalLinkage(decl)) {
            return true;
        }
        if (!loplugin::TypeCheck(decl->getType()).NonConstVolatile().Pointer()
            .Const().Char())
        {
            return true;
        }
        auto init = decl->getAnyInitializer();
        if (init == nullptr) {
            return true;
        }
        if (init->isInstantiationDependent()) {
            // avoid problems with isCXX11ConstantExpr in template code
            return true;
        }
        APValue v;
        if (!init->isCXX11ConstantExpr(compiler.getASTContext(), &v)) {
            return true;
        }
        vars_.insert(decl);
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr const * expr) {
        if (!casted_.empty() && expr == casted_.top()) {
            return true;
        }
        auto vd = dyn_cast<VarDecl>(expr->getDecl());
        if (vd == nullptr) {
            return true;
        }
        vars_.erase(vd->getCanonicalDecl());
        return true;
    }

private:
    std::set<VarDecl const *> vars_;
    std::stack<DeclRefExpr const *> casted_;
};

loplugin::Plugin::Registration<ConstStringVar> X("conststringvar");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
