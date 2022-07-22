/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include "config_clang.h"
#include "plugin.hxx"
#include "check.hxx"
#include <unordered_set>
#include <unordered_map>

/*
Look for local variables that can be std::move'd into parameters.

TODO
(*) Ideally we would use a proper data-flow analysis, to detect that the var is dead after this point,
     like the one in clang at include/clang/Analysis/CFG.h
(*) we could expand the set of approved/interesting types
*/

namespace
{
class MoveIt : public loplugin::FilteringPlugin<MoveIt>
{
public:
    explicit MoveIt(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        //        // false +
        //        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basctl/source/basicide/moduldlg.cxx"))
        //            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        for (auto const& pair : m_possibles)
        {
            auto const& possible = pair.second;
            report(DiagnosticsEngine::Warning, "can std::move this var into this param",
                   possible.argExpr->getBeginLoc());
            report(DiagnosticsEngine::Note, "passing to this param",
                   possible.calleeParmVarDecl->getBeginLoc());
            report(DiagnosticsEngine::Note, "local var declared here",
                   possible.localVarDecl->getBeginLoc());
            report(DiagnosticsEngine::Note, "type declared here",
                   possible.recordDecl->getBeginLoc());
        }
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);
    bool VisitCXXConstructExpr(const CXXConstructExpr*);
    bool VisitDeclRefExpr(const DeclRefExpr*);

private:
    bool isInterestingType(QualType);
    struct Possible
    {
        const Expr* argExpr;
        const ParmVarDecl* calleeParmVarDecl;
        const VarDecl* localVarDecl;
        const CXXRecordDecl* recordDecl;
        const DeclRefExpr* dre;
    };
    std::unordered_map<const VarDecl*, Possible> m_possibles;
    std::unordered_set<const VarDecl*> m_rejected;
};

bool MoveIt::VisitCXXMemberCallExpr(const CXXMemberCallExpr* topExpr)
{
    if (ignoreLocation(topExpr))
        return true;
    const CXXMethodDecl* methodDecl = topExpr->getMethodDecl();
    if (!methodDecl)
        return true;

    unsigned len = std::min(topExpr->getNumArgs(), methodDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        // check if the parameter is a moveable type
        const ParmVarDecl* parmVarDecl = methodDecl->getParamDecl(i);
        if (!parmVarDecl->getType()->isRecordType())
            continue;
        const CXXRecordDecl* recordDecl
            = dyn_cast<CXXRecordDecl>(parmVarDecl->getType()->getAsRecordDecl());
        if (!recordDecl || !recordDecl->hasMoveConstructor() || recordDecl->isTriviallyCopyable())
            continue;
        if (!isInterestingType(parmVarDecl->getType()))
            continue;

        // check if (a) we're making a copy to pass to the param and (b) we're making a copy of a local var
        const Expr* argExpr = topExpr->getArg(i);
        if (!argExpr)
            continue;
        const CXXConstructExpr* argSubExpr = dyn_cast<CXXConstructExpr>(argExpr->IgnoreImplicit());
        if (!argSubExpr || argSubExpr->getNumArgs() == 0)
            continue;
        const DeclRefExpr* dre = dyn_cast<DeclRefExpr>(argSubExpr->getArg(0)->IgnoreImplicit());
        if (!dre)
            continue;
        const VarDecl* localVarDecl = dyn_cast<VarDecl>(dre->getDecl());
        if (!localVarDecl || localVarDecl->getType()->isReferenceType()
            || localVarDecl->getType()->isPointerType() || !localVarDecl->hasLocalStorage())
            continue;
        // because sometimes the parameter type is some obscured STL thing
        if (!isInterestingType(localVarDecl->getType()))
            continue;

        if (m_rejected.count(localVarDecl))
            continue;

        m_possibles[localVarDecl] = Possible{ argExpr, parmVarDecl, localVarDecl, recordDecl, dre };
    }

    return true;
}

bool MoveIt::VisitCXXConstructExpr(const CXXConstructExpr* topExpr)
{
    if (ignoreLocation(topExpr))
        return true;
    if (isa<CXXTemporaryObjectExpr>(topExpr))
        return true;
    const CXXConstructorDecl* methodDecl = topExpr->getConstructor();
    if (!methodDecl)
        return true;

    unsigned len = std::min(topExpr->getNumArgs(), methodDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        // check if the parameter is a moveable type
        const ParmVarDecl* parmVarDecl = methodDecl->getParamDecl(i);
        if (!parmVarDecl->getType()->isRecordType())
            continue;
        const CXXRecordDecl* recordDecl
            = dyn_cast<CXXRecordDecl>(parmVarDecl->getType()->getAsRecordDecl());
        if (!recordDecl || !recordDecl->hasMoveConstructor() || recordDecl->isTriviallyCopyable())
            continue;
        if (!isInterestingType(parmVarDecl->getType()))
            continue;

        // check if (a) we're making a copy to pass to the param and (b) we're making a copy of a local var
        const Expr* argExpr = topExpr->getArg(i);
        if (!argExpr)
            continue;
        const CXXConstructExpr* argSubExpr = dyn_cast<CXXConstructExpr>(argExpr->IgnoreImplicit());
        if (!argSubExpr || argSubExpr->getNumArgs() == 0)
            continue;
        const DeclRefExpr* dre = dyn_cast<DeclRefExpr>(argSubExpr->getArg(0)->IgnoreImplicit());
        if (!dre)
            continue;
        const VarDecl* localVarDecl = dyn_cast<VarDecl>(dre->getDecl());
        if (!localVarDecl || localVarDecl->getType()->isReferenceType()
            || localVarDecl->getType()->isPointerType() || !localVarDecl->hasLocalStorage())
            continue;
        // because sometimes the parameter type is some obscured STL thing
        if (!isInterestingType(localVarDecl->getType()))
            continue;

        if (m_rejected.count(localVarDecl))
            continue;

        m_possibles[localVarDecl] = Possible{ argExpr, parmVarDecl, localVarDecl, recordDecl, dre };
    }

    return true;
}

/// If we have pushed a possibility, and then we see that possibility again,
/// then we cannot std::move it, because it is being referenced after being moved.
///
bool MoveIt::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    if (ignoreLocation(declRefExpr))
        return true;
    const VarDecl* localVarDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!localVarDecl)
        return true;
    auto it = m_possibles.find(localVarDecl);
    if (it == m_possibles.end())
        return true;
    // ignoring the DeclRefExpr* for the expression where we found the Possible
    if (it->second.dre == declRefExpr)
        return true;
    m_possibles.erase(it);
    m_rejected.insert(localVarDecl);
    return true;
}

