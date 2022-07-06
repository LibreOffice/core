/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include "config_clang.h"
#include "plugin.hxx"
#include "check.hxx"

/*
*/

namespace
{
class OptValue : public loplugin::FilteringPlugin<OptValue>
{
public:
    explicit OptValue(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);
};

bool OptValue::VisitCXXMemberCallExpr(const CXXMemberCallExpr* topExpr)
{
    if (ignoreLocation(topExpr))
        return true;
    const CXXMethodDecl* methodDecl = topExpr->getMethodDecl();
    if (!methodDecl)
        return true;
    if (!methodDecl->getIdentifier() || methodDecl->getName() != "value")
        return true;
    auto expr1 = topExpr->getImplicitObjectArgument()->IgnoreImpCasts();
    if (!isa<MaterializeTemporaryExpr>(expr1))
        return true;

    report(DiagnosticsEngine::Warning, "call to OptValue::value()", topExpr->getBeginLoc());

    return true;
}

loplugin::Plugin::Registration<OptValue> optvalue("optvalue", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
