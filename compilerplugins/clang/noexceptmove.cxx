/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// versions before 9.0 didn't have getExceptionSpecType

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

#include "config_clang.h"

#include <string>
#include <set>

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

    bool shouldVisitImplicitCode() const { return true; }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool VisitCallExpr(const CallExpr*);
    bool VisitCXXConstructExpr(const CXXConstructExpr*);
    bool VisitVarDecl(const VarDecl*);

private:
    compat::optional<bool> IsCallThrows(const CallExpr* callExpr);
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
                        && constructorDecl->isThisDeclarationADefinition()
                        && constructorDecl->getBody() != nullptr;
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
                  && methodDecl->isThisDeclarationADefinition() && methodDecl->getBody() != nullptr;
    if (isMove)
    {
        StringRef fn = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(methodDecl->getBeginLoc()));
        // SfxObjectShellLock::operator= calls SotObject::OwnerLock which in turn calls stuff which cannot be noexcept
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
    compat::optional<bool> bCallThrows = IsCallThrows(callExpr);
    if (!bCallThrows)
    {
        callExpr->dump();
        if (callExpr->getCalleeDecl())
            callExpr->getCalleeDecl()->dump();
        report(DiagnosticsEngine::Warning, "what's up doc?", callExpr->getSourceRange().getBegin())
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
    return !(est == EST_DynamicNone || est == EST_NoThrow || est == EST_BasicNoexcept
             || est == EST_NoexceptTrue);
}

bool NoExceptMove::VisitCXXConstructExpr(const CXXConstructExpr* constructExpr)
{
    if (ignoreLocation(constructExpr))
        return true;
    if (m_ConstructorThrows.empty())
        return true;
    auto constructorDecl = constructExpr->getConstructor();
    auto est = constructorDecl->getExceptionSpecType();
    if (constructorDecl->isDefaulted() && est == EST_None)
        ; // ok, non-throwing
    else if (IsCallThrowsSpec(est))
    {
        if (bLookForStuffWeCanFix)
        {
            if (est == EST_None && !ignoreLocation(constructorDecl))
                m_Exclusions.back().push_back(constructorDecl);
            else
                m_CannotFix.back() = true;
        }
        m_ConstructorThrows.back() = true;
    }
    return true;
}

bool NoExceptMove::VisitVarDecl(const VarDecl* varDecl)
{
    if (varDecl->getLocation().isValid() && ignoreLocation(varDecl))
        return true;
    if (m_ConstructorThrows.empty())
        return true;
    // The clang AST does not show me implicit calls to destructors at the end of a block,
    // so assume any local var decls of class type will call their destructor.
    if (!varDecl->getType()->isRecordType())
        return true;
    auto cxxRecordDecl = varDecl->getType()->getAsCXXRecordDecl();
    if (!cxxRecordDecl)
        return true;
    auto destructorDecl = cxxRecordDecl->getDestructor();
    if (!destructorDecl)
        return true;
    auto est = destructorDecl->getExceptionSpecType();
    if (destructorDecl->isDefaulted() && est == EST_None)
        ; // ok, non-throwing
    else if (IsCallThrowsSpec(est))
    {
        if (bLookForStuffWeCanFix)
        {
            if (est == EST_None && !ignoreLocation(destructorDecl))
                m_Exclusions.back().push_back(destructorDecl);
            else
                m_CannotFix.back() = true;
        }
        m_ConstructorThrows.back() = true;
    }
    return true;
}

compat::optional<bool> NoExceptMove::IsCallThrows(const CallExpr* callExpr)
{
    const FunctionDecl* calleeFunctionDecl = callExpr->getDirectCallee();
    if (calleeFunctionDecl)
    {
        auto est = calleeFunctionDecl->getExceptionSpecType();
        if (bLookForStuffWeCanFix)
        {
            if (est == EST_None && !ignoreLocation(calleeFunctionDecl))
                m_Exclusions.back().push_back(calleeFunctionDecl);
            else
                m_CannotFix.back() = true;
        }
        // Allowlist of functions that could be noexcept, but we can't change them because of backwards-compatibility reasons
        // css::uno::XInterface::acquire
        // css::uno::XInterface::release
        if (calleeFunctionDecl->getIdentifier())
        {
            auto name = calleeFunctionDecl->getName();
            if (auto cxxMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl))
                if (loplugin::ContextCheck(cxxMethodDecl->getParent()->getDeclContext())
                        .Namespace("uno")
                        .Namespace("star")
                        .Namespace("sun")
                        .Namespace("com")
                        .GlobalNamespace()
                    && (name == "acquire" || name == "release"))
                    return false;
            if (name == "osl_releasePipe" || name == "osl_destroySocketAddr")
                return false;
        }
        return IsCallThrowsSpec(est);
    }

    auto calleeExpr = callExpr->getCallee();
    if (isa<CXXDependentScopeMemberExpr>(calleeExpr) || isa<UnresolvedLookupExpr>(calleeExpr)
        || isa<UnresolvedMemberExpr>(calleeExpr))
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
        return compat::optional<bool>();
    }

    // allowlist of functions that could be noexcept, but we can't change them because of backwards-compatibility reasons
    if (auto typedefType = calleeType->getAs<TypedefType>())
        if (typedefType->getDecl()->getName() == "uno_ReleaseMappingFunc")
            return false;

    if (calleeType->isPointerType())
        calleeType = calleeType->getPointeeType();
    auto funcProto = calleeType->getAs<FunctionProtoType>();
    if (!funcProto)
    {
        m_CannotFix.back() = true;
        return compat::optional<bool>();
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
