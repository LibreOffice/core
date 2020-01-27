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
    auto cxxNewExpr = dyn_cast<CXXNewExpr>(constructExpr->getArg(0)->IgnoreParenImpCasts());
    if (!cxxNewExpr)
        return true;
    auto construct2 = cxxNewExpr->getConstructExpr();
    if (construct2)
    {
        if (construct2->getConstructor()->getAccess() != AS_public)
            return true;
        if (construct2->getNumArgs() == 1 && isa<CXXStdInitializerListExpr>(construct2->getArg(0)))
            return true;
    }

    StringRef fn = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(constructExpr)));
    if (loplugin::isSamePathname(fn, SRCDIR "/include/o3tl/make_shared.hxx"))
        return true;

    report(DiagnosticsEngine::Warning, "rather use make_shared", compat::getBeginLoc(cxxNewExpr))
        << cxxNewExpr->getSourceRange();
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

loplugin::Plugin::Registration<MakeShared> makeshared("makeshared");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
