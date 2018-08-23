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

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"

/**
look for unnecessary blocks that just catch and rethrow:
    try {
       stuff
    } catch (exception const &) {
      throw;
    }
*/

namespace {

class UnnecessaryCatchThrow:
    public loplugin::FilteringPlugin<UnnecessaryCatchThrow>
{
public:
    explicit UnnecessaryCatchThrow(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXTryStmt(CXXTryStmt const *);
};

bool UnnecessaryCatchThrow::VisitCXXTryStmt(CXXTryStmt const * tryStmt)
{
    if (ignoreLocation(tryStmt))
        return true;
    if (tryStmt->getNumHandlers() != 1)
        return true;
    auto catchStmt = tryStmt->getHandler(0);
    auto compoundStmt = dyn_cast<CompoundStmt>(catchStmt->getHandlerBlock());
    if (!compoundStmt || compoundStmt->size() != 1)
        return true;
    auto throwExpr = dyn_cast<CXXThrowExpr>(compoundStmt->body_front());
    if (!throwExpr)
        return true;
    auto subExpr = throwExpr->getSubExpr();
    if (subExpr)
    {
        if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(subExpr)) {
            if (!cxxConstructExpr->getConstructor()->isCopyConstructor())
                return true;
            if (!cxxConstructExpr->getConstructor()->getParent()->hasAttr<FinalAttr>())
                return true;
            if (cxxConstructExpr->getNumArgs() != 1)
                return true;
            subExpr = cxxConstructExpr->getArg(0);
        }
        auto declRefExpr = dyn_cast<DeclRefExpr>(subExpr->IgnoreImpCasts());
        if (!declRefExpr)
            return true;
        if (declRefExpr->getDecl() != catchStmt->getExceptionDecl())
            return true;
    }

    report( DiagnosticsEngine::Warning, "unnecessary catch and throw",
            compat::getBeginLoc(catchStmt))
            << catchStmt->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< UnnecessaryCatchThrow > X("unnecessarycatchthrow");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
