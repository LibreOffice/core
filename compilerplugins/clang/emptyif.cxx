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
#include "plugin.hxx"

/**
  Check for places where we do
   if (xxx) ;
  or
   if (xxx) {}
 */
namespace
{
class EmptyIf : public loplugin::FilteringRewritePlugin<EmptyIf>
{
public:
    explicit EmptyIf(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitIfStmt(IfStmt const*);

private:
    bool ContainsComment(Stmt const*);
};

static bool empty(Stmt const* stmt)
{
    if (isa<NullStmt>(stmt))
        return true;
    auto compoundStmt = dyn_cast<CompoundStmt>(stmt);
    if (!compoundStmt)
        return false;
    return compoundStmt->size() == 0;
}

bool EmptyIf::ContainsComment(Stmt const* stmt)
{
    auto range = stmt->getSourceRange();
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    char const* p1 = SM.getCharacterData(startLoc);
    char const* p2 = SM.getCharacterData(endLoc);
    p2 += Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    auto s = llvm::StringRef(p1, p2 - p1);
    return s.find("//") != llvm::StringRef::npos || s.find("/*") != llvm::StringRef::npos
           || s.find("#if") != llvm::StringRef::npos;
}

bool EmptyIf::VisitIfStmt(IfStmt const* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    if (ifStmt->getElse() && empty(ifStmt->getElse()) && !ContainsComment(ifStmt->getElse()))
    {
        report(DiagnosticsEngine::Warning, "empty else body",
               compat::getBeginLoc(ifStmt->getElse()))
            << ifStmt->getElse()->getSourceRange();
        return true;
    }

    if (!ifStmt->getElse() && empty(ifStmt->getThen()) && !ContainsComment(ifStmt->getThen()))
    {
        report(DiagnosticsEngine::Warning, "empty if body", compat::getBeginLoc(ifStmt))
            << ifStmt->getSourceRange();
    }

    return true;
}

loplugin::Plugin::Registration<EmptyIf> emptyif("emptyif", true);
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
