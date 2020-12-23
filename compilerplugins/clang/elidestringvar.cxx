/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <algorithm>
#include <cassert>
#include <map>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Find cases where a variable of a OString/OUString type is initialized
// with a literal value (incl. as an empty string) and used only once.  Conservatively this only
// covers local non-static variables that are not defined outside of the loop (if any) in which they
// are used, as other cases may deliberately use the variable for performance (or even correctness,
// if addresses are taken and compared) reasons.
//
// For one, the historically heavy syntax for such uses of string literals
// (RTL_CONSTASCII_USTRINGPARAM etc.) probably explains many of these redundant variables, which can
// now be considered cargo-cult baggage.  For another, some of those variables are used as arguments
// to functions which also have more efficient overloads directly taking string literals.  And for
// yet another, some cases with default-initialized variables turned out to be effectively unused
// code that could be removed completely (d073cca5f7c04de3e1bcedda334d864e98ac7835 "Clean up dead
// code", 91345e7dde6100496a7c9e815b72b2821ae82bc2 "Clean up dead code",
// 868b0763ac47f765cb48c277897274a595b831d0 "Upcoming loplugin:elidestringvar: dbaccess" in
// dbaccess/source/ui/app/AppController.cxx, bde0aac4ccf7b830b5ef21d5b9e75e62aee6aaf9 "Clean up dead
// code", 354aefec42de856b4ab6201ada54a3a3c630b4bd "Upcoming loplugin:elidestringvar: cui" in
// cui/source/dialogs/SpellDialog.cxx).

