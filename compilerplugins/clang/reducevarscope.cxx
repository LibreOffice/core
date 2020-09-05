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
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// Original idea from mike kaganski.
// Look for variables that can have their scoped reduced, which makes the code easier to read.

// TODO when dealing with vars that are referenced in multiple child blocks, the check is very primitive
// and could be greatly improved.

namespace
{
class ReduceVarScope : public loplugin::FilteringPlugin<ReduceVarScope>
{
public:
    explicit ReduceVarScope(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        // ignore some files with problematic macros
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // some declarations look better all together
        if (fn == SRCDIR "/package/source/manifest/ManifestExport.cxx")
            return false;
        // storing pointer to OUString internal data
        if (fn == SRCDIR "/connectivity/source/drivers/odbc/ODatabaseMetaDataResultSet.cxx"
            || fn == SRCDIR "/sc/source/filter/excel/xestyle.cxx"
            || fn == SRCDIR "/sw/source/filter/html/htmlflywriter.cxx"
            || fn == SRCDIR "/unoxml/source/dom/element.cxx"
            || fn == SRCDIR "/unoxml/source/dom/document.cxx"
            || fn == SRCDIR "/sd/source/filter/eppt/pptx-animations.cxx")
            return false;
        if (fn == SRCDIR "/sal/qa/rtl/strings/nonconstarray.cxx")
            return false;
        return true;
    }

    virtual void run() override
    {
        if (!preRun())
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        postRun();
    }

    virtual void postRun() override
    {
        for (auto const& pair : maVarDeclMap)
        {
            auto varDecl = pair.first;
            auto const& depthInfo = pair.second;
            if (depthInfo.maDeclBlockPath.size() == depthInfo.maCommonBlockPath.size())
                continue;
            if (maVarDeclToIgnoreSet.find(varDecl) != maVarDeclToIgnoreSet.end())
                continue;
            auto it = maVarUseSourceRangeMap.find(varDecl);
            if (it == maVarUseSourceRangeMap.end())
                continue;
            report(DiagnosticsEngine::Warning, "can reduce scope of var", varDecl->getLocation())
                << varDecl->getSourceRange();
            for (SourceRange const& useRange : it->second)
                report(DiagnosticsEngine::Note, "used here", useRange.getBegin()) << useRange;
        }
    }

    bool VisitUnaryOperator(UnaryOperator const* expr)
    {
        // if we take the address of it
        UnaryOperator::Opcode op = expr->getOpcode();
        if (op == UO_AddrOf)
            recordIgnore(expr->getSubExpr());
        return true;
    }

    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool VisitVarDecl(const VarDecl*);
    bool VisitLambdaExpr(const LambdaExpr*);

    bool PreTraverseFunctionDecl(FunctionDecl*);
    bool PostTraverseFunctionDecl(FunctionDecl*, bool);
    bool TraverseFunctionDecl(FunctionDecl*);

    bool PreTraverseCompoundStmt(CompoundStmt*);
    bool PostTraverseCompoundStmt(CompoundStmt*, bool);
    bool TraverseCompoundStmt(CompoundStmt*);

    bool PreTraverseWhileStmt(WhileStmt*);
    bool PostTraverseWhileStmt(WhileStmt*, bool);
    bool TraverseWhileStmt(WhileStmt*);

    bool PreTraverseDoStmt(DoStmt*);
    bool PostTraverseDoStmt(DoStmt*, bool);
    bool TraverseDoStmt(DoStmt*);

    bool PreTraverseCXXForRangeStmt(CXXForRangeStmt*);
    bool PostTraverseCXXForRangeStmt(CXXForRangeStmt*, bool);
    bool TraverseCXXForRangeStmt(CXXForRangeStmt*);

    bool PreTraverseForStmt(ForStmt*);
    bool PostTraverseForStmt(ForStmt*, bool);
    bool TraverseForStmt(ForStmt*);

