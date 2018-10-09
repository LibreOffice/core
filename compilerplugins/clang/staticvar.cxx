/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// Look for variables that either
// (a) could be statically initialised, without runtime code, and warn
// (b) variables that are statically declared, but require runtime initialisation, and warn
//
// e.g.
//     static const OUString[] XXX { "xxx" };
// requires runtime initialisation, so should rather be declared as OUStringLiteral
// and
//     static int[] XXX { 1,2 };
// can be declared const since it does not require runtime initialisation.

namespace
{
class StaticVar : public loplugin::FilteringPlugin<StaticVar>
{
public:
    explicit StaticVar(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);

        if (
            // uses icu::UnicodeString
            fn == SRCDIR "/l10ntools/source/xmlparse.cxx"
            // contains mutable state
            || fn == SRCDIR "/sal/osl/unx/signal.cxx"
            || fn == SRCDIR "/sal/qa/rtl/digest/rtl_digest.cxx"
            || fn == SRCDIR "/sal/qa/rtl/strings/test_oustring_endswith.cxx"
            || fn == SRCDIR "/sal/qa/rtl/strings/test_oustring_convert.cxx"
            || fn == SRCDIR "/svl/qa/unit/items/test_itempool.cxx"
            // contains mutable state
            || fn == SRCDIR "/vcl/unx/generic/dtrans/X11_selection.cxx")
            return;
        //        if (fn == SRCDIR "/sal/osl/unx/signal.cxx"
        //            || fn == SRCDIR "/tools/source/reversemap/bestreversemap.cxx"
        //            || fn == SRCDIR "/soltools/cpp/_nlist.c"
        //            || fn == SRCDIR "/soltools/cpp/_macro.c"
        //            || fn == SRCDIR "/cppu/source/typelib/static_types.cxx"
        //            || fn == SRCDIR "/cppu/source/threadpool/current.cxx"
        //            || fn == SRCDIR "/basegfx/source/polygon/b2dpolygonclipper.cxx"
        //            || fn == SRCDIR "/svl/source/crypto/cryptosign.cxx"
        //            || fn == SRCDIR "/jvmfwk/plugins/sunmajor/pluginlib/sunjavaplugin.cxx"
        //            || fn == SRCDIR "/basic/source/runtime/stdobj.cxx"
        //            // TODO for this one we need a static OUString
        //            || fn == SRCDIR "/toolkit/source/helper/property.cxx"
        //            // TODO for this one we need a static OUString
        //            || fn == SRCDIR "/xmloff/source/core/xmltoken.cxx"
        //            || fn == SRCDIR "/editeng/source/editeng/editview.cxx"
        //            || fn == SRCDIR "/connectivity/source/drivers/postgresql/pq_statics.cxx"
        //            // TODO for this one we need a static OUString
        //            || fn == SRCDIR "/svx/source/form/formcontrolling.cxx"
        //            || fn == SRCDIR "/svx/source/form/fmPropBrw.cxx"
        //            )
        //             return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(VarDecl const*);
};

static bool containsNonLiteral(Expr const* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto initList = dyn_cast<InitListExpr>(expr))
    {
        for (Expr const* p : initList->inits())
            if (containsNonLiteral(p))
                return true;
    }
    else if (auto constructExpr = dyn_cast<CXXConstructExpr>(expr))
    {
        for (Expr const* arg : constructExpr->arguments())
            if (containsNonLiteral(arg))
                return true;
    }
    else if (isa<DeclRefExpr>(expr))
        return true;
    else if (isa<MemberExpr>(expr))
        return true;
    return false;
}

bool StaticVar::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!varDecl->hasInit())
        return true;
    auto initList = dyn_cast_or_null<InitListExpr>(varDecl->getInit());
    if (!initList)
        return true;
    if (varDecl->isExceptionVariable() || isa<ParmVarDecl>(varDecl))
        return true;
    if (!varDecl->getType()->isArrayType())
        return true;
    auto elementType = dyn_cast_or_null<CXXRecordDecl>(
        varDecl->getType()->getBaseElementTypeUnsafe()->getAsRecordDecl());
    if (!elementType)
        return true;
    if (containsNonLiteral(initList))
        return true;

    if (elementType->isTriviallyCopyable())
    {
        if (varDecl->isLocalVarDecl())
        {
            if (varDecl->getStorageDuration() == SD_Static && varDecl->getType().isConstQualified())
                return true;
        }
        else
        {
            if (varDecl->getType().isConstQualified())
                return true;
        }

        // TODO cannot figure out how to make the loplugin::TypeCheck stuff match this
        //    std::string typeName = varDecl->getType().getAsString();
        //    if (typeName == "std::va_list" || typeName == "va_list")
        //        return true;

        auto const tcElement = loplugin::TypeCheck(elementType);
        if (tcElement.Struct("ContextID_Index_Pair").GlobalNamespace())
            return true;

        if (varDecl->isLocalVarDecl())
            report(DiagnosticsEngine::Warning, "var should be static const, or whitelisted",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
        else
            report(DiagnosticsEngine::Warning, "var should be const, or whitelisted",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
        varDecl->dump();
    }
    else
    {
        if (varDecl->isLocalVarDecl())
        {
            if (varDecl->getStorageDuration() != SD_Static
                || !varDecl->getType().isConstQualified())
                return true;
        }
        else
        {
            if (!varDecl->getType().isConstQualified())
                return true;
        }

        if (varDecl->isLocalVarDecl())
            report(DiagnosticsEngine::Warning, "static const var requires runtime initialization?",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
        else
            report(DiagnosticsEngine::Warning, "static var requires runtime initialization?",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
        varDecl->dump();
    }
    return true;
}

loplugin::Plugin::Registration<StaticVar> X("staticvar", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
