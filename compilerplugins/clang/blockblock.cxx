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
#include "plugin.hxx"

/**
  Check for places where we declare a block directly inside a block
 */
namespace {

class BlockBlock:
    public RecursiveASTVisitor<BlockBlock>, public loplugin::RewritePlugin
{
public:
    explicit BlockBlock(loplugin::InstantiationData const & data):
        RewritePlugin(data) {}

    virtual void run() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/sal/osl/unx/file_misc.cxx"))
             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const * );
};

bool BlockBlock::VisitCompoundStmt(CompoundStmt const * compound)
{
    if (ignoreLocation(compound))
        return true;
    if (compound->size() != 1)
        return true;
    auto inner = *compound->body_begin();
    if (!isa<CompoundStmt>(inner))
        return true;
    if (compiler.getSourceManager().isMacroBodyExpansion(compound->getLocStart()))
        return true;
    if (compiler.getSourceManager().isMacroBodyExpansion(inner->getLocStart()))
        return true;
    if (containsPreprocessingConditionalInclusion(compound->getSourceRange())) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "block directly inside block",
         compound->getLocStart())
        << compound->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "inner block here",
         inner->getLocStart())
        << inner->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< BlockBlock > X("blockblock", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