    bool PreTraverseSwitchStmt(SwitchStmt*);
    bool PostTraverseSwitchStmt(SwitchStmt*, bool);
    bool TraverseSwitchStmt(SwitchStmt*);

private:
    struct DepthInfo
    {
        unsigned int mnFirstDepth = 0;
        unsigned int mnFirstLoopDepth = 0;
        std::vector<unsigned int> maDeclBlockPath = {};
        std::vector<unsigned int> maCommonBlockPath = {};
    };
    std::unordered_map<VarDecl const*, DepthInfo> maVarDeclMap; // varDecl->depth
    std::unordered_set<VarDecl const*> maVarDeclToIgnoreSet;
    std::map<VarDecl const*, std::vector<SourceRange>> maVarUseSourceRangeMap;
    std::vector<unsigned int> maCurrentBlockPath;
    unsigned int mnCurrentDepth = 0;
    unsigned int mnCurrentLoopDepth = 0;
    static unsigned int gnBlockId;

    bool isTypeOK(QualType qt);
    bool isInitConstant(const VarDecl* varDecl);

    void recordIgnore(Expr const* expr)
    {
        for (;;)
        {
            expr = expr->IgnoreParenImpCasts();
            if (auto const e = dyn_cast<MemberExpr>(expr))
            {
                if (isa<FieldDecl>(e->getMemberDecl()))
                {
                    expr = e->getBase();
                    continue;
                }
            }
            if (auto const e = dyn_cast<ArraySubscriptExpr>(expr))
            {
                expr = e->getBase();
                continue;
            }
            if (auto const e = dyn_cast<BinaryOperator>(expr))
            {
                if (e->getOpcode() == BO_PtrMemD)
                {
                    expr = e->getLHS();
                    continue;
                }
            }
            break;
        }
        auto const dre = dyn_cast<DeclRefExpr>(expr);
        if (dre == nullptr)
            return;
        auto const var = dyn_cast<VarDecl>(dre->getDecl());
        if (var == nullptr)
            return;
        maVarDeclToIgnoreSet.insert(var);
    }
};

unsigned int ReduceVarScope::gnBlockId = 0;

bool ReduceVarScope::PreTraverseFunctionDecl(FunctionDecl* functionDecl)
{
    // Ignore functions that contains #ifdef-ery, can be quite tricky
    // to make useful changes when this plugin fires in such functions
    if (containsPreprocessingConditionalInclusion(functionDecl->getSourceRange()))
        return false;
    return true;
}

bool ReduceVarScope::PostTraverseFunctionDecl(FunctionDecl*, bool) { return true; }

bool ReduceVarScope::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    bool ret = true;
    if (PreTraverseFunctionDecl(functionDecl))
    {
        ret = FilteringPlugin::TraverseFunctionDecl(functionDecl);
        PostTraverseFunctionDecl(functionDecl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseCompoundStmt(CompoundStmt*)
{
    assert(mnCurrentDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentDepth;
    ++gnBlockId;
    maCurrentBlockPath.push_back(gnBlockId);
    return true;
}

bool ReduceVarScope::PostTraverseCompoundStmt(CompoundStmt*, bool)
{
    assert(mnCurrentDepth != 0);
    --mnCurrentDepth;
    maCurrentBlockPath.pop_back();
    return true;
}

bool ReduceVarScope::TraverseCompoundStmt(CompoundStmt* decl)
{
    bool ret = true;
    if (PreTraverseCompoundStmt(decl))
    {
        ret = FilteringPlugin::TraverseCompoundStmt(decl);
        PostTraverseCompoundStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseWhileStmt(WhileStmt*)
{
    assert(mnCurrentLoopDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::PostTraverseWhileStmt(WhileStmt*, bool)
{
    assert(mnCurrentLoopDepth != 0);
    --mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::TraverseWhileStmt(WhileStmt* decl)
{
    bool ret = true;
    if (PreTraverseWhileStmt(decl))
    {
        ret = FilteringPlugin::TraverseWhileStmt(decl);
        PostTraverseWhileStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseDoStmt(DoStmt*)
{
    assert(mnCurrentLoopDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::PostTraverseDoStmt(DoStmt*, bool)
{
    assert(mnCurrentLoopDepth != 0);
    --mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::TraverseDoStmt(DoStmt* decl)
{
    bool ret = true;
    if (PreTraverseDoStmt(decl))
    {
        ret = FilteringPlugin::TraverseDoStmt(decl);
        PostTraverseDoStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseSwitchStmt(SwitchStmt*)
{
    assert(mnCurrentLoopDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::PostTraverseSwitchStmt(SwitchStmt*, bool)
{
    assert(mnCurrentLoopDepth != 0);
    --mnCurrentLoopDepth;
    return true;
}

// Consider a switch to be a loop, because weird things happen inside it
bool ReduceVarScope::TraverseSwitchStmt(SwitchStmt* decl)
{
    bool ret = true;
    if (PreTraverseSwitchStmt(decl))
    {
        ret = FilteringPlugin::TraverseSwitchStmt(decl);
        PostTraverseSwitchStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseCXXForRangeStmt(CXXForRangeStmt*)
{
    assert(mnCurrentLoopDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::PostTraverseCXXForRangeStmt(CXXForRangeStmt*, bool)
{
    assert(mnCurrentLoopDepth != 0);
    --mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::TraverseCXXForRangeStmt(CXXForRangeStmt* decl)
{
    bool ret = true;
    if (PreTraverseCXXForRangeStmt(decl))
    {
        ret = FilteringPlugin::TraverseCXXForRangeStmt(decl);
        PostTraverseCXXForRangeStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::PreTraverseForStmt(ForStmt* forStmt)
{
    assert(mnCurrentLoopDepth != std::numeric_limits<unsigned int>::max());
    ++mnCurrentLoopDepth;

    auto declStmt = dyn_cast_or_null<DeclStmt>(forStmt->getInit());
    if (declStmt)
    {
        if (declStmt->isSingleDecl())
        {
            if (auto varDecl = dyn_cast_or_null<VarDecl>(declStmt->getSingleDecl()))
                maVarDeclToIgnoreSet.insert(varDecl);
        }
        else
        {
            for (auto const& decl : declStmt->getDeclGroup())
                if (auto varDecl = dyn_cast_or_null<VarDecl>(decl))
                    maVarDeclToIgnoreSet.insert(varDecl);
        }
    }

    return true;
}

bool ReduceVarScope::PostTraverseForStmt(ForStmt*, bool)
{
    assert(mnCurrentLoopDepth != 0);
    --mnCurrentLoopDepth;
    return true;
}

bool ReduceVarScope::TraverseForStmt(ForStmt* decl)
{
    bool ret = true;
    if (PreTraverseForStmt(decl))
    {
        ret = FilteringPlugin::TraverseForStmt(decl);
        PostTraverseForStmt(decl, ret);
    }
    return ret;
}

bool ReduceVarScope::VisitVarDecl(const VarDecl* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (varDecl->isExceptionVariable() || isa<ParmVarDecl>(varDecl))
        return true;
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation()))
        return true;
    // Ignore macros like FD_ZERO
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(varDecl)))
        return true;
    if (varDecl->hasGlobalStorage())
        return true;
    if (varDecl->isConstexpr())
        return true;
    if (varDecl->isInitCapture())
        return true;
    if (varDecl->isCXXForRangeDecl())
        return true;
    if (!isTypeOK(varDecl->getType()))
        return true;

    if (varDecl->hasInit() && !isInitConstant(varDecl))
        return true;

    maVarDeclMap[varDecl].mnFirstDepth = mnCurrentDepth;
    maVarDeclMap[varDecl].mnFirstLoopDepth = mnCurrentLoopDepth;
    maVarDeclMap[varDecl].maDeclBlockPath = maCurrentBlockPath;

    return true;
}

bool ReduceVarScope::isInitConstant(const VarDecl* varDecl)
{
    // check for string or scalar literals
    const Expr* initExpr = varDecl->getInit();
    if (auto e = dyn_cast<ExprWithCleanups>(initExpr))
        initExpr = e->getSubExpr();
    if (isa<clang::StringLiteral>(initExpr))
        return true;
    if (auto constructExpr = dyn_cast<CXXConstructExpr>(initExpr))
    {
        if (constructExpr->getNumArgs() == 0)
        {
            return true; // i.e., empty string
        }
        else
        {
            auto stringLit2 = dyn_cast<clang::StringLiteral>(constructExpr->getArg(0));
            if (stringLit2)
                return true;
        }
    }

    auto const init = varDecl->getInit();
    if (init->isValueDependent())
        return false;
    return init->isConstantInitializer(compiler.getASTContext(), false /*ForRef*/);
}

bool ReduceVarScope::isTypeOK(QualType varType)
{
    // TODO improve this - requires more analysis because it's really easy to
    // take a pointer to an array
    if (varType->isArrayType())
        return false;

    if (varType.isCXX11PODType(compiler.getASTContext()))
        return true;
    if (!varType->isRecordType())
        return false;
    auto recordDecl = dyn_cast_or_null<CXXRecordDecl>(varType->getAs<RecordType>()->getDecl());
    if (recordDecl && recordDecl->hasTrivialDestructor())
        return true;
    auto const tc = loplugin::TypeCheck(varType);
    // Safe types with destructors that don't do anything interesting
    if (tc.Class("OString").Namespace("rtl").GlobalNamespace()
        || tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        || tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
        || tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        || tc.Class("Color").GlobalNamespace() || tc.Class("Pair").GlobalNamespace()
        || tc.Class("Point").GlobalNamespace() || tc.Class("Size").GlobalNamespace()
        || tc.Class("Range").GlobalNamespace() || tc.Class("Selection").GlobalNamespace()
        || tc.Class("Rectangle").Namespace("tools").GlobalNamespace())
        return true;
    return false;
}

bool ReduceVarScope::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    if (ignoreLocation(declRefExpr))
        return true;
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<VarDecl>(decl) || isa<ParmVarDecl>(decl))
        return true;
    const VarDecl* varDecl = dyn_cast<VarDecl>(decl)->getCanonicalDecl();
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation()))
        return true;

    auto varIt = maVarDeclMap.find(varDecl);
    if (varIt == maVarDeclMap.end())
        return true;

    auto& depthInfo = varIt->second;

    // merge block paths to get common ancestor path
    if (depthInfo.maCommonBlockPath.empty())
        depthInfo.maCommonBlockPath = maCurrentBlockPath;
    else
    {
        auto len = std::min(depthInfo.maCommonBlockPath.size(), maCurrentBlockPath.size());
        unsigned int i = 0;
        while (i < len && depthInfo.maCommonBlockPath[i] == maCurrentBlockPath[i])
            ++i;
        depthInfo.maCommonBlockPath.resize(i);
        if (depthInfo.maCommonBlockPath == depthInfo.maDeclBlockPath)
        {
            maVarDeclMap.erase(varIt);
            maVarUseSourceRangeMap.erase(varDecl);
            return true;
        }
    }

    // seen in a loop below initial decl
    if (mnCurrentLoopDepth > depthInfo.mnFirstLoopDepth)
    {
        // TODO, we could additionally check if we are reading or writing to the var inside a loop
        // We only need to exclude vars that are written to, or passed taken-addr-of, or have non-const method called,
        // or passed as arg to non-const-ref parameter.
        maVarDeclMap.erase(varIt);
        maVarUseSourceRangeMap.erase(varDecl);
        return true;
    }

    auto it = maVarUseSourceRangeMap.find(varDecl);
    if (it == maVarUseSourceRangeMap.end())
        it = maVarUseSourceRangeMap.emplace(varDecl, std::vector<SourceRange>()).first;
    it->second.push_back(declRefExpr->getSourceRange());

    return true;
}

bool ReduceVarScope::VisitLambdaExpr(const LambdaExpr* lambdaExpr)
{
    if (ignoreLocation(lambdaExpr))
        return true;
    for (auto captureIt = lambdaExpr->capture_begin(); captureIt != lambdaExpr->capture_end();
         ++captureIt)
    {
        const LambdaCapture& capture = *captureIt;
        if (capture.capturesVariable())
        {
            auto varDecl = capture.getCapturedVar();
            maVarDeclMap.erase(varDecl);
            maVarUseSourceRangeMap.erase(varDecl);
        }
    }
    return true;
}

loplugin::Plugin::Registration<ReduceVarScope> reducevarscope("reducevarscope", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
