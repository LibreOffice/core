/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"

#include <string>
#include <set>

/**
 * This plugin is unfinished, abandoned because it did not find anything interesting.
 *
 * Look for variables that are
 * (a) copied from
 * (b) never used after the copy
 * (c) have move operators
 *
 * The intention being to find places where we can move data (e.g. in containers) instead of copying.
*/

namespace
{
class OptMove : public loplugin::FilteringPlugin<OptMove>
{
public:
    explicit OptMove(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (auto const& pair : m_Candidates)
        {
            //auto varDecl = pair.first;
            auto candidate = pair.second;
            if (!candidate.canUseExpr)
                continue;
            report(DiagnosticsEngine::Warning, "can std::move value instead of copy",
                   candidate.canUseExpr->getSourceRange().getBegin())
                << candidate.canUseExpr->getSourceRange();
            //varDecl->dump();
        }
    }

    bool VisitVarDecl(const VarDecl*);
    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr*);
    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool VisitFunctionDecl(const FunctionDecl* f)
    {
        if (f->getIdentifier() && f->getName() == "foo")
            f->dump();
        return true;
    }

private:
    struct Candidate
    {
        const DeclRefExpr* operatorArg1 = nullptr;
        const Expr* canUseExpr = nullptr;
    };
    std::map<const VarDecl*, Candidate> m_Candidates;
};

bool OptMove::VisitVarDecl(const VarDecl* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (varDecl->hasGlobalStorage())
        return true;
    if (varDecl->getLinkageAndVisibility().getLinkage() == ExternalLinkage)
        return true;
    if (!varDecl->getType()->isRecordType())
        return true;

    auto cxxRecord = dyn_cast<CXXRecordDecl>(varDecl->getType()->getAsRecordDecl());
    if (!cxxRecord || !cxxRecord->hasDefinition() || !cxxRecord->hasMoveAssignment())
        return true;
    // ignore our simpler types for now, I'm after bigger game
    auto typeName = cxxRecord->getName();
    if (typeName.contains("Reference") || typeName.contains("Color") || typeName.contains("VclPtr")
        || typeName.contains("OString") || typeName.contains("OUString")
        || typeName.contains("Rectangle") || typeName.contains("Size")
        || typeName.contains("Selection") || typeName.contains("Point")
        || typeName.contains("strong_int"))
        return true;
    m_Candidates.emplace(varDecl, Candidate());

    if (!varDecl->hasInit())
        return true;
    auto cons = dyn_cast<CXXConstructExpr>(varDecl->getInit());
    if (!cons || !cons->getConstructor()->isCopyConstructor())
        return true;
    auto arg1 = dyn_cast<DeclRefExpr>(compat::IgnoreImplicit(cons->getArg(0)));
    if (!arg1)
        return true;
    auto varDecl1 = dyn_cast<VarDecl>(arg1->getDecl());
    if (!varDecl1)
        return true;
    auto it = m_Candidates.find(varDecl1);
    if (it == m_Candidates.end())
        return true;
    it->second.operatorArg1 = arg1;
    it->second.canUseExpr = cons;
    return true;
}

bool OptMove::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOperatorCallExpr)
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;
    auto op = cxxOperatorCallExpr->getOperator();
    if (op != OO_Equal)
        return true;
    auto arg0 = dyn_cast<DeclRefExpr>(compat::IgnoreImplicit(cxxOperatorCallExpr->getArg(0)));
    auto arg1 = dyn_cast<DeclRefExpr>(compat::IgnoreImplicit(cxxOperatorCallExpr->getArg(1)));
    if (!arg0 || !arg1)
        return true;
    auto varDecl0 = dyn_cast<VarDecl>(arg0->getDecl());
    auto varDecl1 = dyn_cast<VarDecl>(arg1->getDecl());
    if (!varDecl0 || !varDecl1)
        return true;
    auto cxxMethodDecl = dyn_cast_or_null<CXXMethodDecl>(cxxOperatorCallExpr->getDirectCallee());
    if (!cxxMethodDecl || !cxxMethodDecl->isCopyAssignmentOperator())
        return true;
    auto it = m_Candidates.find(varDecl1);
    if (it == m_Candidates.end())
        return true;
    it->second.operatorArg1 = arg1;
    it->second.canUseExpr = cxxOperatorCallExpr;
    return true;
}

bool OptMove::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    if (ignoreLocation(declRefExpr))
        return true;
    auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;
    auto it = m_Candidates.find(varDecl);
    if (it == m_Candidates.end())
        return true;
    if (it->second.operatorArg1 == declRefExpr)
        return true;
    m_Candidates.erase(it);
    return true;
}

loplugin::Plugin::Registration<OptMove> noexceptmove("optmove");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
