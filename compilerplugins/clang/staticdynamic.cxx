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

#include "compat.hxx"
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
    typedef std::map<std::pair<VarDecl const*, clang::Type const*>, SourceLocation> MapType;
    MapType staticCastVars;
    // only maintain state inside a single basic block, we're not trying to analyse
    // cross-block interactions.
    std::vector<MapType> blockStack;
};

bool StaticDynamic::PreTraverseCompoundStmt(CompoundStmt*)
{
    blockStack.push_back(std::move(staticCastVars));
    return true;
}

bool StaticDynamic::PostTraverseCompoundStmt(CompoundStmt*, bool)
{
    staticCastVars = std::move(blockStack.back());
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
    staticCastVars.insert({ { varDecl, staticCastExpr->getTypeAsWritten().getTypePtr() },
                            compat::getBeginLoc(staticCastExpr) });
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
    auto it = staticCastVars.find({ varDecl, dynamicCastExpr->getTypeAsWritten().getTypePtr() });
    if (it == staticCastVars.end())
        return true;
    report(DiagnosticsEngine::Warning, "dynamic_cast after static_cast",
           compat::getBeginLoc(dynamicCastExpr))
        << dynamicCastExpr->getSourceRange();
    report(DiagnosticsEngine::Note, "static_cast here", it->second);
    return true;
}

loplugin::Plugin::Registration<StaticDynamic> staticdynamic("staticdynamic");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