namespace
{
bool isStringType(QualType type)
{
    loplugin::TypeCheck const c(type);
    return c.Class("OString").Namespace("rtl").GlobalNamespace()
           || c.Class("OUString").Namespace("rtl").GlobalNamespace();
}

class ElideStringVar : public loplugin::FilteringPlugin<ElideStringVar>
{
public:
    explicit ElideStringVar(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void postRun() override
    {
        for (auto const& var : vars_)
        {
            if (!var.second.singleUse || *var.second.singleUse == nullptr)
            {
                continue;
            }
            if (containsPreprocessingConditionalInclusion(SourceRange(
                    compat::getBeginLoc(var.first), compat::getEndLoc(*var.second.singleUse))))
            {
                // This is not perfect, as additional uses can be hidden in conditional blocks that
                // only start after the (would-be) single use (as was the case in
                // 3bc5057f9689e024957cfa898a221ee2c4c4afe7 "Upcoming loplugin:elidestringvar:
                // testtools" when built with --enable-debug, but where also fixing the hidden
                // additional use was trivial).  If this ever becomes a real problem, we can extend
                // the above check to cover more of the current function body's remainder.
                continue;
            }
            report(DiagnosticsEngine::Warning,
                   "replace single use of literal %0 variable with a literal",
                   (*var.second.singleUse)->getExprLoc())
                << var.first->getType() << (*var.second.singleUse)->getSourceRange();
            report(DiagnosticsEngine::Note, "literal %0 variable defined here",
                   var.first->getLocation())
                << var.first->getType() << var.first->getSourceRange();
        }
    }

    bool VisitVarDecl(VarDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (isa<ParmVarDecl>(decl))
        {
            return true;
        }
        if (decl->getStorageDuration() != SD_Automatic)
        {
            return true;
        }
        if (!isStringType(decl->getType()))
        {
            return true;
        }
        if (!decl->hasInit())
        {
            return true;
        }
        auto const e1 = dyn_cast<CXXConstructExpr>(decl->getInit()->IgnoreParenImpCasts());
        if (e1 == nullptr)
        {
            return true;
        }
        if (!isStringType(e1->getType()))
        {
            return true;
        }
        switch (e1->getNumArgs())
        {
            case 0:
                break;
            case 1:
            {
                auto const e2 = e1->getArg(0);
                loplugin::TypeCheck const c(e2->getType());
                if (c.Class("OStringLiteral").Namespace("rtl").GlobalNamespace()
                    || c.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace())
                {
                    break;
                }
                if (!e2->isValueDependent() && e2->isIntegerConstantExpr(compiler.getASTContext()))
                {
                    break;
                }
                return true;
            }
            case 2:
            {
                auto const e2 = e1->getArg(0);
                auto const t = e2->getType();
                if (!(t.isConstQualified() && t->isConstantArrayType()))
                {
                    return true;
                }
                if (isa<AbstractConditionalOperator>(e2->IgnoreParenImpCasts()))
                {
                    return true;
                }
                auto const e3 = e1->getArg(1);
                if (!(isa<CXXDefaultArgExpr>(e3)
                      && loplugin::TypeCheck(e3->getType())
                             .Struct("Dummy")
                             .Namespace("libreoffice_internal")
                             .Namespace("rtl")
                             .GlobalNamespace()))
                {
                    return true;
                }
                break;
            }
            default:
                return true;
        }
        auto const ok = vars_.emplace(decl, Data(getInnermostLoop()));
        assert(ok.second);
        (void)ok;
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const var = dyn_cast<VarDecl>(expr->getDecl());
        if (var == nullptr)
        {
            return true;
        }
        auto const i = vars_.find(var);
        if (i == vars_.end())
        {
            return true;
        }
        i->second.singleUse
            = i->second.singleUse || i->second.innermostLoop != getInnermostLoop() ? nullptr : expr;
        return true;
    }

    bool VisitMemberExpr(MemberExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const e = dyn_cast<DeclRefExpr>(expr->getBase()->IgnoreParenImpCasts());
        if (e == nullptr)
        {
            return true;
        }
        auto const var = dyn_cast<VarDecl>(e->getDecl());
        if (var == nullptr)
        {
            return true;
        }
        auto const i = vars_.find(var);
        if (i == vars_.end())
        {
            return true;
        }
        i->second.singleUse = nullptr;
        return true;
    }

    bool VisitUnaryOperator(UnaryOperator const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getOpcode() != UO_AddrOf)
        {
            return true;
        }
        auto const e = dyn_cast<DeclRefExpr>(expr->getSubExpr()->IgnoreParenImpCasts());
        if (e == nullptr)
        {
            return true;
        }
        auto const var = dyn_cast<VarDecl>(e->getDecl());
        if (var == nullptr)
        {
            return true;
        }
        auto const i = vars_.find(var);
        if (i == vars_.end())
        {
            return true;
        }
        i->second.singleUse = nullptr;
        return true;
    }

    bool VisitCallExpr(CallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const fun = expr->getDirectCallee();
        if (fun == nullptr)
        {
            return true;
        }
        unsigned const n = std::min(fun->getNumParams(), expr->getNumArgs());
        for (unsigned i = 0; i != n; ++i)
        {
            if (!loplugin::TypeCheck(fun->getParamDecl(i)->getType())
                     .LvalueReference()
                     .NonConstVolatile())
            {
                continue;
            }
            auto const e = dyn_cast<DeclRefExpr>(expr->getArg(i)->IgnoreParenImpCasts());
            if (e == nullptr)
            {
                continue;
            }
            auto const var = dyn_cast<VarDecl>(e->getDecl());
            if (var == nullptr)
            {
                continue;
            }
            auto const j = vars_.find(var);
            if (j == vars_.end())
            {
                continue;
            }
            j->second.singleUse = nullptr;
        }
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const ctor = expr->getConstructor();
        unsigned const n = std::min(ctor->getNumParams(), expr->getNumArgs());
        for (unsigned i = 0; i != n; ++i)
        {
            if (!loplugin::TypeCheck(ctor->getParamDecl(i)->getType())
                     .LvalueReference()
                     .NonConstVolatile())
            {
                continue;
            }
            auto const e = dyn_cast<DeclRefExpr>(expr->getArg(i)->IgnoreParenImpCasts());
            if (e == nullptr)
            {
                continue;
            }
            auto const var = dyn_cast<VarDecl>(e->getDecl());
            if (var == nullptr)
            {
                continue;
            }
            auto const j = vars_.find(var);
            if (j == vars_.end())
            {
                continue;
            }
            j->second.singleUse = nullptr;
        }
        return true;
    }

