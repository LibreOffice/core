/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"
#include "compat.hxx"

// This is similar to inlinesimplememberfunctions but it tries to guess code size.
// It reports even non-trivial functions, so it needs to be enabled explicitly
// and results are meant to be explicitly checked.

namespace
{
constexpr bool ignorePrivate = true;

class InlineSmallMemberFunctions : public loplugin::FilteringPlugin<InlineSmallMemberFunctions>
{
public:
    explicit InlineSmallMemberFunctions(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(const CXXMethodDecl* decl);

private:
    static int countStatements(const Stmt* stmt);
};

bool InlineSmallMemberFunctions::VisitCXXMethodDecl(const CXXMethodDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
    {
        return true;
    }
    // no point in doing virtual methods, the compiler always has to generate a vtable entry and a method
    if (functionDecl->isVirtual())
    {
        return true;
    }
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate)
    {
        return true;
    }
    if (!functionDecl->isOutOfLine())
    {
        return true;
    }
    if (!functionDecl->hasBody())
    {
        return true;
    }
    if (functionDecl->isInlineSpecified())
    {
        return true;
    }
    if (functionDecl->getCanonicalDecl()->isInlineSpecified())
    {
        return true;
    }
    if (functionDecl->getNameAsString().find("Impl") != std::string::npos)
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl))
    {
        return true;
    }

    if (ignorePrivate && functionDecl->getAccess() == AS_private)
        return true;

    // If the function has 5 statements or less, maybe it's small enough, report it.
    if (countStatements(functionDecl->getBody()) <= 5)
    {
        report(DiagnosticsEngine::Warning, "function %0 may be small enough for inlining",
               functionDecl->getSourceRange().getBegin())
            << functionDecl << functionDecl->getSourceRange();
    }

    return true;
}

int InlineSmallMemberFunctions::countStatements(const Stmt* stmt)
{
    int count = 0;
    // Expressions in a compound statement are actually statements too.
    bool includeExprs = isa<CompoundStmt>(stmt);
    for (const Stmt* subStmt : stmt->children())
    {
        if (subStmt == nullptr)
            continue;
        if (!includeExprs && isa<Expr>(subStmt))
            continue;
        ++count;
        count += countStatements(subStmt);
    }
    return count;
}

loplugin::Plugin::Registration<InlineSmallMemberFunctions> X("inlinesmallmemberfunctions", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
