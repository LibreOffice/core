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

// Similar to GCC "warning: ‘void* memset(void*, int, size_t)’ writing to an object of non-trivial
// type ‘...’; use assignment instead [-Wclass-memaccess]", but looking through toplevel cast to
// void* and taking arrays into account in addition to pointers.  (Clang has
// -Wdynamic-class-memaccess, but that only warns about memset overwriting a vtable pointer.  GCC
// deliberately does not warn when there is a toplevel cast to void*, see
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81327> "[8 Regression] cast to void* does not
// suppress -Wclass-memaccess".)

namespace
{
class ClassMemAccess final : public loplugin::FilteringPlugin<ClassMemAccess>
{
public:
    explicit ClassMemAccess(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitCallExpr(CallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const fdecl = expr->getDirectCallee();
        if (fdecl == nullptr)
        {
            return true;
        }
        auto dc = loplugin::DeclCheck(fdecl).Function("memset");
        if (!(dc.GlobalNamespace() || dc.StdNamespace()))
        {
            return true;
        }
        if (expr->getNumArgs() != 3)
        {
            if (isDebugMode())
            {
                report(DiagnosticsEngine::Fatal,
                       "unexpected call to %0 with %1 instead of 3 arguments", expr->getExprLoc())
                    << fdecl << expr->getNumArgs() << expr->getSourceRange();
                report(DiagnosticsEngine::Note, "%0 declared here", fdecl->getLocation())
                    << fdecl << fdecl->getSourceRange();
            }
            return true;
        }
        auto e = expr->getArg(0)->IgnoreParenImpCasts();
        while (auto const cast = dyn_cast<ExplicitCastExpr>(e))
        {
            if (!loplugin::TypeCheck(cast->getTypeAsWritten()).Pointer().Void())
            {
                break;
            }
            e = cast->getSubExprAsWritten()->IgnoreParenImpCasts();
        }
        QualType t;
        if (auto const t1 = e->getType()->getAs<clang::PointerType>())
        {
            t = t1->getPointeeType();
        }
        else if (e->getType()->isArrayType())
        {
            t = e->getType();
            while (auto const t2 = t->getAsArrayTypeUnsafe())
            {
                t = t2->getElementType();
            }
        }
        else
        {
            if (isDebugMode())
            {
                report(DiagnosticsEngine::Fatal,
                       "unexpected call to %0 with first argument of non-pointer type %1",
                       expr->getExprLoc())
                    << fdecl << e->getType() << expr->getSourceRange();
                report(DiagnosticsEngine::Note, "%0 declared here", fdecl->getLocation())
                    << fdecl << fdecl->getSourceRange();
            }
            return true;
        }
        auto const decl = t->getAsCXXRecordDecl();
        if (decl == nullptr)
        {
            return true;
        }
        if (!decl->isCompleteDefinition())
        {
            return true; // conservatively assume it may be trivial
        }
        if (decl->isTrivial())
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               "%0 writing to an object of non-trivial type %1; use assignment instead",
               expr->getExprLoc())
            << fdecl << decl << expr->getSourceRange();
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
};

loplugin::Plugin::Registration<ClassMemAccess> classmemaccess("classmemaccess");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
