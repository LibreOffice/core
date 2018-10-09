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

// Look for variables that are arrays of structs, which almost always want to be static const.

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
        if (fn == SRCDIR "/sal/osl/unx/signal.cxx"
            || fn == SRCDIR "/tools/source/reversemap/bestreversemap.cxx"
            || fn == SRCDIR "/soltools/cpp/_nlist.c" || fn == SRCDIR "/soltools/cpp/_macro.c"
            || fn == SRCDIR "/cppu/source/typelib/static_types.cxx"
            || fn == SRCDIR "/cppu/source/threadpool/current.cxx"
            || fn == SRCDIR "/basegfx/source/polygon/b2dpolygonclipper.cxx"
            || fn == SRCDIR "/svl/source/crypto/cryptosign.cxx"
            || fn == SRCDIR "/jvmfwk/plugins/sunmajor/pluginlib/sunjavaplugin.cxx"
            || fn == SRCDIR "/basic/source/runtime/stdobj.cxx"
            // TODO for this one we need a static OUString
            || fn == SRCDIR "/toolkit/source/helper/property.cxx"
            // TODO for this one we need a static OUString
            || fn == SRCDIR "/xmloff/source/core/xmltoken.cxx"
            || fn == SRCDIR "/editeng/source/editeng/editview.cxx"
            || fn == SRCDIR "/connectivity/source/drivers/postgresql/pq_statics.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(VarDecl const*);
};

bool StaticVar::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!varDecl->hasInit())
        return true;
    if (varDecl->isExceptionVariable() || isa<ParmVarDecl>(varDecl))
        return true;
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation()))
        return true;
    if (!varDecl->getType()->isArrayType())
        return true;
    if (!varDecl->getType()->getPointeeOrArrayElementType()->isStructureType())
        return true;

    // Ignore macros like FD_ZERO
    //    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(varDecl))) {
    //        return true;
    //    }
    //    if (varDecl->hasGlobalStorage()) {
    //        return true;
    //    }
    //    auto const tc = loplugin::TypeCheck(varDecl->getType());
    //    if (tc.Typedef("va_list").StdNamespace())
    //        return true;
    //    if (!varDecl->getType().isCXX11PODType(compiler.getASTContext())
    //        && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
    //        && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
    //        && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
    //        && !tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
    //        && !tc.Class("Color").GlobalNamespace()
    //        && !tc.Class("Pair").GlobalNamespace()
    //        && !tc.Class("Point").GlobalNamespace()
    //        && !tc.Class("Size").GlobalNamespace()
    //        && !tc.Class("Range").GlobalNamespace()
    //        && !tc.Class("Selection").GlobalNamespace()
    //        && !tc.Class("Rectangle").Namespace("tools").GlobalNamespace())
    //    {
    //        return true;
    //    }

    if (varDecl->getType().isConstQualified())
        return true;

    // TODO cannot figure out how to make the loplugin::TypeCheck stuff match this
    std::string typeName = varDecl->getType().getAsString();
    if (typeName == "std::va_list" || typeName == "va_list")
        return true;

    auto const tcElement
        = loplugin::TypeCheck(QualType{ varDecl->getType()->getPointeeOrArrayElementType(), 0 });
    if (tcElement.Struct("ContextID_Index_Pair").GlobalNamespace())
        return true;

    report(DiagnosticsEngine::Warning, "var should be static const, or whitelisted",
           varDecl->getLocation())
        << varDecl->getSourceRange();
    std::cout << varDecl->getType().getAsString() << std::endl;
    return true;
}

loplugin::Plugin::Registration<StaticVar> X("staticvar", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
