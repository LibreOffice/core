/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find constant character array variables that are passed into O[U]String constructors and should
// thus be turned into O[U]StringLiteral variables.
//
// Such a variable may have been used in multiple places, not all of which would be compatible with
// changing the variable's type to O[U]StringLiteral.  However, this plugin is aggressive and
// ignores all but the first use of such a variable.  In all cases of incompatible uses so far, it
// was possible to change to surrounding code (for the better) to make the changes work.
//
// The plugin also flags O[U]StringLiteral variables of automatic storage duration, and uses of such
// variables with sizeof---two likely errors that can occur once a variable has been changed from a
// character array to O[U]StringLiteral.
//
//TODO:  In theory, we should not only look for variables, but also for non-static data members.  In
// practice, those should be rare, though, as they should arguably have been static data members to
// begin with.

#include <cassert>

#include "check.hxx"
#include "plugin.hxx"

namespace
{
bool isAutomaticVariable(VarDecl const* decl)
{
    switch (cast<VarDecl>(decl)->getStorageDuration())
    {
        case SD_Automatic:
            return true;
        case SD_Thread:
        case SD_Static:
            return false;
        case SD_FullExpression:
        case SD_Dynamic:
            assert(false);
        default:
            llvm_unreachable("unknown StorageDuration");
    }
}

class StringLiteralVar final : public loplugin::FilteringPlugin<StringLiteralVar>
{
public:
    explicit StringLiteralVar(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool TraverseInitListExpr(InitListExpr* expr, DataRecursionQueue* queue = nullptr)
    {
        return WalkUpFromInitListExpr(expr)
               && TraverseSynOrSemInitListExpr(
                      expr->isSemanticForm() ? expr : expr->getSemanticForm(), queue);
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        loplugin::TypeCheck const tc(expr->getType());
        if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        switch (expr->getConstructor()->getNumParams())
        {
            case 1:
            {
                auto const e = dyn_cast<DeclRefExpr>(expr->getArg(0)->IgnoreParenImpCasts());
                if (e == nullptr)
                {
                    return true;
                }
                auto const tc = loplugin::TypeCheck(e->getType());
                if (!(tc.Class("OStringLiteral").Namespace("rtl").GlobalNamespace()
                      || tc.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()))
                {
                    return true;
                }
                auto const d = e->getDecl();
                if (!isAutomaticVariable(cast<VarDecl>(d)))
                {
                    return true;
                }
                if (!reportedAutomatic_.insert(d).second)
                {
                    return true;
                }
                report(DiagnosticsEngine::Warning,
                       "variable %0 of type %1 with automatic storage duration most likely needs "
                       "to be static",
                       d->getLocation())
                    << d << d->getType() << d->getSourceRange();
                report(DiagnosticsEngine::Note, "first converted to %0 here", expr->getLocation())
                    << expr->getType() << expr->getSourceRange();
            }
            break;
            case 2:
            {
                auto const e1 = dyn_cast<DeclRefExpr>(expr->getArg(0)->IgnoreParenImpCasts());
                if (e1 == nullptr)
                {
                    return true;
                }
                auto const t = e1->getType();
                if (!(t.isConstQualified() && t->isConstantArrayType()))
                {
                    return true;
                }
                auto const e2 = expr->getArg(1);
                if (!((isa<CXXDefaultArgExpr>(e2)
                       && loplugin::TypeCheck(e2->getType())
                              .Struct("Dummy")
                              .Namespace("libreoffice_internal")
                              .Namespace("rtl")
                              .GlobalNamespace())
                      || (loplugin::TypeCheck(e2->getType()).Typedef("sal_Int32").GlobalNamespace()
                          && e2->isIntegerConstantExpr(compiler.getASTContext()))))
                {
                    return true;
                }
                auto const d = e1->getDecl();
                if (!reportedArray_.insert(d).second)
                {
                    return true;
                }
                report(DiagnosticsEngine::Warning,
                       "change type of variable %0 from constant character array (%1) to "
                       "%select{OStringLiteral|OUStringLiteral}2%select{|, and make it static}3",
                       d->getLocation())
                    << d << d->getType()
                    << (tc.Class("OString").Namespace("rtl").GlobalNamespace() ? 0 : 1)
                    << isAutomaticVariable(cast<VarDecl>(d)) << d->getSourceRange();
                report(DiagnosticsEngine::Note, "first passed into a %0 constructor here",
                       expr->getLocation())
                    << expr->getType().getUnqualifiedType() << expr->getSourceRange();
            }
            break;
        }
        return true;
    }

    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getKind() != UETT_SizeOf)
        {
            return true;
        }
        if (expr->isArgumentType())
        {
            return true;
        }
        auto const e = dyn_cast<DeclRefExpr>(expr->getArgumentExpr()->IgnoreParenImpCasts());
        if (e == nullptr)
        {
            return true;
        }
        auto const tc = loplugin::TypeCheck(e->getType());
        if (!(tc.Class("OStringLiteral").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        auto const d = e->getDecl();
        report(DiagnosticsEngine::Warning,
               "variable %0 of type %1 suspiciously used in a sizeof expression", e->getLocation())
            << d << d->getType() << expr->getSourceRange();
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    std::set<Decl const*> reportedAutomatic_;
    std::set<Decl const*> reportedArray_;
};

static loplugin::Plugin::Registration<StringLiteralVar> reg("stringliteralvar");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
