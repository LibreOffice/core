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
#include <fstream>
#include <set>
#include <unordered_set>

#include <clang/AST/CXXInheritance.h>

#include "config_clang.h"

#include "plugin.hxx"
#include "check.hxx"

/**

*/

namespace
{
class PutPoolItem : public loplugin::FilteringPlugin<PutPoolItem>
{
public:
    explicit PutPoolItem(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        //        StringRef fn(handler.getMainFileName());
        //        if (loplugin::isSamePathname(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
        //            return false;
        return true;
    }
    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);
    bool VisitFunctionDecl(const FunctionDecl*)
    {
        //        if (f->getIdentifier() && f->getName() == "foo")
        //            f->dump();
        return true;
    }
};

bool PutPoolItem::VisitCXXMemberCallExpr(const CXXMemberCallExpr* cxxCallExpr)
{
    if (ignoreLocation(cxxCallExpr))
        return true;
    auto tc = loplugin::TypeCheck(cxxCallExpr->getObjectType());
    if (!tc.Class("SfxItemSet"))
        return true;
    if (!cxxCallExpr->getMethodDecl()->getIdentifier()
        || cxxCallExpr->getMethodDecl()->getName() != "Put")
        return true;
    auto argExpr = dyn_cast<CXXOperatorCallExpr>(cxxCallExpr->getArg(0)->IgnoreImplicit());
    if (!argExpr)
        return true;
    if (argExpr->getOperator() != OO_Star)
        return true;
    auto ptrExpr = argExpr->getArg(0)->IgnoreImplicit();
    auto tc2 = loplugin::TypeCheck(ptrExpr->getType());
    if (!tc2.Class("unique_ptr"))
        return true;
    // ignore calls when we are passing a copy of a member field
    if (isa<MemberExpr>(ptrExpr))
        return true;

    StringRef fn = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(cxxCallExpr->getBeginLoc()));
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/app/inputwin.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/dbgui/csvgrid.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/shells/basesh.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/shells/textsh.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/xml/xmlimpit.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/shells/tabsh.cxx"))
        return true;

    //    argExpr->dump();

    report(DiagnosticsEngine::Warning, "could use std::move?", cxxCallExpr->getBeginLoc())
        << cxxCallExpr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<PutPoolItem> putpoolitem("putpoolitem", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
