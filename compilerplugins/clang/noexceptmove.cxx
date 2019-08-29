/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>

#include "plugin.hxx"

/**
  Look for move constructors that can be noexcept.
*/

namespace
{
/// Look for the stuff that can be marked noexcept, but only if we also mark some of the callees noexcept.
/// Off by default so as not too annoy people.
constexpr bool bLookForStuffWeCanFix = false;

class NoExceptMove : public loplugin::FilteringPlugin<NoExceptMove>
{
public:
    explicit NoExceptMove(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        StringRef fn(handler.getMainFileName());
        // ONDXPagePtr::operator= calls ONDXPage::ReleaseRef which cannot be noexcept
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/connectivity/source/drivers/dbase/dindexnode.cxx"))
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool VisitCallExpr(const CallExpr*);

private:
    llvm::Optional<bool> IsCallThrows(const CallExpr* callExpr);
    std::vector<bool> m_ConstructorThrows;
    std::vector<std::vector<const Decl*>> m_Exclusions;
    std::vector<bool> m_CannotFix;
};

bool NoExceptMove::TraverseCXXConstructorDecl(CXXConstructorDecl* constructorDecl)
{
    const bool isMove = constructorDecl->isMoveConstructor()
                        && constructorDecl->getExceptionSpecType() == EST_None
                        && !constructorDecl->isDefaulted() && !constructorDecl->isDeleted()
                        && !ignoreLocation(constructorDecl)
                        && constructorDecl->isThisDeclarationADefinition();
    if (isMove)
    {
        m_ConstructorThrows.push_back(false);
        m_Exclusions.emplace_back();
        m_CannotFix.push_back(false);
    }
    bool rv = RecursiveASTVisitor::TraverseCXXConstructorDecl(constructorDecl);
    if (isMove)
    {
        if (!m_ConstructorThrows.back())
        {
            report(DiagnosticsEngine::Warning, "move constructor can be noexcept",
                   constructorDecl->getSourceRange().getBegin())
                << constructorDecl->getSourceRange();
            auto canonicalDecl = constructorDecl->getCanonicalDecl();
            if (canonicalDecl != constructorDecl)
                report(DiagnosticsEngine::Note, "declaration here",
                       canonicalDecl->getSourceRange().getBegin())
                    << canonicalDecl->getSourceRange();
        }
        else if (bLookForStuffWeCanFix && !m_CannotFix.back())
        {
            report(DiagnosticsEngine::Warning, "move constructor can be noexcept",
                   constructorDecl->getSourceRange().getBegin())
                << constructorDecl->getSourceRange();
            auto canonicalDecl = constructorDecl->getCanonicalDecl();
            if (canonicalDecl != constructorDecl)
                report(DiagnosticsEngine::Note, "declaration here",
                       canonicalDecl->getSourceRange().getBegin())
                    << canonicalDecl->getSourceRange();
            for (const Decl* callDecl : m_Exclusions.back())
                report(DiagnosticsEngine::Warning, "but need to fix this to be noexcept",
                       callDecl->getSourceRange().getBegin())
                    << callDecl->getSourceRange();
        }
        m_ConstructorThrows.pop_back();
        m_Exclusions.pop_back();
        m_CannotFix.pop_back();
    }
    return rv;
}

bool NoExceptMove::TraverseCXXMethodDecl(CXXMethodDecl* methodDecl)
{
    bool isMove = methodDecl->isMoveAssignmentOperator()
                  && methodDecl->getExceptionSpecType() == EST_None && !methodDecl->isDefaulted()
                  && !methodDecl->isDeleted() && !ignoreLocation(methodDecl)
                  && methodDecl->isThisDeclarationADefinition();
    if (isMove)
    {
        StringRef fn = getFileNameOfSpellingLoc(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(methodDecl)));
        // SfxObjectShellLock::operator= calls SotObject::OwnerLock whichs in turn calls stuff which cannot be noexcept
        if (loplugin::isSamePathname(fn, SRCDIR "/include/sfx2/objsh.hxx"))
            isMove = false;
    }
    if (isMove)
    {
        m_ConstructorThrows.push_back(false);
        m_Exclusions.emplace_back();
        m_CannotFix.push_back(false);
    }
    bool rv = RecursiveASTVisitor::TraverseCXXMethodDecl(methodDecl);
    if (isMove)
    {
        if (!m_ConstructorThrows.back())
        {
            report(DiagnosticsEngine::Warning, "move operator= can be noexcept",
                   methodDecl->getSourceRange().getBegin())
                << methodDecl->getSourceRange();
            auto canonicalDecl = methodDecl->getCanonicalDecl();
            if (canonicalDecl != methodDecl)
                report(DiagnosticsEngine::Note, "declaration here",
                       canonicalDecl->getSourceRange().getBegin())
                    << canonicalDecl->getSourceRange();
        }
        else if (bLookForStuffWeCanFix && !m_CannotFix.back())
        {
            report(DiagnosticsEngine::Warning, "move operator= can be noexcept",
                   methodDecl->getSourceRange().getBegin())
                << methodDecl->getSourceRange();
            auto canonicalDecl = methodDecl->getCanonicalDecl();
            if (canonicalDecl != methodDecl)
                report(DiagnosticsEngine::Note, "declaration here",
                       canonicalDecl->getSourceRange().getBegin())
                    << canonicalDecl->getSourceRange();
            for (const Decl* callDecl : m_Exclusions.back())
                report(DiagnosticsEngine::Warning, "but need to fix this to be noexcept",
                       callDecl->getSourceRange().getBegin())
                    << callDecl->getSourceRange();
        }
        m_ConstructorThrows.pop_back();
        m_Exclusions.pop_back();
        m_CannotFix.pop_back();
    }
    return rv;
}

bool NoExceptMove::VisitCallExpr(const CallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (m_ConstructorThrows.empty())
        return true;

    llvm::Optional<bool> bCallThrows = IsCallThrows(callExpr);
    if (!bCallThrows)
    {
        callExpr->dump();
        if (callExpr->getCalleeDecl())
            callExpr->getCalleeDecl()->dump();
        report(DiagnosticsEngine::Warning, "whats up doc?", callExpr->getSourceRange().getBegin())
            << callExpr->getSourceRange();
        m_ConstructorThrows.back() = true;
        return true;
    }
    if (*bCallThrows)
        m_ConstructorThrows.back() = true;
    return true;
}

static bool IsCallThrowsSpec(clang::ExceptionSpecificationType est)
{
    return est == EST_None || est == EST_Dynamic || est == EST_MSAny;
}

llvm::Optional<bool> NoExceptMove::IsCallThrows(const CallExpr* callExpr)
{
    const FunctionDecl* calleeFunctionDecl = callExpr->getDirectCallee();
    if (calleeFunctionDecl)
    {
        auto est = calleeFunctionDecl->getExceptionSpecType();
        if (bLookForStuffWeCanFix)
        {
            if (est == EST_None
                && !ignoreLocation(calleeFunctionDecl)
                // TODO - we can change stuff in URE, just need to wrap it in LIBO_EXTERNAL_ONLY
                && !isInUnoIncludeFile(calleeFunctionDecl))
                m_Exclusions.back().push_back(calleeFunctionDecl);
            else
                m_CannotFix.back() = true;
        }
        return IsCallThrowsSpec(est);
    }

    auto calleeExpr = callExpr->getCallee();
    if (isa<CXXDependentScopeMemberExpr>(calleeExpr) || isa<UnresolvedLookupExpr>(calleeExpr))
    {
        m_CannotFix.back() = true;
        return true;
    }

    // check for call via function-pointer
    clang::QualType calleeType;
    if (auto fieldDecl = dyn_cast_or_null<FieldDecl>(callExpr->getCalleeDecl()))
        calleeType = fieldDecl->getType();
    else if (auto varDecl = dyn_cast_or_null<VarDecl>(callExpr->getCalleeDecl()))
        calleeType = varDecl->getType();
    else
    {
        m_CannotFix.back() = true;
        return llvm::Optional<bool>();
    }
    if (calleeType->isPointerType())
        calleeType = calleeType->getPointeeType();
    auto funcProto = calleeType->getAs<FunctionProtoType>();
    if (!funcProto)
    {
        m_CannotFix.back() = true;
        return llvm::Optional<bool>();
    }
    auto est = funcProto->getExceptionSpecType();
    if (bLookForStuffWeCanFix)
    {
        m_CannotFix.back() = true; // TODO, could improve
    }
    return IsCallThrowsSpec(est);
}

loplugin::Plugin::Registration<NoExceptMove> noexceptmove("noexceptmove");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
