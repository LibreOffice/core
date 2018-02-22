/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
class RedundantFCast final : public RecursiveASTVisitor<RedundantFCast>, public loplugin::Plugin
{
public:
    explicit RedundantFCast(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t1 = expr->getTypeAsWritten();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();

        if (t1.getCanonicalType().getTypePtr() != t2.getCanonicalType().getTypePtr())
            return true;

        // Look for types marked with SAL_WARN_UNUSED on the assumption that those
        // types are where we want to eliminate unnecessary copies.
        bool bInterestingType = false;
        if (auto const recType = t1->getAs<RecordType>())
            bInterestingType = recType->getDecl()->hasAttr<WarnUnusedAttr>();
        auto tc = loplugin::TypeCheck(t1);
        bInterestingType = bInterestingType || tc.Class("unique_ptr").StdNamespace()
                           || tc.Class("shared_ptr").StdNamespace()
                           || loplugin::isExtraWarnUnusedType(t1);
        if (!bInterestingType)
            return true;

        report(DiagnosticsEngine::Warning, "redundant functional cast from %0 to %1",
               expr->getExprLoc())
            << t2 << t1 << expr->getSourceRange();
        return true;
    }

private:
    void run() override
    {
        if (compiler.getLangOpts().CPlusPlus)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<RedundantFCast> reg("redundantfcast");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
