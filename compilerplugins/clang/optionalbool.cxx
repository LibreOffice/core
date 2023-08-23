/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <stack>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"

// Check for uses of std::optional<bool> being assigned to bool, which generally does not do
// what you would expect.

namespace
{
class OptionalBool final : public loplugin::FilteringPlugin<OptionalBool>
{
public:
    explicit OptionalBool(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitIfStmt(const IfStmt* ifStmt)
    {
        if (ignoreLocation(ifStmt))
            return true;
        m_ignoreIfCondition = ifStmt->getCond()->IgnoreImplicit();
        return true;
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr* memberCall)
    {
        if (ignoreLocation(memberCall))
            return true;
        if (m_ignoreIfCondition == memberCall)
            return true;
        // check if we are calling a 'operator bool' conversion method
        auto conversionDecl = dyn_cast_or_null<CXXConversionDecl>(memberCall->getMethodDecl());
        if (!conversionDecl || !conversionDecl->getConversionType()->isBooleanType())
            return true;
        auto ctsd = dyn_cast_or_null<ClassTemplateSpecializationDecl>(
            memberCall->getObjectType()->getAsRecordDecl());
        if (!ctsd || ctsd->getTemplateArgs().size() < 1)
            return true;
        if (ctsd->getName() != "optional")
            return true;
        if (!ctsd->getTemplateArgs().get(0).getAsType()->isBooleanType())
            return true;
        report(DiagnosticsEngine::Warning,
               "using conversion call to convert std::optional<bool> to bool probably does not do "
               "what you expect, rather use has_value() or value_or()",
               memberCall->getBeginLoc())
            << memberCall->getSourceRange();
        return true;
    }

private:
    const Expr* m_ignoreIfCondition = nullptr;
};

loplugin::Plugin::Registration<OptionalBool> optionalbool("optionalbool");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
