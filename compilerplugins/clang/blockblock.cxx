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
  Check for places where we declare a block directly inside a block
 */
namespace {

class BlockBlock:
    public loplugin::FilteringPlugin<BlockBlock>
{
public:
    explicit BlockBlock(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/sal/osl/unx/file_misc.cxx"))
             return false;
        return true;
    }

    void run() override {
        if (preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
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
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(compound)))
        return true;
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(inner)))
        return true;
    if (containsPreprocessingConditionalInclusion(compound->getSourceRange())) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "block directly inside block",
         compat::getBeginLoc(compound))
        << compound->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "inner block here",
         compat::getBeginLoc(inner))
        << inner->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< BlockBlock > blockblock("blockblock", true);

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