/// Exclude boring types, so that we don't generate too many low-value conversions.
/// e.g. for now I ignore ref-counted types like Sequence and OUString and css::uno::Reference,
/// because that generates too many changes
bool MoveIt::isInterestingType(QualType qt)
{
    if (qt->isEnumeralType())
        return false;
    if (!qt->isRecordType())
        return false;

    auto tc = loplugin::TypeCheck(qt);

    // clang-format off
    return !tc.ClassOrStruct("iterator")
           && !tc.ClassOrStruct("const_iterator")
           && !tc.Typedef("iterator")
           && !tc.Typedef("const_iterator")
           && !tc.Class("_Safe_iterator")
           && !tc.Typedef("string")
           && !tc.ClassOrStruct("shared_ptr").StdNamespace()
           && !tc.ClassOrStruct("shared_ptr").Namespace("boost")
           && !tc.Class("B2DHomMatrix").Namespace("basegfx").GlobalNamespace()
           && !tc.Class("Pipe").Namespace("osl")
           && !tc.Class("Any").Namespace("uno")
           && !tc.Class("TypeDescription").Namespace("uno")
           && !tc.Class("UnoInterfaceReference").Namespace("uno")
           && !tc.Class("ByteSequence").Namespace("rtl").GlobalNamespace()
           && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
           && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
           && !tc.Class("BinaryAny")
           && !tc.Class("Reference")
           && !tc.Class("SvRef").Namespace("tools").GlobalNamespace()
           && !tc.ClassOrStruct("sk_sp") // skia shared pointer
           && !tc.ClassOrStruct("VclPtr")
           && !tc.Typedef("IterString") // SalInstanceTreeView::IterString
           && !tc.Typedef("svtree_render_args")
           && !tc.Typedef("render_args") // weld::ComboBox::render_args
           ;
    // clang-format on
}

/// off by default because each warning needs to be hand checked to ensure it is not a false+
loplugin::Plugin::Registration<MoveIt> moveit("moveit", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
