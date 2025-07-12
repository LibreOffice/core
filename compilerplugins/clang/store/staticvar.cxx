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
#include "config_clang.h"
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
            // contains mutable state
            || fn == SRCDIR "/vcl/unx/generic/dtrans/X11_selection.cxx"
            || fn == SRCDIR "/sax/qa/cppunit/xmlimport.cxx"
            || fn == SRCDIR "/pyuno/source/module/pyuno.cxx"
            || fn == SRCDIR "/pyuno/source/module/pyuno_module.cxx"
            || fn == SRCDIR "/pyuno/source/module/pyuno_struct.cxx"
            // TODO for this one we need a static OUString
            || fn == SRCDIR "/xmloff/source/core/xmltoken.cxx"
            // mutable
            || fn == SRCDIR "/basic/source/runtime/stdobj.cxx"
            // TODO this needs more extensive cleanup
            || fn == SRCDIR "/connectivity/source/drivers/postgresql/pq_statics.cxx"
            // mutable
            || fn == SRCDIR "/hwpfilter/source/hwpreader.cxx"
            // mutable
            || fn == SRCDIR "/sw/source/filter/basflt/fltini.cxx"
            // mutable
            || fn == SRCDIR "/sw/source/uibase/docvw/srcedtw.cxx"
            // mutable
            || fn == SRCDIR "/forms/source/misc/limitedformats.cxx"
            // TODO sorting some of these tables will be a lot of work...
            || fn == SRCDIR "/sw/source/filter/ww8/ww8par6.cxx"
            // this only triggers on older versions of clang, not sure why
            // in any case, it is actually about the array in vcl/inc/units.hrc, which we can't change
            || fn == SRCDIR "/vcl/source/app/svdata.cxx"
            // I tried doing this, but got very weird unit test failures, apparently sorting this table
            // disturbs some code elsewhere
            || fn == SRCDIR "/svx/source/unodraw/unoprov.cxx"
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(VarDecl const*);
};

static bool containsNonLiteral(Expr const* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto initList = dyn_cast<InitListExpr>(expr))
    {
        for (unsigned i = 0; i < initList->getNumInits(); ++i)
            if (containsNonLiteral(initList->getInit(i)))
                return true;
    }
    else if (auto constructExpr = dyn_cast<CXXConstructExpr>(expr))
    {
        for (Expr const* arg : constructExpr->arguments())
            if (containsNonLiteral(arg))
                return true;
    }
    else if (isa<MemberExpr>(expr))
        return true;
    else if (auto declRefExpr = dyn_cast<DeclRefExpr>(expr))
    {
        auto varDecl = dyn_cast_or_null<VarDecl>(declRefExpr->getDecl());
        return varDecl && varDecl->isLocalVarDeclOrParm();
    }
    else if (isa<CXXMemberCallExpr>(expr))
        return true;
    else if (auto castExpr = dyn_cast<CXXFunctionalCastExpr>(expr))
        return containsNonLiteral(castExpr->getSubExpr());
    else if (auto unaryOp = dyn_cast<UnaryOperator>(expr))
        return containsNonLiteral(unaryOp->getSubExpr());

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
    auto elementType = varDecl->getType()->getBaseElementTypeUnsafe();
    if (!elementType->isRecordType())
        return true;
    auto elementRecordDecl
        = dyn_cast_or_null<CXXRecordDecl>(elementType->getAs<RecordType>()->getDecl());
    if (!elementRecordDecl)
        return true;
    if (containsNonLiteral(initList))
        return true;

    if (elementRecordDecl->hasTrivialDestructor())
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

        auto const tcElement = loplugin::TypeCheck(elementRecordDecl);
        if (tcElement.Struct("ContextID_Index_Pair").GlobalNamespace())
            return true;
        if (tcElement.Class("SfxSlot").GlobalNamespace())
            return true;

        if (varDecl->isLocalVarDecl())
            report(DiagnosticsEngine::Warning, "var should be static const, or allowlisted",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
        else
            report(DiagnosticsEngine::Warning, "var should be const, or allowlisted",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
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
    }
    return true;
}

loplugin::Plugin::Registration<StaticVar> X("staticvar", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
