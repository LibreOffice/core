/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

/**
 * look for places we can use std::make_shared
 */

namespace
{
class MakeShared : public loplugin::FilteringPlugin<MakeShared>
{
public:
    explicit MakeShared(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        // uses boost::shared_ptr and we trigger because we're not looking specifically for std::shared_ptr
        if (loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/cmis/cmis_repo_content.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/cmis/cmis_content.cxx"))
            return false;
        // TODO something weird with protected base classes going on here
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xeextlst.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xecontent.cxx"))
            return false;
        // no idea what is going on here
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/sidebar/nbdtmg.cxx"))
            return false;

        // legitimate use of moving std::unique_ptr to std::shared_ptr
        if (loplugin::isSamePathname(fn, SRCDIR "/comphelper/source/container/enumerablemap.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/items/style.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/app/weldutils.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/appopen.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/table/tablertfimporter.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/docshell/externalrefmgr.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/attr/swatrset.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/condformat/condformatdlg.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/layout/frmtool.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xihelper.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xeformula.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xichart.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/html/htmlpars.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/view/cellsh1.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/html/htmltab.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/docxattributeoutput.cxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool shouldVisitTemplateInstantiations() const { return true; }

    bool VisitCXXConstructExpr(CXXConstructExpr const*);
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitVarDecl(VarDecl const*);
};

bool MakeShared::VisitCXXConstructExpr(CXXConstructExpr const* constructExpr)
{
    if (ignoreLocation(constructExpr))
        return true;
    if (!loplugin::TypeCheck(constructExpr->getType()).ClassOrStruct("shared_ptr").StdNamespace())
        return true;
    if (!(constructExpr->getNumArgs() == 1
          || (constructExpr->getNumArgs() > 1 && isa<CXXDefaultArgExpr>(constructExpr->getArg(1)))))
        return true;
    auto arg0 = constructExpr->getArg(0)->IgnoreParenImpCasts();
    auto cxxNewExpr = dyn_cast<CXXNewExpr>(arg0);
    if (cxxNewExpr)
    {
        auto construct2 = cxxNewExpr->getConstructExpr();
        if (construct2)
        {
            if (construct2->getConstructor()->getAccess() != AS_public)
                return true;
            if (construct2->getNumArgs() == 1
                && isa<CXXStdInitializerListExpr>(construct2->getArg(0)))
                return true;
        }
    }
    else if (loplugin::TypeCheck(arg0->getType()).ClassOrStruct("shared_ptr").StdNamespace())
        return true;
    else if (loplugin::TypeCheck(arg0->getType()).ClassOrStruct("weak_ptr").StdNamespace())
        return true;
    else if (arg0->getType()->isDependentType())
        return true;
    else if (isa<CXXNullPtrLiteralExpr>(arg0))
        return true;
    else if (auto const call = dyn_cast<CallExpr>(arg0))
    {
        if (auto const decl = call->getDirectCallee())
        {
            // Don't warn about cases where e.g. the Bitmap* result of calling Windows'
            // Bitmap::FromBITMAPINFO is wrapped in a shared_ptr:
            if (decl->getReturnType()->isPointerType()
                && compiler.getSourceManager().isInSystemHeader(decl->getLocation()))
            {
                return true;
            }
        }
    }

    StringRef fn = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(constructExpr)));
    if (loplugin::isSamePathname(fn, SRCDIR "/include/o3tl/make_shared.hxx"))
        return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/items/stylepool.cxx"))
        return true;

    report(DiagnosticsEngine::Warning, "rather use make_shared than constructing from %0",
           compat::getBeginLoc(constructExpr))
        << arg0->getType() << constructExpr->getSourceRange();
    return true;
}

bool MakeShared::VisitCXXMemberCallExpr(CXXMemberCallExpr const* cxxMemberCallExpr)
{
    if (ignoreLocation(cxxMemberCallExpr))
        return true;

    if (cxxMemberCallExpr->getNumArgs() != 1)
        return true;

    // cannot find a way to use the loplugin::DeclCheck stuff here
    auto templateDecl
        = dyn_cast<ClassTemplateSpecializationDecl>(cxxMemberCallExpr->getRecordDecl());
    if (!templateDecl)
        return true;
    auto cxxRecordDecl = templateDecl->getSpecializedTemplate()->getTemplatedDecl();
    if (!cxxRecordDecl->getName().contains("shared_ptr"))
        return true;

    if (auto const id = cxxMemberCallExpr->getMethodDecl()->getIdentifier())
    {
        if (id->getName() != "reset")
            return true;
    }
    auto cxxNewExpr = dyn_cast<CXXNewExpr>(cxxMemberCallExpr->getArg(0)->IgnoreParenImpCasts());
    if (!cxxNewExpr)
        return true;
    if (cxxNewExpr->getConstructExpr()
        && cxxNewExpr->getConstructExpr()->getConstructor()->getAccess() != AS_public)
        return true;

    StringRef fn = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(cxxMemberCallExpr)));
    if (loplugin::isSamePathname(fn, SRCDIR "/include/o3tl/make_shared.hxx"))
        return true;

    report(DiagnosticsEngine::Warning, "rather use make_shared", compat::getBeginLoc(cxxNewExpr))
        << cxxNewExpr->getSourceRange();

    return true;
}

bool MakeShared::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* operCallExpr)
{
    if (ignoreLocation(operCallExpr))
        return true;
    if (!operCallExpr->isAssignmentOp())
        return true;

    if (!loplugin::TypeCheck(operCallExpr->getType()).ClassOrStruct("shared_ptr").StdNamespace())
        return true;

    if (loplugin::TypeCheck(operCallExpr->getArg(1)->getType())
            .ClassOrStruct("shared_ptr")
            .StdNamespace())
        return true;

    report(DiagnosticsEngine::Warning, "rather use make_shared than constructing from %0",
           compat::getBeginLoc(operCallExpr))
        << operCallExpr->getArg(1)->getType() << operCallExpr->getSourceRange();

    return true;
}

bool MakeShared::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!varDecl->hasInit())
        return true;

    if (!loplugin::TypeCheck(varDecl->getType()).ClassOrStruct("shared_ptr").StdNamespace())
        return true;

    if (varDecl->getInit()->getType().isNull())
        return true;
    if (varDecl->getInit()->getType()->isDependentType())
        return true;
    if (loplugin::TypeCheck(varDecl->getInit()->IgnoreParenImpCasts()->getType())
            .ClassOrStruct("shared_ptr")
            .StdNamespace())
        return true;

    report(DiagnosticsEngine::Warning, "rather use make_shared than constructing from %0",
           compat::getBeginLoc(varDecl))
        << varDecl->getInit()->getType() << varDecl->getSourceRange();

    return true;
}

loplugin::Plugin::Registration<MakeShared> makeshared("makeshared");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