    bool TraverseWhileStmt(WhileStmt* stmt)
    {
        bool ret = true;
        if (PreTraverseWhileStmt(stmt))
        {
            ret = FilteringPlugin::TraverseWhileStmt(stmt);
            PostTraverseWhileStmt(stmt, ret);
        }
        return ret;
    }

    bool PreTraverseWhileStmt(WhileStmt* stmt)
    {
        innermostLoop_.push(stmt);
        return true;
    }

    bool PostTraverseWhileStmt(WhileStmt* stmt, bool)
    {
        assert(!innermostLoop_.empty());
        assert(innermostLoop_.top() == stmt);
        (void)stmt;
        innermostLoop_.pop();
        return true;
    }

    bool TraverseDoStmt(DoStmt* stmt)
    {
        bool ret = true;
        if (PreTraverseDoStmt(stmt))
        {
            ret = FilteringPlugin::TraverseDoStmt(stmt);
            PostTraverseDoStmt(stmt, ret);
        }
        return ret;
    }

    bool PreTraverseDoStmt(DoStmt* stmt)
    {
        innermostLoop_.push(stmt);
        return true;
    }

    bool PostTraverseDoStmt(DoStmt* stmt, bool)
    {
        assert(!innermostLoop_.empty());
        assert(innermostLoop_.top() == stmt);
        (void)stmt;
        innermostLoop_.pop();
        return true;
    }

    bool TraverseForStmt(ForStmt* stmt)
    {
        bool ret = true;
        if (PreTraverseForStmt(stmt))
        {
            ret = FilteringPlugin::TraverseForStmt(stmt);
            PostTraverseForStmt(stmt, ret);
        }
        return ret;
    }

    bool PreTraverseForStmt(ForStmt* stmt)
    {
        innermostLoop_.push(stmt);
        return true;
    }

    bool PostTraverseForStmt(ForStmt* stmt, bool)
    {
        assert(!innermostLoop_.empty());
        assert(innermostLoop_.top() == stmt);
        (void)stmt;
        innermostLoop_.pop();
        return true;
    }

    bool TraverseCXXForRangeStmt(CXXForRangeStmt* stmt)
    {
        bool ret = true;
        if (PreTraverseCXXForRangeStmt(stmt))
        {
            ret = FilteringPlugin::TraverseCXXForRangeStmt(stmt);
            PostTraverseCXXForRangeStmt(stmt, ret);
        }
        return ret;
    }

    bool PreTraverseCXXForRangeStmt(CXXForRangeStmt* stmt)
    {
        innermostLoop_.push(stmt);
        return true;
    }

    bool PostTraverseCXXForRangeStmt(CXXForRangeStmt* stmt, bool)
    {
        assert(!innermostLoop_.empty());
        assert(innermostLoop_.top() == stmt);
        (void)stmt;
        innermostLoop_.pop();
        return true;
    }

private:
    void run() override
    {
        if (preRun() && TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        {
            postRun();
        }
    }

    Stmt const* getInnermostLoop() const
    {
        return innermostLoop_.empty() ? nullptr : innermostLoop_.top();
    }

    struct Data
    {
        Data(Stmt const* theInnermostLoop)
            : innermostLoop(theInnermostLoop)
        {
        }
        Stmt const* innermostLoop;
        llvm::Optional<Expr const*> singleUse;
    };

    std::stack<Stmt const*> innermostLoop_;
    std::map<VarDecl const*, Data> vars_;
};

loplugin::Plugin::Registration<ElideStringVar> elidestringvar("elidestringvar");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
