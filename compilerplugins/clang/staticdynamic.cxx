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
#include <map>
#include <vector>

#include "config_clang.h"

#include "check.hxx"
#include "plugin.hxx"

namespace
{
class StaticDynamic : public loplugin::FilteringPlugin<StaticDynamic>
{
public:
    explicit StaticDynamic(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }
    void postRun() override {}
    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const*);
    bool VisitCXXStaticCastExpr(CXXStaticCastExpr const*);
    bool PreTraverseCompoundStmt(CompoundStmt*);
    bool PostTraverseCompoundStmt(CompoundStmt*, bool);
    bool TraverseCompoundStmt(CompoundStmt*);

private:
    // the key is the pair of VarDecl and the type being cast to.
    struct BlockState
    {
        std::map<std::pair<VarDecl const*, clang::Type const*>, SourceLocation> staticCastVars;
        std::map<std::pair<VarDecl const*, clang::Type const*>, SourceLocation> dynamicCastVars;
    };
    // only maintain state inside a single basic block, we're not trying to analyse
    // cross-block interactions.
    std::vector<BlockState> blockStack;
    BlockState blockState;
};

bool StaticDynamic::PreTraverseCompoundStmt(CompoundStmt*)
{
    blockStack.push_back(std::move(blockState));
    return true;
}

bool StaticDynamic::PostTraverseCompoundStmt(CompoundStmt*, bool)
{
    blockState = std::move(blockStack.back());
    blockStack.pop_back();
    return true;
}

bool StaticDynamic::TraverseCompoundStmt(CompoundStmt* compoundStmt)
{
    bool ret = true;
    if (PreTraverseCompoundStmt(compoundStmt))
    {
        ret = FilteringPlugin::TraverseCompoundStmt(compoundStmt);
        PostTraverseCompoundStmt(compoundStmt, ret);
    }
    return ret;
}

const clang::Type* strip(QualType qt)
{
    const clang::Type* varType = qt->getUnqualifiedDesugaredType();
    if (varType->isPointerType())
        varType = varType->getPointeeType()->getUnqualifiedDesugaredType();
    if (varType->isReferenceType())
        varType = varType->getAs<clang::ReferenceType>()
                      ->getPointeeType()
                      ->getUnqualifiedDesugaredType();
    return varType;
}

bool StaticDynamic::VisitCXXStaticCastExpr(CXXStaticCastExpr const* staticCastExpr)
{
    if (ignoreLocation(staticCastExpr))
        return true;
    auto subExprDecl = dyn_cast<DeclRefExpr>(staticCastExpr->getSubExpr()->IgnoreParenImpCasts());
    if (!subExprDecl)
        return true;
    auto varDecl = dyn_cast_or_null<VarDecl>(subExprDecl->getDecl());
    if (!varDecl)
        return true;
    auto varType = strip(staticCastExpr->getType());
    auto it = blockState.dynamicCastVars.find({ varDecl, varType });
    if (it != blockState.dynamicCastVars.end())
    {
        StringRef fn = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(staticCastExpr->getBeginLoc()));
        // loop
        if (loplugin::isSamePathname(fn, SRCDIR "/basctl/source/basicide/basobj3.cxx"))
            return true;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/doc/swserv.cxx"))
            return true;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/text/txtfly.cxx"))
            return true;

        report(DiagnosticsEngine::Warning, "static_cast after dynamic_cast",
               staticCastExpr->getBeginLoc())
            << staticCastExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "dynamic_cast here", it->second);
        return true;
    }
    blockState.staticCastVars.insert({ { varDecl, varType }, staticCastExpr->getBeginLoc() });
    return true;
}

bool StaticDynamic::VisitCXXDynamicCastExpr(CXXDynamicCastExpr const* dynamicCastExpr)
{
    if (ignoreLocation(dynamicCastExpr))
        return true;

    auto subExprDecl = dyn_cast<DeclRefExpr>(dynamicCastExpr->getSubExpr()->IgnoreParenImpCasts());
    if (!subExprDecl)
        return true;
    auto varDecl = dyn_cast_or_null<VarDecl>(subExprDecl->getDecl());
    if (!varDecl)
        return true;
    auto varType = strip(dynamicCastExpr->getTypeAsWritten());
    auto it = blockState.staticCastVars.find({ varDecl, varType });
    if (it != blockState.staticCastVars.end())
    {
        report(DiagnosticsEngine::Warning, "dynamic_cast after static_cast",
               dynamicCastExpr->getBeginLoc())
            << dynamicCastExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "static_cast here", it->second);
        return true;
    }
    auto loc = dynamicCastExpr->getBeginLoc();
    if (compiler.getSourceManager().isMacroArgExpansion(loc)
        && (Lexer::getImmediateMacroNameForDiagnostics(loc, compiler.getSourceManager(),
                                                       compiler.getLangOpts())
            == "assert"))
    {
        return true;
    }
    blockState.dynamicCastVars.insert({ { varDecl, varType }, dynamicCastExpr->getBeginLoc() });
    return true;
}

loplugin::Plugin::Registration<StaticDynamic> staticdynamic("staticdynamic");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
