/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"
#include "check.hxx"

namespace
{
class SimplifyConstruct : public loplugin::FilteringPlugin<SimplifyConstruct>
{
public:
    explicit SimplifyConstruct(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXConstructExpr(CXXConstructExpr const*);

    // ignore some contexts within which nullptr is fine
    bool TraverseReturnStmt(ReturnStmt*) { return true; }
    bool TraverseInitListExpr(InitListExpr*) { return true; }
    bool TraverseCXXBindTemporaryExpr(CXXBindTemporaryExpr*) { return true; }
};

bool SimplifyConstruct::VisitCXXConstructExpr(CXXConstructExpr const* constructExpr)
{
    if (ignoreLocation(constructExpr))
        return true;
    auto tc = loplugin::TypeCheck(constructExpr->getType());
    if (!tc.Class("unique_ptr").StdNamespace() && !tc.Class("shared_ptr").StdNamespace()
        && !tc.Class("SvRef").Namespace("tools").GlobalNamespace()
        && !tc.Class("Reference").Namespace("rtl").GlobalNamespace()
        && !tc.Class("Reference")
                .Namespace("uno")
                .Namespace("star")
                .Namespace("sun")
                .Namespace("com")
                .GlobalNamespace())
        return true;
    if (constructExpr->getNumArgs() == 1
        && isa<CXXNullPtrLiteralExpr>(constructExpr->getArg(0)->IgnoreParenImpCasts()))
    {
        report(DiagnosticsEngine::Warning,
               "no need to explicitly init an instance of %0 with nullptr, just use default "
               "constructor",
               constructExpr->getSourceRange().getBegin())
            << constructExpr->getType() << constructExpr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<SimplifyConstruct> X("simplifyconstruct", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
